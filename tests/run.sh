#!/bin/sh
set -eu

test_binary="$(mktemp "${TMPDIR:-/tmp}/luna-test.XXXXXX")"
trap 'rm -f "$test_binary"' EXIT

cc -std=gnu11 -Wall -Wextra -Werror \
  -Itests/include -I. \
  display/widgets/widget_wpm.c tests/test_luna.c \
  -o "$test_binary"
"$test_binary"
