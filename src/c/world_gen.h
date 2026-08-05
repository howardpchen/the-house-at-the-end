#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define WORLD_SIZE 31
#define WORLD_CENTER (WORLD_SIZE / 2)
#define WORLD_TILE_COUNT (WORLD_SIZE * WORLD_SIZE)
#define WORLD_VISIBILITY_BYTES ((WORLD_TILE_COUNT + 7) / 8)
#define WORLD_LANDMARK_COUNT 24
#define WORLD_LANDMARK_BYTES ((WORLD_LANDMARK_COUNT + 7) / 8)

typedef enum {
  WORLD_TILE_PATH = 0,
  WORLD_TILE_UNSTABLE,
  WORLD_TILE_DOMESTIC,
  WORLD_TILE_TRANSIT,
  WORLD_TILE_CIVIC,
  WORLD_TILE_SHORELINE,
  WORLD_TILE_MACHINE,
  WORLD_TILE_HAZARD,
  WORLD_TILE_ECHO,
  WORLD_TILE_EVENT,
  WORLD_TILE_LANDMARK,
  WORLD_TILE_HOUSE
} WorldTile;

typedef enum {
  WORLD_REGION_HEARTH = 0,
  WORLD_REGION_NEAR,
  WORLD_REGION_CONTRADICTION,
  WORLD_REGION_MACHINE,
  WORLD_REGION_THRESHOLD
} WorldRegion;

typedef struct {
  int8_t x;
  int8_t y;
} WorldPoint;

typedef struct {
  uint32_t seed;
  uint8_t visible[WORLD_VISIBILITY_BYTES];
  uint8_t cleared[WORLD_LANDMARK_BYTES];
} WorldState;

void world_state_init(WorldState *state, uint32_t seed);
bool world_is_in_bounds(int8_t x, int8_t y);
WorldRegion world_region_at(int8_t x, int8_t y);
WorldTile world_tile_at(uint32_t seed, int8_t x, int8_t y,
                        uint8_t *landmark_id);
WorldPoint world_landmark_position(uint32_t seed, uint8_t landmark_id);
bool world_is_visible(const WorldState *state, int8_t x, int8_t y);
void world_reveal(WorldState *state, int8_t x, int8_t y);
bool world_landmark_is_cleared(const WorldState *state, uint8_t landmark_id);
void world_clear_landmark(WorldState *state, uint8_t landmark_id);
uint32_t world_golden_hash(uint32_t seed);

