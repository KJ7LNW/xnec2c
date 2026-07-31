#!/bin/sh

# Canonicalize one catalog: merge it against the template, drop obsolete
# entries, and suppress backups. Single point of truth for catalog
# regeneration; every generation path calls this so no catalog keeps a
# #~ obsolete entry or a .po~ backup.

set -e

po=$1
pot=${2:-po/xnec2c.pot}

msgmerge --update --backup=none "$po" "$pot"
msgattrib --no-obsolete -o "$po" "$po"
