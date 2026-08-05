#include "world_gen.h"

#include <string.h>

static uint32_t prv_mix(uint32_t value) {
  value ^= value >> 16;
  value *= 0x7feb352dU;
  value ^= value >> 15;
  value *= 0x846ca68bU;
  value ^= value >> 16;
  return value;
}

static uint32_t prv_cell_hash(uint32_t seed, int8_t x, int8_t y,
                              uint32_t stream) {
  const uint32_t packed = ((uint32_t)(uint8_t)x << 8) | (uint8_t)y;
  return prv_mix(seed ^ packed ^ stream);
}

static uint16_t prv_index(int8_t x, int8_t y) {
  return (uint16_t)((y + WORLD_CENTER) * WORLD_SIZE +
                    (x + WORLD_CENTER));
}

static uint8_t prv_abs8(int8_t value) {
  return (uint8_t)(value < 0 ? -value : value);
}

bool world_is_in_bounds(int8_t x, int8_t y) {
  return x >= -WORLD_CENTER && x <= WORLD_CENTER &&
         y >= -WORLD_CENTER && y <= WORLD_CENTER;
}

WorldRegion world_region_at(int8_t x, int8_t y) {
  const uint8_t distance = prv_abs8(x) > prv_abs8(y)
      ? prv_abs8(x) : prv_abs8(y);
  if (distance <= 3) {
    return WORLD_REGION_HEARTH;
  }
  if (distance <= 7) {
    return WORLD_REGION_NEAR;
  }
  if (distance <= 10) {
    return WORLD_REGION_CONTRADICTION;
  }
  if (distance <= 13) {
    return WORLD_REGION_MACHINE;
  }
  return WORLD_REGION_THRESHOLD;
}

WorldPoint world_landmark_position(uint32_t seed, uint8_t landmark_id) {
  if (landmark_id >= WORLD_LANDMARK_COUNT) {
    return (WorldPoint){0, 0};
  }
  static const WorldPoint canonical[WORLD_LANDMARK_COUNT] = {
      {2, 0}, {0, 2}, {-2, 0}, {0, -2}, {3, 3},
      {4, 1}, {1, 5}, {-6, 2}, {-2, -7}, {7, -4},
      {8, 1}, {3, 9}, {-10, 4}, {-4, -8}, {9, -6},
      {11, 2}, {4, 12}, {-13, 5}, {-5, -11}, {12, -8},
      {14, 3}, {6, 15}, {-15, 7}, {-8, -14}};
  WorldPoint point = canonical[landmark_id];
  const uint8_t region = landmark_id / 5U;
  const uint8_t transform = (uint8_t)(prv_mix(seed ^
      ((uint32_t)region * 0x9e3779b9U)) & 7U);
  if (transform & 4U) {
    const int8_t swap = point.x;
    point.x = point.y;
    point.y = swap;
  }
  for (uint8_t turn = 0; turn < (transform & 3U); ++turn) {
    const int8_t old_x = point.x;
    point.x = (int8_t)-point.y;
    point.y = old_x;
  }
  return point;
}

WorldTile world_tile_at(uint32_t seed, int8_t x, int8_t y,
                        uint8_t *landmark_id) {
  if (landmark_id) {
    *landmark_id = UINT8_MAX;
  }
  if (!world_is_in_bounds(x, y)) {
    return WORLD_TILE_UNSTABLE;
  }
  if (x == 0 && y == 0) {
    return WORLD_TILE_HOUSE;
  }
  for (uint8_t id = 0; id < WORLD_LANDMARK_COUNT; ++id) {
    const WorldPoint point = world_landmark_position(seed, id);
    if (point.x == x && point.y == y) {
      if (landmark_id) {
        *landmark_id = id;
      }
      return WORLD_TILE_LANDMARK;
    }
  }

  const WorldRegion region = world_region_at(x, y);
  const uint32_t roll = prv_cell_hash(seed, x, y, 0x574f524cU) % 100U;
  if (roll < 8U) {
    return WORLD_TILE_HAZARD;
  }
  if (roll < 13U) {
    return WORLD_TILE_ECHO;
  }
  if (roll < 22U) {
    return WORLD_TILE_EVENT;
  }
  if (roll < 34U) {
    return WORLD_TILE_UNSTABLE;
  }

  switch (region) {
    case WORLD_REGION_HEARTH:
      return roll < 61U ? WORLD_TILE_DOMESTIC : WORLD_TILE_PATH;
    case WORLD_REGION_NEAR:
      if (roll < 53U) {
        return WORLD_TILE_TRANSIT;
      }
      return roll < 71U ? WORLD_TILE_SHORELINE : WORLD_TILE_PATH;
    case WORLD_REGION_CONTRADICTION:
      return roll < 59U ? WORLD_TILE_CIVIC : WORLD_TILE_DOMESTIC;
    case WORLD_REGION_MACHINE:
      return roll < 72U ? WORLD_TILE_MACHINE : WORLD_TILE_PATH;
    case WORLD_REGION_THRESHOLD:
      return roll < 78U ? WORLD_TILE_UNSTABLE : WORLD_TILE_MACHINE;
  }
  return WORLD_TILE_PATH;
}

void world_state_init(WorldState *state, uint32_t seed) {
  if (!state) {
    return;
  }
  memset(state, 0, sizeof(*state));
  state->seed = seed ? seed : 0x484f5553U;
  world_reveal(state, 0, 0);
}

bool world_is_visible(const WorldState *state, int8_t x, int8_t y) {
  if (!state || !world_is_in_bounds(x, y)) {
    return false;
  }
  const uint16_t index = prv_index(x, y);
  return (state->visible[index / 8U] & (1U << (index % 8U))) != 0;
}

void world_reveal(WorldState *state, int8_t x, int8_t y) {
  if (!state) {
    return;
  }
  static const int8_t offsets[][2] = {
      {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
  for (size_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); ++i) {
    const int8_t reveal_x = x + offsets[i][0];
    const int8_t reveal_y = y + offsets[i][1];
    if (world_is_in_bounds(reveal_x, reveal_y)) {
      const uint16_t index = prv_index(reveal_x, reveal_y);
      state->visible[index / 8U] |= (uint8_t)(1U << (index % 8U));
    }
  }
}

bool world_landmark_is_cleared(const WorldState *state, uint8_t landmark_id) {
  return state && landmark_id < WORLD_LANDMARK_COUNT &&
         (state->cleared[landmark_id / 8U] &
          (1U << (landmark_id % 8U))) != 0;
}

void world_clear_landmark(WorldState *state, uint8_t landmark_id) {
  if (state && landmark_id < WORLD_LANDMARK_COUNT) {
    state->cleared[landmark_id / 8U] |=
        (uint8_t)(1U << (landmark_id % 8U));
  }
}

uint32_t world_golden_hash(uint32_t seed) {
  uint32_t hash = 2166136261U;
  for (int8_t y = -WORLD_CENTER; y <= WORLD_CENTER; ++y) {
    for (int8_t x = -WORLD_CENTER; x <= WORLD_CENTER; ++x) {
      const uint8_t tile = (uint8_t)world_tile_at(seed, x, y, NULL);
      hash ^= tile;
      hash *= 16777619U;
    }
  }
  return hash;
}
