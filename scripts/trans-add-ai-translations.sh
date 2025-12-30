#!/bin/sh

gpt-po-translator --default-context "$(cat doc/TRANSLATING.md)" --folder po --bulk --bulksize 11 --provider claude_sdk -vv


