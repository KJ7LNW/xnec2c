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

#include "opengl_rdpattern_geometry.h"
#include "../opengl-engine/opengl_cylinder.h"
#include "../shared.h"
#include "../rdpattern_ui.h"
#include "../prerender/prerender_color.h"

#ifdef HAVE_OPENGL

/* Triangle buffer for radiation pattern mesh */
static lit_color_triangle_t *rdpat_triangles = NULL;
static int rdpat_triangle_count = 0;

/* Far-field wireframe line buffer */
static lit_color_point_t *rdpat_lines = NULL;
static int rdpat_line_count = 0;

/* Near-field line buffer */
static lit_color_point_t *nf_lines = NULL;
static int nf_line_count = 0;

/* Generation counters for change detection */
static unsigned int rdpat_ff_generation = 0;
static unsigned int rdpat_nf_generation = 0;

/*-----------------------------------------------------------------------*/

/** nf_line_append() - Fill two consecutive color_point_t entries as a line segment
 * @buf: vertex buffer
 * @line_idx: starting index in buf
 * @origin: line start point
 * @endpoint: line end point
 * @color: RGBA color applied to both vertices
 *
 * Returns line_idx advanced by 2.
 */
  static int
nf_line_append(
    lit_color_point_t *buf, int line_idx,
    point_f_3d_t origin, point_f_3d_t endpoint, rgba_f_t color)
{
  /* Constant normal (0, 0, 1) gives uniform shading on line primitives */
  set_lit_vertex(&buf[line_idx],
      origin.x, origin.y, origin.z,
      0.0f, 0.0f, 1.0f,
      color.r, color.g, color.b, color.a);

  set_lit_vertex(&buf[line_idx + 1],
      endpoint.x, endpoint.y, endpoint.z,
      0.0f, 0.0f, 1.0f,
      color.r, color.g, color.b, color.a);

  return( line_idx + 2 );
}

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_generate_nf_field_lines() - Convert prerendered NF vectors to GL line geometry
 * @origins:  sample point positions [npts]
 * @npts:     number of near-field sample points
 * @fields:   dispatch-resolved vector sets (precomputed displacement + color)
 * @n_fields: number of active field sets (0-3)
 * @dr:       normalization scale distance (for endpoint computation)
 *
 * Iterates each field set, emitting one line per sample point per set.
 * Zero field-type branching — backend receives only data to iterate.
 * Returns total line count, or -1 on empty input.
 */
  int
opengl_rdpattern_generate_nf_field_lines(
    const near_field_point_t *origins, int npts,
    const nf_field_set_t *fields, int n_fields,
    double dr)
{
  int fi, idx, line_idx;
  int total_lines;

  if( n_fields <= 0 || npts <= 0 )
    return( -1 );

  total_lines = n_fields * npts;
  mem_array_realloc(&nf_lines, (size_t)total_lines * 2);

  line_idx = 0;

  for( fi = 0; fi < n_fields; fi++ )
  {
    const nf_vector_t *vecs = fields[fi].vecs;
    const rgb_f_t *colors = fields[fi].colors;

    for( idx = 0; idx < npts; idx++ )
    {
      point_f_3d_t org = {
        (float)origins[idx].px,
        (float)origins[idx].py,
        (float)origins[idx].pz
      };

      point_f_3d_t end = {
        org.x + vecs[idx].dx,
        org.y + vecs[idx].dy,
        org.z + vecs[idx].dz
      };

      rgba_f_t col = {
        colors[idx].r, colors[idx].g, colors[idx].b, 1.0f
      };

      line_idx = nf_line_append(nf_lines, line_idx, org, end, col);
    }
  }

  nf_line_count = total_lines;
  rdpat_nf_generation++;

  return( nf_line_count );

} /* opengl_rdpattern_generate_nf_field_lines() */

/*-----------------------------------------------------------------------*/

/** rdpat_line_edge() - Emit one wireframe edge as two colored vertices
 * @buf: vertex buffer
 * @vi: starting index in buf
 * @pt0: first endpoint
 * @pt1: second endpoint
 * @color: precomputed edge color from ff_pre
 *
 * Returns vi advanced by 2.
 */
  static int
