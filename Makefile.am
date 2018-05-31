## Process this file with automake to produce Makefile.in

SUBDIRS = src po

EXTRA_DIST = \
	autogen.sh \
	files \
	examples \
	doc

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
	if test -d files/; then \
	  mkdir -p $(DESTDIR)/usr/share/applications/; \
	  cp files/$(PACKAGE).desktop $(DESTDIR)/usr/share/applications/; \
	  mkdir -p $(DESTDIR)/usr/share/pixmaps/; \
	  mkdir -p $(DESTDIR)/usr/share/icons/hicolor/scalable/apps/; \
	  cp files/$(PACKAGE).svg $(DESTDIR)/usr/share/pixmaps/; \
	  cp files/$(PACKAGE).svg $(DESTDIR)/usr/share/icons/hicolor/scalable/apps/; \
	  mkdir -p $(DESTDIR)/usr/share/$(PACKAGE); \
	  cp files/$(PACKAGE).glade $(DESTDIR)/usr/share/$(PACKAGE); \
	  cp files/link.svg files/unlink.svg $(DESTDIR)/usr/share/$(PACKAGE); \
	fi; \
	if !(test -d $(DESTDIR)/usr/share/doc/$(PACKAGE)); then \
	  mkdir -p $(DESTDIR)/usr/share/doc/$(PACKAGE)/examples/; \
	  cp examples/* $(DESTDIR)/usr/share/doc/$(PACKAGE)/examples/; \
	fi; \
	if test -d doc/; then \
	  cp -r doc/* $(DESTDIR)/usr/share/doc/$(PACKAGE)/; \
	  mkdir -p $(DESTDIR)/usr/share/man/man1/; \
	  cp -r doc/$(PACKAGE).1.gz $(DESTDIR)/usr/share/man/man1/; \
	fi;

