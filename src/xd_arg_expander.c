/*
 * ==============================================================================
 * File: xd_arg_expander.c
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

#include "xd_arg_expander.h"

#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xd_list.h"
#include "xd_string.h"
#include "xd_utils.h"
#include "xd_vars.h"

// ========================
// Macros
// ========================

// ========================
// Typedefs
// ========================

// ========================
// Function Declarations
// ========================

static char *xd_tidle_expansion(char *arg, char **orig_mask);

// ========================
// Variables
// ========================

/**
 * @brief Pointer to the original (current) arg to be expanded before copying
 * it.
 *
 * To be freed in command substitution's child process (just to get zero memory
 * errors when running with valgrind).
 */
static char *xd_original_arg = NULL;

// ========================
// Function Definitions
// ========================

/**
 * @brief Performs tilde expansion on the passed argument string.
 *
 * @param arg Pointer to the null-terminated argument string to be expanded.
 * @param orig_mask Pointer to a pointer to a null-terminated string containing
 * the argument's originality mask, which indicates which characters are from
 * the original argument and which are a result of expansion. This mask will be
 * updated when this function is called.
 *
 * @return Pointer to a newly allocated string containing the result of the
 * expansion or `NULL` if the passed pointer is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `free()` and passing it the returned pointer.
 */
static char *xd_tidle_expansion(char *arg, char **orig_mask) {
  if (arg == NULL) {
    return NULL;
  }

  if (*arg != '~') {
    return xd_utils_strdup(arg);
  }

  char *prefix = arg + 1;
  int prefix_len;
  char *suffix;

  char *slash = strchr(prefix, '/');
  if (slash == NULL) {
    prefix_len = (int)strlen(prefix);
    suffix = prefix + prefix_len;
  }
  else {
    prefix_len = (int)(slash - prefix);
    suffix = slash;
  }
  int suffix_len = (int)strlen(suffix);

  const char *expanded_prefix = NULL;
  if (prefix_len == 0) {
    // "~" or "~/..."
    expanded_prefix = xd_vars_get("HOME");
    if (expanded_prefix == NULL) {
      struct passwd *pwd = getpwuid(getuid());
      if (pwd != NULL) {
        expanded_prefix = pwd->pw_dir;
      }
    }
  }
  else if (prefix_len == 1 && prefix[0] == '+') {
    // "~+" or "~+/..."
    expanded_prefix = xd_vars_get("PWD");
  }
  else if (prefix_len == 1 && prefix[0] == '-') {
    // "~-" or "~-/..."
    expanded_prefix = xd_vars_get("OLDPWD");
  }
  else {
    // "~user" or "~user/..."
    char saved_char = prefix[prefix_len];
    prefix[prefix_len] = '\0';
    struct passwd *pwd = getpwnam(prefix);
    prefix[prefix_len] = saved_char;
    if (pwd != NULL) {
      expanded_prefix = pwd->pw_dir;
    }
  }

  if (expanded_prefix == NULL) {
    return xd_utils_strdup(arg);
  }

  int expanded_prefix_len = (int)strlen(expanded_prefix);

  xd_string_t *str = xd_string_create();
  xd_string_append_str(str, expanded_prefix);
  xd_string_append_str(str, suffix);
  char *expanded_arg = xd_utils_strdup(str->str);

  xd_string_clear(str);
  for (int i = 0; i < expanded_prefix_len; i++) {
    xd_string_append_chr(str, '0');
  }
  for (int i = 0; i < suffix_len; i++) {
    xd_string_append_chr(str, (*orig_mask)[i]);
  }

  free(*orig_mask);
  *orig_mask = xd_utils_strdup(str->str);
  xd_string_destroy(str);

  return expanded_arg;
}  // xd_tidle_expansion()

// ========================
// Public Functions
// ========================

xd_list_t *xd_arg_expander(char *arg) {
  xd_original_arg = arg;
  arg = xd_utils_strdup(arg);

  // initialize orignality mask
  char *orig_mask = xd_utils_strdup(arg);
  for (int i = 0; orig_mask[i] != '\0'; i++) {
    orig_mask[i] = '1';
  }

  // 1. Tilde expansion
  char *expanded_arg = xd_tidle_expansion(arg, &orig_mask);
  free(arg);

  xd_list_t *list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);
  xd_list_add_last(list, expanded_arg);

  free(expanded_arg);
  free(orig_mask);

  return list;
}  // xd_arg_expander()
