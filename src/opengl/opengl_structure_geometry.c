/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

#include "opengl_structure_geometry.h"
#include "opengl_patch_flow.h"
#include "opengl_structure.h"
#include "../shared.h"
#include "../prerender/prerender_state.h"
#include "../chroma/chroma_glyph.h"

#ifdef HAVE_OPENGL

#include "../opengl-engine/opengl_cylinder.h"
#include "../opengl-engine/opengl_view.h"

/* Minimum radius as fraction of model extent for visible cylinders */
#define CYLINDER_MIN_VISIBLE_FRACTION 0.0015

/* Default scale factor for cylinder radius display */
#define CYLINDER_RADIUS_SCALE_DEFAULT 1.0

/* Number of sides for cylinder rendering */
#define STRUCTURE_CYLINDER_SEGMENTS 24

/* Maximum GL_LINES vertices per network entry: every ntyp emits 2 lines
 * (4 vertices); the two-port quadrilateral fill rides on a separate batch. */
#define STRUCTURE_NETWORK_MAX_VERTS 4

/* Vertices per ntyp 1 polygon-fill instance: two GL_TRIANGLES (6 vertices) */
#define STRUCTURE_NETWORK_FILL_VERTS_PER_INSTANCE 6

/* Node/antinode tick radius as a multiple of the displayed wire radius
 * (user-scaled, clamped), so the mark clears a thick cylinder; the GL
 * analogue of the cairo pixel rule */
#define STRUCTURE_GLYPH_TICK_RADIUS_K 3.0f

/* GL_LINES vertices budgeted per marked segment: two strokes, two vertices each */
#define STRUCTURE_GLYPH_MAX_STROKE_VERTS 4

/* Line width for the node/antinode overlay marks */
#define STRUCTURE_GLYPH_LINE_WIDTH 2.0f

/* Segment-axis length floor below which the second glyph basis vector
 * falls back to the Y axis */
#define STRUCTURE_GLYPH_AXIS_LEN_MIN 1e-10

/* Network/transmission-line draw width in pixels */
#define STRUCTURE_NETWORK_LINE_WIDTH 3.0f




/* Record what the resolved patch batch consumed at its last refill. A full
 * geometry pass or a later patch refill advances the structure publication
 * version; the color version enters because this batch bakes color into its
 * vertices. */
typedef struct
{
  unsigned int         structure_generation;
  struct_draw_params_t params;
  gboolean             valid;
} patch_batch_edge_t;

/* Per-type draw batches: each batch owns its own vertex allocation and GL mode */
static gl_draw_batch_t batches[GL_VIEW_MAX_BATCHES];
static int batch_count = 0;
/* Upload key every batch publishes under, advanced by the geometry pass and by
 * the patch refill alike, so a reused slot never repeats the version its
 * previous owner published */
static unsigned int structure_geometry_generation = 1;
static const rgb_f_t *last_wire_colors = NULL;
/* Track previous radius scale to detect changes requiring regeneration */
static double structure_last_radius_scale = CYLINDER_RADIUS_SCALE_DEFAULT;
static patch_batch_edge_t patch_batch_edge;

/* Batch slot indices resolved at generation; -1 when the batch is absent.
 * Read by the per-frame visual-parameter block so rc_config values apply
 * without depending on a fixed batch ordering. */
static int structure_patch_batch_index = -1;
static int structure_net_batch_index = -1;
static int structure_net_fill_batch_index = -1;
static int structure_glyph_batch_index = -1;


/* Public accessor for shared geometry */
static structure_overlay_data_t shared_overlay_data = { 0 };

/*-----------------------------------------------------------------------*/

/* Vertices are initialized via C99 compound literal assignment:
 *   verts[vidx++] = (structure_vertex_t){ .point = {…}, .normal = {…}, … };
 * Omitted fields default to zero (uv and flow for flat vertices). */

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/

/** line_perp_normal() - Unit normal perpendicular to a line axis for lit shading
 * @dx,@dy,@dz: line direction (need not be normalized)
 * @nx,@ny,@nz: output unit normal; (0,0,1) for degenerate zero-length input
 *
 * Matches the cylinder cross-section basis so GL_LINES shade like the
 * cylinders they replace.
 */
  static void
