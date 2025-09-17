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
// Function Declarations
// ========================

static int xd_getc();
static void xd_reset_scanner();
static void xd_alias_expansion_stack_push(char *alias_name);
static void xd_alias_expansion_stack_pop();
static int xd_alias_is_being_expanded(const char *alias_name);

void yylex_initialize();
void yylex_cleanup();

// ========================
// Variables
// ========================

/**
 * @brief Current command being parsed.
 */
extern xd_command_t *xd_current_command;

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
 * @brief Stack of currently expanded aliases, in sync with the yy buffer stack.
 *
 * Each element is a string for an alias buffer or `NULL` for non-alias buffers
 * (e.g., source files). Used to prevent recursive alias expansions.
 */
static xd_list_t *xd_alias_expansion_stack = NULL;

/**
 * @brief Dynamic string for accumulating arguments.
 */
static xd_string_t *xd_arg_str = NULL;

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
  yyless(0);

  if (xd_arg_str->length > 0) {
    if (xd_current_command == NULL &&
        xd_aliases_is_valid_name(xd_arg_str->str)) {
      // first argument in argv
      const char *alias = xd_aliases_get(xd_arg_str->str);
      if (alias != NULL && !xd_alias_is_being_expanded(xd_arg_str->str)) {
        // alias expansion
        xd_alias_expansion_stack_push(xd_arg_str->str);
        xd_string_clear(xd_arg_str);

        // append delimiter to the alias to finish scanning it before `EOF`
        xd_string_append_str(xd_arg_str, alias);
        xd_string_append_chr(xd_arg_str, ' ');

        YY_BUFFER_STATE current_buf = YY_CURRENT_BUFFER;
        YY_BUFFER_STATE new_buf = yy_scan_string(xd_arg_str->str);
        yy_switch_to_buffer(current_buf);
        yypush_buffer_state(new_buf);
        xd_string_clear(xd_arg_str);
      }
      else {
        yylval.string = xd_utils_strdup(xd_arg_str->str);
        xd_string_clear(xd_arg_str);
        return ARG;
      }
    }
    else {
      yylval.string = xd_utils_strdup(xd_arg_str->str);
        xd_string_clear(xd_arg_str);
      return ARG;
    }
  }
}

<SQ_STATE><<EOF>> {
  if (xd_input_interrupted) {
    xd_reset_scanner();
    xd_input_interrupted = 0;
    return LEX_INTR;
  }
  else {
    fprintf(stderr, "xd-shell: unexpected EOF while waiting for matching '''\n");
    xd_sh_last_exit_code = 2;
    yyterminate();
  }
}

<DQ_STATE><<EOF>> {
  if (xd_input_interrupted) {
    xd_reset_scanner();
    xd_input_interrupted = 0;
    return LEX_INTR;
  }
  else {
    fprintf(stderr, "xd-shell: unexpected EOF while waiting for matching '\"'\n");
    xd_sh_last_exit_code = 2;
    yyterminate();
  }
}

<PRM_STATE><<EOF>> {
  if (xd_input_interrupted) {
    xd_reset_scanner();
    xd_input_interrupted = 0;
    return LEX_INTR;
  }
  else {
    fprintf(stderr, "xd-shell: unexpected EOF while waiting for matching '}'\n");
    xd_sh_last_exit_code = 2;
    yyterminate();
  }
}

<CMD_STATE><<EOF>> {
  if (xd_input_interrupted) {
    xd_reset_scanner();
    xd_input_interrupted = 0;
    return LEX_INTR;
  }
  else {
    fprintf(stderr, "xd-shell: unexpected EOF while waiting for matching ')'\n");
    xd_sh_last_exit_code = 2;
    yyterminate();
  }
}

<<EOF>> {
  if (xd_input_interrupted) {
    xd_reset_scanner();
    xd_input_interrupted = 0;
    return LEX_INTR;
  }
  else {
    xd_alias_expansion_stack_pop();
    yypop_buffer_state();
    if (YY_CURRENT_BUFFER == NULL) {
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
 * @brief Reads one character from `stdin` and returns it.
 *
 * If shell is running in interactive mode, it uses `xd_readline()`, otherwise
 * it reads the character normally using `getchar()`.
 *
 * @return The read character.
 */
static int xd_getc() {
  if (!xd_sh_is_interactive) {
    return getchar();
  }

  if (xd_interactive_next_char == NULL || *xd_interactive_next_char == '\0') {
    errno = 0;
    if (!xd_line_cont && (YYSTATE == INITIAL || YYSTATE == ARG_STATE)) {
      xd_readline_prompt = xd_sh_prompt;
    }
    else {
      xd_line_cont = 0;
      xd_readline_prompt = XD_SH_PROMPT2;
    }
    xd_sh_readline_running = 1;
    xd_interactive_next_char = xd_readline();
    xd_sh_readline_running = 0;
    if (xd_interactive_next_char == NULL) {
      if (errno == EINTR) {
        xd_input_interrupted = 1;
      }
      return EOF;
    }
  }
  return *xd_interactive_next_char++;
}  // xd_getc()

/**
 * @brief Resets the scanner to its initial state, clearing the input buffer,
 * argument buffer, start conditions stack, and buffers stack.
 */
static void xd_reset_scanner() {
  while (YY_CURRENT_BUFFER != NULL) {
    xd_alias_expansion_stack_pop();
    yypop_buffer_state();
  }
  while (YYSTATE != INITIAL) {
    yy_pop_state();
  }
  xd_string_clear(xd_arg_str);
  yyrestart(yyin);
}  // xd_reset_scanner()

/**
 * @brief Pushes the passed alias name to the top of the alias expansion stack.
 *
 * @param alias_name Pointer to the alias name string.
 */
static void xd_alias_expansion_stack_push(char *alias_name) {
  xd_list_add_first(xd_alias_expansion_stack, alias_name);
}  // xd_alias_expansion_stack_push()

/**
 * @brief Pops an element from the top of the alias expansion stack.
 */
static void xd_alias_expansion_stack_pop() {
  if (xd_alias_expansion_stack->length == 0) {
    return;
  }
  xd_list_remove_first(xd_alias_expansion_stack);
}  // xd_alias_expansion_stack_pop()

/**
 * @brief Checks if an alias is currently being expanded.
 *
 * @param alias_name Name of the alias to be checked.
 * @return `1` if the alias is being expanded, `0` otherwise.
 */
static int xd_alias_is_being_expanded(const char *alias_name) {
  xd_list_node_t *node = xd_alias_expansion_stack->head;
  while (node != NULL && node->data != NULL) {
    if (xd_alias_expansion_stack->comp_func(alias_name, node->data) == 0) {
      return 1;
    }
    node = node->next;
  }
  return 0;
}  // xd_alias_is_being_expanded()

// ========================
// Public Functions
// ========================

/**
 * @brief Initializes the scanner.
 */
void yylex_initialize() {
  xd_alias_expansion_stack =
      xd_list_create(xd_utils_str_copy_func, xd_utils_str_destroy_func,
                     xd_utils_str_comp_func);
  xd_arg_str = xd_string_create();
}  // yylex_init()

/**
 * @brief Frees up the resources allocaed for the scanner.
 */
void yylex_cleanup() {
  xd_list_destroy(xd_alias_expansion_stack);
  yylex_destroy();
  xd_string_destroy(xd_arg_str);
}  // yylex_cleanup()
