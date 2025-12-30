#!/bin/bash

cd "$(dirname "$0")/.."

for po in po/*.po; do
  echo "Updating $po..."
  msgmerge --update --no-wrap --backup=none "$po" po/xnec2c.pot
done