line_perp_normal(double dx, double dy, double dz,
    float *nx, float *ny, float *nz)
{
  double len = sqrt(dx * dx + dy * dy + dz * dz);

  if( len > 1e-10 )
  {
    double ax_d = dx / len;
    double ay_d = dy / len;
    double az_d = dz / len;
    double px, py, pz, pmag;

    /* Perpendicular via cross with the least-parallel basis vector */
    if( fabs(ax_d) < 0.9 )
    {
      px = 0.0;
      py = -az_d;
      pz = ay_d;
    }
    else
    {
      px = -az_d;
      py = 0.0;
      pz = ax_d;
    }

    pmag = sqrt(px * px + py * py + pz * pz);
    *nx = (float)(px / pmag);
    *ny = (float)(py / pmag);
    *nz = (float)(pz / pmag);
  }
  else
  {
    *nx = 0.0f;
    *ny = 0.0f;
    *nz = 1.0f;
  }
}

/*-----------------------------------------------------------------------*/

/** emit_world_line() - Append one world-space GL_LINES segment with a lit normal
 * @verts: vertex buffer
 * @vidx:  current write index
 * @x1,@y1,@z1,@x2,@y2,@z2: world endpoints
 * @c:     line color
 *
 * Returns the advanced write index.
 */
  static int
emit_world_line(structure_vertex_t *verts, int vidx,
    double x1, double y1, double z1,
    double x2, double y2, double z2, rgb_f_t c)
{
  float nx, ny, nz;

  line_perp_normal(x2 - x1, y2 - y1, z2 - z1, &nx, &ny, &nz);

  verts[vidx++] = (structure_vertex_t){
      .point  = {(float)x1, (float)y1, (float)z1},
      .normal = {nx, ny, nz},
      .color  = {c.r, c.g, c.b, 1.0f},
  };
  verts[vidx++] = (structure_vertex_t){
      .point  = {(float)x2, (float)y2, (float)z2},
      .normal = {nx, ny, nz},
      .color  = {c.r, c.g, c.b, 1.0f},
  };

  return vidx;
}

/*-----------------------------------------------------------------------*/

/** generate_network_lines() - Emit GL_LINES vertices for networks and transmission lines
 * @batch: draw batch with pre-allocated vertices buffer
 *
 * Two-port networks (ntyp 1) emit only the two port-connector lines; the
 * quadrilateral boundary shape is carried by the GL_TRIANGLES fill batch
 * produced by generate_network_fill().  Straight (ntyp 2) and crossed
 * (ntyp 3) transmission lines emit two cyan lines.  Network colors are fixed,
 * independent of current/charge mode.  Sets batch vertex_count.
 */
  static void
generate_network_lines(gl_draw_batch_t *batch)
{
  int idx, vidx = 0;
  structure_vertex_t *verts = (structure_vertex_t *)batch->vertices;

  for( idx = 0; idx < netcx.nonet; idx++ )
  {
    int i1 = netcx.iseg1[idx] - 1;
    int i2 = netcx.iseg2[idx] - 1;

    /* Port endpoints in world space: a = seg i1, b = seg i2 */
    double a1x = data.segments[i1].x1, a1y = data.segments[i1].y1, a1z = data.segments[i1].z1;
    double a2x = data.segments[i1].x2, a2y = data.segments[i1].y2, a2z = data.segments[i1].z2;
    double b1x = data.segments[i2].x1, b1y = data.segments[i2].y1, b1z = data.segments[i2].z1;
    double b2x = data.segments[i2].x2, b2y = data.segments[i2].y2, b2z = data.segments[i2].z2;

    switch( netcx.ntyp[idx] )
    {
      case 1: /* Two-port network: port-connector lines; polygon fill on separate batch */
        vidx = emit_world_line(verts, vidx, a1x, a1y, a1z, b1x, b1y, b1z, COLOR_MAGENTA);
        vidx = emit_world_line(verts, vidx, a2x, a2y, a2z, b2x, b2y, b2z, COLOR_MAGENTA);
        break;

      case 2: /* Straight transmission line */
        vidx = emit_world_line(verts, vidx, a1x, a1y, a1z, b1x, b1y, b1z, COLOR_CYAN);
        vidx = emit_world_line(verts, vidx, a2x, a2y, a2z, b2x, b2y, b2z, COLOR_CYAN);
        break;

      case 3: /* Crossed transmission line */
        vidx = emit_world_line(verts, vidx, a1x, a1y, a1z, b2x, b2y, b2z, COLOR_CYAN);
        vidx = emit_world_line(verts, vidx, a2x, a2y, a2z, b1x, b1y, b1z, COLOR_CYAN);
        break;

      default:
        BUG("generate_network_lines: unknown ntyp=%d\n", netcx.ntyp[idx]);
    }
  }

  batch->vertex_count = vidx;
}

