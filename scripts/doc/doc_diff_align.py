"""Mark one HTML document's text against its committed and staged sources.

Walks the new document's own tree and wraps changed words in ins and del
elements placed inside the text nodes that already hold them. Every tag the
new document owns keeps its position, so discarding each del element restores
that document exactly as its own source renders. Each mark names the stage
that produced it, separating staged work from unstaged work.
"""

import difflib
import html
import re
from dataclasses import dataclass

from lxml import html as lxml_html
from lxml.html import defs

WORD_PATTERN = re.compile(r'\S+')

STAGE_STAGED = 'xd-staged'
STAGE_UNSTAGED = 'xd-unstaged'

MARK_INSERT = 'ins'
MARK_DELETE = 'del'


@dataclass(frozen=True)
class StageTally:
    """Insertions and deletions one stage contributed."""

    insertions: int
    deletions: int


@dataclass(frozen=True)
class MarkTally:
    """Marks each stage contributed, and the blocks carrying them."""

    staged: StageTally
    unstaged: StageTally
    blocks: int


@dataclass(frozen=True)
class WordSpan:
    """One word's character range in a text piece, and its rank in the block."""

    piece: int
    start: int
    end: int
    order: int


@dataclass(frozen=True)
class BlockReading:
    """One leaf block read once, in the single reading every comparison uses.

    Words come from the text pieces that carry them rather than from the
    block's collapsed text, so a word ends wherever an inline tag interrupts
    it. Both documents are read this way, which is what lets an unchanged
    block compare equal to itself.
    """

    element: lxml_html.HtmlElement
    pieces: list[tuple[str, str]]
    spans: list[WordSpan]
    words: list[str]
    text: str


@dataclass(frozen=True)
class BlockSources:
    """One new block beside the words each earlier document holds for it."""

    reading: BlockReading
    committed: list[str]
    staged: list[str]


@dataclass(frozen=True)
class BlockPairing:
    """Old words carried onto each new block, and the blocks with no new home."""

    paired: list[list[str]]
    orphaned: dict[int, list[list[str]]]


NO_MARKS = MarkTally(staged=StageTally(insertions=0, deletions=0),
                     unstaged=StageTally(insertions=0, deletions=0),
                     blocks=0)


def block_text(element):
    """Return an element's rendered text with whitespace runs collapsed."""
    return ' '.join(element.text_content().split())


def has_block_descendant(element):
    """Report whether any element below this one is itself a block."""
    return any(descendant.tag in defs.block_tags
               for descendant in element.iterdescendants()
               if isinstance(descendant.tag, str))


def leaf_blocks(root):
    """Return the blocks under the root that carry text and no nested block."""
    blocks = []
    for element in root.iter():
        if not isinstance(element.tag, str):
            continue
        if element.tag not in defs.block_tags:
            continue
        if has_block_descendant(element):
            continue
        if block_text(element) == '':
            continue
        blocks.append(element)
    return blocks


def open_tag(element):
    """Serialize an element's start tag with its attributes."""
    attributes = ''.join(
        ' {}="{}"'.format(name, html.escape(value, quote=True))
        for name, value in element.attrib.items())
    return '<{}{}>'.format(element.tag, attributes)


def flatten_inline(element):
    """Return an element's content as ordered markup and text pieces."""
    pieces = []
    if element.text is not None:
        pieces.append(('text', element.text))
    for child in element:
        if isinstance(child.tag, str):
            pieces.append(('markup', open_tag(child)))
            pieces.extend(flatten_inline(child))
            if child.tag not in defs.empty_tags:
                pieces.append(('markup', '</{}>'.format(child.tag)))
        else:
            pieces.append(('markup', lxml_html.tostring(
                child, encoding='unicode', with_tail=False)))
        if child.tail is not None:
            pieces.append(('text', child.tail))
    return pieces


def mark_markup(tag, stage, text):
    """Wrap text in a mark of the given tag, naming the stage that made it."""
    return '<{0} class="{1}">{2}</{0}>'.format(
        tag, stage, html.escape(text, quote=False))


def deleted_markup(words, stage):
    """Wrap removed words in a mark naming the stage that removed them."""
    return mark_markup(MARK_DELETE, stage, ' '.join(words)) + ' '


def stage_tally(events, stage):
    """Count one stage's insertions and deletions among the marks emitted."""
    tags = [tag for mark, tag in events if mark == stage]
    return StageTally(insertions=tags.count(MARK_INSERT),
                      deletions=tags.count(MARK_DELETE))


