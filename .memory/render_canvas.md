# Canvas binding and frame requests

Apply each line when binding engines to canvases, requesting frames or widget repaints, or calling GTK draw functions.

## User-given

- gtk_widget_queue_draw is NOT thread safe so MUST ONLY call under GTK main thread: that is why the redraw scheduler in `src/render/render_redraw.c` marshals through `g_main_context_invoke_full()` at `GDK_PRIORITY_REDRAW`
- Canvases are the drawing surfaces: `CANVAS_STRUCTURE`, `CANVAS_RDPATTERN`, `CANVAS_FREQPLOTS`. Every engine able to draw into one registers its surface with `canvas_add_surface(id, widget, &engine_ops)` where that widget is created; `canvas_set_engine(id, &engine_ops)` presents that surface, hides the others and makes that engine the frame producer; `canvas_clear(id)` drops them at window close. The renderer swap names an engine and never a widget.
- `canvas_widget(id)` reports a bug when the canvas presents no surface; test `canvas_bound(id)` at sites reached while the window may be closed.
- Request a canvas frame with `canvas_queue_redraw(id, force)`. Request a repaint of a widget that no canvas presents with `xnec2_widget_queue_draw(w, force)`: colorcode legend strips, frequency-plot popups, the animate dialog. Both gate and marshal through `src/render/render_redraw.c`. Call `gtk_widget_queue_draw()` and `gtk_gl_area_queue_render()` only inside an engine `queue_redraw` leaf.
- Never infer the frame-producing engine from a widget's GTK class or from `rc_config.use_opengl_renderer`; read it from the canvas binding.
- A synchronous `gtk_widget_draw()` of a canvas is preceded by `canvas_invalidate()` in the same function; GL areas run with auto-render disabled and otherwise re-present a stale frame.
- Engine-internal frame requests (realize, resize, drag, notice fade, MSAA rebuild) call `gl_view_queue_render()` directly, below the suppression gate.
- freq_step_update_ui() is the single redraw orchestration path for all data/parameter mutation sites
