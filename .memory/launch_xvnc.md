# Launching xnec2c on a private display

Apply each line when launching xnec2c for a test, proof, or screenshot, or reproducing serial sweep failures.

## User-given

- For serial sweep reproduction, run `python3.11 t/serial_sweep_batch.py --pairs 10`; the script manages its display and batch trials.

## Model-derived

- Pass Mesa's glvnd EGL vendor file through `-E` for GLX-on runs.
- Treat as expected noise: `[crit] not forked, ignoring SIGCHLD` and kernel-ratio warnings.
