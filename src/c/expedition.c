#include "expedition.h"

#include <string.h>

static uint8_t prv_max_distance(const GameState *state) {
  switch ((GameMovement)state->story.movement) {
    case GAME_MOVEMENT_WARMTH:
      return 3;
    case GAME_MOVEMENT_FRAGMENTS:
      return 7;
    case GAME_MOVEMENT_CONTRADICTIONS:
      return 10;
    case GAME_MOVEMENT_MACHINERY:
      return 13;
    case GAME_MOVEMENT_FINAL_DOOR:
    case GAME_MOVEMENT_COMPLETE:
      return 15;
  }
  return 3;
}

static uint8_t prv_distance(int8_t x, int8_t y) {
  const uint8_t abs_x = (uint8_t)(x < 0 ? -x : x);
  const uint8_t abs_y = (uint8_t)(y < 0 ? -y : y);
  return abs_x > abs_y ? abs_x : abs_y;
}

static void prv_clear(GameExpedition *expedition) {
  memset(expedition, 0, sizeof(*expedition));
}

ExpeditionResult expedition_start(GameState *state) {
  if (!state || state->story.movement < GAME_MOVEMENT_FRAGMENTS ||
      !house_has_build(&state->house, HOUSE_BUILD_ANCHOR_LINE) ||
      state->house.hearth_level < HOUSE_HEARTH_SHARED) {
    return EXPEDITION_RESULT_LOCKED;
  }
  if (state->expedition.active) {
    return EXPEDITION_RESULT_OK;
  }
  if (state->house.clarity < 4 || state->house.rations < 2) {
    return EXPEDITION_RESULT_NO_RESOURCES;
  }
  const uint16_t clarity_capacity = (uint16_t)(12U +
      (state->story.movement - GAME_MOVEMENT_FRAGMENTS) * 6U);
  const uint16_t ration_capacity = (uint16_t)(4U +
      (state->story.movement - GAME_MOVEMENT_FRAGMENTS) * 2U);
  const uint16_t loaded_clarity = state->house.clarity > clarity_capacity
      ? clarity_capacity : (uint16_t)state->house.clarity;
  const uint16_t loaded_rations = state->house.rations > ration_capacity
      ? ration_capacity : (uint16_t)state->house.rations;
  state->house.clarity -= (int16_t)loaded_clarity;
  state->house.rations -= (int16_t)loaded_rations;
  prv_clear(&state->expedition);
  state->expedition.active = 1;
  state->expedition.clarity = loaded_clarity;
  state->expedition.rations = loaded_rations;
  state->expedition.resolve = (uint8_t)(6U +
      state->story.movement - GAME_MOVEMENT_FRAGMENTS);
  world_reveal(&state->world, 0, 0);
  return EXPEDITION_RESULT_OK;
}

ExpeditionResult expedition_move(GameState *state,
                                 ExpeditionDirection direction,
                                 uint8_t *landmark_id) {
  if (landmark_id) {
    *landmark_id = UINT8_MAX;
  }
  if (!state || !state->expedition.active) {
    return EXPEDITION_RESULT_LOCKED;
  }
  int8_t next_x = state->expedition.x;
  int8_t next_y = state->expedition.y;
  switch (direction) {
    case EXPEDITION_NORTH:
      next_y--;
      break;
    case EXPEDITION_EAST:
      next_x++;
      break;
    case EXPEDITION_SOUTH:
      next_y++;
      break;
    case EXPEDITION_WEST:
      next_x--;
      break;
  }
  if (!world_is_in_bounds(next_x, next_y) ||
      prv_distance(next_x, next_y) > prv_max_distance(state)) {
    return EXPEDITION_RESULT_BOUNDARY;
  }
  if (state->expedition.clarity == 0) {
    prv_clear(&state->expedition);
    return EXPEDITION_RESULT_FAILED;
  }
  const uint8_t next_step = (uint8_t)(state->expedition.steps + 1U);
  if (next_step % 4U == 0) {
    if (state->expedition.rations == 0) {
      prv_clear(&state->expedition);
      return EXPEDITION_RESULT_FAILED;
    }
    state->expedition.rations--;
  }
  state->expedition.clarity--;
  state->expedition.steps = next_step;
  state->expedition.x = next_x;
  state->expedition.y = next_y;
  world_reveal(&state->world, next_x, next_y);

  uint8_t found_landmark = UINT8_MAX;
  const WorldTile tile = world_tile_at(state->world.seed, next_x, next_y,
                                       &found_landmark);
  if (tile == WORLD_TILE_HAZARD || tile == WORLD_TILE_ECHO) {
    const uint8_t damage = tile == WORLD_TILE_ECHO ? 2 : 1;
    state->expedition.resolve = state->expedition.resolve > damage
        ? (uint8_t)(state->expedition.resolve - damage) : 0;
    if (state->expedition.resolve == 0) {
      prv_clear(&state->expedition);
      return EXPEDITION_RESULT_FAILED;
    }
    return EXPEDITION_RESULT_HIT;
  }
  if (tile == WORLD_TILE_EVENT) {
    return EXPEDITION_RESULT_EVENT;
  }
  if (tile == WORLD_TILE_LANDMARK &&
      !world_landmark_is_cleared(&state->world, found_landmark)) {
    if (landmark_id) {
      *landmark_id = found_landmark;
    }
    return EXPEDITION_RESULT_LANDMARK;
  }
  if (tile == WORLD_TILE_DOMESTIC || tile == WORLD_TILE_TRANSIT ||
      tile == WORLD_TILE_CIVIC || tile == WORLD_TILE_MACHINE) {
    if (state->expedition.cargo < 9) {
      state->expedition.cargo++;
    }
  }
  return EXPEDITION_RESULT_OK;
}

ExpeditionResult expedition_return(GameState *state) {
  if (!state || !state->expedition.active) {
    return EXPEDITION_RESULT_LOCKED;
  }
  int32_t remnants = state->house.remnants + state->expedition.cargo;
  state->house.remnants = (int16_t)(remnants > HOUSE_RESOURCE_MAX
      ? HOUSE_RESOURCE_MAX : remnants);
  prv_clear(&state->expedition);
  return EXPEDITION_RESULT_RETURNED;
}
