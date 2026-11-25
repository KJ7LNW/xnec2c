# Batch PNG export

Apply each line when changing how batch mode sizes, renders, fits, or captures PNG frames.

## User-given

- Batch output resolution comes from `--png-size` alone.
- Prove each batch export change with opened PNGs rendered at scale 2.

## Model-derived

- Enforce the batch frame size once, at the canvas surface layer, for every engine.
