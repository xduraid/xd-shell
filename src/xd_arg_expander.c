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

#include <ctype.h>
#include <errno.h>
#include <glob.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#include "xd_list.h"
#include "xd_shell.h"
#include "xd_string.h"
#include "xd_utils.h"
#include "xd_vars.h"

// ========================
// Macros
// ========================

/**
 * @brief Default initial capacity for `xd_state_stack`.
 */
#define XD_SS_DEF_CAP (32)

/**
 * @brief Maximum length of a special parameter value string (`$`, `?`, `!`, ...
 * etc).
 */
#define XD_SPEC_PAR_MAX (32)

/**
 * @brief The characters at which word splitting will occur.
 */
#define XD_IFS " \t\n"

// ========================
// Typedefs
// ========================

/**
 * @brief Represents the scanning state.
 */
typedef enum xd_scan_state_t {
  XD_SS_NA,   // No State - Empty Scanning State Stack
  XD_SS_UQ,   // Unquoted state
  XD_SS_SQ,   // Single quoted state
  XD_SS_DQ,   // Double quoted state
  XD_SS_PRM,  // Parameter state
  XD_SS_CMD,  // Command substitution state
  XD_SS_ESC,  // Escape `\` state
} xd_scan_state_t;

// ========================
// Function Declarations
// ========================

// flex & bison funcs
extern void yylex_scan_string(char *str);
extern void yyparse_initialize();
extern void yyparse_cleanup();
extern int yyparse();

static int xd_glob_sort_func(const void *first, const void *second);

static void xd_ss_stack_push(xd_scan_state_t state);
static void xd_ss_stack_pop();
static xd_scan_state_t xd_ss_stack_top();
static void xd_ss_stack_clear();
static int xd_ss_stack_update(const char *arg, const char *orig_mask, int idx);

static int xd_special_param_value(const char *prm_id, char *out);
static void xd_exec_capture_output(char *arg, const char *orig_mask,
                                   char *cmd_str, xd_string_t *exp_arg_str,
                                   xd_string_t *orig_mask_str);

static char *xd_tidle_expansion(char *arg, char **orig_mask);
static char *xd_param_expansion(char *arg, char **orig_mask);
static char *xd_command_substitution(char *arg, char **orig_mask);
static xd_list_t *xd_word_splitting(char *arg, char *orig_mask,
                                    xd_list_t **orig_mask_list);
static int xd_filename_expansion(xd_list_t **arg_list,
                                 xd_list_t **orig_mask_list);
static int xd_quote_removal(xd_list_t **arg_list,
                            const xd_list_t *orig_mask_list);

// ========================
// Variables
// ========================

/**
 * @brief Scanning state stack.
 */
static xd_scan_state_t *xd_ss_stack = NULL;

/**
 * @brief Length of `xd_ss_stack`.
 */
static int xd_ss_stack_length = 0;

/**
 * @brief Capacity of `xd_ss_stack`.
 */
static int xd_ss_stack_capacity = 0;

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
 * @brief Helper comparison function for sorting path strings resulting from
 * `glob()`.
 *
 * @param first Pointer to the first element being compared.
 * @param second Pointer to the second element being compared.
 *
 * @return A negative value if first should come before second, a positive
 * value if second should come before first, zero if both are equal.
 */
static int xd_glob_sort_func(const void *first, const void *second) {
  const char *path1 = *(const char **)first;
  const char *path2 = *(const char **)second;
  return strcasecmp(path1, path2);
}  // xd_glob_sort_func()

