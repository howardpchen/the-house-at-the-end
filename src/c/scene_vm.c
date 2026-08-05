#include "scene_vm.h"

#include <limits.h>
#include <string.h>

#define SCENE_VM_INSTRUCTION_LIMIT 64

static bool prv_read_u8(SceneVm *vm, uint8_t *value) {
  if (vm->pc >= vm->size) {
    return false;
  }
  *value = vm->code[vm->pc++];
  return true;
}

static bool prv_read_u16(SceneVm *vm, uint16_t *value) {
  uint8_t low = 0;
  uint8_t high = 0;
  if (!prv_read_u8(vm, &low) || !prv_read_u8(vm, &high)) {
    return false;
  }
  *value = (uint16_t)(low | ((uint16_t)high << 8));
  return true;
}

static SceneEvent prv_error(SceneVm *vm) {
  memset(&vm->pending, 0, sizeof(vm->pending));
  vm->pending.type = SCENE_EVENT_ERROR;
  return vm->pending;
}

static int16_t prv_add_capped(int16_t value, int16_t amount) {
  const int32_t result = (int32_t)value + amount;
  if (result > 999) {
    return 999;
  }
  if (result < 0) {
    return 0;
  }
  return (int16_t)result;
}

void scene_vm_init(SceneVm *vm, const uint8_t *code, size_t size) {
  if (!vm) {
    return;
  }
  memset(vm, 0, sizeof(*vm));
  vm->code = code;
  vm->size = size;
}

SceneEvent scene_vm_run(SceneVm *vm, SceneContext *context) {
  if (!vm || !context || !vm->code || vm->pending.type == SCENE_EVENT_CHOICE) {
    return vm ? prv_error(vm) : (SceneEvent){.type = SCENE_EVENT_ERROR};
  }
  memset(&vm->pending, 0, sizeof(vm->pending));
  for (uint8_t instruction = 0;
       instruction < SCENE_VM_INSTRUCTION_LIMIT; ++instruction) {
    uint8_t opcode = 0;
    if (!prv_read_u8(vm, &opcode)) {
      return prv_error(vm);
    }
    if (opcode == SCENE_OP_TEXT) {
      if (!prv_read_u16(vm, &vm->pending.string_id)) {
        return prv_error(vm);
      }
      vm->pending.type = SCENE_EVENT_TEXT;
      return vm->pending;
    }
    if (opcode == SCENE_OP_CHOICE) {
      if (!prv_read_u8(vm, &vm->pending.choice_count) ||
          vm->pending.choice_count == 0 ||
          vm->pending.choice_count > SCENE_VM_MAX_CHOICES) {
        return prv_error(vm);
      }
      for (uint8_t i = 0; i < vm->pending.choice_count; ++i) {
        if (!prv_read_u16(vm, &vm->pending.choice_string_ids[i]) ||
            !prv_read_u16(vm, &vm->pending.choice_targets[i]) ||
            vm->pending.choice_targets[i] >= vm->size) {
          return prv_error(vm);
        }
      }
      vm->pending.type = SCENE_EVENT_CHOICE;
      return vm->pending;
    }
    if (opcode == SCENE_OP_IF_FLAG) {
      uint8_t flag = 0;
      uint16_t target = 0;
      if (!prv_read_u8(vm, &flag) || flag >= 64 ||
          !prv_read_u16(vm, &target) || target >= vm->size) {
        return prv_error(vm);
      }
      if (context->flags & (UINT64_C(1) << flag)) {
        vm->pc = target;
      }
      continue;
    }
    if (opcode == SCENE_OP_IF_RESOURCE) {
      uint8_t resource = 0;
      uint16_t amount = 0;
      uint16_t target = 0;
      if (!prv_read_u8(vm, &resource) ||
          resource >= SCENE_VM_RESOURCE_COUNT ||
          !prv_read_u16(vm, &amount) || !prv_read_u16(vm, &target) ||
          target >= vm->size) {
        return prv_error(vm);
      }
      if (context->resources[resource] >= (int16_t)amount) {
        vm->pc = target;
      }
      continue;
    }
    if (opcode == SCENE_OP_COST || opcode == SCENE_OP_REWARD) {
      uint8_t resource = 0;
      uint16_t amount = 0;
      if (!prv_read_u8(vm, &resource) ||
          resource >= SCENE_VM_RESOURCE_COUNT ||
          !prv_read_u16(vm, &amount)) {
        return prv_error(vm);
      }
      const int16_t delta = opcode == SCENE_OP_COST
          ? (int16_t)-(int16_t)amount : (int16_t)amount;
      if (opcode == SCENE_OP_COST &&
          context->resources[resource] < (int16_t)amount) {
        return prv_error(vm);
      }
      context->resources[resource] = prv_add_capped(
          context->resources[resource], delta);
      continue;
    }
    if (opcode == SCENE_OP_SET_FLAG || opcode == SCENE_OP_CLEAR_FLAG) {
      uint8_t flag = 0;
      if (!prv_read_u8(vm, &flag) || flag >= 64) {
        return prv_error(vm);
      }
      if (opcode == SCENE_OP_SET_FLAG) {
        context->flags |= UINT64_C(1) << flag;
      } else {
        context->flags &= ~(UINT64_C(1) << flag);
      }
      continue;
    }
    if (opcode == SCENE_OP_TRUST) {
      uint8_t guest = 0;
      uint8_t encoded_delta = 0;
      if (!prv_read_u8(vm, &guest) || guest >= SCENE_VM_GUEST_COUNT ||
          !prv_read_u8(vm, &encoded_delta)) {
        return prv_error(vm);
      }
      const int8_t delta = (int8_t)encoded_delta;
      int16_t trust = context->trust[guest] + delta;
      if (trust < 0) {
        trust = 0;
      } else if (trust > 3) {
        trust = 3;
      }
      context->trust[guest] = (uint8_t)trust;
      continue;
    }
    if (opcode == SCENE_OP_GOTO) {
      uint16_t target = 0;
      if (!prv_read_u16(vm, &target) || target >= vm->size) {
        return prv_error(vm);
      }
      vm->pc = target;
      continue;
    }
    if (opcode == SCENE_OP_END) {
      if (!prv_read_u8(vm, &vm->pending.result)) {
        return prv_error(vm);
      }
      vm->pending.type = SCENE_EVENT_END;
      return vm->pending;
    }
    return prv_error(vm);
  }
  return prv_error(vm);
}

bool scene_vm_choose(SceneVm *vm, uint8_t choice_index) {
  if (!vm || vm->pending.type != SCENE_EVENT_CHOICE ||
      choice_index >= vm->pending.choice_count) {
    return false;
  }
  vm->pc = vm->pending.choice_targets[choice_index];
  memset(&vm->pending, 0, sizeof(vm->pending));
  return true;
}