rdpat_line_edge(
    lit_color_point_t *buf, int vi,
    point_3d_t *pt0, point_3d_t *pt1,
    const rgb_f_t *color)
{
  set_lit_vertex(&buf[vi],
      (float)pt0->x, (float)pt0->y, (float)pt0->z,
      0.0f, 0.0f, 1.0f,
      color->r, color->g, color->b, 1.0f);

  set_lit_vertex(&buf[vi + 1],
      (float)pt1->x, (float)pt1->y, (float)pt1->z,
      0.0f, 0.0f, 1.0f,
      color->r, color->g, color->b, 1.0f);

  return( vi + 2 );
}

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_generate_lines() - Generate wireframe line pairs from radiation pattern grid
 * @points: array of radiation pattern sample points
 * @nth: number of theta samples
 * @nph: number of phi samples
 * @theta_rgb: precomputed per-edge colors [(nth-1)*nph]
 * @phi_rgb: precomputed per-edge colors [nth*(nph-1)]
 *
 * Produces line pairs matching Cairo wireframe topology:
 *   Theta direction: (nth-1) * nph edges within each phi ring
 *   Phi direction:   nth * (nph-1) edges across adjacent phi rings
 * Returns line count, or -1 on invalid input.
 */
  int
opengl_rdpattern_generate_lines(
    point_3d_t *points, int nth, int nph,
    const rgb_f_t *theta_rgb, const rgb_f_t *phi_rgb)
{
  int nph_idx, nth_idx, vi;
  int theta_edges, phi_edges, total_lines;
  int edge_idx;

  if( !points || nth < 2 || nph < 1 || !theta_rgb || !phi_rgb )
    return( -1 );

  /* Edge counts matching Cairo wireframe topology */
  theta_edges = (nth - 1) * nph;
  phi_edges = nth * (nph - 1);
  total_lines = theta_edges + phi_edges;
  mem_array_realloc(&rdpat_lines, (size_t)total_lines * 2);

  vi = 0;
  edge_idx = 0;

  /* Theta-direction edges: adjacent theta within each phi ring */
  for( nph_idx = 0; nph_idx < nph; nph_idx++ )
  {
    for( nth_idx = 0; nth_idx < nth - 1; nth_idx++ )
    {
      int p0 = nth_idx + nph_idx * nth;
      int p1 = (nth_idx + 1) + nph_idx * nth;

      vi = rdpat_line_edge(rdpat_lines, vi,
          &points[p0], &points[p1], &theta_rgb[edge_idx]);
      edge_idx++;
    }
  }

  edge_idx = 0;

  /* Phi-direction edges: same theta across adjacent phi rings */
  for( nth_idx = 0; nth_idx < nth; nth_idx++ )
  {
    for( nph_idx = 0; nph_idx < nph - 1; nph_idx++ )
    {
      int p0 = nth_idx + nph_idx * nth;
      int p1 = nth_idx + (nph_idx + 1) * nth;

      vi = rdpat_line_edge(rdpat_lines, vi,
          &points[p0], &points[p1], &phi_rgb[edge_idx]);
      edge_idx++;
    }
  }

  rdpat_line_count = total_lines;
  rdpat_ff_generation++;

  return( rdpat_line_count );

} /* opengl_rdpattern_generate_lines() */

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_get_lines() - Return far-field wireframe line buffer and count
 * @count: output line count
 */
  lit_color_point_t*
opengl_rdpattern_get_lines(int *count)
{
  *count = rdpat_line_count;
  return( rdpat_lines );
}

/*-----------------------------------------------------------------------*/

/** fill_tri_vertex() - Fill one vertex slot of a lit_color_triangle_t from a point_3d_t
 * @tri: triangle to populate
 * @vi: vertex index within triangle (0, 1, or 2)
 * @pt: source point with position and radius
 * @normal: pre-computed smooth vertex normal
 * @color: precomputed vertex color
 */
  static void
