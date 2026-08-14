/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

#include <ctype.h>
#include <sys/stat.h>
#include <cairo.h>
#include "shared.h"
#include "rc_config.h"
#include "mathlib.h"
#include "measurements.h"
#include "config_hooks.h"
#include "chroma/chroma.h"

#include "opengl/opengl_structure.h"
#include "opengl/opengl_msaa.h"
#include "settings/render_settings.h"


/* Add configuration options here. To add new variables:
 *    Add a new section and description
 *    Set the format string
 *    Create references to the .vars structure to load/save values
 */

/* OpenGL-tab widgets live in opengl_settings.glade, loaded only under
 * HAVE_OPENGL.  A Cairo build still persists the underlying fields but binds
 * no widget: GL_CONFIG_WIDGET_TREE/SINGLE resolve to the widget tree under
 * HAVE_OPENGL and to NULL otherwise, so every row keeps a literal .widgets
 * assignment and rc_config_register_widgets skips the NULL binding.
 * GL_CONFIG_WIDGET_GROUP emits one render-settings group inside a multi-group
 * tree, or nothing, so it needs no .widgets prefix. */
#ifdef HAVE_OPENGL
#define GL_CONFIG_WIDGET_TREE(...)   CONFIG_WIDGET_TREE(__VA_ARGS__)
#define GL_CONFIG_WIDGET_SINGLE(...) CONFIG_WIDGET_SINGLE(__VA_ARGS__)
#define GL_CONFIG_WIDGET_GROUP(...)  CONFIG_WIDGET_GROUP(__VA_ARGS__),
#else
#define GL_CONFIG_WIDGET_TREE(...)   NULL
#define GL_CONFIG_WIDGET_SINGLE(...) NULL
#define GL_CONFIG_WIDGET_GROUP(...)
#endif

/* polarization stores an int; the "%d"/sizeof(int) row accessor requires it */
CONFIG_FIELD_INT_ASSERT(calc_data.pol_type);

static void working_dir_set_default(rc_config_vars_t *v);