/**
 * @brief Pushes the passed state to the top of the scan state stack.
 *
 * @param state The state to be pushed to the top of the scan state stack.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
static void xd_ss_stack_push(xd_scan_state_t state) {
  // resize if needed
  if (xd_ss_stack_length > xd_ss_stack_capacity - 1) {
    int new_capacity =
        (xd_ss_stack_capacity == 0 ? XD_SS_DEF_CAP : xd_ss_stack_capacity * 2);
    xd_scan_state_t *ptr = (xd_scan_state_t *)realloc(
        xd_ss_stack, sizeof(xd_scan_state_t) * new_capacity);
    if (ptr == NULL) {
      fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
              strerror(errno));
      exit(EXIT_FAILURE);
    }
    xd_ss_stack = ptr;
    xd_ss_stack_capacity = new_capacity;
  }
  xd_ss_stack[xd_ss_stack_length++] = state;
}  // xd_ss_stack_push()

/**
 * @brief Removes the element at the top of the scan state stack.
 *
 * @note If the scan state stack is empty, nothing shall occur.
 */
static void xd_ss_stack_pop() {
  if (xd_ss_stack_length > 0) {
    xd_ss_stack_length--;
  }
}  // xd_ss_stack_pop()

/**
 * @brief Returns the element at the top of the scan state stack or `XD_SS_NA`
 * if the stack is empty.
 */
static xd_scan_state_t xd_ss_stack_top() {
  if (xd_ss_stack_length == 0) {
    return XD_SS_NA;
  }
  return xd_ss_stack[xd_ss_stack_length - 1];
}  // xd_ss_stack_top()

/**
 * @brief Clears the scan state stack.
 */
static void xd_ss_stack_clear() {
  xd_ss_stack_length = 0;
}  // xd_ss_stack_clear()

/**
 * @brief Helper used while scanning arguments to update the current scanning
 * state by pushing the new state to the scanning state stack.
 *
 * @param arg A pointer to the argument string being scanned.
 * @param orig_mask Pointer to a pointer to a null-terminated string containing
 * the argument's originality mask, which indicates which characters are from
 * the original argument and which are a result of expansion. This mask will be
 * updated when this function is called.
 * @param idx Current position within the argument being scanned.
 *
 * @return `1` if the state was changed, `0` if not.
 */
static int xd_ss_stack_update(const char *arg, const char *orig_mask, int idx) {
  xd_scan_state_t state = xd_ss_stack_top();
  char chr = arg[idx];
  int is_orig = (orig_mask[idx] == '1');

  if (state == XD_SS_ESC) {
    xd_ss_stack_pop();
    return 1;
  }

  if (!is_orig) {
    return 0;
  }

  if (chr == '\\' && state != XD_SS_SQ) {
    xd_ss_stack_push(XD_SS_ESC);
    return 1;
  }

  if (chr == '\'' && state != XD_SS_DQ) {
    if (state == XD_SS_SQ) {
      xd_ss_stack_pop();
    }
    else {
      xd_ss_stack_push(XD_SS_SQ);
    }
    return 1;
  }

  if (chr == '\"' && state != XD_SS_SQ) {
    if (state == XD_SS_DQ) {
      xd_ss_stack_pop();
    }
    else {
      xd_ss_stack_push(XD_SS_DQ);
    }
    return 1;
  }

  if (chr == '$' && state != XD_SS_SQ) {
    char next = arg[idx + 1];
    int is_next_orig = (orig_mask[idx + 1] == '1');
    if (is_next_orig && next == '{') {
      xd_ss_stack_push(XD_SS_PRM);
      return 1;
    }
    if (is_next_orig && next == '(') {
      xd_ss_stack_push(XD_SS_CMD);
      return 1;
    }
    return 0;
  }

  if (chr == '}' && state == XD_SS_PRM) {
    xd_ss_stack_pop();
    return 1;
  }

  if (chr == ')' && state == XD_SS_CMD) {
    xd_ss_stack_pop();
    return 1;
  }

  return 0;
}  // xd_ss_stack_update()

