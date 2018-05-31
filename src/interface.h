/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
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
 */

#ifndef INTERFACE_H
#define INTERFACE_H		1

#include "common.h"

#define ABOUT_DIALOG_IDS \
"aboutdialog", \
NULL

#define ANIMATE_DIALOG_IDS \
"animate_freq_adjustment", \
"animate_steps_adjustment", \
"animate_dialog", \
"animate_steps_spinbutton", \
"animate_freq_spinbutton", \
NULL

#define ARC_EDITOR_IDS \
"arc_dia_adjustment", \
"arc_end1_adjustment", \
"arc_end2_adjustment", \
"arc_numseg_adjustment", \
"arc_pcl_adjustment", \
"arc_rad_adjustment", \
"arc_res_adjustment", \
"arc_tagnum_adjustment", \
"arc_editor", \
"arc_tagnum_spinbutton", \
"arc_numseg_spinbutton", \
"arc_pcl_spinbutton", \
"arc_dia_spinbutton", \
"arc_rad_spinbutton", \
"arc_end1_spinbutton", \
"arc_end2_spinbutton", \
"arc_res_spinbutton", \
NULL

#define CYLINDER_EDITOR_IDS \
"cylinder_taginc_adjustment", \
"cylinder_total_adjustment", \
"cylinder_editor", \
"cylinder_total_spinbutton", \
"cylinder_taginc_spinbutton", \
NULL

#define ERROR_DIALOG_IDS \
"error_dialog", \
"error_label", \
NULL

#define EXCITATION_EDITOR_IDS \
"excitation_f1_adjustment", \
"excitation_f2_adjustment", \
"excitation_f3_adjustment", \
"excitation_f4_adjustment", \
"excitation_f5_adjustment", \
"excitation_f6_adjustment", \
"excitation_i2_adjustment", \
"excitation_i3_adjustment", \
"excitation_command", \
"excitation_i10_radiobutton", \
"excitation_i13_radiobutton", \
"excitation_i11_radiobutton", \
"excitation_i14_radiobutton", \
"excitation_i12_radiobutton", \
"excitation_i15_radiobutton", \
"excitation_i419_checkbutton", \
"excitation_i420_checkbutton", \
"excitation_i2_label", \
"excitation_i3_label", \
"excitation_i2_spinbutton", \
"excitation_i3_spinbutton", \
"excitation_f1_label", \
"excitation_f2_label", \
"excitation_f3_label", \
"excitation_f1_spinbutton", \
"excitation_f2_spinbutton", \
"excitation_f3_spinbutton", \
"excitation_f4_label", \
"excitation_f5_label", \
"excitation_f6_label", \
"excitation_f6_spinbutton", \
"excitation_f5_spinbutton", \
"excitation_f4_spinbutton", \
NULL

#define EXECUTE_EDITOR_IDS \
"execute_command", \
"execute_none_radiobutton", \
"execute_xz_radiobutton", \
"execute_yz_radiobutton", \
"execute_both_radiobutton", \
NULL

#define FILECHOOSER_DIALOG_IDS \
"filechooserdialog", \
NULL

#define FREQPLOTS_WINDOW_IDS \
"freqplots_zo_adjustment", \
"freqplots_window", \
"freqplots_box", \
"saveas_gnuplot", \
"freqplots_nec2_edit", \
"freqplots_fmhz_entry", \
"freqplots_maxgain_entry", \
"freqplots_vswr_entry", \
"freqplots_zreal_entry", \
"freqplots_zimag_entry", \
"freqplots_zo_spinbutton", \
"freqplots_drawingarea", \
"freqplots_label", \
"freqplots_net_gain", \
NULL

#define FREQUENCY_EDITOR_IDS \
"frequency_end_adjustment", \
"frequency_num_adjustment", \
"frequency_start_adjustment", \
"frequency_step_adjustment", \
"frequency_command", \
"frequency_add_radiobutton", \
"frequency_mul_radiobutton", \
"frequency_num_spinbutton", \
"frequency_start_spinbutton", \
"frequency_step_spinbutton", \
"frequency_end_spinbutton", \
"frequency_new_button", \
"frequency_cancel_button", \
"frequency_apply_button", \
"frequency_ok_button", \
NULL

