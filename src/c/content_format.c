#include "content_format.h"

#include <string.h>

static uint16_t prv_u16(const uint8_t *data) {
  return (uint16_t)(data[0] | ((uint16_t)data[1] << 8));
}

bool content_find_scene(const uint8_t *data, size_t size, uint8_t scene_id,
                        const uint8_t **code, size_t *code_size) {
  if (!data || size < 5 || memcmp(data, "HSC1", 4) != 0) {
    return false;
  }
  const uint8_t count = data[4];
  if (size < 5U + (size_t)count * 5U) {
    return false;
  }
  for (uint8_t index = 0; index < count; ++index) {
    const uint8_t *entry = data + 5U + (size_t)index * 5U;
    const uint16_t offset = prv_u16(entry + 1);
    const uint16_t length = prv_u16(entry + 3);
    if ((size_t)offset + length > size) {
      return false;
    }
    if (entry[0] == scene_id) {
      if (code) {
        *code = data + offset;
      }
      if (code_size) {
        *code_size = length;
      }
      return true;
    }
  }
  return false;
}

bool content_read_string(const uint8_t *data, size_t size, uint16_t string_id,
                         char *buffer, size_t buffer_size) {
  if (!data || !buffer || buffer_size == 0 || size < 6 ||
      memcmp(data, "HST1", 4) != 0) {
    return false;
  }
  const uint16_t count = prv_u16(data + 4);
  if (string_id >= count || size < 6U + (size_t)count * 3U) {
    return false;
  }
  const uint8_t *entry = data + 6U + (size_t)string_id * 3U;
  const uint16_t offset = prv_u16(entry);
  const uint8_t length = entry[2];
  if ((size_t)offset + length > size || (size_t)length + 1U > buffer_size) {
    return false;
  }
  memcpy(buffer, data + offset, length);
  buffer[length] = '\0';
  return true;
}