/**
 * @brief Retrieves the value of a special parameter ($, ?, !, etc.) and stores
 * it as a string in the provided output buffer.
 *
 * @param prm_id Pointer to the null-terminated string representing the special
 * parameter identifier (e.g., `"$"`, `"?"`, `"!"`).
 * @param out Pointer to the output buffer where the resulting value will be
 * stored as a null-terminated string. The buffer must be at least
 * `XD_SPEC_PAR_MAX` bytes in size.
 *
 * @return `0` on success, `-1` if not a special parameter.
 */
static int xd_special_param_value(const char *prm_id, char *out) {
  if (prm_id == NULL) {
    return -1;
  }

  if (strcmp(prm_id, "$") == 0) {
    snprintf(out, XD_SPEC_PAR_MAX, "%d", xd_sh_pid);
    return 0;
  }
  if (strcmp(prm_id, "?") == 0) {
    snprintf(out, XD_SPEC_PAR_MAX, "%d", xd_sh_last_exit_code);
    return 0;
  }
  if (strcmp(prm_id, "!") == 0) {
    snprintf(out, XD_SPEC_PAR_MAX, "%d", xd_sh_last_bg_job_pid);
    return 0;
  }
  return -1;
}  // xd_special_param_value()

/**
 * @brief Executes the passed command string in a forked process and captures
 * its standard output and appends it to the passed dynamic string
 * `exp_arg_str` and appends the originality bits which indicate non-original
 * (expanded) characters `'0'` to the passed dynamic string `orig_mask_str`.
 *
 * @param arg A pointer to the argument string being expanded (containing
 * `cmd_str`).
 * @param orig_mask Pointer to a pointer to a null-terminated string containing
 * the argument's originality mask, which indicates which characters are from
 * the original argument and which are a result of expansion. This mask will be
 * updated when this function is called.
 * @param cmd_str A pointer to the null-terminated command string to be
 * executed.
 * @param exp_arg_str A pointer to the `xd_string_t` to which the output is
 * appended.
 * @param exp_arg_str A pointer to the `xd_string_t` to which the originality
 * chars are appended.
 */
static void xd_exec_capture_output(char *arg, const char *orig_mask,
                                   char *cmd_str, xd_string_t *exp_arg_str,
                                   xd_string_t *orig_mask_str) {
  if (cmd_str == NULL || *cmd_str == '\0') {
    return;
  }

  int pipe_fd[2] = {-1, -1};
  if (pipe(pipe_fd) == -1) {
    fprintf(stderr, "xd-shell: pipe: %s\n", strerror(errno));
    return;
  }

  pid_t child_pid = fork();
  if (child_pid == -1) {
    fprintf(stderr, "xd-shell: fork: %s\n", strerror(errno));
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    return;
  }

  if (child_pid == 0) {
    free(xd_original_arg);
    xd_string_destroy(exp_arg_str);  // not needed in child
    xd_string_destroy(orig_mask_str);
    free((void *)orig_mask);

    close(pipe_fd[0]);  // read-end is not needed in child

    // redirect output to the pipe
    if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
      fprintf(stderr, "xd-shell: dup2: %s\n", strerror(errno));
      close(pipe_fd[1]);
      free(arg);
      free(xd_original_arg);
      exit(EXIT_FAILURE);
    }
    close(pipe_fd[1]);

    xd_sh_is_subshell = 1;

    // re-initialize the scanner and parser
    yyparse_cleanup();
    yyparse_initialize();

    // setup scanner input to be the command string
    yylex_scan_string(cmd_str);
    free(arg);

    yyparse();

    exit(EXIT_FAILURE);  // shouldn't reach this
  }

  close(pipe_fd[1]);  // write-end is not needed in parent

  int old_exp_arg_len = exp_arg_str->length;
  char buf[LINE_MAX];
  while (1) {
    ssize_t byte_count = read(pipe_fd[0], buf, LINE_MAX - 1);
    if (byte_count > 0) {
      buf[byte_count] = '\0';
      xd_string_append_str(exp_arg_str, buf);
      continue;
    }
    if (byte_count == 0) {
      break;  // EOF
    }
    if (errno == EINTR) {
      continue;
    }
    break;
  }

  close(pipe_fd[0]);

  // wait for child to terminate and capture its exit code
  int wait_status = 0;
  while (waitpid(child_pid, &wait_status, 0) == -1) {
    if (errno == EINTR) {
      continue;
    }
    break;
  }
  if (WIFEXITED(wait_status)) {
    xd_sh_last_exit_code = WEXITSTATUS(wait_status);
  }
  else if (WIFSIGNALED(wait_status)) {
    xd_sh_last_exit_code =
        XD_SH_EXIT_CODE_SIGNAL_OFFSET + WTERMSIG(wait_status);
  }
  else if (WIFSTOPPED(wait_status)) {
    xd_sh_last_exit_code =
        XD_SH_EXIT_CODE_SIGNAL_OFFSET + WSTOPSIG(wait_status);
  }

  // remove trailing newlines
  while (exp_arg_str->length > 0 &&
         exp_arg_str->str[exp_arg_str->length - 1] == '\n') {
    exp_arg_str->str[--exp_arg_str->length] = '\0';
  }

  // append originality bits
  for (int i = old_exp_arg_len; i < exp_arg_str->length; i++) {
    xd_string_append_chr(orig_mask_str, '0');
  }
}  // xd_exec_capture_output()

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

