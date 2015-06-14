#include <gtk/gtk.h>


void
on_main_window_destroy                 (GtkObject       *object,
                                        gpointer         user_data);

gboolean
on_main_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_new_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open_input_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_nec2_edit_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_main_save_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_main_save_as_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
main_view_menuitem_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_main_rdpattern_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_main_freqplots_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
main_pol_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_total_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_horizontal_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_vertical_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_right_hand_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_left_hand_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_common_projection_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_common_freq_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_main_currents_togglebutton_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_main_charges_togglebutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_loop_start_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_loop_pause_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_loop_reset_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_main_colorcode_drawingarea_expose_event
                                        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
on_main_freq_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_main_freq_checkbutton_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_main_new_freq_clicked               (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_structure_drawingarea_configure_event
                                        (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

gboolean
on_structure_drawingarea_expose_event  (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_structure_drawingarea_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_structure_drawingarea_motion_notify_event( GtkWidget       *widget,
                                              GdkEventMotion  *event,
                                              gpointer         user_data);

void
on_main_x_axis_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_main_y_axis_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_main_z_axis_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_main_default_view_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_main_rotate_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_main_incline_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_structure_zoom_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_structure_plus_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_structure_minus_button_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_structure_one_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_filechooserdialog_response          (GtkDialog       *dialog,
                                        gint             response_id,
                                        gpointer         user_data);

void
on_filechoser_cancel_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_fileselection_response              (GtkDialog       *dialog,
                                        gint             response_id,
                                        gpointer         user_data);

void
on_fileselection_cancel_clicked        (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_freqplots_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_freqplots_window_destroy            (GtkObject       *object,
                                        gpointer         user_data);

void
on_freqplots_save_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_freqplots_save_as_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
freqplots_pol_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_net_gain_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_freqplots_gmax_togglebutton_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_freqplots_gdir_togglebutton_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_freqplots_gviewer_togglebutton_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_freqplots_vswr_togglebutton_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_freqplots_zrlzim_togglebutton_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_freqplots_zmgzph_togglebutton_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_freqplots_zo_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

gboolean
on_freqplots_drawingarea_expose_event  (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_freqplots_drawingarea_configure_event
                                        (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

gboolean
on_freqplots_drawingarea_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_rdpattern_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_rdpattern_window_destroy            (GtkObject       *object,
                                        gpointer         user_data);

void
on_rdpattern_save_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_save_as_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
rdpattern_view_menuitem_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
rdpattern_pol_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_linear_power_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_linear_voltage_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_arrl_style_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_logarithmic_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_e_field_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_h_field_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_poynting_vector_activate  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_animate_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_near_peak_value_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_near_snapshot_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_gain_togglebutton_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_rdpattern_eh_togglebutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

gboolean
on_rdpattern_colorcode_drawingarea_expose_event
                                        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
on_rdpattern_freq_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_rdpattern_new_freq_clicked          (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_rdpattern_drawingarea_configure_event
                                        (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

gboolean
on_rdpattern_drawingarea_expose_event  (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_rdpattern_drawingarea_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_rdpattern_drawingarea_motion_notify_event( GtkWidget       *widget,
                                              GdkEventMotion  *event,
                                              gpointer         user_data);

void
on_rdpattern_x_axis_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_rdpattern_y_axis_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_rdpattern_z_axis_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_rdpattern_default_view_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_rdpattern_rotate_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_rdpattern_incline_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_rdpattern_zoom_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_rdpattern_plus_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_rdpattern_minus_button_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_rdpattern_one_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_quit_dialog_destroy                 (GtkObject       *object,
                                        gpointer         user_data);

void
on_quit_cancelbutton_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_quit_okbutton_clicked               (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_error_dialog_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_error_quitbutton_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_error_okbutton_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_error_stopbutton_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_animate_dialog_destroy              (GtkObject       *object,
                                        gpointer         user_data);

void
on_animation_applybutton_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_animation_cancelbutton_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_animation_okbutton_clicked          (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_nec2_editor_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_nec2_editor_destroy                 (GtkObject       *object,
                                        gpointer         user_data);

void
on_nec2_row_add_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_nec2_row_remv_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_nec2_treeview_clear_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_nec2_save_as_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_nec2_save_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_nec2_revert_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_nec2_cmnt_treeview_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_gw_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_ga_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_gh_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_sp_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_gr_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_gm_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_gx_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_gs_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_nec2_geom_treeview_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_ex_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_fr_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_gn_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_rp_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_ld_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_nt_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_tl_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_ne_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_xq_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_kh_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_ek_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_gd_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_nec2_cmnd_treeview_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_wire_editor_destroy                 (GtkObject       *object,
                                        gpointer         user_data);

void
on_wire_tagnum_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_wire_data_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_wire_pcl_spinbutton_value_changed   (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_wire_len_spinbutton_value_changed   (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_wire_res_spinbutton_value_changed   (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_wire_taper_checkbutton_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_wire_rdia_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_wire_rlen_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_wire_new_button_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_wire_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_wire_apply_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_wire_ok_button_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_patch_editor_destroy                (GtkObject       *object,
                                        gpointer         user_data);

void
on_patch_arbitrary_radiobutton_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_patch_rectangular_radiobutton_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_patch_triangular_radiobutton_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_patch_quadrilateral_radiobutton_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_patch_surface_radiobutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_patch_data_spinbutton_value_changed (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_patch_new_button_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_patch_cancel_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_patch_apply_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_patch_ok_button_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_arc_editor_destroy                  (GtkObject       *object,
                                        gpointer         user_data);

void
on_arc_data_spinbutton_value_changed   (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_arc_tagnum_spinbutton_value_changed (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_arc_pcl_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_arc_res_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_arc_new_button_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_arc_cancel_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_arc_apply_button_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_arc_ok_button_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_transform_editor_destroy            (GtkObject       *object,
                                        gpointer         user_data);

void
on_transform_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_transform_new_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_transform_cancel_button_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_transform_apply_button_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_transform_ok_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_helix_editor_destroy                (GtkObject       *object,
                                        gpointer         user_data);

void
on_helix_tagnum_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_helix_data_spinbutton_value_changed (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_helix_pcl_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_helix_nturns_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_helix_res_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_helix_linkzo_radiobutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_helix_linkzhl_radiobutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_helix_linkall_radiobutton_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_helix_lh_checkbutton_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_helix_new_button_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_helix_cancel_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_helix_apply_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_helix_ok_button_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_reflect_editor_destroy              (GtkObject       *object,
                                        gpointer         user_data);

void
on_reflect_checkbutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_reflect_taginc_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_reflect_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_reflect_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_reflect_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_reflect_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_scale_editor_destroy                (GtkObject       *object,
                                        gpointer         user_data);

void
on_scale_spinbutton_value_changed      (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_scale_new_button_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_scale_cancel_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_scale_apply_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_scale_ok_button_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_cylinder_editor_destroy             (GtkObject       *object,
                                        gpointer         user_data);

void
on_cylinder_taginc_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_cylinder_total_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_cylinder_new_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_cylinder_cancel_button_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_cylinder_apply_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_cylinder_ok_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_kernel_command_destroy              (GtkObject       *object,
                                        gpointer         user_data);

void
on_kernel_checkbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_kernel_new_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_kernel_cancel_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_kernel_apply_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_kernel_ok_button_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_execute_command_destroy             (GtkObject       *object,
                                        gpointer         user_data);

void
on_execute_radiobutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_execute_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_execute_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_execute_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_execute_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_intrange_command_destroy            (GtkObject       *object,
                                        gpointer         user_data);

void
on_intrange_spinbutton_value_changed   (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_intrange_new_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_intrange_cancel_button_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_intrange_apply_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_intrange_ok_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_ground_command_destroy              (GtkObject       *object,
                                        gpointer         user_data);

void
on_ground_radiobutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_ground_checkbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_ground_spinbutton_value_changed     (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_ground_new_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_ground_cancel_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_ground_apply_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_ground_ok_button_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_nearfield_command_destroy           (GtkObject       *object,
                                        gpointer         user_data);

void
on_nearfield_nh_checkbutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_nearfield_ne_checkbutton_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_nearfield_radiobutton_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_nearfield_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_nearfield_new_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_nearfield_cancel_button_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_nearfield_apply_button_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_nearfield_ok_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiation_command_destroy           (GtkObject       *object,
                                        gpointer         user_data);

void
on_radiation_radiobutton_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiation_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_radiation_new_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiation_cancel_button_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiation_apply_button_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiation_ok_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_excitation_command_destroy          (GtkObject       *object,
                                        gpointer         user_data);

void
on_excitation_radiobutton_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_excitation_checkbutton_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_excitation_spinbutton_value_changed (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_excitation_new_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_excitation_cancel_button_clicked    (GtkButton       *button,
                                        gpointer         user_data);

void
on_excitation_apply_button_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_excitation_ok_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_frequency_command_destroy           (GtkObject       *object,
                                        gpointer         user_data);

void
on_frequency_radiobutton_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_frequency_spinbutton_value_changed  (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_frequency_step_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_frequency_new_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_frequency_cancel_button_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_frequency_apply_button_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_frequency_ok_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_loading_command_destroy             (GtkObject       *object,
                                        gpointer         user_data);

void
on_loading_radiobutton_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_loading_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_loading_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_loading_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_loading_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_loading_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_network_command_destroy             (GtkObject       *object,
                                        gpointer         user_data);

void
on_network_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_network_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_network_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_network_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_network_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_txline_command_destroy              (GtkObject       *object,
                                        gpointer         user_data);

void
on_txline_spinbutton_value_changed     (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_txline_checkbutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_txline_new_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_txline_cancel_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_txline_apply_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_txline_ok_button_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_ground2_command_destroy             (GtkObject       *object,
                                        gpointer         user_data);

void
on_ground2_spinbutton_value_changed    (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_ground2_new_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_ground2_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_ground2_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_ground2_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_gend_editor_destroy                 (GtkObject       *object,
                                        gpointer         user_data);

void
on_gend_radiobutton_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_gend_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_gend_apply_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_gend_ok_button_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_aboutdialog_close                   (GtkDialog       *dialog,
                                        gpointer         user_data);

void
on_aboutdialog_response                (GtkDialog       *dialog,
                                        gint             response_id,
                                        gpointer         user_data);

gboolean
on_structure_drawingarea_scroll_event  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_rdpattern_drawingarea_scroll_event  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_rdpattern_overlay_structure_activate
                                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rdpattern_save_as_gnuplot_activate  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_freqplots_save_as_gnuplot_activate  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_struct_save_as_gnuplot_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
