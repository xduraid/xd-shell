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

#endif  // XD_UTILS_H
