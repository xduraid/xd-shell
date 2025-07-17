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

#include "xd_shell.tab.h"

// ========================
// Macros
// ========================

// ========================
// Function Declarations
// ========================

// ========================
// Variables
// ========================

// ========================
// Public Variables
// ========================

// ========================
// Function Definitions
// ========================

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

"foo" {
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

.+ {
  puts("UNKNOWN TOKEN");
}

<<EOF>> {
  return 0;
}

%%
