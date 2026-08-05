#pragma once

#include <stdint.h>

#include "game_state.h"

typedef enum {
  EXPEDITION_NORTH = 0,
  EXPEDITION_EAST,
  EXPEDITION_SOUTH,
  EXPEDITION_WEST
} ExpeditionDirection;

typedef enum {
  EXPEDITION_RESULT_OK = 0,
  EXPEDITION_RESULT_LOCKED,
  EXPEDITION_RESULT_NO_RESOURCES,
  EXPEDITION_RESULT_BOUNDARY,
  EXPEDITION_RESULT_EVENT,
  EXPEDITION_RESULT_LANDMARK,
  EXPEDITION_RESULT_HIT,
  EXPEDITION_RESULT_FAILED,
  EXPEDITION_RESULT_RETURNED
} ExpeditionResult;

ExpeditionResult expedition_start(GameState *state);
ExpeditionResult expedition_move(GameState *state,
                                 ExpeditionDirection direction,
                                 uint8_t *landmark_id);
ExpeditionResult expedition_return(GameState *state);

