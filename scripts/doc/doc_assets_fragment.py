#!/usr/bin/env python3
"""Write the automake fragment shipping and installing the manual's assets.

Parses the manual's HTML, resolves every local src and href reference against
the manual's directory, and writes one automake data variable per asset
directory, so make dist and make install carry exactly the files the manual
references. The generated example gallery stays out: only the gallery targets
produce it.

Run from the top source directory.
"""

import argparse
import re
import subprocess
import sys
from dataclasses import dataclass
from html.parser import HTMLParser
from pathlib import Path, PurePosixPath
from urllib.parse import unquote, urlsplit


@dataclass(frozen=True)
class FragmentSpec:
    """Describe the page whose assets the fragment registers."""

    page: PurePosixPath
    generated_dirs: tuple[PurePosixPath, ...]
    install_dir: str
    variable_prefix: str
    reference_attributes: frozenset[str]


@dataclass
class AssetDirectory:
    """Hold the assets sharing one install directory and its variable stem."""

    stem: str
    directory: PurePosixPath
    install_path: str
    assets: list[PurePosixPath]


GALLERY_MODULE_DIR = 'scripts/gallery'


def gallery_dir():
    """Return the page-relative gallery directory the gallery module names."""
    result = subprocess.run(
        ['perl', '-I', GALLERY_MODULE_DIR, '-MGalleryBundle=GALLERY_DIR',
         '-e', 'print STDOUT GALLERY_DIR'],
        check=True, capture_output=True, text=True)
    return PurePosixPath(result.stdout)


def manual_spec():
    """Return the manual's FragmentSpec, its generated gallery excluded."""
    return FragmentSpec(
        page=PurePosixPath('doc/xnec2c.html'),
        generated_dirs=(gallery_dir(),),
        install_dir='$(docdir)',
        variable_prefix='dochtml',
        reference_attributes=frozenset({'src', 'href'}),
    )


class ReferenceCollector(HTMLParser):
    """Collect the value of every reference attribute in document order."""

    def __init__(self, reference_attributes):
        super().__init__(convert_charrefs=True)
        self.reference_attributes = reference_attributes
        self.references = []

    def handle_starttag(self, tag, attrs):
        """Record each reference attribute value the tag carries."""
        for name, value in attrs:
            if name not in self.reference_attributes or value is None:
                continue
            self.references.append(value)
        return None

    def handle_startendtag(self, tag, attrs):
        """Record the references of a self-closing tag like any start tag."""
        return self.handle_starttag(tag, attrs)


def page_references(spec):
    """Return every reference attribute value of the spec's page, in order."""
    collector = ReferenceCollector(spec.reference_attributes)
    collector.feed(Path(spec.page).read_text(encoding='utf-8'))
    collector.close()
    return collector.references


def local_asset(reference):
    """Return the page-relative path a reference names in the page's tree.

    Return None for a reference leaving the file tree: one carrying a scheme
    or network location, or an in-page anchor holding no path.
    """
    parts = urlsplit(reference)
    if parts.scheme != '' or parts.netloc != '' or parts.path == '':
        asset = None
    else:
        asset = PurePosixPath(unquote(parts.path))
    return asset


def is_generated(spec, asset):
    """Return whether asset lies under a directory only a build target writes."""
    return any(asset.is_relative_to(directory)
               for directory in spec.generated_dirs)


def validate_asset(spec, reference, asset):
    """Raise unless asset names an existing file inside the page's directory."""
    page_dir = spec.page.parent
    if asset.is_absolute() or '..' in asset.parts:
        raise ValueError(f'{spec.page}: reference "{reference}" leaves '
                         f'{page_dir}; expected a path inside it')
    source = page_dir / asset
    if not Path(source).is_file():
        raise FileNotFoundError(f'{spec.page}: reference "{reference}" names '
                                f'{source}, which is no file')
    return None


def page_assets(spec):
    """Return the sorted page-relative files the spec's page references.

    The page itself installs as documentation in its own right, so a
    reference back to it registers no asset.
    """
    page_self = PurePosixPath(spec.page.name)
    assets = set()
    for reference in page_references(spec):
        asset = local_asset(reference)
        if asset is None or asset == page_self or is_generated(spec, asset):
            continue
        validate_asset(spec, reference, asset)
        assets.add(asset)
    return sorted(assets)


def asset_directory(spec, directory):
    """Return an empty AssetDirectory naming directory's variable stem."""
    if directory == PurePosixPath('.'):
        stem = spec.variable_prefix
        install_path = spec.install_dir
    else:
        stem = spec.variable_prefix + '_' + re.sub(r'[^A-Za-z0-9]', '_',
                                                   str(directory))
        install_path = f'{spec.install_dir}/{directory}'
    return AssetDirectory(stem=stem, directory=directory,
                          install_path=install_path, assets=[])


def group_by_directory(spec, assets):
    """Return the AssetDirectory groups of assets, ordered by directory."""
    groups = {}
    for asset in assets:
        group = asset_directory(spec, asset.parent)
        held = groups.setdefault(group.stem, group)
        if held.directory != group.directory:
            raise ValueError(f'{spec.page}: directories {held.directory} and '
                             f'{group.directory} share variable stem '
                             f'{group.stem}; expected one stem per directory')
        held.assets.append(asset)
    return sorted(groups.values(), key=lambda group: group.directory)


def render_fragment(spec, groups):
    """Return the automake fragment text declaring each group's data files."""
    page_dir = spec.page.parent
    lines = [f'# Generated from {spec.page} by {Path(__file__).name}; '
             'edit the page, not this file.']
    for group in groups:
        lines.append('')
        lines.append(f'{group.stem}dir = {group.install_path}')
        lines.append(f'dist_{group.stem}_DATA =')
        for asset in group.assets:
            lines.append(f'dist_{group.stem}_DATA += {page_dir / asset}')
    return '\n'.join(lines) + '\n'


def parse_args():
    """Return the command line holding the fragment path to write."""
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('fragment', help='automake fragment to write')
    return parser.parse_args()


def main():
    """Write the manual's asset fragment to the path the command line names."""
    args = parse_args()
    spec = manual_spec()
    groups = group_by_directory(spec, page_assets(spec))
    Path(args.fragment).write_text(render_fragment(spec, groups),
                                   encoding='utf-8')
    return 0


if __name__ == '__main__':
    sys.exit(main())
