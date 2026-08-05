#include "save_store.h"

#include <stddef.h>
#include <string.h>

#define SAVE_MAGIC 0x484f5553U

typedef struct {
  uint32_t magic;
  uint32_t generation;
  uint16_t schema;
  uint8_t active_bank;
  uint8_t segment_count;
  uint32_t checksum;
} SaveManifest;

typedef struct {
  uint32_t magic;
  uint32_t generation;
  uint16_t schema;
  uint8_t segment;
  uint8_t payload_size;
  uint32_t checksum;
  uint8_t payload[SAVE_STORE_MAX_VALUE_SIZE - 16];
} SaveRecord;

_Static_assert(sizeof(SaveManifest) <= SAVE_STORE_MAX_VALUE_SIZE,
               "manifest must fit one persistence value");
_Static_assert(sizeof(SaveRecord) == SAVE_STORE_MAX_VALUE_SIZE,
               "record size math must match persistence limit");
_Static_assert(sizeof(WorldState) <= sizeof(((SaveRecord *)0)->payload),
               "world segment must fit one persistence value");

static uint32_t prv_checksum(const void *data, size_t length) {
  const uint8_t *bytes = data;
  uint32_t hash = 2166136261U;
  for (size_t i = 0; i < length; ++i) {
    hash ^= bytes[i];
    hash *= 16777619U;
  }
  return hash;
}

static bool prv_backend_is_valid(const SaveBackend *backend) {
  return backend && backend->exists && backend->size && backend->read &&
         backend->write;
}

static void *prv_segment(GameState *state, uint8_t segment, size_t *size) {
  switch (segment) {
    case 0:
      *size = sizeof(state->house);
      return &state->house;
    case 1:
      *size = sizeof(state->guests);
      return &state->guests;
    case 2:
      *size = sizeof(state->world);
      return &state->world;
    case 3:
      *size = sizeof(state->story);
      return &state->story;
    case 4:
      *size = sizeof(state->inventory);
      return &state->inventory;
    case 5:
      *size = sizeof(state->expedition);
      return &state->expedition;
    default:
      *size = 0;
      return NULL;
  }
}

static const void *prv_const_segment(const GameState *state, uint8_t segment,
                                     size_t *size) {
  return prv_segment((GameState *)state, segment, size);
}

static bool prv_manifest_valid(const SaveManifest *manifest) {
  return manifest->magic == SAVE_MAGIC &&
         manifest->schema == GAME_STATE_SCHEMA &&
         manifest->active_bank <= 1 &&
         manifest->segment_count == SAVE_STORE_SEGMENT_COUNT &&
         manifest->checksum ==
             prv_checksum(manifest, offsetof(SaveManifest, checksum));
}

static bool prv_read_manifest(const SaveBackend *backend,
                              SaveManifest *manifest) {
  if (!backend->exists(backend->context, SAVE_STORE_MANIFEST_KEY) ||
      backend->size(backend->context, SAVE_STORE_MANIFEST_KEY) !=
          (int)sizeof(*manifest) ||
      backend->read(backend->context, SAVE_STORE_MANIFEST_KEY, manifest,
                    sizeof(*manifest)) != (int)sizeof(*manifest)) {
    return false;
  }
  return prv_manifest_valid(manifest);
}

static int prv_record_key(uint8_t bank, uint8_t segment) {
  return (bank ? SAVE_STORE_BANK1_KEY : SAVE_STORE_BANK0_KEY) + segment;
}

static bool prv_load_bank(const SaveBackend *backend, uint8_t bank,
                          uint32_t expected_generation, GameState *state,
                          uint32_t *actual_generation) {
  GameState loaded;
  memset(&loaded, 0, sizeof(loaded));
  uint32_t bank_generation = 0;
  for (uint8_t segment = 0; segment < SAVE_STORE_SEGMENT_COUNT; ++segment) {
    const int key = prv_record_key(bank, segment);
    SaveRecord record;
    memset(&record, 0, sizeof(record));
    size_t payload_size = 0;
    void *payload = prv_segment(&loaded, segment, &payload_size);
    const size_t record_size = offsetof(SaveRecord, payload) + payload_size;
    if (!backend->exists(backend->context, key) ||
        backend->size(backend->context, key) != (int)record_size ||
        backend->read(backend->context, key, &record, record_size) !=
            (int)record_size) {
      return false;
    }
    const uint32_t checksum = prv_checksum(
        &record, offsetof(SaveRecord, checksum));
    const uint32_t payload_checksum = prv_checksum(record.payload,
                                                   record.payload_size);
    if (record.magic != SAVE_MAGIC || record.schema != GAME_STATE_SCHEMA ||
        record.segment != segment || record.payload_size != payload_size ||
        record.checksum != (checksum ^ payload_checksum) ||
        (expected_generation && record.generation != expected_generation) ||
        (segment > 0 && record.generation != bank_generation)) {
      return false;
    }
    bank_generation = record.generation;
    memcpy(payload, record.payload, payload_size);
  }
  if (!game_state_is_valid(&loaded)) {
    return false;
  }
  *state = loaded;
  if (actual_generation) {
    *actual_generation = bank_generation;
  }
  return true;
}

