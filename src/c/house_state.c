#include "house_state.h"

#include <stddef.h>
#include <string.h>

#define BUILD_BIT(build) (1U << (build))

static int16_t prv_add_capped(int16_t value, int32_t amount) {
  const int32_t result = value + amount;
  return (int16_t)(result > HOUSE_RESOURCE_MAX ? HOUSE_RESOURCE_MAX : result);
}

static void prv_add_gather_yields(HouseState *state, uint32_t kindling) {
  state->kindling = prv_add_capped(state->kindling, (int32_t)kindling);
  const uint32_t progress = state->gather_progress + kindling;
  state->remnants = prv_add_capped(
      state->remnants, (int32_t)(progress / HOUSE_KINDLING_PER_REMNANT));
  state->gather_progress =
      (uint16_t)(progress % HOUSE_KINDLING_PER_REMNANT);
}

static void prv_clear_expedition(HouseState *state) {
  state->expedition_active = 0;
  state->expedition_step = 0;
  state->expedition_clarity = 0;
  state->expedition_rations = 0;
  state->expedition_resolve = 0;
  state->encounter_strength = 0;
  state->cargo_remnants = 0;
}

static HouseResult prv_fail_expedition(HouseState *state) {
  prv_clear_expedition(state);
  return HOUSE_RESULT_FAILED;
}

void house_state_init(HouseState *state, int64_t now) {
  if (!state) {
    return;
  }

  memset(state, 0, sizeof(*state));
  state->kindling = 3;
  state->last_updated = now;
}

bool house_state_is_valid(const HouseState *state) {
  if (!state) {
    return false;
  }

  if (state->kindling < 0 || state->kindling > HOUSE_RESOURCE_MAX ||
      state->remnants < 0 || state->remnants > HOUSE_RESOURCE_MAX ||
      state->rations < 0 || state->rations > HOUSE_RESOURCE_MAX ||
      state->clarity < 0 || state->clarity > HOUSE_RESOURCE_MAX) {
    return false;
  }

  if (state->gather_progress >= HOUSE_KINDLING_PER_REMNANT ||
      state->hearth_level > HOUSE_HEARTH_MAX ||
      state->hearth_elapsed >= HOUSE_HEARTH_DECAY_SECONDS ||
      (state->hearth_level == 0 && state->hearth_elapsed != 0) ||
      state->residents > 2 || state->memories > 1 ||
      state->gatherers + state->listeners != state->residents ||
      state->built_mask >= BUILD_BIT(HOUSE_BUILD_COUNT)) {
    return false;
  }

  if (state->expedition_clarity > 4 || state->expedition_rations > 2 ||
      state->expedition_resolve > 4 || state->encounter_strength > 2 ||
      state->expedition_step > 4) {
    return false;
  }
  if (!state->expedition_active &&
      (state->expedition_clarity != 0 || state->expedition_rations != 0 ||
       state->expedition_resolve != 0 || state->encounter_strength != 0 ||
       state->expedition_step != 0 || state->cargo_remnants != 0)) {
    return false;
  }

  return true;
}

bool house_apply_elapsed(HouseState *state, int64_t now) {
  if (!state || now <= state->last_updated) {
    return false;
  }

  int64_t elapsed = now - state->last_updated;
  if (elapsed > HOUSE_ELAPSED_CAP_SECONDS) {
    elapsed = HOUSE_ELAPSED_CAP_SECONDS;
  }
  state->last_updated = now;

  uint32_t guest_elapsed = 0;
  if (state->hearth_level >= HOUSE_HEARTH_SEEN) {
    const uint32_t seconds_until_unseen =
        HOUSE_HEARTH_DECAY_SECONDS - state->hearth_elapsed +
        (state->hearth_level - HOUSE_HEARTH_SEEN) *
            HOUSE_HEARTH_DECAY_SECONDS;
    guest_elapsed = (uint32_t)elapsed < seconds_until_unseen
        ? (uint32_t)elapsed : seconds_until_unseen;
  }

  bool changed = false;
  if (state->hearth_level > 0) {
    const uint32_t total = state->hearth_elapsed + (uint32_t)elapsed;
    const uint32_t decay = total / HOUSE_HEARTH_DECAY_SECONDS;
    if (decay >= state->hearth_level) {
      state->hearth_level = 0;
      state->hearth_elapsed = 0;
      changed = true;
    } else {
      state->hearth_elapsed =
          (uint8_t)(total % HOUSE_HEARTH_DECAY_SECONDS);
      if (decay > 0) {
        state->hearth_level -= (uint8_t)decay;
        changed = true;
      }
    }
  } else {
    state->hearth_elapsed = 0;
  }

  if (state->gatherers > 0 && guest_elapsed > 0) {
    const uint32_t total = state->gather_elapsed + guest_elapsed;
    const uint32_t cycles = total / 30U;
    state->gather_elapsed = (uint16_t)(total % 30U);
    if (cycles > 0) {
      prv_add_gather_yields(state, cycles * state->gatherers);
      changed = true;
    }
  } else if (state->gatherers == 0) {
    state->gather_elapsed = 0;
  }

  if (state->listeners > 0 && guest_elapsed > 0) {
    const uint32_t total = state->listen_elapsed + guest_elapsed;
    const uint32_t cycles = total / 45U;
    state->listen_elapsed = (uint16_t)(total % 45U);
    if (cycles > 0) {
      state->clarity = prv_add_capped(
          state->clarity, (int32_t)cycles * state->listeners);
      changed = true;
    }
  } else if (state->listeners == 0) {
    state->listen_elapsed = 0;
  }

  return changed;
}

