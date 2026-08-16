## Process this file with automake to produce Makefile.in

# The clean target below intentionally overrides automake's; silence
# the override warning.
AUTOMAKE_OPTIONS = -Wno-override

CLEANFILES = .vscode/c_cpp_properties.json
EXTRA_DIST =
SUFFIXES =
man1_MANS =
noinst_DATA =

SUBDIRS = src po t

# Quiet clean, kernel style: automake's generated rm rules echo every
# command and its recursion prints "Making clean in ..." banners, so
# walk SUBDIRS directly under make -s; make clean V=1 shows everything.
clean:
	@if $(AM_V_P); then \
		$(MAKE) clean-recursive; \
	else \
		for d in $(SUBDIRS) .; do \
			echo "  CLEAN    $$d"; \
			if test "$$d" = .; then \
				$(MAKE) -s --no-print-directory clean-am || exit 1; \
			else \
				$(MAKE) -s --no-print-directory -C $$d clean || exit 1; \
			fi; \
		done; \
	fi

# Keep po/POTFILES.in synced with git-tracked sources on every make in
# git checkouts; tarball builds lack .git and skip silently.
BUILT_SOURCES = potfiles-sync
.PHONY: potfiles-sync
potfiles-sync:
	@if test -d $(top_srcdir)/.git; then \
		$(top_srcdir)/scripts/po/po-update.sh --sync; \
	fi

EXTRA_DIST += autogen.sh

EXTRA_DIST += .gitignore
EXTRA_DIST += m4/.gitignore

EXTRA_DIST += debian/upstream/metadata

EXTRA_DIST += MAINTAINER
EXTRA_DIST += README.md
EXTRA_DIST += PACKAGING

EXTRA_DIST += m4-include/ax_check_compile_flag.m4
EXTRA_DIST += m4-include/pkg.m4

EXTRA_DIST += resources/link.svg
EXTRA_DIST += resources/unlink.svg
EXTRA_DIST += resources/xnec2c.glade
EXTRA_DIST += resources/sy_overrides.glade
EXTRA_DIST += resources/xnec2c.gresource.xml

SED_CMDS  =
SED_CMDS += -e 's|[@]PACKAGE_NAME@|$(PACKAGE_NAME)|g'
SED_CMDS += -e 's|[@]PACKAGE_STRING@|$(PACKAGE_STRING)|g'
SED_CMDS += -e 's|[@]PACKAGE_TARNAME@|$(PACKAGE_TARNAME)|g'
SED_CMDS += -e 's|[@]PACKAGE_VERSION@|$(PACKAGE_VERSION)|g'


########################################################################
# Desktop integration
########################################################################

applicationsdir        = $(datarootdir)/applications

if HAVE_PROG_MSGFMT
files/xnec2c.desktop: $(srcdir)/files/xnec2c.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template=$(srcdir)/files/xnec2c.desktop.in \
	  -d $(top_srcdir)/po \
	  -o $@
else
files/xnec2c.desktop: $(srcdir)/files/xnec2c.desktop.in
	$(AM_V_GEN)cp $(srcdir)/files/xnec2c.desktop.in $@
endif

dist_applications_DATA = files/xnec2c.desktop

CLEANFILES += files/xnec2c.desktop

appiconsdir            = $(datarootdir)/icons/hicolor/256x256/apps
dist_appicons_DATA     = files/xnec2c.png

mimepackagesdir        = $(datarootdir)/mime/packages/
dist_mimepackages_DATA = files/x-nec2.xml

pixmapsdir         = $(datarootdir)/pixmaps
dist_pixmaps_DATA  = resources/xnec2c.svg

install-data-hook:
	printf "\n\nRun 'make desktop-install' for icons and .NEC file associations.\n\n\n"

desktop-install:
	  update-mime-database $(datarootdir)/mime || :
	  update-desktop-database || :
	  gtk-update-icon-cache $(datarootdir)/icons/hicolor || :


