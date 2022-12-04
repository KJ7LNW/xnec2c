## Process this file with automake to produce Makefile.in

CLEANFILES =
EXTRA_DIST =
SUFFIXES =
man1_MANS =
noinst_DATA =

SUBDIRS = src po

EXTRA_DIST += autogen.sh

EXTRA_DIST += .gitignore
EXTRA_DIST += m4/.gitignore

EXTRA_DIST += debian/upstream/metadata
EXTRA_DIST += update-example-data.sh

EXTRA_DIST += MAINTAINER
EXTRA_DIST += README.md
EXTRA_DIST += PACKAGING

EXTRA_DIST += m4-include/ax_check_compile_flag.m4
EXTRA_DIST += m4-include/pkg.m4

EXTRA_DIST += resources/link.svg
EXTRA_DIST += resources/unlink.svg
EXTRA_DIST += resources/xnec2c.glade
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
dist_applications_DATA = files/xnec2c.desktop

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
	$(SED) $(SED_CMDS) < $< > $@

dist_doc_DATA  =
dist_doc_DATA += doc/NEC2-bug.txt
dist_doc_DATA += doc/NearFieldCalcs.txt
dist_doc_DATA += doc/nec2c.txt
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


########################################################################
# dist and install examples/ files
########################################################################

nobase_dist_pkgdata_DATA  =

nobase_dist_pkgdata_DATA += examples/conductivity.txt
nobase_dist_pkgdata_DATA += examples/ground.txt

nobase_dist_pkgdata_DATA += examples/data/README

nobase_dist_pkgdata_DATA += examples/10-20m-moxon.nec
nobase_dist_pkgdata_DATA += examples/data/10-20m-moxon.csv

nobase_dist_pkgdata_DATA += examples/10-30m-box.nec
nobase_dist_pkgdata_DATA += examples/data/10-30m-box.csv

nobase_dist_pkgdata_DATA += examples/10-30m_MultiBand_Vertical.nec
nobase_dist_pkgdata_DATA += examples/data/10-30m_MultiBand_Vertical.csv

nobase_dist_pkgdata_DATA += examples/10-30m_bipyramid.nec
nobase_dist_pkgdata_DATA += examples/data/10-30m_bipyramid.csv

nobase_dist_pkgdata_DATA += examples/10-30m_inv_cone.nec
nobase_dist_pkgdata_DATA += examples/data/10-30m_inv_cone.csv

nobase_dist_pkgdata_DATA += examples/10-30m_sphere.nec
nobase_dist_pkgdata_DATA += examples/data/10-30m_sphere.csv

nobase_dist_pkgdata_DATA += examples/10-40m_windom.nec
nobase_dist_pkgdata_DATA += examples/data/10-40m_windom.csv

nobase_dist_pkgdata_DATA += examples/10-80m_Classic_Windom-optimized.nec
nobase_dist_pkgdata_DATA += examples/data/10-80m_Classic_Windom-optimized.csv

nobase_dist_pkgdata_DATA += examples/10-80m_G5RV.nec
nobase_dist_pkgdata_DATA += examples/data/10-80m_G5RV.csv

nobase_dist_pkgdata_DATA += examples/10-80m_Inverted-L.nec
nobase_dist_pkgdata_DATA += examples/data/10-80m_Inverted-L.csv

nobase_dist_pkgdata_DATA += examples/10-80m_windom.nec
nobase_dist_pkgdata_DATA += examples/data/10-80m_windom.csv

nobase_dist_pkgdata_DATA += examples/137MHz_broadside_Yagi.nec
nobase_dist_pkgdata_DATA += examples/data/137MHz_broadside_Yagi.csv

nobase_dist_pkgdata_DATA += examples/137MHz_turnstile.nec
nobase_dist_pkgdata_DATA += examples/data/137MHz_turnstile.csv