#define GEND_EDITOR_IDS \
"gend_editor", \
"gend_nognd_radiobutton", \
"gend_img_radiobutton", \
"gend_noimg_radiobutton", \
"gend_cancel_button", \
"gend_apply_button", \
"gend_ok_button", \
NULL

#define GROUND2_EDITOR_IDS \
"ground2_below_adjustment", \
"ground2_diel_adjustment", \
"ground2_dist_adjustment", \
"ground2_res_adjustment", \
"ground2_command", \
"ground2_diel_spinbutton", \
"ground2_dist_spinbutton", \
"ground2_res_spinbutton", \
"ground2_below_spinbutton", \
"ground2_new_button", \
"ground2_cancel_button", \
"ground2_apply_button", \
"ground2_ok_button", \
NULL

#define GROUND_EDITOR_IDS \
"ground_below_adjustment", \
"ground_diam_adjustment", \
"ground_diel1_adjustment", \
"ground_diel2_adjustment", \
"ground_dist2_adjustment", \
"ground_nrad_adjustment", \
"ground_res1_adjustment", \
"ground_res2_adjustment", \
"ground_scrnrd_adjustment", \
"ground_command", \
"ground_perf_radiobutton", \
"ground_smfld_radiobutton", \
"ground_radl_checkbutton", \
"ground_refl_radiobutton", \
"ground_null_radiobutton", \
"ground_secmd_checkbutton", \
"ground_med1_frame", \
"ground_diel1_spinbutton", \
"ground_res1_spinbutton", \
"ground_med2_frame", \
"ground_diel2_spinbutton", \
"ground_dist2_spinbutton", \
"ground_res2_spinbutton", \
"ground_below_spinbutton", \
"ground_radial_frame", \
"ground_scrnrd_spinbutton", \
"ground_nrad_spinbutton", \
"ground_diam_spinbutton", \
"ground_new_button", \
"ground_cancel_button", \
"ground_apply_button", \
"ground_ok_button", \
NULL

#define HELIX_EDITOR_IDS \
"helix_dia_adjustment", \
"helix_len_adjustment", \
"helix_nturns_adjustment", \
"helix_numseg_adjustment", \
"helix_pcl_adjustment", \
"helix_radxzhl_adjustment", \
"helix_radxzo_adjustment", \
"helix_radyzhl_adjustment", \
"helix_radyzo_adjustment", \
"helix_res_adjustment", \
"helix_tagnum_adjustment", \
"helix_tspace_adjustment", \
"helix_editor", \
"helix_right_hand_radiobutton", \
"helix_left_hand_radiobutton", \
"spiral_right_hand_radiobutton", \
"spiral_left_hand_radiobutton", \
"helix_tagnum_spinbutton", \
"helix_tspace_spinbutton", \
"helix_radxzo_spinbutton", \
"helix_numseg_spinbutton", \
"helix_len_spinbutton", \
"helix_radyzo_spinbutton", \
"helix_pcl_spinbutton", \
"helix_dia_spinbutton", \
"helix_radxzhl_spinbutton", \
"helix_nturns_spinbutton", \
"helix_grid", \
"helix_link_a1b1", \
"helix_link_b1a2", \
"helix_link_a2b2", \
"helix_res_spinbutton", \
"helix_radyzhl_spinbutton", \
"helix_new_button", \
"helix_cancel_button", \
"helix_apply_button", \
"helix_ok_button", \
NULL

#define INTRANGE_EDITOR_IDS \
"intrange_wlen_adjustment", \
"intrange_command", \
"intrange_wlen_spinbutton", \
"intrange_new_button", \
"intrange_cancel_button", \
"intrange_apply_button", \
"intrange_ok_button", \
NULL

#define ZO_EDITOR_IDS \
"zo_adjustment", \
"zo_command", \
"zo_wlen_spinbutton", \
"zo_new_button", \
"zo_cancel_button", \
"zo_apply_button", \
"zo_ok_button", \
NULL

#define KERNEL_EDITOR_IDS \
"kernel_command", \
"kernel_checkbutton", \
"kernel_new_button", \
"kernel_cancel_button", \
"kernel_apply_button", \
"kernel_ok_button", \
NULL