########################################################################
# Documentation
########################################################################

EXTRA_DIST    += doc/xnec2c.1.in
CLEANFILES    += doc/xnec2c.1
man1_MANS     += doc/xnec2c.1

SUFFIXES += .1.in .1
.1.in.1:
	@$(MKDIR_P) $(@D)
	$(AM_V_GEN)$(SED) $(SED_CMDS) < $< > $@

dist_doc_DATA  =
dist_doc_DATA += doc/NEC2-bug.txt
dist_doc_DATA += doc/NearFieldCalcs.txt
dist_doc_DATA += doc/nec2c.txt
dist_doc_DATA += doc/TRANSLATING.md
dist_doc_DATA += doc/xnec2c.html

docimagesdir = $(docdir)/images
dist_docimages_DATA  =
dist_docimages_DATA += doc/images/currents.png
dist_docimages_DATA += doc/images/excitation.png
dist_docimages_DATA += doc/images/freq_plots.png
dist_docimages_DATA += doc/images/nec2_edit.png
dist_docimages_DATA += doc/images/radiation.png
dist_docimages_DATA += doc/images/web-heading.png
dist_docimages_DATA += doc/images/web-heading.xcf
dist_docimages_DATA += doc/images/wire.png

doccssdir = $(docdir)/css
dist_doccss_DATA  =
dist_doccss_DATA += doc/css/gallery.css
dist_doccss_DATA += doc/css/gallery-controls.css
dist_doccss_DATA += doc/css/gallery-nec.css
dist_doccss_DATA += doc/css/gallery-zoom.css
dist_doccss_DATA += doc/css/mobile-styles.css

docjsdir = $(docdir)/js
dist_docjs_DATA  =
dist_docjs_DATA += doc/js/gallery.js
dist_docjs_DATA += doc/js/gallery-events.js
dist_docjs_DATA += doc/js/gallery-launch.js
dist_docjs_DATA += doc/js/gallery-nec.js
dist_docjs_DATA += doc/js/gallery-render.js
dist_docjs_DATA += doc/js/gallery-view.js
dist_docjs_DATA += doc/js/gallery-warm.js
dist_docjs_DATA += doc/js/gallery-warm-queue.js
dist_docjs_DATA += doc/js/mobile-menu.js


########################################################################
# dist and install examples/ files
########################################################################

nobase_dist_pkgdata_DATA  =

nobase_dist_pkgdata_DATA += examples/conductivity.txt
nobase_dist_pkgdata_DATA += examples/ground.txt

nobase_dist_pkgdata_DATA += examples/10-20m-moxon.nec

nobase_dist_pkgdata_DATA += examples/10-30m-box.nec

nobase_dist_pkgdata_DATA += examples/10-30m_MultiBand_Vertical.nec

nobase_dist_pkgdata_DATA += examples/10-30m_bipyramid.nec

nobase_dist_pkgdata_DATA += examples/10-30m_inv_cone.nec

nobase_dist_pkgdata_DATA += examples/10-30m_sphere.nec

nobase_dist_pkgdata_DATA += examples/10-40m_windom.nec

nobase_dist_pkgdata_DATA += examples/10-80m_Classic_Windom-optimized.nec

nobase_dist_pkgdata_DATA += examples/10-80m_G5RV.nec

nobase_dist_pkgdata_DATA += examples/10-80m_Inverted-L.nec

nobase_dist_pkgdata_DATA += examples/10-80m_windom.nec

nobase_dist_pkgdata_DATA += examples/137MHz_broadside_Yagi.nec

nobase_dist_pkgdata_DATA += examples/137MHz_turnstile.nec

nobase_dist_pkgdata_DATA += examples/137MHz_turnstile_sloped.nec

nobase_dist_pkgdata_DATA += examples/137Mhz-QFHA1.nec

nobase_dist_pkgdata_DATA += examples/137Mhz-QFHA2.nec

