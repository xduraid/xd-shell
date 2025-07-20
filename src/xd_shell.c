/*
 * ==============================================================================
 * File: xd_shell.c
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

#include "xd_shell.h"

#include <string.h>
#include <unistd.h>

// ========================
// Function Declarations
// ========================

static void xd_sh_init() __attribute__((constructor));
static void xd_sh_destroy() __attribute__((destructor));
static int xd_sh_run();

// flex and bison functions

extern int yylex_destroy();
extern int yyparse();
extern void yyparse_cleanup();

// ========================
// Variables
// ========================

// ========================
// Public Variables
// ========================

int xd_sh_is_interactive = 0;
char xd_sh_prompt[XD_SH_PROMPT_MAX_LENGTH] = {0};

// ========================
// Function Definitions
// ========================

/**
 * @brief Constructor, runs before main to initialize the shell.
 */
static void xd_sh_init() {
  xd_sh_is_interactive = (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO));
  if (xd_sh_is_interactive) {
    strncpy(xd_sh_prompt, "\e[0;94mxd-shell\e[0m$ ", XD_SH_PROMPT_MAX_LENGTH);
  }
}  // xd_sh_init()

/**
 * @brief Destructor, runs before exit to cleanup after the shell.
 */
static void xd_sh_destroy() {
  yylex_destroy();
  yyparse_cleanup();
}  // xd_sh_destroy()

/**
 * @brief Runs the shell.
 *
 * @return returns the shell's exit code.
 */
static int xd_sh_run() {
  return yyparse();
}  // xd_sh_run()

// ========================
// Public Functions
// ========================

// ========================
// Main
// ========================

/**
 * @brief Program entry point.
 */
int main() {
  return xd_sh_run();
}  // main()
