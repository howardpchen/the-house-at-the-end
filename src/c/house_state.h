#pragma once

#include <stdbool.h>
#include <stdint.h>

#define HOUSE_STATE_SCHEMA 1
#define HOUSE_RESOURCE_MAX 999
#define HOUSE_ELAPSED_CAP_SECONDS (6 * 60 * 60)

typedef enum {
  HOUSE_BUILD_GUEST_ROOM = 0,
  HOUSE_BUILD_WORKTABLE = 1,
  HOUSE_BUILD_ANCHOR_LINE = 2,
  HOUSE_BUILD_COUNT = 3
} HouseBuild;

typedef enum {
  HOUSE_ROLE_GATHERER = 0,
  HOUSE_ROLE_LISTENER = 1
} HouseRole;

typedef enum {
  HOUSE_RESULT_OK = 0,
  HOUSE_RESULT_LOCKED,
  HOUSE_RESULT_NO_RESOURCES,
  HOUSE_RESULT_AT_LIMIT,
  HOUSE_RESULT_NOT_ACTIVE,
  HOUSE_RESULT_ENCOUNTER,
  HOUSE_RESULT_COMPLETE,
  HOUSE_RESULT_FAILED
} HouseResult;

enum {
  HOUSE_STORY_FIRST_GUEST = 1 << 0,
  HOUSE_STORY_FIRST_MEMORY = 1 << 1
};

typedef struct {
  int16_t kindling;
  int16_t remnants;
  int16_t rations;
  int16_t clarity;

  uint16_t searches;
  uint16_t gather_elapsed;
  uint16_t listen_elapsed;

  uint8_t hearth_level;
  uint8_t residents;
  uint8_t gatherers;
  uint8_t listeners;
  uint8_t built_mask;
  uint8_t memories;

  uint8_t expedition_active;
  uint8_t expedition_step;
  uint8_t expedition_clarity;
  uint8_t expedition_rations;
  uint8_t expedition_resolve;
  uint8_t encounter_strength;
  uint8_t cargo_remnants;

  uint32_t story_flags;
  int64_t last_updated;
} HouseState;

void house_state_init(HouseState *state, int64_t now);
bool house_state_is_valid(const HouseState *state);
bool house_apply_elapsed(HouseState *state, int64_t now);

HouseResult house_search(HouseState *state);
HouseResult house_tend_hearth(HouseState *state);
HouseResult house_prepare_ration(HouseState *state);

bool house_has_build(const HouseState *state, HouseBuild build);
void house_build_cost(HouseBuild build, int16_t *kindling, int16_t *remnants);
HouseResult house_construct(HouseState *state, HouseBuild build);
HouseResult house_assign(HouseState *state, HouseRole role);

bool house_can_expedition(const HouseState *state);
HouseResult house_start_expedition(HouseState *state);
HouseResult house_expedition_advance(HouseState *state);
HouseResult house_expedition_remember(HouseState *state);
HouseResult house_expedition_retreat(HouseState *state);
