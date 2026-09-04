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

#include <string.h>

#include "opengl_rdpattern.h"
#include "opengl_rdpattern_geometry.h"
#include "../settings/render_settings.h"
#include "opengl_state.h"
#include "opengl_structure.h"
#include "opengl_structure_geometry.h"
#include "../shared.h"
#include "../rdpattern_ui.h"

#ifdef HAVE_OPENGL

#include "../opengl-engine/opengl_view.h"
#include "../opengl-engine/opengl_view_fit.h"
#include "../render/render_canvas.h"
#include "../render/render_dispatch.h"

/* Surface color dim values in rc_config.brightness_rdpat_surface
 * and rc_config.brightness_rdpat_wire (applied via u_color_dim
 * shader uniform in the engine).
 *
 * In "both" mode the surface is dimmed further so wireframe lines
 * stand out.  The ratio preserves the original relationship between
 * RDPAT_SURFACE_BOTH_DIM (0.30) and RDPAT_SURFACE_DIM (0.47). */
#define RDPAT_BOTH_SURFACE_DIM_RATIO 0.64f


/* Translated far-field points buffer for excitation center offset */
static point_3d_t *rdpat_translated_points = NULL;


/* Validity snapshot for the cached far-field mesh tessellation.
 * Bundles the producer data-version with the GL presentation parameters
 * that shape the mesh; a mismatch against current state forces
 * re-tessellation.  fp->generation is per-slot and not globally unique,
 * so fstep is part of the snapshot to disambiguate slots whose
 * generations coincide. */
typedef struct
{
  int      fstep;       /* slot the mesh was tessellated from */
  uint32_t generation;  /* fp->generation captured at tessellation */
  int      draw_style;  /* rc_config.rdpattern_draw_style */
  float    off_len;     /* excitation translation magnitude */
} rdpat_mesh_cache_t;


/* rdpat_mesh_cache_capture() - Snapshot current mesh-validity inputs
 * @fstep:      active far-field slot index
 * @generation: fp->generation for that slot
 * @off_len:    excitation translation magnitude from dispatch
 *
 * draw_style is read from the engine-specific render config. */
  static rdpat_mesh_cache_t
rdpat_mesh_cache_capture(int fstep, uint32_t generation, float off_len)
{
  rdpat_mesh_cache_t snap = {0};
  snap.fstep      = fstep;
  snap.generation = generation;
  snap.draw_style = rc_config.rdpattern_draw_style;
  snap.off_len    = off_len;
  return snap;
}

/* rdpat_mesh_cache_match() - Single comparison point for mesh-validity
 * @a: stored snapshot
 * @b: freshly captured snapshot
 *
 * Bytewise equality; the struct has no internal padding (four 4-byte
 * members) and both operands originate from = {0} capture, so memcmp
 * is well-defined. */
  static inline int
rdpat_mesh_cache_match(const rdpat_mesh_cache_t *a,
    const rdpat_mesh_cache_t *b)
{
  return memcmp(a, b, sizeof(*a)) == 0;
}


/*-----------------------------------------------------------------------*/

/** gl_rdpat_draw_field_vectors() - Vector leaf: convert resolved vectors to a GL batch
 * @surface: GL surface holding the frame content
 * @sets:    dispatch-resolved vector sets, terminated by absent entries
 * @r_max:   maximum distance from origin for view scaling
 *
 * One GL batch carries every set, appended to the batches the frame already
 * holds so a pattern surface drawn before it keeps its own.  Backend
 * iterates — zero field-type branching.
 */
  gboolean
