// Construct the static gallery controls and expose their element references.

/** Build and mount the gallery user interface. */
function create_gallery_view(manifest, axes) {
  const modal = document.createElement("div");
  modal.className = "gallery-modal";
  modal.setAttribute("aria-hidden", "true");
  modal.innerHTML = `
    <div class="gallery-dialog" role="dialog" aria-modal="true" aria-labelledby="gallery-title" tabindex="-1">
      <div class="gallery-header">
        <button type="button" class="gallery-browse" aria-expanded="false">Browse</button>
        <span id="gallery-title" class="gallery-title"></span>
        <div class="gallery-axis-bar" role="group" aria-labelledby="gallery-axis-label">
          <span id="gallery-axis-label" class="gallery-axis-label">Select a view</span>
        </div>
        <button type="button" class="gallery-source-toggle" aria-expanded="false">.nec</button>
        <button type="button" class="gallery-close" aria-label="Close gallery">×</button>
      </div>
      <div class="gallery-body">
        <nav class="gallery-pane" aria-label="Antenna examples"><ul></ul></nav>
        <div class="gallery-stage">
          <button type="button" class="gallery-chevron gallery-chevron-left" aria-label="Previous example">‹</button>
          <button type="button" class="gallery-image-button" aria-label="Zoom radiation pattern">
            <img decoding="async" fetchpriority="high" class="gallery-image">
          </button>
          <button type="button" class="gallery-chevron gallery-chevron-right" aria-label="Next example">›</button>
          <div class="gallery-source"><pre></pre></div>
          <div class="gallery-nav" aria-label="Example navigation">
            <button type="button" class="gallery-prev">‹ Prev</button>
            <button type="button" class="gallery-next">Next ›</button>
          </div>
        </div>
      </div>
      <div class="gallery-zoom" role="dialog" aria-modal="true" aria-label="Radiation pattern zoom" aria-hidden="true">
        <button type="button" class="gallery-zoom-close" aria-label="Close zoom">×</button>
        <img decoding="async" class="gallery-zoom-image">
        <span class="gallery-zoom-caption"></span>
      </div>
    </div>
    <div class="gallery-hotkey-tooltip"></div>`;

  const axis_bar = modal.querySelector(".gallery-axis-bar");
  const axis_controls = axes.map(function append_axis(axis) {
    const button = document.createElement("button");
    button.type = "button";
    button.className = "gallery-axis";
    button.textContent = axis.label;
    button.dataset.axis = axis.key;
    axis_bar.appendChild(button);
    return { axis, button };
  });

  const pane_list = modal.querySelector(".gallery-pane ul");
  const pane_controls = manifest.examples.map(function append_example(entry, index) {
    const item = document.createElement("li");
    const button = document.createElement("button");
    button.type = "button";
    button.className = "gallery-pane-row";
    button.textContent = entry.name;
    button.dataset.index = String(index);
    item.appendChild(button);
    pane_list.appendChild(item);
    return { index, button };
  });

  document.body.appendChild(modal);
  return {
    modal,
    dialog: modal.querySelector(".gallery-dialog"),
    hotkey_tooltip: modal.querySelector(".gallery-hotkey-tooltip"),
    browse_button: modal.querySelector(".gallery-browse"),
    title: modal.querySelector(".gallery-title"),
    axis_bar,
    axis_controls,
    source_button: modal.querySelector(".gallery-source-toggle"),
    close_button: modal.querySelector(".gallery-close"),
    pane: modal.querySelector(".gallery-pane"),
    pane_controls,
    stage: modal.querySelector(".gallery-stage"),
    previous_button: modal.querySelector(".gallery-prev"),
    next_button: modal.querySelector(".gallery-next"),
    previous_chevron: modal.querySelector(".gallery-chevron-left"),
    next_chevron: modal.querySelector(".gallery-chevron-right"),
    image_button: modal.querySelector(".gallery-image-button"),
    image: modal.querySelector(".gallery-image"),
    source_panel: modal.querySelector(".gallery-source"),
    source_pre: modal.querySelector(".gallery-source pre"),
    zoom: modal.querySelector(".gallery-zoom"),
    zoom_close: modal.querySelector(".gallery-zoom-close"),
    zoom_image: modal.querySelector(".gallery-zoom-image"),
    zoom_caption: modal.querySelector(".gallery-zoom-caption"),
  };
}

window.XNEC2C_GALLERY_VIEW = { create_gallery_view };
