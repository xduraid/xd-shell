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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========================
// Macros
// ========================

/**
 * @brief Base used in `strtol()`.
 */
#define XD_UTILS_STRTOL_BASE (10)

/**
 * @brief Initial value for djb2 string hash.
 */
#define XD_UTILS_DJB2_INITIAL (5381)

/**
 * @brief Multiplier used in djb2 string hash.
 */
#define XD_UTILS_DJB2_MULTIPLIER (5)

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

void *xd_utils_str_copy_func(void *data) {
  if (data == NULL) {
    return NULL;
  }
  char *copy = strdup(data);
  if (copy == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  return copy;
}  // xd_utils_str_copy_func()

void xd_utils_str_destroy_func(void *data) {
  free(data);
}  // xd_utils_str_destroy_func()

int xd_utils_str_comp_func(const void *data1, const void *data2) {
  if (data1 == NULL && data2 == NULL) {
    return 0;
  }
  if (data1 == NULL) {
    return -1;
  }
  if (data2 == NULL) {
    return 1;
  }
  return strcmp(data1, data2);
}  // xd_utils_str_comp_func()

unsigned int xd_utils_str_hash_func(void *data) {
  if (data == NULL) {
    return 0;
  }
  char *str = data;
  unsigned int hash = XD_UTILS_DJB2_INITIAL;
  int chr;
  while ((chr = (unsigned char)*str++)) {
    hash = ((hash << XD_UTILS_DJB2_MULTIPLIER) + hash) + chr;
  }
  return hash;
}  // xd_utils_str_hash_func()