nobase_dist_pkgdata_DATA += examples/137Mhz-QFHA3.nec

nobase_dist_pkgdata_DATA += examples/137Mhz_xpol_omni.nec

nobase_dist_pkgdata_DATA += examples/13cm_Yagi.nec

nobase_dist_pkgdata_DATA += examples/13cm_corner_reflector.nec

nobase_dist_pkgdata_DATA += examples/13cm_helix+screen.nec

nobase_dist_pkgdata_DATA += examples/15m_delta-loop.nec

nobase_dist_pkgdata_DATA += examples/1MHz_3x_helicone.nec

nobase_dist_pkgdata_DATA += examples/1MHz_3x_helisphere.nec

nobase_dist_pkgdata_DATA += examples/1MHz_4x_helisphere.nec

nobase_dist_pkgdata_DATA += examples/1MHz_helivert.nec

nobase_dist_pkgdata_DATA += examples/1MHz_tower.nec

nobase_dist_pkgdata_DATA += examples/20-40m_ground_plane.nec

nobase_dist_pkgdata_DATA += examples/20-40m_vert_circ_cliff.nec

nobase_dist_pkgdata_DATA += examples/20-40m_vert_linear_cliff.nec

nobase_dist_pkgdata_DATA += examples/20-40m_vert_sommerfeld_cliff.nec

nobase_dist_pkgdata_DATA += examples/20m_car_ant.nec

nobase_dist_pkgdata_DATA += examples/20m_quad.nec

nobase_dist_pkgdata_DATA += examples/23cm_helix+radials.nec

nobase_dist_pkgdata_DATA += examples/23cm_helix+screen.nec

nobase_dist_pkgdata_DATA += examples/2m_1to4l-gp_on_pole.nec

nobase_dist_pkgdata_DATA += examples/2m_1to4l-horiz_gp_on_pole.nec

nobase_dist_pkgdata_DATA += examples/2m_5to8l-gp_on_pole.nec

nobase_dist_pkgdata_DATA += examples/2m_EME_ant.nec

nobase_dist_pkgdata_DATA += examples/2m_Lindenblad.nec

nobase_dist_pkgdata_DATA += examples/2m_bigwheel.nec

nobase_dist_pkgdata_DATA += examples/2m_extended_Xpol_yagi-2-optimized.nec

nobase_dist_pkgdata_DATA += examples/2m_extended_Xpol_yagi-2.nec

nobase_dist_pkgdata_DATA += examples/2m_extended_Xpol_yagi.nec

nobase_dist_pkgdata_DATA += examples/2m_extended_yagi-optimized.nec

nobase_dist_pkgdata_DATA += examples/20m_dipole_NT_50ohm.nec

nobase_dist_pkgdata_DATA += examples/2m-5el-rhcp-ARISS-KJ7NLL.nec

nobase_dist_pkgdata_DATA += examples/2m_extended_yagi.nec

nobase_dist_pkgdata_DATA += examples/2m_halo_stack.nec

nobase_dist_pkgdata_DATA += examples/2m_sqr_halo.nec

nobase_dist_pkgdata_DATA += examples/2m_sqr_halo_stack.nec

nobase_dist_pkgdata_DATA += examples/2m_xpol_omni.nec

nobase_dist_pkgdata_DATA += examples/2m_xpol_omni_stack.nec

nobase_dist_pkgdata_DATA += examples/2m_yagi.nec

nobase_dist_pkgdata_DATA += examples/2m_yagi_SY_parametric.nec

nobase_dist_pkgdata_DATA += examples/2m_yagi_stack.nec

nobase_dist_pkgdata_DATA += examples/30-80m_inv_L.nec

nobase_dist_pkgdata_DATA += examples/35-55MHz_logper.nec

nobase_dist_pkgdata_DATA += examples/40-80m_Inv_L.nec

