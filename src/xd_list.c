/*
 * ==============================================================================
 * File: xd_list.c
 * Author: Duraid Maihoub
 * Date: 26 July 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#include "xd_list.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xd_generic_funcs.h"

// ========================
// Function Declarations
// ========================

static xd_list_node_t *xd_list_node_create(void *data,
                                           xd_gens_copy_func_t copy_func);
static void xd_list_node_destroy(xd_list_node_t *node,
                                 xd_gens_destroy_func_t destroy_func);

// ========================
// Function Definitions
// ========================

/**
 * @brief Creates and initializes a new `xd_list_node_t` structure.
 *
 * @param copy_func A pointer to the function used to create a copy of the
 * passed data.
 * @param data A pointer to the data to be copied and stored in the node.
 *
 * @return A pointer to the newly created `xd_list_node_t`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_list_node_destroy()` and passing it the returned pointer.
 */
static xd_list_node_t *xd_list_node_create(void *data,
                                           xd_gens_copy_func_t copy_func) {
  xd_list_node_t *node = (xd_list_node_t *)malloc(sizeof(xd_list_node_t));
  if (node == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  node->data = copy_func(data);
  node->prev = node->next = NULL;
  return node;
}  // xd_list_node_create()

/**
 * @brief Frees the memory allocated for the passed `xd_list_node_t` structure.
 *
 * @param node A pointer to the `xd_list_node_t` to be freed.
 * @param destroy_func A pointer to the function used to free the data stored in
 * the node.
 *
 * @note If the passed node pointer is `NULL` no action shall occur.
 */
static void xd_list_node_destroy(xd_list_node_t *node,
                                 xd_gens_destroy_func_t destroy_func) {
  if (node == NULL) {
    return;
  }
  destroy_func(node->data);
  free(node);
}  // xd_list_node_destroy()

// ========================
// Public Functions
// ========================

xd_list_t *xd_list_create(xd_gens_copy_func_t copy_func,
                          xd_gens_destroy_func_t destroy_func,
                          xd_gens_comp_func_t comp_func) {
  if (copy_func == NULL || destroy_func == NULL || comp_func == NULL) {
    return NULL;
  }
  xd_list_t *list = (xd_list_t *)malloc(sizeof(xd_list_t));
  if (list == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  list->copy_func = copy_func;
  list->destroy_func = destroy_func;
  list->comp_func = comp_func;
  list->length = 0;
  list->head = list->tail = NULL;
  return list;
}  // xd_list_create()

void xd_list_destroy(xd_list_t *list) {
  if (list == NULL) {
    return;
  }
  xd_list_clear(list);
  free(list);
}  // xd_list_destroy()

void xd_list_clear(xd_list_t *list) {
  if (list == NULL) {
    return;
  }
  xd_list_node_t *curr = list->head;
  xd_list_node_t *prev = NULL;
  while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    xd_list_node_destroy(prev, list->destroy_func);
  }
  list->head = list->tail = NULL;
  list->length = 0;
}  // xd_list_clear()

void xd_list_add_first(xd_list_t *list, void *data) {
  if (list == NULL) {
    return;
  }
  xd_list_node_t *node = xd_list_node_create(data, list->copy_func);
  if (list->length == 0) {
    list->head = list->tail = node;
  }
  else {
    node->next = list->head;
    list->head->prev = node;
    list->head = node;
  }
  list->length++;
}  // xd_list_add_first()

void xd_list_add_last(xd_list_t *list, void *data) {
  if (list == NULL) {
    return;
  }
  xd_list_node_t *node = xd_list_node_create(data, list->copy_func);
  if (list->length == 0) {
    list->head = list->tail = node;
  }
  else {
    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
  }
  list->length++;
}  // xd_list_add_last()

int xd_list_remove_first(xd_list_t *list) {
  if (list == NULL || list->length == 0) {
    return -1;
  }
  xd_list_node_t *node = list->head;
  if (list->length == 1) {
    list->head = list->tail = NULL;
  }
  else {
    list->head = list->head->next;
    list->head->prev = NULL;
  }
  list->length--;
  xd_list_node_destroy(node, list->destroy_func);
  return 0;
}  // xd_list_remove_first()

int xd_list_remove_last(xd_list_t *list) {
  if (list == NULL || list->length == 0) {
    return -1;
  }
  xd_list_node_t *node = list->tail;
  if (list->length == 1) {
    list->head = list->tail = NULL;
  }
  else {
    list->tail = list->tail->prev;
    list->tail->next = NULL;
  }
  xd_list_node_destroy(node, list->destroy_func);
  list->length--;
  return 0;
}  // xd_list_remove_last()

int xd_list_remove(xd_list_t *list, void *data) {
  if (list == NULL || list->comp_func == NULL) {
    return -1;
  }
  xd_list_node_t *node = xd_list_find_node(list, data);
  return xd_list_remove_node(list, node);
}  // xd_list_remove()

void *xd_list_find(xd_list_t *list, void *data) {
  xd_list_node_t *node = xd_list_find_node(list, data);
  return node == NULL ? NULL : node->data;
}  // xd_list_find()

void *xd_list_find_last(xd_list_t *list, void *data) {
  xd_list_node_t *node = xd_list_find_last_node(list, data);
  return node == NULL ? NULL : node->data;
}  // xd_list_find_last()

void *xd_list_get(xd_list_t *list, int index) {
  xd_list_node_t *node = xd_list_get_node(list, index);
  return node == NULL ? NULL : node->data;
}  // xd_list_get()

xd_list_node_t *xd_list_find_node(xd_list_t *list, void *data) {
  if (list == NULL || list->comp_func == NULL) {
    return NULL;
  }

  xd_list_node_t *curr = list->head;
  while (curr != NULL) {
    if (list->comp_func(curr->data, data) == 0) {
      break;
    }
    curr = curr->next;
  }

  return curr;
}  // xd_list_find_node()

xd_list_node_t *xd_list_find_last_node(xd_list_t *list, void *data) {
  if (list == NULL || list->comp_func == NULL) {
    return NULL;
  }

  xd_list_node_t *curr = list->tail;
  while (curr != NULL) {
    if (list->comp_func(curr->data, data) == 0) {
      break;
    }
    curr = curr->prev;
  }

  return curr;
}  // xd_list_find_last_node()

int xd_list_remove_node(xd_list_t *list, xd_list_node_t *node) {
  if (list == NULL || node == NULL) {
    return -1;
  }
  if (list->head == node) {
    return xd_list_remove_first(list);
  }
  if (list->tail == node) {
    return xd_list_remove_last(list);
  }

  node->prev->next = node->next;
  node->next->prev = node->prev;

  xd_list_node_destroy(node, list->destroy_func);
  list->length--;

  return 0;
}  // xd_list_remove_node()

void *xd_list_get_node(xd_list_t *list, int index) {
  if (list == NULL || index < 0 || index >= list->length) {
    return NULL;
  }
  xd_list_node_t *curr = list->head;
  for (int i = 0; i < index; i++) {
    curr = curr->next;
  }
  return curr;
}  // xd_list_get_node()
