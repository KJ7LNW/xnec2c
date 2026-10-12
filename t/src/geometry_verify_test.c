/*
 *  Unit tests for the near-miss junction check in verify_segments().
 *
 *  Two straight wires meet end to end across a gap.  conect() links a
 *  segment end to another end lying within SMIN times that segment's
 *  length, measured as a Manhattan distance, so a gap inside the larger of
 *  the two thresholds links the wires.  A gap beyond that threshold but
 *  within NEAR_MISS_JUNCTION_MULTIPLE times it marks a junction that a
 *  hand-typed coordinate missed, and verify_segments() reports it.  Every
 *  case runs in all four end pairings, since a wire meets its neighbor at
 *  either end.  Short-wire cases cover the relative-length check, which
 *  reports a wire under 1% of its neighbor's length whose end lies within
 *  either wire's threshold.
 *
 *  Copyright (C) 2026 eWheeler, Inc. <https://www.linuxglobal.com/>
 */

#include <stdio.h>

#include "common.h"
#include "shared.h"
#include "geometry.h"

/* Length of wire A, whose connection threshold scales every case gap */
#define WIRE_A_LENGTH 1.0

/* Radius holding every length/radius ratio far above the kernel limits */
#define WIRE_RADIUS 1.0e-4

/* Point or displacement in model coordinates */
typedef struct
{
  double x, y, z;
} test_point_t;

/* Two-wire layout and the verification result it expects */
typedef struct
{
  const char *name;

  /* Offset from wire A's junction end to wire B's, in wire A connection thresholds */
  test_point_t gap;

  /* Length of wire B */
  double length_b;

  /* Expected verify_segments() result */
  gboolean expect_valid;
} junction_case_t;

/* Orientation of each wire, selecting which of its ends meets the gap */
typedef struct
{
  const char *name;
  gboolean junction_at_end1_a;
  gboolean junction_at_end1_b;
} end_pairing_t;

static const junction_case_t junction_cases[] =
{
  { .name = "touching ends",                 .gap = { 0.0, 0.0, 0.0 }, .length_b = 1.0, .expect_valid = TRUE },
  { .name = "gap inside the threshold",      .gap = { 0.5, 0.0, 0.0 }, .length_b = 1.0, .expect_valid = TRUE },
  { .name = "gap just beyond the threshold", .gap = { 1.5, 0.0, 0.0 }, .length_b = 1.0, .expect_valid = FALSE },
  { .name = "gap near the window edge",
    .gap = { 0.9 * NEAR_MISS_JUNCTION_MULTIPLE, 0.0, 0.0 }, .length_b = 1.0, .expect_valid = FALSE },
  { .name = "gap beyond the window",
    .gap = { 1.1 * NEAR_MISS_JUNCTION_MULTIPLE, 0.0, 0.0 }, .length_b = 1.0, .expect_valid = TRUE },
  { .name = "diagonal gap inside the Euclidean but beyond the Manhattan threshold",
    .gap = { 0.6, 0.6, 0.0 }, .length_b = 1.0, .expect_valid = FALSE },
  { .name = "gap linked only by the longer wire's threshold",
    .gap = { 0.5, 0.0, 0.0 }, .length_b = 0.2, .expect_valid = TRUE },
  { .name = "short wire within the long wire's threshold",
    .gap = { 0.5, 0.0, 0.0 }, .length_b = 0.005, .expect_valid = FALSE },
  { .name = "short wire beyond the window",
    .gap = { 2.0 * NEAR_MISS_JUNCTION_MULTIPLE, 0.0, 0.0 }, .length_b = 0.005, .expect_valid = TRUE },
  { .name = NULL }
};

static const end_pairing_t end_pairings[] =
{
  { .name = "A end 2 to B end 1", .junction_at_end1_a = FALSE, .junction_at_end1_b = TRUE },
  { .name = "A end 2 to B end 2", .junction_at_end1_a = FALSE, .junction_at_end1_b = FALSE },
  { .name = "A end 1 to B end 1", .junction_at_end1_a = TRUE,  .junction_at_end1_b = TRUE },
  { .name = "A end 1 to B end 2", .junction_at_end1_a = TRUE,  .junction_at_end1_b = FALSE },
  { .name = NULL }
};

static int tests_run;
static int tests_passed;
static int tests_failed;

/*
 * place_segment - lay a wire segment between its junction end and its far end
 * @seg: segment to fill
 * @itag: tag number of the segment's wire
 * @junction: end meeting the other wire across the gap
 * @far_end: end away from the gap
 * @junction_at_end1: TRUE stores the junction as end 1, FALSE as end 2
 */
static void
place_segment(wire_segment_t *seg, int itag, test_point_t junction,
              test_point_t far_end, gboolean junction_at_end1)
{
  test_point_t end1 = junction_at_end1 ? junction : far_end;
  test_point_t end2 = junction_at_end1 ? far_end : junction;

  seg->x1 = end1.x;
  seg->y1 = end1.y;
  seg->z1 = end1.z;
  seg->x2 = end2.x;
  seg->y2 = end2.y;
  seg->z2 = end2.z;
  seg->bi = WIRE_RADIUS;
  seg->itag = itag;
}

/*
 * run_case - lay out one case's wires in one end pairing and check verify_segments()
 * @jc: gap, wire B length, and expected result
 * @pairing: which end of each wire meets the gap
 *
 * Wire A runs along x from the origin; wire B continues along x beyond the gap.
 */
static void
run_case(const junction_case_t *jc, const end_pairing_t *pairing)
{
  double threshold_a = SMIN * WIRE_A_LENGTH;
  const test_point_t far_a = { 0 };
  const test_point_t junction_a = { .x = WIRE_A_LENGTH };
  const test_point_t junction_b =
  {
    .x = junction_a.x + (jc->gap.x * threshold_a),
    .y = junction_a.y + (jc->gap.y * threshold_a),
    .z = junction_a.z + (jc->gap.z * threshold_a),
  };
  const test_point_t far_b = { .x = junction_b.x + jc->length_b, .y = junction_b.y, .z = junction_b.z };

  place_segment(&data.segments[0], 1, junction_a, far_a, pairing->junction_at_end1_a);
  place_segment(&data.segments[1], 2, junction_b, far_b, pairing->junction_at_end1_b);

  tests_run++;
  if (verify_segments() == jc->expect_valid) {
    tests_passed++;
  }
  else {
    tests_failed++;
    fprintf(stderr, "FAIL %s, %s: verify_segments() expected %s\n",
            jc->name, pairing->name, jc->expect_valid ? "TRUE" : "FALSE");
  }
}

/*
 * Run every junction case in every end pairing; exit nonzero on any failure.
 * The test reads no arguments; common.h fixes the signature.
 */
int
main(int _argc, char *_argv[])
{
  data.n = 2;
  mem_array_alloc(&data.segments, data.n);

  for (const junction_case_t *jc = junction_cases; jc->name != NULL; jc++) {
    for (const end_pairing_t *pairing = end_pairings; pairing->name != NULL; pairing++)
      run_case(jc, pairing);
  }

  mem_array_free(&data.segments);

  printf("geometry_verify_test: %d run, %d passed, %d failed\n",
         tests_run, tests_passed, tests_failed);

  return tests_failed == 0 ? 0 : 1;
}
