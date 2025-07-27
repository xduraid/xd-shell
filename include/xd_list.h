/*
 * ==============================================================================
 * File: xd_list.h
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

#ifndef XD_LIST_H
#define XD_LIST_H

#include "xd_generic_funcs.h"

// ========================
// Typedefs
// ========================

/**
 * @brief Represents a node in a doubly linked list.
 */
typedef struct xd_list_node_t {
  struct xd_list_node_t *prev;  // Pointer to the previous node
  struct xd_list_node_t *next;  // Pointer to the next node
  void *data;                   // Pointer to the data stored in the node
} xd_list_node_t;

/**
 * @brief Represents a generic doubly linked list.
 */
typedef struct xd_list_t {
  xd_list_node_t *head;           // Pointer to the first node in the list
  xd_list_node_t *tail;           // Pointer to the last node in the list
  int length;                     // Number of elements currently in the list
  xd_gens_copy_func_t copy_func;  // Function to copy data into new nodes
  xd_gens_destroy_func_t destroy_func;  // Function to free data in nodes
  xd_gens_comp_func_t comp_func;  // Function to compare data between nodes
} xd_list_t;

// ========================
// Function Declarations
// ========================

/**
 * @brief Creates and initializes a new `xd_list_t` structure.
 *
 * @param copy_func A pointer to the function used to copy data into the list.
 * @param destroy_func A pointer to the function used to free data stored in the
 * list.
 * @param comp_func A pointer to the function used to compare two data elements
 * in the list.
 *
 * @return A pointer to the newly created `xd_list_t` on success, or `NULL` if
 * any of the provided function pointers are `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_list_destroy()` and passing it the returned pointer.
 */
xd_list_t *xd_list_create(xd_gens_copy_func_t copy_func,
                          xd_gens_destroy_func_t destroy_func,
                          xd_gens_comp_func_t comp_func);

/**
 * @brief Frees the memory allocated for the passed `xd_list_t` structure.
 *
 * @param list A pointer to the `xd_list_t` to be freed.
 *
 * @note If the passed list pointer is `NULL` no action shall occur.
 */
void xd_list_destroy(xd_list_t *list);

/**
 * @brief Removes all elements from the passed list, leaving it empty.
 *
 * @param list A pointer to the list to be cleared.
 */
void xd_list_clear(xd_list_t *list);

/**
 * @brief Inserts a copy of the passed element at the beginning of the
 * passed list.
 *
 * @param list A pointer to the `xd_list_t` to add the element to.
 * @param data A pointer to the data element to be added.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note If the passed list pointer is `NULL` no action shall occur.
 */
void xd_list_add_first(xd_list_t *list, void *data);

/**
 * @brief Inserts a copy of the passed element at the end of the passed list.
 *
 * @param list A pointer to the `xd_list_t` to add the element to.
 * @param data A pointer to the data element to be added.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note If the passed list pointer is `NULL` no action shall occur.
 */
void xd_list_add_last(xd_list_t *list, void *data);

/**
 * @brief Removes the first element of the passed list.
 *
 * @param list A pointer to the `xd_list_t` to remove the first element from.
 *
 * @return `0` on success, `-1` if the list is `NULL` or empty.
 */
int xd_list_remove_first(xd_list_t *list);

/**
 * @brief Removes the last element of the passed list.
 *
 * @param list A pointer to the `xd_list_t` to remove the last element from.
 *
 * @return `0` on success, `-1` if the list is `NULL` or empty.
 */
int xd_list_remove_last(xd_list_t *list);

/**
 * @brief Remove the first element in the passed list that matches the passed
 * data.
 *
 * @param list A pointer to the `xd_list_t` to remove from.
 * @param data A pointer to the data element to be removed.
 *
 * @return `0` on success, `-1` if the list is `NULL` or if no matching element
 * is found.
 *
 * @warning The comparison function (`comp_func`) must be non-NULL or this
 * function will not work and will return `-1`.
 */
int xd_list_remove(xd_list_t *list, void *data);

/**
 * @brief Searches for the first element in the passed list that matches the
 * passed data.
 *
 * @param list A pointer to the `xd_list_t` to be searched.
 * @param data A pointer to the data element to be searched for.
 *
 * @return A pointer to the matching element's data, or `NULL` if not found.
 *
 * @warning The comparison function (`comp_func`) must be non-NULL or this
 * function will not work and will return `NULL`.
 */
void *xd_list_find(xd_list_t *list, void *data);

/**
 * @brief Searches for the last element in the passed list that matches the
 * passed data.
 *
 * @param list A pointer to the `xd_list_t` to be searched.
 * @param data A pointer to the data element to be searched for.
 *
 * @return A pointer to the matching element's data, or `NULL` if not found.
 *
 * @warning The comparison function (`comp_func`) must be non-NULL or this
 * function will not work and will return `NULL`.
 */
void *xd_list_find_last(xd_list_t *list, void *data);

/**
 * @brief Returns the element at the passed index from the passed list.
 *
 * @param list A pointer of the `xd_list_t`.
 * @param index The index of the element to be returned.
 *
 * @return A pointer to the element at the passed index, or `NULL` if the passed
 * list is `NULL` or if the index is out of bounds.
 */
void *xd_list_get(xd_list_t *list, int index);

/**
 * @brief Searches for the first node in the passed list that matches the passed
 * data.
 *
 * @param list A pointer to the `xd_list_t` to be searched.
 * @param data A pointer to the data element to be searched for.
 *
 * @return A pointer to the matching node, or `NULL` if no match is found or if
 * the passed list is `NULL`.
 *
 * @warning The comparison function (`comp_func`) must be non-NULL or this
 * function will not work and will return `NULL`.
 */
xd_list_node_t *xd_list_find_node(xd_list_t *list, void *data);

/**
 * @brief Searches for the last node in the passed list that matches the passed
 * data.
 *
 * @param list A pointer to the `xd_list_t` to be searched.
 * @param data A pointer to the data element to be searched for.
 *
 * @return A pointer to the matching node, or `NULL` if no match is found or if
 * the passed list is `NULL`.
 *
 * @warning The comparison function (`comp_func`) must be non-NULL or this
 * function will not work and will return `NULL`.
 */
xd_list_node_t *xd_list_find_last_node(xd_list_t *list, void *data);

/**
 * @brief Removes the passed node from the passed list.
 *
 * @param list A pointer to the `xd_list_t` to remove the node from.
 * @param node A pointer to the `xd_list_node_t` to be removed.
 *
 * @return `0` on sucess, or -1 if either `list` or `node` is `NULL`.
 *
 * @warning The `node` must be part of the provided `list`. Passing a node not
 * owned by the list leads to undefined behavior such as memory corruption or
 * crashes.
 *
 * @warning After this operation, the `node` is deallocated and must not be
 * accessed.
 */
int xd_list_remove_node(xd_list_t *list, xd_list_node_t *node);

/**
 * @brief Returns the node at the passed index from the passed list.
 *
 * @param list A pointer of the `xd_list_t`.
 * @param index The index of the node to be returned.
 *
 * @return A pointer to the node at the passed index, or `NULL` if the passed
 * list is `NULL` or if the index is out of bounds.
 */
void *xd_list_get_node(xd_list_t *list, int index);

#endif  // XD_LIST_H
