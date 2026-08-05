#include "expedition.h"

#include <assert.h>
#include <stdio.h>

static GameState ready_state(void) {
  GameState state;
  game_state_init(&state, 1000, 0x12345678U);
  state.story.movement = GAME_MOVEMENT_FRAGMENTS;
  state.house.built_mask |= 1U << HOUSE_BUILD_ANCHOR_LINE;
  state.house.hearth_level = HOUSE_HEARTH_SHARED;
  state.house.clarity = 20;
  state.house.rations = 10;
  return state;
}

static void test_start_move_and_return(void) {
  GameState state = ready_state();
  assert(expedition_start(&state) == EXPEDITION_RESULT_OK);
  assert(state.house.clarity == 8 && state.house.rations == 6);
  assert(state.expedition.active && state.expedition.resolve == 6);
  assert(expedition_move(&state, EXPEDITION_EAST, NULL) !=
         EXPEDITION_RESULT_LOCKED);
  assert(state.expedition.x == 1 && state.expedition.clarity == 11);
  state.expedition.cargo = 3;
  assert(expedition_return(&state) == EXPEDITION_RESULT_RETURNED);
  assert(!state.expedition.active && state.house.remnants == 3);
  assert(game_state_is_valid(&state));
}

static void test_movement_gate(void) {
  GameState state = ready_state();
  assert(expedition_start(&state) == EXPEDITION_RESULT_OK);
  state.expedition.clarity = 30;
  for (uint8_t step = 0; step < 7; ++step) {
    assert(expedition_move(&state, EXPEDITION_EAST, NULL) !=
           EXPEDITION_RESULT_BOUNDARY);
    state.expedition.rations = 20;
  }
  assert(expedition_move(&state, EXPEDITION_EAST, NULL) ==
         EXPEDITION_RESULT_BOUNDARY);
  assert(state.expedition.x == 7);
}

static void test_supply_failure(void) {
  GameState state = ready_state();
  assert(expedition_start(&state) == EXPEDITION_RESULT_OK);
  state.expedition.clarity = 0;
  assert(expedition_move(&state, EXPEDITION_NORTH, NULL) ==
         EXPEDITION_RESULT_FAILED);
  assert(!state.expedition.active);
}

int main(void) {
  test_start_move_and_return();
  test_movement_gate();
  test_supply_failure();
  puts("expedition tests passed");
  return 0;
}