#define LOADING_EDITOR_IDS \
"loading_f1_adjustment", \
"loading_f2_adjustment", \
"loading_f3_adjustment", \
"loading_i2_adjustment", \
"loading_i3_adjustment", \
"loading_i4_adjustment", \
"loading_command", \
"loading_null_radiobutton", \
"loading_slrlc_radiobutton", \
"loading_pdrlc_radiobutton", \
"loading_plrlc_radiobutton", \
"loading_rr_radiobutton", \
"loading_sdrlc_radiobutton", \
"loading_wcon_radiobutton", \
"loading_frame", \
"loading_i2_label", \
"loading_i2_spinbutton", \
"loading_f1_label", \
"loading_f1_spinbutton", \
"loading_i3_label", \
"loading_i3_spinbutton", \
"loading_f2_label", \
"loading_f2_spinbutton", \
"loading_i4_label", \
"loading_i4_spinbutton", \
"loading_f3_label", \
"loading_f3_spinbutton", \
"loading_new_button", \
"loading_cancel_button", \
"loading_apply_button", \
"loading_ok_button", \
NULL

#define NEARFIELD_EDITOR_IDS \
"nearfield_f1_adjustment", \
"nearfield_f2_adjustment", \
"nearfield_f3_adjustment", \
"nearfield_f4_adjustment", \
"nearfield_f5_adjustment", \
"nearfield_f6_adjustment", \
"nearfield_nx_adjustment", \
"nearfield_ny_adjustment", \
"nearfield_nz_adjustment", \
"nearfield_command", \
"nearfield_ne_checkbutton", \
"nearfield_rect_radiobutton", \
"nearfield_nh_checkbutton", \
"nearfield_sph_radiobutton", \
"nearfield_i1_label", \
"nearfield_nx_spinbutton", \
"nearfield_f1_label", \
"nearfield_f1_spinbutton", \
"nearfield_f4_label", \
"nearfield_f4_spinbutton", \
"nearfield_i2_label", \
"nearfield_ny_spinbutton", \
"nearfield_f2_label", \
"nearfield_f2_spinbutton", \
"nearfield_f5_label", \
"nearfield_f5_spinbutton", \
"nearfield_i3_label", \
"nearfield_nz_spinbutton", \
"nearfield_f3_label", \
"nearfield_f3_spinbutton", \
"nearfield_f6_label", \
"nearfield_f6_spinbutton", \
"nearfield_new_button", \
"nearfield_cancel_button", \
"nearfield_apply_button", \
"nearfield_ok_button", \
NULL

#define NEC2_EDITOR_IDS \
"nec2_editor", \
"nec2_row_add", \
"nec2_row_remv", \
"nec2_treeview_clear", \
"nec2_save_as", \
"nec2_save", \
"nec2_apply_checkbutton", \
"nec2_revert", \
"cmnt_scrolledwindow", \
"nec2_cmnt_treeview", \
"gw", \
"ga", \
"gh", \
"sp", \
"gr", \
"gm", \
"gx", \
"gs", \
"geom_scrolledwindow", \
"nec2_geom_treeview", \
"ex", \
"rp", \
"fr", \
"ne", \
"gn", \
"gd", \
"ld", \
"ek", \
"nt", \
"kh", \
"tl", \
"xq", \
"cmnd_scrolledwindow", \
"nec2_cmnd_treeview", \
NULL

#define NETWORK_EDITOR_IDS \
"network_f1_adjustment", \
"network_f2_adjustment", \
"network_f3_adjustment", \
"network_f4_adjustment", \
"network_f5_adjustment", \
"network_f6_adjustment", \
"network_i1_adjustment", \
"network_i2_adjustment", \
"network_i3_adjustment", \
"network_i4_adjustment", \
"network_command", \
"network_i1_spinbutton", \
"network_i3_spinbutton", \
"network_i2_spinbutton", \
"network_i4_spinbutton", \
"network_f1_spinbutton", \
"network_f3_spinbutton", \
"network_f5_spinbutton", \
"network_f2_spinbutton", \
"network_f4_spinbutton", \
"network_f6_spinbutton", \
"network_new_button", \
"network_cancel_button", \
"network_apply_button", \
"network_ok_button", \
NULL

