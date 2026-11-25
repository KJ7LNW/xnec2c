# Structure view

Apply each line when changing structure view startup, theme timing before the first draw, structure geometry colors, or two-port network drawing.

## User-given

- Apply the selected theme before any engine draws on the structure and radiation pattern surfaces.
- Take wire colors from the theme under all conditions, legacy colors when no theme is defined; map structure colors through existing theme attributes.
- Keep each engine's own two-port corner formulas.

## Model-derived

- Rebuild every theme-derived datum through the one rebuild path that startup and theme change share.
- Seed the structure viewport at startup.