nobase_dist_pkgdata_DATA += examples/40m-moxon.nec

nobase_dist_pkgdata_DATA += examples/5el_yagi_SY_parametric.nec
nobase_dist_pkgdata_DATA += examples/5el_yagi_SY_parametric.opt

nobase_dist_pkgdata_DATA += examples/6-17m_bipyramid.nec

nobase_dist_pkgdata_DATA += examples/6-20m_fan.nec

nobase_dist_pkgdata_DATA += examples/6-20m_inv_cone.nec

nobase_dist_pkgdata_DATA += examples/6-40m_5B4AZ-optimized.nec

nobase_dist_pkgdata_DATA += examples/6-40m_Classic_Windom-optimized.nec

nobase_dist_pkgdata_DATA += examples/6m_big-square_stack.nec

nobase_dist_pkgdata_DATA += examples/6m_bigwheel-stack.nec

nobase_dist_pkgdata_DATA += examples/6m_horizomni.nec

nobase_dist_pkgdata_DATA += examples/70cm-5el-rhcp-KJ7NLL.nec

nobase_dist_pkgdata_DATA += examples/70cm_collinear.nec

nobase_dist_pkgdata_DATA += examples/80m_zepp.nec

nobase_dist_pkgdata_DATA += examples/T12m-H24m.nec

nobase_dist_pkgdata_DATA += examples/T20m-H18m.nec

nobase_dist_pkgdata_DATA += examples/airplane.nec

nobase_dist_pkgdata_DATA += examples/buoy.nec

nobase_dist_pkgdata_DATA += examples/ex2_current_slope_disc_dipole.nec

nobase_dist_pkgdata_DATA += examples/ex4_current_source_sq_loop.nec

nobase_dist_pkgdata_DATA += examples/gray_hoverman.nec

nobase_dist_pkgdata_DATA += examples/k9ay_5b4az.nec

nobase_dist_pkgdata_DATA += examples/k9ay_orig.nec

nobase_dist_pkgdata_DATA += examples/satellite.nec

# Ensure that the above nobase_dist_pkgdata_DATA list is consistent
# with the files existing in the $(srcdir)/examples directory tree.
# Root-level examples/*.csv are program-generated outputs (see .gitignore
# rule /examples/*.csv) and are excluded, as are *.sy/*.SY source cards;
# committed reference data under examples/data/*.csv stays in the list.
CLEANFILES += examples-automake.lst
CLEANFILES += examples-srcdir.lst
# PO catalog maintenance gate
check-local: check-potfiles-list check-po-exemptions
check-potfiles-list:
	@$(srcdir)/scripts/po/po-update.sh --check
# Check the catalogs after the refresh sweep so the gate reads final tree
# state rather than a pre-strip transient. The complete gate runs here: an
# authoritative catalog answers every entry with a translation or with an
# exemption comment recording why it holds none, so an entry carrying neither
# fails rather than passing as work in progress.
check-po-catalogs: po-refresh
	@lc=$$(grep -v '^[[:space:]]*#' $(srcdir)/po/LINGUAS | grep -v '^[[:space:]]*$$'); \
	for lang in $$lc; do \
	  $(srcdir)/scripts/po/po-check.sh $(srcdir)/po/$$lang.po || exit 1; \
	done
.PHONY: check-po-catalogs
# Compare source-based exemptions after the per-catalog sweep so both gates
# read the same refreshed catalogs. Run from the source tree because the audit
# derives every catalog path from po/.
check-po-exemptions: check-po-catalogs
	@cd "$(srcdir)" && scripts/po/po-exempt-check.pl
.PHONY: check-po-exemptions
# Regenerate the template alone, then discard the creation-date churn xgettext
# writes when no message changed.
po-template:
	@$(MAKE) -C po $(PACKAGE).pot-update
	@$(srcdir)/scripts/po/po-revert-churn.sh $(srcdir)/po/$(PACKAGE).pot
