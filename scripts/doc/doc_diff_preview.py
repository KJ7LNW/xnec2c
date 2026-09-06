#!/usr/bin/env python3
"""Preview an HTML document's changes as a rendered, navigable diff page.

Diffs the working-tree copy of an HTML file against a git revision, marks
every insertion and deletion inline, and opens the result in a browser.
"""

import argparse
import importlib.util
import os
import subprocess
import sys
from pathlib import Path


ENVIRONMENT_DIR = '.venv'


def workspace_root():
    """Return the workspace holding this launcher's scripts/doc/ directory."""
    return Path(__file__).resolve().parents[2]


def environment_python(root):
    """Return the interpreter path inside the workspace environment."""
    return root / ENVIRONMENT_DIR / 'bin' / 'python'


def build_environment(root, interpreter):
    """Create the workspace environment and install the rendering dependency."""
    python = environment_python(root)
    subprocess.run([interpreter, '-m', 'venv', str(root / ENVIRONMENT_DIR)],
                   check=True)
    subprocess.run([str(python), '-m', 'pip', 'install', 'lxml'], check=True)


def enter_environment(root, interpreter):
    """Return once lxml is importable, replacing this process to reach it."""
    if importlib.util.find_spec('lxml') is not None:
        return
    python = environment_python(root)
    if Path(sys.executable).resolve() == python.resolve():
        raise RuntimeError(
            'environment {} carries no lxml distribution'.format(python))

    build_environment(root, interpreter)
    script = Path(__file__).resolve()
    os.execv(str(python), [str(python), str(script)] + sys.argv[1:])


def parse_args():
    """Return the target, revision, output page, opener, and interpreter."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('target', nargs='?', default='doc/xnec2c.html',
                        help='HTML file to preview (default: doc/xnec2c.html)')
    parser.add_argument('--rev', default='HEAD',
                        help='revision to diff against (default: HEAD)')
    parser.add_argument('--out', help='output page (default: <target>.diff.html)')
    parser.add_argument('--opener', default='xdg-open',
                        help='command opening the page (default: xdg-open)')
    parser.add_argument('--no-launch', action='store_true',
                        help='write the page without opening it')
    parser.add_argument('--python', default=sys.executable,
                        help='interpreter building the environment '
                             '(default: %(default)s)')
    return parser.parse_args()


def main():
    """Prepare the environment, write the preview page, and open it."""
    args = parse_args()
    enter_environment(workspace_root(), args.python)

    # imported below the bootstrap because the rendering module binds lxml at
    # its own top level, which resolves only once enter_environment returns
    from doc_diff_render import build_preview, read_revision, repo_paths

    target = Path(args.target)
    root, relative = repo_paths(target)
    committed_text = read_revision(root, relative, args.rev)

    # an empty revision addresses git's index, holding the staged content
    staged_text = read_revision(root, relative, '')
    new_text = target.read_text(encoding='utf-8', errors='replace')

    label = '{} : {} to working tree'.format(relative, args.rev)
    preview = build_preview(committed_text, staged_text, new_text, label)

    # relative image and asset references resolve against the page's own directory
    out = (Path(args.out) if args.out is not None
           else target.resolve().with_suffix('.diff.html'))
    out.write_text(preview.markup, encoding='utf-8')
    print('{}  staged=+{}/-{} unstaged=+{}/-{} blocks={} bytes={}'.format(
        out, preview.tally.staged.insertions, preview.tally.staged.deletions,
        preview.tally.unstaged.insertions, preview.tally.unstaged.deletions,
        preview.tally.blocks, len(preview.markup)))

    if not args.no_launch:
        subprocess.Popen([args.opener, str(out)],
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return 0


if __name__ == '__main__':
    sys.exit(main())
