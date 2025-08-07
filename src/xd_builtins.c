/*
 * ==============================================================================
 * File: xd_builtins.c
 * Author: Duraid Maihoub
 * Date: 7 July 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#include "xd_builtins.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xd_jobs.h"
#include "xd_shell.h"

// ========================
// Typedefs
// ========================

/**
 * @brief Signature of builtin-command executor function.
 */
typedef int (*xd_builtin_func_t)(int argc, char **argv);

/**
 * @brief Represents a mapping of builtin command name to its executor function.
 */
typedef struct xd_builtin_mapping_t {
  const char *name;        // Name of the builtin command
  xd_builtin_func_t func;  // Executor function of the builtin
} xd_builtin_mapping_t;

// ========================
// Function Declarations
// ========================

static int xd_jobs(int argc, char **argv);

// ========================
// Variables
// ========================

/**
 * @brief Array of defined builtins.
 */
static const xd_builtin_mapping_t xd_builtins[] = {
    {"jobs", xd_jobs},
};

/**
 * @brief Number of defined builtins.
 */
static const int xd_builtins_count =
    sizeof(xd_builtins) / sizeof(xd_builtins[0]);

// ========================
// Public Variables
// ========================

// ========================
// Function Definitions
// ========================

/**
 * @brief Executor of `jobs` builtin command.
 */
static int xd_jobs(int argc, char **argv) {
  (void)argc;
  (void)argv;
  puts("Hello World!");
  return 0;
}  // xd_jobs()

// ========================
// Public Functions
// ========================

int xd_builtins_is_builtin(const char *str) {
  if (str == NULL) {
    return 0;
  }
  for (int i = 0; i < xd_builtins_count; i++) {
    if (strcmp(str, xd_builtins[i].name) == 0) {
      return 1;
    }
  }
  return 0;
}  // xd_builtins_is_builtin()

int xd_builtins_execute(int argc, char **argv) {
  if (argc == 0 || argv == NULL || argv[0] == NULL) {
    fprintf(stderr, "xd-shell: builtins: not a builtin!\n");
    return 3;
  }
  for (int i = 0; i < xd_builtins_count; i++) {
    if (strcmp(argv[0], xd_builtins[i].name) == 0) {
      return xd_builtins[i].func(argc, argv);
    }
  }
  fprintf(stderr, "xd-shell: builtins: not a builtin!\n");
  return 3;
}  // xd_builtins_execute()
