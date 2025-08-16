/*
 * ==============================================================================
 * File: test_xd_map.c
 * Author: Duraid Maihoub
 * Date: 16 Aug 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "xd_ctest.h"
#include "xd_generic_funcs.h"
#include "xd_map.h"

// ========================
// Util Functions
// ========================

static void *xd_string_copy(void *data) {
  if (data == NULL) {
    return NULL;
  }
  return strdup(data);
}  // xd_string_copy()

static void xd_string_destroy(void *data) {
  free(data);
}  // xd_string_free()

static int xd_string_comp(const void *data1, const void *data2) {
  if (data1 == NULL && data2 == NULL) {
    return 0;
  }
  if (data1 == NULL) {
    return -1;
  }
  if (data2 == NULL) {
    return 1;
  }
  return strcmp(data1, data2);
}  // xd_string_comp()

static unsigned int xd_string_hash(void *data) {
  if (data == NULL) {
    return 0;
  }
  char *str = data;
  return (int)str[0];
}  // xd_string_hash()

// ========================
// Test Functions
// ========================

static int test_xd_map_create() {
  XD_TEST_START;

  // Arrange - Act
  xd_map_t *map = xd_map_create(
      xd_string_copy, xd_string_destroy, xd_string_comp, xd_string_copy,
      xd_string_destroy, xd_string_comp, xd_string_hash);

  // Assert
  XD_TEST_ASSERT(map != NULL);
  XD_TEST_ASSERT(map->entry_count == 0);
  XD_TEST_ASSERT(map->bucket_count == 17);
  XD_TEST_ASSERT(map->buckets != NULL);
  for (int i = 0; i < map->bucket_count; i++) {
    XD_TEST_ASSERT(map->buckets[i] != NULL);
    XD_TEST_ASSERT(map->buckets[i]->head == NULL);
    XD_TEST_ASSERT(map->buckets[i]->tail == NULL);
    XD_TEST_ASSERT(map->buckets[i]->length == 0);
  }

xd_test_cleanup:
  xd_map_destroy(map);
  XD_TEST_END;
}  // test_xd_map_create()

static int test_xd_map_put1() {
  XD_TEST_START;

  // Arrange - Act
  xd_map_t *map = xd_map_create(
      xd_string_copy, xd_string_destroy, xd_string_comp, xd_string_copy,
      xd_string_destroy, xd_string_comp, xd_string_hash);
  const char *keys[] = {"1", "11", "2", "22", "3", "33",
                        "4", "44", "5", "55", "6", "66"};
  const char *values[] = {"1", "2", "3", "4",  "5",  "6",
                          "7", "8", "9", "10", "11", "12"};
  int entry_count = sizeof(keys) / sizeof(const char *);
  for (int i = 0; i < entry_count; i++) {
    xd_map_put(map, (void *)keys[i], (void *)values[i]);
  }

  // 1 -> 49 -> 15
  // 2 -> 50 -> 16
  // 3 -> 51 -> 0
  // 4 -> 52 -> 1
  // 5 -> 53 -> 2
  // 6 -> 54 -> 3
  // 7 -> 55 -> 4
  // 8 -> 56 -> 5
  // 9 -> 57 -> 6

  // Assert
  XD_TEST_ASSERT(map->entry_count == entry_count);
  XD_TEST_ASSERT(map->bucket_count == 17);
  XD_TEST_ASSERT(map->buckets != NULL);

  xd_list_t *bucket = map->buckets[0];
  XD_TEST_ASSERT(bucket->length == 2);
  xd_bucket_entry_t *entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "3") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "5") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "33") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "6") == 0);

  bucket = map->buckets[1];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "4") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "7") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "44") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "8") == 0);

  bucket = map->buckets[2];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "5") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "9") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "55") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "10") == 0);

  bucket = map->buckets[3];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "6") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "11") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "66") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "12") == 0);

  for (int i = 4; i <= 14; i++) {
    XD_TEST_ASSERT(map->buckets[i]->length == 0);
    XD_TEST_ASSERT(map->buckets[i]->head == NULL);
    XD_TEST_ASSERT(map->buckets[i]->tail == NULL);
  }

  bucket = map->buckets[15];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "1") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "1") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "11") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "2") == 0);

  bucket = map->buckets[16];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "2") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "3") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "22") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "4") == 0);

xd_test_cleanup:
  xd_map_destroy(map);
  XD_TEST_END;
}  // test_xd_map_put1()

static int test_xd_map_put2() {
  XD_TEST_START;

  // Arrange - Act
  xd_map_t *map = xd_map_create(
      xd_string_copy, xd_string_destroy, xd_string_comp, xd_string_copy,
      xd_string_destroy, xd_string_comp, xd_string_hash);

  const char *keys[] = {"1", "11", "2", "22", "3", "33", "333",
                        "4", "44", "5", "55", "6", "66"};
  const char *values[] = {"1", "2", "3",  "4",  "5",  "6", "7",
                          "8", "9", "10", "11", "12", "13"};
  int entry_count = sizeof(keys) / sizeof(const char *);
  for (int i = 0; i < entry_count; i++) {
    xd_map_put(map, (void *)keys[i], (void *)values[i]);
  }

  // 1 -> 49 -> 12
  // 2 -> 50 -> 13
  // 3 -> 51 -> 14
  // 4 -> 52 -> 15
  // 5 -> 53 -> 16
  // 6 -> 54 -> 17
  // 7 -> 55 -> 18
  // 8 -> 56 -> 19
  // 9 -> 57 -> 20

  // Assert
  XD_TEST_ASSERT(map->entry_count == entry_count);
  XD_TEST_ASSERT(map->bucket_count == 37);
  XD_TEST_ASSERT(map->buckets != NULL);

  for (int i = 0; i < 37; i++) {
    if (i < 12 || i > 17) {
      XD_TEST_ASSERT(map->buckets[i]->length == 0);
      XD_TEST_ASSERT(map->buckets[i]->head == NULL);
      XD_TEST_ASSERT(map->buckets[i]->tail == NULL);
    }
  }

  xd_list_t *bucket = map->buckets[12];
  XD_TEST_ASSERT(bucket->length == 2);
  xd_bucket_entry_t *entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "1") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "1") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "11") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "2") == 0);

  bucket = map->buckets[13];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "2") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "3") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "22") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "4") == 0);

  bucket = map->buckets[14];
  XD_TEST_ASSERT(bucket->length == 3);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "3") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "5") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "33") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "6") == 0);
  entry = bucket->head->next->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "333") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "7") == 0);

  bucket = map->buckets[15];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "4") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "8") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "44") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "9") == 0);

  bucket = map->buckets[16];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "5") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "10") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "55") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "11") == 0);

  bucket = map->buckets[17];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "6") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "12") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "66") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "13") == 0);

xd_test_cleanup:
  xd_map_destroy(map);
  XD_TEST_END;
}  // test_xd_map_put2()

static int test_xd_map_get() {
  XD_TEST_START;

  // Arrange
  xd_map_t *map = xd_map_create(
      xd_string_copy, xd_string_destroy, xd_string_comp, xd_string_copy,
      xd_string_destroy, xd_string_comp, xd_string_hash);

  const char *keys[] = {"1", "11", "2", "22", "3", "33", "333",
                        "4", "44", "5", "55", "6", "66"};
  const char *values[] = {"1", "2", "3",  "4",  "5",  "6", "7",
                          "8", "9", "10", "11", "12", "13"};
  int entry_count = sizeof(keys) / sizeof(const char *);
  for (int i = 0; i < entry_count; i++) {
    xd_map_put(map, (void *)keys[i], (void *)values[i]);
  }

  // Act - Assert
  XD_TEST_ASSERT(strcmp(xd_map_get(map, "333"), "7") == 0);
  XD_TEST_ASSERT(strcmp(xd_map_get(map, "44"), "9") == 0);
  XD_TEST_ASSERT(xd_map_get(map, "x") == NULL);

xd_test_cleanup:
  xd_map_destroy(map);
  XD_TEST_END;
}  // test_xd_map_get()

static int test_xd_map_contains_key() {
  XD_TEST_START;

  // Arrange
  xd_map_t *map = xd_map_create(
      xd_string_copy, xd_string_destroy, xd_string_comp, xd_string_copy,
      xd_string_destroy, xd_string_comp, xd_string_hash);

  const char *keys[] = {"1", "11", "2", "22", "3", "33", "333",
                        "4", "44", "5", "55", "6", "66"};
  const char *values[] = {"1", "2", "3",  "4",  "5",  "6", "7",
                          "8", "9", "10", "11", "12", "13"};
  int entry_count = sizeof(keys) / sizeof(const char *);
  for (int i = 0; i < entry_count; i++) {
    xd_map_put(map, (void *)keys[i], (void *)values[i]);
  }

  // Act - Assert
  XD_TEST_ASSERT(xd_map_contains_key(map, "333") == 1);
  XD_TEST_ASSERT(xd_map_contains_key(map, "44") == 1);
  XD_TEST_ASSERT(xd_map_contains_key(map, "x") == 0);

xd_test_cleanup:
  xd_map_destroy(map);
  XD_TEST_END;
}  // test_xd_map_contains_key()

static int test_xd_map_contains_value() {
  XD_TEST_START;

  // Arrange
  xd_map_t *map = xd_map_create(
      xd_string_copy, xd_string_destroy, xd_string_comp, xd_string_copy,
      xd_string_destroy, xd_string_comp, xd_string_hash);

  const char *keys[] = {"1", "11", "2", "22", "3", "33", "333",
                        "4", "44", "5", "55", "6", "66"};
  const char *values[] = {"1", "2", "3",  "4",  "5",  "6", "7",
                          "8", "9", "10", "11", "12", "13"};
  int entry_count = sizeof(keys) / sizeof(const char *);
  for (int i = 0; i < entry_count; i++) {
    xd_map_put(map, (void *)keys[i], (void *)values[i]);
  }

  // Act - Assert
  XD_TEST_ASSERT(xd_map_contains_value(map, "1") == 1);
  XD_TEST_ASSERT(xd_map_contains_value(map, "3") == 1);
  XD_TEST_ASSERT(xd_map_contains_value(map, "x") == 0);

xd_test_cleanup:
  xd_map_destroy(map);
  XD_TEST_END;
}  // test_xd_map_contains_value()

static int test_xd_map_remove1() {
  XD_TEST_START;

  // Arrange
  xd_map_t *map = xd_map_create(
      xd_string_copy, xd_string_destroy, xd_string_comp, xd_string_copy,
      xd_string_destroy, xd_string_comp, xd_string_hash);

  const char *keys[] = {"1", "11", "2", "22", "3", "33", "333",
                        "4", "44", "5", "55", "6", "66"};
  const char *values[] = {"1", "2", "3",  "4",  "5",  "6", "7",
                          "8", "9", "10", "11", "12", "13"};
  int entry_count = sizeof(keys) / sizeof(const char *);
  for (int i = 0; i < entry_count; i++) {
    xd_map_put(map, (void *)keys[i], (void *)values[i]);
  }

  // Act - Assert
  for (int i = 0; i < entry_count; i++) {
    int ret = xd_map_remove(map, (void *)keys[i]);
    XD_TEST_ASSERT(ret == 0);
  }

  XD_TEST_ASSERT(map != NULL);
  XD_TEST_ASSERT(map->entry_count == 0);
  XD_TEST_ASSERT(map->bucket_count == 17);
  XD_TEST_ASSERT(map->buckets != NULL);
  for (int i = 0; i < map->bucket_count; i++) {
    XD_TEST_ASSERT(map->buckets[i] != NULL);
    XD_TEST_ASSERT(map->buckets[i]->head == NULL);
    XD_TEST_ASSERT(map->buckets[i]->tail == NULL);
    XD_TEST_ASSERT(map->buckets[i]->length == 0);
  }

xd_test_cleanup:
  xd_map_destroy(map);
  XD_TEST_END;
}  // test_xd_map_remove1()

static int test_xd_map_remove2() {
  XD_TEST_START;

  // Arrange
  xd_map_t *map = xd_map_create(
      xd_string_copy, xd_string_destroy, xd_string_comp, xd_string_copy,
      xd_string_destroy, xd_string_comp, xd_string_hash);

  const char *keys[] = {"1", "11", "2", "22", "3", "33", "333",
                        "4", "44", "5", "55", "6", "66"};
  const char *values[] = {"1", "2", "3",  "4",  "5",  "6", "7",
                          "8", "9", "10", "11", "12", "13"};
  int entry_count = sizeof(keys) / sizeof(const char *);
  for (int i = 0; i < entry_count; i++) {
    xd_map_put(map, (void *)keys[i], (void *)values[i]);
  }
  xd_map_remove(map, "55");
  xd_map_remove(map, "6");
  xd_map_remove(map, "66");

  // Act
  int ret1 = xd_map_remove(map, "5");
  int ret2 = xd_map_remove(map, "f");
  int ret3 = xd_map_remove(NULL, "1");
  // 1 -> 49 -> 15
  // 2 -> 50 -> 16
  // 3 -> 51 -> 0
  // 4 -> 52 -> 1
  // 5 -> 53 -> 2
  // 6 -> 54 -> 3
  // 7 -> 55 -> 4
  // 8 -> 56 -> 5
  // 9 -> 57 -> 6

  // Assert
  XD_TEST_ASSERT(ret1 == 0);
  XD_TEST_ASSERT(ret2 == -1);
  XD_TEST_ASSERT(ret3 == -1);
  XD_TEST_ASSERT(map->entry_count == entry_count - 4);
  XD_TEST_ASSERT(map->bucket_count == 17);
  XD_TEST_ASSERT(map->buckets != NULL);

  xd_list_t *bucket = map->buckets[0];
  printf("%d\n", bucket->length);
  XD_TEST_ASSERT(bucket->length == 3);
  xd_bucket_entry_t *entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "3") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "5") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "33") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "6") == 0);
  entry = bucket->head->next->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "333") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "7") == 0);

  bucket = map->buckets[1];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "4") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "8") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "44") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "9") == 0);

  for (int i = 2; i <= 14; i++) {
    XD_TEST_ASSERT(map->buckets[i]->length == 0);
    XD_TEST_ASSERT(map->buckets[i]->head == NULL);
    XD_TEST_ASSERT(map->buckets[i]->tail == NULL);
  }

  bucket = map->buckets[15];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "1") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "1") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "11") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "2") == 0);

  bucket = map->buckets[16];
  XD_TEST_ASSERT(bucket->length == 2);
  entry = bucket->head->data;
  XD_TEST_ASSERT(strcmp(entry->key, "2") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "3") == 0);
  entry = bucket->head->next->data;
  XD_TEST_ASSERT(strcmp(entry->key, "22") == 0);
  XD_TEST_ASSERT(strcmp(entry->value, "4") == 0);

xd_test_cleanup:
  xd_map_destroy(map);
  XD_TEST_END;
}  // test_xd_map_remove2()

static int test_xd_map_clear() {
  XD_TEST_START;

  // Arrange
  xd_map_t *map = xd_map_create(
      xd_string_copy, xd_string_destroy, xd_string_comp, xd_string_copy,
      xd_string_destroy, xd_string_comp, xd_string_hash);

  const char *keys[] = {"1", "11", "2", "22", "3", "33", "333",
                        "4", "44", "5", "55", "6", "66"};
  const char *values[] = {"1", "2", "3",  "4",  "5",  "6", "7",
                          "8", "9", "10", "11", "12", "13"};
  int entry_count = sizeof(keys) / sizeof(const char *);
  for (int i = 0; i < entry_count; i++) {
    xd_map_put(map, (void *)keys[i], (void *)values[i]);
  }

  // Act
  xd_map_clear(map);

  // Assert
  XD_TEST_ASSERT(map->entry_count == 0);
  XD_TEST_ASSERT(map->bucket_count == 17);
  XD_TEST_ASSERT(map->buckets != NULL);

  for (int i = 0; i < 17; i++) {
    XD_TEST_ASSERT(map->buckets[i]->length == 0);
    XD_TEST_ASSERT(map->buckets[i]->head == NULL);
    XD_TEST_ASSERT(map->buckets[i]->tail == NULL);
  }

xd_test_cleanup:
  xd_map_destroy(map);
  XD_TEST_END;
}  // test_xd_map_clear()

static int test_xd_map_to_array() {
  XD_TEST_START;

  // Arrange
  xd_map_t *map = xd_map_create(
      xd_string_copy, xd_string_destroy, xd_string_comp, xd_string_copy,
      xd_string_destroy, xd_string_comp, xd_string_hash);

  const char *keys[] = {"1", "11", "2", "22", "3", "33", "333",
                        "4", "44", "5", "55", "6", "66"};
  const char *values[] = {"1", "2", "3",  "4",  "5",  "6", "7",
                          "8", "9", "10", "11", "12", "13"};
  int entry_count = sizeof(keys) / sizeof(const char *);
  for (int i = 0; i < entry_count; i++) {
    xd_map_put(map, (void *)keys[i], (void *)values[i]);
  }

  // Act
  char **arr = (char **)xd_map_to_array(map);

  for (int i = 0; i < entry_count; i++) {
    XD_TEST_ASSERT(strcmp(arr[i], values[i]) == 0);
  }
  XD_TEST_ASSERT(arr[entry_count] == NULL);

xd_test_cleanup:
  xd_map_destroy(map);
  XD_TEST_END;
}  // test_xd_map_to_array()

static xd_test_case test_suite[] = {
    XD_TEST_CASE(test_xd_map_create),
    XD_TEST_CASE(test_xd_map_put1),
    XD_TEST_CASE(test_xd_map_put2),
    XD_TEST_CASE(test_xd_map_get),
    XD_TEST_CASE(test_xd_map_contains_key),
    XD_TEST_CASE(test_xd_map_contains_value),
    XD_TEST_CASE(test_xd_map_remove1),
    XD_TEST_CASE(test_xd_map_remove2),
    XD_TEST_CASE(test_xd_map_clear),
    XD_TEST_CASE(test_xd_map_to_array),
};

int main() {
  XD_TEST_RUN_ALL(test_suite);
}  // main()