HouseResult house_search(HouseState *state) {
  if (!state) {
    return HOUSE_RESULT_LOCKED;
  }

  prv_add_gather_yields(state, 1);
  return HOUSE_RESULT_OK;
}

HouseResult house_check_tend_hearth(const HouseState *state) {
  if (!state) {
    return HOUSE_RESULT_LOCKED;
  }
  if (state->hearth_level >= HOUSE_HEARTH_MAX) {
    return HOUSE_RESULT_AT_LIMIT;
  }
  if (state->kindling < 2) {
    return HOUSE_RESULT_NO_RESOURCES;
  }

  return HOUSE_RESULT_OK;
}

HouseResult house_tend_hearth(HouseState *state) {
  const HouseResult check = house_check_tend_hearth(state);
  if (check != HOUSE_RESULT_OK) {
    return check;
  }

  state->kindling -= 2;
  state->hearth_level++;
  state->hearth_elapsed = 0;
  if (state->hearth_level >= HOUSE_HEARTH_SEEN && state->residents == 0) {
    state->residents = 1;
    state->gatherers = 1;
    state->story_flags |= HOUSE_STORY_FIRST_GUEST;
  }
  return HOUSE_RESULT_OK;
}

HouseResult house_check_prepare_ration(const HouseState *state) {
  if (!state || state->hearth_level < HOUSE_HEARTH_HELD ||
      !house_has_build(state, HOUSE_BUILD_WORKTABLE)) {
    return HOUSE_RESULT_LOCKED;
  }
  if (state->kindling < 1 || state->remnants < 1) {
    return HOUSE_RESULT_NO_RESOURCES;
  }

  return HOUSE_RESULT_OK;
}

HouseResult house_prepare_ration(HouseState *state) {
  const HouseResult check = house_check_prepare_ration(state);
  if (check != HOUSE_RESULT_OK) {
    return check;
  }

  state->kindling--;
  state->remnants--;
  state->rations = prv_add_capped(state->rations, 1);
  return HOUSE_RESULT_OK;
}

bool house_has_build(const HouseState *state, HouseBuild build) {
  return state && build < HOUSE_BUILD_COUNT &&
         (state->built_mask & BUILD_BIT(build));
}

void house_build_cost(HouseBuild build, int16_t *kindling, int16_t *remnants) {
  int16_t cost_kindling = 0;
  int16_t cost_remnants = 0;
  switch (build) {
    case HOUSE_BUILD_GUEST_ROOM:
      cost_kindling = 8;
      cost_remnants = 4;
      break;
    case HOUSE_BUILD_WORKTABLE:
      cost_kindling = 6;
      cost_remnants = 5;
      break;
    case HOUSE_BUILD_ANCHOR_LINE:
      cost_kindling = 10;
      cost_remnants = 8;
      break;
    case HOUSE_BUILD_COUNT:
      break;
  }
  if (kindling) {
    *kindling = cost_kindling;
  }
  if (remnants) {
    *remnants = cost_remnants;
  }
}

HouseResult house_construct(HouseState *state, HouseBuild build) {
  if (!state || build >= HOUSE_BUILD_COUNT ||
      state->hearth_level < HOUSE_HEARTH_HELD) {
    return HOUSE_RESULT_LOCKED;
  }
  if (build == HOUSE_BUILD_ANCHOR_LINE &&
      state->hearth_level < HOUSE_HEARTH_SHARED) {
    return HOUSE_RESULT_LOCKED;
  }
  if (house_has_build(state, build)) {
    return HOUSE_RESULT_AT_LIMIT;
  }
  if (build == HOUSE_BUILD_ANCHOR_LINE &&
      (!house_has_build(state, HOUSE_BUILD_GUEST_ROOM) ||
       !house_has_build(state, HOUSE_BUILD_WORKTABLE))) {
    return HOUSE_RESULT_LOCKED;
  }

  int16_t kindling = 0;
  int16_t remnants = 0;
  house_build_cost(build, &kindling, &remnants);
  if (state->kindling < kindling || state->remnants < remnants) {
    return HOUSE_RESULT_NO_RESOURCES;
  }

  state->kindling -= kindling;
  state->remnants -= remnants;
  state->built_mask |= BUILD_BIT(build);
  if (build == HOUSE_BUILD_GUEST_ROOM && state->residents < 2) {
    state->residents++;
    state->listeners++;
  }
  return HOUSE_RESULT_OK;
}

