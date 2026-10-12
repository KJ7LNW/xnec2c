// Derive every gallery surface from the current gallery state.

const GALLERY_DECK_REQUEST_IDLE = Symbol();
const GALLERY_DECK_REQUESTED = Symbol();
const GALLERY_DECK_REQUEST_FAILED = Symbol();

/** Create the gallery renderer and lazy deck-source loader. */
function create_gallery_renderer(manifest, state, view, gallery_data) {
  let deck_request = GALLERY_DECK_REQUEST_IDLE;
  const stage_surface = { element: view.image, source: null };
  const zoom_surface = { element: view.zoom_image, source: null };

  /** Point a surface's image at a source when that source changes. */
  function render_surface_source(surface, source) {
    if (surface.source === source) {
      return;
    }
    surface.source = source;
    surface.element.src = source;
  }

  /** Load the full-size zoom source only while zoom is open. */
  function render_zoom_source(image) {
    if (!state.zoom_open) {
      return;
    }
    render_surface_source(zoom_surface, gallery_data.gallery_image_source(image));
  }

  /** Mark the zoom surface loading while its full-size image is in flight. */
  function render_zoom_loading() {
    view.zoom.classList.toggle("gallery-zoom-loading", !view.zoom_image.complete);
  }

  /** Repaint after the deck bundle loads. */
  function handle_deck_load() {
    deck_request = GALLERY_DECK_REQUESTED;
    render_gallery();
  }

  /** Report a failed deck bundle load. */
  function handle_deck_error() {
    deck_request = GALLERY_DECK_REQUEST_FAILED;
    render_gallery();
  }

  /** Request the deck bundle once. */
  function request_decks() {
    if (deck_request !== GALLERY_DECK_REQUEST_IDLE) {
      return;
    }
    deck_request = GALLERY_DECK_REQUESTED;
    const script = document.createElement("script");
    script.src = gallery_data.gallery_deck_bundle_source();
    script.addEventListener("load", handle_deck_load);
    script.addEventListener("error", handle_deck_error);
    document.head.appendChild(script);
  }

  /** Render the selected deck source. */
  function render_source(entry) {
    if (!state.source_open) {
      return;
    }
    if (deck_request === GALLERY_DECK_REQUEST_FAILED) {
      view.source_pre.textContent = "Deck source unavailable: " + entry.deck;
      return;
    }
    const decks = window.XNEC2C_DECKS;
    if (decks === undefined || decks === null) {
      view.source_pre.textContent = "Loading deck source…";
      request_decks();
      return;
    }
    const text = decks[entry.deck];
    if (text === undefined || text === null) {
      view.source_pre.textContent = "Deck source unavailable: " + entry.deck;
      return;
    }
    window.XNEC2C_GALLERY_NEC.render_nec_source(view.source_pre, text);
  }

  /** Render axis availability and selection. */
  function render_axes(entry, axis) {
    view.axis_controls.forEach(function render_axis(control) {
      const axis_row = control.axis;
      const button = control.button;
      const available = entry.images.some(function has_axis(image) {
        return image.format === axis_row.key;
      });
      button.classList.toggle("gallery-axis-active", axis_row.key === axis);
      button.setAttribute("aria-pressed", String(axis_row.key === axis));
      button.disabled = !available;
      if (available) {
        button.removeAttribute("title");
      } else {
        button.title = axis_row.purpose + ".\n"
          + "Available when this example includes that view; choose an enabled view.";
      }
    });
  }

  /** Render image, title, pane, and modal state. */
  function render_gallery() {
    const index = state.preview_index === null ? state.index : state.preview_index;
    const entry = manifest.examples[index];
    const requested_axis = state.preview_axis === null ? state.axis : state.preview_axis;
    const image = gallery_data.find_gallery_image(entry, requested_axis);
    const axis = image.format;
    const description = entry.name + " " + gallery_data.gallery_image_subject(image);

    render_axes(entry, axis);
    view.title.textContent = entry.name;
    render_surface_source(stage_surface, gallery_data.gallery_thumb_source(image));
    render_zoom_source(image);
    view.image.alt = description;
    view.zoom_image.alt = description;
    view.zoom_caption.textContent = entry.deck;
    view.pane_controls.forEach(function render_row(control) {
      const current = control.index === index;
      control.button.classList.toggle("gallery-pane-current", current);
      control.button.setAttribute("aria-current", current ? "true" : "false");
    });
    render_source(entry);

    view.pane.classList.toggle("gallery-pane-open", state.pane_open);
    view.source_panel.classList.toggle("gallery-source-open", state.source_open);
    view.source_button.classList.toggle("gallery-toggle-active", state.source_open);
    view.source_button.setAttribute("aria-expanded", String(state.source_open));
    view.browse_button.classList.toggle("gallery-toggle-active", state.pane_open);
    view.browse_button.setAttribute("aria-expanded", String(state.pane_open));
    view.zoom.classList.toggle("gallery-zoom-open", state.zoom_open);
    view.zoom.setAttribute("aria-hidden", String(!state.zoom_open));
    view.zoom_image.classList.toggle("gallery-zoom-magnified", state.zoom_magnified);
    render_zoom_loading();
  }

  view.zoom_image.addEventListener("load", render_zoom_loading);
  view.zoom_image.addEventListener("error", render_zoom_loading);
  return { render_gallery, request_decks };
}

window.XNEC2C_GALLERY_RENDER = { create_gallery_renderer };
