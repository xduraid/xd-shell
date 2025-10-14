/*
 * ==============================================================================
 * File: xd_aliases.h
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

#include "xd_list.h"

/**
 * @brief Initializes the aliases hash map.
 *
 * @warning This function may call `exit(EXIT_FAILURE)` if memory allocation
 * fails.
 */
void xd_aliases_init();

/**
 * @brief Frees the memory allocated for the aliases hash map.
 */
void xd_aliases_destroy();

/**
 * @brief Clears the aliases hash map, removing all defined aliases and
 * resetting it to its initial new state.
 *
 * @warning This function may call `exit(EXIT_FAILURE)` if memory allocation
 * fails when rehashing the aliases map.
 */
void xd_aliases_clear();

/**
 * @brief Retrieves the value of an alias by its name.
 *
 * @param name Pointer to the null-terminated string representing the alias
 * name.
 *
 * @return A pointer to the value associated with the alias name, or `NULL`
 * if the alias does not exist.
 *
 * @note The returned string is owned by the aliases map and must not
 * be freed or modified directly.
 */
char *xd_aliases_get(char *name);

/**
 * @brief Inserts a new alias or updates an existing alias in the aliases map.
 *
 * @param name Pointer to the null-terminated string representing the alias
 * name.
 * @param value Pointer to the null-terminated string representing the alias
 * value.
 *
 * @warning This function may call `exit(EXIT_FAILURE)` if memory allocation
 * fails.
 */
void xd_aliases_put(char *name, char *value);

/**
 * @brief Removes an alias from the aliases map by its name.
 *
 * @param name Pointer to the null-terminated string representing the alias
 * name.
 *
 * @return `0` if the alias was found and removed, `-1` otherwise.
 *
 * @warning This function may call `exit(EXIT_FAILURE)` if memory allocation
 * fails when rehashing the aliases map.
 */
int xd_aliases_remove(char *name);

/**
 * @brief Returns a newly allocated `xd_list` structure containing the names of
 * all defined aliases.
 *
 * @return Pointer to a newly allocated `xd_list_t` containing the names of all
 * defined aliases or `NULL` on failure.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_listr_destroy()` and passing it the returned pointer.
 */
xd_list_t *xd_aliases_names_list();

/**
 * @brief Prints all aliases to stdout.
 */
void xd_aliases_print_all();

/**
 * @brief Checks if the passed string is a valid alias name.
 *
 * @param name Pointer to the null-terminated string to be checked.
 *
 * @return `1` if the passed string is a valid alias name, `0` otherwise.
 */
int xd_aliases_is_valid_name(const char *name);
