// Bind gallery input interactions to controller operations.

const GALLERY_KEY_BINDINGS = Object.freeze([
  {
    key: "ArrowLeft",
    label: "Left Arrow",
    purpose: "Show the previous antenna example",
    control_names: ["previous_button", "previous_chevron"],
    action: function previous(controller) { controller.navigate(-1); },
  },
  {
    key: "ArrowRight",
    label: "Right Arrow",
    purpose: "Show the next antenna example",
    control_names: ["next_button", "next_chevron"],
    action: function next(controller) { controller.navigate(1); },
  },
  {
    key: "s",
    label: "S",
    purpose: "Show or hide the selected NEC source deck",
    control_names: ["source_button"],
    action: function toggle_source(controller) { controller.toggle_source(); },
  },
  {
    key: "b",
    label: "B",
    purpose: "Show or hide the antenna example list",
    control_names: ["browse_button"],
    action: function toggle_pane(controller) { controller.toggle_pane(); },
  },
  {
    key: "Escape",
    label: "Escape",
    purpose: "Close the active gallery surface",
    control_names: ["close_button", "zoom_close"],
    action: function close_surface(controller) { controller.close_deepest_surface(); },
  },
]);

/** Trigger navigation when a horizontal swipe exceeds this pixel distance. */
const GALLERY_SWIPE_THRESHOLD = 40;

/** Resolve hotkey bindings against their rendered controls. */
function resolve_gallery_key_bindings(view) {
  const static_bindings = GALLERY_KEY_BINDINGS.map(function resolve_static_binding(binding) {
    const { control_names, ...binding_fields } = binding;
    return {
      ...binding_fields,
      buttons: control_names.map(function resolve_button(control_name) {
        return view[control_name];
      }),
    };
  });
  const axis_bindings = view.axis_controls.map(function resolve_axis_binding(control) {
    const axis = control.axis;
    return {
      key: axis.shortcut,
      label: axis.shortcut.toUpperCase(),
      purpose: axis.purpose,
      buttons: [control.button],
      action: function select_axis(controller) { controller.select_axis(axis.key); },
    };
  });
  return static_bindings.concat(axis_bindings);
}

/** Bind measured hotkey tooltips to their controls. */
function bind_hotkey_tooltips(bindings, tooltip) {
  let focused_target = null;
  let hovered_target = null;

  /** Render the currently active hotkey tooltip above its control. */
  function render_hotkey_tooltip() {
    const target = hovered_target === null ? focused_target : hovered_target;
    if (target === null) {
      tooltip.classList.remove("gallery-hotkey-tooltip-open");
      return;
    }

    tooltip.textContent = target.binding.purpose + "\nHotkey: " + target.binding.label;
    const button_rect = target.button.getBoundingClientRect();
    const tooltip_rect = tooltip.getBoundingClientRect();
    const half_width = tooltip_rect.width / 2;
    const center = button_rect.left + (button_rect.width / 2);
    const left = Math.max(
      half_width + 8,
      Math.min(window.innerWidth - half_width - 8, center),
    );
    const top = Math.max(tooltip_rect.height + 8, button_rect.top - 6);
    tooltip.style.left = String(left) + "px";
    tooltip.style.top = String(top) + "px";
    tooltip.classList.add("gallery-hotkey-tooltip-open");
  }

  bindings.forEach(function bind_hotkey(binding) {
    binding.buttons.forEach(function bind_hotkey_button(button) {
      const target = { button, binding };
      button.addEventListener("mouseenter", function show_hovered_tooltip() {
        hovered_target = target;
        render_hotkey_tooltip();
      });
      button.addEventListener("mouseleave", function hide_hovered_tooltip() {
        if (hovered_target !== target) {
          return;
        }
        hovered_target = null;
        render_hotkey_tooltip();
      });
      button.addEventListener("focus", function show_focused_tooltip() {
        focused_target = target;
        render_hotkey_tooltip();
      });
      button.addEventListener("blur", function hide_focused_tooltip() {
        if (focused_target !== target) {
          return;
        }
        focused_target = null;
        render_hotkey_tooltip();
      });
      button.addEventListener("click", function hide_activated_tooltip() {
        focused_target = null;
        hovered_target = null;
        render_hotkey_tooltip();
      });
    });
  });
  window.addEventListener("resize", render_hotkey_tooltip);
}