nobase_dist_pkgdata_DATA += examples/137MHz_turnstile_sloped.nec
nobase_dist_pkgdata_DATA += examples/data/137MHz_turnstile_sloped.csv

nobase_dist_pkgdata_DATA += examples/137Mhz-QFHA1.nec
nobase_dist_pkgdata_DATA += examples/data/137Mhz-QFHA1.csv

nobase_dist_pkgdata_DATA += examples/137Mhz-QFHA2.nec
nobase_dist_pkgdata_DATA += examples/data/137Mhz-QFHA2.csv

nobase_dist_pkgdata_DATA += examples/137Mhz-QFHA3.nec
nobase_dist_pkgdata_DATA += examples/data/137Mhz-QFHA3.csv

nobase_dist_pkgdata_DATA += examples/137Mhz_xpol_omni.nec
nobase_dist_pkgdata_DATA += examples/data/137Mhz_xpol_omni.csv

nobase_dist_pkgdata_DATA += examples/13cm_Yagi.nec
nobase_dist_pkgdata_DATA += examples/data/13cm_Yagi.csv

nobase_dist_pkgdata_DATA += examples/13cm_corner_reflector.nec
nobase_dist_pkgdata_DATA += examples/data/13cm_corner_reflector.csv

nobase_dist_pkgdata_DATA += examples/13cm_helix+screen.nec
nobase_dist_pkgdata_DATA += examples/data/13cm_helix+screen.csv

nobase_dist_pkgdata_DATA += examples/15m_delta-loop.nec
nobase_dist_pkgdata_DATA += examples/data/15m_delta-loop.csv

nobase_dist_pkgdata_DATA += examples/1MHz_3x_helicone.nec
nobase_dist_pkgdata_DATA += examples/data/1MHz_3x_helicone.csv

nobase_dist_pkgdata_DATA += examples/1MHz_3x_helisphere.nec
nobase_dist_pkgdata_DATA += examples/data/1MHz_3x_helisphere.csv

nobase_dist_pkgdata_DATA += examples/1MHz_4x_helisphere.nec
nobase_dist_pkgdata_DATA += examples/data/1MHz_4x_helisphere.csv

nobase_dist_pkgdata_DATA += examples/1MHz_helivert.nec
nobase_dist_pkgdata_DATA += examples/data/1MHz_helivert.csv

nobase_dist_pkgdata_DATA += examples/1MHz_tower.nec
nobase_dist_pkgdata_DATA += examples/data/1MHz_tower.csv

nobase_dist_pkgdata_DATA += examples/20-40m_ground_plane.nec
nobase_dist_pkgdata_DATA += examples/data/20-40m_ground_plane.csv

nobase_dist_pkgdata_DATA += examples/20-40m_vert_circ_cliff.nec
nobase_dist_pkgdata_DATA += examples/data/20-40m_vert_circ_cliff.csv

nobase_dist_pkgdata_DATA += examples/20-40m_vert_linear_cliff.nec
nobase_dist_pkgdata_DATA += examples/data/20-40m_vert_linear_cliff.csv

nobase_dist_pkgdata_DATA += examples/20-40m_vert_sommerfeld_cliff.nec
nobase_dist_pkgdata_DATA += examples/data/20-40m_vert_sommerfeld_cliff.csv

nobase_dist_pkgdata_DATA += examples/20m_car_ant.nec
nobase_dist_pkgdata_DATA += examples/data/20m_car_ant.csv

nobase_dist_pkgdata_DATA += examples/20m_quad.nec
nobase_dist_pkgdata_DATA += examples/data/20m_quad.csv

nobase_dist_pkgdata_DATA += examples/23cm_helix+radials.nec
nobase_dist_pkgdata_DATA += examples/data/23cm_helix+radials.csv

nobase_dist_pkgdata_DATA += examples/23cm_helix+screen.nec
nobase_dist_pkgdata_DATA += examples/data/23cm_helix+screen.csv