/*-----------------------------------------------------------------------*/

/** generate_network_fill() - Emit GL_TRIANGLES vertices for ntyp 1 polygon fill
 * @batch: draw batch with pre-allocated vertices buffer
 *
 * Two-port networks (ntyp 1) draw a magenta filled quadrilateral whose corners
 * sit at the one-third points between the endpoints of segments i1 and i2,
 * matching the corner formulas in generate_network_lines().  Triangulated as
 * (c0,c1,c2) and (c0,c2,c3); 6 vertices per ntyp 1 instance.  Transmission
 * line types (ntyp 2,3) contribute nothing.  Sets batch vertex_count.
 */
  static void
generate_network_fill(gl_draw_batch_t *batch)
{
  int idx, vidx = 0;
  structure_vertex_t *verts = (structure_vertex_t *)batch->vertices;

  for( idx = 0; idx < netcx.nonet; idx++ )
  {
    int i1, i2;
    double a1x, a1y, a1z, a2x, a2y, a2z;
    double b1x, b1y, b1z, b2x, b2y, b2z;
    double c0x, c0y, c0z, c1x, c1y, c1z;
    double c2x, c2y, c2z, c3x, c3y, c3z;
    float nx, ny, nz;
    rgb_f_t c = COLOR_MAGENTA;

    if( netcx.ntyp[idx] != 1 )
      continue;

    i1 = netcx.iseg1[idx] - 1;
    i2 = netcx.iseg2[idx] - 1;

    a1x = data.segments[i1].x1; a1y = data.segments[i1].y1; a1z = data.segments[i1].z1;
    a2x = data.segments[i1].x2; a2y = data.segments[i1].y2; a2z = data.segments[i1].z2;
    b1x = data.segments[i2].x1; b1y = data.segments[i2].y1; b1z = data.segments[i2].z1;
    b2x = data.segments[i2].x2; b2y = data.segments[i2].y2; b2z = data.segments[i2].z2;

    c0x = a1x + (b1x - a1x) / 3.0; c0y = a1y + (b1y - a1y) / 3.0; c0z = a1z + (b1z - a1z) / 3.0;
    c1x = b1x + (a1x - b1x) / 3.0; c1y = b1y + (a1y - b1y) / 3.0; c1z = b1z + (a1z - b1z) / 3.0;
    c2x = b2x + (a2x - b2x) / 3.0; c2y = b2y + (a2y - b2y) / 3.0; c2z = b2z + (a2z - b2z) / 3.0;
    c3x = a2x + (b2x - a2x) / 3.0; c3y = a2y + (b2y - a2y) / 3.0; c3z = a2z + (b2z - a2z) / 3.0;

    /* Normal from one quad edge so the fill shades consistently with
     * the connector lines on the same plane. */
    line_perp_normal(c1x - c0x, c1y - c0y, c1z - c0z, &nx, &ny, &nz);

    /* Triangle 1: c0, c1, c2 */
    verts[vidx++] = (structure_vertex_t){
        .point  = {(float)c0x, (float)c0y, (float)c0z},
        .normal = {nx, ny, nz},
        .color  = {c.r, c.g, c.b, 1.0f},
    };
    verts[vidx++] = (structure_vertex_t){
        .point  = {(float)c1x, (float)c1y, (float)c1z},
        .normal = {nx, ny, nz},
        .color  = {c.r, c.g, c.b, 1.0f},
    };
    verts[vidx++] = (structure_vertex_t){
        .point  = {(float)c2x, (float)c2y, (float)c2z},
        .normal = {nx, ny, nz},
        .color  = {c.r, c.g, c.b, 1.0f},
    };

    /* Triangle 2: c0, c2, c3 */
    verts[vidx++] = (structure_vertex_t){
        .point  = {(float)c0x, (float)c0y, (float)c0z},
        .normal = {nx, ny, nz},
        .color  = {c.r, c.g, c.b, 1.0f},
    };
    verts[vidx++] = (structure_vertex_t){
        .point  = {(float)c2x, (float)c2y, (float)c2z},
        .normal = {nx, ny, nz},
        .color  = {c.r, c.g, c.b, 1.0f},
    };
    verts[vidx++] = (structure_vertex_t){
        .point  = {(float)c3x, (float)c3y, (float)c3z},
        .normal = {nx, ny, nz},
        .color  = {c.r, c.g, c.b, 1.0f},
    };
  }

  batch->vertex_count = vidx;
}

