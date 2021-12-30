## Process this file with automake to produce Makefile.in

SUBDIRS = src po

EXTRA_DIST = \
	autogen.sh \
	files \
	examples \
	doc \
	.xnec2c

DISTCLEANFILES = \
				 autom4te.cache/* \
				 intl/* \
				 m4/* \
				 aclocal.m4 \
				 compile \
				 config.guess \
				 config.rpath \
				 config.status \
				 config.sub \
				 configure \
				 depcomp \
				 install-sh \
				 missing \
				 ABOUT-NLS \
				 Makefile.in \
				 config.h \
				 config.h.in \
				 config.log \
				 configure \
				 stamp-h1 \
				 stamp-h.in \
				 src/Makefile.in \
				 po/Rules-quot \
				 po/en@quot.header \
				 po/remove-potcdate.sin \
				 po/Makevars.template \
				 po/boldquot.sed \
				 po/insert-header.sin \
				 po/stamp-po \
				 po/en@boldquot.header \
				 po/quot.sed \
				 po/Makefile.in.in \
				 po/xnec2c.pot \
				 Makefile

install-data-local:
	@$(NORMAL_INSTALL)
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir); \
	if test -d files/; then \
	  $(MKDIR_P) $(DESTDIR)$(datarootdir)/applications;\
	  $(MKDIR_P) $(DESTDIR)$(datarootdir)/pixmaps;\
	  $(install_sh_DATA) files/$(PACKAGE).desktop $(DESTDIR)$(datarootdir)/applications/; \
	fi; \
	if test -d resources/; then \
	  $(install_sh_DATA) resources/$(PACKAGE).svg $(DESTDIR)$(datarootdir)/pixmaps/; \
	fi; \
	if test -d doc/; then \
	  $(MKDIR_P) $(DESTDIR)$(docdir)/images/;\
	  $(MKDIR_P) $(DESTDIR)$(mandir)/man1;\
	  $(install_sh_DATA) doc/*.txt $(DESTDIR)$(docdir)/; \
	  $(install_sh_DATA) doc/$(PACKAGE).html $(DESTDIR)$(docdir)/; \
	  $(install_sh_DATA) doc/images/* $(DESTDIR)$(docdir)/images/; \
	  $(install_sh_DATA) doc/$(PACKAGE).1.gz $(DESTDIR)$(mandir)/man1/; \
	  ln -s $(DESTDIR)$(docdir) $(DESTDIR)$(pkgdatadir)/doc; \
	fi; \
	if test -d examples/; then \
	  $(MKDIR_P) $(DESTDIR)$(pkgdatadir)/examples;\
	  cp -r examples/* $(DESTDIR)$(pkgdatadir)/examples/; \
	fi;

