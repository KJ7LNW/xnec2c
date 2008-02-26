## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = xnec2c

xnec2c_SOURCES = \
	main.c		 	xnec2c.h	\
	support.c	 	support.h	\
	interface.c	 	interface.h	\
	callbacks.c	 	callbacks.h	\
	callback_func.c	xnec2c.h	\
	calculations.c	xnec2c.h	\
	cmnd_edit.c		xnec2c.h	\
	geom_edit.c	 	xnec2c.h	\
	draw.c		 	xnec2c.h	\
	draw_structure.c xnec2c.h	\
	draw_radiation.c xnec2c.h	\
	fields.c	 	xnec2c.h	\
	fork.c	 		xnec2c.h	\
	geometry.c	 	xnec2c.h	\
	ground.c	 	xnec2c.h	\
	xnec2c.c	 	xnec2c.h	\
	input.c		 	xnec2c.h	\
	matrix.c	 	xnec2c.h	\
	misc.c		 	xnec2c.h	\
	nec2_model.c 	xnec2c.h	\
	network.c	 	xnec2c.h	\
	plot_freqdata.c	xnec2c.h	\
	radiation.c	 	xnec2c.h	\
	somnec.c	 	xnec2c.h

xnec2c_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