def event_tally(events):
    """Count the marks one block emitted, grouped by stage."""
    return MarkTally(staged=stage_tally(events, STAGE_STAGED),
                     unstaged=stage_tally(events, STAGE_UNSTAGED),
                     blocks=1)


def remainder(items, subtracted):
    """Return the items left once each subtracted item is matched off once."""
    pool = list(subtracted)
    left = []
    for item in items:
        if item in pool:
            pool.remove(item)
        else:
            left.append(item)
    return left


def stage_split(committed, staged):
    """Pair each key with the items each stage dropped."""
    split = {}
    for key in set(committed) | set(staged):
        unstaged = staged.get(key, [])
        split[key] = (remainder(committed.get(key, []), unstaged), unstaged)
    return split


def stage_insertions(committed_ranks, staged_ranks):
    """Name the stage that inserted each word rank.

    A rank the index still lacks is unstaged, whether or not the commit lacks
    it too, so the staged reading covers only what the index already carries.
    """
    stages = {rank: STAGE_STAGED for rank in committed_ranks}
    stages.update({rank: STAGE_UNSTAGED for rank in staged_ranks})
    return stages


def stage_removals(committed_removed, staged_removed):
    """Pair each removal rank with the staged and unstaged words it anchors."""
    return {rank: [(stage, words) for stage, words
                   in ((STAGE_STAGED, staged), (STAGE_UNSTAGED, unstaged))
                   if len(words) > 0]
            for rank, (staged, unstaged)
            in stage_split(committed_removed, staged_removed).items()}


def word_spans(pieces):
    """Return every word the text pieces carry, in document order."""
    spans = []
    for index, (kind, value) in enumerate(pieces):
        if kind != 'text':
            continue
        for match in WORD_PATTERN.finditer(value):
            spans.append(WordSpan(piece=index, start=match.start(),
                                  end=match.end(), order=len(spans)))
    return spans


def block_words(pieces, spans):
    """Return the words the spans name, read from their own text pieces."""
    return [pieces[span.piece][1][span.start:span.end] for span in spans]


def block_readings(root):
    """Read every leaf block under the root once, in the shared reading."""
    readings = []
    for element in leaf_blocks(root):
        pieces = flatten_inline(element)
        spans = word_spans(pieces)
        words = block_words(pieces, spans)
        readings.append(BlockReading(element=element, pieces=pieces,
                                     spans=spans, words=words,
                                     text=' '.join(words)))
    return readings


def classify_words(old_words, new_words):
    """Return the inserted word ranks and the removed words each rank anchors."""
    matcher = difflib.SequenceMatcher(a=old_words, b=new_words, autojunk=False)
    inserted = set()
    removed = {}
    for opcode, old_start, old_end, new_start, new_end in matcher.get_opcodes():
        if opcode == 'equal':
            continue
        if opcode == 'insert':
            inserted.update(range(new_start, new_end))
        elif opcode == 'delete':
            removed.setdefault(new_start, []).extend(old_words[old_start:old_end])
        elif opcode == 'replace':
            inserted.update(range(new_start, new_end))
            removed.setdefault(new_start, []).extend(old_words[old_start:old_end])
        else:
            raise ValueError('unknown difflib opcode {}'.format(opcode))
    return inserted, removed


def emit_piece(value, spans, inserted, removed):
    """Re-serialize one text piece, returning its markup and the marks made."""
    out = []
    events = []
    cursor = 0
    position = 0
    while position < len(spans):
        span = spans[position]
        out.append(html.escape(value[cursor:span.start], quote=False))
        for stage, words in removed.get(span.order, []):
            out.append(deleted_markup(words, stage))
            events.append((stage, MARK_DELETE))

        stage = inserted.get(span.order)
        if stage is not None:
            # carry one mark across neighbouring insertions of the same stage so
            # the words it covers read as a phrase rather than a row of boxes
            run_end = span.end
            while (position + 1 < len(spans)
                   and inserted.get(spans[position + 1].order) == stage
                   and spans[position + 1].order not in removed):
                position += 1
                run_end = spans[position].end
            out.append(mark_markup(MARK_INSERT, stage,
                                   value[span.start:run_end]))
            events.append((stage, MARK_INSERT))
            cursor = run_end
        else:
            out.append(html.escape(value[span.start:span.end], quote=False))
            cursor = span.end
        position += 1
    out.append(html.escape(value[cursor:], quote=False))
    return ''.join(out), events


