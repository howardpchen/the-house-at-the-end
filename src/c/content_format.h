#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool content_find_scene(const uint8_t *data, size_t size, uint8_t scene_id,
                        const uint8_t **code, size_t *code_size);
bool content_read_string(const uint8_t *data, size_t size, uint16_t string_id,
                         char *buffer, size_t buffer_size);

