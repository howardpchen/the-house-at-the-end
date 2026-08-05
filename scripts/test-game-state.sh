#!/bin/sh
set -eu

test_directory="$(mktemp -d)"
trap 'rm -rf "$test_directory"' EXIT

test_binary="$test_directory/house-state-test"
cc -std=c11 -Wall -Wextra -Werror -pedantic -Isrc/c \
  tests/house_state_test.c src/c/house_state.c \
  -o "$test_binary"
"$test_binary"

test_binary="$test_directory/world-gen-test"
cc -std=c11 -Wall -Wextra -Werror -pedantic -Isrc/c \
  tests/world_gen_test.c src/c/world_gen.c \
  -o "$test_binary"
"$test_binary"

test_binary="$test_directory/save-store-test"
cc -std=c11 -Wall -Wextra -Werror -pedantic -Isrc/c \
  tests/save_store_test.c src/c/save_store.c src/c/game_state.c \
  src/c/house_state.c src/c/world_gen.c \
  -o "$test_binary"
"$test_binary"

python3 tools/compile_scenes.py --check
test_binary="$test_directory/scene-vm-test"
cc -std=c11 -Wall -Wextra -Werror -pedantic -Isrc/c \
  tests/scene_vm_test.c src/c/scene_vm.c src/c/content_format.c \
  -o "$test_binary"
"$test_binary"

test_binary="$test_directory/expedition-test"
cc -std=c11 -Wall -Wextra -Werror -pedantic -Isrc/c \
  tests/expedition_test.c src/c/expedition.c src/c/game_state.c \
  src/c/house_state.c src/c/world_gen.c \
  -o "$test_binary"
"$test_binary"

test_binary="$test_directory/game-state-test"
cc -std=c11 -Wall -Wextra -Werror -pedantic -Isrc/c \
  tests/game_state_test.c src/c/game_state.c src/c/house_state.c \
  src/c/world_gen.c \
  -o "$test_binary"
"$test_binary"