/** Bind gallery controls to controller operations. */
function bind_gallery_events(view, controller) {
  const key_bindings = resolve_gallery_key_bindings(view);
  const key_actions = new Map(key_bindings.map(function map_key_action(binding) {
    return [binding.key, binding.action];
  }));
  bind_hotkey_tooltips(key_bindings, view.hotkey_tooltip);

  view.previous_button.addEventListener("click", function previous() { controller.navigate(-1); });
  view.next_button.addEventListener("click", function next() { controller.navigate(1); });
  view.previous_chevron.addEventListener("click", function previous() { controller.navigate(-1); });
  view.next_chevron.addEventListener("click", function next() { controller.navigate(1); });
  view.axis_controls.forEach(function bind_axis(control) {
    const axis = control.axis;
    control.button.addEventListener("click", function select() { controller.select_axis(axis.key); });
    control.button.addEventListener("mouseenter", function preview() {
      controller.set_axis_preview(axis.key);
    });
  });
  view.axis_bar.addEventListener("mouseleave", function clear_preview() {
    controller.set_axis_preview(null);
  });
  view.pane_controls.forEach(function bind_row(control) {
    control.button.addEventListener("click", function select() {
      controller.select_example(control.index);
    });
    control.button.addEventListener("mouseenter", function preview() {
      controller.set_example_preview(control.index);
    });
  });
  view.pane.addEventListener("mouseleave", function clear_preview() {
    controller.set_example_preview(null);
  });
  view.browse_button.addEventListener("click", controller.toggle_pane);
  view.source_button.addEventListener("click", controller.toggle_source);
  view.close_button.addEventListener("click", controller.close_gallery);
  view.image_button.addEventListener("click", controller.open_zoom);
  view.zoom_close.addEventListener("click", controller.close_zoom);
  view.modal.addEventListener("click", function close_backdrop(event) {
    if (event.target !== view.modal) {
      return;
    }
    controller.close_gallery();
  });
  view.zoom.addEventListener("click", function close_backdrop(event) {
    if (event.target !== view.zoom) {
      return;
    }
    controller.close_zoom();
  });

  /** Navigate images from wheel gestures. */
  function handle_wheel(event) {
    if (!controller.process_wheel(event.deltaX, event.deltaY)) {
      return;
    }
    event.preventDefault();
  }

  /** Keep keyboard focus inside the active modal surface. */
  function trap_focus(event) {
    if (event.key !== "Tab") {
      return false;
    }
    const surface = controller.get_focus_surface();
    const controls = Array.from(surface.querySelectorAll("button:not(:disabled), [tabindex='0']"));
    const first = controls[0];
    const last = controls[controls.length - 1];
    const at_edge = event.shiftKey
      ? document.activeElement === first
      : document.activeElement === last;
    if (!at_edge) {
      return true;
    }
    const target = event.shiftKey ? last : first;
    target.focus();
    event.preventDefault();
    return true;
  }

  /** Handle gallery keyboard commands. */
  function handle_key(event) {
    if (trap_focus(event)) {
      return;
    }
    const action = key_actions.get(event.key);
    if (action === undefined) {
      return;
    }
    action(controller);
    event.preventDefault();
  }

  /** Navigate images from a horizontal swipe on a gallery surface. */
  function bind_swipe(surface) {
    let start_x = null;
    let start_y = null;
    surface.addEventListener("touchstart", function begin_swipe(event) {
      const touch = event.changedTouches[0];
      start_x = touch.clientX;
      start_y = touch.clientY;
    }, { passive: true });
    surface.addEventListener("touchend", function end_swipe(event) {
      if (start_x === null) {
        return;
      }
      const touch = event.changedTouches[0];
      const delta_x = touch.clientX - start_x;
      const delta_y = touch.clientY - start_y;
      start_x = null;
      start_y = null;
      if (Math.abs(delta_x) < GALLERY_SWIPE_THRESHOLD || Math.abs(delta_x) <= Math.abs(delta_y)) {
        return;
      }
      const direction = delta_x < 0 ? 1 : -1;
      controller.navigate(direction);
    }, { passive: true });
  }

  view.stage.addEventListener("wheel", handle_wheel, { passive: false });
  view.zoom.addEventListener("wheel", handle_wheel, { passive: false });
  bind_swipe(view.stage);
  bind_swipe(view.zoom);
  return { handle_key };
}

window.XNEC2C_GALLERY_EVENTS = { bind_gallery_events };
