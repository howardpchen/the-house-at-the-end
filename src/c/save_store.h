#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "game_state.h"

#define SAVE_STORE_MANIFEST_KEY 20
#define SAVE_STORE_BANK0_KEY 30
#define SAVE_STORE_BANK1_KEY 40
#define SAVE_STORE_SEGMENT_COUNT 6
#define SAVE_STORE_MAX_VALUE_SIZE 256

typedef struct {
  void *context;
  bool (*exists)(void *context, int key);
  int (*size)(void *context, int key);
  int (*read)(void *context, int key, void *data, size_t size);
  int (*write)(void *context, int key, const void *data, size_t size);
} SaveBackend;

typedef enum {
  SAVE_STORE_OK = 0,
  SAVE_STORE_EMPTY,
  SAVE_STORE_CORRUPT,
  SAVE_STORE_IO_ERROR,
  SAVE_STORE_INVALID_STATE
} SaveStoreResult;

SaveStoreResult save_store_load(const SaveBackend *backend, GameState *state,
                                uint32_t *generation);
SaveStoreResult save_store_save(const SaveBackend *backend,
                                const GameState *state,
                                uint32_t *generation);

