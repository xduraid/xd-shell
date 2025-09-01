/*
 * ==============================================================================
 * File: xd_vars.h
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

/**
 * @brief Initializes the variables hash map and loads the environment.
 *
 * @warning This function may call `exit(EXIT_FAILURE)` if memory allocation
 * fails.
 */
void xd_vars_init();

/**
 * @brief Frees the memory allocated for the variables hash map.
 */
void xd_vars_destroy();

/**
 * @brief Retrieves the value of a variable by its name.
 *
 * @param name Pointer to the null-terminated string representing the variable
 * name.
 *
 * @return A pointer to the value associated with the variable name, or `NULL`
 * if the variable does not exist.
 *
 * @note The returned string is owned by the variables map and must not be freed
 * or modified directly.
 */
char *xd_vars_get(char *name);

/**
 * @brief Inserts a new variable or updates an existing variable in the
 * variables map.
 *
 * @param name Pointer to the null-terminated string representing the variable
 * name.
 * @param value Pointer to the null-terminated string representing the variable
 * value.
 * @param is_exported Whether the variable is exported (an environment variable)
 * or not.
 *
 * @warning This function may call `exit(EXIT_FAILURE)` if memory allocation
 * fails.
 */
void xd_vars_put(char *name, char *value, int is_exported);

/**
 * @brief Removes a variable from the variables map by its name.
 *
 * @param name Pointer to the null-terminated string representing the variable
 * name.
 *
 * @return `0` if the variables was found and removed, `-1` otherwise.
 *
 * @warning This function may call `exit(EXIT_FAILURE)` if memory allocation
 * fails when rehashing the variables map.
 */
int xd_vars_remove(char *name);

/**
 * @brief Checks if the variable with the passed name is an exported
 * (environment) variable or not.
 *
 * @param name Pointer to the null-terminated string representing the variable
 * name.
 *
 * @return `1` if the variable is found and is exported, `0` otherwise.
 */
int xd_vars_is_exported(char *name);

/**
 * @brief Prints all variables to stdout in the reusable form
 * `set name=value`.
 */
void xd_vars_print_all();

/**
 * @brief Prints all exported variables to stdout in the reusable form
 * `export name=value`.
 */
void xd_vars_print_all_exported();

/**
 * @brief Constructs a null-terminated array of environment variables and
 * returns it.
 *
 * @return A newly allocated null-terminated array of string pointers containing
 * all defined environment variables.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_vars_destroy_envp()` and passing it the returned pointer.
 */
char **xd_vars_create_envp();

/**
 * @brief Frees an environment array created by `xd_vars_get_env()`.
 *
 * @param envp The null-terminated array of environment variable strings to be
 * freed.
 *
 * @note If the passed pointer is `NULL`, no action shall occur.
 */
void xd_vars_destroy_envp(char **envp);

/**
 * @brief Checks if the passed string is a valid variable name.
 *
 * @param name Pointer to the null-terminated string to be checked.
 *
 * @return `1` if the passed string is a valid variable name, `0` otherwise.
 */
int xd_vars_is_valid_name(const char *name);
