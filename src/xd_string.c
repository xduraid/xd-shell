/*
 * ==============================================================================
 * File: xd_string.c
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

#include "xd_string.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========================
// Public Functions
// ========================

xd_string_t *xd_string_create() {
  xd_string_t *string = (xd_string_t *)malloc(sizeof(xd_string_t));
  char *str = (char *)malloc(sizeof(char) * XD_STR_DEF_CAP);
  if (string == NULL || str == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  string->length = 0;
  string->capacity = XD_STR_DEF_CAP;
  string->str = str;
  string->str[0] = '\0';
  return string;
}  // xd_string_create()

void xd_string_destroy(xd_string_t *string) {
  if (string == NULL) {
    return;
  }
  free(string->str);
  free(string);
}  // xd_string_destroy()

void xd_string_clear(xd_string_t *string) {
  if (string == NULL) {
    return;
  }
  string->str[0] = '\0';
  string->length = 0;
}

void xd_string_append_str(xd_string_t *string, const char *str) {
  if (string == NULL || str == NULL) {
    return;
  }

  int str_len = (int)strlen(str);

  // resize if needed
  if (string->length + str_len > string->capacity - 1) {
    // resize to multiple of `XD_STR_DEF_CAP`
    int new_capacity = string->length + str_len + 1;
    if (new_capacity % XD_STR_DEF_CAP != 0) {
      new_capacity += XD_STR_DEF_CAP - (new_capacity % XD_STR_DEF_CAP);
    }

    char *ptr = (char *)realloc(string->str, sizeof(char) * new_capacity);
    if (ptr == NULL) {
      fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
              strerror(errno));
      exit(EXIT_FAILURE);
    }

    string->str = ptr;
    string->capacity = new_capacity;
  }
  memcpy(string->str + string->length, str, str_len);
  string->length += str_len;
  string->str[string->length] = '\0';
}  // xd_string_append_str()

void xd_string_append_chr(xd_string_t *string, char chr) {
  if (string == NULL) {
    return;
  }

  // resize if needed
  if (string->length + 1 > string->capacity - 1) {
    // resize to multiple of `XD_STR_DEF_CAP`
    int new_capacity = string->length + 2;
    if (new_capacity % XD_STR_DEF_CAP != 0) {
      new_capacity += XD_STR_DEF_CAP - (new_capacity % XD_STR_DEF_CAP);
    }

    char *ptr = (char *)realloc(string->str, sizeof(char) * new_capacity);
    if (ptr == NULL) {
      fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
              strerror(errno));
      exit(EXIT_FAILURE);
    }

    string->str = ptr;
    string->capacity = new_capacity;
  }
  string->str[string->length++] = chr;
  string->str[string->length] = '\0';
}  // xd_string_append_chr()
