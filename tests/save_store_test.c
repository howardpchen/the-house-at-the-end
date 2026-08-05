#include "save_store.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TEST_KEY_COUNT 64

typedef struct {
  uint8_t values[TEST_KEY_COUNT][SAVE_STORE_MAX_VALUE_SIZE];
  int sizes[TEST_KEY_COUNT];
  int writes_before_failure;
} MemoryStore;

static bool memory_exists(void *context, int key) {
  MemoryStore *store = context;
  return key >= 0 && key < TEST_KEY_COUNT && store->sizes[key] > 0;
}

static int memory_size(void *context, int key) {
  MemoryStore *store = context;
  return key >= 0 && key < TEST_KEY_COUNT ? store->sizes[key] : -1;
}

static int memory_read(void *context, int key, void *data, size_t size) {
  MemoryStore *store = context;
  if (key < 0 || key >= TEST_KEY_COUNT || store->sizes[key] != (int)size) {
    return -1;
  }
  memcpy(data, store->values[key], size);
  return (int)size;
}

static int memory_write(void *context, int key, const void *data,
                        size_t size) {
  MemoryStore *store = context;
  if (key < 0 || key >= TEST_KEY_COUNT ||
      size > SAVE_STORE_MAX_VALUE_SIZE || store->writes_before_failure == 0) {
    return -1;
  }
  if (store->writes_before_failure > 0) {
    store->writes_before_failure--;
  }
  memcpy(store->values[key], data, size);
  store->sizes[key] = (int)size;
  return (int)size;
}

static SaveBackend backend_for(MemoryStore *store) {
  return (SaveBackend){
      .context = store,
      .exists = memory_exists,
      .size = memory_size,
      .read = memory_read,
      .write = memory_write};
}

static void test_round_trip_and_generations(void) {
  MemoryStore store;
  memset(&store, 0, sizeof(store));
  store.writes_before_failure = -1;
  const SaveBackend backend = backend_for(&store);
  GameState state;
  game_state_init(&state, 1234, 0x12345678U);
  state.house.kindling = 17;
  state.story.thread = 3;
  state.guests.guest[GAME_GUEST_MARA].present = 1;
  state.guests.guest[GAME_GUEST_MARA].role = GAME_ROLE_GATHERER;
  world_reveal(&state.world, 3, 2);

  uint32_t generation = 0;
  assert(save_store_save(&backend, &state, &generation) == SAVE_STORE_OK);
  assert(generation == 1);

  GameState loaded;
  memset(&loaded, 0, sizeof(loaded));
  assert(save_store_load(&backend, &loaded, &generation) == SAVE_STORE_OK);
  assert(generation == 1);
  assert(loaded.house.kindling == 17);
  assert(loaded.story.thread == 3);
  assert(world_is_visible(&loaded.world, 3, 2));

  state.house.kindling = 29;
  assert(save_store_save(&backend, &state, &generation) == SAVE_STORE_OK);
  assert(generation == 2);
  assert(save_store_load(&backend, &loaded, &generation) == SAVE_STORE_OK);
  assert(loaded.house.kindling == 29);
}

static void test_interrupted_write_keeps_active_bank(void) {
  MemoryStore store;
  memset(&store, 0, sizeof(store));
  store.writes_before_failure = -1;
  const SaveBackend backend = backend_for(&store);
  GameState state;
  game_state_init(&state, 2000, 99U);
  state.house.kindling = 10;
  assert(save_store_save(&backend, &state, NULL) == SAVE_STORE_OK);
  state.house.kindling = 20;
  assert(save_store_save(&backend, &state, NULL) == SAVE_STORE_OK);

  state.house.kindling = 30;
  store.writes_before_failure = 2;
  assert(save_store_save(&backend, &state, NULL) == SAVE_STORE_IO_ERROR);

  GameState loaded;
  uint32_t generation = 0;
  assert(save_store_load(&backend, &loaded, &generation) == SAVE_STORE_OK);
  assert(generation == 2);
  assert(loaded.house.kindling == 20);
}

static void test_corrupt_active_bank_recovers_prior(void) {
  MemoryStore store;
  memset(&store, 0, sizeof(store));
  store.writes_before_failure = -1;
  const SaveBackend backend = backend_for(&store);
  GameState state;
  game_state_init(&state, 3000, 101U);
  state.house.kindling = 11;
  assert(save_store_save(&backend, &state, NULL) == SAVE_STORE_OK);
  state.house.kindling = 22;
  assert(save_store_save(&backend, &state, NULL) == SAVE_STORE_OK);

  store.values[SAVE_STORE_BANK1_KEY][20] ^= 0xffU;
  GameState loaded;
  uint32_t generation = 0;
  assert(save_store_load(&backend, &loaded, &generation) == SAVE_STORE_OK);
  assert(generation == 1);
  assert(loaded.house.kindling == 11);
}

static void test_legacy_migration(void) {
  HouseState legacy;
  house_state_init(&legacy, 4000);
  legacy.residents = 2;
  legacy.gatherers = 1;
  legacy.listeners = 1;
  legacy.story_flags = HOUSE_STORY_FIRST_GUEST | HOUSE_STORY_FIRST_MEMORY;

  GameState migrated;
  game_state_migrate_legacy(&migrated, &legacy, 55U);
  assert(migrated.story.movement == GAME_MOVEMENT_FRAGMENTS);
  assert(migrated.guests.guest[GAME_GUEST_MARA].present);
  assert(!migrated.guests.guest[GAME_GUEST_OREN].present);
  assert(game_state_is_valid(&migrated));
}

int main(void) {
  test_round_trip_and_generations();
  test_interrupted_write_keeps_active_bank();
  test_corrupt_active_bank_recovers_prior();
  test_legacy_migration();
  puts("save_store tests passed");
  return 0;
}