/**
 * @brief Performs variable/parameter expansion on the passed argument string.
 *
 * @param arg Pointer to the null-terminated argument string to be expanded.
 * @param orig_mask Pointer to a pointer to a null-terminated string containing
 * the argument's originality mask, which indicates which characters are from
 * the original argument and which are a result of expansion. This mask will be
 * updated when this function is called.
 *
 * @return Pointer to a newly allocated string containing the result of the
 * expansion or `NULL` on failure or if the passed pointer is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `free()` and passing it the returned pointer.
 */
static char *xd_param_expansion(char *arg, char **orig_mask) {
  if (arg == NULL) {
    return NULL;
  }

  xd_string_t *exp_arg_str = xd_string_create();
  xd_string_t *orig_mask_str = xd_string_create();

  char param_str[XD_SPEC_PAR_MAX];
  int idx = 0;
  xd_ss_stack_clear();
  xd_ss_stack_push(XD_SS_UQ);
  xd_ss_stack_update(arg, *orig_mask, idx);

  while (arg[idx] != '\0') {
    xd_scan_state_t state = xd_ss_stack_top();

    if (state == XD_SS_ESC) {
      xd_string_append_chr(exp_arg_str, arg[idx]);
      xd_string_append_chr(orig_mask_str, (*orig_mask)[idx]);
      idx++;
      xd_string_append_chr(exp_arg_str, arg[idx]);
      xd_string_append_chr(orig_mask_str, (*orig_mask)[idx]);
      idx++;
    }
    else if (state == XD_SS_PRM) {
      // param/var ${var}

      // reached `{`
      // store stack length to know when we reach matching '}'
      int old_stack_len = xd_ss_stack_length - 1;

      // scan until matching '}'
      int lbrace_idx = idx + 1;
      int rbrace_idx = lbrace_idx;
      while (xd_ss_stack_length != old_stack_len) {
        xd_ss_stack_update(arg, *orig_mask, ++rbrace_idx);
      }

      // temp null-terminate
      char saved_char = arg[rbrace_idx];
      arg[rbrace_idx] = '\0';

      char *var_name = arg + lbrace_idx + 1;
      char *value = NULL;
      if (xd_special_param_value(var_name, param_str) == 0) {
        value = param_str;
      }
      else {
        if (!xd_vars_is_valid_name(var_name)) {
          xd_string_destroy(exp_arg_str);
          xd_string_destroy(orig_mask_str);
          arg[rbrace_idx] = saved_char;
          return NULL;  // error
        }
        value = xd_vars_get(var_name);
      }

      // restore
      arg[rbrace_idx] = saved_char;

      // if var is set expand to its value, if not set expand to empty (skip)
      if (value != NULL) {
        xd_string_append_str(exp_arg_str, value);
        for (int i = 0; value[i] != '\0'; i++) {
          xd_string_append_chr(orig_mask_str, '0');
        }
      }
      idx = rbrace_idx + 1;
    }
    else if (arg[idx] == '$' && (*orig_mask)[idx] == '1' && state != XD_SS_SQ) {
      int start_idx = idx + 1;
      int end_idx = start_idx + 1;
      char next = arg[start_idx];

      if (next == '$' || next == '?' || next == '!') {
        // special parameter $$, $?, $!

        // temp null-terminate
        char saved_char = arg[end_idx];
        arg[end_idx] = '\0';

        char *var_name = arg + start_idx;
        if (xd_special_param_value(var_name, param_str) == 0) {
          xd_string_append_str(exp_arg_str, param_str);
          for (int i = 0; param_str[i] != '\0'; i++) {
            xd_string_append_chr(orig_mask_str, '0');
          }
        }

        // restore
        arg[end_idx] = saved_char;

        idx = end_idx;
      }
      else if (next == '_' || isalpha(next)) {
        // normal var $var

        while (arg[end_idx] == '_' || isalnum(arg[end_idx])) {
          end_idx++;
        }

        // temp null-terminate
        char saved_char = arg[end_idx];
        arg[end_idx] = '\0';

        char *var_name = arg + start_idx;
        char *value = xd_vars_get(var_name);

        // restore
        arg[end_idx] = saved_char;

        // if var is set expand to its value, if not set expand to empty (skip)
        if (value != NULL) {
          xd_string_append_str(exp_arg_str, value);
          for (int i = 0; value[i] != '\0'; i++) {
            xd_string_append_chr(orig_mask_str, '0');
          }
        }
        idx = end_idx;
      }
      else {
        // not a special param neither a valid var name - tread `$` as litteral
        xd_string_append_chr(exp_arg_str, arg[idx]);
        xd_string_append_chr(orig_mask_str, (*orig_mask)[idx]);
        idx++;
      }
    }
    else {
      xd_string_append_chr(exp_arg_str, arg[idx]);
      xd_string_append_chr(orig_mask_str, (*orig_mask)[idx]);
      idx++;
    }

    xd_ss_stack_update(arg, *orig_mask, idx);
  }

  char *expanded_arg = xd_utils_strdup(exp_arg_str->str);
  xd_string_destroy(exp_arg_str);

  free(*orig_mask);
  *orig_mask = xd_utils_strdup(orig_mask_str->str);
  xd_string_destroy(orig_mask_str);

  return expanded_arg;
}  // xd_param_expansion()

