/*
 * SY Card Integration Test
 * Loads fixture files through actual xnec2c parsing functions
 * Verifies SY card support works end-to-end
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <limits.h>
#include <locale.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "common.h"
#include "shared.h"
#include "sy_expr.h"

static const double TOLERANCE = 1e-6;

static int test_failures = 0;
static char fixture_base[PATH_MAX];

/* Fixture expectation structure using GW card field naming convention */
typedef struct
{
  const char *filename;
  int i1, i2;                         /* tag, segments */
  double f1, f2, f3, f4, f5, f6, f7;  /* GW card floating point fields */
} fixture_expectation_t;

/* Integer field check for data-driven verification */
typedef struct
{
  const char *name;
  int (*get_expected)(const fixture_expectation_t *);
  int (*get_actual)(void);
} int_check_t;

/* Double field check for data-driven verification */
typedef struct
{
  const char *name;
  double (*get_expected)(const fixture_expectation_t *);
  double (*get_actual)(void);
  gboolean requires_segments;
} double_check_t;

/* Integer field getters */
static int exp_i1(const fixture_expectation_t *e) { return e->i1; }
static int exp_i2(const fixture_expectation_t *e) { return e->i2; }
static int act_i1(void) { return data.segments[0].itag; }
static int act_i2(void) { return data.n; }

/* Double field getters - expected */
static double exp_f1(const fixture_expectation_t *e) { return e->f1; }
static double exp_f2(const fixture_expectation_t *e) { return e->f2; }
static double exp_f3(const fixture_expectation_t *e) { return e->f3; }
static double exp_f4(const fixture_expectation_t *e) { return e->f4; }
static double exp_f5(const fixture_expectation_t *e) { return e->f5; }
static double exp_f6(const fixture_expectation_t *e) { return e->f6; }
static double exp_f7(const fixture_expectation_t *e) { return e->f7; }

/* Double field getters - actual */
static double act_f1(void) { return data.segments[0].x1; }
static double act_f2(void) { return data.segments[0].y1; }
static double act_f3(void) { return data.segments[0].z1; }
static double act_f4(void) { return data.segments[data.n - 1].x2; }
static double act_f5(void) { return data.segments[data.n - 1].y2; }
static double act_f6(void) { return data.segments[data.n - 1].z2; }
static double act_f7(void) { return data.segments[0].bi; }

/* Integer field checks table */
static const int_check_t int_checks[] =
{
  { "I1", exp_i1, act_i1 },
  { "I2", exp_i2, act_i2 },
  { NULL, NULL, NULL }
};

/* Double field checks table */
static const double_check_t double_checks[] =
{
  { "F1", exp_f1, act_f1, FALSE },
  { "F2", exp_f2, act_f2, FALSE },
  { "F3", exp_f3, act_f3, FALSE },
  { "F4", exp_f4, act_f4, TRUE },
  { "F5", exp_f5, act_f5, TRUE },
  { "F6", exp_f6, act_f6, TRUE },
  { "F7", exp_f7, act_f7, FALSE },
  { NULL, NULL, NULL, FALSE }
};

/* Expected values for each fixture */
static const fixture_expectation_t expectations[] =
{
  {
    "sy_comma_format.nec",
    1, 5,                                    /* tag=1, segments=5 */
    0.0, 0.0, 1.0, 0.0, 0.0, 3.0, 0.001     /* a=1, c=3 */
  },
  {
    "sy_separate_cards.nec",
    1, 5,                                    /* tag=1, segments=5 */
    0.0, 0.0, 1.0, 0.0, 0.0, 3.0, 0.001     /* a=1, c=3 */
  },
  {
    "sy_math_geom.nec",
    1, 11,                                   /* tag=1, segments=11 */
    0.0, 0.0, 0.0, 0.0, 0.0, 0.068807339449541284, 0.0015  /* height=300/1090/4 */
  },
  {
    "sy_units_spaces.nec",
    1, 11,                                   /* tag=1, segments=11 */
    0.0, 0.0, 0.0, 0.0, 0.0, 0.068807339449541284, 0.0015  /* r = 1.5 * mm */
  },
  {
    "sy_math_cmnd.nec",
    1, 11,                                   /* tag=1, segments=11 */
    0.0, 0.0, 0.0, 0.0, 0.0, 0.069, 0.0015  /* hardcoded geometry */
  },
  { NULL, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }
};

