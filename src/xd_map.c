/*
 * ==============================================================================
 * File: xd_map.c
 * Author: Duraid Maihoub
 * Date: 14 Aug 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#include "xd_map.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xd_list.h"

// ========================
// Macros
// ========================

/**
 * @brief Minimum length of buckets array.
 */
#define XD_MAP_MIN_BUCKET_COUNT (17)

/**
 * @brief Maximum load factor before the hash map resizes to more buckets.
 */
#define XD_MAP_MAX_LOAD_FACTOR (0.75)

/**
 * @brief Minimum load factor before the hash map resizes to fewer buckets.
 */
#define XD_MAP_MIN_LOAD_FACTOR (0.25)

// ========================
// Function Declarations
// ========================

static xd_bucket_entry_t *xd_bucket_entry_create(
    void *key, void *value, xd_gens_copy_func_t copy_key_func,
    xd_gens_copy_func_t copy_value_func);
static void xd_bucket_entry_destroy(xd_bucket_entry_t *entry,
                                    xd_gens_destroy_func_t destroy_key_func,
                                    xd_gens_destroy_func_t destroy_value_func);

static void *xd_bucket_list_copy_func(void *data);
static void xd_bucket_list_destroy_func(void *data);
static int xd_bucket_list_comp_func(const void *data1, const void *data2);

static int xd_util_is_prime(int num);
static int xd_util_next_prime(int num);
static int xd_util_prev_prime(int num);

static void xd_map_rehash(xd_map_t *map);

// ========================
// Function Definitions
// ========================

/**
 * @brief Creates and initializes a new `xd_bucket_entry_t` structure.
 *
 * @param key A pointer to the key.
 * @param value A pointer to the value.
 * @param copy_key_func A pointer to the function used to copy the key.
 * @param copy_value_func A pointer to the function used to copy the value.
 *
 * @return A pointer to the newly created `xd_bucket_entry_t`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by
 * calling `xd_bucket_entry_destroy()` and passing it the returned pointer.
 */