gl_rdpat_draw_field_vectors(render_surface_t *surface,
    const field_vector_set_t *sets, double r_max)
{
  gl_view_content_t *out = &gl_view_state(surface)->content;
  lit_color_point_t *line_buf;
  gl_draw_batch_t *batch;
  double extent = 0.0;
  float clip;
  int total_lines, line_count, idx;

  if( out->batch_count >= GL_VIEW_MAX_BATCHES )
  {
    BUG("field vectors: frame already holds %d batches\n", out->batch_count);
    return FALSE;
  }

  total_lines = opengl_rdpattern_generate_field_vector_lines(sets);
  if( total_lines <= 0 )
    return FALSE;

  /* The longest displacement any set draws sets the clip allowance */
  for( idx = 0; sets[idx].entries != NULL; idx++ )
    if( sets[idx].extent > extent )
      extent = sets[idx].extent;

  line_buf = opengl_rdpattern_get_field_vector_lines(&line_count);

  batch = &out->batches[out->batch_count];
  batch->vertices = line_buf;
  batch->vertex_count = line_count * 2;
  batch->draw_mode = GL_LINES;
  batch->polygon_offset = FALSE;
  batch->line_width = 1.0f;
  batch->color_dim = rc_config.brightness_nearfield;
  batch->alpha = TRANSPARENCY_TO_ALPHA(rc_config.transparency_nearfield);
  batch->program_key = GL_PROGRAM_LIT;
  batch->generation = opengl_rdpattern_get_field_vector_generation();
  out->batch_count++;

  out->layout = &opengl_lit_point_layout;
  out->r_max = (float)r_max;
  out->model_scale = 1.0f;

  /* Arrows reach one extent beyond the content they attach to */
  clip = (float)(r_max + extent);
  if( clip > out->clip_extent )
    out->clip_extent = clip;

  return TRUE;
}

/*-----------------------------------------------------------------------*/

/** gl_rdpat_draw_farfield() - Far-field leaf: tessellate gain pattern and populate batches
 * @surface: GL surface holding the frame content
 * @fstep: current frequency step, indexes ff_pre[] for vertex data
 * @ff:    dispatch-resolved far-field draw parameters
 *
 * Returns TRUE when batches are populated, FALSE on data dependency failure.
 */
  gboolean
