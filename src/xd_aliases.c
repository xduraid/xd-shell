/*
 * ==============================================================================
 * File: xd_aliases.c
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

#include "xd_aliases.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xd_list.h"
#include "xd_map.h"
#include "xd_utils.h"

// ========================
// Macros
// ========================

// ========================
// Function Declarations
// ========================

// ========================
// Variables
// ========================

/**
 * @brief Hash-map of defined aliases.
 */
static xd_map_t *xd_aliases = NULL;

// ========================
// Function Definitions
// ========================

// ========================
// Public Functions
// ========================

void xd_aliases_init() {
  xd_aliases = xd_map_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                             xd_utils_str_comp_func, xd_utils_str_copy_func,
                             xd_utils_str_destroy_func, xd_utils_str_comp_func,
                             xd_utils_str_hash_func);
}  // xd_aliases_init()

void xd_aliases_destroy() {
  xd_map_destroy(xd_aliases);
}  // xd_aliases_destroy()

void xd_aliases_clear() {
  xd_map_clear(xd_aliases);
}  // xd_aliases_clear()

char *xd_aliases_get(char *name) {
  return xd_map_get(xd_aliases, name);
}  // xd_aliases_get()

void xd_aliases_put(char *name, char *value) {
  xd_map_put(xd_aliases, name, value);
}  // xd_aliases_put()

int xd_aliases_remove(char *name) {
  return xd_map_remove(xd_aliases, name);
}  // xd_aliases_remove()

xd_list_t *xd_aliases_names_list() {
  if (xd_aliases == NULL) {
    return NULL;
  }

  xd_list_t *list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);
  for (int i = 0; i < xd_aliases->bucket_count; i++) {
    xd_list_t *bucket = xd_aliases->buckets[i];
    for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
      xd_bucket_entry_t *entry = node->data;
      char *name = entry->key;
      xd_list_add_last(list, name);
    }
  }

  return list;
}  // xd_aliases_names_list()

void xd_aliases_print_all() {
  if (xd_aliases == NULL) {
    return;
  }
  for (int i = 0; i < xd_aliases->bucket_count; i++) {
    xd_list_t *bucket = xd_aliases->buckets[i];
    for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
      xd_bucket_entry_t *entry = node->data;
      char *name = entry->key;
      char *value = entry->value;
      printf("alias %s='%s'\n", name, value);
    }
  }
}  // xd_aliases_print()

int xd_aliases_is_valid_name(const char *name) {
  if (name == NULL || *name == '\0') {
    return 0;
  }
  if (*name != '_' && !isalpha(*name)) {
    return 0;
  }
  for (int i = 1; name[i] != '\0'; i++) {
    if (name[i] != '_' && !isalnum(name[i])) {
      return 0;
    }
  }
  return 1;
}  // xd_aliases_is_valid_name()