fill_tri_vertex(
    lit_color_triangle_t *tri, int vi,
    point_3d_t *pt, point_f_3d_t *normal,
    const rgb_f_t *color)
{
  set_lit_vertex(&tri->cp[vi],
      (float)pt->x, (float)pt->y, (float)pt->z,
      normal->x, normal->y, normal->z,
      color->r, color->g, color->b, 1.0f);
}

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_generate_triangles() - Tessellate point_3d buffer into colored triangles
 * @points: array of radiation pattern sample points
 * @nth: number of theta samples
 * @nph: number of phi samples
 * @vertex_rgb: precomputed per-vertex colors [nth * nph]
 *
 * Per-vertex color derived from normalized radius through the themed ramp palette.
 * Returns triangle count, or -1 on invalid input.
 */
  int
opengl_rdpattern_generate_triangles(
    point_3d_t *points, int nth, int nph,
    const rgb_f_t *vertex_rgb)
{
  int nph_idx, nth_idx, tri_idx, vi;
  int p0, p1, p2, p3;
  int npts;
  point_f_3d_t *vertex_normals;

  if( !points || !vertex_rgb || nth < 2 || nph < 1 )
    return( -1 );

  /* Calculate triangle count: 2 per grid cell */
  rdpat_triangle_count = 2 * nph * (nth - 1);
  mem_array_realloc(&rdpat_triangles, rdpat_triangle_count);

  /* Allocate and zero temporary per-vertex normal accumulator */
  npts = nth * nph;
  vertex_normals = NULL;
  mem_array_realloc(&vertex_normals, npts);
  mem_array_zero(vertex_normals);

  /* Pass 1: accumulate face normals into each corner vertex */
  for( nph_idx = 0; nph_idx < nph; nph_idx++ )
  {
    for( nth_idx = 0; nth_idx < nth - 1; nth_idx++ )
    {
      float e1x, e1y, e1z, e2x, e2y, e2z, fnx, fny, fnz;
      point_3d_t *pp0, *pp1, *pp2, *pp3;

      p0 = nth_idx + nph_idx * nth;
      p1 = nth_idx + ((nph_idx + 1) % nph) * nth;
      p2 = (nth_idx + 1) + ((nph_idx + 1) % nph) * nth;
      p3 = (nth_idx + 1) + nph_idx * nth;

      pp0 = &points[p0];
      pp1 = &points[p1];
      pp2 = &points[p2];
      pp3 = &points[p3];

      /* Triangle A face normal: cross(p1-p0, p2-p0) */
      e1x = (float)(pp1->x - pp0->x);
      e1y = (float)(pp1->y - pp0->y);
      e1z = (float)(pp1->z - pp0->z);
      e2x = (float)(pp2->x - pp0->x);
      e2y = (float)(pp2->y - pp0->y);
      e2z = (float)(pp2->z - pp0->z);
      fnx = e1y * e2z - e1z * e2y;
      fny = e1z * e2x - e1x * e2z;
      fnz = e1x * e2y - e1y * e2x;

      vertex_normals[p0].x += fnx; vertex_normals[p0].y += fny; vertex_normals[p0].z += fnz;
      vertex_normals[p1].x += fnx; vertex_normals[p1].y += fny; vertex_normals[p1].z += fnz;
      vertex_normals[p2].x += fnx; vertex_normals[p2].y += fny; vertex_normals[p2].z += fnz;

      /* Triangle B face normal: cross(p2-p0, p3-p0) */
      e1x = (float)(pp2->x - pp0->x);
      e1y = (float)(pp2->y - pp0->y);
      e1z = (float)(pp2->z - pp0->z);
      e2x = (float)(pp3->x - pp0->x);
      e2y = (float)(pp3->y - pp0->y);
      e2z = (float)(pp3->z - pp0->z);
      fnx = e1y * e2z - e1z * e2y;
      fny = e1z * e2x - e1x * e2z;
      fnz = e1x * e2y - e1y * e2x;

      vertex_normals[p0].x += fnx; vertex_normals[p0].y += fny; vertex_normals[p0].z += fnz;
      vertex_normals[p2].x += fnx; vertex_normals[p2].y += fny; vertex_normals[p2].z += fnz;
      vertex_normals[p3].x += fnx; vertex_normals[p3].y += fny; vertex_normals[p3].z += fnz;

    } /* for( nth_idx < nth - 1 ) */
  } /* for( nph_idx < nph ) — pass 1 */

  /* Normalize accumulated vertex normals */
  for( vi = 0; vi < npts; vi++ )
  {
    float len = sqrtf(
        vertex_normals[vi].x * vertex_normals[vi].x +
        vertex_normals[vi].y * vertex_normals[vi].y +
        vertex_normals[vi].z * vertex_normals[vi].z );

    if( len > 1e-6f )
    {
      vertex_normals[vi].x /= len;
      vertex_normals[vi].y /= len;
      vertex_normals[vi].z /= len;
    }
    /* else: degenerate vertex retains zero normal — ambient only */
  }

  /* Pass 2: fill triangles with position, smooth normal, and color */
  tri_idx = 0;

  for( nph_idx = 0; nph_idx < nph; nph_idx++ )
  {
    for( nth_idx = 0; nth_idx < nth - 1; nth_idx++ )
    {
      p0 = nth_idx + nph_idx * nth;
      p1 = nth_idx + ((nph_idx + 1) % nph) * nth;
      p2 = (nth_idx + 1) + ((nph_idx + 1) % nph) * nth;
      p3 = (nth_idx + 1) + nph_idx * nth;

      /* Triangle A: p0 -> p1 -> p2 */
      fill_tri_vertex(&rdpat_triangles[tri_idx], 0,
          &points[p0], &vertex_normals[p0], &vertex_rgb[p0]);
      fill_tri_vertex(&rdpat_triangles[tri_idx], 1,
          &points[p1], &vertex_normals[p1], &vertex_rgb[p1]);
      fill_tri_vertex(&rdpat_triangles[tri_idx], 2,
          &points[p2], &vertex_normals[p2], &vertex_rgb[p2]);
      tri_idx++;

      /* Triangle B: p0 -> p2 -> p3 */
      fill_tri_vertex(&rdpat_triangles[tri_idx], 0,
          &points[p0], &vertex_normals[p0], &vertex_rgb[p0]);
      fill_tri_vertex(&rdpat_triangles[tri_idx], 1,
          &points[p2], &vertex_normals[p2], &vertex_rgb[p2]);
      fill_tri_vertex(&rdpat_triangles[tri_idx], 2,
          &points[p3], &vertex_normals[p3], &vertex_rgb[p3]);
      tri_idx++;

    } /* for( nth_idx < nth - 1 ) */
  } /* for( nph_idx < nph ) — pass 2 */

  mem_array_free(&vertex_normals);
  rdpat_ff_generation++;

  return( rdpat_triangle_count );

} /* opengl_rdpattern_generate_triangles() */

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_get_nf_lines() - Return near-field line vertex buffer and line count
 * @count: output line count
 */
  lit_color_point_t*
opengl_rdpattern_get_nf_lines(int *count)
{
  *count = nf_line_count;
  return( nf_lines );
}

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_get_triangles() - Return far-field triangle buffer and triangle count
 * @count: output triangle count
 */
  lit_color_triangle_t*
opengl_rdpattern_get_triangles(int *count)
{
  *count = rdpat_triangle_count;
  return( rdpat_triangles );
}

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_get_ff_generation() - Return current far-field generation counter
 */
  unsigned int
opengl_rdpattern_get_ff_generation(void)
{
  return( rdpat_ff_generation );
}

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_get_nf_generation() - Return current near-field generation counter
 */
  unsigned int
opengl_rdpattern_get_nf_generation(void)
{
  return( rdpat_nf_generation );
}

/*-----------------------------------------------------------------------*/

/** opengl_rdpattern_geometry_cleanup() - Free all geometry buffers and reset state
 */
  void
opengl_rdpattern_geometry_cleanup(void)
{
  mem_array_free(&rdpat_triangles);
  rdpat_triangle_count = 0;

  mem_array_free(&rdpat_lines);
  rdpat_line_count = 0;

  mem_array_free(&nf_lines);
  nf_line_count = 0;

}

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
