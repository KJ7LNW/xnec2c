# Test suite for xnec2c
# Defines unit tests for symbol expression evaluation

check_PROGRAMS = sy_expr_test sy_expr_extended_test sy_fixture_test sy_input_integration_test
TESTS = sy_expr_test sy_expr_extended_test sy_fixture_test sy_input_integration_test

sy_expr_test_SOURCES = src/sy_expr_test.c \
	src/test_stubs.c \
	$(top_srcdir)/src/sy_expr.c \
	$(top_srcdir)/src/sy_expr.h \
	$(top_srcdir)/src/console.c
sy_expr_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS)
sy_expr_test_LDADD = $(GTK_LIBS) -lm

sy_expr_extended_test_SOURCES = src/sy_expr_extended_test.c \
	src/test_stubs.c \
	$(top_srcdir)/src/sy_expr.c \
	$(top_srcdir)/src/sy_expr.h \
	$(top_srcdir)/src/console.c
sy_expr_extended_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS)
sy_expr_extended_test_LDADD = $(GTK_LIBS) -lm

sy_fixture_test_SOURCES = src/sy_fixture_test.c \
	src/test_stubs.c \
	$(top_srcdir)/src/sy_expr.c \
	$(top_srcdir)/src/sy_expr.h \
	$(top_srcdir)/src/console.c
sy_fixture_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS)
sy_fixture_test_LDADD = $(GTK_LIBS) -lm

# Integration test calls Read_Geometry/Read_Commands from input.c to verify full file loading with SY cards
# Links against actual input.c and all dependencies to test real file parsing
# Stubs provide minimal implementations for GUI/runtime functions not needed by parsing
# utils.c excluded due to Stop/Notice conflict with stubs
sy_input_integration_test_SOURCES = src/sy_input_integration_test.c \
	src/integration_test_stubs.c \
	$(top_srcdir)/src/sy_expr.c \
	$(top_srcdir)/src/input.c \
	$(top_srcdir)/src/shared.c \
	$(top_srcdir)/src/console.c \
	$(top_srcdir)/src/geometry.c \
	$(top_srcdir)/src/mathlib.c \
	$(top_srcdir)/src/matrix.c \
	$(top_srcdir)/src/calculations.c \
	$(top_srcdir)/src/network.c \
	$(top_srcdir)/src/ground.c \
	$(top_srcdir)/src/somnec.c \
	$(top_srcdir)/src/radiation.c \
	$(top_srcdir)/src/fields.c
sy_input_integration_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS) $(GMODULE_CFLAGS)
sy_input_integration_test_LDADD = $(GTK_LIBS) $(GMODULE_LIBS) -lm

EXTRA_DIST = fixtures src

CLEANFILES = *.log logs/*.log
