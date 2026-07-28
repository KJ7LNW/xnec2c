// Render highlighted Numerical Electromagnetics Code deck source.

const NEC_COMMENT_CARDS = new Set(["CM", "CE"]);

/** Append text only when it contributes a text node. */
function append_nec_text(container, text) {
  if (text.length === 0) {
    return;
  }
  container.appendChild(document.createTextNode(text));
}

/** Append one highlighted deck line to a container. */
function append_nec_line(container, line) {
  const parsed = line.match(/^(\s*)([A-Za-z]{2})(.*)$/);
  if (parsed === null) {
    container.appendChild(document.createTextNode(line));
    return;
  }

  container.appendChild(document.createTextNode(parsed[1]));
  const card = document.createElement("span");
  card.className = "gallery-nec-card";
  card.textContent = parsed[2];
  container.appendChild(card);

  if (NEC_COMMENT_CARDS.has(parsed[2].toUpperCase())) {
    const comment = document.createElement("span");
    comment.className = "gallery-nec-comment";
    comment.textContent = parsed[3];
    container.appendChild(comment);
  } else {
    append_nec_numbers(container, parsed[3]);
  }
}

/** Append text while highlighting numeric fields. */
function append_nec_numbers(container, text) {
  const numbers = /[-+]?\d[\d.eE+-]*/g;
  let cursor = 0;
  let match = numbers.exec(text);

  while (match !== null) {
    append_nec_text(container, text.slice(cursor, match.index));
    const number = document.createElement("span");
    number.className = "gallery-nec-num";
    number.textContent = match[0];
    container.appendChild(number);
    cursor = match.index + match[0].length;
    match = numbers.exec(text);
  }

  append_nec_text(container, text.slice(cursor));
}

/** Replace a source container with highlighted deck text. */
function render_nec_source(container, text) {
  container.textContent = "";
  const lines = text.split(/\n/);
  lines.forEach(function render_line(line, index) {
    append_nec_line(container, line);
    append_nec_text(container, index < lines.length - 1 ? "\n" : "");
  });
}

window.XNEC2C_GALLERY_NEC = { render_nec_source };