#define PATCH_EDITOR_IDS \
"patch_nx_adjustment", \
"patch_ny_adjustment", \
"patch_x1_adjustment", \
"patch_x2_adjustment", \
"patch_x3_adjustment", \
"patch_x4_adjustment", \
"patch_y1_adjustment", \
"patch_y2_adjustment", \
"patch_y3_adjustment", \
"patch_y4_adjustment", \
"patch_z1_adjustment", \
"patch_z2_adjustment", \
"patch_z3_adjustment", \
"patch_z4_adjustment", \
"patch_editor", \
"patch_type_frame", \
"patch_arbitrary_radiobutton", \
"patch_rectangular_radiobutton", \
"patch_triangular_radiobutton", \
"patch_quadrilateral_radiobutton", \
"patch_surface_radiobutton", \
"patch_sm_frame", \
"patch_nx_spinbutton", \
"patch_ny_spinbutton", \
"patch_x1_label", \
"patch_x1_spinbutton", \
"patch_x2_label", \
"patch_x2_spinbutton", \
"patch_y1_label", \
"patch_y1_spinbutton", \
"patch_y2_label", \
"patch_y2_spinbutton", \
"patch_z1_label", \
"patch_z1_spinbutton", \
"patch_z2_label", \
"patch_z2_spinbutton", \
"patch_sc_frame", \
"patch_sc_grid", \
"patch_x3_spinbutton", \
"patch_y3_spinbutton", \
"patch_z3_spinbutton", \
"patch_x4_spinbutton", \
"patch_y4_spinbutton", \
"patch_z4_spinbutton", \
"patch_new_button", \
"patch_cancel_button", \
"patch_apply_button", \
"patch_ok_button", \
NULL

#define QUIT_DIALOG_IDS \
"quit_dialog", \
"quit_cancelbutton", \
"quit_okbutton", \
"quit_label", \
NULL

#define RADIATION_EDITOR_IDS \
"radiation_f1_adjustment", \
"radiation_f2_adjustment", \
"radiation_f3_adjustment", \
"radiation_f4_adjustment", \
"radiation_f5_adjustment", \
"radiation_f6_adjustment", \
"radiation_i2_adjustment", \
"radiation_i3_adjustment", \
"radiation_command", \
"radiation_i10_radiobutton", \
"radiation_i11_radiobutton", \
"radiation_i12_radiobutton", \
"radiation_i14_radiobutton", \
"radiation_i16_radiobutton", \
"radiation_i13_radiobutton", \
"radiation_i15_radiobutton", \
"radiation_x0_radiobutton", \
"radiation_x1_radiobutton", \
"radiation_n1_radiobutton", \
"radiation_n3_radiobutton", \
"radiation_n5_radiobutton", \
"radiation_d1_radiobutton", \
"radiation_n0_radiobutton", \
"radiation_n2_radiobutton", \
"radiation_n4_radiobutton", \
"radiation_d0_radiobutton", \
"radiation_a0_radiobutton", \
"radiation_a2_radiobutton", \
"radiation_a1_radiobutton", \
"radiation_f1_label", \
"radiation_f3_label", \
"radiation_i2_spinbutton", \
"radiation_f1_spinbutton", \
"radiation_f3_spinbutton", \
"radiation_i1_label", \
"radiation_i3_spinbutton", \
"radiation_f2_spinbutton", \
"radiation_f4_spinbutton", \
"radiation_f5_label", \
"radiation_f5_spinbutton", \
"radiation_f6_label", \
"radiation_f6_spinbutton", \
"radiation_new_button", \
"radiation_cancel_button", \
"radiation_apply_button", \
"radiation_ok_button", \
NULL