gl_rdpat_draw_farfield(render_surface_t *surface, int fstep,
    const ff_draw_params_t *ff)
{
  static rdpat_mesh_cache_t mesh_cache = {
    .fstep = -1, .generation = 0, .draw_style = -1, .off_len = NAN };
  gl_view_content_t *out = &gl_view_state(surface)->content;
  uint32_t current_gen;
  int batch_idx, nth, nph, npts;
  point_3d_t *verts;
  point_3d_t *points_to_use;
  gboolean translate_to_excitation;

  if( ff_pre == NULL || fstep < 0 )
    return FALSE;

  ff_pre_t *fp = &ff_pre[fstep];
  nth = fpat.nth;
  nph = fpat.nph;
  npts = nth * nph;

  if( npts <= 0 || fp->vertices == NULL )
    return FALSE;

  verts = fp->vertices;
  current_gen = fp->generation;

  translate_to_excitation = (ff->off_len > FF_EXCITATION_OFFSET_MIN);
  points_to_use = verts;

  if( translate_to_excitation )
  {
    int i;

    if( npts > mem_array_capacity(rdpat_translated_points))
    {
      mem_array_realloc(&rdpat_translated_points, npts);
    }

    /* ff->x/y/z already pre-scaled to pattern space by dispatch */
    for( i = 0; i < npts; i++ )
    {
      rdpat_translated_points[i].x = verts[i].x + (double)ff->x;
      rdpat_translated_points[i].y = verts[i].y + (double)ff->y;
      rdpat_translated_points[i].z = verts[i].z + (double)ff->z;
      rdpat_translated_points[i].r = verts[i].r;
    }

    points_to_use = rdpat_translated_points;
  }

  gboolean need_tris =
    (rc_config.rdpattern_draw_style == RDPAT_STYLE_SURFACE ||
     rc_config.rdpattern_draw_style == RDPAT_STYLE_BOTH);
  gboolean need_lines =
    (rc_config.rdpattern_draw_style == RDPAT_STYLE_WIREFRAME ||
     rc_config.rdpattern_draw_style == RDPAT_STYLE_BOTH);
  int cached_tri_count, cached_line_count;

  opengl_rdpattern_get_triangles(&cached_tri_count);
  opengl_rdpattern_get_lines(&cached_line_count);

  rdpat_mesh_cache_t cur =
      rdpat_mesh_cache_capture(fstep, current_gen, ff->off_len);

  /* Regenerate geometry on data change, translation change, draw style change,
   * or frequency step change.  fp->generation is a per-slot counter and is not
   * globally unique, so switching to a slot whose generation coincides with the
   * last rendered slot must still force a rebuild.  A matching key records only
   * the inputs the last tessellation ran on; closing the window frees the mesh
   * buffers through opengl_rdpattern_geometry_cleanup() while this key
   * survives, so read the buffer counts to confirm the meshes the active draw
   * style needs are still resident. */
  if( !rdpat_mesh_cache_match(&mesh_cache, &cur) ||
      (need_tris && cached_tri_count == 0) ||
      (need_lines && cached_line_count == 0) )
  {
    if( need_tris )
    {
      int tri_count = opengl_rdpattern_generate_triangles(
          points_to_use, nth, nph, fp->vertex_rgb);

      if( tri_count <= 0 )
        return FALSE;
    }

    if( need_lines )
    {
      int line_count = opengl_rdpattern_generate_lines(
          points_to_use, nth, nph, fp->theta_rgb, fp->phi_rgb);

      if( line_count <= 0 )
        return FALSE;
    }

    mesh_cache = cur;
  }

  /* Populate batches per draw style */
  switch( rc_config.rdpattern_draw_style )
  {
    case RDPAT_STYLE_SURFACE:
    {
      int tri_count;
      lit_color_triangle_t *tri_buf =
        opengl_rdpattern_get_triangles(&tri_count);

      if( tri_count == 0 )
        return FALSE;

      out->batches[0].vertices = tri_buf;
      out->batches[0].vertex_count = tri_count * 3;
      out->batches[0].draw_mode = GL_TRIANGLES;
      out->batches[0].polygon_offset = FALSE;
      out->batches[0].line_width = 1.0f;
      out->batches[0].color_dim = rc_config.brightness_rdpat_surface;
      out->batches[0].alpha = TRANSPARENCY_TO_ALPHA(rc_config.transparency_rdpat_surface);
      out->batch_count = 1;
      break;
    }

    case RDPAT_STYLE_WIREFRAME:
    {
      int line_count;
      lit_color_point_t *line_buf =
        opengl_rdpattern_get_lines(&line_count);

      if( line_count == 0 )
        return FALSE;

      out->batches[0].vertices = line_buf;
      out->batches[0].vertex_count = line_count * 2;
      out->batches[0].draw_mode = GL_LINES;
      out->batches[0].polygon_offset = FALSE;
      out->batches[0].line_width = 1.0f;
      out->batches[0].color_dim = rc_config.brightness_rdpat_wire;
      out->batches[0].alpha = TRANSPARENCY_TO_ALPHA(rc_config.transparency_rdpat_wire);
      out->batch_count = 1;
      break;
    }

    case RDPAT_STYLE_BOTH:
    {
      int tri_count, line_count;
      lit_color_triangle_t *tri_buf =
        opengl_rdpattern_get_triangles(&tri_count);
      lit_color_point_t *line_buf =
        opengl_rdpattern_get_lines(&line_count);

      if( tri_count == 0 || line_count == 0 )
        return FALSE;

      /* Surface pushed behind wireframe via glPolygonOffset */
      out->batches[0].vertices = tri_buf;
      out->batches[0].vertex_count = tri_count * 3;
      out->batches[0].draw_mode = GL_TRIANGLES;
      out->batches[0].polygon_offset = TRUE;
      out->batches[0].line_width = 1.0f;
      out->batches[0].color_dim =
          rc_config.brightness_rdpat_surface * RDPAT_BOTH_SURFACE_DIM_RATIO;
      out->batches[0].alpha = TRANSPARENCY_TO_ALPHA(rc_config.transparency_rdpat_surface);

      out->batches[1].vertices = line_buf;
      out->batches[1].vertex_count = line_count * 2;
      out->batches[1].draw_mode = GL_LINES;
      out->batches[1].polygon_offset = FALSE;
      out->batches[1].line_width = 1.0f;
      out->batches[1].color_dim = rc_config.brightness_rdpat_wire;
      out->batches[1].alpha = TRANSPARENCY_TO_ALPHA(rc_config.transparency_rdpat_wire);
      out->batch_count = 2;
      break;
    }

    default:
      pr_err("rdpattern: invalid draw style %d, using surface\n",
          rc_config.rdpattern_draw_style);
      rc_config.rdpattern_draw_style = RDPAT_STYLE_SURFACE;
      return FALSE;
  }

  out->layout = &opengl_lit_point_layout;

  /* pattern_radius from dispatch (ff_presentation_recompute result) */
  out->r_max = ff->pattern_radius;

  /* Clip extent accounts for excitation center translation */
  out->clip_extent = ff->pattern_radius + ff->off_len;
  out->model_scale = 1.0f;

  /* Every mesh batch this pass filled carries the far-field version */
  for( batch_idx = 0; batch_idx < out->batch_count; batch_idx++ )
  {
    out->batches[batch_idx].program_key = GL_PROGRAM_LIT;
    out->batches[batch_idx].generation =
      opengl_rdpattern_get_ff_generation();
  }

  return TRUE;
}

