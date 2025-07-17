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

// ========================
// Function Declarations
// ========================

static void xd_sh_init() __attribute__((constructor));
static void xd_sh_destroy() __attribute__((destructor));
static int xd_sh_run();

// flex and bison functions

extern int yylex_destroy();
extern int yyparse();

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
 * @brief Constructor, runs before main to initialize the shell.
 */
static void xd_sh_init() {
}  // xd_sh_init()

/**
 * @brief Destructor, runs before exit to cleanup after the shell.
 */
static void xd_sh_destroy() {
  yylex_destroy();
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