/* Verify parsed geometry matches expectations */
static gboolean
verify_fixture(const fixture_expectation_t *exp)
{
  int i;
  int fail_count = 0;
  int expected_int, actual_int;
  double expected_dbl, actual_dbl;

  /* Verify integer fields */
  for( i = 0; int_checks[i].name != NULL; i++ )
  {
    expected_int = int_checks[i].get_expected(exp);
    actual_int = int_checks[i].get_actual();

    if( actual_int != expected_int )
    {
      printf("  FAIL: %s mismatch: expected %d, got %d\n",
             int_checks[i].name, expected_int, actual_int);
      fail_count++;
    }
  }

  /* Verify double fields */
  for( i = 0; double_checks[i].name != NULL; i++ )
  {
    if( double_checks[i].requires_segments && data.n <= 0 )
    {
      continue;
    }

    expected_dbl = double_checks[i].get_expected(exp);
    actual_dbl = double_checks[i].get_actual();

    if( fabs(actual_dbl - expected_dbl) > TOLERANCE )
    {
      printf("  FAIL: %s mismatch: expected %.9f, got %.9f\n",
             double_checks[i].name, expected_dbl, actual_dbl);
      fail_count++;
    }
  }

  if( fail_count > 0 )
  {
    test_failures += fail_count;
    return FALSE;
  }
  else
  {
    printf("  PASS: All values verified\n");
    return TRUE;
  }
}

/* Test loading a single fixture file */
static gboolean
test_fixture(const fixture_expectation_t *exp)
{
  gboolean ok = FALSE;
  char full_path[PATH_MAX];

  snprintf(full_path, sizeof(full_path), "%s/%s", fixture_base, exp->filename);
  printf("Testing: %s\n", full_path);

  Open_File(&input_fp, full_path, "r");
  if( input_fp == NULL )
  {
    printf("  FAIL: Could not open file: %s\n", full_path);
    test_failures++;
    return FALSE;
  }

  ok = Read_Comments() && Read_Geometry() && Read_Commands();
  Close_File(&input_fp);

  if( ok )
  {
    printf("  PASS: File parsed successfully\n");
    ok = verify_fixture(exp);
  }
  else
  {
    printf("  FAIL: Parse error\n");
    test_failures++;
  }

  return ok;
}

/* Cap the calling process address space at its current footprint plus a
 * fixed margin. A regression of the locale decimal-point bug appends to a
 * GArray without bound; this limit makes the allocator fail and the child
 * die at the margin instead of exhausting host memory. */
static gboolean
limit_address_space(void)
{
  FILE *fp;
  unsigned long vm_pages = 0;
  unsigned long base;
  struct rlimit rl;
  rlim_t desired;

  fp = fopen("/proc/self/statm", "r");
  if( fp != NULL )
  {
    if( fscanf(fp, "%lu", &vm_pages) != 1 )
      vm_pages = 0;
    fclose(fp);
  }

  base = vm_pages * (unsigned long)sysconf(_SC_PAGESIZE);
  if( base == 0 )
    base = 4UL * 1024 * 1024 * 1024;   /* baseline unknown: absolute floor */

  /* Read the current hard limit and lower only the soft limit, so the
   * call cannot fail by attempting to raise an unprivileged hard limit. */
  if( getrlimit(RLIMIT_AS, &rl) != 0 )
  {
    printf("  FAIL: getrlimit(RLIMIT_AS) failed: %s\n", strerror(errno));
    return FALSE;
  }

  desired = (rlim_t)base + (rlim_t)512 * 1024 * 1024;
  if( rl.rlim_max == RLIM_INFINITY || desired < rl.rlim_max )
    rl.rlim_cur = desired;
  else
    rl.rlim_cur = rl.rlim_max;

  if( setrlimit(RLIMIT_AS, &rl) != 0 )
  {
    printf("  FAIL: setrlimit(RLIMIT_AS, %lu bytes) failed: %s\n",
           (unsigned long)rl.rlim_cur, strerror(errno));
    return FALSE;
  }

  return TRUE;
}

