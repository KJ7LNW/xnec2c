// Control the static xnec2c radiation-pattern gallery.

const GALLERY_INITIAL_STATE = Object.freeze({
  index: 0,
  axis: "iso",
  pane_open: true,
  source_open: false,
  preview_index: null,
  preview_axis: null,
  zoom_open: false,
});

/** Collapse the example pane by default at or below this viewport width. */
const GALLERY_MOBILE_QUERY = "(max-width: 768px)";

const GALLERY_AXES = Object.freeze([
  { key: "x", label: "X", shortcut: "x" },
  { key: "y", label: "Y", shortcut: "y" },
  { key: "z", label: "Z", shortcut: "z" },
  { key: "iso", label: "ISO", shortcut: "i" },
].map(function create_gallery_axis(axis) {
  return Object.freeze({
    ...axis,
    get purpose() {
      return "Show the " + axis.label + "-axis radiation pattern";
    },
  });
}));

const GALLERY_DATA = Object.freeze({
  find_gallery_image,
  gallery_image_source,
});

/** Resolve the requested image or the example's first available image. */
function find_gallery_image(entry, requested_axis) {
  const image = entry.images.find(function find_axis(record) {
    return record.format === requested_axis;
  });
  return image === undefined ? entry.images[0] : image;
}

/** Derive a gallery image source from its manifest record. */
function gallery_image_source(image) {
  return "images/gallery/" + image.path;
}

/** Report whether the viewport is narrow enough to default the example pane collapsed. */
function gallery_is_mobile_viewport() {
  return window.matchMedia(GALLERY_MOBILE_QUERY).matches;
}

/** Build a gallery controller for a validated manifest. */
function build_gallery(manifest) {
  const state = Object.assign({}, GALLERY_INITIAL_STATE);
  const view = window.XNEC2C_GALLERY_VIEW.create_gallery_view(manifest, GALLERY_AXES);
  const renderer = window.XNEC2C_GALLERY_RENDER.create_gallery_renderer(
    manifest,
    state,
    view,
    GALLERY_DATA,
  );
  const warm = window.XNEC2C_GALLERY_WARM.create_gallery_warm({
    manifest,
    state,
    view,
    gallery_data: GALLERY_DATA,
    request_decks: renderer.request_decks,
  });
  let return_focus = null;

  /** Commit navigation to an adjacent example. */
  function navigate(delta) {
    const count = manifest.examples.length;
    state.index = (state.index + delta + count) % count;
    state.preview_index = null;
    state.preview_axis = null;
    renderer.render_gallery();
    warm.refresh();
  }

  /** Commit an axis selection. */
  function select_axis(axis) {
    state.axis = axis;
    state.preview_axis = null;
    state.source_open = false;
    renderer.render_gallery();
    warm.refresh();
  }

  /** Commit an example selection. */
  function select_example(index) {
    state.index = index;
    state.preview_index = null;
    state.preview_axis = null;
    renderer.render_gallery();
    warm.refresh();
  }

  /** Set the preview axis and repaint the gallery. */
  function set_axis_preview(axis) {
    state.preview_axis = axis;
    renderer.render_gallery();
    warm.defer();
  }

  /** Set the preview example and repaint the gallery. */
  function set_example_preview(index) {
    state.preview_index = index;
    renderer.render_gallery();
    warm.defer();
  }

  /** Cycle through the selected example's available axes. */
  function cycle_axis(delta) {
    const entry = manifest.examples[state.index];
    const keys = GALLERY_AXES
      .filter(function include_axis(axis) {
        return entry.images.some(function has_axis(image) {
          return image.format === axis.key;
        });
      })
      .map(function axis_key(axis) {
        return axis.key;
      });
    const current = find_gallery_image(entry, state.axis).format;
    const position = keys.indexOf(current);
    select_axis(keys[(position + delta + keys.length) % keys.length]);
  }

  /** Apply wheel navigation unless source view owns scrolling. */
  function process_wheel(delta_x, delta_y) {
    if (state.source_open) {
      return false;
    }
    if (Math.abs(delta_x) > Math.abs(delta_y)) {
      cycle_axis(delta_x > 0 ? 1 : -1);
    } else {
      navigate(delta_y > 0 ? 1 : -1);
    }
    return true;
  }

  /** Toggle the source panel. */
  function toggle_source() {
    state.source_open = !state.source_open;
    renderer.render_gallery();
  }

  /** Toggle the example pane. */
  function toggle_pane() {
    state.pane_open = !state.pane_open;
    renderer.render_gallery();
  }

  /** Open the zoom surface around the shared navigation controls. */
  function open_zoom() {
    state.zoom_open = true;
    view.zoom.insertBefore(view.previous_chevron, view.zoom_image);
    view.zoom.appendChild(view.next_chevron);
    renderer.render_gallery();
    view.zoom_close.focus();
  }

  /** Close the zoom surface and restore its controls and trigger focus. */
  function close_zoom() {
    state.zoom_open = false;
    view.stage.insertBefore(view.previous_chevron, view.image_button);
    view.stage.insertBefore(view.next_chevron, view.source_panel);
    renderer.render_gallery();
    view.image_button.focus();
  }

  /** Close the deepest open gallery surface. */
  function close_deepest_surface() {
    if (state.zoom_open) {
      close_zoom();
    } else if (state.source_open) {
      toggle_source();
    } else if (state.pane_open) {
      toggle_pane();
    } else {
      close_gallery();
    }
  }

  /** Return the gallery surface that currently owns keyboard focus. */
  function get_focus_surface() {
    return state.zoom_open ? view.zoom : view.dialog;
  }

  /** Open and initialize the gallery modal. */
  function open_gallery(start_index) {
    Object.assign(state, GALLERY_INITIAL_STATE);
    state.index = start_index;
    state.pane_open = !gallery_is_mobile_viewport();
    return_focus = document.activeElement;
    view.modal.classList.add("gallery-modal-open");
    view.modal.setAttribute("aria-hidden", "false");
    document.body.classList.add("gallery-open");
    document.addEventListener("keydown", events.handle_key);
    renderer.render_gallery();
    view.dialog.focus();
    warm.observe();
    warm.refresh();
  }

  /** Close the gallery modal and restore launcher focus. */
  function close_gallery() {
    view.modal.classList.remove("gallery-modal-open");
    view.modal.setAttribute("aria-hidden", "true");
    document.body.classList.remove("gallery-open");
    document.removeEventListener("keydown", events.handle_key);
    warm.release();
    return_focus?.focus();
    return_focus = null;
  }

  const events = window.XNEC2C_GALLERY_EVENTS.bind_gallery_events(view, {
    navigate, select_axis, select_example,
    set_axis_preview, set_example_preview,
    process_wheel,
    toggle_source, toggle_pane,
    open_zoom, close_zoom,
    close_deepest_surface, close_gallery,
    get_focus_surface,
  });
  return { open_gallery, warm_example: warm.warm_example };
}

window.XNEC2C_GALLERY_CONTROLLER = {
  build_gallery,
  default_index: GALLERY_INITIAL_STATE.index,
};
