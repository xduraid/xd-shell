/*
 * ==============================================================================
 * File: xd_shell.l
 * Author: Duraid Maihoub
 * Date: 17 July 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

%{

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xd_aliases.h"
#include "xd_command.h"
#include "xd_list.h"
#include "xd_readline.h"
#include "xd_shell.h"
#include "xd_shell.tab.h"
#include "xd_string.h"
#include "xd_utils.h"

// ========================
// Macros
// ========================

/**
 * @brief Custom flex input.
 */
#define YY_INPUT(buf, result, max_size)              \
  {                                                  \
    int c = xd_getc();                               \
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
  }

// ========================
// Typedefs
// ========================

/**
 * @brief Represents the type of an input source currently being scanned.
 */
typedef enum xd_input_type_t {
  XD_INPUT_TYPE_ALIAS,   // alias string source
  XD_INPUT_TYPE_STRING,  // string source (not an alias)
  XD_INPUT_TYPE_FILE,    // file source
} xd_input_type_t;

/**
 * @brief Represents an input stack frame.
 */
typedef struct xd_input_stack_frame_t {
  xd_input_type_t input_type;  // type of input
  int is_interacive;           // indicates whether input is interactive (stdin)
  FILE *file;                  // for `XD_INPUT_TYPE_FILE`
  char *str;                   // string for `_TYPE_STRING/_TYPE_ALIAS`
  char *alias_name;            // alias name
  int str_pos;                 // current offset within `str`
} xd_input_stack_frame_t;

// ========================
// Function Declarations
// ========================

static int xd_getc();
static void xd_reset_scanner();

static void *xd_input_stack_frame_copy_func(void *data);
static void xd_input_stack_frame_destroy_func(void *data);
static int xd_input_stack_frame_cmp_func(const void *data1, const void *data2);

static void xd_input_stack_push_string(char *str);
static void xd_input_stack_push_alias(char *alias_name, char *alias);
static void xd_input_stack_push_file(FILE *file);
static void xd_input_stack_pop();
static int xd_is_alias_being_expanded(const char *alias_name);

void yylex_initialize();
void yylex_cleanup();

void yylex_scan_string(char *str);
void yylex_scan_file(FILE *file);
void yylex_scan_stdin_interactive();
void yylex_scan_stdin_noninteractive();

// ========================
// Variables
// ========================

/**
 * @brief Current command being parsed.
 */
extern xd_command_t *xd_current_command;

/**
 * @brief Copy of the last interactive input line returned by `xd_readline()`.
 */
static char *xd_last_interactive_line = NULL;

/**
 * @brief Pointer to the next input character in interactive mode.
 */
static char *xd_interactive_next_char = NULL;

/**
 * @brief Indicates whether input was interrupted due to a `SIGINT`.
 */
static int xd_input_interrupted = 0;

/**
 * @brief Indicates line continuation input (after '\newline').
 */
static int xd_line_cont = 0;

/**
 * @brief Stack of input sources currently being scanned.
 */
static xd_list_t *xd_input_stack = NULL;

/**
 * @brief Dynamic string for accumulating arguments.
 */
static xd_string_t *xd_arg_str = NULL;

/**
 * @brief Dynamic string structure for building strings.
 */
static xd_string_t *xd_temp_str = NULL;

// ========================
// Public Variables
// ========================

%}

/* ============================== */
/* Options                        */
/* ============================== */

%option 8bit noyywrap noinput nounput stack noyy_top_state

/* ============================== */
/* Start Conditions               */
/* ============================== */

%x ARG_STATE SQ_STATE DQ_STATE PRM_STATE CMD_STATE

/* ============================== */
/* Patterns                       */
/* ============================== */

/* ============================== */
/* Rules                          */
/* ============================== */

%%

[ \t]+ {
  // ignore whitespaces
}

\n {
  return NEWLINE;
}

"#".* {
  // ignore comments
}

"<" {
  return LT;
}

">" {
  return GT;
}

">>" {
  return GT_GT;
}

"2>" {
  return TWO_GT;
}

"2>>" {
  return TWO_GT_GT;
}

">&" {
  return GT_AMPERSAND;
}

">>&" {
  return GT_GT_AMPERSAND;
}

"|" {
  return PIPE;
}

"&" {
  return AMPERSAND;
}

