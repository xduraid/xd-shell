/*
 * ==============================================================================
 * File: xd_string.h
 * Author: Duraid Maihoub
 * Date: 6 September 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#ifndef XD_STRING_H
#define XD_STRING_H

// ========================
// Macros
// ========================

/**
 * Default initial capacity for `xd_string_t` buffers. The capacity grows in
 * multiples of this value when appending exceeds the current capacity.
 */
#define XD_STR_DEF_CAP (32)

// ========================
// Typedefs
// ========================

/**
 * @brief Represents a dynamically growable string buffer.
 */
typedef struct xd_string_t {
  char *str;     // Pointer to the internal null-terminated buffer
  int length;    // Number of characters currently stored excluding '\0'
  int capacity;  // Allocated buffer size in bytes including '\0'
} xd_string_t;

// ========================
// Function Declarations
// ========================

/**
 * @brief Creates and initializes a new `xd_string_t` structure.
 *
 * @return A pointer to the newly created `xd_string_t`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
xd_string_t *xd_string_create();

/**
 * @brief Frees the memory allocated for the passed `xd_string_t`.
 *
 * @param string A pointer to the `xd_string_t` to be freed.
 *
 * @note If the passed pointer is `NULL` no action shall occur.
 */
void xd_string_destroy(xd_string_t *string);

/**
 * @brief Removes all characters from the passed string, leaving it empty.
 *
 * @param string A pointer to the `xd_string_t` to be cleared.
 *
 * @note If the passed pointer is `NULL` no action shall occur.
 */
void xd_string_clear(xd_string_t *string);

/**
 * @brief Appends the passed null-terminated string to the end of the passed
 * `xd_string_t`.
 *
 * @param string A pointer to the target `xd_string_t` to append to.
 * @param str A pointer to the null-terminated string to append.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note If `string` or `str` is `NULL` no action shall occur.
 */
void xd_string_append_str(xd_string_t *string, const char *str);

/**
 * @brief Appends a single character to the end of the passed `xd_string_t`.
 *
 * @param string A pointer to the target `xd_string_t` to append to.
 * @param chr The character to append.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note If `string` is `NULL` no action shall occur.
 */
void xd_string_append_chr(xd_string_t *string, char chr);

#endif  // XD_STRING_H