static xd_bucket_entry_t *xd_bucket_entry_create(
    void *key, void *value, xd_gens_copy_func_t copy_key_func,
    xd_gens_copy_func_t copy_value_func) {
  xd_bucket_entry_t *entry =
      (xd_bucket_entry_t *)malloc(sizeof(xd_bucket_entry_t));
  if (entry == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  entry->key = copy_key_func(key);
  entry->value = copy_value_func(value);
  return entry;
}  // xd_bucket_entry_create()

/**
 * @brief Frees the memory allocated for the passed `xd_bucket_entry_t`
 * structure.
 *
 * @param entry A pointer to the `xd_bucket_entry_t` to be freed.
 * @param destroy_key_func A pointer to the function used to free the key
 * stored in the entry.
 * @param destroy_value_func A pointer to the function used to free the value
 * stored in the entry.
 *
 * @note If the passed entry pointer is `NULL` no action shall occur.
 */
static void xd_bucket_entry_destroy(xd_bucket_entry_t *entry,
                                    xd_gens_destroy_func_t destroy_key_func,
                                    xd_gens_destroy_func_t destroy_value_func) {
  if (entry == NULL) {
    return;
  }
  destroy_key_func(entry->key);
  destroy_value_func(entry->value);
  free(entry);
}  // xd_bucket_entry_destroy()

/**
 * @brief Passed to `xd_list_create()` as copy function.
 */
static void *xd_bucket_list_copy_func(void *data) {
  return data;
}  // xd_bucket_list_copy_func()

/**
 * @brief Passed to `xd_list_create()` as destroy function.
 */
static void xd_bucket_list_destroy_func(void *data) {
  (void)data;
}  // xd_bucket_list_destroy_func()

/**
 * @brief Passed to `xd_list_create()` as comp function.
 */
static int xd_bucket_list_comp_func(const void *data1, const void *data2) {
  (void)data1;
  (void)data2;
  return 0;
}  // xd_bucket_list_comp_func()

/**
 * @brief Checks whether the passed integer is a prime number.
 *
 * @param num The integer to check if prime.
 *
 * @return `1` if the passed integer is prime, `0` otherwise.
 */
static int xd_util_is_prime(int num) {
  if (num <= 1) {
    return 0;
  }
  if (num == 2) {
    return 1;
  }
  if (num % 2 == 0) {
    return 0;
  }
  for (int i = 3; i * i <= num; i += 2) {
    if (num % i == 0) {
      return 0;
    }
  }
  return 1;
}  // xd_util_is_prime()

/**
 * @brief Returns the smallest prime greater than the given number.
 *
 * @param num The number for which to find the next prime.
 *
 * @return The smallest prime greater than `num`, or `-1` if none exists.
 *
 * @note The result is constrained to the valid bucket count range
 * `[XD_MAP_MIN_BUCKET_COUNT, INT_MAX]`.
 */
static int xd_util_next_prime(int num) {
  if (num == INT_MAX) {
    return -1;
  }
  num = (num < XD_MAP_MIN_BUCKET_COUNT) ? XD_MAP_MIN_BUCKET_COUNT : num + 1;
  for (int i = num; i < INT_MAX; i++) {
    if (xd_util_is_prime(i)) {
      return i;
    }
  }
  return -1;
}  // xd_util_next_prime()

/**
 * @brief Returns the largest prime less than the given number.
 *
 * @param num The number for which to find the previous prime.
 *
 * @return The largest prime less than `num`, or `-1` if none exists.
 *
 * @note The result is constrained to the valid bucket count range
 * `[XD_MAP_MIN_BUCKET_COUNT, INT_MAX]`.
 */
static int xd_util_prev_prime(int num) {
  if (num <= XD_MAP_MIN_BUCKET_COUNT) {
    return -1;
  }
  for (int i = num - 1; i >= XD_MAP_MIN_BUCKET_COUNT; i--) {
    if (xd_util_is_prime(i)) {
      return i;
    }
  }
  return -1;
}  // xd_util_prev_prime()

/**
 * @brief Rehashes the map if load factor is outside allowed range.
 *
 * If load factor >= `XD_MAP_MAX_LOAD_FACTOR`, grows the map.
 * If load factor <= `XD_MAP_MIN_LOAD_FACTOR`, shrinks the map.
 * Uses prime bucket counts and rehashes all entries.
 *
 * @param map The map to be rehashed.
 */
static void xd_map_rehash(xd_map_t *map) {
  double load_factor = (double)map->entry_count / map->bucket_count;

  int new_bucket_count = -1;
  if (load_factor >= XD_MAP_MAX_LOAD_FACTOR) {
    new_bucket_count = xd_util_next_prime(map->bucket_count * 2);
  }
  else if (load_factor <= XD_MAP_MIN_LOAD_FACTOR) {
    new_bucket_count = xd_util_prev_prime(map->bucket_count / 2);
  }

  if (new_bucket_count == -1) {
    return;
  }

  // create the new array of buckets
  xd_list_t **new_buckets =
      (xd_list_t **)malloc(sizeof(xd_list_t *) * new_bucket_count);
  if (new_buckets == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < new_bucket_count; i++) {
    new_buckets[i] =
        xd_list_create(xd_bucket_list_copy_func, xd_bucket_list_destroy_func,
                       xd_bucket_list_comp_func);
  }

  // re-hash and add the entries from the old buckets to the new buckets
  for (int i = 0; i < map->bucket_count; i++) {
    xd_list_t *bucket = map->buckets[i];
    for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
      xd_bucket_entry_t *entry = node->data;
      unsigned int hash = map->hash_func(entry->key) % new_bucket_count;
      xd_list_add_last(new_buckets[hash], entry);
    }
    xd_list_destroy(bucket);
  }
  free((void *)map->buckets);

  // update the map buckets to point to the new one
  map->buckets = new_buckets;
  map->bucket_count = new_bucket_count;
}  // xd_map_rehash()

// ========================
// Public Functions
// ========================

xd_map_t *xd_map_create(xd_gens_copy_func_t copy_key_func,
                        xd_gens_destroy_func_t destroy_key_func,
                        xd_gens_comp_func_t comp_key_func,
                        xd_gens_copy_func_t copy_value_func,
                        xd_gens_destroy_func_t destroy_value_func,
                        xd_gens_comp_func_t comp_value_func,
                        xd_gens_hash_func_t hash_func) {
  if (copy_key_func == NULL || copy_value_func == NULL ||
      destroy_key_func == NULL || destroy_value_func == NULL ||
      comp_key_func == NULL || comp_value_func == NULL || hash_func == NULL) {
    return NULL;
  }
  xd_map_t *map = (xd_map_t *)malloc(sizeof(xd_map_t));
  if (map == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  map->bucket_count = XD_MAP_MIN_BUCKET_COUNT;
  map->entry_count = 0;
  map->buckets = (xd_list_t **)malloc(sizeof(xd_list_t *) * map->bucket_count);
  if (map->buckets == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < map->bucket_count; i++) {
    map->buckets[i] =
        xd_list_create(xd_bucket_list_copy_func, xd_bucket_list_destroy_func,
                       xd_bucket_list_comp_func);
  }
  map->copy_key_func = copy_key_func;
  map->copy_value_func = copy_value_func;
  map->destroy_key_func = destroy_key_func;
  map->destroy_value_func = destroy_value_func;
  map->comp_key_func = comp_key_func;
  map->comp_value_func = comp_value_func;
  map->hash_func = hash_func;
  return map;
}  // xd_map_create()

void xd_map_destroy(xd_map_t *map) {
  if (map == NULL) {
    return;
  }
  for (int i = 0; i < map->bucket_count; i++) {
    xd_list_t *bucket = map->buckets[i];
    for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
      xd_bucket_entry_destroy(node->data, map->destroy_key_func,
                              map->destroy_value_func);
    }
    xd_list_destroy(bucket);
  }
  free((void *)map->buckets);
  free(map);
}  // xd_map_destroy()

