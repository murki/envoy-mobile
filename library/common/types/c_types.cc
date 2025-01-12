// NOLINT(namespace-envoy)
#include "library/common/types/c_types.h"

#include <string>

#include "common/common/assert.h"

const int kEnvoySuccess = ENVOY_SUCCESS;
const int kEnvoyFailure = ENVOY_FAILURE;

void* safe_malloc(size_t size) {
  void* ptr = malloc(size);
  if (size > 0) {
    RELEASE_ASSERT(ptr != nullptr, "malloc failure");
  }
  return ptr;
}

void* safe_calloc(size_t count, size_t size) {
  void* ptr = calloc(count, size);
  if (count > 0 && size > 0) {
    RELEASE_ASSERT(ptr != nullptr, "calloc failure");
  }
  return ptr;
}

void envoy_noop_release(void* context) { (void)context; }

void release_envoy_data_map(envoy_map map) {
  for (envoy_map_size_t i = 0; i < map.length; i++) {
    envoy_map_entry entry = map.entries[i];
    entry.key.release(entry.key.context);
    entry.value.release(entry.value.context);
  }
  free(map.entries);
}

void release_envoy_headers(envoy_headers headers) { release_envoy_data_map(headers); }

envoy_map copy_envoy_data_map(envoy_map src) {
  envoy_map_entry* dst_entries =
      static_cast<envoy_map_entry*>(safe_malloc(sizeof(envoy_map_entry) * src.length));
  for (envoy_map_size_t i = 0; i < src.length; i++) {
    envoy_map_entry new_entry = {
        copy_envoy_data(src.entries[i].key.length, src.entries[i].key.bytes),
        copy_envoy_data(src.entries[i].value.length, src.entries[i].value.bytes)};
    dst_entries[i] = new_entry;
  }
  envoy_map dst = {src.length, dst_entries};
  return dst;
}

envoy_headers copy_envoy_headers(envoy_headers src) { return copy_envoy_data_map(src); }

envoy_data copy_envoy_data(size_t length, const uint8_t* src_bytes) {
  uint8_t* dst_bytes = static_cast<uint8_t*>(safe_malloc(sizeof(uint8_t) * length));
  memcpy(dst_bytes, src_bytes, length);
  // Note: since this function is copying the bytes over to freshly allocated memory, free is an
  // appropriate release function and dst_bytes is an appropriate context.
  envoy_data dst = {length, dst_bytes, free, dst_bytes};
  return dst;
}

const envoy_data envoy_nodata = {0, NULL, envoy_noop_release, NULL};

const envoy_headers envoy_noheaders = {0, NULL};
