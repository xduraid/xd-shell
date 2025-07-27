/*
 * ==============================================================================
 * File: xd_generic_funcs.h
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

#ifndef XD_GENERIC_FUNCS
#define XD_GENERIC_FUNCS

/**
 * @brief Function pointer type for copying a generic data element.
 *
 * @param data pointer to the data to be copied.
 *
 * @return A pointer to the newly allocated copy of the data, or `NULL` if the
 * copy operation fails.
 *
 * @note The implementation of this function must handle the case where `data`
 * is `NULL`. It should either return `NULL` or allocate a default/empty
 * representation, depending on the intended behavior.
 */
typedef void *(*xd_gens_copy_func_t)(void *);

/**
 * @brief Function pointer type for destroying (freeing) a generic data element.
 *
 * @param data A pointer to the data to be destroyed.
 *
 * @note The implementation must handle the case where `data` is `NULL`. It
 * should treat `NULL` as a no-op and return safely.
 */
typedef void (*xd_gens_destroy_func_t)(void *);

/**
 * @brief Function pointer type for comparing two generic data elements.
 *
 * @param a A pointer to the first element to compare.
 * @param b A pointer to the second element to compare.
 *
 * @return returns:
 *
 * - `0` if `a` and `b` are equal,
 *
 * - A negative value if `a` < `b`,
 *
 * - A positive value if `a` > `b`.
 *
 * @note The implementation must handle cases where either or both arguments are
 * `NULL`. The expected behavior (e.g., treating `NULL` as less than any
 * value) must be well-defined and consistent with how the structure
 * uses the comparator.
 */
typedef int (*xd_gens_comp_func_t)(const void *, const void *);

#endif  // XD_GENERIC_FUNCS
