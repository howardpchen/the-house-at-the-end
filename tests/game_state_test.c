#include "game_state.h"

#include <assert.h>
#include <stdio.h>

static GameState movement_two(void) {
  GameState state;
  game_state_init(&state, 1000, 42U);
  state.story.movement = GAME_MOVEMENT_FRAGMENTS;
  return state;
}

static void complete_range(GameState *state, uint8_t first, uint8_t last) {
  for (uint8_t id = first; id <= last; ++id) {
    game_state_complete_landmark(state, id);
  }
}

static void test_movement_progression(void) {
  GameState state = movement_two();
  complete_range(&state, 5, 8);
  assert(state.story.movement == GAME_MOVEMENT_FRAGMENTS);
  assert(game_state_complete_landmark(&state, 9));
  assert(state.story.movement == GAME_MOVEMENT_CONTRADICTIONS);
  assert(state.story.keys == 4);
  assert(state.story.facilities & (1U << GAME_FACILITY_ARCHIVE));

  complete_range(&state, 10, 14);
  assert(state.story.movement == GAME_MOVEMENT_MACHINERY);
  assert(state.story.facilities & (1U << GAME_FACILITY_TERMINAL));
  complete_range(&state, 15, 19);
  assert(state.story.movement == GAME_MOVEMENT_FINAL_DOOR);
  assert(state.story.facilities & (1U << GAME_FACILITY_DOORFRAME));
}

static void test_all_endings_reachable(void) {
  for (uint8_t ending = 1; ending <= 3; ++ending) {
    GameState state = movement_two();
    complete_range(&state, 5, 19);
    complete_range(&state, 20, 23);
    assert(game_state_choose_ending(&state, ending));
    assert(state.story.ending == ending);
    assert(state.story.movement == GAME_MOVEMENT_COMPLETE);
    assert(game_state_is_valid(&state));
  }
}

int main(void) {
  test_movement_progression();
  test_all_endings_reachable();
  puts("game_state tests passed");
  return 0;
}