"'" {
  yy_push_state(ARG_STATE);
  yy_push_state(SQ_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

"\"" {
  yy_push_state(ARG_STATE);
  yy_push_state(DQ_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

"${" {
  yy_push_state(ARG_STATE);
  yy_push_state(PRM_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

"$(" {
  yy_push_state(ARG_STATE);
  yy_push_state(CMD_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

\\\n {
  xd_line_cont = 1;
}

\\. {
  yy_push_state(ARG_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

"$"|"{"|"("|"\\"|[^ \t\n<>|&'"${(\\] {
  yy_push_state(ARG_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<SQ_STATE>"'" {
  yy_pop_state();
  xd_string_append_str(xd_arg_str, yytext);
}

<SQ_STATE>[^']+ {
  xd_string_append_str(xd_arg_str, yytext);
}

<DQ_STATE>"\"" {
  yy_pop_state();
  xd_string_append_str(xd_arg_str, yytext);
}

<DQ_STATE>"${" {
  yy_push_state(PRM_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<DQ_STATE>"$(" {
  yy_push_state(CMD_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<DQ_STATE>\\\n {
  xd_line_cont = 1;
}

<DQ_STATE>\\. {
  xd_string_append_str(xd_arg_str, yytext);
}

<DQ_STATE>"$"|"{"|"("|"\\"|[^"${(\\]+ {
  xd_string_append_str(xd_arg_str, yytext);
}

<PRM_STATE>"}" {
  yy_pop_state();
  xd_string_append_str(xd_arg_str, yytext);
}

<PRM_STATE>"'" {
  yy_push_state(SQ_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<PRM_STATE>"\"" {
  yy_push_state(DQ_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<PRM_STATE>"${" {
  yy_push_state(PRM_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<PRM_STATE>"$(" {
  yy_push_state(CMD_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<PRM_STATE>\\\n {
  xd_line_cont = 1;
}

<PRM_STATE>\\. {
  xd_string_append_str(xd_arg_str, yytext);
}

<PRM_STATE>"$"|"{"|"("|"\\"|[^}'"${(\\]+ {
  xd_string_append_str(xd_arg_str, yytext);
}

<CMD_STATE>")" {
  yy_pop_state();
  xd_string_append_str(xd_arg_str, yytext);
}

<CMD_STATE>"'" {
  yy_push_state(SQ_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<CMD_STATE>"\"" {
  yy_push_state(DQ_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<CMD_STATE>"${" {
  yy_push_state(PRM_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<CMD_STATE>"$(" {
  yy_push_state(CMD_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<CMD_STATE>\\\n {
  xd_line_cont = 1;
}

<CMD_STATE>\\. {
  xd_string_append_str(xd_arg_str, yytext);
}

<CMD_STATE>"$"|"{"|"("|"\\"|[^)'"${(\\]+ {
  xd_string_append_str(xd_arg_str, yytext);
}

<ARG_STATE>"'" {
  yy_push_state(SQ_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<ARG_STATE>"\"" {
  yy_push_state(DQ_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<ARG_STATE>"${" {
  yy_push_state(PRM_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<ARG_STATE>"$(" {
  yy_push_state(CMD_STATE);
  xd_string_append_str(xd_arg_str, yytext);
}

<ARG_STATE>\\\n {
  xd_line_cont = 1;
}

<ARG_STATE>\\. {
  xd_string_append_str(xd_arg_str, yytext);
}

<ARG_STATE>"$"|"{"|"("|"\\"|[^ \t\n<>|&'"${(\\]+ {
  xd_string_append_str(xd_arg_str, yytext);
}

<ARG_STATE>[ \t\n<>|&] {
  yy_pop_state();

  if (xd_arg_str->length > 0) {
    if (xd_current_command == NULL &&
        xd_aliases_is_valid_name(xd_arg_str->str)) {
      // first argument in argv
      const char *alias = xd_aliases_get(xd_arg_str->str);
      if (alias != NULL && !xd_is_alias_being_expanded(xd_arg_str->str)) {
        // alias expansion
        xd_string_clear(xd_temp_str);
        xd_string_append_str(xd_temp_str, alias);
        xd_string_append_chr(xd_temp_str, yytext[0]);

        xd_sh_is_interactive = 0;
        xd_input_stack_push_alias(xd_arg_str->str, xd_temp_str->str);
        xd_string_clear(xd_arg_str);
      }
      else {
        yylval.string = xd_utils_strdup(xd_arg_str->str);
        xd_string_clear(xd_arg_str);
        yyless(0);
        return ARG;
      }
    }
    else {
      yylval.string = xd_utils_strdup(xd_arg_str->str);
      xd_string_clear(xd_arg_str);
      yyless(0);
      return ARG;
    }
  }
  else {
    yyless(0);
  }
}

<SQ_STATE><<EOF>> {
  if (xd_input_interrupted) {
    xd_reset_scanner();
    xd_input_interrupted = 0;
    return LEX_INTR;
  }
  else {
    xd_input_stack_pop();
    if (xd_input_stack->length == 0) {
      fprintf(stderr,
              "xd-shell: unexpected EOF while waiting for matching '''\n");
      xd_sh_last_exit_code = 2;
      yyterminate();
    }
  }
}

<DQ_STATE><<EOF>> {
  if (xd_input_interrupted) {
    xd_reset_scanner();
    xd_input_interrupted = 0;
    return LEX_INTR;
  }
  else {
    xd_input_stack_pop();
    if (xd_input_stack->length == 0) {
      fprintf(stderr,
              "xd-shell: unexpected EOF while waiting for matching '\"'\n");
      xd_sh_last_exit_code = 2;
      yyterminate();
    }
  }
}

<PRM_STATE><<EOF>> {
  if (xd_input_interrupted) {
    xd_reset_scanner();
    xd_input_interrupted = 0;
    return LEX_INTR;
  }
  else {
    xd_input_stack_pop();
    if (xd_input_stack->length == 0) {
      fprintf(stderr,
              "xd-shell: unexpected EOF while waiting for matching '}'\n");
      xd_sh_last_exit_code = 2;
      yyterminate();
    }
  }
}

<CMD_STATE><<EOF>> {
  if (xd_input_interrupted) {
    xd_reset_scanner();
    xd_input_interrupted = 0;
    return LEX_INTR;
  }
  else {
    xd_input_stack_pop();
    if (xd_input_stack->length == 0) {
      fprintf(stderr,
              "xd-shell: unexpected EOF while waiting for matching ')'\n");
      xd_sh_last_exit_code = 2;
      yyterminate();
    }
  }
}

<<EOF>> {
  if (xd_input_interrupted) {
    xd_reset_scanner();
    xd_input_interrupted = 0;
    return LEX_INTR;
  }
  else {
    xd_input_stack_pop();
    if (xd_input_stack->length == 0) {
      if (xd_sh_is_interactive) {
        printf("exit\n");
        fflush(stdout);
      }
      yyterminate();
    }
  }
}

%%

// ========================
// Function Definitions
// ========================

/**
 * @brief Retrieves the next character from the active input source and returns
 * it.
 *
 * Clears pending shell interrupts, then reads from the top frame on the input
 * stack when running non-interactively (files or string expansions). In
 * interactive mode it refreshes prompts, obtains a new line from
 * `xd_readline()` when necessary, and records non-duplicate entries in the
 * history buffer.
 *
 * @return The next character or `EOF` when no input remains or an interrupt is
 * pending.
 */
static int xd_getc() {
  if (xd_sh_is_interrupted) {
    xd_sh_is_interrupted = 0;
    xd_input_interrupted = 1;
    return EOF;
  }

  if (!xd_sh_is_interactive) {
    if (xd_input_stack == NULL || xd_input_stack->length == 0) {
      return EOF;
    }

    xd_input_stack_frame_t *top_frame = xd_input_stack->head->data;
    if (top_frame->input_type == XD_INPUT_TYPE_FILE) {
      return fgetc(top_frame->file);
    }
    else {
      char chr = top_frame->str[top_frame->str_pos++];
      return chr == '\0' ? EOF : chr;
    }
  }

  if (xd_interactive_next_char == NULL || *xd_interactive_next_char == '\0') {
    errno = 0;
    if (!xd_line_cont && (YYSTATE == INITIAL || YYSTATE == ARG_STATE)) {
      xd_sh_update_prompt();
      xd_readline_prompt = xd_sh_prompt;
    }
    else {
      xd_line_cont = 0;
      xd_readline_prompt = XD_SH_PROMPT2;
    }
    xd_sh_readline_running = 1;
    char *input_line = xd_readline();
    xd_sh_readline_running = 0;
    if (input_line == NULL) {
      if (errno == EINTR) {
        xd_input_interrupted = 1;
      }
      return EOF;
    }

    if (xd_last_interactive_line == NULL) {
      xd_last_interactive_line = xd_readline_history_get(-1);
    }

    // add non-duplicate and non-empty input lines to history
    if (!isspace(*input_line) &&
        (xd_last_interactive_line == NULL ||
         strcmp(input_line, xd_last_interactive_line) != 0)) {
      xd_readline_history_add(input_line);
      free(xd_last_interactive_line);
      xd_last_interactive_line = xd_utils_strdup(input_line);
    }

    xd_interactive_next_char = input_line;
  }
  return *xd_interactive_next_char++;
}  // xd_getc()

/**
 * @brief Resets the scanner to its initial state.
 */
static void xd_reset_scanner() {
  while (xd_input_stack->length > 1) {
    xd_input_stack_pop();
  }
  while (YYSTATE != INITIAL) {
    yy_pop_state();
  }
  xd_string_clear(xd_arg_str);
  yyrestart(yyin);
}  // xd_reset_scanner()

/**
 * @brief Creates a newly-allocated shallow copy of the passed input stack
 * frame.
 *
 * @param data Pointer to the `xd_input_stack_frame_t` structure to be copied.
 *
 * @return A pointer to the newly allocated shallow copy of the frame, or `NULL`
 * if the passed pointer is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `free()` and passing it the returned pointer.
 */
static void *xd_input_stack_frame_copy_func(void *data) {
  if (data == NULL) {
    return NULL;
  }

  xd_input_stack_frame_t *frame = data;
  xd_input_stack_frame_t *copy =
      (xd_input_stack_frame_t *)malloc(sizeof(xd_input_stack_frame_t));
  if (copy == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  copy->input_type = frame->input_type;
  copy->file = frame->file;
  copy->str = frame->str;
  copy->alias_name = frame->alias_name;
  copy->str_pos = frame->str_pos;
  copy->is_interacive = frame->is_interacive;
  return copy;
}  // xd_input_stack_frame_copy_func()

/**
 * @brief Frees the memory and associated resources of the passed input stack
 * frame.
 *
 * @param data Pointer to the `xd_input_stack_frame_t` structure to be freed.
 */
static void xd_input_stack_frame_destroy_func(void *data) {
  xd_input_stack_frame_t *frame = data;
  if (frame->input_type == XD_INPUT_TYPE_FILE) {
    if (frame->file != stdin) {
      fclose(frame->file);
    }
  }
  else {
    free(frame->str);
    free(frame->alias_name);
  }
  free(frame);
}  // xd_input_stack_frame_destroy_func()

/**
 * @brief Compares two input stack frames (not-implemented).
 *
 * @param data1 Pointer to the first `xd_input_stack_frame_t` structure.
 * @param data2 Pointer to the second `xd_input_stack_frame_t` structure.
 *
 * @return Always returns `0`.
 *
 * @note This function always returns `0` and is only provided to satisfy the
 * generic list `xd_list_t` interface requirements.
 */
static int xd_input_stack_frame_cmp_func(const void *data1, const void *data2) {
  (void)data1;
  (void)data2;
  return 0;
}  // xd_input_stack_frame_cmp_func()

/**
 * @brief Pushes a string input frame onto the scanner stack.
 *
 * @param str Pointer to the null-terminated string to be scanned.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
static void xd_input_stack_push_string(char *str) {
  xd_input_stack_frame_t frame;
  frame.input_type = XD_INPUT_TYPE_STRING;
  frame.file = NULL;
  frame.alias_name = NULL;
  frame.str = xd_utils_strdup(str);
  frame.str_pos = 0;
  frame.is_interacive = xd_sh_is_interactive;
  xd_list_add_first(xd_input_stack, &frame);
}  // xd_input_stack_push_string()

/**
 * @brief Pushes an alias expansion frame onto the scanner stack.
 *
 * @param alias_name Pointer to the alias name being expanded.
 * @param alias Pointer to the alias definition that will be scanned.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
static void xd_input_stack_push_alias(char *alias_name, char *alias) {
  xd_input_stack_frame_t frame;
  frame.input_type = XD_INPUT_TYPE_ALIAS;
  frame.file = NULL;
  frame.alias_name = xd_utils_strdup(alias_name);
  frame.str = xd_utils_strdup(alias);
  frame.str_pos = 0;
  frame.is_interacive = xd_sh_is_interactive;
  xd_list_add_first(xd_input_stack, &frame);
}  // xd_input_stack_push_alias()

/**
 * @brief Pushes a file input frame onto the scanner stack.
 *
 * @param file Pointer to the open file stream to be scanned.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
static void xd_input_stack_push_file(FILE *file) {
  xd_input_stack_frame_t frame;
  frame.input_type = XD_INPUT_TYPE_FILE;
  frame.file = file;
  frame.str = NULL;
  frame.alias_name = NULL;
  frame.str_pos = 0;
  frame.is_interacive = xd_sh_is_interactive;
  xd_list_add_first(xd_input_stack, &frame);
}  // xd_input_stack_push_file()

/**
 * @brief Pops the top frame from the input stack.
 *
 * @note This function does nothing when the stack is empty.
 */
static void xd_input_stack_pop() {
  if (xd_input_stack->length == 0) {
    return;
  }
  xd_list_remove_first(xd_input_stack);
  if (xd_input_stack->length > 0) {
    xd_input_stack_frame_t *top = xd_input_stack->head->data;
    xd_sh_is_interactive = top->is_interacive;
  }
}  // xd_input_stack_pop()

/**
 * @brief Checks if the passed alias name is currently being expanded.
 *
 * @param alias_name The alias name to be checked.
 *
 * @return `1` if the alias name is being expanded, `0` otherwise.
 */
static int xd_is_alias_being_expanded(const char *alias_name) {
  if (xd_input_stack->length == 0) {
    return 0;
  }

  xd_list_node_t *node = xd_input_stack->head;
  while (node != NULL) {
    xd_input_stack_frame_t *frame = node->data;
    if (frame->input_type != XD_INPUT_TYPE_ALIAS) {
      break;
    }
    if (xd_utils_str_comp_func(frame->alias_name, alias_name) == 0) {
      return 1;
    }
    node = node->next;
  }
  return 0;
}  // xd_is_alias_being_expanded()

// ========================
// Public Functions
// ========================

/**
 * @brief Initializes the scanner.
 */
void yylex_initialize() {
  xd_input_stack = xd_list_create(xd_input_stack_frame_copy_func,
                                  xd_input_stack_frame_destroy_func,
                                  xd_input_stack_frame_cmp_func);
  xd_arg_str = xd_string_create();
  xd_temp_str = xd_string_create();
}  // yylex_init()

/**
 * @brief Frees up the resources allocaed for the scanner.
 */
void yylex_cleanup() {
  yylex_destroy();
  xd_list_destroy(xd_input_stack);
  xd_string_destroy(xd_arg_str);
  xd_string_destroy(xd_temp_str);
  free(xd_last_interactive_line);
  xd_last_interactive_line = NULL;
}  // yylex_cleanup()

/**
 * @brief Pushes a string input source onto the scanner stack.
 *
 * Marks the scanner as non-interactive and copies the provided string so it
 * can be tokenised independently of the caller's buffer.
 *
 * @param str Pointer to the null-terminated string to scan.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The function returns immediately if `str` is `NULL`.
 */
void yylex_scan_string(char *str) {
  if (str == NULL) {
    return;
  }
  xd_sh_is_interactive = 0;
  xd_input_stack_push_string((char *)str);
}  // yylex_scan_string()

/**
 * @brief Pushes a file input source onto the scanner stack.
 *
 * Marks the scanner as non-interactive and begins consuming tokens from the
 * provided file stream without taking ownership of the `FILE`.
 *
 * @param file Pointer to an open file stream to scan.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The function returns immediately if `file` is `NULL`.
 */
void yylex_scan_file(FILE *file) {
  if (file == NULL) {
    return;
  }
  xd_sh_is_interactive = 0;
  xd_input_stack_push_file(file);
}  // yylex_scan_file()

/**
 * @brief Pushes `stdin` onto the scanner stack in interactive mode.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
void yylex_scan_stdin_interactive() {
  xd_sh_is_interactive = 1;
  xd_input_stack_push_file(stdin);
}  // yylex_scan_stdin_interactive()

/**
 * @brief Pushes `stdin` onto the scanner stack in non-interactive mode.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
void yylex_scan_stdin_noninteractive() {
  xd_sh_is_interactive = 0;
  xd_input_stack_push_file(stdin);
}  // yylex_scan_stdin_noninteractive()
