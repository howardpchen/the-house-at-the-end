#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "house_state.h"
#include "world_gen.h"

#define GAME_STATE_SCHEMA 4
#define GAME_GUEST_COUNT 4

typedef enum {
  GAME_MOVEMENT_WARMTH = 1,
  GAME_MOVEMENT_FRAGMENTS,
  GAME_MOVEMENT_CONTRADICTIONS,
  GAME_MOVEMENT_MACHINERY,
  GAME_MOVEMENT_FINAL_DOOR,
  GAME_MOVEMENT_COMPLETE
} GameMovement;

typedef enum {
  GAME_GUEST_MARA = 0,
  GAME_GUEST_OREN,
  GAME_GUEST_SERA,
  GAME_GUEST_BELL
} GameGuestId;

typedef enum {
  GAME_ROLE_GATHERER = 0,
  GAME_ROLE_LISTENER,
  GAME_ROLE_MENDER,
  GAME_ROLE_COOK,
  GAME_ROLE_CARTOGRAPHER,
  GAME_ROLE_WITNESS
} GameRole;

typedef enum {
  GAME_FACILITY_GUEST_ROOM = 0,
  GAME_FACILITY_WORKTABLE,
  GAME_FACILITY_ANCHOR_LINE,
  GAME_FACILITY_PANTRY,
  GAME_FACILITY_MAP_ROOM,
  GAME_FACILITY_QUIET_ROOM,
  GAME_FACILITY_LOOM,
  GAME_FACILITY_ARCHIVE,
  GAME_FACILITY_INFIRMARY,
  GAME_FACILITY_SIGNAL_WALL,
  GAME_FACILITY_ROOT_STAIR,
  GAME_FACILITY_TERMINAL,
  GAME_FACILITY_DOORFRAME
} GameFacility;

typedef struct {
  uint16_t arc_flags;
  uint8_t present;
  uint8_t role;
  uint8_t trust;
  uint8_t ending_opinion;
} GameGuest;

typedef struct {
  GameGuest guest[GAME_GUEST_COUNT];
} GameGuests;

typedef struct {
  uint64_t story_flags;
  uint32_t anchored_choices;
  uint16_t facilities;
  uint8_t movement;
  uint8_t keys;
  uint16_t thread;
  uint8_t ending;
} GameStory;

typedef struct {
  uint16_t equipment_mask;
  uint8_t anchor_spool_tier;
  uint8_t satchel_tier;
  uint8_t lantern_tier;
  uint8_t ward_thread;
  uint8_t brass_keys;
  uint8_t keepsakes;
} GameInventory;

typedef struct {
  int8_t x;
  int8_t y;
  int8_t return_x;
  int8_t return_y;
  uint16_t clarity;
  uint16_t rations;
  uint8_t resolve;
  uint8_t cargo;
  uint8_t active;
  uint8_t encounter;
  uint8_t steps;
  uint8_t destination;
} GameExpedition;

typedef struct {
  HouseState house;
  GameGuests guests;
  WorldState world;
  GameStory story;
  GameInventory inventory;
  GameExpedition expedition;
} GameState;

void game_state_init(GameState *state, int64_t now, uint32_t world_seed);
void game_state_migrate_legacy(GameState *state, const HouseState *legacy,
                               uint32_t world_seed);
bool game_state_is_valid(const GameState *state);
bool game_state_complete_landmark(GameState *state, uint8_t landmark_id);
bool game_state_choose_ending(GameState *state, uint8_t ending);