/**
 * @brief Performs command substitution on the passed argument string.
 *
 * @param arg Pointer to the null-terminated argument string to be expanded.
 * @param orig_mask Pointer to a pointer to a null-terminated string containing
 * the argument's originality mask, which indicates which characters are from
 * the original argument and which are a result of expansion. This mask will be
 * updated when this function is called.
 *
 * @return Pointer to a newly allocated string containing the result of the
 * expansion or `NULL` on failure or if the passed pointer is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `free()` and passing it the returned pointer.
 */
static char *xd_command_substitution(char *arg, char **orig_mask) {
  if (arg == NULL) {
    return NULL;
  }

  xd_string_t *exp_arg_str = xd_string_create();
  xd_string_t *orig_mask_str = xd_string_create();

  int idx = 0;
  xd_ss_stack_clear();
  xd_ss_stack_push(XD_SS_UQ);
  xd_ss_stack_update(arg, *orig_mask, idx);

  while (arg[idx] != '\0') {
    xd_scan_state_t state = xd_ss_stack_top();

    if (state == XD_SS_CMD) {
      // command substitution $(cmd)

      // reached `(`
      // store stack length to know when we reach matching ')'
      int old_stack_len = xd_ss_stack_length - 1;

      // scan until matching ')'
      int lparen_idx = idx + 1;
      int rparen_idx = lparen_idx;
      while (xd_ss_stack_length != old_stack_len) {
        xd_ss_stack_update(arg, *orig_mask, ++rparen_idx);
      }

      // temp newline and null-terminate
      char saved_char1 = arg[rparen_idx];
      char saved_char2 = arg[rparen_idx + 1];
      arg[rparen_idx] = '\n';
      arg[rparen_idx + 1] = '\0';

      char *cmd_str = arg + lparen_idx + 1;
      xd_exec_capture_output(arg, *orig_mask, cmd_str, exp_arg_str,
                             orig_mask_str);

      // restore
      arg[rparen_idx] = saved_char1;
      arg[rparen_idx + 1] = saved_char2;

      idx = rparen_idx + 1;
    }
    else {
      xd_string_append_chr(exp_arg_str, arg[idx]);
      xd_string_append_chr(orig_mask_str, (*orig_mask)[idx]);
      idx++;
    }

    xd_ss_stack_update(arg, *orig_mask, idx);
  }

  char *expanded_arg = xd_utils_strdup(exp_arg_str->str);
  xd_string_destroy(exp_arg_str);

  free(*orig_mask);
  *orig_mask = xd_utils_strdup(orig_mask_str->str);
  xd_string_destroy(orig_mask_str);

  return expanded_arg;
}  // xd_command_substitution()

