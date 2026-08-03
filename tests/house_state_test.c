#include "house_state.h"

#include <assert.h>
#include <stdio.h>

static void test_opening_and_first_guest(void) {
  HouseState state;
  house_state_init(&state, 1000);
  assert(house_state_is_valid(&state));
  assert(state.kindling == 3);

  assert(house_search(&state) == HOUSE_RESULT_OK);
  assert(house_tend_hearth(&state) == HOUSE_RESULT_OK);
  assert(state.hearth_level == 1);
  assert(state.residents == 0);

  assert(house_search(&state) == HOUSE_RESULT_OK);
  assert(house_tend_hearth(&state) == HOUSE_RESULT_OK);
  assert(state.hearth_level == 2);
  assert(state.residents == 1);
  assert(state.gatherers == 1);
  assert(state.story_flags & HOUSE_STORY_FIRST_GUEST);
  assert(house_state_is_valid(&state));
}

static void test_search_and_elapsed_production(void) {
  HouseState state;
  house_state_init(&state, 2000);
  house_search(&state);
  house_search(&state);
  house_search(&state);
  assert(state.kindling == 6);
  assert(state.remnants == 1);

  state.residents = 2;
  state.gatherers = 1;
  state.listeners = 1;
  const int16_t starting_kindling = state.kindling;
  assert(house_apply_elapsed(&state, 2090));
  assert(state.kindling == starting_kindling + 3);
  assert(state.clarity == 2);
  assert(!house_apply_elapsed(&state, 2090));
  assert(!house_apply_elapsed(&state, 2080));
  assert(house_state_is_valid(&state));
}

static void test_construction_and_assignments(void) {
  HouseState state;
  house_state_init(&state, 3000);
  state.kindling = 100;
  state.remnants = 100;
  state.residents = 1;
  state.gatherers = 1;

  assert(house_construct(&state, HOUSE_BUILD_ANCHOR_LINE) ==
         HOUSE_RESULT_LOCKED);
  assert(house_construct(&state, HOUSE_BUILD_GUEST_ROOM) == HOUSE_RESULT_OK);
  assert(state.residents == 2);
  assert(state.listeners == 1);
  assert(house_construct(&state, HOUSE_BUILD_WORKTABLE) == HOUSE_RESULT_OK);
  assert(house_construct(&state, HOUSE_BUILD_ANCHOR_LINE) == HOUSE_RESULT_OK);
  assert(house_has_build(&state, HOUSE_BUILD_ANCHOR_LINE));

  assert(house_assign(&state, HOUSE_ROLE_GATHERER) == HOUSE_RESULT_OK);
  assert(state.gatherers == 2 && state.listeners == 0);
  assert(house_assign(&state, HOUSE_ROLE_GATHERER) == HOUSE_RESULT_AT_LIMIT);
  assert(house_assign(&state, HOUSE_ROLE_LISTENER) == HOUSE_RESULT_OK);
  assert(state.gatherers == 1 && state.listeners == 1);
  assert(house_state_is_valid(&state));
}

static void test_first_expedition(void) {
  HouseState state;
  house_state_init(&state, 4000);
  state.built_mask = (1U << HOUSE_BUILD_GUEST_ROOM) |
                     (1U << HOUSE_BUILD_WORKTABLE) |
                     (1U << HOUSE_BUILD_ANCHOR_LINE);
  state.rations = 2;
  state.clarity = 4;

  assert(house_start_expedition(&state) == HOUSE_RESULT_OK);
  assert(state.rations == 0 && state.clarity == 0);
  assert(house_expedition_advance(&state) == HOUSE_RESULT_OK);
  assert(state.expedition_step == 1 && state.cargo_remnants == 1);
  assert(house_expedition_advance(&state) == HOUSE_RESULT_ENCOUNTER);
  assert(state.encounter_strength == 2);
  assert(house_expedition_advance(&state) == HOUSE_RESULT_LOCKED);

  assert(house_expedition_remember(&state) == HOUSE_RESULT_ENCOUNTER);
  assert(state.expedition_resolve == 3);
  assert(house_expedition_remember(&state) == HOUSE_RESULT_OK);
  assert(state.cargo_remnants == 3);

  assert(house_expedition_advance(&state) == HOUSE_RESULT_OK);
  assert(house_expedition_advance(&state) == HOUSE_RESULT_COMPLETE);
  assert(!state.expedition_active);
  assert(state.memories == 1);
  assert(state.remnants == 5);
  assert(state.story_flags & HOUSE_STORY_FIRST_MEMORY);
  assert(house_state_is_valid(&state));
}

static void test_retreat_and_failure(void) {
  HouseState state;
  house_state_init(&state, 5000);
  state.built_mask = 1U << HOUSE_BUILD_ANCHOR_LINE;
  state.rations = 4;
  state.clarity = 8;

  assert(house_start_expedition(&state) == HOUSE_RESULT_OK);
  assert(house_expedition_advance(&state) == HOUSE_RESULT_OK);
  assert(house_expedition_retreat(&state) == HOUSE_RESULT_OK);
  assert(!state.expedition_active);
  assert(state.remnants == 0);

  assert(house_start_expedition(&state) == HOUSE_RESULT_OK);
  state.expedition_clarity = 0;
  state.cargo_remnants = 8;
  assert(house_expedition_advance(&state) == HOUSE_RESULT_FAILED);
  assert(!state.expedition_active);
  assert(state.remnants == 0);
  assert(house_state_is_valid(&state));
}

static void test_invalid_inactive_expedition_state(void) {
  HouseState state;
  house_state_init(&state, 6000);
  state.expedition_clarity = 1;
  assert(!house_state_is_valid(&state));
}

int main(void) {
  test_opening_and_first_guest();
  test_search_and_elapsed_production();
  test_construction_and_assignments();
  test_first_expedition();
  test_retreat_and_failure();
  test_invalid_inactive_expedition_state();
  puts("house_state tests passed");
  return 0;
}