/*-----------------------------------------------------------------------*/

/* gl_ops defined in opengl_ops.c; declared in opengl_structure.h */

/*-----------------------------------------------------------------------*/

/** rdpattern_content_cleanup() - Release the radiation geometry caches
 */
  static void
rdpattern_content_cleanup(void)
{
  opengl_rdpattern_geometry_cleanup();

  mem_array_free(&rdpat_translated_points);
}

/*-----------------------------------------------------------------------*/

/* Modifier scroll capabilities of the radiation-pattern domain */
static const surface_input_ops_t rdpattern_input_ops = {
  .by_modifier = {
    [SURFACE_MOD_CTRL]  = &opengl_structure_wire_radius_cap,
    [SURFACE_MOD_SHIFT] = &rdpattern_overlay_scale_cap
  }
};

/* Static view configuration */
static gl_view_config_t rdpattern_view_config = {
  .presents_overlay = TRUE,
  .on_gl_init_failed = opengl_gl_init_failed,
  .content_cleanup = rdpattern_content_cleanup
};

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_surface_new() - Build the radiation-pattern GL surface
 * @parent: container the presented widget joins
 */
  render_surface_t *
opengl_rdpattern_surface_new(GtkContainer *parent)
{
  render_surface_t *surface;

  surface = gl_view_surface_new( &rdpattern_view_config,
      &rdpattern_input_ops, rdpattern_view, parent );

  if( surface != NULL )
    gtk_widget_show( surface->widget );

  return( surface );
}

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_cleanup_impl() - Destroy the rdpattern GL area
 *
 * Unrealizing the widget drives gl_view_gpu_release(), which releases the
 * GPU resources and invokes the configured content cleanup; the canvas
 * releases the surface itself.
 */
  static void
opengl_rdpattern_cleanup_impl(void)
{
  render_surface_t *surface = canvas_surface_of( CANVAS_RDPATTERN,
      &gl_engine );

  if( surface == NULL )
    return;

  gtk_widget_destroy( surface->widget );
}

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_cleanup() - Public API: cleanup rdpattern resources
 */
  void
opengl_rdpattern_cleanup(void)
{
#ifdef HAVE_OPENGL
  opengl_rdpattern_cleanup_impl();
#endif
}

/*-----------------------------------------------------------------------*/
