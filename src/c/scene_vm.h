#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SCENE_VM_MAX_CHOICES 3
#define SCENE_VM_RESOURCE_COUNT 7
#define SCENE_VM_GUEST_COUNT 4

typedef enum {
  SCENE_OP_TEXT = 1,
  SCENE_OP_CHOICE,
  SCENE_OP_IF_FLAG,
  SCENE_OP_IF_RESOURCE,
  SCENE_OP_COST,
  SCENE_OP_REWARD,
  SCENE_OP_SET_FLAG,
  SCENE_OP_CLEAR_FLAG,
  SCENE_OP_TRUST,
  SCENE_OP_GOTO,
  SCENE_OP_END
} SceneOpcode;

typedef enum {
  SCENE_EVENT_NONE = 0,
  SCENE_EVENT_TEXT,
  SCENE_EVENT_CHOICE,
  SCENE_EVENT_END,
  SCENE_EVENT_ERROR
} SceneEventType;

typedef struct {
  int16_t resources[SCENE_VM_RESOURCE_COUNT];
  uint64_t flags;
  uint8_t trust[SCENE_VM_GUEST_COUNT];
} SceneContext;

typedef struct {
  SceneEventType type;
  uint16_t string_id;
  uint8_t choice_count;
  uint16_t choice_string_ids[SCENE_VM_MAX_CHOICES];
  uint16_t choice_targets[SCENE_VM_MAX_CHOICES];
  uint8_t result;
} SceneEvent;

typedef struct {
  const uint8_t *code;
  size_t size;
  uint16_t pc;
  SceneEvent pending;
} SceneVm;

void scene_vm_init(SceneVm *vm, const uint8_t *code, size_t size);
SceneEvent scene_vm_run(SceneVm *vm, SceneContext *context);
bool scene_vm_choose(SceneVm *vm, uint8_t choice_index);

