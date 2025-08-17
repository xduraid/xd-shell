/*
 * ==============================================================================
 * File: xd_utils.h
 * Author: Duraid Maihoub
 * Date: 10 Aug 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#ifndef XD_UTILS_H
#define XD_UTILS_H

/**
 * @brief Parse a string into a `long` using `strtol` with strict validation.
 *
 *
 * @param str NUL-terminated string to parse. Must not be `NULL` or empty.
 * @param out Output parameter, parsed value will be stored here on success.
 * Must not be `NULL`.
 *
 * @return `0` on success, `-1` on failure.
 */
int xd_utils_strtol(const char *str, long *out);

/**
 * @brief Creates a newly-allocated copy of the passed string.
 *
 * @param data Pointer to the null-terminated string to be copied
 *
 * @return A pointer to the newly allocated copy of the string, or `NULL` if the
 * passed pointer is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The returned string must be freed with `xd_util_str_destroy_func()`.
 */
void *xd_utils_str_copy_func(void *data);

/**
 * @brief Frees the memory allocated for the passe string.
 *
 * @param data Pointer to the string to be freed.
 *
 * @note If the passed pointer is `NULL` no action shall occur.
 */
void xd_utils_str_destroy_func(void *data);

/**
 * @brief Compares two strings for lexical ordering.
 *
 * @param data1 Pointer to the first null-terminated string.
 * @param data2 Pointer to the second null-terminated string.
 *
 * @return
 * - 0 if both strings are equal or both are `NULL`.
 *
 * - A negative value if `data1` is `NULL` or is lexicographically less than
 * `data2`.
 *
 * - A positive value if `data2` is `NULL` or is lexicographically greater than
 * `data2`.
 */
int xd_utils_str_comp_func(const void *data1, const void *data2);

/**
 * @brief Calculates the hash value for the passed string useing the `djb2`
 * algorithm.
 *
 * @param data Pointer to the null-terminated string to be hashed.
 *
 * @return An unsigned integer representing the hash value of the string,
 * or 0 if the input is `NULL`.
 */
unsigned int xd_utils_str_hash_func(void *data);

#endif  // XD_UTILS_H
