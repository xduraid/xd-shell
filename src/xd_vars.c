/*
 * ==============================================================================
 * File: xd_vars.c
 * Author: Duraid Maihoub
 * Date: 28 Aug 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#include "xd_vars.h"

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
// Typedefs
// ========================

/**
 * @brief Represents a shell variable.
 */
typedef struct xd_var_t {
  char *name;       // Variable name
  char *value;      // Variable value
  int is_exported;  // Whether exported (an environment variable) or not
} xd_var_t;

// ========================
// Function Declarations
// ========================

static void *xd_var_copy_func(void *data);
static void xd_var_destroy_func(void *data);
static int xd_var_comp_func(const void *data1, const void *data2);

// ========================
// Variables
// ========================

/**
 * @brief Shell variables hash map.
 */
static xd_map_t *xd_vars = NULL;

// ========================
// Function Definitions
// ========================

/**
 * @brief Creates a newly-allocated copy of the passed `xd_var_t` structure.
 *
 * @param data Pointer to the `xd_var_t` structure to be copied.
 *
 * @return A pointer to the newly allocated copy of the variable, or `NULL` if
 * the passed pointer is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The returned string must be freed with `xd_var_destroy_func()`.
 */
static void *xd_var_copy_func(void *data) {
  if (data == NULL) {
    return NULL;
  }

  xd_var_t *var = data;
  xd_var_t *copy = (xd_var_t *)malloc(sizeof(xd_var_t));
  if (copy == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  copy->name = strdup(var->name);
  copy->value = strdup(var->value);
  copy->is_exported = var->is_exported;
  if (copy->name == NULL || copy->value == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  return copy;
}  // xd_var_copy_func()

/**
 * @brief Frees the memory allocated for the passed `xd_var_t` structure.
 *
 * @param data Pointer to the `xd_var_t` structure to be freed.
 *
 * @note If the passed pointer is `NULL` no action shall occur.
 */
static void xd_var_destroy_func(void *data) {
  if (data == NULL) {
    return;
  }
  xd_var_t *var = data;
  free(var->name);
  free(var->value);
  free(var);
}  // xd_var_destroy_func()

/**
 * @brief Compares two shell variables based on their values.
 *
 * @param data1 Pointer to the first `xd_var_t` structure.
 * @param data2 Pointer to the second `xd_var_t` structure.
 *
 * @return
 * - 0 if both variables are `NULL` or if their values are equal.
 *
 * - A negative value if `data1` is `NULL` or its value is lexicographically
 *   less than `data2`'s value.
 *
 * - A positive value if `data2` is `NULL` or its value is lexicographically
 *   greater than `data1`'s value.
 */
static int xd_var_comp_func(const void *data1, const void *data2) {
  const xd_var_t *var1 = data1;
  const xd_var_t *var2 = data2;
  if (var1 == NULL && var2 == NULL) {
    return 0;
  }
  if (var1 == NULL) {
    return -1;
  }
  if (var2 == NULL) {
    return 1;
  }
  return xd_utils_str_comp_func(var1->value, var2->value);
}  // xd_var_comp_func()

// ========================
// Public Functions
// ========================

void xd_vars_init() {
  xd_vars = xd_map_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                          xd_utils_str_comp_func, xd_var_copy_func,
                          xd_var_destroy_func, xd_var_comp_func,
                          xd_utils_str_hash_func);
  // load environment variables
}  // xd_vars_init()

void xd_vars_destroy() {
  xd_map_destroy(xd_vars);
}  // xd_vars_destroy()

char *xd_vars_get(char *name) {
  xd_var_t *var = xd_map_get(xd_vars, name);
  return var == NULL ? NULL : var->value;
}  // xd_vars_get()

void xd_vars_put(char *name, char *value, int is_exported) {
  xd_var_t new_var = {name, value, is_exported};
  xd_map_put(xd_vars, name, &new_var);
}  // xd_vars_put()

int xd_vars_remove(char *name) {
  return xd_map_remove(xd_vars, name);
}  // xd_vars_remove()

int xd_vars_is_exported(char *name) {
  xd_var_t *var = xd_map_get(xd_vars, name);
  if (var != NULL && var->is_exported) {
    return 1;
  }
  return 0;
}  // xd_vars_is_exported()

void xd_vars_print_all() {
  if (xd_vars == NULL) {
    return;
  }
  for (int i = 0; i < xd_vars->bucket_count; i++) {
    xd_list_t *bucket = xd_vars->buckets[i];
    for (xd_list_node_t *node = bucket->head; node != NULL; node = node->next) {
      xd_bucket_entry_t *entry = node->data;
      char *name = entry->key;
      xd_var_t *var = entry->value;
      printf("set %s='%s'\n", name, var->value);
    }
  }
}  // xd_vars_print_all()

int xd_vars_is_valid_name(const char *name) {
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
}  // xd_vars_is_valid_name()
