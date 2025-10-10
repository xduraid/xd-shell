/*
 * ==============================================================================
 * File: xd_comp_generator.c
 * Author: Duraid Maihoub
 * Date: 9 Oct 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#include "xd_comp_generator.h"

#include <errno.h>
#include <glob.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "xd_list.h"
#include "xd_utils.h"
#include "xd_vars.h"

// ========================
// Macros
// ========================

// ========================
// Function Declarations
// ========================

static int xd_str_cmp(const void *first, const void *second);

static xd_list_t *xd_username_completions_generator(const char *partial_text);
static xd_list_t *xd_home_path_completions_generator(const char *partial_text);
static xd_list_t *xd_tilde_completions_generator(const char *partial_text);
static xd_list_t *xd_var_completions_generator(const char *partial_text);
static xd_list_t *xd_param_completions_generator(const char *partial_text);

// ========================
// Variables
// ========================

// ========================
// Function Definitions
// ========================

/**
 * @brief Comparison function for sorting strings.
 *
 * @param first Pointer to the first element being compared.
 * @param second Pointer to the second element being compared.
 *
 * @return A negative value if first should come before second, a positive
 * value if second should come before first, zero if both are equal.
 */
static int xd_str_cmp(const void *first, const void *second) {
  const char *str1 = *(const char **)first;
  const char *str2 = *(const char **)second;
  return strcmp(str1, str2);
}  // xd_str_cmp()

/**
 * @brief Generates a list of all possible tilde username completions for the
 * passed text.
 *
 * @param partial_text The partial text to be completed.
 *
 * @return Pointer to a newly allocated `xd_list_t` containing all possible
 * completions or `NULL` on failure.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_list_destroy()` and passing it the returned pointer.
 */
static xd_list_t *xd_username_completions_generator(const char *partial_text) {
  partial_text = partial_text + 1;  // skip `~`

  int partial_text_len = (int)strlen(partial_text);

  xd_list_t *comp_list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);

  struct passwd *pwd_entry = NULL;
  struct stat file_stat;
  char temp[LINE_MAX];
  setpwent();
  while ((pwd_entry = getpwent()) != NULL) {
    if (strncmp(pwd_entry->pw_name, partial_text, partial_text_len) != 0) {
      // skip non-matches
      continue;
    }

    int is_dir = 0;
    if (stat(pwd_entry->pw_dir, &file_stat) == 0 &&
        S_ISDIR(file_stat.st_mode)) {
      // add `/` if the user's home directory is defined and exist
      is_dir = 1;
    }

    snprintf(temp, LINE_MAX, "~%s%s", pwd_entry->pw_name, (is_dir ? "/" : ""));
    xd_list_add_last(comp_list, temp);
  }
  endpwent();

  return comp_list;
}  // xd_username_completions_generator()

/**
 * @brief Generates a list of all possible tilde home path completions for the
 * passed text.
 *
 * @param partial_text The partial text to be completed.
 *
 * @return Pointer to a newly allocated `xd_list_t` containing all possible
 * completions or `NULL` on failure.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_list_destroy()` and passing it the returned pointer.
 */
static xd_list_t *xd_home_path_completions_generator(const char *partial_text) {
  partial_text = partial_text + 1;  // skip `~`

  char *first_slash = strchr(partial_text, '/');

  int prefix_len = (int)(first_slash - partial_text);
  if (prefix_len > LOGIN_NAME_MAX - 1) {
    return NULL;
  }

  char prefix[LOGIN_NAME_MAX];
  if (prefix_len > 0) {
    memcpy(prefix, partial_text, prefix_len);
  }
  prefix[prefix_len] = '\0';

  char *home_path = NULL;
  if (prefix_len == 0) {
    // prefix (username) is not specified
    home_path = xd_vars_get("HOME");
    if (home_path == NULL) {
      uid_t uid = getuid();
      struct passwd *pwd = getpwuid(uid);
      if (pwd != NULL) {
        home_path = pwd->pw_dir;
      }
    }
  }
  else {
    // prefix (username) specified
    struct passwd *pwd = getpwnam(prefix);
    if (pwd != NULL) {
      home_path = pwd->pw_dir;
    }
  }

  if (home_path == NULL) {
    return NULL;
  }
  int home_path_len = (int)strlen(home_path);

  // initialize glob pattern
  char path[PATH_MAX] = {0};
  snprintf(path, PATH_MAX, "%s%s*", home_path, first_slash);

  // find glob matches
  glob_t glob_result;
  int glob_flags = GLOB_MARK | GLOB_NOSORT;
  if (glob(path, glob_flags, NULL, &glob_result) != 0) {
    return NULL;
  }

  int comp_count = (int)glob_result.gl_pathc;

  xd_list_t *comp_list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);
  for (int i = 0; i < comp_count; i++) {
    snprintf(path, PATH_MAX, "~%s%s", prefix,
             glob_result.gl_pathv[i] + home_path_len);
    xd_list_add_last(comp_list, path);
  }
  globfree(&glob_result);

  return comp_list;
}  // xd_home_path_completions_generator()

