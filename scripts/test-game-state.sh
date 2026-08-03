#!/bin/sh
set -eu

test_binary="${TMPDIR:-/tmp}/house-at-the-end-state-test"
cc -std=c11 -Wall -Wextra -Werror -pedantic -Isrc/c \
  tests/house_state_test.c src/c/house_state.c \
  -o "$test_binary"
"$test_binary"
