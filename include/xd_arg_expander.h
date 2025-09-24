/*
 * ==============================================================================
 * File: xd_arg_expander.h
 * Author: Duraid Maihoub
 * Date: 19 Sep 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#ifndef XD_ARG_EXPANDER_H
#define XD_ARG_EXPANDER_H

#include "xd_list.h"

/**
 * @brief Initializes the argument expander.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
void xd_arg_expander_init();

/**
 * @brief Frees resources allocated for the argument expander.
 */
void xd_arg_expander_destroy();

/**
 * @brief Performs shell expansions on the passed argument and returns a list
 * containing the result of the expansions.
 *
 * Expansions are performed in the following order:
 *
 * 1. Tilde expansion
 *
 * 2. Variable and Parameter expansion
 *
 * 3. Command substitution
 *
 * 4. Word Splitting
 *
 * 5. Filename expansion (globbing)
 *
 * 6. Quote removal and escape character handling
 *
 * @param arg Pointer to the null-terminated argument string to be expanded.
 *
 * @return Pointer to a newly allocated `xd_list_t` structure containing the
 * result of the expansions or `NULL` on failure.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_list_destroy()` and passing it the returned pointer.
 */
xd_list_t *xd_arg_expander(char *arg);

#endif  // XD_ARG_EXPANDER_H
