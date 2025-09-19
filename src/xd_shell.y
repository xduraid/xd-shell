/*
 * ==============================================================================
 * File: xd_shell.y
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
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "xd_arg_expander.h"
#include "xd_command.h"
#include "xd_job.h"
#include "xd_jobs.h"
#include "xd_shell.h"
#include "xd_string.h"
#include "xd_utils.h"

// ========================
// Macros
// ========================

// ========================
// Function Declarations
// ========================

void yyparse_initialize();
void yyparse_cleanup();
void yyerror(const char *s);

extern void yylex_initialize();
extern void yylex_cleanup();
extern int yylex();

// ========================
// Variables
// ========================

/**
 * @brief Current token text.
 */
extern char *yytext;

/**
 * @brief Current job being parsed.
 */
static xd_job_t *xd_current_job = NULL;

/**
 * @brief Dynamic string for accumulating commands.
 */
static xd_string_t *xd_command_str = NULL;

/**
 * @brief The lookahead token.
 */
extern int yychar;

// ========================
// Public Variables
// ========================

/**
 * @brief Current command being parsed.
 */
xd_command_t *xd_current_command = NULL;

%}

/* ============================== */
/* Bison Definitions              */
/* ============================== */

%define parse.error detailed

/* ============================== */
/* Types and Tokens               */
/* ============================== */

%code requires {
  /**
   * @brief Represents a pair of strings.
   */
  typedef struct xd_string_pair_t {
    char *first;   // First string in the pair
    char *second;  // Second string in the pair
  } xd_string_pair_t;
}

%union
{
  char *string;
  xd_string_pair_t *string_pair;
}

%token <string> ARG
%token PIPE AMPERSAND NEWLINE
%token LT GT GT_GT TWO_GT TWO_GT_GT GT_AMPERSAND GT_GT_AMPERSAND
%token LEX_INTR

%nterm <string_pair> redirection_arg

%destructor { free($$); } <string>
%destructor { free($$->first); free($$->second); free($$); } <string_pair>

/* ============================== */
/* Grammar Rules                  */
/* ============================== */

%%

command_line:
    command_line job
  | %empty
  ;

job:
    command_list optional_ampersand NEWLINE {
      xd_jobs_sigchld_block();
      xd_job_execute(xd_current_job);
      xd_jobs_refresh();
      xd_jobs_sigchld_unblock();

      xd_current_job = xd_job_create();
    }
  | NEWLINE {
      xd_jobs_sigchld_block();
      xd_jobs_refresh();
      xd_jobs_sigchld_unblock();
    }
  | error NEWLINE {
      xd_jobs_sigchld_block();
      xd_jobs_refresh();
      xd_jobs_sigchld_unblock();

      xd_command_destroy(xd_current_command);
      xd_current_command = NULL;

      xd_job_destroy(xd_current_job);
      xd_current_job = xd_job_create();

      xd_sh_last_exit_code = 2;
      yyerrok;
      yyclearin;
    }
  | error LEX_INTR {
      xd_jobs_sigchld_block();
      xd_jobs_refresh();
      xd_jobs_sigchld_unblock();

      xd_command_destroy(xd_current_command);
      xd_current_command = NULL;

      xd_job_destroy(xd_current_job);
      xd_current_job = xd_job_create();

      xd_sh_last_exit_code = XD_SH_EXIT_CODE_SIGINTR;
      yyerrok;
      yyclearin;
    }
  ;

optional_ampersand:
    AMPERSAND {
      xd_current_job->is_background = 1;
    }
  | %empty
  ;

command_list:
    command_list PIPE command
  | command
  ;

command:
    argument_list io_redirection_list {
      xd_current_command->str = xd_utils_strdup(xd_command_str->str);
      xd_job_add_command(xd_current_job, xd_current_command);
      xd_current_command = NULL;
    }
  ;

argument_list:
    argument_list argument
  | argument
  ;

argument:
    ARG {
      if (xd_current_command == NULL) {
        xd_current_command = xd_command_create();
        xd_string_clear(xd_command_str);
      }
      else {
        xd_string_append_str(xd_command_str, " ");
      }

      xd_string_append_str(xd_command_str, $1);

      xd_list_t *list = xd_arg_expander($1);
      if (list == NULL) {
        xd_list_destroy(list);
        free($1);
        YYERROR;
      }
      free($1);

      for (xd_list_node_t *node = list->head; node != NULL; node = node->next) {
        xd_command_add_arg(xd_current_command, node->data);
      }
      xd_list_destroy(list);
    }
  ;

io_redirection_list:
    io_redirection_list io_redirection
  | %empty
  ;

