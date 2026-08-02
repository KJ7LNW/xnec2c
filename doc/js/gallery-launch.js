// Mount the gallery launcher and route opener controls into the gallery.

/** Resolve the example index an opener control targets. */
function resolve_example_index(manifest, opener) {
  const name = opener.dataset.galleryExample;
  if (name === undefined) {
    return window.XNEC2C_GALLERY_CONTROLLER.default_index;
  }
  const index = manifest.examples.findIndex(function match_example(entry) {
    return entry.name === name;
  });
  if (index === -1) {
    throw new Error("Unknown gallery example: " + name);
  }
  return index;
}

/** Warm the presented example once the page reaches idle. */
function warm_presented_example(manifest, gallery) {
  const selected_opener = document.querySelector(
    "[data-gallery-open][data-gallery-example]",
  );
  return selected_opener === null
    ? null
    : window.XNEC2C_GALLERY_WARM.schedule_at_idle(function warm_selected_example() {
        gallery.warm_example(resolve_example_index(manifest, selected_opener));
      });
}

/** Mount the gallery launcher when its generated data is available. */
function mount_gallery() {
  const manifest = window.XNEC2C_GALLERY;
  if (manifest === undefined || manifest === null) {
    return;
  }
  if (!Array.isArray(manifest.examples) || manifest.examples.length === 0) {
    return;
  }
  const mount = document.getElementById("gallery-launch");
  if (mount === null) {
    return;
  }

  const gallery = window.XNEC2C_GALLERY_CONTROLLER.build_gallery(manifest);
  const launch = document.createElement("button");
  launch.type = "button";
  launch.className = "gallery-launch-button";
  launch.textContent = "Browse Antenna Gallery";
  launch.dataset.galleryOpen = "";
  mount.appendChild(launch);
  let cancel_idle_warm = warm_presented_example(manifest, gallery);

  document.addEventListener("click", function open_gallery_from_control(event) {
    const opener = event.target.closest("[data-gallery-open]");
    if (opener === null) {
      return;
    }
    event.preventDefault();
    cancel_idle_warm?.();
    cancel_idle_warm = null;
    gallery.open_gallery(resolve_example_index(manifest, opener));
  });
}

document.addEventListener("DOMContentLoaded", mount_gallery);
