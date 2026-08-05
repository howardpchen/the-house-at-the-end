#include "game_state.h"

#include <string.h>

void game_state_init(GameState *state, int64_t now, uint32_t world_seed) {
  if (!state) {
    return;
  }
  memset(state, 0, sizeof(*state));
  house_state_init(&state->house, now);
  world_state_init(&state->world, world_seed);
  state->story.movement = GAME_MOVEMENT_WARMTH;
}

void game_state_migrate_legacy(GameState *state, const HouseState *legacy,
                               uint32_t world_seed) {
  if (!state || !legacy) {
    return;
  }
  game_state_init(state, legacy->last_updated, world_seed);
  state->house = *legacy;
  state->story.facilities = legacy->built_mask;
  state->story.movement = (legacy->story_flags & HOUSE_STORY_FIRST_MEMORY)
      ? GAME_MOVEMENT_FRAGMENTS : GAME_MOVEMENT_WARMTH;
  if (legacy->residents > 0) {
    state->guests.guest[GAME_GUEST_MARA].present = 1;
    state->guests.guest[GAME_GUEST_MARA].role = GAME_ROLE_GATHERER;
    state->guests.guest[GAME_GUEST_MARA].trust = 1;
  }
}

bool game_state_is_valid(const GameState *state) {
  if (!state || !house_state_is_valid(&state->house) ||
      state->story.movement < GAME_MOVEMENT_WARMTH ||
      state->story.movement > GAME_MOVEMENT_COMPLETE ||
      state->story.thread > HOUSE_RESOURCE_MAX ||
      state->story.ending > 3 ||
      state->expedition.resolve > 20 ||
      (state->expedition.active &&
       !world_is_in_bounds(state->expedition.x, state->expedition.y))) {
    return false;
  }
  for (uint8_t id = 0; id < GAME_GUEST_COUNT; ++id) {
    const GameGuest *guest = &state->guests.guest[id];
    if (guest->present > 1 || guest->role > GAME_ROLE_WITNESS ||
        guest->trust > 3) {
      return false;
    }
  }
  return true;
}

static bool prv_band_cleared(const GameState *state, uint8_t first,
                             uint8_t last) {
  for (uint8_t id = first; id <= last; ++id) {
    if (!world_landmark_is_cleared(&state->world, id)) {
      return false;
    }
  }
  return true;
}

bool game_state_complete_landmark(GameState *state, uint8_t landmark_id) {
  if (!state || landmark_id >= WORLD_LANDMARK_COUNT) {
    return false;
  }
  world_clear_landmark(&state->world, landmark_id);
  if (state->story.movement == GAME_MOVEMENT_FRAGMENTS &&
      prv_band_cleared(state, 5, 9)) {
    state->story.movement = GAME_MOVEMENT_CONTRADICTIONS;
    if (state->story.keys < 4) {
      state->story.keys = 4;
    }
    state->story.facilities |=
        (1U << GAME_FACILITY_LOOM) |
        (1U << GAME_FACILITY_ARCHIVE) |
        (1U << GAME_FACILITY_INFIRMARY);
    return true;
  }
  if (state->story.movement == GAME_MOVEMENT_CONTRADICTIONS &&
      prv_band_cleared(state, 10, 14)) {
    state->story.movement = GAME_MOVEMENT_MACHINERY;
    state->story.facilities |=
        (1U << GAME_FACILITY_SIGNAL_WALL) |
        (1U << GAME_FACILITY_ROOT_STAIR) |
        (1U << GAME_FACILITY_TERMINAL);
    return true;
  }
  if (state->story.movement == GAME_MOVEMENT_MACHINERY &&
      prv_band_cleared(state, 15, 19)) {
    state->story.movement = GAME_MOVEMENT_FINAL_DOOR;
    if (state->story.keys < UINT8_MAX) {
      state->story.keys++;
    }
    state->story.facilities |= 1U << GAME_FACILITY_DOORFRAME;
    return true;
  }
  return false;
}

bool game_state_choose_ending(GameState *state, uint8_t ending) {
  if (!state || state->story.movement != GAME_MOVEMENT_FINAL_DOOR ||
      ending < 1 || ending > 3 || !prv_band_cleared(state, 20, 23)) {
    return false;
  }
  state->story.ending = ending;
  state->story.movement = GAME_MOVEMENT_COMPLETE;
  return true;
}