/**
 * @brief Performs word splitting on the passed argument string.
 *
 * @param arg Pointer to the null-terminated argument string to be split.
 * @param orig_mask Pointer to a null-terminated string containing
 * the argument's originality mask, which indicates which characters are from
 * the original argument and which are a result of expansion.
 * @param orig_mask_list Output parameter, pointer to a newly allocated
 * `xd_list_t` containing the splitted originality mask.
 *
 * @return Pointer to a newly allocated `xd_list_t` containing the splitted
 * argument or `NULL` on failure or if the passed pointer is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_list_destroy()` on both the returned lists.
 */
static xd_list_t *xd_word_splitting(char *arg, char *orig_mask,
                                    xd_list_t **orig_mask_list) {
  if (arg == NULL) {
    return NULL;
  }

  xd_list_t *arg_list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);
  xd_list_t *mask_list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);

  int start_idx = 0;
  int end_idx = 0;
  xd_ss_stack_clear();
  xd_ss_stack_push(XD_SS_UQ);
  xd_ss_stack_update(arg, orig_mask, end_idx);

  while (arg[end_idx] != '\0') {
    xd_scan_state_t state = xd_ss_stack_top();

    // reached non-quoted IFS character
    if (strchr(XD_IFS, arg[end_idx]) != NULL && state != XD_SS_SQ &&
        state != XD_SS_DQ) {
      // temp null-terminate
      char saved_char1 = arg[end_idx];
      char saved_char2 = orig_mask[end_idx];
      arg[end_idx] = '\0';
      orig_mask[end_idx] = '\0';

      xd_list_add_last(arg_list, arg + start_idx);
      xd_list_add_last(mask_list, orig_mask + start_idx);

      // restore
      arg[end_idx] = saved_char1;
      orig_mask[end_idx] = saved_char2;

      // skip additional IFS chars
      while (arg[end_idx] != '\0' && strchr(XD_IFS, arg[end_idx]) != NULL) {
        end_idx++;
      }
      start_idx = end_idx;
      xd_ss_stack_update(arg, orig_mask, end_idx);
    }
    else {
      end_idx++;
      xd_ss_stack_update(arg, orig_mask, end_idx);
    }
  }

  if (start_idx != end_idx) {
    xd_list_add_last(arg_list, arg + start_idx);
    xd_list_add_last(mask_list, orig_mask + start_idx);
  }

  *orig_mask_list = mask_list;
  return arg_list;
}  // xd_word_splitting()