/* Parse every fixture under the German numeric locale. NEC files are
 * C-format regardless of locale, so the results must match the same
 * expectations. Returns FALSE on any mismatch; TRUE when the locale is
 * unavailable so an unconfigured host does not report a failure. */
static gboolean
de_locale_body(void)
{
  gboolean ok = TRUE;
  int i;

  if( setlocale(LC_NUMERIC, "de_DE.UTF-8") == NULL )
  {
    printf("  SKIP: de_DE.UTF-8 locale unavailable\n");
    return TRUE;
  }

  for( i = 0; expectations[i].filename != NULL; i++ )
  {
    if( !test_fixture(&expectations[i]) )
      ok = FALSE;
  }

  return ok;
}

/* Run body() in a memory-bounded child so a non-terminating allocator
 * loop cannot crash the host. The locale change stays isolated to the
 * child. Returns TRUE only on a clean child exit. */
static gboolean
run_bounded(gboolean (*body)(void))
{
  pid_t pid;
  int status;
  gboolean ok;

  /* Drain the parent buffer before forking so the child does not inherit
   * and later re-emit the parent's pending block-buffered output. */
  fflush(stdout);

  pid = fork();
  if( pid < 0 )
  {
    printf("  FAIL: fork() failed\n");
    ok = FALSE;
  }
  else if( pid == 0 )
  {
    gboolean child_ok;

    /* Refuse to run unbounded: an unlimited child could exhaust the host. */
    if( !limit_address_space() )
    {
      fflush(stdout);
      _exit( 3 );
    }

    child_ok = body();

    /* _exit() skips stdio flushing; flush so the child diagnostics survive. */
    fflush(stdout);
    _exit( child_ok ? 0 : 2 );
  }
  else
  {
    if( waitpid(pid, &status, 0) < 0 )
    {
      printf("  FAIL: waitpid() failed\n");
      ok = FALSE;
    }
    else if( WIFSIGNALED(status) )
    {
      printf("  FAIL: child terminated by signal %d (memory bound or crash)\n",
             WTERMSIG(status));
      ok = FALSE;
    }
    else if( WIFEXITED(status) && WEXITSTATUS(status) == 0 )
    {
      ok = TRUE;
    }
    else
    {
      printf("  FAIL: child exit status %d\n",
             WIFEXITED(status) ? WEXITSTATUS(status) : -1);
      ok = FALSE;
    }
  }

  return ok;
}

int
main(int argc, char *argv[])
{
  int i;
  int exit_code;
  char exe_path[PATH_MAX];
  char *exe_dir;
  ssize_t len;
  const char *display_env;

  /* Fail hard without a display, naming the DISPLAY value and the remedy */
  display_env = g_getenv("DISPLAY");
  if( !gtk_init_check(&argc, &argv) )
  {
    fprintf(stderr, "GTK cannot open a display (DISPLAY=\"%s\"); run under an X11 session or xvfb-run\n",
      display_env != NULL ? display_env : "(unset)");
    return 1;
  }

  len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
  if( len != -1 )
  {
    exe_path[len] = '\0';
    exe_dir = dirname(exe_path);
    snprintf(fixture_base, sizeof(fixture_base), "%s/../fixtures", exe_dir);
  }
  else
  {
    snprintf(fixture_base, sizeof(fixture_base), "fixtures");
  }

  printf("=== SY Card Integration Test ===\n");
  printf("Fixture directory: %s\n\n", fixture_base);

  if( !sy_init() )
  {
    printf("FATAL: sy_init() failed\n");
    return 1;
  }

  for( i = 0; expectations[i].filename != NULL; i++ )
  {
    test_fixture(&expectations[i]);
  }

  printf("\n--- DE locale (memory-bounded) ---\n");
  if( !run_bounded(de_locale_body) )
    test_failures++;

  sy_cleanup();

  printf("\n=== Test Summary ===\n");
  if( test_failures == 0 )
  {
    printf("All tests PASSED\n");
    exit_code = 0;
  }
  else
  {
    printf("%d test(s) FAILED\n", test_failures);
    exit_code = 1;
  }

  return exit_code;
}