rc_config_vars_t rc_config_vars[] = {
	// Historical typo "Vesrsion", but it cannot changed for backwards
	// compatibility because we parse the comment:
	{ .desc = "Application Vesrsion", .format = "%s", .ro = 1, 
		.vars = { PACKAGE_STRING } },

	{ .desc = "Current Working Directory", .format = "%s",
		.vars = { rc_config.working_dir }, .size = FILENAME_LEN,
		.set_default = working_dir_set_default },

	{ .desc = "Main Window Size, in pixels", .format = "%d,%d",
		.vars = { &rc_config.main_width, &rc_config.main_height },
		.def = { { .i = 600 }, { .i = 400 } } },

	{ .desc = "Main Window Position (root x and y)", .format = "%d,%d",
		.vars = { &rc_config.main_x, &rc_config.main_y },
		.def = { { .i = 50 }, { .i = 50 } } },

	// Reuses the historical "currents" key as a struct_view_t enum holder:
	// value 1 keeps its old CURRENTS meaning, 0 becomes CHARGES, and the
	// DISABLED (-1) geometry view is a value no old file ever wrote, reached
	// by releasing the pressed toggle.  The two-value toggles carry the
	// released DISABLED state, so clearing the active view shows geometry and
	// the fresh default is geometry; the engine sync enforces exclusivity.
	// The dropped "charges" key is ignored.
	{ .desc = "Main Window Currents toggle button state", .format = "%d",
		.vars = { &rc_config.structure_view }, .def = { { .i = STRUCT_VIEW_DISABLED } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = structure_view_apply,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &main_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "main_currents_togglebutton",
							.values = CONFIG_WIDGET_VALUES(STRUCT_VIEW_CURRENTS, STRUCT_VIEW_DISABLED) ),
						CONFIG_WIDGET( .widget_id = "main_charges_togglebutton",
							.values = CONFIG_WIDGET_VALUES(STRUCT_VIEW_CHARGES, STRUCT_VIEW_DISABLED) ),
						NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &animate_dialog_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "anim_qty_off",
							.values = CONFIG_WIDGET_VALUES(STRUCT_VIEW_DISABLED) ),
						CONFIG_WIDGET( .widget_id = "anim_qty_charges",
							.values = CONFIG_WIDGET_VALUES(STRUCT_VIEW_CHARGES) ),
						CONFIG_WIDGET( .widget_id = "anim_qty_currents",
							.values = CONFIG_WIDGET_VALUES(STRUCT_VIEW_CURRENTS) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "Polarization Type", .format = "%d",
		.vars = { &calc_data.pol_type }, .def = { { .i = POL_TOTAL } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_polarization,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &main_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "main_total",
							.values = CONFIG_WIDGET_VALUES(POL_TOTAL) ),
						CONFIG_WIDGET( .widget_id = "main_horizontal",
							.values = CONFIG_WIDGET_VALUES(POL_HORIZ) ),
						CONFIG_WIDGET( .widget_id = "main_vertical",
							.values = CONFIG_WIDGET_VALUES(POL_VERT) ),
						CONFIG_WIDGET( .widget_id = "main_right_hand",
							.values = CONFIG_WIDGET_VALUES(POL_RHCP) ),
						CONFIG_WIDGET( .widget_id = "main_left_hand",
							.values = CONFIG_WIDGET_VALUES(POL_LHCP) ),
						NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "rdpattern_total",
							.values = CONFIG_WIDGET_VALUES(POL_TOTAL) ),
						CONFIG_WIDGET( .widget_id = "rdpattern_horizontal",
							.values = CONFIG_WIDGET_VALUES(POL_HORIZ) ),
						CONFIG_WIDGET( .widget_id = "rdpattern_vertical",
							.values = CONFIG_WIDGET_VALUES(POL_VERT) ),
						CONFIG_WIDGET( .widget_id = "rdpattern_right_hand",
							.values = CONFIG_WIDGET_VALUES(POL_RHCP) ),
						CONFIG_WIDGET( .widget_id = "rdpattern_left_hand",
							.values = CONFIG_WIDGET_VALUES(POL_LHCP) ),
						NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &freqplots_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "freqplots_total",
							.values = CONFIG_WIDGET_VALUES(POL_TOTAL) ),
						CONFIG_WIDGET( .widget_id = "freqplots_horizontal",
							.values = CONFIG_WIDGET_VALUES(POL_HORIZ) ),
						CONFIG_WIDGET( .widget_id = "freqplots_vertical",
							.values = CONFIG_WIDGET_VALUES(POL_VERT) ),
						CONFIG_WIDGET( .widget_id = "freqplots_right_hand",
							.values = CONFIG_WIDGET_VALUES(POL_RHCP) ),
						CONFIG_WIDGET( .widget_id = "freqplots_left_hand",
							.values = CONFIG_WIDGET_VALUES(POL_LHCP) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "Main Window Frequency loop start state", .format = "%d",
		.vars = { &rc_config.main_loop_start }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_SINGLE( &main_window_builder,
			"config_main_loop_start", NULL ) },

	{ .desc = "Main Window Rotate spinbutton state", .format = "%d",
		.vars = { &rc_config.main_rotate_spinbutton }, .def = { { .i = 45 } } },

	{ .desc = "Main Window Incline spinbutton state", .format = "%d",
		.vars = { &rc_config.main_incline_spinbutton }, .def = { { .i = 45 } } },

	{ .desc = "Main Window Zoom spinbutton state", .format = "%d",
		.vars = { &rc_config.main_zoom_spinbutton }, .def = { { .i = 100 } } },

	{ .desc = "Radiation Pattern Window Size, in pixels", .format = "%d,%d",
		.vars = { &rc_config.rdpattern_width, &rc_config.rdpattern_height } },

	{ .desc = "Radiation Pattern Window Position (root x and y)", .format = "%d,%d",
		.vars = { &rc_config.rdpattern_x, &rc_config.rdpattern_y },
		.def = { { .i = -1 }, { .i = -1 } } },

	// Reuses the historical "gain" key as a rdpat_mode_t enum holder: value 1
	// keeps its old GAIN meaning and 0 becomes EHFIELD.  The two-value toggles
	// carry the released DISABLED state, so clearing the active field shows no
	// pattern; the engine sync enforces exclusivity.  The dropped "eh" key is
	// ignored.
	{ .desc = "Radiation Pattern Window Gain toggle button state", .format = "%d",
		.vars = { &rc_config.rdpattern_mode }, .def = { { .i = RDPAT_FIELD_GAIN } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = rdpattern_mode_apply,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "rdpattern_gain_togglebutton",
							.values = CONFIG_WIDGET_VALUES(RDPAT_FIELD_GAIN, RDPAT_FIELD_DISABLED) ),
						CONFIG_WIDGET( .widget_id = "rdpattern_eh_togglebutton",
							.values = CONFIG_WIDGET_VALUES(RDPAT_FIELD_EHFIELD, RDPAT_FIELD_DISABLED) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "Radiation Pattern Window Menu E-field state", .format = "%d",
		.vars = { &rc_config.rdpattern_e_field }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_rdpat_e_field,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "rdpattern_e_field" ), NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &animate_dialog_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "anim_efield" ), NULL ) ),
				NULL ) ) },

	{ .desc = "Radiation Pattern Window Menu H-field state", .format = "%d",
		.vars = { &rc_config.rdpattern_h_field }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_rdpat_h_field,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "rdpattern_h_field" ), NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &animate_dialog_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "anim_hfield" ), NULL ) ),
				NULL ) ) },

	{ .desc = "Radiation Pattern Window Menu Poynting vector state", .format = "%d",
		.vars = { &rc_config.rdpattern_poynting_vector },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_rdpat_poynting,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "rdpattern_poynting_vector" ), NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &animate_dialog_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "anim_poynting" ), NULL ) ),
				NULL ) ) },

	{ .desc = "Near Field Static Baseline", .format = "%d",
		.vars = { &rc_config.nf_static_mode }, .def = { { .i = NF_STATIC_PEAK } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_render_redraw,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "near_peak_value",
							.values = CONFIG_WIDGET_VALUES(NF_STATIC_PEAK) ),
						CONFIG_WIDGET( .widget_id = "near_snapshot",
							.values = CONFIG_WIDGET_VALUES(NF_STATIC_SNAPSHOT) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "Radiation Pattern Window Gradient Key", .format = "%d",
		.vars = { &rc_config.rdpattern_gradient_key }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_SINGLE( &rdpattern_window_builder,
			"rdpattern_gradient_key", hook_rdpat_gradient_key ) },

	{ .desc = "Radiation Pattern Window Zoom spinbutton state", .format = "%d",
		.vars = { &rc_config.rdpattern_zoom_spinbutton }, .def = { { .i = 100 } } },

	{ .desc = "Use OpenGL Renderer for Radiation Patterns", .format = "%d",
		.vars = { &rc_config.use_opengl_renderer },
		.def = { { .i = RENDERER_RESET_DEFAULT } },
		.widgets = CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"chk_opengl_renderer", hook_set_renderer ) },

	{ .desc = "Use Constrained View Drag Rotation", .format = "%d",
		.vars = { &rc_config.view_drag_constrained }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"chk_constrained_rotation", hook_set_constrained ) },

	{ .desc = "Main Window Common Projection", .format = "%d",
		.vars = { &rc_config.main_common_projection }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_common_projection,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &main_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "main_common_projection" ), NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "rdpattern_common_projection" ), NULL ) ),
				NULL ) ) },

	{ .desc = "Common Pan (couple structure/rdpattern arrow-key panning)", .format = "%d",
		.vars = { &rc_config.common_pan }, .def = { { .i = 0 } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_common_pan,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &main_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "main_common_pan" ), NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "rdpattern_common_pan" ), NULL ) ),
				NULL ) ) },

	{ .desc = "Radiation Pattern Window Overlay Structure", .format = "%d",
		.vars = { &rc_config.rdpattern_overlay_structure },
		.widgets = CONFIG_WIDGET_SINGLE( &rdpattern_window_builder,
			"rdpattern_overlay_structure", hook_rdpat_overlay ) },

	{ .desc = "Radiation Pattern Draw Style", .format = "%d",
		.vars = { &rc_config.rdpattern_draw_style },
		.def = { { .i = RDPAT_STYLE_BOTH } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_rdpat_draw_style,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "rdpattern_style_surface",
							.values = CONFIG_WIDGET_VALUES(RDPAT_STYLE_SURFACE) ),
						CONFIG_WIDGET( .widget_id = "rdpattern_style_wireframe",
							.values = CONFIG_WIDGET_VALUES(RDPAT_STYLE_WIREFRAME) ),
						CONFIG_WIDGET( .widget_id = "rdpattern_style_both",
							.values = CONFIG_WIDGET_VALUES(RDPAT_STYLE_BOTH) ),
						NULL ) ),
				GL_CONFIG_WIDGET_GROUP( .builder = &render_settings_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "radio_style_surface",
							.values = CONFIG_WIDGET_VALUES(RDPAT_STYLE_SURFACE) ),
						CONFIG_WIDGET( .widget_id = "radio_style_wireframe",
							.values = CONFIG_WIDGET_VALUES(RDPAT_STYLE_WIREFRAME) ),
						CONFIG_WIDGET( .widget_id = "radio_style_both",
							.values = CONFIG_WIDGET_VALUES(RDPAT_STYLE_BOTH) ),
						NULL ) )
				NULL ) ) },

	{ .desc = "OpenGL Transparent on Click", .format = "%d",
		.vars = { &rc_config.opengl_transparent_on_click }, .def = { { .i = 1 } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"chk_only_on_click", hook_render_redraw ) },

	{ .desc = "OpenGL Orthographic Projection", .format = "%d",
		.vars = { &rc_config.opengl_orthographic }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_orthographic,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &main_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "main_ortho_button" ), NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "rdpattern_ortho_button" ), NULL ) ),
				GL_CONFIG_WIDGET_GROUP( .builder = &render_settings_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "chk_orthographic" ), NULL ) )
				NULL ) ) },

	{ .desc = "OpenGL Anti-Aliasing Samples", .format = "%d",
		.vars = { &rc_config.opengl_msaa_samples }, .def = { { .i = MSAA_4X } },
		.widgets = GL_CONFIG_WIDGET_TREE( .post_apply = hook_set_msaa,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &render_settings_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "radio_msaa_off",
							.values = CONFIG_WIDGET_VALUES(MSAA_OFF) ),
						CONFIG_WIDGET( .widget_id = "radio_msaa_2x",
							.values = CONFIG_WIDGET_VALUES(MSAA_2X) ),
						CONFIG_WIDGET( .widget_id = "radio_msaa_4x",
							.values = CONFIG_WIDGET_VALUES(MSAA_4X) ),
						CONFIG_WIDGET( .widget_id = "radio_msaa_8x",
							.values = CONFIG_WIDGET_VALUES(MSAA_8X) ),
						CONFIG_WIDGET( .widget_id = "radio_msaa_16x",
							.values = CONFIG_WIDGET_VALUES(MSAA_16X) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "OpenGL Cylinder Radius Scale", .format = "%lf",
		.vars = { &rc_config.opengl_cylinder_radius_scale }, .def = { { .d = 1.0 } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_cylinder_scale", hook_set_radius_scale ) },

	{ .desc = "Rdpattern Overlay Scale Adjustment", .format = "%lf",
		.vars = { &rc_config.rdpattern_overlay_scale_adj }, .def = { { .d = 1.0 } } },

	{ .desc = "Cairo Anti-Aliasing Mode", .format = "%d",
		.vars = { &rc_config.cairo_antialias },
		.def = { { .i = CAIRO_ANTIALIAS_DEFAULT } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_render_redraw,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &render_settings_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "radio_cairo_antialias_default",
							.values = CONFIG_WIDGET_VALUES(CAIRO_ANTIALIAS_DEFAULT) ),
						CONFIG_WIDGET( .widget_id = "radio_cairo_antialias_fast",
							.values = CONFIG_WIDGET_VALUES(CAIRO_ANTIALIAS_FAST) ),
						CONFIG_WIDGET( .widget_id = "radio_cairo_antialias_none",
							.values = CONFIG_WIDGET_VALUES(CAIRO_ANTIALIAS_NONE) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "Cairo Depth Bins", .format = "%d",
		.vars = { &rc_config.cairo_depth_bins }, .def = { { .i = 16 } },
		.widgets = CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"spin_cairo_depth_bins", hook_render_redraw ) },

	{ .desc = "Cairo Color Quantization Levels", .format = "%d",
		.vars = { &rc_config.cairo_color_quant },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_render_redraw,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &render_settings_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "radio_cairo_color_quant_off",
							.values = CONFIG_WIDGET_VALUES(0) ),
						CONFIG_WIDGET( .widget_id = "radio_cairo_color_quant_8",
							.values = CONFIG_WIDGET_VALUES(8) ),
						CONFIG_WIDGET( .widget_id = "radio_cairo_color_quant_64",
							.values = CONFIG_WIDGET_VALUES(64) ),
						CONFIG_WIDGET( .widget_id = "radio_cairo_color_quant_128",
							.values = CONFIG_WIDGET_VALUES(128) ),
						CONFIG_WIDGET( .widget_id = "radio_cairo_color_quant_256",
							.values = CONFIG_WIDGET_VALUES(256) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "Cairo Line Cap Style", .format = "%d",
		.vars = { &rc_config.cairo_line_cap },
		.def = { { .i = CAIRO_LINE_CAP_BUTT } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_render_redraw,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &render_settings_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "radio_cairo_line_cap_butt",
							.values = CONFIG_WIDGET_VALUES(CAIRO_LINE_CAP_BUTT) ),
						CONFIG_WIDGET( .widget_id = "radio_cairo_line_cap_round",
							.values = CONFIG_WIDGET_VALUES(CAIRO_LINE_CAP_ROUND) ),
						CONFIG_WIDGET( .widget_id = "radio_cairo_line_cap_square",
							.values = CONFIG_WIDGET_VALUES(CAIRO_LINE_CAP_SQUARE) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "Current Flow Visualization Mode", .format = "%d",
		.vars = { &rc_config.current_flow_visualization_mode },
		.def = { { .i = FLOW_DIR_REFERENCE_PHASE } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_flow_direction,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &main_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "main_flow_dir_ref_phase",
							.values = CONFIG_WIDGET_VALUES(FLOW_DIR_REFERENCE_PHASE) ),
						CONFIG_WIDGET( .widget_id = "main_flow_dir_pol_axis",
							.values = CONFIG_WIDGET_VALUES(FLOW_DIR_POLARIZATION_TILT) ),
						CONFIG_WIDGET( .widget_id = "main_flow_dir_peak_mag",
							.values = CONFIG_WIDGET_VALUES(FLOW_DIR_PEAK_MAGNITUDE) ),
						CONFIG_WIDGET( .widget_id = "main_flow_dir_lic",
							.values = CONFIG_WIDGET_VALUES(FLOW_DIR_LIC) ),
						CONFIG_WIDGET( .widget_id = "main_flow_dir_wireframe",
							.values = CONFIG_WIDGET_VALUES(FLOW_DIR_WIREFRAME) ),
						NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &animate_dialog_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "anim_flow_dir",
							.values = CONFIG_WIDGET_VALUES(FLOW_DIR_REFERENCE_PHASE,
								FLOW_DIR_LIC, FLOW_DIR_WIREFRAME) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "Animate Color Projection", .format = "%d",
		.vars = { &rc_config.anim_color_proj },
		.def = { { .i = CHROMA_PROJ_INSTANT } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_color_vis,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &animate_dialog_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "anim_projsel_instant",
							.values = CONFIG_WIDGET_VALUES(CHROMA_PROJ_INSTANT) ),
						CONFIG_WIDGET( .widget_id = "anim_projsel_polarity",
							.values = CONFIG_WIDGET_VALUES(CHROMA_PROJ_SIGNED) ),
						CONFIG_WIDGET( .widget_id = "anim_projsel_phase",
							.values = CONFIG_WIDGET_VALUES(CHROMA_PROJ_PHASE) ),
						CONFIG_WIDGET( .widget_id = "anim_projsel_dual",
							.values = CONFIG_WIDGET_VALUES(CHROMA_PROJ_DUAL) ),
						CONFIG_WIDGET( .widget_id = "anim_projsel_amplitude",
							.values = CONFIG_WIDGET_VALUES(CHROMA_PROJ_AMPLITUDE) ),
						CONFIG_WIDGET( .widget_id = "anim_projsel_standing",
							.values = CONFIG_WIDGET_VALUES(CHROMA_PROJ_STANDING) ),
						CONFIG_WIDGET( .widget_id = "anim_projsel_farfield",
							.values = CONFIG_WIDGET_VALUES(CHROMA_PROJ_FARFIELD) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "Color Scale", .format = "%d",
		.vars = { &rc_config.color_scale },
		.def = { { .i = COLOR_TONE_POWER } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_color_family,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &main_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "main_color_fam_power",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_POWER) ),
						CONFIG_WIDGET( .widget_id = "main_color_fam_db",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_DB) ),
						CONFIG_WIDGET( .widget_id = "main_color_fam_asinh",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_ASINH) ),
						CONFIG_WIDGET( .widget_id = "main_color_fam_mulaw",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_MULAW) ),
						CONFIG_WIDGET( .widget_id = "main_color_fam_reinhard",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_REINHARD) ),
						CONFIG_WIDGET( .widget_id = "main_color_fam_sigmoid",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_SIGMOID) ),
						CONFIG_WIDGET( .widget_id = "main_color_fam_none",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_NONE) ),
						NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &animate_dialog_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "anim_famsel_power",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_POWER) ),
						CONFIG_WIDGET( .widget_id = "anim_famsel_db",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_DB) ),
						CONFIG_WIDGET( .widget_id = "anim_famsel_asinh",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_ASINH) ),
						CONFIG_WIDGET( .widget_id = "anim_famsel_mulaw",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_MULAW) ),
						CONFIG_WIDGET( .widget_id = "anim_famsel_reinhard",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_REINHARD) ),
						CONFIG_WIDGET( .widget_id = "anim_famsel_sigmoid",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_SIGMOID) ),
						CONFIG_WIDGET( .widget_id = "anim_famsel_none",
							.values = CONFIG_WIDGET_VALUES(COLOR_TONE_NONE) ),
						NULL ) ),
				NULL ) ) },

	{ .desc = "Color Family Param Power", .format = "%lf",
		.vars = { &rc_config.color_fam_param[COLOR_TONE_POWER] }, .def = { { .d = 0.0 } },
		.widgets = CONFIG_WIDGET_SINGLE( &animate_dialog_builder,
			"anim_fam_power", hook_color_vis ) },

	{ .desc = "Color Family Param dB", .format = "%lf",
		.vars = { &rc_config.color_fam_param[COLOR_TONE_DB] }, .def = { { .d = 40.0 } },
		.widgets = CONFIG_WIDGET_SINGLE( &animate_dialog_builder,
			"anim_fam_db", hook_color_vis ) },

	{ .desc = "Color Family Param asinh", .format = "%lf",
		.vars = { &rc_config.color_fam_param[COLOR_TONE_ASINH] }, .def = { { .d = -1.0 } },
		.widgets = CONFIG_WIDGET_SINGLE( &animate_dialog_builder,
			"anim_fam_asinh", hook_color_vis ) },

	{ .desc = "Color Family Param mu-law", .format = "%lf",
		.vars = { &rc_config.color_fam_param[COLOR_TONE_MULAW] }, .def = { { .d = 40.0 } },
		.widgets = CONFIG_WIDGET_SINGLE( &animate_dialog_builder,
			"anim_fam_mulaw", hook_color_vis ) },

	{ .desc = "Color Family Param Reinhard", .format = "%lf",
		.vars = { &rc_config.color_fam_param[COLOR_TONE_REINHARD] }, .def = { { .d = 0.0 } },
		.widgets = CONFIG_WIDGET_SINGLE( &animate_dialog_builder,
			"anim_fam_reinhard", hook_color_vis ) },

	{ .desc = "Color Family Param Sigmoid", .format = "%lf",
		.vars = { &rc_config.color_fam_param[COLOR_TONE_SIGMOID] }, .def = { { .d = 0.0 } },
		.widgets = CONFIG_WIDGET_SINGLE( &animate_dialog_builder,
			"anim_fam_sigmoid", hook_color_vis ) },

	{ .desc = "Color Brightness Floor", .format = "%lf",
		.vars = { &rc_config.color_lum_floor }, .def = { { .d = 0.20 } },
		.widgets = CONFIG_WIDGET_SINGLE( &animate_dialog_builder,
			"anim_bright_floor", hook_color_vis ) },

	{ .desc = "Color Width From Amplitude", .format = "%d",
		.vars = { &rc_config.color_width_amp },
		.widgets = CONFIG_WIDGET_SINGLE( &animate_dialog_builder,
			"anim_width_amp", hook_color_vis ) },

	{ .desc = "Overlay Nodes", .format = "%d",
		.vars = { &rc_config.overlay_nodes },
		.widgets = CONFIG_WIDGET_SINGLE( &animate_dialog_builder,
			"anim_overlay_nodes", hook_color_vis ) },

	{ .desc = "Overlay Comet", .format = "%d",
		.vars = { &rc_config.overlay_comet },
		.widgets = CONFIG_WIDGET_SINGLE( &animate_dialog_builder,
			"anim_overlay_comet", hook_color_vis ) },

	{ .desc = "Brightness Segments", .format = "%f",
		.vars = { &rc_config.brightness_segments }, .def = { { .f = 0.47f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_bright_segments", hook_render_redraw ) },

	{ .desc = "Brightness Patches", .format = "%f",
		.vars = { &rc_config.brightness_patches }, .def = { { .f = 0.47f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_bright_patches", hook_render_redraw ) },

	{ .desc = "Brightness Rdpat Surface", .format = "%f",
		.vars = { &rc_config.brightness_rdpat_surface }, .def = { { .f = 0.47f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_bright_rdpat_surface", hook_render_redraw ) },

	{ .desc = "Brightness Rdpat Wire", .format = "%f",
		.vars = { &rc_config.brightness_rdpat_wire }, .def = { { .f = 1.0f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_bright_rdpat_wire", hook_render_redraw ) },

	{ .desc = "Brightness Nearfield", .format = "%f",
		.vars = { &rc_config.brightness_nearfield }, .def = { { .f = 1.0f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_bright_nearfield", hook_render_redraw ) },

	{ .desc = "Brightness Ground Plane", .format = "%f",
		.vars = { &rc_config.brightness_ground_plane }, .def = { { .f = 1.0f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_bright_ground_plane", hook_render_redraw ) },

	{ .desc = "Brightness Axes", .format = "%f",
		.vars = { &rc_config.brightness_axes }, .def = { { .f = 1.0f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_bright_axes", hook_render_redraw ) },

	{ .desc = "Transparency Segments", .format = "%f",
		.vars = { &rc_config.transparency_segments }, .def = { { .f = 0.5f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_trans_segments", hook_render_redraw ) },

	{ .desc = "Transparency Patches", .format = "%f",
		.vars = { &rc_config.transparency_patches }, .def = { { .f = 0.5f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_trans_patches", hook_render_redraw ) },

	{ .desc = "Transparency Rdpat Surface", .format = "%f",
		.vars = { &rc_config.transparency_rdpat_surface }, .def = { { .f = 0.5f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_trans_rdpat_surface", hook_render_redraw ) },

	{ .desc = "Transparency Rdpat Wire", .format = "%f",
		.vars = { &rc_config.transparency_rdpat_wire }, .def = { { .f = 0.5f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_trans_rdpat_wire", hook_render_redraw ) },

	{ .desc = "Transparency Nearfield", .format = "%f",
		.vars = { &rc_config.transparency_nearfield },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_trans_nearfield", hook_render_redraw ) },

	{ .desc = "Transparency Ground Plane", .format = "%f",
		.vars = { &rc_config.transparency_ground_plane }, .def = { { .f = 0.5f } },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_trans_ground_plane", hook_render_redraw ) },

	{ .desc = "Transparency Axes", .format = "%f",
		.vars = { &rc_config.transparency_axes },
		.widgets = GL_CONFIG_WIDGET_SINGLE( &render_settings_builder,
			"scale_trans_axes", hook_render_redraw ) },

	{ .desc = "Frequency Plots Window Size, in pixels", .format = "%d,%d",
		.vars = { &rc_config.freqplots_width, &rc_config.freqplots_height } },

	{ .desc = "Frequency Plots Window Position (root x and y)", .format = "%d,%d",
		.vars = { &rc_config.freqplots_x, &rc_config.freqplots_y },
		.def = { { .i = -1 }, { .i = -1 } } },

	{ .desc = "Frequency Plots Window Max Gain toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_gmax_togglebutton }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_gmax_togglebutton", hook_freqplots_gmax ) },

	{ .desc = "Frequency Plots Window Gain Direction toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_gdir_togglebutton },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_gdir_togglebutton", hook_freqplots_gdir ) },

	{ .desc = "Frequency Plots Window Viewer Direction Gain toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_gviewer_togglebutton },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_gviewer_togglebutton", hook_freqplots_gviewer ) },

	{ .desc = "Frequency Plots Window VSWR toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_vswr_togglebutton }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_vswr_togglebutton", hook_freqplots_vswr ) },

	{ .desc = "Frequency Plots Window Z-real/Z-imag toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_zrlzim_togglebutton },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_zrlzim_togglebutton", hook_freqplots_zrlzim ) },

	{ .desc = "Frequency Plots Window Z-mag/Z-phase toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_zmgzph_togglebutton },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_zmgzph_togglebutton", hook_freqplots_zmgzph ) },

	{ .desc = "Frequency Plots Window Smith toggle button state", .format = "%d",
		.vars = { &rc_config.freqplots_smith_togglebutton },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_smith_togglebutton", hook_freqplots_smith ) },

	{ .desc = "Freqplots Ant Temp Toggle", .format = "%d",
		.vars = { &rc_config.freqplots_ant_temp_togglebutton },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_ant_temp_togglebutton", hook_freqplots_ant_temp ) },

	{ .desc = "Freqplots Show Ant Temp (Ta instead of TA)", .format = "%d",
		.vars = { &rc_config.freqplots_show_ant_temp },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_show_ant_temp", hook_freqplots_show_ant_temp ) },

	{ .desc = "Frequency Plots Window Net Gain checkbutton state", .format = "%d",
		.vars = { &rc_config.freqplots_net_gain },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_net_gain", hook_freqplots_net_gain ) },

	{ .desc = "Frequency Plots Window Min/Max checkbutton state", .format = "%d",
		.vars = { &rc_config.freqplots_min_max },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_min_max", hook_freqplots_min_max ) },

	{ .desc = "NEC2 Editor Window Size, in pixels", .format = "%d,%d",
		.vars = { &rc_config.nec2_edit_width, &rc_config.nec2_edit_height } },

	{ .desc = "NEC2 Editor Window Position (root x and y)", .format = "%d,%d",
		.vars = { &rc_config.nec2_edit_x, &rc_config.nec2_edit_y } },

	{ .desc = "Enable Confirm Quit Dialog", .format = "%d",
		.vars = { &rc_config.confirm_quit }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_SINGLE( &main_window_builder, "confirm_quit", NULL ) },

	{ .desc = "Selected Mathlib", .format = "%s",
		.vars = { rc_config.mathlib_id }, .size = MATHLIB_ID_LEN, .init = mathlib_config_init },

	{ .desc = "Selected Batch Mathlib", .format = "%s",
		.vars = { rc_config.mathlib_batch_id }, .size = MATHLIB_ID_LEN, .init = mathlib_config_init },

	{ .desc = "Selected Mathlib Benchmarks",
		.parse = mathlib_config_benchmark_parse,
		.save = mathlib_config_benchmark_save  },

	{ .desc = "Selected fmhz_save Frequency", .format = "%lf",
		.vars = { &calc_data.fmhz_save },
		.widgets = CONFIG_WIDGET_TREE( .on_change = hook_frequency,
			.groups = CONFIG_WIDGET_GROUPS(
				CONFIG_WIDGET_GROUP( .builder = &main_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "main_freq_spinbutton" ), NULL ) ),
				CONFIG_WIDGET_GROUP( .builder = &rdpattern_window_builder,
					.elements = CONFIG_WIDGETS(
						CONFIG_WIDGET( .widget_id = "rdpattern_freq_spinbutton" ), NULL ) ),
				NULL ) ) },

	{ .desc = "Frequency Plots Show S11 checkbutton state", .format = "%d",
		.vars = { &rc_config.freqplots_s11 },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_s11", hook_freqplots_s11 ) },

	{ .desc = "Frequency Plots Show Clamp VSWR checkbutton state", .format = "%d",
		.vars = { &rc_config.freqplots_clamp_vswr }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_clamp_vswr", hook_freqplots_clamp_vswr ) },

	{ .desc = "Radiation Plots Gain Style", .format = "%d",
		.vars = { &rc_config.gain_style }, .def = { { .i = GS_LINP } } },

	{ .desc = "Round X Axis", .format = "%d",
		.vars = { &rc_config.freqplots_round_x_axis },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_round_x_axis", hook_freqplots_round_x_axis ) },

	{ .desc = "Frequency Plots Swap Click", .format = "%d",
		.vars = { &rc_config.freqplots_swap_click }, .def = { { .i = 1 } },
		.widgets = CONFIG_WIDGET_SINGLE( &freqplots_window_builder,
			"freqplots_swap_click", hook_freqplots_swap_click ) },

	{ .desc = "Optimizer Write CSV", .format = "%d", .batch_mode_skip = TRUE,
		.vars = { &rc_config.opt_write_csv },
		.widgets = CONFIG_WIDGET_SINGLE( &main_window_builder, "optimizer_write_csv", NULL ) },

	{ .desc = "Optimizer Write S1P", .format = "%d", .batch_mode_skip = TRUE,
		.vars = { &rc_config.opt_write_s1p },
		.widgets = CONFIG_WIDGET_SINGLE( &main_window_builder, "optimizer_write_s1p", NULL ) },

	{ .desc = "Optimizer Write S2P Max Gain", .format = "%d", .batch_mode_skip = TRUE,
		.vars = { &rc_config.opt_write_s2p_max_gain },
		.widgets = CONFIG_WIDGET_SINGLE( &main_window_builder, "optimizer_write_s2p_max_gain", NULL ) },

	{ .desc = "Optimizer Write S2P Viewer Gain", .format = "%d", .batch_mode_skip = TRUE,
		.vars = { &rc_config.opt_write_s2p_viewer_gain },
		.widgets = CONFIG_WIDGET_SINGLE( &main_window_builder, "optimizer_write_s2p_viewer_gain", NULL ) },

	{ .desc = "Optimizer Write CSV Radiation Pattern", .format = "%d", .batch_mode_skip = TRUE,
		.vars = { &rc_config.opt_write_rdpat },
		.widgets = CONFIG_WIDGET_SINGLE( &main_window_builder, "optimizer_write_rdpat", NULL ) },

	{ .desc = "Optimizer Write CSV Currents/Charges", .format = "%d", .batch_mode_skip = TRUE,
		.vars = { &rc_config.opt_write_currents},
		.widgets = CONFIG_WIDGET_SINGLE( &main_window_builder, "optimizer_write_currents", NULL ) },

	{ .desc = "Optimizer Write Gnuplot Structure", .format = "%d", .batch_mode_skip = TRUE,
		.vars = { &rc_config.opt_write_gnuplot_structure},
		.widgets = CONFIG_WIDGET_SINGLE( &main_window_builder, "optimizer_write_gnuplot_structure", NULL ) },

	{ .desc = "Optimizer Write Patch Currents", .format = "%d", .batch_mode_skip = TRUE,
		.vars = { &rc_config.opt_write_patch_currents},
		.widgets = CONFIG_WIDGET_SINGLE( &main_window_builder, "optimizer_write_patch_currents", NULL ) },

	// The *_is_open values below default to "1" for backward compatiblity.  It
	// still does not open the window if width/height are undefined so defaults
	// are consistent.  However, if they are defined but *_is_open is not defined
	// (ie, <= v4.1.12) then the specified windows will open.  After the first
	// run the new version will use xnec2c.conf for *_is_open values.
	{ .desc = "Frequency Plots window is open", .format = "%d",
		.vars = { &rc_config.freqplots_is_open }, .def = { { .i = 1 } } },

	{ .desc = "Radiation Pattern Window window is open", .format = "%d",
		.vars = { &rc_config.rdpattern_is_open }, .def = { { .i = 1 } } },

	{ .desc = "Symbol Overrides Window is open", .format = "%d",
		.vars = { &rc_config.sy_overrides_is_open } },

	{ .desc = "Symbol Overrides Window Size, in pixels", .format = "%d,%d",
		.vars = { &rc_config.sy_overrides_width, &rc_config.sy_overrides_height } },

	{ .desc = "Symbol Overrides Window Position (root x and y)", .format = "%d,%d",
		.vars = { &rc_config.sy_overrides_x, &rc_config.sy_overrides_y },
		.def = { { .i = -1 }, { .i = -1 } } },

	{ .desc = "Antenna Temp Sky Model", .format = "%d",
		.vars = { &rc_config.ant_temp_sky },
		.def = { { .i = ANT_TEMP_SKY_SYNTH_AVG } } },

	{ .desc = "Antenna Temp Earth Model", .format = "%d",
		.vars = { &rc_config.ant_temp_earth },
		.def = { { .i = ANT_TEMP_EARTH_DG7YBN_RESIDENTIAL } } },

	{ .desc = "Antenna Temp Interp Method", .format = "%d",
		.vars = { &rc_config.ant_temp_interp }, .def = { { .i = ANT_TEMP_INTERP } } },

	{ .desc = "Antenna Temp Elevation (deg, +=up)", .format = "%lf",
		.vars = { &rc_config.ant_temp_elevation } },

	{ .desc = "Antenna Temp Custom Sky (K)", .format = "%lf",
		.vars = { &rc_config.ant_temp_custom_t_sky },
		.def = { { .d = ANT_TEMP_CUSTOM_T_SKY_DEFAULT } } },

	{ .desc = "Antenna Temp Custom Earth (K)", .format = "%lf",
		.vars = { &rc_config.ant_temp_custom_t_earth },
		.def = { { .d = ANT_TEMP_CUSTOM_T_EARTH_DEFAULT } } },

	/* The theme selector radios are runtime-built menu items owned by
	 * theme.c, so these trees bind no widgets; the rows still own the
	 * unified theme-derived refresh fired by config_widget_field_changed. */
	{ .desc = "Frequency Plots Color Theme", .format = "%s",
		.vars = { rc_config.freqplots_theme }, .size = sizeof(rc_config.freqplots_theme),
		.def = { { .s = "legacy" } },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_theme_change,
			.groups = CONFIG_WIDGET_GROUPS( NULL ) ) },

	{ .desc = "Frequency Plots Theme Inverted", .format = "%d",
		.vars = { &rc_config.freqplots_theme_invert },
		.widgets = CONFIG_WIDGET_TREE( .post_apply = hook_theme_change,
			.groups = CONFIG_WIDGET_GROUPS( NULL ) ) },

};


rc_config_vars_t rc_config_vars[];
int num_rc_config_vars = sizeof(rc_config_vars) / sizeof(rc_config_vars_t);

char *get_conf_dir(char *s, int len)
{
	char *home = getenv("HOME");
	if (home == NULL || strlen(home) == 0)
	{
		if (getcwd(s, len) == NULL)
		{
			pr_warn("unable to getcwd(): %s\n", strerror(errno));
			strncpy(s, "./", len);
		}

		pr_warn("environment variable HOME is undefined, using %s\n", s);
	}
	else
	{
		strncpy(s, home, len);
	}

	return s;
}

// Trim the newline
void chomp(char *line)
{
	int i;
	int len = strlen(line);

	for (i = 0; i < len; i++)
		if (line[i] == '\r' || line[i] == '\n')
		{
			line[i] = 0;
			break;
		}
}

// Find a variable defined by its comment
rc_config_vars_t *find_var(char *s)
{
	while (s[0] && (isspace(s[0]) || s[0] == '#'))
		s++;

	for (int i = 0; i < num_rc_config_vars; i++)
		if (!strcmp(rc_config_vars[i].desc, s))
			return &rc_config_vars[i];

	return NULL;
}

// Read the line into the variable's .vars reference(s)
int parse_var(rc_config_vars_t *v, char *line, char *locale)
{
	char *format = NULL;
	int count = 0;
	int check = 0;
	size_t mreq;

	// Use the parse function if available:
	if (v->parse != NULL)
		return v->parse(v, line);

	// Skip read-only or missing vars:
	if (v->ro || (v->vars[0] == NULL && v->vars[1] == NULL))
		return 0;

	mreq = strlen(v->format)+4;
	mem_alloc(&format, mreq);
	snprintf(format, mreq-1, "%s%%n", v->format);

	setlocale(LC_NUMERIC, locale);

	if (strcmp(v->format, "%d") == 0)
		count = sscanf(line, format, (int *)v->vars[0], &check);
	else if (strcmp(v->format, "%s") == 0)
	{
		strncpy((char*)v->vars[0], line, v->size - 1);
		((char*)v->vars[0])[v->size - 1] = '\0';
		count = 1;
		check = strlen(line);
	}
	else if (strcmp(v->format, "%f") == 0)
		count = sscanf(line, format, (float *)v->vars[0], &check);
	else if (strcmp(v->format, "%lf") == 0)
		count = sscanf(line, format, (double *)v->vars[0], &check);
	else if (strcmp(v->format, "%d,%d") == 0)
		count = sscanf(line, format, (int *)v->vars[0], (int *)v->vars[1], &check);
	else if (strcmp(v->format, "%f,%f") == 0)
		count = sscanf(line, format, (float *)v->vars[0], (float *)v->vars[1], &check);
	else if (strcmp(v->format, "%lf,%lf") == 0)
		count = sscanf(line, format, (double *)v->vars[0], (double *)v->vars[1], &check);

	setlocale(LC_NUMERIC, orig_numeric_locale);

	mem_free(&format);

	if (check != strlen(line))
	{
		pr_warn("%s (locale=%s): only matchd %d of %lu chars, trying another locale: %s\n",
		        v->desc, locale, check, (unsigned long)strlen(line), line);
		return 0;
	}

	// `count` contains the number of vars parsed, return true
	// if the count matches the number of vars, otherwise it failed
	// to parse.  Make this a loop if you add support for more than
	// two vars being parsed (see struct rc_config_vars_t.vars[2]).
	return (v->vars[0] == NULL && count == 0)
		|| (v->vars[1] == NULL && count == 1)
		|| (v->vars[1] != NULL && count == 2);
}

// Print a var to a file descriptor based on its type:
int fprint_var(FILE *fp, rc_config_vars_t *v)
{
	int count = 0;
	
	if (v->save != NULL)
		return v->save(v, fp);

	if (v->vars[0] == NULL && v->vars[1] == NULL)
		return 0;

	setlocale(LC_NUMERIC, "C");

	if (strcmp(v->format, "%d") == 0)
		count = fprintf(fp, v->format, *(int*)v->vars[0]);
	else if (strcmp(v->format, "%s") == 0)
		count = fprintf(fp, v->format, (char*)v->vars[0]);
	else if (strcmp(v->format, "%f") == 0)
		count = fprintf(fp, v->format, (double)*(float*)v->vars[0]);
	else if (strcmp(v->format, "%lf") == 0)
		count = fprintf(fp, v->format, *(double*)v->vars[0]);
	else if (strcmp(v->format, "%d,%d") == 0)
		count = fprintf(fp, v->format, *(int*)v->vars[0], *(int*)v->vars[1]);
	else if (strcmp(v->format, "%f,%f") == 0)
		count = fprintf(fp, v->format, *(float*)v->vars[0], *(float*)v->vars[1]);
	else if (strcmp(v->format, "%lf,%lf") == 0)
		count = fprintf(fp, v->format, *(double*)v->vars[0], *(double*)v->vars[1]);

	setlocale(LC_NUMERIC, orig_numeric_locale);

	return count;
}


// Compute the working-directory default at reset time: the user's home
// directory (or the get_conf_dir fallback) with a trailing slash.
static void working_dir_set_default(rc_config_vars_t *v)
{
	char home[PATH_MAX];

	Strlcpy((char*)v->vars[0], get_conf_dir(home, sizeof(home)), v->size);
	Strlcat((char*)v->vars[0], "/", v->size);
}

// Return the first config row whose primary variable matches f, or NULL.
rc_config_vars_t *rc_config_find_by_field(void *f)
{
	if (f == NULL)
		return NULL;

	for (int i = 0; i < num_rc_config_vars; i++)
		if (rc_config_vars[i].vars[0] == f)
			return &rc_config_vars[i];

	return NULL;
}

// Return the compiled-in integer default for the field f.  A missing owner row
// or non-integer format is a build-time wiring error, so surface it and yield
// zero; callers clamp a local copy without mutating the stored value.
int rc_config_default_int(void *f)
{
	rc_config_vars_t *v = rc_config_find_by_field(f);

	if (v == NULL || strcmp(v->format, "%d") != 0)
	{
		BUG("rc_config_default_int: no integer default for field\n");
		return 0;
	}

	return v->def[0].i;
}

// Reset one config row to its compiled-in default.  The type switch mirrors
// fprint_var(); the %s branch writes only when a string default is present so
// init-reactive rows (eg mathlib) keep their loaded value.  A set_default hook
// computes the default instead (eg the runtime working directory).
void rc_config_set_default(rc_config_vars_t *v)
{
	if (v == NULL)
		return;

	if (v->set_default != NULL)
	{
		v->set_default(v);
		return;
	}

	// Skip read-only, custom parse/save, and unbacked rows.
	if (v->ro || v->parse != NULL || v->save != NULL || v->vars[0] == NULL)
		return;

	if (strcmp(v->format, "%d") == 0)
		*(int*)v->vars[0] = v->def[0].i;
	else if (strcmp(v->format, "%s") == 0)
	{
		if (v->def[0].s != NULL)
			Strlcpy((char*)v->vars[0], v->def[0].s, v->size);
	}
	else if (strcmp(v->format, "%f") == 0)
		*(float*)v->vars[0] = v->def[0].f;
	else if (strcmp(v->format, "%lf") == 0)
		*(double*)v->vars[0] = v->def[0].d;
	else if (strcmp(v->format, "%d,%d") == 0)
	{
		*(int*)v->vars[0] = v->def[0].i;
		*(int*)v->vars[1] = v->def[1].i;
	}
	else if (strcmp(v->format, "%f,%f") == 0)
	{
		*(float*)v->vars[0] = v->def[0].f;
		*(float*)v->vars[1] = v->def[1].f;
	}
	else if (strcmp(v->format, "%lf,%lf") == 0)
	{
		*(double*)v->vars[0] = v->def[0].d;
		*(double*)v->vars[1] = v->def[1].d;
	}
}

// Apply every row's compiled-in default; the sole init-time default path.
void rc_config_apply_defaults(void)
{
	for (int i = 0; i < num_rc_config_vars; i++)
	{
		rc_config_vars_t *v = &rc_config_vars[i];

		// A field may appear in more than one row (eg the dual
		// main_common_projection keys kept for backward compatibility).
		// Only the first row owning the field applies its default; later
		// rows defer to that canonical owner.
		if (v->vars[0] != NULL && rc_config_find_by_field(v->vars[0]) != v)
			continue;

		rc_config_set_default(v);
	}
}


/*
 * rc_config_field_size()
 *
 * Field storage width derived from a row's format string, matching the
 * discriminant parse_var/fprint_var dispatch on.  Returns 0 on an
 * unsupported format after raising a BUG.
 */
size_t rc_config_field_size(const rc_config_vars_t *v)
{
	if (strcmp(v->format, "%d") == 0)
		return sizeof(int);
	if (strcmp(v->format, "%f") == 0)
		return sizeof(float);
	if (strcmp(v->format, "%lf") == 0)
		return sizeof(double);

	/* String rows carry their buffer width in .size */
	if (strcmp(v->format, "%s") == 0 && v->size > 0)
		return v->size;

	BUG("rc_config_field_size: %s: unsupported format %s\n", v->desc, v->format);
	return 0;
}


/*
 * rc_config_register_widgets()
 *
 * Registers every row's .widgets tree with the config_widget engine,
 * keyed by the row's primary field and width derived from its format
 * string.  Called once from main.c startup, before Read_Config().
 */
void rc_config_register_widgets(void)
{
	for (int i = 0; i < num_rc_config_vars; i++)
	{
		rc_config_vars_t *v = &rc_config_vars[i];
		size_t size;

		if (v->widgets == NULL)
			continue;

		size = rc_config_field_size(v);
		if (size == 0)
			continue;

		config_widget_register(v->vars[0], size, v->widgets);
	}
}


/*------------------------------------------------------------------------*/

/* Get_Widget_Geometry()
 *
 * Gets a widget's size and position
 * Return: 1 if open, 0 if closed.
 */
int Get_Window_Geometry(
    GtkWidget *window,
    gint *x, gint *y, gint *width, gint *height )
{
  if( window ) /* Save window state for restoring later */
  {
    gtk_window_get_size( GTK_WINDOW(window), width, height );
    gtk_window_get_position( GTK_WINDOW(window), x, y );
	pr_debug("window: x=%-5d y=%-5d w=%-5d h=%-5d  title: \"%s\"\n",
		*x, *y, *width, *height,
		gtk_window_get_title(GTK_WINDOW(window)));
    return 1;
  }
  else /* Disable window restoring */
  {
    return 0;
  }

} /* Get_Widget_Geometry() */

/*------------------------------------------------------------------------*/

/* Create_Default_Config()
 *
 * Provide a warning if the version changes.
 * Set defaults, they will be saved later.  Read_Config() is called after
 * this function so it will override any settings if they are available in the config
 * and save a new file if it is missing using the defaults defined below.
 */

  gboolean
Create_Default_Config( void )
{
  char line[LINE_LEN];
  FILE *fp = NULL;

  fp = fopen(rc_config.config_file, "r");
  if( fp != NULL )
  {
    /* Read Application Version */
    if( Load_Line(line, fp) == EOF )
    {
      pr_err("%s: EOF reading application version.\n", rc_config.config_file);
      strcpy(line, "(unknown)");
    }

    /* Produce fresh default config file if version number new */
    if( strncmp(line, PACKAGE_STRING, sizeof(line)) != 0 )
      pr_notice("existing config file version differs: %s != %s\n", line, PACKAGE_STRING);

    Close_File( &fp );
  }

  rc_config_apply_defaults();

  return( TRUE );

} /* Create_Default_Config() */

/*------------------------------------------------------------------------*/

/* Set_Window_Geometry()
 *
 * Sets the size and position of a window
 */
  void
Set_Window_Geometry(
    GtkWidget *window,
    gint x, gint y, gint width, gint height )
{
  /* Set size and position of window */
  /* gtk_widget_hide( window ); this leads to an undecorated window in icewm */
  if (width && height)
    gtk_window_resize( GTK_WINDOW(window), width, height );

  if (x >= 0 && y >= 0)
    gtk_window_move( GTK_WINDOW(window), x, y );

} /* Set_Window_Geometry() */

/*------------------------------------------------------------------------*/

/* Restore_Windows()
 *
 * Restores the rdpattern and freq plots windows
 */
  static gboolean
Restore_Windows( gpointer dat )
{
  GtkWidget *widget;

  /* Open frequency plots window if state data available */
  if( rc_config.main_loop_start || isFlagSet(SUPPRESS_INTERMEDIATE_REDRAWS) ||
	  (rc_config.freqplots_is_open && rc_config.freqplots_width && rc_config.freqplots_height))
  {
    widget = Builder_Get_Object( main_window_builder, "main_freqplots" );
    gtk_menu_item_activate( GTK_MENU_ITEM(widget) );
  }

  /* Open radiation pattern window if state data available */
  if( rc_config.rdpattern_is_open && rc_config.rdpattern_width && rc_config.rdpattern_height)
  {
    widget = Builder_Get_Object( main_window_builder, "main_rdpattern" );
    gtk_menu_item_activate( GTK_MENU_ITEM(widget) );
  }

  /* Open symbol overrides window if state data available */
  if( rc_config.sy_overrides_is_open && rc_config.sy_overrides_width && rc_config.sy_overrides_height)
  {
    widget = Builder_Get_Object( main_window_builder, "show_sy_overrides" );
    gtk_menu_item_activate( GTK_MENU_ITEM(widget) );
  }

  return( FALSE );
}

/*------------------------------------------------------------------------*/

/* Restore_GUI_State()
 *
 * Restores the state of the GUI including window geometry
 */
  static void
Restore_GUI_State( void )
{
  /* Write every registered field into its live widgets, then run the
   * change-edge hooks of every tree bound to the main window so their
   * derived state (flags, labels, menu-item sensitivity) matches the
   * values just loaded from the config file. */
  config_widget_sync_all();
  config_widget_run_hooks( &main_window_builder );

#ifdef HAVE_OPENGL
  /* Restore MSAA menu selection */
  Set_MSAA_Samples( rc_config.opengl_msaa_samples );

  /* Sync render settings dialog widgets from rc_config */
  render_settings_sync_from_config();
#endif

  /* Restore main (structure) window geometry after all widget state
   * restorations so layout-affecting changes precede sizing */
  Set_Window_Geometry( main_window,
      rc_config.main_x, rc_config.main_y,
      rc_config.main_width, rc_config.main_height );
  gtk_widget_show( main_window );

  g_idle_add( Restore_Windows, NULL );

} /* Restore_GUI_State() */

/*------------------------------------------------------------------------*/

/* Read_Config()
 *
 * Loads the xnec2crc configuration file
 */

  gboolean
Read_Config( void )
{
  char
    home[PATH_MAX],
    fpath[FILENAME_LEN], /* File path to xnec2crc */
    line[LINE_LEN];
  int lnum;

  struct stat st;
  int umsk, newmode;

  /* Config and mnemonics file pointer */
  FILE *fp = NULL;


  /* Create the dir if missing */
  snprintf( fpath, sizeof(fpath), "%s/.xnec2c", get_conf_dir(home, sizeof(home)));
  if( access(fpath, R_OK) < 0 && errno == ENOENT)
	  mkdir(fpath, 0755);

  // In commit 4e62893b the mkdir call used 755 instead of 0755 so
  // permissions were broken.  This fixes that:
  if (stat(fpath, &st) != 0)
	  pr_err("stat failed for %s: %s\n", fpath, strerror(errno));
  st.st_mode &= 01777;
  umask(umsk = umask(0));
  if (st.st_mode == 755 || st.st_mode == (755 & (~umsk)))
  {
	  newmode = 0755 & (~umsk) & 0777;
	  pr_notice("Fixed %s directory permissions from 0%o to 0%o from bug introduced in 4e62893b.\n",
                  fpath, st.st_mode, newmode);
	  chmod(fpath, newmode);
  }

  /* Create the file if missing */
  if (access(rc_config.config_file, R_OK) < 0 && errno == ENOENT)
  {
	  rc_config.first_run = 1;
	  Save_Config();
  }
  else
	  rc_config.first_run = 0;

  /* Open xnec2c config file */
  if (!Open_File(&fp, rc_config.config_file, "r")) return(FALSE);

  // Iterate over each line and parse the variables into
  // their references defined by rc_config_vars[].
  lnum = 0;

  while ( fgets(line, LINE_LEN, fp) != NULL)
  {
	  lnum++;
      
	  chomp(line);

	  rc_config_vars_t *v = find_var(line);
	  if (!v)
	  {
		  if (line[0] != '#')
			  pr_err("%s:%d: Line not parsed: %s\n", rc_config.config_file, lnum, line);
		  continue;
	  }
	  
	  if ( fgets(line, LINE_LEN, fp) == NULL)
	  {
		  pr_err("%s:%d: Early end of file for %s: %s \n", rc_config.config_file, lnum, v->desc, line);
		  break;
	  }

	  lnum++;

	  // Skip read-only vars:
	  if (v->ro)
		  continue;

	  // Skip vars that should be excluded when batch_mode is enabled:
	  if (rc_config.batch_mode && v->batch_mode_skip)
		  continue;

	  chomp(line);

	  // 1. Skip comments.
	  // 2. Then try in the "C" locale
	  // 3. Try in the current locale
	  // 4. Then try in a known locale with comma for decimals
	  if (line[0] != '#' &&
		  !parse_var(v, line, "C") &&
		  !parse_var(v, line, NULL) &&
		  !parse_var(v, line, "en_DK")
		  )
		  pr_err("%s:%d: parse error (%s): %s \n", rc_config.config_file, lnum, v->desc, line);
	  else
	  {
		  if (v->init != NULL)
			  v->init(v, line);
	  }
  }

  /* Close the config file pointer */
  Close_File( &fp );

  /* Validate noise model indices; reset to defaults if out of range */
  if (rc_config.ant_temp_sky < 0 || rc_config.ant_temp_sky >= ANT_TEMP_SKY_COUNT)
  {
    pr_warn("Invalid sky model index %d in config, resetting to default\n",
        rc_config.ant_temp_sky);
    rc_config_set_default(rc_config_find_by_field(&rc_config.ant_temp_sky));
  }
  if (rc_config.ant_temp_earth < 0 || rc_config.ant_temp_earth >= ANT_TEMP_EARTH_COUNT)
  {
    pr_warn("Invalid earth model index %d in config, resetting to default\n",
        rc_config.ant_temp_earth);
    rc_config_set_default(rc_config_find_by_field(&rc_config.ant_temp_earth));
  }
  if (rc_config.ant_temp_interp < 0 || rc_config.ant_temp_interp >= ANT_TEMP_METHOD_COUNT)
  {
    pr_warn("Invalid interp method %d in config, resetting to default\n",
        rc_config.ant_temp_interp);
    rc_config_set_default(rc_config_find_by_field(&rc_config.ant_temp_interp));
  }

  /* Custom temperatures must be positive (zero collapses the pattern) */
  if (rc_config.ant_temp_custom_t_sky <= ANT_TEMP_K_MIN)
    rc_config_set_default(rc_config_find_by_field(&rc_config.ant_temp_custom_t_sky));
  if (rc_config.ant_temp_custom_t_earth <= ANT_TEMP_K_MIN)
    rc_config_set_default(rc_config_find_by_field(&rc_config.ant_temp_custom_t_earth));

  Restore_GUI_State();

  return( TRUE );
} /* Read_Config() */

/*------------------------------------------------------------------------*/

/* get_main_window_state()
 *
 * Captures main (structure) window geometry and widget state
 * into the rc_config buffer
 */
  void
get_main_window_state( void )
{
  GtkWidget *widget;

  /* Get geometry of main (structure) window */
  Get_Window_Geometry( main_window,
      &(rc_config.main_x), &(rc_config.main_y),
      &(rc_config.main_width), &(rc_config.main_height) );

  widget = Builder_Get_Object( main_window_builder, "main_rotate_spinbutton" );
  gtk_spin_button_update( GTK_SPIN_BUTTON(widget) );
  rc_config.main_rotate_spinbutton =
    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget) );

  widget = Builder_Get_Object( main_window_builder, "main_incline_spinbutton" );
  gtk_spin_button_update( GTK_SPIN_BUTTON(widget) );
  rc_config.main_incline_spinbutton =
    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget) );

  widget = Builder_Get_Object( main_window_builder, "main_zoom_spinbutton" );
  gtk_spin_button_update( GTK_SPIN_BUTTON(widget) );
  rc_config.main_zoom_spinbutton =
    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget) );
}

/*------------------------------------------------------------------------*/

/* get_rdpattern_window_state()
 *
 * Captures radiation pattern window geometry and widget state
 * into the rc_config buffer
 */
  void
get_rdpattern_window_state( void )
{
  GtkWidget *widget;

  /* Get geometry of radiation patterns window */
  rc_config.rdpattern_is_open = Get_Window_Geometry( rdpattern_window,
      &(rc_config.rdpattern_x), &(rc_config.rdpattern_y),
      &(rc_config.rdpattern_width), &(rc_config.rdpattern_height) );

  /* Get state of widgets in radiation patterns window */
  if( rdpattern_window )
  {
    widget = Builder_Get_Object(
        rdpattern_window_builder, "rdpattern_zoom_spinbutton" );
    gtk_spin_button_update( GTK_SPIN_BUTTON(widget) );
    rc_config.rdpattern_zoom_spinbutton =
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget) );

    /* Antenna temperature elevation */
    widget = Builder_Get_Object(
        rdpattern_window_builder, "rdpattern_elevation_spinbutton" );
    gtk_spin_button_update( GTK_SPIN_BUTTON(widget) );
    rc_config.ant_temp_elevation =
      gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget) );

    /* Noise models — values are maintained by signal handlers;
     * no widget query needed here since rc_config fields are
     * updated directly by on_rdpattern_noise_{sky,earth,interp}_activate */
  }
}

/*------------------------------------------------------------------------*/

/* get_freqplots_window_state()
 *
 * Captures frequency plots window geometry and widget state
 * into the rc_config buffer
 */
  void
get_freqplots_window_state( void )
{
  /* Get geometry of frequency plots window */
  rc_config.freqplots_is_open = Get_Window_Geometry( freqplots_window,
      &(rc_config.freqplots_x), &(rc_config.freqplots_y),
      &(rc_config.freqplots_width), &(rc_config.freqplots_height) );
}

/*------------------------------------------------------------------------*/

/* get_nec2_edit_window_state()
 *
 * Captures NEC2 editor window geometry into the rc_config buffer
 */
  void
get_nec2_edit_window_state( void )
{
  /* Get geometry of NEC2 editor window */
  Get_Window_Geometry( nec2_edit_window,
      &(rc_config.nec2_edit_x), &(rc_config.nec2_edit_y),
      &(rc_config.nec2_edit_width), &(rc_config.nec2_edit_height) );
}

/*------------------------------------------------------------------------*/

/* get_sy_overrides_window_state()
 *
 * Captures Symbol Overrides window geometry into the rc_config buffer
 */
  void
get_sy_overrides_window_state( void )
{
  /* Get geometry of Symbol Overrides window */
  rc_config.sy_overrides_is_open = Get_Window_Geometry( sy_overrides_window,
      &(rc_config.sy_overrides_x), &(rc_config.sy_overrides_y),
      &(rc_config.sy_overrides_width), &(rc_config.sy_overrides_height) );
}

/*------------------------------------------------------------------------*/

/* Get_GUI_State()
 *
 * Captures all window geometry and widget state into the rc_config
 * buffer by dispatching to each window's own capture function
 */
  void
Get_GUI_State( void )
{
  get_main_window_state();
  get_rdpattern_window_state();
  get_freqplots_window_state();
  get_nec2_edit_window_state();
  get_sy_overrides_window_state();
} /* Get_GUI_State */

/*------------------------------------------------------------------------*/

/* Save_Config()
 *
 * Saves the current values in rc_config to xnec2c.conf
 *
 * The format is perhaps unusual.  For historical and backwards-compatibility
 * reasons the comment indicates the variable on the next line.  It used to be
 * parsed with a strict ordering, but now rc_config_vars_t defines the output
 * ordering and assigns values by reference to `rc_config`.
 */
  gboolean
Save_Config( void )
{
  FILE *fp = NULL;  /* File pointer to write config file */

  /* Batch runs must not persist configuration: validation_dump_force_config()
   * forces a canonical mathlib, antenna-temperature models, polarization, and
   * viewer orientation, and an OpenGL context that fails on a headless server
   * falls back to Cairo (opengl_gl_init_failed).  Writing any of these transient
   * values back would corrupt the host's saved configuration, so skip
   * persistence entirely in batch mode. */
  if( rc_config.batch_mode )
  {
    pr_notice("batch mode: skipping configuration save\n");
    return( TRUE );
  }

  /* Open config file for writing */
  if (!Open_File(&fp, rc_config.config_file, "w"))
  {
    pr_err("cannot open xnec2c's config file %s: %s\n", rc_config.config_file, strerror(errno));
    return( FALSE );
  }

  fprintf(fp, "# Xnec2c configuration file\n#\n");
  for (int i = 0; i < num_rc_config_vars; i++)
  {
	  fprintf(fp, "# %s\n", rc_config_vars[i].desc);
	  fprint_var(fp, &rc_config_vars[i]);
	  fprintf(fp, "\n");
  }

  Close_File( &fp );

  return( TRUE );
} /* Save_Config() */

/*------------------------------------------------------------------------*/
