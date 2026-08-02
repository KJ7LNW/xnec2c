// Plan which gallery images warm in the background and when they warm.

/** Identify the deck-source bundle among warm tasks. */
const GALLERY_DECK_TASK_KEY = Symbol();

/** Report whether the browser exposes the idle callback queue. */
const GALLERY_IDLE_SUPPORTED = "requestIdleCallback" in window;

/** Schedule cancellable work after page load when the browser becomes idle. */
function schedule_at_idle(callback) {
  let idle_request = null;

  /** Defer through the browser idle queue when that API is available. */
  function queue_idle_callback() {
    if (GALLERY_IDLE_SUPPORTED) {
      idle_request = window.requestIdleCallback(callback);
    } else {
      callback();
    }
  }

  if (document.readyState === "complete") {
    queue_idle_callback();
  } else {
    window.addEventListener("load", queue_idle_callback, { once: true });
  }

  /** Cancel either the pending load listener or idle request. */
  return function cancel_idle_callback() {
    window.removeEventListener("load", queue_idle_callback);
    window.cancelIdleCallback?.(idle_request);
    idle_request = null;
  };
}

/** Create the subsystem that plans and paces gallery background loads. */
function create_gallery_warm(config) {
  const manifest = config.manifest;
  const state = config.state;
  const view = config.view;
  const gallery_data = config.gallery_data;
  const request_decks = config.request_decks;
  const queue = window.XNEC2C_GALLERY_WARM_QUEUE.create_gallery_warm_queue();
  const in_flight = new Map();
  const visible_indexes = new Set();

  /** Derive one example's source for a single axis. */
  function axis_source(entry, axis) {
    return gallery_data.gallery_image_source(gallery_data.find_gallery_image(entry, axis));
  }

  /** Derive one example's sources for every axis except the active one. */
  function other_axis_sources(entry, axis) {
    const active = gallery_data.find_gallery_image(entry, axis);
    return entry.images
      .filter(function skip_active(image) {
        return image !== active;
      })
      .map(gallery_data.gallery_image_source);
  }

  /** Create a task that loads one image source into the browser cache. */
  function image_task(source) {
    return {
      key: source,
      start: function start_image_warm(settle) {
        const warm_image = new Image();

        /** Release the in-flight reference and report the request outcome. */
        function settle_warm(event) {
          in_flight.delete(source);
          if (event.type === "error") {
            console.debug("Gallery prefetch failed:", source);
            settle(queue.result.failed);
          } else if (event.type === "load") {
            console.debug("Gallery prefetch cached:", source);
            settle(queue.result.loaded);
          } else {
            throw new Error("Unexpected gallery prefetch event: " + event.type);
          }
        }

        warm_image.decoding = "async";
        warm_image.fetchPriority = "low";
        warm_image.addEventListener("load", settle_warm, { once: true });
        warm_image.addEventListener("error", settle_warm, { once: true });
        in_flight.set(source, warm_image);
        warm_image.src = source;
      },
    };
  }

  /** Create a task that requests the deck-source bundle. */
  function deck_task() {
    return {
      key: GALLERY_DECK_TASK_KEY,
      start: function start_deck_warm(settle) {
        request_decks();
        settle(queue.result.loaded);
      },
    };
  }

  /** Rebuild warming around the current selection and visible rows. */
  function refresh() {
    const count = manifest.examples.length;
    const entry = manifest.examples[state.index];
    const axis_tasks = other_axis_sources(entry, state.axis).map(image_task);
    axis_tasks.push(deck_task());
    queue.mark_warmed(axis_source(entry, state.axis));
    queue.set_plan({
      selected: [],
      neighbor: [state.index - 1, state.index + 1]
        .map(function create_neighbor_task(neighbor_index) {
          const neighbor = manifest.examples[(neighbor_index + count) % count];
          return image_task(axis_source(neighbor, state.axis));
        }),
      axis: axis_tasks,
      visible: Array.from(visible_indexes)
        .map(function create_visible_task(row_index) {
          return image_task(axis_source(manifest.examples[row_index], state.axis));
        }),
    });
  }

  /** Track which browse rows are on screen and requeue their images. */
  function handle_row_visibility(entries) {
    entries.forEach(function record_row(row) {
      const row_index = Number(row.target.dataset.index);
      if (row.isIntersecting) {
        visible_indexes.add(row_index);
      } else {
        visible_indexes.delete(row_index);
      }
    });
    refresh();
  }

  const row_observer = new IntersectionObserver(handle_row_visibility, { root: view.pane });

  /** Watch browse rows while the gallery owns the screen. */
  function observe() {
    view.pane_controls.forEach(function observe_row(control) {
      row_observer.observe(control.button);
    });
  }

  /** Stop watching rows and drop pending background work. */
  function release() {
    row_observer.disconnect();
    visible_indexes.clear();
    queue.stop();
  }

  /** Warm an unopened example's current-axis image before its first open. */
  function warm_example(index) {
    queue.append_selected(image_task(axis_source(manifest.examples[index], state.axis)));
  }

  return {
    refresh,
    defer: queue.schedule,
    observe,
    release,
    warm_example,
  };
}

window.XNEC2C_GALLERY_WARM = { create_gallery_warm, schedule_at_idle };
