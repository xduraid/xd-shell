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

#include "xd_readline.h"
#include "xd_shell.h"
#include "xd_shell.tab.h"

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

// ========================
// Variables
// ========================

/**
 * @brief Pointer to the next input character in interactive mode.
 */
static char *xd_interactive_next_char = NULL;

// ========================
// Public Variables
// ========================

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
    xd_readline_prompt = xd_sh_prompt;
    xd_sh_readline_running = 1;
    xd_interactive_next_char = xd_readline();
    xd_sh_readline_running = 0;
    if (xd_interactive_next_char == NULL) {
      if (errno == EINTR) {
        // system call interrupt
        return xd_getc();
      }
      // EOF (`Ctrl+D`)
      return EOF;
    }
  }
  return *xd_interactive_next_char++;
}  // xd_getc()

// ========================
// Public Functions
// ========================

%}

/* ============================== */
/* Options                        */
/* ============================== */

%option noyywrap noinput nounput

/* ============================== */
/* Start Conditions               */
/* ============================== */

/* ============================== */
/* Patterns                       */
/* ============================== */

/* ============================== */
/* Rules                          */
/* ============================== */

%%

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

"$?" {
  printf("%d\n", xd_sh_last_exit_code);
}

[^ \t\n<>|&]+ {
  yylval.string = strdup(yytext);
  if (yylval.string == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  return ARG;
}

[ \t]+ {
  // ignore whitespaces
}

\n {
  return NEWLINE;
}

<<EOF>> {
  return 0;
}

%%