/**
 * @brief Generates a list of all possible tilde completions for the passed
 * text.
 *
 * @param partial_text The partial text to be completed.
 *
 * @return Pointer to a newly allocated `xd_list_t` containing all possible
 * completions or `NULL` on failure.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_list_destroy()` and passing it the returned pointer.
 */
static xd_list_t *xd_tilde_completions_generator(const char *partial_text) {
  if (strchr(partial_text, '/') == NULL) {
    // username completion
    return xd_username_completions_generator(partial_text);
  }
  // home path completion
  return xd_home_path_completions_generator(partial_text);
}  // xd_tilde_completions_generator()

/**
 * @brief Generates a list of all possible variable (`$var`) completions for the
 * passed text.
 *
 * @param partial_text The partial text to be completed.
 *
 * @return Pointer to a newly allocated `xd_list_t` containing all possible
 * completions or `NULL` on failure.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_list_destroy()` and passing it the returned pointer.
 */
static xd_list_t *xd_var_completions_generator(const char *partial_text) {
  partial_text = partial_text + 1;  // skip `$`
  int partial_text_len = (int)strlen(partial_text);

  xd_list_t *var_names = xd_vars_names_list();
  if (var_names == NULL) {
    return NULL;
  }

  xd_list_t *comp_list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);
  char temp[LINE_MAX];
  for (xd_list_node_t *node = var_names->head; node != NULL;
       node = node->next) {
    if (strncmp(node->data, partial_text, partial_text_len) == 0) {
      snprintf(temp, LINE_MAX, "$%s", (char *)node->data);
      xd_list_add_last(comp_list, temp);
    }
  }

  xd_list_destroy(var_names);
  return comp_list;
}  // xd_var_completions_generator()

/**
 * @brief Generates a list of all possible param (`${var}`) completions for the
 * passed text.
 *
 * @param partial_text The partial text to be completed.
 *
 * @return Pointer to a newly allocated `xd_list_t` containing all possible
 * completions or `NULL` on failure.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_list_destroy()` and passing it the returned pointer.
 */
static xd_list_t *xd_param_completions_generator(const char *partial_text) {
  partial_text = partial_text + 1;  // skip `{`
  int partial_text_len = (int)strlen(partial_text);

  xd_list_t *var_names = xd_vars_names_list();
  if (var_names == NULL) {
    return NULL;
  }

  xd_list_t *comp_list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);
  char temp[LINE_MAX];
  for (xd_list_node_t *node = var_names->head; node != NULL;
       node = node->next) {
    if (strncmp(node->data, partial_text, partial_text_len) == 0) {
      snprintf(temp, LINE_MAX, "{%s}", (char *)node->data);
      xd_list_add_last(comp_list, temp);
    }
  }

  xd_list_destroy(var_names);
  return comp_list;
}

// ========================
// Public Functions
// ========================

char **xd_completions_generator(const char *line, int start, int end) {
  if (start == end) {
    return NULL;
  }

  xd_list_t *comp_list = NULL;
  char chr = line[start];
  char prev_chr = ' ';
  if (start > 0) {
    prev_chr = line[start - 1];
  }
  int partial_text_len = end - start;

  char *partial_text = strndup(line + start, partial_text_len);
  if (partial_text == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (chr == '~') {
    comp_list = xd_tilde_completions_generator(partial_text);
  }
  else if (chr == '$') {
    comp_list = xd_var_completions_generator(partial_text);
  }
  else if (chr == '{' && prev_chr == '$') {
    comp_list = xd_param_completions_generator(partial_text);
  }

  free(partial_text);
  if (comp_list == NULL) {
    return NULL;
  }

  // build null-terminated array of completions
  char **comp_arr = (char **)malloc(sizeof(char *) * (comp_list->length + 1));
  xd_list_node_t *node = comp_list->head;
  for (int i = 0; i < comp_list->length; i++) {
    comp_arr[i] = xd_utils_strdup(node->data);
    node = node->next;
  }
  comp_arr[comp_list->length] = NULL;

  // sort the completions
  qsort((void *)comp_arr, comp_list->length, sizeof(char *), xd_str_cmp);
  xd_list_destroy(comp_list);

  return comp_arr;
}  // xd_completions_generator()