io_redirection:
    LT redirection_arg {
      // input redirection
      xd_string_append_str(xd_command_str, " < ");
      xd_string_append_str(xd_command_str, $2->first);

      if (xd_current_command->input_file != NULL) {
        free(xd_current_command->input_file);
      }
      xd_current_command->input_file = $2->second;
      free($2->first);
      free($2);
    }
  | GT redirection_arg {
      // output redirection
      xd_string_append_str(xd_command_str, " > ");
      xd_string_append_str(xd_command_str, $2->first);

      if (xd_current_command->output_file != NULL) {
        free(xd_current_command->output_file);
      }
      xd_current_command->output_file = $2->second;
      xd_current_command->append_output = 0;
      free($2->first);
      free($2);
    }
  | GT_GT redirection_arg  {
      // output redirection (append)
      xd_string_append_str(xd_command_str, " >> ");
      xd_string_append_str(xd_command_str, $2->first);

      if (xd_current_command->output_file != NULL) {
        free(xd_current_command->output_file);
      }
      xd_current_command->output_file = $2->second;
      xd_current_command->append_output = 1;
      free($2->first);
      free($2);
    }
  | TWO_GT redirection_arg {
      // error redirection
      xd_string_append_str(xd_command_str, " 2> ");
      xd_string_append_str(xd_command_str, $2->first);

      if (xd_current_command->error_file != NULL) {
        free(xd_current_command->error_file);
      }
      xd_current_command->error_file = $2->second;
      xd_current_command->append_error = 0;
      free($2->first);
      free($2);
    }
  | TWO_GT_GT redirection_arg {
      // error redirection (append)
      xd_string_append_str(xd_command_str, " 2>> ");
      xd_string_append_str(xd_command_str, $2->first);

      if (xd_current_command->error_file != NULL) {
        free(xd_current_command->error_file);
      }
      xd_current_command->error_file = $2->second;
      xd_current_command->append_error = 1;
      free($2->first);
      free($2);
    }
  | GT_AMPERSAND redirection_arg {
      // output and error redirection
      xd_string_append_str(xd_command_str, " >& ");
      xd_string_append_str(xd_command_str, $2->first);

      if (xd_current_command->output_file != NULL) {
        free(xd_current_command->output_file);
      }
      xd_current_command->output_file = $2->second;
      xd_current_command->append_output = 0;

      if (xd_current_command->error_file != NULL) {
        free(xd_current_command->error_file);
      }
      xd_current_command->error_file = xd_utils_strdup($2->second);
      xd_current_command->append_error = 0;
      free($2->first);
      free($2);
    }
  | GT_GT_AMPERSAND redirection_arg {
      // output and error redirection
      xd_string_append_str(xd_command_str, " >>& ");
      xd_string_append_str(xd_command_str, $2->first);

      if (xd_current_command->output_file != NULL) {
        free(xd_current_command->output_file);
      }
      xd_current_command->output_file = $2->second;
      xd_current_command->append_output = 1;

      if (xd_current_command->error_file != NULL) {
        free(xd_current_command->error_file);
      }
      xd_current_command->error_file = xd_utils_strdup($2->second);
      xd_current_command->append_error = 1;
      free($2->first);
      free($2);
    }
  ;

redirection_arg:
    ARG {
      xd_list_t *list = xd_arg_expander($1);
      if (list == NULL) {
        xd_list_destroy(list);
        free($1);
        YYERROR;
      }
      if (list->length != 1) {
        fprintf(stderr, "xd-shell: %s: ambiguous redirect\n", $1);
        xd_list_destroy(list);
        free($1);
        YYERROR;
      }

      xd_string_pair_t *pair =
          (xd_string_pair_t *)malloc(sizeof(xd_string_pair_t));
      if (pair == NULL) {
        fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
      }
      pair->first = $1;
      pair->second = xd_utils_strdup(list->head->data);

      $$ = pair;
      xd_list_destroy(list);
    }
  ;

%%

// ========================
// Function Definitions
// ========================

// ========================
// Public Functions
// ========================

/**
 * @brief Initializes the scanner and parser.
 */
void yyparse_initialize() {
  yylex_initialize();
  xd_current_job = xd_job_create();
  xd_command_str = xd_string_create();
}  // yyparse_initialize()

/**
 * @brief Frees up the resources allocaed for the scanner and parser.
 */
void yyparse_cleanup() {
  yylex_cleanup();
  xd_job_destroy(xd_current_job);
  xd_command_destroy(xd_current_command);
  xd_string_destroy(xd_command_str);
}  // yyparse_cleanup()

/**
 * @brief Prints parsing errors.
 */
void yyerror(const char *s) {
  (void)s;
  if (yychar == LEX_INTR) {
    return; // don't print error message on `SIGINTR`
  }
  if (yytext == NULL) {
    fprintf(stderr, "xd-shell: syntax error\n");
  }
  else if (yychar == YYEOF) {
    fprintf(stderr, "xd-shell: syntax error near unexpected token 'EOF'\n");
  }
  else if (strcmp(yytext, "\n") == 0) {
    fprintf(stderr, "xd-shell: syntax error near unexpected token 'newline'\n");
  }
  else {
    fprintf(stderr, "xd-shell: syntax error near unexpected token '%s'\n",
            yytext);
  }
}  // yyerror()
