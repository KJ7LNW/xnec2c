## Process this file with automake to produce Makefile.in
AM_LDFLAGS = -Wl,--export-dynamic -Wl,--as-needed

# Disable warnings for overlength-strings, nothing we can do about that for GTK resources.
# Maybe some compilers can't handle the length, but gcc and clang take it fine.
# See this bug reference: https://gitlab.gnome.org/GNOME/glib/-/merge_requests/763
AM_CFLAGS = --pedantic -Wall -std=gnu11 -O2 -g \
            -Wformat -Werror=format-security -fpie \
            -Wno-overlength-strings \
            -DGTK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
            -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE \
            -DGDK_PIXBUF_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED

AM_CPPFLAGS = \
    -DPROGRAMNAME_LOCALEDIR=\""$(PROGRAMNAME_LOCALEDIR)"\" \
    -DPACKAGE_DATA_DIR=\""$(pkgdata)"\" \
    -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
    @PACKAGE_CFLAGS@

bin_PROGRAMS = xnec2c

xnec2c_SOURCES = \
    main.c          main.h \
    mathlib.c       mathlib.h \
    measurements.c  measurements.h \
    interface.c     interface.h \
    callbacks.c     callbacks.h \
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
    common.h        editors.h \
    xnec2c-resources.c

xnec2c_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

PKGCONFIG = $(shell which pkg-config)
GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)

xnec2c-resources.c: ../resources/xnec2c.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=../resources --generate-dependencies ../resources/xnec2c.gresource.xml)
	$(GLIB_COMPILE_RESOURCES) --sourcedir=../resources --target=xnec2c-resources.c --generate-source ../resources/xnec2c.gresource.xml