.PHONY: po-template
# Refresh every catalog through the single merge-and-strip point so no
# generation path leaves an obsolete entry or a backup; depends on po-template
# so the template is regenerated before the per-catalog sweep.
po-refresh: po-template
	@lc=$$(grep -v '^[[:space:]]*#' $(srcdir)/po/LINGUAS | grep -v '^[[:space:]]*$$'); \
	for lang in $$lc; do \
	  $(srcdir)/scripts/po/po-refresh.sh $(srcdir)/po/$$lang.po $(srcdir)/po/$(PACKAGE).pot || exit 1; \
	done
.PHONY: po-refresh
# Authoritative gallery deck membership: every NEC deck under examples/.
gallery_decks = $(patsubst $(srcdir)/%,%,$(wildcard $(srcdir)/examples/*.nec))
render-example-gallery:
	@cd $(srcdir) && $(PERL) scripts/gallery/update-example-gallery.pl render $(gallery_decks)
refresh-example-gallery:
	@cd $(srcdir) && $(PERL) scripts/gallery/update-example-gallery.pl refresh $(gallery_decks)
.PHONY: render-example-gallery refresh-example-gallery
# Rebuild the gallery data from the decks, then report whether the published
# website has drifted from the local examples tree and needs a resync.
gallery: refresh-example-gallery
	@cd $(srcdir) && $(PERL) scripts/gallery/check-gallery-website.pl $(gallery_decks)
.PHONY: gallery
# Prove deck registration: the examples/ tree matches the automake dist list.
check-deck:
	@for f in $(nobase_dist_pkgdata_DATA); do echo "$$f"; done | sort > examples-automake.lst
	@(cd $(srcdir) && find examples -type f -not -name '*.sy' -not -name '*.SY' -not \( -path 'examples/*.csv' ! -path 'examples/*/*.csv' \)) | sort > examples-srcdir.lst
	@if cmp examples-automake.lst examples-srcdir.lst > /dev/null; then :; else \
	  echo "Error: Makefile.am nobase_dist_pkgdata_DATA and examples/ dir are inconsistent:"; \
	  diff examples-automake.lst examples-srcdir.lst | grep -v '^[0-9]'; \
	  echo "Explanation:"; \
	  echo "   < file listed in Makefile.am more than once"; \
	  echo "   > file exists in examples/ but is not listed in Makefile.am"; \
	  echo "Files listed in Makefile.am but not existing in examples/ would have"; \
	  echo "produced an error earlier about there being no rule to make that file."; \
	  rm -f examples-automake.lst examples-srcdir.lst; \
	  exit 1; \
	fi
	@rm -f examples-automake.lst examples-srcdir.lst
.PHONY: check-deck
# Deck registration gate
check-local: check-deck


########################################################################
# Local convenience RPM builds
########################################################################

EXTRA_DIST  += xnec2c.spec.in
CLEANFILES  += xnec2c.spec
noinst_DATA += xnec2c.spec

SUFFIXES += .spec.in .spec
.spec.in.spec:
	@$(MKDIR_P) $(@D)
	$(AM_V_GEN)$(SED) $(SED_CMDS) < $< > $@

########################################################################
# Create locale tree for running from source tree
########################################################################

all-local: locale-tree

locale-tree:
	@$(MKDIR_P) locale
	@for gmo in po/*.gmo; do \
		test -f "$$gmo" || continue; \
		lang=$$(basename "$$gmo" .gmo); \
		$(MKDIR_P) locale/$$lang/LC_MESSAGES; \
		cp "$$gmo" locale/$$lang/LC_MESSAGES/$(PACKAGE).mo; \
	done

CLEANFILES += -r locale

# Force po revalidation after clean by removing stamp-po and .gmo files
# that the stock gettext Makefile.in.in only removes in maintainer-clean
clean-local:
	rm -f po/stamp-po po/*.gmo

.PHONY: locale-tree
