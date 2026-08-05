#include "content_format.h"
#include "scene_vm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t *read_file(const char *path, size_t *size) {
  FILE *file = fopen(path, "rb");
  assert(file);
  assert(fseek(file, 0, SEEK_END) == 0);
  const long length = ftell(file);
  assert(length > 0);
  assert(fseek(file, 0, SEEK_SET) == 0);
  uint8_t *data = malloc((size_t)length);
  assert(data);
  assert(fread(data, 1, (size_t)length, file) == (size_t)length);
  assert(fclose(file) == 0);
  *size = (size_t)length;
  return data;
}

static void test_platform_scene(const uint8_t *scenes, size_t scene_size,
                                const uint8_t *strings, size_t string_size) {
  const uint8_t *code = NULL;
  size_t code_size = 0;
  assert(content_find_scene(scenes, scene_size, 1, &code, &code_size));
  SceneVm vm;
  SceneContext context = {0};
  scene_vm_init(&vm, code, code_size);

  SceneEvent event = scene_vm_run(&vm, &context);
  assert(event.type == SCENE_EVENT_TEXT);
  char page[81];
  assert(content_read_string(strings, string_size, event.string_id,
                             page, sizeof(page)));
  assert(page[0] == 'E');
  event = scene_vm_run(&vm, &context);
  assert(event.type == SCENE_EVENT_TEXT);
  event = scene_vm_run(&vm, &context);
  assert(event.type == SCENE_EVENT_CHOICE);
  assert(event.choice_count == 2);
  assert(scene_vm_choose(&vm, 0));
  event = scene_vm_run(&vm, &context);
  assert(event.type == SCENE_EVENT_TEXT);
  assert(context.resources[5] == 1);
  assert(context.flags & (UINT64_C(1) << 2));
  assert(context.trust[1] == 1);
  event = scene_vm_run(&vm, &context);
  assert(event.type == SCENE_EVENT_END && event.result == 1);
}

static void test_resource_branch(const uint8_t *scenes, size_t scene_size) {
  const uint8_t *code = NULL;
  size_t code_size = 0;
  assert(content_find_scene(scenes, scene_size, 2, &code, &code_size));
  SceneVm vm;
  SceneContext context = {0};
  scene_vm_init(&vm, code, code_size);
  assert(scene_vm_run(&vm, &context).type == SCENE_EVENT_TEXT);
  assert(scene_vm_run(&vm, &context).type == SCENE_EVENT_TEXT);
  SceneEvent event = scene_vm_run(&vm, &context);
  assert(event.type == SCENE_EVENT_END && event.result == 0);

  context.resources[3] = 2;
  scene_vm_init(&vm, code, code_size);
  assert(scene_vm_run(&vm, &context).type == SCENE_EVENT_TEXT);
  event = scene_vm_run(&vm, &context);
  assert(event.type == SCENE_EVENT_CHOICE);
  assert(scene_vm_choose(&vm, 0));
  assert(scene_vm_run(&vm, &context).type == SCENE_EVENT_TEXT);
  assert(context.resources[3] == 0);
  assert(context.flags & (UINT64_C(1) << 4));
  assert(scene_vm_run(&vm, &context).type == SCENE_EVENT_END);
}

static void test_invalid_code(void) {
  const uint8_t invalid[] = {SCENE_OP_GOTO, 0xff, 0xff};
  SceneVm vm;
  SceneContext context = {0};
  scene_vm_init(&vm, invalid, sizeof(invalid));
  assert(scene_vm_run(&vm, &context).type == SCENE_EVENT_ERROR);
}

int main(void) {
  size_t scene_size = 0;
  size_t string_size = 0;
  uint8_t *scenes = read_file("resources/generated/scenes.bin", &scene_size);
  uint8_t *strings = read_file("resources/generated/strings.bin", &string_size);
  test_platform_scene(scenes, scene_size, strings, string_size);
  test_resource_branch(scenes, scene_size);
  test_invalid_code();
  free(strings);
  free(scenes);
  puts("scene_vm tests passed");
  return 0;
}