def set_inner(element, markup):
    """Replace an element's content with the given markup."""
    holder = lxml_html.fragment_fromstring(markup, create_parent='div')
    del element[:]
    element.text = holder.text
    element.extend(list(holder))


def mark_element(sources):
    """Mark one block against both earlier documents and report its marks."""
    reading = sources.reading
    committed_inserted, committed_removed = classify_words(
        sources.committed, reading.words)
    staged_inserted, staged_removed = classify_words(
        sources.staged, reading.words)
    inserted = stage_insertions(committed_inserted, staged_inserted)
    removed = stage_removals(committed_removed, staged_removed)
    if len(inserted) == 0 and len(removed) == 0:
        return NO_MARKS

    by_piece = {}
    for span in reading.spans:
        by_piece.setdefault(span.piece, []).append(span)

    out = []
    events = []
    for index, (kind, value) in enumerate(reading.pieces):
        if kind == 'markup':
            out.append(value)
        else:
            markup, marks = emit_piece(value, by_piece.get(index, []),
                                       inserted, removed)
            out.append(markup)
            events.extend(marks)
    for stage, trailing in removed.get(len(reading.spans), []):
        out.append(deleted_markup(trailing, stage))
        events.append((stage, MARK_DELETE))

    set_inner(reading.element, ''.join(out))
    return event_tally(events)


def append_removed_block(element, words, stage):
    """Attach a removed block's words to the block that now precedes it."""
    element.append(lxml_html.fragment_fromstring(
        deleted_markup(words, stage).strip()))
    return event_tally([(stage, MARK_DELETE)])


def sum_tallies(tallies):
    """Sum a sequence of tallies into one."""
    return MarkTally(
        staged=StageTally(
            insertions=sum(item.staged.insertions for item in tallies),
            deletions=sum(item.staged.deletions for item in tallies)),
        unstaged=StageTally(
            insertions=sum(item.unstaged.insertions for item in tallies),
            deletions=sum(item.unstaged.deletions for item in tallies)),
        blocks=sum(item.blocks for item in tallies))


def align_blocks(old_root, new_readings):
    """Carry one earlier document's blocks onto the new document's blocks."""
    old_readings = block_readings(old_root)
    matcher = difflib.SequenceMatcher(
        a=[reading.text for reading in old_readings],
        b=[reading.text for reading in new_readings], autojunk=False)

    # an unchanged block descends from its own words, so it yields no mark
    paired = [reading.words for reading in new_readings]
    orphaned = {}
    for opcode, old_start, old_end, new_start, new_end in matcher.get_opcodes():
        if opcode == 'equal':
            continue
        count = min(old_end - old_start, new_end - new_start)
        for offset in range(count):
            paired[new_start + offset] = old_readings[old_start + offset].words
        for offset in range(count, new_end - new_start):
            paired[new_start + offset] = []

        # a removed block has no place of its own left in the new tree, so it
        # rides the block that now stands where it used to follow
        anchor = max(new_start + count - 1, 0)
        for offset in range(count, old_end - old_start):
            orphaned.setdefault(anchor, []).append(
                old_readings[old_start + offset].words)
    return BlockPairing(paired=paired, orphaned=orphaned)


def orphan_stages(committed_orphans, staged_orphans):
    """Name the stage that dropped each block the new document no longer holds."""
    return {anchor: ([(STAGE_STAGED, words) for words in staged]
                     + [(STAGE_UNSTAGED, words) for words in unstaged])
            for anchor, (staged, unstaged)
            in stage_split(committed_orphans, staged_orphans).items()}


def mark_document(committed_root, staged_root, new_root):
    """Mark the new tree against the committed and staged trees."""
    new_readings = block_readings(new_root)
    committed = align_blocks(committed_root, new_readings)
    staged = align_blocks(staged_root, new_readings)

    tallies = [mark_element(BlockSources(reading=reading,
                                         committed=committed.paired[index],
                                         staged=staged.paired[index]))
               for index, reading in enumerate(new_readings)]

    # marking replaces each block's children, so removed blocks attach only
    # once every surviving block has been marked
    for anchor, entries in orphan_stages(committed.orphaned,
                                         staged.orphaned).items():
        for stage, words in entries:
            tallies.append(append_removed_block(
                new_readings[anchor].element, words, stage))
    return sum_tallies(tallies)
