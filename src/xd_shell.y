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

#include <stdio.h>

// ========================
// Macros
// ========================

// ========================
// Function Declarations
// ========================

void yyerror(const char *s);
extern int yylex();

// ========================
// Variables
// ========================

// ========================
// Public Variables
// ========================

// ========================
// Function Definitions
// ========================

/**
 * @brief Prints parsing errors.
 */
void yyerror(const char *s) {
  (void)s;
  fprintf(stderr, "xd-shell: syntax error\n");
}  // yyerror()

// ========================
// Public Functions
// ========================

%}

/* ============================== */
/* Bison Definitions              */
/* ============================== */

%define parse.error detailed

/* ============================== */
/* Types and Tokens               */
/* ============================== */

%union
{
  char *string;
}

%token <string> ARG
%token NEWLINE

%destructor { free($$); } <string>

/* ============================== */
/* Grammar Rules                  */
/* ============================== */

%%

command_line:
    command_line ARG NEWLINE {
      puts("[BAR]");
      free($2);
    }
  | %empty
  ;

%%
