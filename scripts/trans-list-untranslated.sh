#!/bin/bash

cd "$(dirname "$0")/.."

for po_file in po/*.po; do
  untranslated=$(msggrep --msgstr --no-wrap -v -T -e '.' "$po_file" 2>/dev/null)

  if [ -n "$untranslated" ]; then
    echo "=== $po_file ==="
    echo "$untranslated"
    echo ""
  fi
done
