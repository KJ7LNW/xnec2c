## Process this file with automake to produce Makefile.in

CLEANFILES =
DISTCLEANFILES =


bin_PROGRAMS = xnec2c

xnec2c_SOURCES = \
    main.c          main.h \
    mathlib.c       mathlib.h \
    measurements.c  measurements.h \
    interface.c     interface.h \
    callbacks.c     callbacks.h \
    console.c       console.h \
    callback_func.c callback_func.h \
    calculations.c  calculations.h \
    cmnd_edit.c     cmnd_edit.h \
    geom_edit.c     geom_edit.h \
    gnuplot.c       gnuplot.h \
    draw.c          draw.h \
    draw_structure.c draw_structure.h \
    draw_radiation.c draw_radiation.h \
    fields.c        fields.h \
    fork.c          fork.h \
    geometry.c      geometry.h \
    ground.c        ground.h \
    xnec2c.c        xnec2c.h \
    input.c         input.h \
    matrix.c        matrix.h \
    utils.c         utils.h \
    nec2_model.c    nec2_model.h \
    network.c       network.h \
    optimize.c      optimize.h \
    plot_freqdata.c plot_freqdata.h \
    radiation.c     radiation.h \
    rc_config.c     rc_config.h \
    shared.c        shared.h \
    somnec.c        somnec.h \
    common.h        editors.h

xnec2c_CPPFLAGS  =
xnec2c_CFLAGS    =
xnec2c_LDFLAGS   =
xnec2c_LDADD     =

if OSX
xnec2c_LDFLAGS += -Wl,-export_dynamic
else
xnec2c_LDFLAGS += -Wl,--export-dynamic -Wl,--as-needed
endif

# Disable warnings for overlength-strings, nothing we can do about that for GTK resources.
# Maybe some compilers can't handle the length, but gcc and clang take it fine.
# See this bug reference: https://gitlab.gnome.org/GNOME/glib/-/merge_requests/763
xnec2c_CFLAGS   += --pedantic
xnec2c_CFLAGS   +=  -Wall
xnec2c_CFLAGS   += -std=gnu11
xnec2c_CFLAGS   += -O2 -g
xnec2c_CFLAGS   += -Wformat
xnec2c_CFLAGS   += -Werror=format-security
xnec2c_CFLAGS   += -fPIC
xnec2c_CFLAGS   += -Wno-overlength-strings
xnec2c_CFLAGS   += -DGTK_DISABLE_SINGLE_INCLUDES
xnec2c_CFLAGS   += -DGDK_DISABLE_DEPRECATED
xnec2c_CFLAGS   += -DGTK_DISABLE_DEPRECATED
xnec2c_CFLAGS   += -DGSEAL_ENABLE
xnec2c_CFLAGS   += -DGDK_PIXBUF_DISABLE_DEPRECATED
xnec2c_CFLAGS   += -DG_DISABLE_DEPRECATED

xnec2c_CPPFLAGS += -DPROGRAMNAME_LOCALEDIR="\"$(PROGRAMNAME_LOCALEDIR)\""
xnec2c_CPPFLAGS += -DPACKAGE_DATA_DIR="\"$(pkgdata)\""
xnec2c_CPPFLAGS += -DPACKAGE_LOCALE_DIR="\"$(prefix)/$(DATADIRNAME)/locale\""

xnec2c_CPPFLAGS += $(GMODULE_CFLAGS)
xnec2c_LDADD    += $(GMODULE_LIBS)

xnec2c_CPPFLAGS += $(GTK_CFLAGS)
xnec2c_LDADD    += $(GTK_LIBS)

xnec2c_LDADD    += $(INTLLIBS)


########################################################################
# Build xnec2c-resources.c from xnec2c.gresource.xml and other
# resource files, including automatic dependency tracking.
########################################################################
#
# A few remarks on this, as it is neither very obvious method nor is
# it well documented on the Automake side, even though there is a nice
# writeup on Auto-Dependecy Generation on the GNU make website[1].
#
#   * We follow the Automake system of unconditionally "include"ing
#     the dependency file into the Makefile, and then providing a make
#     recipe to create the dependency file with dummy no-op content,
#     so that "make" can actually work before running the compilation.
#     We just copy Automake's way for that.
#
#     The first compilation *will* happen when the *.c file must be
#     generated, and then the dependency file will be updated to the
#     actual dependency information as a side effect of that first
#     build.
#
#   * Automake places dependency files into .deps aka $(DEPDIR).  We
#     use the same principle, but use our own XNEC2C_DEPDIR variable
#     and .xnec2c-deps directory for that. Using a subdirectory and
#     a make variable are actually required for Automake to copy the
#     "include ..." line to Makefile.in verbatim so that it can end
#     up in Makefile verbatim as well.
#
#   * Automake uses extensions like *.Po for the dependency files for
#     generating a *.o file, so using something like *.Pc for the
#     dependency file creating a *.c file is logical.
#
#     If we were using Automake's $(DEPDIR), we would need to worry
#     about name collisions of dep files, but as we are using our own
#     $(XNEC2C_DEPDIR), we can use any name and follow the Automake
#     convention.
#
#   * As xnec2c-resources.c is explicitly listed in the
#     xnec2c_SOURCES, we do not need to list it in BUILT_SOURCES.
#
#   * By default, Automake distributes the generated *.c source
#     file. However, we want builds to use the glib-compile-resources
#     which came with the glib we build against, so that the APIs/ABIs
#     are guaranteed to be compatible. The dist-hook removes the *.c
#     source file from $(distdir) before it ends up in the dist
#     tarball.
#
#     The Automake option no-dist-built-sources would change that
#     Automake behaviour for the whole package, but it was only
#     introduced in automake-1.16.4 which xnec2c cannot affort to
#     require yet, and perhaps the change should not happen for the
#     whole package.
#
# [1] https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
#
########################################################################

XNEC2C_DEPDIR = .xnec2c-deps

DISTCLEANFILES += ./$(XNEC2C_DEPDIR)/xnec2c-resources.Pc
include ./$(XNEC2C_DEPDIR)/xnec2c-resources.Pc
./$(XNEC2C_DEPDIR)/xnec2c-resources.Pc:
	@$(MKDIR_P) $(@D)
	@: >>$@

xnec2c_SOURCES += xnec2c-resources.c
CLEANFILES     += xnec2c-resources.c
xnec2c-resources.c: $(top_srcdir)/resources/xnec2c.gresource.xml
	$(GLIB_COMPILE_RESOURCES) --sourcedir=$(top_srcdir)/resources --dependency-file=./$(XNEC2C_DEPDIR)/xnec2c-resources.Pc --target=xnec2c-resources.c --generate-source $(top_srcdir)/resources/xnec2c.gresource.xml

# Do not dist the generated source file so that building from tarball
# will use glib-compile-resources together with compatible libraries.
dist-hook:
	rm -f $(distdir)/xnec2c-resources.c