/**
 * @brief Performs filename expansion (globbing) on the passed arguments.
 *
 * @param arg_list Pointer to a pointer to the `xd_list_t` structure containing
 * the arguments resulting from word splitting, it will be updated when this
 * function is called to store the result of expansions.
 * @param orig_mask_list Pointer to a pointer to the `xd_list_t` structure
 * containing the argument masks resulting from word splitting, it will be
 * updated when this function is called to store masks after after filename
 * expansion.
 *
 * @return `0` on success or `-1` on failure or if the passed pointer is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
static int xd_filename_expansion(xd_list_t **arg_list,
                                 xd_list_t **orig_mask_list) {
  if (*arg_list == NULL) {
    return -1;
  }

  xd_list_t *new_arg_list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);
  xd_list_t *new_mask_list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);

  xd_string_t *mask_str = xd_string_create();
  glob_t glob_result;

  xd_list_node_t *arg_node = (*arg_list)->head;
  xd_list_node_t *mask_node = (*orig_mask_list)->head;
  for (int i = 0; i < (*arg_list)->length; i++) {
    int glob_ret =
        glob(arg_node->data, GLOB_BRACE | GLOB_NOSORT, NULL, &glob_result);

    if (glob_ret == 0) {
      // sort then add matches
      qsort((void *)glob_result.gl_pathv, glob_result.gl_pathc, sizeof(char *),
            xd_glob_sort_func);
      for (size_t j = 0; j < glob_result.gl_pathc; j++) {
        xd_string_clear(mask_str);

        char *path = glob_result.gl_pathv[j];
        for (int k = 0; path[k] != '\0'; k++) {
          xd_string_append_chr(mask_str, '0');
        }
        xd_list_add_last(new_arg_list, path);
        xd_list_add_last(new_mask_list, mask_str->str);
      }
    }
    else if (glob_ret == GLOB_NOMATCH) {
      // no match leave as is
      xd_list_add_last(new_arg_list, arg_node->data);
      xd_list_add_last(new_mask_list, mask_node->data);
    }
    else {
      // error
      xd_list_destroy(new_arg_list);
      xd_list_destroy(new_mask_list);
      xd_string_destroy(mask_str);
      return -1;
    }

    globfree(&glob_result);
    arg_node = arg_node->next;
    mask_node = mask_node->next;
  }

  xd_string_destroy(mask_str);
  xd_list_destroy(*arg_list);
  xd_list_destroy(*orig_mask_list);

  *arg_list = new_arg_list;
  *orig_mask_list = new_mask_list;

  return 0;
}  // xd_filename_expansion()

/**
 * @brief Performs quote removal and escape character handling on the passed
 * arguments.
 *
 * @param arg_list Pointer to a pointer to the `xd_list_t` structure containing
 * the arguments resulting from word splitting, it will be updated when this
 * function is called to store the result.
 * @param orig_mask_list Pointer to a pointer to the `xd_list_t` structure
 * containing the argument masks resulting from word splitting.
 *
 * @return `0` on success or `-1` on failure or if the passed pointer is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
static int xd_quote_removal(xd_list_t **arg_list,
                            const xd_list_t *orig_mask_list) {
  if (arg_list == NULL || orig_mask_list == NULL) {
    return -1;
  }

  xd_list_t *new_arg_list =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);

  xd_list_node_t *arg_node = (*arg_list)->head;
  xd_list_node_t *mask_node = orig_mask_list->head;
  xd_string_t *exp_arg_str = xd_string_create();

  for (int i = 0; i < (*arg_list)->length; i++) {
    char *arg = arg_node->data;
    char *orig_mask = mask_node->data;

    xd_string_clear(exp_arg_str);

    int idx = 0;
    xd_ss_stack_clear();
    xd_ss_stack_push(XD_SS_UQ);
    xd_ss_stack_update(arg, orig_mask, idx);

    xd_scan_state_t state = XD_SS_UQ;
    xd_scan_state_t prev_state;

    while (arg[idx] != '\0') {
      prev_state = state;
      state = xd_ss_stack_top();

      if (state == XD_SS_ESC) {
        xd_ss_stack_update(arg, orig_mask, idx++);
        state = xd_ss_stack_top();
        if (state == XD_SS_DQ && strchr("$\"\\\n", arg[idx]) == NULL) {
          xd_string_append_chr(exp_arg_str, '\\');
        }
        xd_string_append_chr(exp_arg_str, arg[idx++]);
      }
      else if (((state == XD_SS_SQ || prev_state == XD_SS_SQ) &&
                arg[idx] == '\'') ||
               ((state == XD_SS_DQ || prev_state == XD_SS_DQ) &&
                arg[idx] == '"')) {
        idx++;
      }
      else {
        xd_string_append_chr(exp_arg_str, arg[idx++]);
      }

      xd_ss_stack_update(arg, orig_mask, idx);
    }

    xd_list_add_last(new_arg_list, exp_arg_str->str);
    arg_node = arg_node->next;
    mask_node = mask_node->next;
  }

  xd_string_destroy(exp_arg_str);
  xd_list_destroy(*arg_list);
  *arg_list = new_arg_list;
  return 0;
}  // xd_quote_removal()

// ========================
// Public Functions
// ========================

void xd_arg_expander_init() {
  xd_ss_stack =
      (xd_scan_state_t *)malloc(sizeof(xd_scan_state_t) * XD_SS_DEF_CAP);
  if (xd_ss_stack == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  xd_ss_stack_length = 0;
  xd_ss_stack_capacity = XD_SS_DEF_CAP;
}  // xd_arg_expander_init()

void xd_arg_expander_destroy() {
  free(xd_ss_stack);
}  // xd_arg_expander_destroy()

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
  arg = expanded_arg;

  // 2. Parameter expansion
  expanded_arg = xd_param_expansion(arg, &orig_mask);
  free(arg);
  if (expanded_arg == NULL) {
    fprintf(stderr, "xd-shell: %s: bad substitution\n", xd_original_arg);
    free(orig_mask);
    return NULL;
  }
  arg = expanded_arg;

  // 3. Command substitution
  expanded_arg = xd_command_substitution(arg, &orig_mask);
  free(arg);
  if (expanded_arg == NULL) {
    fprintf(stderr, "xd-shell: %s: cmd substitution error\n", xd_original_arg);
    free(orig_mask);
    return NULL;
  }
  arg = expanded_arg;

  // 4. Word splitting
  xd_list_t *orig_mask_list = NULL;
  xd_list_t *exp_arg_list = xd_word_splitting(arg, orig_mask, &orig_mask_list);
  free(arg);
  free(orig_mask);
  if (exp_arg_list == NULL) {
    fprintf(stderr, "xd-shell: %s: word splitting error\n", xd_original_arg);
    return NULL;
  }

  // 5. Filename expansion
  if (xd_filename_expansion(&exp_arg_list, &orig_mask_list) == -1) {
    xd_list_destroy(exp_arg_list);
    xd_list_destroy(orig_mask_list);
    fprintf(stderr, "xd-shell: %s: filename expansion error\n",
            xd_original_arg);
    return NULL;
  }

  // 6. Quote removal and escape character handling
  if (xd_quote_removal(&exp_arg_list, orig_mask_list) == -1) {
    xd_list_destroy(exp_arg_list);
    xd_list_destroy(orig_mask_list);
    fprintf(stderr, "xd-shell: %s: quote removal error\n", xd_original_arg);
    return NULL;
  }

  xd_list_destroy(orig_mask_list);
  return exp_arg_list;
}  // xd_arg_expander()
