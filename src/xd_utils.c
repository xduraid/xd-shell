/*
 * ==============================================================================
 * File: xd_utils.c
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

#include "xd_utils.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

// ========================
// Macros
// ========================

/**
 * @brief Base used in `strtol()`.
 */
#define XD_UTILS_STRTOL_BASE (10)

// ========================
// Function Declarations
// ========================

// ========================
// Variables
// ========================

// ========================
// Function Definitions
// ========================

// ========================
// Public Functions
// ========================

/**
 * @brief Parse a string into a `long` using `strtol` with strict
 * validation.
 *
 * @param str NULL-terminated string to parse. Must not be `NULL` or empty.
 * @param out Output param, prased value will be stored in it. Must not be
 * `NULL`.
 *
 * @return `0` on success, `-1` on failure.
 */
int xd_utils_strtol(const char *str, long *out) {
  if (str == NULL || *str == '\0' || out == NULL) {
    return -1;
  }

  int saved_errno = errno;
  errno = 0;

  char *endptr = NULL;
  long tmp = strtol(str, &endptr, XD_UTILS_STRTOL_BASE);

  int ret = 0;
  if (errno != 0 || endptr == str || *endptr != '\0') {
    ret = -1;
  }
  else {
    *out = tmp;
  }

  errno = saved_errno;
  return ret;
}  // xd_utils_strtol()