nobase_dist_pkgdata_DATA += examples/2m_1to4l-gp_on_pole.nec
nobase_dist_pkgdata_DATA += examples/data/2m_1to4l-gp_on_pole.csv

nobase_dist_pkgdata_DATA += examples/2m_1to4l-horiz_gp_on_pole.nec
nobase_dist_pkgdata_DATA += examples/data/2m_1to4l-horiz_gp_on_pole.csv

nobase_dist_pkgdata_DATA += examples/2m_5to8l-gp_on_pole.nec
nobase_dist_pkgdata_DATA += examples/data/2m_5to8l-gp_on_pole.csv

nobase_dist_pkgdata_DATA += examples/2m_EME_ant.nec
nobase_dist_pkgdata_DATA += examples/data/2m_EME_ant.csv

nobase_dist_pkgdata_DATA += examples/2m_Lindenblad.nec
nobase_dist_pkgdata_DATA += examples/data/2m_Lindenblad.csv

nobase_dist_pkgdata_DATA += examples/2m_bigwheel.nec
nobase_dist_pkgdata_DATA += examples/data/2m_bigwheel.csv

nobase_dist_pkgdata_DATA += examples/2m_extended_Xpol_yagi-2-optimized.nec
nobase_dist_pkgdata_DATA += examples/data/2m_extended_Xpol_yagi-2-optimized.csv

nobase_dist_pkgdata_DATA += examples/2m_extended_Xpol_yagi-2.nec
nobase_dist_pkgdata_DATA += examples/data/2m_extended_Xpol_yagi-2.csv

nobase_dist_pkgdata_DATA += examples/2m_extended_Xpol_yagi.nec
nobase_dist_pkgdata_DATA += examples/data/2m_extended_Xpol_yagi.csv

nobase_dist_pkgdata_DATA += examples/2m_extended_yagi-optimized.nec
nobase_dist_pkgdata_DATA += examples/data/2m_extended_yagi-optimized.csv

nobase_dist_pkgdata_DATA += examples/2m_extended_yagi.nec
nobase_dist_pkgdata_DATA += examples/data/2m_extended_yagi.csv

nobase_dist_pkgdata_DATA += examples/2m_halo_stack.nec
nobase_dist_pkgdata_DATA += examples/data/2m_halo_stack.csv

nobase_dist_pkgdata_DATA += examples/2m_sqr_halo.nec
nobase_dist_pkgdata_DATA += examples/data/2m_sqr_halo.csv

nobase_dist_pkgdata_DATA += examples/2m_sqr_halo_stack.nec
nobase_dist_pkgdata_DATA += examples/data/2m_sqr_halo_stack.csv

nobase_dist_pkgdata_DATA += examples/2m_xpol_omni.nec
nobase_dist_pkgdata_DATA += examples/data/2m_xpol_omni.csv

nobase_dist_pkgdata_DATA += examples/2m_xpol_omni_stack.nec
nobase_dist_pkgdata_DATA += examples/data/2m_xpol_omni_stack.csv

nobase_dist_pkgdata_DATA += examples/2m_yagi.nec
nobase_dist_pkgdata_DATA += examples/data/2m_yagi.csv

nobase_dist_pkgdata_DATA += examples/2m_yagi_stack.nec
nobase_dist_pkgdata_DATA += examples/data/2m_yagi_stack.csv

nobase_dist_pkgdata_DATA += examples/30-80m_inv_L.nec
nobase_dist_pkgdata_DATA += examples/data/30-80m_inv_L.csv

nobase_dist_pkgdata_DATA += examples/35-55MHz_logper.nec
nobase_dist_pkgdata_DATA += examples/data/35-55MHz_logper.csv

nobase_dist_pkgdata_DATA += examples/40-80m_Inv_L.nec
nobase_dist_pkgdata_DATA += examples/data/40-80m_Inv_L.csv

nobase_dist_pkgdata_DATA += examples/40m-moxon.nec
nobase_dist_pkgdata_DATA += examples/data/40m-moxon.csv

