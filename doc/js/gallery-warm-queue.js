// Pace prioritized background tasks behind interactive browser work.

/** Pause between background tasks so interaction keeps request priority. */
const GALLERY_WARM_DELAY_MS = 300;

/** Bound automatic recovery from a failed background task. */
const GALLERY_WARM_RETRY_LIMIT = 1;

const GALLERY_WARM_BAND = Object.freeze({
  selected: "selected",
  neighbor: "neighbor",
  axis: "axis",
  visible: "visible",
});

/** Order the warm bands from highest to lowest priority. */
const GALLERY_WARM_BANDS = Object.freeze(Object.values(GALLERY_WARM_BAND));

const GALLERY_WARM_RESULT = Object.freeze({
  loaded: Symbol(),
  failed: Symbol(),
});

/** Create the paced queue that owns background task state. */
function create_gallery_warm_queue() {
  const bands = new Map(GALLERY_WARM_BANDS.map(function create_band(band) {
    return [band, []];
  }));
  const warmed_keys = new Set();
  const attempts = new Map();
  let delay_timer = null;
  let active_entry = null;
  let plan_generation = 0;

  /** Take the highest-priority task that has not already been warmed. */
  function take_task() {
    let taken = null;
    GALLERY_WARM_BANDS.some(function take_from_band(band) {
      const tasks = bands.get(band);
      while (taken === null && tasks.length > 0) {
        const candidate = tasks.shift();
        if (warmed_keys.has(candidate.key)) {
          continue;
        }
        console.debug("Gallery prefetch dequeued " + band + ":", candidate.key);
        taken = { task: candidate, band };
      }
      return taken !== null;
    });
    return taken;
  }

  /** Cancel the owned delay and clear its handle. */
  function cancel_delay() {
    window.clearTimeout(delay_timer);
    delay_timer = null;
  }

  /** Restart the delay so interactive work keeps request priority. */
  function schedule() {
    cancel_delay();
    delay_timer = active_entry === null
      ? window.setTimeout(run_next, GALLERY_WARM_DELAY_MS)
      : null;
  }

  /** Return one failed task to its band while budget and plan allow. */
  function retry_failed(taken, issued_generation) {
    const key = taken.task.key;
    const count = (attempts.get(key) ?? 0) + 1;
    attempts.set(key, count);
    if (count > GALLERY_WARM_RETRY_LIMIT) {
      console.debug("Gallery prefetch abandoned:", key);
    } else {
      warmed_keys.delete(key);
      if (issued_generation === plan_generation) {
        bands.get(taken.band).push(taken.task);
      } else {
        console.debug("Gallery prefetch deferred to next plan:", key);
      }
    }
  }

  /** Warm one task, then pace the next behind the delay. */
  function run_next() {
    delay_timer = null;
    const taken = take_task();
    if (taken === null) {
      active_entry = null;
      return;
    }
    const issued_generation = plan_generation;
    active_entry = taken;
    warmed_keys.add(taken.task.key);
    taken.task.start(function settle_task(result) {
      active_entry = null;
      if (result === GALLERY_WARM_RESULT.failed) {
        retry_failed(taken, issued_generation);
      } else if (result === GALLERY_WARM_RESULT.loaded) {
        attempts.delete(taken.task.key);
      } else {
        throw new Error("Unexpected gallery warm result: " + String(result));
      }
      schedule();
    });
  }

  /** Record one task key as already satisfied outside this queue. */
  function mark_warmed(key) {
    warmed_keys.add(key);
  }

  /** Replace every band with a new plan and pace its first task. */
  function set_plan(plan_bands) {
    plan_generation += 1;
    GALLERY_WARM_BANDS.forEach(function fill_band(band) {
      const tasks = plan_bands[band];
      if (tasks === undefined) {
        throw new Error("Gallery warm plan omits band: " + band);
      }
      bands.set(band, tasks);
    });
    schedule();
  }

  /** Append one unopened-selection task and pace it behind the delay. */
  function append_selected(task) {
    bands.get(GALLERY_WARM_BAND.selected).push(task);
    schedule();
  }

  /** Drop pending work, cancel the delay, and retire the current plan. */
  function stop() {
    cancel_delay();
    plan_generation += 1;
    GALLERY_WARM_BANDS.forEach(function clear_band(band) {
      bands.set(band, []);
    });
  }

  return {
    result: GALLERY_WARM_RESULT,
    mark_warmed,
    set_plan,
    append_selected,
    schedule,
    stop,
  };
}

window.XNEC2C_GALLERY_WARM_QUEUE = { create_gallery_warm_queue };
