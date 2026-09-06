"""Compose a navigable preview page from a marked HTML document.

Carries the presentation the page needs: the mark styling, the navigation
toolbar, and the page assembly around a document the aligner has marked.
"""

import html
import subprocess
from dataclasses import dataclass
from pathlib import Path

from lxml import html as lxml_html

from doc_diff_align import MarkTally, mark_document


@dataclass(frozen=True)
class PreviewPage:
    """A rendered diff page together with the mark counts it carries."""

    markup: str
    tally: MarkTally


DIFF_STYLE = """
body { padding-top: 3.5em; }
ins { text-decoration: none; }
del { text-decoration: line-through; }
ins.xd-staged, ins.xd-staged > * {
  background-color: #c8f0c8 !important; outline: 1px solid #4c8c4c; }
del.xd-staged, del.xd-staged > * {
  background-color: #f5c9c9 !important; outline: 1px solid #a05050; }
ins.xd-unstaged, ins.xd-unstaged > * {
  background-color: #e4f4e4 !important; outline: 1px dashed #4c8c4c; }
del.xd-unstaged, del.xd-unstaged > * {
  background-color: #fae6e6 !important; outline: 1px dashed #a05050; }
body.xd-plain-staged del.xd-staged { display: none; }
body.xd-plain-staged ins.xd-staged, body.xd-plain-staged ins.xd-staged > * {
  background-color: transparent !important; outline: none !important;
}
body.xd-plain-unstaged del.xd-unstaged { display: none; }
body.xd-plain-unstaged ins.xd-unstaged, body.xd-plain-unstaged ins.xd-unstaged > * {
  background-color: transparent !important; outline: none !important;
}
ins, del { scroll-margin-top: 5em; }
.xd-current { outline: 3px solid #d08000 !important; }
#xd-nav {
  position: fixed; top: 0; left: 0; right: 0; z-index: 9999;
  background: #222; color: #eee; padding: 0.4em 0.8em;
  font: 13px/1.6 monospace; display: flex; gap: 0.8em; align-items: center;
}
#xd-nav button { font: inherit; padding: 0 0.6em; cursor: pointer; }
#xd-nav label { cursor: pointer; user-select: none; }
#xd-nav label.xd-off { color: #888; cursor: default; }
#xd-label { flex: 1; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
#xd-key { color: #999; }
body { padding-left: 1.6em; }
#xd-rail {
  position: fixed; top: 3.5em; left: 0; bottom: 0; width: 1.2em; z-index: 9998;
  background: #f4f4f4; border-right: 1px solid #ccc; cursor: pointer;
}
.xd-tick {
  position: absolute; left: 2px; right: 2px; height: 3px; margin-top: -1px;
  cursor: pointer;
}
.xd-tick-ins.xd-staged { background-color: #4c8c4c; }
.xd-tick-del.xd-staged { background-color: #a05050; }
.xd-tick-ins.xd-unstaged { background-color: #a8d0a8; }
.xd-tick-del.xd-unstaged { background-color: #e0b0b0; }
.xd-tick-current { outline: 2px solid #d08000; height: 5px; }
#xd-view {
  position: absolute; left: 0; right: 0; min-height: 2px; pointer-events: none;
  background: rgba(0, 0, 0, 0.10); border: 1px solid #888;
}
"""