HouseResult house_assign(HouseState *state, HouseRole role) {
  if (!state || state->hearth_level < HOUSE_HEARTH_SHARED ||
      state->residents == 0) {
    return HOUSE_RESULT_LOCKED;
  }

  if (role == HOUSE_ROLE_GATHERER) {
    if (state->listeners == 0) {
      return HOUSE_RESULT_AT_LIMIT;
    }
    state->listeners--;
    state->gatherers++;
  } else if (role == HOUSE_ROLE_LISTENER) {
    if (state->gatherers == 0) {
      return HOUSE_RESULT_AT_LIMIT;
    }
    state->gatherers--;
    state->listeners++;
  } else {
    return HOUSE_RESULT_LOCKED;
  }
  return HOUSE_RESULT_OK;
}

bool house_can_expedition(const HouseState *state) {
  return state && house_has_build(state, HOUSE_BUILD_ANCHOR_LINE) &&
         state->hearth_level >= HOUSE_HEARTH_SHARED &&
         state->rations >= 2 && state->clarity >= 4 &&
         !state->expedition_active;
}

HouseResult house_start_expedition(HouseState *state) {
  if (!state || state->hearth_level < HOUSE_HEARTH_SHARED ||
      !house_has_build(state, HOUSE_BUILD_ANCHOR_LINE)) {
    return HOUSE_RESULT_LOCKED;
  }
  if (state->expedition_active) {
    return HOUSE_RESULT_AT_LIMIT;
  }
  if (state->rations < 2 || state->clarity < 4) {
    return HOUSE_RESULT_NO_RESOURCES;
  }

  state->rations -= 2;
  state->clarity -= 4;
  state->expedition_active = 1;
  state->expedition_step = 0;
  state->expedition_clarity = 4;
  state->expedition_rations = 2;
  state->expedition_resolve = 4;
  state->encounter_strength = 0;
  state->cargo_remnants = 0;
  return HOUSE_RESULT_OK;
}

HouseResult house_expedition_advance(HouseState *state) {
  if (!state || !state->expedition_active) {
    return HOUSE_RESULT_NOT_ACTIVE;
  }
  if (state->encounter_strength > 0) {
    return HOUSE_RESULT_LOCKED;
  }
  if (state->expedition_clarity == 0) {
    return prv_fail_expedition(state);
  }

  const uint8_t next_step = state->expedition_step + 1;
  if (next_step % 2U == 0) {
    if (state->expedition_rations == 0) {
      return prv_fail_expedition(state);
    }
    state->expedition_rations--;
  }
  state->expedition_clarity--;
  state->expedition_step = next_step;

  switch (state->expedition_step) {
    case 1:
      state->cargo_remnants++;
      return HOUSE_RESULT_OK;
    case 2:
      state->encounter_strength = 2;
      return HOUSE_RESULT_ENCOUNTER;
    case 3:
      state->cargo_remnants++;
      return HOUSE_RESULT_OK;
    case 4:
      state->remnants = prv_add_capped(state->remnants,
                                       state->cargo_remnants + 1);
      if (!(state->story_flags & HOUSE_STORY_FIRST_MEMORY)) {
        state->memories++;
      }
      state->story_flags |= HOUSE_STORY_FIRST_MEMORY;
      prv_clear_expedition(state);
      return HOUSE_RESULT_COMPLETE;
    default:
      return prv_fail_expedition(state);
  }
}

HouseResult house_expedition_remember(HouseState *state) {
  if (!state || !state->expedition_active) {
    return HOUSE_RESULT_NOT_ACTIVE;
  }
  if (state->encounter_strength == 0) {
    return HOUSE_RESULT_LOCKED;
  }

  state->encounter_strength--;
  if (state->encounter_strength == 0) {
    state->cargo_remnants += 2;
    return HOUSE_RESULT_OK;
  }

  if (state->expedition_resolve > 0) {
    state->expedition_resolve--;
  }
  if (state->expedition_resolve == 0) {
    return prv_fail_expedition(state);
  }
  return HOUSE_RESULT_ENCOUNTER;
}

HouseResult house_expedition_retreat(HouseState *state) {
  if (!state || !state->expedition_active) {
    return HOUSE_RESULT_NOT_ACTIVE;
  }

  state->remnants = prv_add_capped(state->remnants,
                                   state->cargo_remnants / 2);
  prv_clear_expedition(state);
  return HOUSE_RESULT_OK;
}
