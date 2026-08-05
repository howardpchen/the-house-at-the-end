#include "world_gen.h"

#include <assert.h>
#include <stdio.h>

static void test_regions(void) {
  assert(world_region_at(0, 0) == WORLD_REGION_HEARTH);
  assert(world_region_at(4, 0) == WORLD_REGION_NEAR);
  assert(world_region_at(8, -2) == WORLD_REGION_CONTRADICTION);
  assert(world_region_at(-11, 3) == WORLD_REGION_MACHINE);
  assert(world_region_at(15, 15) == WORLD_REGION_THRESHOLD);
}

static void test_visibility(void) {
  WorldState state;
  world_state_init(&state, 1234U);
  assert(world_is_visible(&state, 0, 0));
  assert(world_is_visible(&state, 1, 0));
  assert(world_is_visible(&state, 0, -1));
  assert(!world_is_visible(&state, 2, 0));
  world_reveal(&state, 1, 0);
  assert(world_is_visible(&state, 2, 0));
}

static void test_landmarks(void) {
  const uint32_t seed = 0x12345678U;
  for (uint8_t id = 0; id < WORLD_LANDMARK_COUNT; ++id) {
    const WorldPoint point = world_landmark_position(seed, id);
    uint8_t found = UINT8_MAX;
    assert(world_is_in_bounds(point.x, point.y));
    assert(world_tile_at(seed, point.x, point.y, &found) ==
           WORLD_TILE_LANDMARK);
    assert(found == id);
    for (uint8_t other = 0; other < id; ++other) {
      const WorldPoint prior = world_landmark_position(seed, other);
      assert(prior.x != point.x || prior.y != point.y);
    }
  }

  WorldState state;
  world_state_init(&state, seed);
  assert(!world_landmark_is_cleared(&state, 7));
  world_clear_landmark(&state, 7);
  assert(world_landmark_is_cleared(&state, 7));
}

static void test_determinism(void) {
  const uint32_t first = world_golden_hash(0x12345678U);
  const uint32_t again = world_golden_hash(0x12345678U);
  const uint32_t other = world_golden_hash(0x87654321U);
  assert(first == again);
  assert(first != other);
  printf("world hash 12345678: %08lx\n", (unsigned long)first);
}

int main(void) {
  test_regions();
  test_visibility();
  test_landmarks();
  test_determinism();
  puts("world_gen tests passed");
  return 0;
}