#define RDPATTERN_WINDOW_IDS \
"rdpattern_freq_adjustment", \
"rdpattern_incline_adjustment", \
"rdpattern_rotate_adjustment", \
"rdpattern_zoom_adjustment", \
"rdpattern_window", \
"rdpattern_box", \
"menuitem9_menu", \
"rdpattern_new", \
"rdpattern_open_input", \
"rdpattern_nec2_edit", \
"rdpattern_save", \
"rdpattern_save_as", \
"rdpattern_save_as_gnuplot", \
"rdpattern_quit", \
"rdpattern_view_menuitem", \
"rdpattern_view_menuitem_menu", \
"rdpattern_pol_menu", \
"rdpattern_pol_menu_menu", \
"rdpattern_total", \
"rdpattern_horizontal", \
"rdpattern_vertical", \
"rdpattern_right_hand", \
"rdpattern_left_hand", \
"rdpattern_gain_scaling", \
"rdpattern_gain_scaling_menu", \
"rdpattern_linear_power", \
"rdpattern_linear_voltage", \
"rdpattern_arrl_style", \
"rdpattern_logarithmic", \
"near_field", \
"near_field_menu", \
"rdpattern_e_field", \
"rdpattern_h_field", \
"rdpattern_poynting_vector", \
"rdpattern_animate", \
"total_field", \
"total_field_menu", \
"near_peak_value", \
"near_snapshot", \
"rdpattern_common_projection", \
"rdpattern_common_frequency", \
"rdpattern_overlay_structure", \
"rdpattern_gain_togglebutton", \
"rdpattern_eh_togglebutton", \
"rdpatttern_loop_start", \
"rdpattern_loop_pause", \
"rdpattern_loop_reset", \
"rdpattern_fstep_entry", \
"rdpattern_colorcode_minlabel", \
"rdpattern_colorcode_drawingarea", \
"rdpattern_colorcode_maxlabel", \
"rdpattern_viewer_gain", \
"rdpattern_freq_spinbutton", \
"rdpattern_freq_label", \
"rdpattern_freq_checkbutton", \
"rdpattern_new_freq_button", \
"rdpattern_x_axis", \
"rdpattern_y_axis", \
"rdpattern_z_axis", \
"rdpattern_default_view", \
"rdpattern_rotate_spinbutton", \
"rdpattern_incline_spinbutton", \
"rdpattern_zoom_spinbutton", \
"rdpattern_plus_button", \
"rdpattern_minus_button", \
"rdpattern_one_button", \
"rdpattern_drawingarea", \
"rdpattern_label", \
"saveas_gnuplot", \
NULL

#define REFLECT_EDITOR_IDS \
"reflect_taginc_adjustment", \
"reflect_editor", \
"reflect_x_checkbutton", \
"reflect_z_checkbutton", \
"reflect_y_checkbutton", \
"reflect_taginc_spinbutton", \
"reflect_new_button", \
"reflect_cancel_button", \
"reflect_apply_button", \
"reflect_ok_button", \
NULL

#define SCALE_EDITOR_IDS \
"scale_factor_adjustment", \
"scale_from_adjustment", \
"scale_to_adjustment", \
"scale_editor", \
"scale_from_spinbutton", \
"scale_factor_spinbutton", \
"scale_to_spinbutton", \
"scale_new_button", \
"scale_cancel_button", \
"scale_apply_button", \
"scale_ok_button", \
NULL

#define MAIN_WINDOW_IDS \
"main_menu", \
"main_freq_adjustment", \
"main_incline_adjustment", \
"main_rotate_adjustment", \
"main_zoom_adjustment", \
"main_window", \
"menuitem1_menu", \
"main_new", \
"main_open_input", \
"nec2_edit", \
"main_save", \
"main_save_as", \
"struct_save_as_gnuplot", \
"confirm_quit", \
"main_quit", \
"main_view_menuitem", \
"main_view_menuitem_menu", \
"main_rdpattern", \
"main_freqplots", \
"main_pol_menu", \
"main_pol_menu_menu", \
"main_total", \
"main_horizontal", \
"main_vertical", \
"main_right_hand", \
"main_left_hand", \
"main_common_projection", \
"main_common_frequency", \
"menuitem4_menu", \
"about", \
"main_hbox", \
"main_hbox1", \
"main_currents_togglebutton", \
"main_charges_togglebutton", \
"main_loop_start", \
"main_loop_pause", \
"main_loop_reset", \
"structure_fstep_entry", \
"main_hbox2", \
"main_colorcode_zerolabel", \
"main_colorcode_drawingarea", \
"main_colorcode_maxlabel", \
"main_gain_entry", \
"main_freq_spinbutton", \
"main_freq_label", \
"main_freq_checkbutton", \
"main_new_freq_button", \
"main_grid1", \
"main_x_axis", \
"main_y_axis", \
"main_z-axis", \
"main_default_view", \
"main_rotate_spinbutton", \
"main_incline_spinbutton", \
"saveas_gnuplot", \
"main_zoom_spinbutton", \
"structure_plus_button", \
"structure_minus_button", \
"structure_one_button", \
"structure_drawingarea", \
"structure_frame", \
"struct_label", \
NULL

