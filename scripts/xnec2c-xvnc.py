#!/usr/bin/env python3.11
"""Run the in-tree xnec2c on a private Xvnc display that closes when xnec2c exits.

Xvnc listens on no network port; a VNC viewer attaches through a socket in the run directory.
"""

import argparse
import os
from pathlib import Path
import signal
import subprocess
import sys
from typing import TextIO

ROOT = Path(__file__).resolve().parents[1]
PROGRAM = Path(sys.argv[0]).name
DISPLAY_LOG = "display.log"
VIEWER_SOCKET = "vnc.sock"


def pr_message(severity: str, subject: str, state: str) -> None:
    """Write one program: severity: subject: state line to standard error."""
    print(f"{PROGRAM}: {severity}: {subject}: {state}", file=sys.stderr, flush=True)


def pr_info(subject: str, state: str) -> None:
    """Report a state the run reached."""
    pr_message("info", subject, state)


def pr_err(subject: str, state: str) -> None:
    """Report a failure the run met."""
    pr_message("err", subject, state)


def software_gl_environment(egl_vendor: Path | None) -> dict[str, str]:
    """Return the process environment selecting software GL and the EGL vendor for Xvnc and xnec2c."""
    if egl_vendor is None:
        # glvnd selects the EGL vendor from its installed manifests
        vendor_environment = {}
    else:
        vendor_environment = {"__EGL_VENDOR_LIBRARY_FILENAMES": str(egl_vendor)}

    # Render GL in software, since Xvnc has no rendering device; Xvnc's GLX loads the same EGL vendor
    return dict(os.environ, **vendor_environment, LIBGL_ALWAYS_SOFTWARE="1")


def start_display(gl_environment: dict[str, str], viewer_socket: Path, glx: bool,
                  log: TextIO) -> tuple[subprocess.Popen[bytes], str]:
    """Start Xvnc and return it with the display number it reports, empty when it exits first."""
    glx_options = [] if glx else ["-extension", "GLX"]
    reader, writer = os.pipe()
    with os.fdopen(reader) as ready:
        server = subprocess.Popen(["Xvnc", "-displayfd", str(writer), "-rfbport", "-1",
                                   "-rfbunixpath", str(viewer_socket), "-localhost", "-nolisten", "tcp",
                                   *glx_options],
                                  env=gl_environment, stdout=log, stderr=subprocess.STDOUT, pass_fds=(writer,))

        # Close the parent's copy so the read ends when Xvnc exits without reporting
        os.close(writer)
        number = ready.readline().strip()
    return server, number


def run_app(gl_environment: dict[str, str], run_dir: Path, display: str, app_args: list[str]) -> int:
    """Run xnec2c on the display with its home in the run directory and return its exit status."""

    # Keep the app's settings out of the user's home
    environment = dict(gl_environment, DISPLAY=display, GDK_BACKEND="x11", HOME=str(run_dir),
                       XDG_CONFIG_HOME=str(run_dir / "config"), XDG_CACHE_HOME=str(run_dir / "cache"))
    result = subprocess.run([str(ROOT / "src" / "xnec2c"), *app_args], env=environment)
    if result.returncode < 0:
        pr_err("xnec2c", f"terminated by {signal.Signals(-result.returncode).name}")
        status = 128 - result.returncode
    else:
        # xnec2c reports its own failures
        status = result.returncode
    return status


def main() -> int:
    """Parse the options, run xnec2c on a private display, and return its exit status."""
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-r", "--run-dir", type=Path, required=True,
                        help=f"directory for {DISPLAY_LOG}, {VIEWER_SOCKET}, and the app's home; created when absent")
    parser.add_argument("-G", "--no-glx", dest="glx", action="store_false",
                        help="start the display without GLX, so OpenGL is unavailable")
    parser.add_argument("-E", "--egl-vendor", type=Path, help="glvnd EGL vendor manifest for xnec2c, eg Mesa's")
    parser.add_argument("app_args", nargs="*", metavar="XNEC2C_ARG", help="xnec2c options, after --")
    args = parser.parse_args()
    if args.egl_vendor is not None and not args.egl_vendor.is_file():
        parser.error(f"--egl-vendor {args.egl_vendor}: no such manifest file")
    viewer_socket = args.run_dir / VIEWER_SOCKET
    args.run_dir.mkdir(parents=True, exist_ok=True)
    gl_environment = software_gl_environment(args.egl_vendor)
    with (args.run_dir / DISPLAY_LOG).open("w") as log:
        server, number = start_display(gl_environment, viewer_socket, args.glx, log)
        try:
            if number.isdecimal():
                display = f":{number}"
                pr_info("display", f"{display} (GLX {'on' if args.glx else 'off'}; viewer {viewer_socket})")
                status = run_app(gl_environment, args.run_dir, display, args.app_args)
            else:
                pr_err("Xvnc", f"exited before reporting a display ({log.name})")
                status = 1
        finally:
            server.terminate()
            server.wait()
    return status


if __name__ == "__main__":
    raise SystemExit(main())