/*-----------------------------------------------------------------------*/

/** generate_segments_lines() - Emit GL_LINES vertices for wire segments
 * @batch:  draw batch with pre-allocated vertices buffer
 * @params: dispatch-resolved draw parameters (precomputed colors)
 *
 * Populates batch with 2 vertices per segment. Sets vertex_count.
 */
  static void
generate_segments_lines(gl_draw_batch_t *batch, const struct_draw_params_t *params)
{
  int idx, vidx = 0;
  float r, g, b;
  structure_vertex_t *verts = (structure_vertex_t *)batch->vertices;

  for( idx = 0; idx < data.n; idx++ )
  {
    double dx, dy, dz;
    float nx, ny, nz;

    r = params->wire_colors[idx].r;
    g = params->wire_colors[idx].g;
    b = params->wire_colors[idx].b;

    /* Normal perpendicular to segment axis for consistent lighting,
     * so lines shade like the visible side of the cylinder they replace. */
    dx = data.segments[idx].x2 - data.segments[idx].x1;
    dy = data.segments[idx].y2 - data.segments[idx].y1;
    dz = data.segments[idx].z2 - data.segments[idx].z1;
    line_perp_normal(dx, dy, dz, &nx, &ny, &nz);

    /* Endpoint 1 */
    verts[vidx++] = (structure_vertex_t){
        .point  = {(float) data.segments[idx].x1,
(float) data.segments[idx].y1, (float) data.segments[idx].z1},
        .normal = {nx, ny, nz},
        .color  = {r, g, b, 1.0f},
    };

    /* Endpoint 2 */
    verts[vidx++] = (structure_vertex_t){
        .point  = {(float) data.segments[idx].x2,
(float) data.segments[idx].y2, (float) data.segments[idx].z2},
        .normal = {nx, ny, nz},
        .color  = {r, g, b, 1.0f},
    };
  }

  batch->vertex_count = vidx;
}

/*-----------------------------------------------------------------------*/

/** cyl_display_radius() - Displayed physical wire radius of a segment
 * @idx:          segment index into save.bitemp
 * @radius_scale: user-adjustable radius multiplier (ctrl+scroll)
 * @seg_scale:    dimensionless per-segment size gain
 *
 * Shared by the cylinder and glyph generators so overlay marks track the
 * rendered cylinder surface exactly.
 *
 * Holds the model radius at a scene-proportional floor, applies the uniform
 * ctrl+scroll radius scale, then applies the data-derived segment scale so the
 * carrier keeps its whole span on a model whose wires are thinner than the floor.
 */
  static double
cyl_display_radius(int idx, double radius_scale, float seg_scale)
{
  double min_visible = CYLINDER_MIN_VISIBLE_FRACTION * geom_pre.scene_radius;

  return fmax(fabs(save.bitemp[idx]), min_visible) * radius_scale * seg_scale;
}

/*-----------------------------------------------------------------------*/

/** generate_segments_glyphs() - Emit GL_LINES tick marks for node/antinode segments
 * @batch:  draw batch with pre-allocated vertices buffer
 * @params: dispatch-resolved draw parameters (wire_glyphs, segment scales)
 * @radius_scale: user-adjustable radius multiplier (ctrl+scroll)
 *
 * For each flagged segment, builds an orthonormal basis perpendicular to the
 * segment axis and maps the shape's unit strokes into world space, scaled to
 * STRUCTURE_GLYPH_TICK_RADIUS_K times the displayed wire radius so the mark
 * clears a thick cylinder at any user scale.  The engine reads only
 * glyph_shapes[] geometry and color, never the mark meaning.  Sets vertex_count.
 */
  static void