NAV_SCRIPT = """
function xdStage(name) {
  return {toggle: document.getElementById('xd-show-' + name), name: name,
          mark: 'xd-' + name, plain: 'xd-plain-' + name};
}
function xdSetAvailable(stage) {
  // a stage the comparison never produced has nothing to filter, so its
  // control states that reason instead of accepting a click
  var present = document.querySelector(
    'ins.' + stage.mark + ', del.' + stage.mark) !== null;
  var label = stage.toggle.parentNode;
  stage.toggle.disabled = !present;
  label.classList.toggle('xd-off', !present);
  label.title = present
    ? '' : 'this comparison carries no ' + stage.name + ' changes';
}
var xdKeys = new Map([['n', 1], ['j', 1], ['p', -1], ['k', -1]]);
var xdStages = ['staged', 'unstaged'].map(xdStage);
xdStages.forEach(xdSetAvailable);
var xdPos = document.getElementById('xd-pos');
var xdRail = document.getElementById('xd-rail');
var xdView = document.getElementById('xd-view');
var xdStops = [];
var xdCursor = -1;
function xdContiguous(first, second) {
  // one region runs unbroken, so only whitespace may stand between its marks
  var node = first.nextSibling;
  while (node !== null && node !== second
         && node.nodeType === 3 && node.textContent.trim() === '') {
    node = node.nextSibling;
  }
  return node === second;
}
function xdGroup(marks) {
  var groups = [];
  marks.forEach(function (mark, index) {
    if (index > 0 && xdContiguous(marks[index - 1], mark)) {
      groups[groups.length - 1].push(mark);
    } else {
      groups.push([mark]);
    }
  });
  return groups;
}
function xdStop(marks) {
  var lead = marks[0];
  var tick = document.createElement('div');
  tick.className = 'xd-tick xd-tick-' + lead.tagName.toLowerCase() + ' '
    + (lead.classList.contains('xd-staged') ? 'xd-staged' : 'xd-unstaged');
  return {marks: marks, tick: tick};
}
function xdTrack() {
  var height = document.documentElement.scrollHeight;
  xdView.style.top = (100 * window.scrollY / height) + '%';
  xdView.style.height = (100 * window.innerHeight / height) + '%';
}
function xdPlace() {
  var height = document.documentElement.scrollHeight;
  xdStops.forEach(function (stop) {
    var offset = stop.marks[0].getBoundingClientRect().top + window.scrollY;
    stop.tick.style.top = (100 * offset / height) + '%';
  });
  xdTrack();
}
function xdCollect() {
  Array.prototype.forEach.call(document.querySelectorAll('.xd-current'),
    function (mark) { mark.classList.remove('xd-current'); });
  var selectors = [];
  xdStages.forEach(function (stage) {
    document.body.classList.toggle(stage.plain, !stage.toggle.checked);
    if (stage.toggle.checked) {
      selectors.push('ins.' + stage.mark, 'del.' + stage.mark);
    }
  });
  var found = selectors.length === 0 ? [] : Array.prototype.slice.call(
    document.querySelectorAll(selectors.join(', ')));
  // the viewport box shares the rail with the ticks, so ticks leave one by one
  xdStops.forEach(function (stop) { xdRail.removeChild(stop.tick); });
  xdStops = xdGroup(found).map(xdStop);
  xdStops.forEach(function (stop) { xdRail.appendChild(stop.tick); });
  xdPlace();
  xdCursor = -1;
  xdPos.textContent = '0/' + xdStops.length;
}
xdStages.forEach(function (stage) {
  stage.toggle.addEventListener('change', xdCollect);
});
xdCollect();
// images load after this script runs, so every tick re-places once the
// document reaches its final height
window.addEventListener('load', xdPlace);
window.addEventListener('resize', xdPlace);
window.addEventListener('scroll', xdTrack);
function xdNearest(fraction) {
  var best = 0;
  var gap = Infinity;
  xdStops.forEach(function (stop, index) {
    var distance = Math.abs(parseFloat(stop.tick.style.top) - fraction);
    if (distance < gap) { gap = distance; best = index; }
  });
  return best;
}
// the rail answers a click anywhere along its length, so reaching a thin tick
// asks for no precise aim
xdRail.addEventListener('click', function (event) {
  if (xdStops.length === 0) { return; }
  var rect = xdRail.getBoundingClientRect();
  xdJump(xdNearest(100 * (event.clientY - rect.top) / rect.height));
});
function xdHighlight(stop, on) {
  stop.marks.forEach(function (mark) {
    mark.classList.toggle('xd-current', on);
  });
  stop.tick.classList.toggle('xd-tick-current', on);
}
function xdJump(index) {
  if (xdCursor >= 0) { xdHighlight(xdStops[xdCursor], false); }
  xdCursor = index;
  xdHighlight(xdStops[xdCursor], true);
  xdStops[xdCursor].marks[0].scrollIntoView({block: 'center'});
  xdPos.textContent = (xdCursor + 1) + '/' + xdStops.length;
}
function xdGo(step) {
  if (xdStops.length === 0) { return; }
  xdJump((xdCursor + step + xdStops.length) % xdStops.length);
}
document.addEventListener('keydown', function (event) {
  var step = xdKeys.get(event.key);
  if (step !== undefined) { xdGo(step); }
});
"""


def repo_paths(target):
    """Return the repository root and the target's path relative to it."""
    root = Path(subprocess.run(
        ['git', '-C', str(target.parent), 'rev-parse', '--show-toplevel'],
        capture_output=True, check=True, text=True).stdout.strip())
    return root, target.resolve().relative_to(root)


def read_revision(root, relative, revision):
    """Return the target's content as recorded at the named git revision."""
    return subprocess.run(
        ['git', '-C', str(root), 'show', '{}:{}'.format(revision, relative)],
        capture_output=True, check=True, text=True, errors='replace').stdout


def inner_html(element):
    """Serialize an element's text and children without its own tags."""
    parts = [element.text or '']
    parts.extend(lxml_html.tostring(child, encoding='unicode') for child in element)
    return ''.join(parts)


def overlay_markup(label):
    """Build the fixed toolbar and the tick rail laid over the document."""
    return ('<div id="xd-nav">'
            '<span id="xd-label">' + html.escape(label) + '</span>'
            '<button onclick="xdGo(-1)">&#9664; prev</button>'
            '<span id="xd-pos"></span>'
            '<button onclick="xdGo(1)">next &#9654;</button>'
            '<label><input type="checkbox" id="xd-show-staged" checked>'
            ' staged</label>'
            '<label><input type="checkbox" id="xd-show-unstaged" checked>'
            ' unstaged</label>'
            '<span id="xd-key">n/p or j/k</span>'
            '</div>\n'
            '<div id="xd-rail"><div id="xd-view"></div></div>\n')


def build_preview(committed_text, staged_text, new_text, label):
    """Compose a standalone page marking every insertion and deletion."""
    new_root = lxml_html.document_fromstring(new_text)
    tally = mark_document(lxml_html.document_fromstring(committed_text),
                          lxml_html.document_fromstring(staged_text),
                          new_root)

    head = new_root.find('head')
    page = ('<!DOCTYPE html>\n<html>\n<head>\n<meta charset="utf-8">\n'
            + ('' if head is None else inner_html(head))
            + '\n<style>' + DIFF_STYLE + '</style>\n</head>\n<body>\n'
            + overlay_markup(label)
            + inner_html(new_root.find('body'))
            + '\n<script>' + NAV_SCRIPT + '</script>\n</body>\n</html>\n')
    return PreviewPage(markup=page, tally=tally)