nobase_dist_pkgdata_DATA += examples/6-17m_bipyramid.nec
nobase_dist_pkgdata_DATA += examples/data/6-17m_bipyramid.csv

nobase_dist_pkgdata_DATA += examples/6-20m_fan.nec
nobase_dist_pkgdata_DATA += examples/data/6-20m_fan.csv

nobase_dist_pkgdata_DATA += examples/6-20m_inv_cone.nec
nobase_dist_pkgdata_DATA += examples/data/6-20m_inv_cone.csv

nobase_dist_pkgdata_DATA += examples/6-40m_5B4AZ-optimized.nec
nobase_dist_pkgdata_DATA += examples/data/6-40m_5B4AZ-optimized.csv

nobase_dist_pkgdata_DATA += examples/6-40m_Classic_Windom-optimized.nec
nobase_dist_pkgdata_DATA += examples/data/6-40m_Classic_Windom-optimized.csv

nobase_dist_pkgdata_DATA += examples/6m_big-square_stack.nec
nobase_dist_pkgdata_DATA += examples/data/6m_big-square_stack.csv

nobase_dist_pkgdata_DATA += examples/6m_bigwheel-stack.nec
nobase_dist_pkgdata_DATA += examples/data/6m_bigwheel-stack.csv

nobase_dist_pkgdata_DATA += examples/6m_horizomni.nec
nobase_dist_pkgdata_DATA += examples/data/6m_horizomni.csv

nobase_dist_pkgdata_DATA += examples/70cm_collinear.nec
nobase_dist_pkgdata_DATA += examples/data/70cm_collinear.csv

nobase_dist_pkgdata_DATA += examples/80m_zepp.nec
nobase_dist_pkgdata_DATA += examples/data/80m_zepp.csv

nobase_dist_pkgdata_DATA += examples/T12m-H24m.nec
nobase_dist_pkgdata_DATA += examples/data/T12m-H24m.csv

nobase_dist_pkgdata_DATA += examples/T20m-H18m.nec
nobase_dist_pkgdata_DATA += examples/data/T20m-H18m.csv

nobase_dist_pkgdata_DATA += examples/airplane.nec
nobase_dist_pkgdata_DATA += examples/data/airplane.csv

nobase_dist_pkgdata_DATA += examples/buoy.nec
nobase_dist_pkgdata_DATA += examples/data/buoy.csv

nobase_dist_pkgdata_DATA += examples/gray_hoverman.nec
nobase_dist_pkgdata_DATA += examples/data/gray_hoverman.csv

nobase_dist_pkgdata_DATA += examples/k9ay_5b4az.nec
nobase_dist_pkgdata_DATA += examples/data/k9ay_5b4az.csv

nobase_dist_pkgdata_DATA += examples/k9ay_orig.nec
nobase_dist_pkgdata_DATA += examples/data/k9ay_orig.csv

nobase_dist_pkgdata_DATA += examples/satellite.nec
nobase_dist_pkgdata_DATA += examples/data/satellite.csv

# Ensure that the above nobase_dist_pkgdata_DATA list is consistent
# with the files existing in the $(srcdir)/examples directory tree.
CLEANFILES += examples-automake.lst
CLEANFILES += examples-srcdir.lst
check-local: check-examples-file-list
check-examples-file-list:
	@for f in $(nobase_dist_pkgdata_DATA); do echo "$$f"; done | sort > examples-automake.lst
	@(cd $(srcdir) && find examples -type f) | sort > examples-srcdir.lst
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


########################################################################
# Local convenience RPM builds
########################################################################

EXTRA_DIST  += xnec2c.spec.in
CLEANFILES  += xnec2c.spec
noinst_DATA += xnec2c.spec

SUFFIXES += .spec.in .spec
.spec.in.spec:
	@$(MKDIR_P) $(@D)
	$(SED) $(SED_CMDS) < $< > $@