void xd_map_clear(xd_map_t *map) {
  if (map == NULL) {
    return;
  }

  // create the new array of buckets
  int new_bucket_count = XD_MAP_MIN_BUCKET_COUNT;
  xd_list_t **new_buckets =
      (xd_list_t **)malloc(sizeof(xd_list_t *) * new_bucket_count);
  if (new_buckets == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < new_bucket_count; i++) {
    new_buckets[i] =
        xd_list_create(xd_bucket_list_copy_func, xd_bucket_list_destroy_func,
                       xd_bucket_list_comp_func);
  }

  // remove old buckets and their entries
  for (int i = 0; i < map->bucket_count; i++) {
    xd_list_t *bucket = map->buckets[i];
    for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
      xd_bucket_entry_destroy(node->data, map->destroy_key_func,
                              map->destroy_value_func);
    }
    xd_list_destroy(bucket);
  }
  free((void *)map->buckets);

  // update the map buckets to point to the new one
  map->buckets = new_buckets;
  map->bucket_count = new_bucket_count;
  map->entry_count = 0;
}  // xd_map_clear()

void xd_map_put(xd_map_t *map, void *key, void *value) {
  if (map == NULL) {
    return;
  }
  unsigned int hash = map->hash_func(key) % map->bucket_count;
  xd_list_t *bucket = map->buckets[hash];
  xd_bucket_entry_t *new_entry = xd_bucket_entry_create(
      key, value, map->copy_key_func, map->copy_value_func);
  xd_bucket_entry_t *old_entry = NULL;

  // update old entry
  for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
    old_entry = node->data;
    if (map->comp_key_func(old_entry->key, new_entry->key) == 0) {
      node->data = new_entry;
      xd_bucket_entry_destroy(old_entry, map->destroy_key_func,
                              map->destroy_value_func);
      return;
    }
  }

  // new entry
  xd_list_add_last(bucket, new_entry);
  map->entry_count++;
  xd_map_rehash(map);
}  // xd_map_put()

int xd_map_remove(xd_map_t *map, void *key) {
  if (map == NULL) {
    return -1;
  }
  unsigned int hash = map->hash_func(key) % map->bucket_count;
  xd_list_t *bucket = map->buckets[hash];
  for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
    xd_bucket_entry_t *entry = node->data;
    if (map->comp_key_func(entry->key, key) == 0) {
      xd_bucket_entry_destroy(entry, map->destroy_key_func,
                              map->destroy_value_func);
      xd_list_remove_node(bucket, node);
      map->entry_count--;
      xd_map_rehash(map);
      return 0;
    }
  }
  return -1;
}  // xd_map_remove()

void *xd_map_get(xd_map_t *map, void *key) {
  if (map == NULL) {
    return NULL;
  }
  unsigned int hash = map->hash_func(key) % map->bucket_count;
  xd_list_t *bucket = map->buckets[hash];
  for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
    xd_bucket_entry_t *entry = node->data;
    if (map->comp_key_func(entry->key, key) == 0) {
      return entry->value;
    }
  }
  return NULL;
}  // xd_map_get()

int xd_map_contains_key(xd_map_t *map, void *key) {
  if (map == NULL) {
    return 0;
  }
  unsigned int hash = map->hash_func(key) % map->bucket_count;
  xd_list_t *bucket = map->buckets[hash];

  for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
    xd_bucket_entry_t *entry = node->data;
    if (map->comp_key_func(entry->key, key) == 0) {
      return 1;
    }
  }
  return 0;
}  // xd_map_contains_key()

int xd_map_contains_value(xd_map_t *map, void *key) {
  if (map == NULL) {
    return 0;
  }
  for (int i = 0; i < map->bucket_count; i++) {
    xd_list_t *bucket = map->buckets[i];
    for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
      xd_bucket_entry_t *entry = node->data;
      if (map->comp_value_func(entry->key, key) == 0) {
        return 1;
      }
    }
  }
  return 0;
}  // xd_map_contains_value()

void **xd_map_to_array(xd_map_t *map) {
  if (map == NULL) {
    return NULL;
  }
  void **arr = (void **)malloc(sizeof(void *) * (map->entry_count + 1));
  if (arr == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  int idx = 0;
  for (int i = 0; i < map->bucket_count; i++) {
    xd_list_t *bucket = map->buckets[i];
    for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
      xd_bucket_entry_t *entry = node->data;
      arr[idx++] = entry->value;
    }
  }
  arr[idx] = NULL;

  return arr;
}  // xd_map_to_array()
