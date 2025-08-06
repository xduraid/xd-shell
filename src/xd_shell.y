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

#include "xd_command.h"
#include "xd_job.h"
#include "xd_jobs.h"

// ========================
// Macros
// ========================

// ========================
// Function Declarations
// ========================

static void xd_command_buffer_append_str(const char *str);
static void xd_command_buffer_reset();
void yyparse_cleanup();
void yyerror(const char *s);
extern int yylex();

// ========================
// Variables
// ========================

/**
 * @brief Current token text.
 */
extern char *yytext;

/**
 * @brief Current command being parsed.
 */
static xd_command_t *xd_current_command = NULL;

/**
 * @brief Current job being parsed.
 */
static xd_job_t *xd_current_job = NULL;

/**
 * @brief The command string buffer.
 */
static char *xd_command_buffer = NULL;

/**
 * @brief The length of the command.
 */
static int xd_command_buffer_length = 0;

/**
 * @brief The maximum capacity of `xd_command_buffer`.
 */
static int xd_command_buffer_capacity = 0;

// ========================
// Public Variables
// ========================

// ========================
// Function Definitions
// ========================

static void xd_command_buffer_append_str(const char *str) {
  int str_len = (int)strlen(str);

  // resize if needed
  if (xd_command_buffer_length + str_len > xd_command_buffer_capacity - 1) {
    // resize to multiple of `LINE_MAX`
    int new_capacity = xd_command_buffer_length + str_len + 1;
    if (new_capacity % LINE_MAX != 0) {
      new_capacity += LINE_MAX - (new_capacity % LINE_MAX);
    }

    char *ptr = (char *)realloc(xd_command_buffer, sizeof(char) * new_capacity);
    if (ptr == NULL) {
      fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
              strerror(errno));
      exit(EXIT_FAILURE);
    }

    xd_command_buffer = ptr;
    xd_command_buffer_capacity = new_capacity;
  }
  memcpy(xd_command_buffer + xd_command_buffer_length, str, str_len);
  xd_command_buffer_length += str_len;
  xd_command_buffer[xd_command_buffer_length] = '\0';
}  // xd_command_buffer_append_str()

static void xd_command_buffer_reset() {
  if (xd_command_buffer == NULL) {
    return;
  }
  xd_command_buffer_length = 0;
  xd_command_buffer[0] = '\0';
}  // xd_command_buffer_reset()


/**
 * @brief Prints parsing errors.
 */
void yyerror(const char *s) {
  (void)s;
  if (yytext == NULL) {
    fprintf(stderr, "xd-shell: syntax error\n");
    return;
  }
  if (strcmp(yytext, "\n") == 0) {
    fprintf(stderr, "xd-shell: syntax error near unexpected token 'newline'\n");
  }
  else {
    fprintf(stderr, "xd-shell: syntax error near unexpected token '%s'\n",
            yytext);
  }
}  // yyerror()

// ========================
// Public Functions
// ========================

/**
 * @brief Frees up the memory allocated during parsing.
 */
void yyparse_cleanup() {
  xd_job_destroy(xd_current_job);
  xd_command_destroy(xd_current_command);
  free(xd_command_buffer);
}  // yyparse_cleanup()

%}

/* ============================== */
/* Bison Definitions              */
/* ============================== */

%define parse.error detailed

%initial-action {
  xd_current_job = xd_job_create();
}

/* ============================== */
/* Types and Tokens               */
/* ============================== */

%union
{
  char *string;
}

%token <string> ARG
%token PIPE AMPERSAND NEWLINE
%token LT GT GT_GT TWO_GT TWO_GT_GT GT_AMPERSAND GT_GT_AMPERSAND

%destructor { free($$); } <string>

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

      xd_job_destroy(xd_current_job);
      xd_current_job = xd_job_create();
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
    executable argument_list io_redirection_list {
      xd_current_command->str = strdup(xd_command_buffer);
      if (xd_current_command->str == NULL) {
        fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
        exit(EXIT_FAILURE);
      }

      xd_job_add_command(xd_current_job, xd_current_command);
      xd_current_command = NULL;
    }
  ;

executable:
    ARG {
      xd_command_buffer_reset();
      xd_command_buffer_append_str($1);

      xd_current_command = xd_command_create($1);
      free($1);
    }
  ;

argument_list:
    argument_list argument
  | %empty
  ;

argument:
    ARG {
      xd_command_buffer_append_str(" ");
      xd_command_buffer_append_str($1);

      xd_command_add_arg(xd_current_command, $1);
      free($1);
    }
  ;

io_redirection_list:
    io_redirection_list io_redirection
  | %empty
  ;

io_redirection:
    LT ARG {
      // input redirection
      xd_command_buffer_append_str(" < ");
      xd_command_buffer_append_str($2);

      if (xd_current_command->input_file != NULL) {
        free(xd_current_command->input_file);
      }
      xd_current_command->input_file = $2;
    }
  | GT ARG {
      // output redirection
      xd_command_buffer_append_str(" > ");
      xd_command_buffer_append_str($2);

      if (xd_current_command->output_file != NULL) {
        free(xd_current_command->output_file);
      }
      xd_current_command->output_file = $2;
      xd_current_command->append_output = 0;
    }
  | GT_GT ARG  {
      // output redirection (append)
      xd_command_buffer_append_str(" >> ");
      xd_command_buffer_append_str($2);

      if (xd_current_command->output_file != NULL) {
        free(xd_current_command->output_file);
      }
      xd_current_command->output_file = $2;
      xd_current_command->append_output = 1;
    }
  | TWO_GT ARG {
      // error redirection
      xd_command_buffer_append_str(" 2> ");
      xd_command_buffer_append_str($2);

      if (xd_current_command->error_file != NULL) {
        free(xd_current_command->error_file);
      }
      xd_current_command->error_file = $2;
      xd_current_command->append_error = 0;
    }
  | TWO_GT_GT ARG {
      // error redirection (append)
      xd_command_buffer_append_str(" 2>> ");
      xd_command_buffer_append_str($2);

      if (xd_current_command->error_file != NULL) {
        free(xd_current_command->error_file);
      }
      xd_current_command->error_file = $2;
      xd_current_command->append_error = 1;
    }
  | GT_AMPERSAND ARG {
      // output and error redirection
      xd_command_buffer_append_str(" >& ");
      xd_command_buffer_append_str($2);

      if (xd_current_command->output_file != NULL) {
        free(xd_current_command->output_file);
      }
      xd_current_command->output_file = $2;
      xd_current_command->append_output = 0;

      if (xd_current_command->error_file != NULL) {
        free(xd_current_command->error_file);
      }
      xd_current_command->error_file = strdup($2);
      xd_current_command->append_error = 0;
      if (xd_current_command->error_file == NULL) {
        fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
        exit(EXIT_FAILURE);
      }
    }
  | GT_GT_AMPERSAND ARG {
      // output and error redirection
      xd_command_buffer_append_str(" >>& ");
      xd_command_buffer_append_str($2);

      if (xd_current_command->output_file != NULL) {
        free(xd_current_command->output_file);
      }
      xd_current_command->output_file = $2;
      xd_current_command->append_output = 1;

      if (xd_current_command->error_file != NULL) {
        free(xd_current_command->error_file);
      }
      xd_current_command->error_file = strdup($2);
      xd_current_command->append_error = 1;
      if (xd_current_command->error_file == NULL) {
        fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
        exit(EXIT_FAILURE);
      }
    }
  ;

%%
