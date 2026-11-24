#!/bin/bash

cd "$(dirname "$0")/../.." || exit 1

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
total_exempt=0
num_langs=0

get_line_ranges() {
  local lines="$1"

  printf '%s\n' "$lines" | {
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

# Print the catalog line of each classifier record in one state, one per line.
classified_lines() {
  local records="$1"
  local state="$2"

  printf '%s\n' "$records" | awk -F'\t' -v state="$state" '$1 == state { print $2 }'
}

# Read the registered languages first so the reader's exit status gates the
# loop, then split them into words as the make recipes do.
linguas_path=po/LINGUAS
if ! languages=$(scripts/po/po-linguas.sh "$linguas_path"); then
  echo "$0: err: $linguas_path: unreadable" >&2
  exit 1
fi

for lang in $languages; do
  po="po/$lang.po"

  # Read the statistics in the C locale, since gettext translates their wording.
  if ! stats=$(LC_ALL=C msgfmt --statistics -o /dev/null "$po" 2>&1); then
    printf '%s\n' "$stats" >&2
    echo "$0: err: $po: msgfmt could not read the catalog" >&2
    exit 1
  fi

  # msgfmt always reports the translated count and omits a zero fuzzy or
  # untranslated count.
  if [[ $stats =~ ([0-9]+)\ translated ]]; then
    translated=${BASH_REMATCH[1]}
  else
    echo "$0: err: $po: msgfmt reported no translated count ($stats)" >&2
    exit 1
  fi
  if [[ $stats =~ ([0-9]+)\ fuzzy ]]; then
    fuzzy=${BASH_REMATCH[1]}
  else
    # Count the fuzzy entries msgfmt omits as zero.
    fuzzy=0
  fi
  if [[ $stats =~ ([0-9]+)\ untranslated ]]; then
    untranslated=${BASH_REMATCH[1]}
  else
    # Count the untranslated entries msgfmt omits as zero.
    untranslated=0
  fi

  # Classify the entries through the shared classifier. It names the entries
  # whose empty msgstr is a recorded exemption; msgfmt counts those as
  # untranslated, so they report apart from the untranslated count.
  if ! records=$(awk -f scripts/po/po-classify.awk "$po"); then
    echo "$0: err: $po: entry classifier failed" >&2
    exit 1
  fi
  exempt=$(classified_lines "$records" exempt | wc -l)
  if [ $exempt -gt $untranslated ]; then
    echo "$0: err: $po: $exempt exempt entries exceed the $untranslated untranslated msgfmt counted" >&2
    exit 1
  fi
  untranslated=$((untranslated - exempt))

  total=$((translated + fuzzy + untranslated))

  if [ $total -gt 0 ]; then
    percent=$((translated * 100 / total))
  else
    percent=0
  fi

  if [ $VERBOSE -eq 0 ] && [ $fuzzy -eq 0 ] && [ $untranslated -eq 0 ]; then
    :
  else
    printf "%-8s: %3d%% (%3d translated, %3d fuzzy, %3d untranslated, %3d exempt)\n" \
      "$lang" "$percent" "$translated" "$fuzzy" "$untranslated" "$exempt"

    if [ $fuzzy -gt 0 ]; then
      fuzzy_ranges=$(get_line_ranges "$(classified_lines "$records" fuzzy)")
      echo "         Fuzzy entries at lines: $fuzzy_ranges"
    fi

    if [ $untranslated -gt 0 ]; then
      untrans_ranges=$(get_line_ranges "$(classified_lines "$records" untranslated)")
      echo "         Untranslated entries at lines: $untrans_ranges"
    fi
  fi

  total_translated=$((total_translated + translated))
  total_fuzzy=$((total_fuzzy + fuzzy))
  total_untranslated=$((total_untranslated + untranslated))
  total_exempt=$((total_exempt + exempt))
  num_langs=$((num_langs + 1))
done

echo ""
echo "Summary"
echo "-------"
echo "Languages: $num_langs"
echo "Total translated messages: $total_translated"
echo "Total fuzzy messages: $total_fuzzy"
echo "Total untranslated messages: $total_untranslated"
echo "Total exempt messages: $total_exempt"

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