generate_segments_glyphs(gl_draw_batch_t *batch, const struct_draw_params_t *params,
    double radius_scale)
{
  int idx, vidx = 0;
  structure_vertex_t *verts = (structure_vertex_t *)batch->vertices;

  for( idx = 0; idx < data.n; idx++ )
  {
    const glyph_shape_t *gs;
    double dx, dy, dz, len;
    float nx, ny, nz;   /* first perpendicular basis vector (u) */
    float vx, vy, vz;   /* second perpendicular basis vector (v) */
    float mx, my, mz, ext;
    int k;

    if( params->wire_glyphs[idx] == GLYPH_NONE )
      continue;

    gs = &glyph_shapes[params->wire_glyphs[idx]];

    dx = data.segments[idx].x2 - data.segments[idx].x1;
    dy = data.segments[idx].y2 - data.segments[idx].y1;
    dz = data.segments[idx].z2 - data.segments[idx].z1;
    line_perp_normal(dx, dy, dz, &nx, &ny, &nz);

    /* Second basis vector: unit axis cross u, perpendicular to both */
    len = sqrt(dx * dx + dy * dy + dz * dz);
    if( len > STRUCTURE_GLYPH_AXIS_LEN_MIN )
    {
      double ux = dx / len, uy = dy / len, uz = dz / len;
      vx = (float)(uy * nz - uz * ny);
      vy = (float)(uz * nx - ux * nz);
      vz = (float)(ux * ny - uy * nx);
    }
    else
    {
      vx = 0.0f; vy = 1.0f; vz = 0.0f;
    }

    mx = 0.5f * (float)(data.segments[idx].x1 + data.segments[idx].x2);
    my = 0.5f * (float)(data.segments[idx].y1 + data.segments[idx].y2);
    mz = 0.5f * (float)(data.segments[idx].z1 + data.segments[idx].z2);
    ext = STRUCTURE_GLYPH_TICK_RADIUS_K
        * (float)cyl_display_radius(idx, radius_scale,
            params->wire_seg_scale[idx]);

    for( k = 0; k < gs->n_strokes; k++ )
    {
      const glyph_stroke_t *st = &gs->strokes[k];

      verts[vidx++] = (structure_vertex_t){
          .point  = { mx + (st->ax * nx + st->ay * vx) * ext,
                      my + (st->ax * ny + st->ay * vy) * ext,
                      mz + (st->ax * nz + st->ay * vz) * ext },
          .normal = { nx, ny, nz },
          .color  = { gs->color.r, gs->color.g, gs->color.b, 1.0f },
      };
      verts[vidx++] = (structure_vertex_t){
          .point  = { mx + (st->bx * nx + st->by * vx) * ext,
                      my + (st->bx * ny + st->by * vy) * ext,
                      mz + (st->bx * nz + st->by * vz) * ext },
          .normal = { nx, ny, nz },
          .color  = { gs->color.r, gs->color.g, gs->color.b, 1.0f },
      };
    }
  }

  batch->vertex_count = vidx;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

/** generate_segments_cylinders() - Emit GL_TRIANGLES vertices for cylinder wire segments
 * @batch:                draw batch with pre-allocated vertices buffer
 * @params:               dispatch-resolved wire colors and segment scales
 * @cylinder_radius_scale: user-adjustable radius multiplier
 *
 * Populates batch with cylinder vertices per segment. Sets vertex_count.
 */
  static void
generate_segments_cylinders(gl_draw_batch_t *batch, const struct_draw_params_t *params,
    double cylinder_radius_scale)
{
  int idx, vidx = 0;
  float r, g, b;
  structure_vertex_t *verts = (structure_vertex_t *)batch->vertices;

  /* Temp buffer for one cylinder's vertices (segments x 12) */
  int cyl_vcount = opengl_cylinder_vertex_count(STRUCTURE_CYLINDER_SEGMENTS);
  lit_color_point_t cyl_temp[STRUCTURE_CYLINDER_SEGMENTS * 12];

  for( idx = 0; idx < data.n; idx++ )
  {
    lit_cylinder_mesh_t mesh;
    double radius;
    int cyl_vidx, j;

    mesh.vertices = cyl_temp;
    mesh.vertex_count = cyl_vcount;

    r = params->wire_colors[idx].r;
    g = params->wire_colors[idx].g;
    b = params->wire_colors[idx].b;

    radius = cyl_display_radius(idx, cylinder_radius_scale,
        params->wire_seg_scale[idx]);

    {
      point_f_3d_t seg_p1 = {(float) data.segments[idx].x1,
(float) data.segments[idx].y1, (float) data.segments[idx].z1};
      point_f_3d_t seg_p2 = {(float) data.segments[idx].x2,
(float) data.segments[idx].y2, (float) data.segments[idx].z2};
      rgba_f_t seg_color = {r, g, b, 1.0f};

      cyl_vidx = opengl_lit_cylinder_append(&mesh, 0,
          &seg_p1, &seg_p2, radius, STRUCTURE_CYLINDER_SEGMENTS, &seg_color);
    }

    /* Expand lit_color_point_t -> structure_vertex_t (zero extended fields) */
    for( j = 0; j < cyl_vidx; j++ )
    {
      memset(&verts[vidx], 0, sizeof(verts[vidx]));
      verts[vidx].point  = cyl_temp[j].point;
      verts[vidx].normal = cyl_temp[j].normal;
      verts[vidx].color  = cyl_temp[j].color;
      vidx++;
    }
  }

  batch->vertex_count = vidx;
}

/*-----------------------------------------------------------------------*/

/** opengl_structure_generate_geometry() - Generate geometry for antenna wire segments, patches, and networks
 * @params:               dispatch-resolved draw parameters (precomputed colors)
 * @cylinder_radius_scale: user-adjustable radius multiplier
 *
 * Packs batches by a running index: segments always at slot 0, patches and
 * networks/transmission lines in following slots when present.  Each batch
 * owns an independent vertex allocation and GL draw mode.  The patch and
 * network slot indices are recorded for the per-frame visual-parameter pass.
 */
  static void
opengl_structure_generate_geometry(
    const struct_draw_params_t *params,
    double cylinder_radius_scale)
{
  int seg_verts;
  int bc = 0;
  int idx;
  gboolean seg_line_mode;

  structure_patch_batch_index = -1;
  structure_net_batch_index = -1;
  structure_net_fill_batch_index = -1;
  structure_glyph_batch_index = -1;

  if( data.n <= 0 && data.m <= 0 )
  {
    batch_count = 0;
    return;
  }

  /* Every slot presents with the lit program unless a producer below selects
   * another, so no slot inherits the key an earlier generation left. */
  for( idx = 0; idx < GL_VIEW_MAX_BATCHES; idx++ )
    batches[idx].program_key = GL_PROGRAM_LIT;

  seg_line_mode = (cylinder_radius_scale < CYLINDER_SCALE_LINE_THRESHOLD);

  /* Budget the segment geometry selected by its radius presentation. */
  seg_verts = seg_line_mode
    ? data.n * 2
    : data.n * opengl_cylinder_vertex_count(STRUCTURE_CYLINDER_SEGMENTS);

  /* Segment batch: always present at slot 0 */
  mem_realloc(&batches[bc].vertices,
              (size_t)seg_verts * sizeof(structure_vertex_t));

  if( seg_line_mode )
    generate_segments_lines(&batches[bc], params);
  else
    generate_segments_cylinders(&batches[bc], params,
        cylinder_radius_scale);

  batches[bc].draw_mode = seg_line_mode ? GL_LINES : GL_TRIANGLES;
  batches[bc].polygon_offset = FALSE;
  batches[bc].line_width = 1.0f;
  bc++;

  /* Reserve the patch slot; its fill strategy owns allocation and content. */
  if( data.m > 0 )
  {
    structure_patch_batch_index = bc;
    batches[bc].polygon_offset = TRUE;
    batches[bc].line_width = 1.0f;
    bc++;
  }

  /* Network and transmission-line batch when networks exist */
  if( netcx.nonet > 0 )
  {
    structure_net_batch_index = bc;
    mem_realloc(&batches[bc].vertices,
                (size_t)(netcx.nonet * STRUCTURE_NETWORK_MAX_VERTS)
                * sizeof(structure_vertex_t));

    generate_network_lines(&batches[bc]);

    batches[bc].draw_mode = GL_LINES;
    batches[bc].polygon_offset = FALSE;
    batches[bc].line_width = STRUCTURE_NETWORK_LINE_WIDTH;
    bc++;

    /* ntyp 1 polygon fill batch: GL_TRIANGLES with polygon_offset so the
     * fill recedes behind the connector lines that share its plane. */
    structure_net_fill_batch_index = bc;
    mem_realloc(&batches[bc].vertices,
                (size_t)(netcx.nonet * STRUCTURE_NETWORK_FILL_VERTS_PER_INSTANCE)
                * sizeof(structure_vertex_t));

    generate_network_fill(&batches[bc]);

    batches[bc].draw_mode = GL_TRIANGLES;
    batches[bc].polygon_offset = TRUE;
    batches[bc].line_width = 1.0f;
    bc++;
  }

  /* Node/antinode overlay batch when the projection marks wire segments */
  if( params->wire_glyphs != NULL )
  {
    structure_glyph_batch_index = bc;
    mem_realloc(&batches[bc].vertices,
                (size_t)(data.n * STRUCTURE_GLYPH_MAX_STROKE_VERTS)
                * sizeof(structure_vertex_t));

    generate_segments_glyphs(&batches[bc], params,
        cylinder_radius_scale);

    batches[bc].draw_mode = GL_LINES;
    batches[bc].polygon_offset = FALSE;
    batches[bc].line_width = STRUCTURE_GLYPH_LINE_WIDTH;
    bc++;
  }

  batch_count = bc;

  structure_last_radius_scale = cylinder_radius_scale;
  structure_geometry_generation++;

  /* Every batch this pass rebuilt carries the version that rebuilt it */
  for( idx = 0; idx < batch_count; idx++ )
    batches[idx].generation = structure_geometry_generation;
}

/*-----------------------------------------------------------------------*/

/** patch_batch_edge_make() - Capture what the patch batch consumes
 * @generation: structure publication version this capture is taken against
 * @params: dispatch-resolved structure frame
 *
 * Returns the capture built whole, so the compared capture and the stored
 * capture cannot diverge in membership.
 */
  static patch_batch_edge_t
patch_batch_edge_make(unsigned int generation,
    const struct_draw_params_t *params)
{
  return (patch_batch_edge_t){
      .structure_generation = generation,
      .params               = *params,
      .valid                = TRUE };
}

/*-----------------------------------------------------------------------*/

/** patch_batch_edge_eq() - Compare what two patch batch captures consumed
 * @a: capture the last refill stored
 * @b: capture describing what this frame consumes
 *
 * Returns TRUE when both captures carry the same structure publication
 * version, resolved flow version, and baked color version.
 */
  static gboolean
patch_batch_edge_eq(const patch_batch_edge_t *a, const patch_batch_edge_t *b)
{
  return a->valid == b->valid
      && a->structure_generation == b->structure_generation
      && a->params.patch_flow.generation == b->params.patch_flow.generation
      && a->params.color_generation == b->params.color_generation;
}

/*-----------------------------------------------------------------------*/

/** structure_patch_batch_refill() - Rebuild the resolved patch batch
 * @params: dispatch-resolved structure frame
 *
 * Rebuilds when the geometry pass owning the slot, the resolved flow frame,
 * or the baked patch color changes, and advances the one structure
 * publication counter, so the next owner of this slot cannot publish a
 * version its predecessor already held.
 */
  static void
structure_patch_batch_refill(const struct_draw_params_t *params)
{
  patch_batch_edge_t want;

  if( structure_patch_batch_index < 0 )
    return;

  want = patch_batch_edge_make(structure_geometry_generation, params);

  if( patch_batch_edge_eq(&patch_batch_edge, &want) )
    return;

  opengl_patch_flow_generate(&batches[structure_patch_batch_index], params);
  batches[structure_patch_batch_index].generation =
    ++structure_geometry_generation;

  /* Capture after publishing, so this refill does not invalidate the version
   * it just stamped */
  patch_batch_edge =
    patch_batch_edge_make(structure_geometry_generation, params);
}

/*-----------------------------------------------------------------------*/

/** opengl_structure_update_shared_geometry_with_params() - Check staleness and regenerate shared geometry
 * @params: dispatch-resolved draw parameters (precomputed colors, cmax)
 *
 * Called by the structure window's render() path.  Regenerates when colors,
 * fstep, radius scale, or geometry has changed.
 */
  void
opengl_structure_update_shared_geometry_with_params(const struct_draw_params_t *params)
{
  static int last_fstep = -1;

  /* Track freq_mhz separately: freq_step stays at steps_total for all
   * left-click (arbitrary) frequencies, so step index alone is not sufficient
   * to detect data changes in the extra slot. */
  static double last_freq_mhz = NAN;

  /* Baked projection colors reuse one scratch buffer, so pointer identity
   * alone misses rebakes; the generation counter signals new content. */
  static uint32_t last_color_generation;

  double cylinder_radius_scale;
  gboolean extra_slot_changed;
  gboolean current_colors;

  cylinder_radius_scale = opengl_structure_get_radius_scale();
  extra_slot_changed = params->fstep == calc_data.steps_total
      && !FREQ_EQ(params->freq_mhz, last_freq_mhz);
  current_colors = dl_fgt(params->cmax, -DL_EPS);

  /* Regenerate on color pointer change (mode/fstep change), empty buffer, new data, or scale change */
  if( params->wire_colors != last_wire_colors ||
      params->color_generation != last_color_generation ||
      batch_count == 0 ||
      !dl_feq(cylinder_radius_scale, structure_last_radius_scale) ||
      (current_colors && CRNT_FSTEP_AVAILABLE(params->fstep) &&
       (params->fstep != last_fstep || extra_slot_changed)) )
  {
    last_wire_colors = params->wire_colors;
    last_color_generation = params->color_generation;
    opengl_structure_generate_geometry(params, cylinder_radius_scale);

    /* Prevent redundant regeneration on subsequent expose events */
    if( current_colors )
    {
      last_fstep = params->fstep;
      last_freq_mhz = params->freq_mhz;
    }

  }

  /* The patch batch bakes resolved flow into its vertices, so it refills on
   * its own edge rather than on the geometry staleness test above. */
  structure_patch_batch_refill(params);

  /* Per-batch visual parameters read from rc_config every frame.
   * Set unconditionally so brightness/transparency slider changes
   * take effect without geometry regeneration. */
  batches[0].color_dim = rc_config.brightness_segments;
  batches[0].alpha = TRANSPARENCY_TO_ALPHA(rc_config.transparency_segments);
  if( structure_patch_batch_index >= 0 )
  {
    batches[structure_patch_batch_index].color_dim = rc_config.brightness_patches;
    batches[structure_patch_batch_index].alpha =
      TRANSPARENCY_TO_ALPHA(rc_config.transparency_patches);
  }
  if( structure_net_batch_index >= 0 )
  {
    /* Networks/transmission lines are wire-like connectors */
    batches[structure_net_batch_index].color_dim = rc_config.brightness_segments;
    batches[structure_net_batch_index].alpha =
      TRANSPARENCY_TO_ALPHA(rc_config.transparency_segments);
  }
  if( structure_net_fill_batch_index >= 0 )
  {
    /* ntyp 1 polygon fill shares the segment visual category */
    batches[structure_net_fill_batch_index].color_dim = rc_config.brightness_segments;
    batches[structure_net_fill_batch_index].alpha =
      TRANSPARENCY_TO_ALPHA(rc_config.transparency_segments);
  }
  if( structure_glyph_batch_index >= 0 )
  {
    /* Marks render at full intensity so the fixed cyan/red is not dimmed */
    batches[structure_glyph_batch_index].color_dim = 1.0f;
    batches[structure_glyph_batch_index].alpha = 1.0f;
  }

  /* Publish after every mutation this frame, so the overlay presents the
   * same vertices, program keys, and visual parameters as the scene. */
  memcpy(shared_overlay_data.batches, batches,
      (size_t)batch_count * sizeof(batches[0]));
  shared_overlay_data.batch_count = batch_count;
  shared_overlay_data.layout = &opengl_structure_layout;
  shared_overlay_data.view_scale = (float)geom_pre.scene_radius;
}

/*-----------------------------------------------------------------------*/

/** opengl_structure_get_shared_geometry() - Return read-only pointer to shared structure geometry data
 */
  const structure_overlay_data_t*
opengl_structure_get_shared_geometry(void)
{
  return( &shared_overlay_data );
}

/*-----------------------------------------------------------------------*/

/** opengl_structure_geometry_cleanup() - Free vertex buffer and reset geometry state
 */
  void
opengl_structure_geometry_cleanup(void)
{
  int i;

  for( i = 0; i < GL_VIEW_MAX_BATCHES; i++ )
    mem_free(&batches[i].vertices);

  batch_count = 0;
}

/*-----------------------------------------------------------------------*/

/** opengl_structure_geometry_invalidate() - Mark cached geometry stale so next render regenerates from NEC2 data
 */
  void
opengl_structure_geometry_invalidate(void)
{
  batch_count = 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
