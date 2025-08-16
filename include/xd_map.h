/*
 * ==============================================================================
 * File: xd_map.h
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

#ifndef XD_MAP_H
#define XD_MAP_H

#include "xd_generic_funcs.h"
#include "xd_list.h"

// ========================
// Typedefs
// ========================

/**
 * @brief Represents a hash map bucket entry.
 */
typedef struct xd_bucket_entry_t {
  void *key;    // The key
  void *value;  // The value
} xd_bucket_entry_t;

/**
 * @brief Represents a generic hash map.
 */
typedef struct xd_map_t {
  xd_list_t **buckets;                        // Array of buckets
  int bucket_count;                           // Number of buckets
  int entry_count;                            // Number of key-value pairs
  xd_gens_copy_func_t copy_key_func;          // Function to copy keys
  xd_gens_copy_func_t copy_value_func;        // Function to copy values
  xd_gens_destroy_func_t destroy_key_func;    // Function to destroy keys
  xd_gens_destroy_func_t destroy_value_func;  // Function to destroy values
  xd_gens_comp_func_t comp_key_func;          // Function to compare keys
  xd_gens_comp_func_t comp_value_func;        // Function to compare values
  xd_gens_hash_func_t hash_func;              // Function to hash the keys
} xd_map_t;

// ========================
// Function Declarations
// ========================

/**
 * @brief Creates and initializes a new `xd_map_t` structure.
 *
 * @param copy_key_func A pointer to the function used to copy the keys.
 * @param destroy_key_func A pointer to the function used to free the keys.
 * @param comp_key_func A pointer to the function used to compare the keys.
 * @param copy_value_func A pointer to the function used to copy the values.
 * @param destroy_value_func A pointer to the function used to free the values.
 * @param comp_value_func A pointer to the function used to compare the values.
 * @param hash_func A pointer to the function used to hash the keys.
 *
 * @return A pointer to the newly created `xd_map_t` on success, or `NULL`
 * if any of the provided function pointers are `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by
 * calling `xd_map_destroy()` and passing it the returned pointer.
 */
xd_map_t *xd_map_create(xd_gens_copy_func_t copy_key_func,
                        xd_gens_destroy_func_t destroy_key_func,
                        xd_gens_comp_func_t comp_key_func,
                        xd_gens_copy_func_t copy_value_func,
                        xd_gens_destroy_func_t destroy_value_func,
                        xd_gens_comp_func_t comp_value_func,
                        xd_gens_hash_func_t hash_func);

/**
 * @brief Frees the memory allocated for the passed `xd_map_t` structure.
 *
 * @param map A pointer to the `xd_map_t` to be freed.
 *
 * @note If the passed map pointer is `NULL` no action shall occur.
 */
void xd_map_destroy(xd_map_t *map);

/**
 * @brief Clears the passed map and removes all its entries, restting it to its
 * initial newly created state.
 *
 * @param map A pointer to the `xd_map_t` to be cleared.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
void xd_map_clear(xd_map_t *map);

/**
 * @brief Inserts or updates the passed key-value pair in the passed map.
 *
 * @param map Pointer to the `xd_map_t` structure to modify.
 * @param key Pointer to the key of the entry to be added or updated.
 * @param value Pointer to the value associate with the key.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
void xd_map_put(xd_map_t *map, void *key, void *value);

/**
 * @brief Removes the entry with the passed key from the passed map if exists.
 *
 * @param map Pointer to the `xd_map_t` structure to remove from.
 * @param key Pointer to the key of the entry to be removed.
 *
 * @return `0` if the entry was found or removed, `-1` otherwise.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure when
 * re-hashing.
 */
int xd_map_remove(xd_map_t *map, void *key);

/**
 * @brief Retrieves the value associated with the passed key from the passed
 * map.
 *
 * @param map Pointer to the `xd_map_t` structure to search in.
 * @param key Pointer to the key whose associated value is to be returned.
 *
 * @return A pointer to the value associated with the key if found,
 * or `NULL` if the key does not exist in the map or if the map is `NULL`.
 *
 * @note The returned value is owned by the map and must not be freed
 * or modified directly by the caller.
 */
void *xd_map_get(xd_map_t *map, void *key);

/**
 * @brief Checks if the passed key exists in the passed map.
 *
 * @param map Pointer to the `xd_map_t` structure to search in.
 * @param key Pointer to the key to look for.
 *
 * @return `1` if the key exists in the map, `0` otherwise.
 *
 * @note If the passed map pointer is `NULL`, this function will return `0`.
 */
int xd_map_contains_key(xd_map_t *map, void *key);

/**
 * @brief Checks if the passed value exists in the passed map.
 *
 * @param map Pointer to the `xd_map_t` structure to search in.
 * @param key Pointer to the value to look for.
 *
 * @return `1` if the value exists in the map, `0` otherwise.
 *
 * @note If the passed map pointer is `NULL`, this function will return `0`.
 */
int xd_map_contains_value(xd_map_t *map, void *key);

/**
 * @brief Returns a `NULL` terminated array containing all values stored in the
 * map.
 *
 * @param map Pointer to the `xd_map_t` structure to convert.
 *
 * @return A newly allocated array of `void *` pointers containing all
 * values stored in the map. Returns `NULL` if the passed map pointer is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the returned array using
 * `free()` after use. The values themselves are not duplicated and remain
 * owned by the map.
 */
void **xd_map_to_array(xd_map_t *map);

#endif  // XD_MAP_H
