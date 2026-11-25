# Theme palette and gradient ladders

Apply each line when changing magnitude color producers or presentation caches, or authoring a theme gradient ladder.

## User-given

Magnitude color comes from `palette_get(PALETTE_RAMP)` in every producer (far field, near field, structure segments, legend bar); `Value_to_Color` is reserved for discrete COLORCODE trace identity. Every presentation cache gates on `presentation_cache_key_t.palette_gen` so a theme change re-colors without a manual redraw.

When authoring a `gradient_0..5` ladder in `resources/themes.ini`, lightness carries the reading and hue carries the distinction; keep them on separate axes:

- L\* must move monotonically across the six stops in even steps; a reversal makes two gain values identical in greyscale
- The extreme stop holds about 4:1 contrast against its own background so weak-signal regions stay visible
- No achromatic endpoints; a tinted near-white reads as brightest while holding hue to the end
- Maximize hue travel; under about 150° of the hue circle the bands stop separating
- Fit a stop into sRGB by shrinking chroma, never by moving L\*
- When an endpoint moves, remap every stop linearly onto the new span rather than patching its neighbors
- Measure with `debug/ramp_relight.py`; report L\* and C\* per stop instead of judging by eye
