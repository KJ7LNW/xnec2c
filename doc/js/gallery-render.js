// Derive every gallery surface from the current gallery state.

const GALLERY_DECK_REQUEST_IDLE = Symbol();
const GALLERY_DECK_REQUESTED = Symbol();
const GALLERY_DECK_REQUEST_FAILED = Symbol();

/** Create the gallery renderer and lazy deck-source loader. */
function create_gallery_renderer(manifest, state, view, gallery_data) {
  let deck_request = GALLERY_DECK_REQUEST_IDLE;

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
    script.src = "images/gallery/decks.js";
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
    const source = gallery_data.gallery_image_source(image);
    const description = entry.name + " " + axis + " radiation pattern";

    render_axes(entry, axis);
    view.title.textContent = entry.name;
    view.image.src = source;
    view.image.alt = description;
    view.zoom_image.src = source;
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
  }

  return { render_gallery };
}

window.XNEC2C_GALLERY_RENDER = { create_gallery_renderer };