#define TRANSFORM_EDITOR_IDS \
"transform_mx_adjustment", \
"transform_my_adjustment", \
"transform_mz_adjustment", \
"transform_new_adjustment", \
"transform_rx_adjustment", \
"transform_ry_adjustment", \
"transform_rz_adjustment", \
"transform_start_adjustment", \
"transform_taginc_adjustment", \
"transform_editor", \
"transform_taginc_spinbutton", \
"transform_rx_spinbutton", \
"transform_mx_spinbutton", \
"transform_new_spinbutton", \
"transform_ry_spinbutton", \
"transform_my_spinbutton", \
"transform_start_spinbutton", \
"transform_rz_spinbutton", \
"transform_mz_spinbutton", \
"transform_new_button", \
"transform_cancel_button", \
"transform_apply_button", \
"transform_ok_button", \
NULL

#define TXLINE_EDITOR_IDS \
"txline_f1_adjustment", \
"txline_f2_adjustment", \
"txline_f3_adjustment", \
"txline_f4_adjustment", \
"txline_f5_adjustment", \
"txline_f6_adjustment", \
"txline_command", \
"txline_i1_spinbutton", \
"txline_i3_spinbutton", \
"txline_i2_spinbutton", \
"txline_i4_spinbutton", \
"txline_crossed_checkbutton", \
"txline_f1_spinbutton", \
"txline_f3_spinbutton", \
"txline_f5_spinbutton", \
"txline_f2_spinbutton", \
"txline_f4_spinbutton", \
"txline_f6_spinbutton", \
"txline_new_button", \
"txline_cancel_button", \
"txline_apply_button", \
"txline_ok_button", \
"txline_i1_adjustment", \
"txline_i2_adjustment", \
"txline_i3_adjustment", \
"txline_i4_adjustment", \
NULL

#define WIRE_EDITOR_IDS \
"wire_dia1_adjustment", \
"wire_dia_adjustment", \
"wire_dian_adjustment", \
"wire_len_adjustment", \
"wire_numseg_adjustment", \
"wire_pcl_adjustment", \
"wire_rdia_adjustment", \
"wire_res_adjustment", \
"wire_rlen_adjustment", \
"wire_tagnum_adjustment", \
"wire_x1_adjustment", \
"wire_x2_adjustment", \
"wire_y1_adjustment", \
"wire_y2_adjustment", \
"wire_z1_adjustment", \
"wire_z2_adjustment", \
"wire_editor", \
"wire_tagnum_spinbutton", \
"wire_x1_spinbutton", \
"wire_x2_spinbutton", \
"wire_numseg_spinbutton", \
"wire_y1_spinbutton", \
"wire_y2_spinbutton", \
"wire_pcl_spinbutton", \
"wire_z1_spinbutton", \
"wire_z2_spinbutton", \
"wire_dia_spinbutton", \
"wire_len_spinbutton", \
"wire_res_spinbutton", \
"wire_taperframe", \
"wire_taper_checkbutton", \
"wire_rlen_spinbutton", \
"wire_dia1_spinbutton", \
"wire_dian_spinbutton", \
"wire_rdia_spinbutton", \
"wire_new_button", \
"wire_cancel_button", \
"wire_apply_button", \
"wire_ok_button", \
NULL

#define SAVE_DIALOG_IDS \
"nec2_save_dialog",\
"nec2_save_button",\
"nec2_discard_button",\
"nec2_save_image",\
NULL

#endif