SaveStoreResult save_store_load(const SaveBackend *backend, GameState *state,
                                uint32_t *generation) {
  if (!prv_backend_is_valid(backend) || !state) {
    return SAVE_STORE_IO_ERROR;
  }
  SaveManifest manifest;
  if (prv_read_manifest(backend, &manifest)) {
    if (prv_load_bank(backend, manifest.active_bank, manifest.generation,
                      state, generation)) {
      return SAVE_STORE_OK;
    }
    const uint8_t other = (uint8_t)(1U - manifest.active_bank);
    if (prv_load_bank(backend, other, 0, state, generation)) {
      return SAVE_STORE_OK;
    }
    return SAVE_STORE_CORRUPT;
  }

  bool any_record = backend->exists(backend->context, SAVE_STORE_MANIFEST_KEY);
  for (uint8_t bank = 0; bank < 2; ++bank) {
    uint32_t recovered_generation = 0;
    if (prv_load_bank(backend, bank, 0, state, &recovered_generation)) {
      if (generation) {
        *generation = recovered_generation;
      }
      return SAVE_STORE_OK;
    }
    any_record = any_record || backend->exists(
        backend->context, prv_record_key(bank, 0));
  }
  return any_record ? SAVE_STORE_CORRUPT : SAVE_STORE_EMPTY;
}

SaveStoreResult save_store_save(const SaveBackend *backend,
                                const GameState *state,
                                uint32_t *generation) {
  if (!prv_backend_is_valid(backend)) {
    return SAVE_STORE_IO_ERROR;
  }
  if (!game_state_is_valid(state)) {
    return SAVE_STORE_INVALID_STATE;
  }

  SaveManifest prior;
  const bool has_prior = prv_read_manifest(backend, &prior);
  const uint8_t bank = has_prior ? (uint8_t)(1U - prior.active_bank) : 0;
  const uint32_t next_generation = has_prior ? prior.generation + 1U : 1U;

  for (uint8_t segment = 0; segment < SAVE_STORE_SEGMENT_COUNT; ++segment) {
    SaveRecord record;
    memset(&record, 0, sizeof(record));
    record.magic = SAVE_MAGIC;
    record.generation = next_generation;
    record.schema = GAME_STATE_SCHEMA;
    record.segment = segment;
    size_t payload_size = 0;
    const void *payload = prv_const_segment(state, segment, &payload_size);
    record.payload_size = (uint8_t)payload_size;
    memcpy(record.payload, payload, payload_size);
    record.checksum = prv_checksum(&record, offsetof(SaveRecord, checksum)) ^
                      prv_checksum(record.payload, payload_size);
    const int key = prv_record_key(bank, segment);
    const size_t record_size = offsetof(SaveRecord, payload) + payload_size;
    if (backend->write(backend->context, key, &record, record_size) !=
        (int)record_size) {
      return SAVE_STORE_IO_ERROR;
    }
    SaveRecord verification;
    memset(&verification, 0, sizeof(verification));
    if (backend->read(backend->context, key, &verification,
                      record_size) != (int)record_size ||
        memcmp(&verification, &record, record_size) != 0) {
      return SAVE_STORE_IO_ERROR;
    }
  }

  SaveManifest manifest;
  memset(&manifest, 0, sizeof(manifest));
  manifest.magic = SAVE_MAGIC;
  manifest.generation = next_generation;
  manifest.schema = GAME_STATE_SCHEMA;
  manifest.active_bank = bank;
  manifest.segment_count = SAVE_STORE_SEGMENT_COUNT;
  manifest.checksum = prv_checksum(&manifest,
                                   offsetof(SaveManifest, checksum));
  if (backend->write(backend->context, SAVE_STORE_MANIFEST_KEY, &manifest,
                     sizeof(manifest)) != (int)sizeof(manifest)) {
    return SAVE_STORE_IO_ERROR;
  }
  if (generation) {
    *generation = next_generation;
  }
  return SAVE_STORE_OK;
}
