# Test suite for xnec2c
# Defines unit tests for symbol expression evaluation

check_PROGRAMS = bin/sy_expr_test bin/sy_expr_extended_test bin/sy_fixture_test bin/sy_input_integration_test bin/sy_value_test bin/sy_load_overrides_test
TESTS = bin/sy_expr_test bin/sy_expr_extended_test bin/sy_fixture_test bin/sy_input_integration_test bin/sy_value_test bin/sy_load_overrides_test

bin_sy_expr_test_SOURCES = src/sy_expr_test.c \
	src/test_stubs.c \
	$(top_srcdir)/src/sy_expr.c \
	$(top_srcdir)/src/sy_expr.h \
	$(top_srcdir)/src/console.c
bin_sy_expr_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS)
bin_sy_expr_test_LDADD = $(GTK_LIBS) -lm

bin_sy_expr_extended_test_SOURCES = src/sy_expr_extended_test.c \
	src/test_stubs.c \
	$(top_srcdir)/src/sy_expr.c \
	$(top_srcdir)/src/sy_expr.h \
	$(top_srcdir)/src/console.c
bin_sy_expr_extended_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS)
bin_sy_expr_extended_test_LDADD = $(GTK_LIBS) -lm

bin_sy_fixture_test_SOURCES = src/sy_fixture_test.c \
	src/test_stubs.c \
	$(top_srcdir)/src/sy_expr.c \
	$(top_srcdir)/src/sy_expr.h \
	$(top_srcdir)/src/console.c
bin_sy_fixture_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS)
bin_sy_fixture_test_LDADD = $(GTK_LIBS) -lm

bin_sy_value_test_SOURCES = src/sy_value_test.c \
	src/test_stubs.c \
	$(top_srcdir)/src/sy_expr.c \
	$(top_srcdir)/src/sy_expr.h \
	$(top_srcdir)/src/console.c
bin_sy_value_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS)
bin_sy_value_test_LDADD = $(GTK_LIBS) -lm

bin_sy_load_overrides_test_SOURCES = src/sy_load_overrides_test.c \
	src/test_stubs.c \
	$(top_srcdir)/src/sy_expr.c \
	$(top_srcdir)/src/sy_expr.h \
	$(top_srcdir)/src/console.c
bin_sy_load_overrides_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS)
bin_sy_load_overrides_test_LDADD = $(GTK_LIBS) -lm

# Integration test calls Read_Geometry/Read_Commands from input.c to verify full file loading with SY cards
# Links against actual input.c and all dependencies to test real file parsing
# Stubs provide minimal implementations for GUI/runtime functions not needed by parsing
# utils.c excluded due to Stop/Notice conflict with stubs
bin_sy_input_integration_test_SOURCES = src/sy_input_integration_test.c \
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
bin_sy_input_integration_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS) $(GMODULE_CFLAGS)
bin_sy_input_integration_test_LDADD = $(GTK_LIBS) $(GMODULE_LIBS) -lm

EXTRA_DIST = fixtures src

CLEANFILES = *.log logs/*.log
