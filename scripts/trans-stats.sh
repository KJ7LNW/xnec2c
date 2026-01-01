#!/bin/bash

cd "$(dirname "$0")/.."

VERBOSE=0
if [ "$1" = "-v" ] || [ "$1" = "--verbose" ]; then
  VERBOSE=1
fi

echo "Translation Statistics for xnec2c"
echo "=================================="
echo ""

total_translated=0
total_fuzzy=0
total_untranslated=0
num_langs=0

get_line_ranges() {
  local po_file="$1"
  local pattern="$2"

  grep -n "$pattern" "$po_file" | cut -d: -f1 | {
    read -r first
    [ -z "$first" ] && return

    prev=$first
    start=$first
    ranges=""

    while read -r line; do
      if [ $((line - prev)) -gt 50 ]; then
        if [ -n "$ranges" ]; then
          ranges="$ranges, "
        fi
        if [ $start -eq $prev ]; then
          ranges="${ranges}${start}"
        else
          ranges="${ranges}${start}-${prev}"
        fi
        start=$line
      fi
      prev=$line
    done

    if [ -n "$ranges" ]; then
      ranges="$ranges, "
    fi
    if [ $start -eq $prev ]; then
      ranges="${ranges}${start}"
    else
      ranges="${ranges}${start}-${prev}"
    fi

    echo "$ranges"
  }
}

for po in po/*.po; do
  lang=$(basename "$po" .po)

  stats=$(msgfmt --statistics -o /dev/null "$po" 2>&1)

  translated=$(echo "$stats" | grep -oP '\d+(?= translated)' || echo "0")
  fuzzy=$(echo "$stats" | grep -oP '\d+(?= fuzzy)' || echo "0")
  untranslated=$(echo "$stats" | grep -oP '\d+(?= untranslated)' || echo "0")

  total=$((translated + fuzzy + untranslated))

  if [ $total -gt 0 ]; then
    percent=$((translated * 100 / total))
  else
    percent=0
  fi

  if [ $VERBOSE -eq 0 ] && [ $fuzzy -eq 0 ] && [ $untranslated -eq 0 ]; then
    :
  else
    printf "%-8s: %3d%% (%3d translated, %3d fuzzy, %3d untranslated)\n" \
      "$lang" "$percent" "$translated" "$fuzzy" "$untranslated"

    if [ $fuzzy -gt 0 ]; then
      fuzzy_ranges=$(get_line_ranges "$po" "^#, fuzzy")
      echo "         Fuzzy entries at lines: $fuzzy_ranges"
    fi

    if [ $untranslated -gt 0 ]; then
      untrans_ranges=$(get_line_ranges "$po" '^msgstr ""$')
      echo "         Untranslated entries at lines: $untrans_ranges"
    fi
  fi

  total_translated=$((total_translated + translated))
  total_fuzzy=$((total_fuzzy + fuzzy))
  total_untranslated=$((total_untranslated + untranslated))
  num_langs=$((num_langs + 1))
done

echo ""
echo "Summary"
echo "-------"
echo "Languages: $num_langs"
echo "Total translated messages: $total_translated"
echo "Total fuzzy messages: $total_fuzzy"
echo "Total untranslated messages: $total_untranslated"

if [ $num_langs -gt 0 ]; then
  total_messages=$((total_translated + total_fuzzy + total_untranslated))
  avg_translated=$((total_translated / num_langs))
  avg_percent=$((total_translated * 100 / total_messages))
  echo "Average translated per language: $avg_translated ($avg_percent%)"
fi

if [ $VERBOSE -eq 0 ]; then
  echo ""
  echo "Use -v or --verbose to show line numbers of fuzzy/untranslated entries"
fi
