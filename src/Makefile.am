## Process this file with automake to produce Makefile.in
AM_LDFLAGS = -Wl,--as-needed

AM_CFLAGS = --pedantic -Wall -std=gnu99 -O2 -D_FORTIFY_SOURCE=2 \
		 -fstack-protector -Wformat -Werror=format-security -fpie

AM_CPPFLAGS = \
	-DPROGRAMNAME_LOCALEDIR=\""$(PROGRAMNAME_LOCALEDIR)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = xnec2c

xnec2c_SOURCES = \
	main.c		 	main.h \
	support.c	 	support.h \
	interface.c	 	interface.h \
	callbacks.c	 	callbacks.h \
	callback_func.c	callback_func.h \
	calculations.c	calculations.h \
	cmnd_edit.c		cmnd_edit.h \
	geom_edit.c	 	geom_edit.h \
	gnuplot.c	 	gnuplot.h \
	draw.c		 	draw.h \
	draw_structure.c draw_structure.h \
	draw_radiation.c draw_radiation.h \
	fields.c	 	fields.h \
	fork.c	 		fork.h \
	geometry.c	 	geometry.h \
	ground.c	 	ground.h \
	xnec2c.c	 	xnec2c.h \
	input.c		 	input.h \
	matrix.c	 	matrix.h \
	utils.c		 	utils.h \
	nec2_model.c 	nec2_model.h \
	network.c	 	network.h \
	plot_freqdata.c	plot_freqdata.h \
	radiation.c	 	radiation.h \
	shared.c	 	shared.h \
	somnec.c	 	somnec.h \
	common.h		editors.h

xnec2c_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

