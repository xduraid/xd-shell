/*
 * ==============================================================================
 * File: xd_builtins.c
 * Author: Duraid Maihoub
 * Date: 7 Aug 2025
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

static void xd_jobs_usage();
static void xd_jobs_help();
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
 * @brief Prints usage information for the `jobs` builtin.
 */
static void xd_jobs_usage() {
  fprintf(stderr, "jobs: usage: jobs [-lp]\n");
}  // xd_jobs_usage()

/**
 * @brief Prints detailed help information for the `jobs` builtin.
 */
static void xd_jobs_help() {
  printf(
      "jobs: jobs [-lp]\n"
      "    Display status of all jobs.\n"
      "\n"
      "    Options:\n"
      "      -l    show detailed status of each process in the job\n"
      "      -p    show process ID(s)\n"
      "\n"
      "    Exit Status:\n"
      "    Returns success unless invalid option is given or error occurs.\n");
}  // xd_jobs_help()

/**
 * @brief Executor of `jobs` builtin command.
 */
static int xd_jobs(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      xd_jobs_help();
      return EXIT_SUCCESS;
    }
  }

  int detailed = 0;
  int print_pids = 0;

  int opt;
  while ((opt = getopt(argc, argv, "+lp")) != -1) {
    switch (opt) {
      case 'l':
        detailed = 1;
        break;
      case 'p':
        print_pids = 1;
        break;
      case '?':
      default:
        fprintf(stderr, "xd-shell: jobs: -%c: invalid option\n",
                optopt != 0 ? optopt : '?');
        xd_jobs_usage();
        return XD_SH_EXIT_CODE_USAGE;
    }
  }

  if (optind < argc) {
    fprintf(stderr, "xd-shell: jobs: %s: invalid argument\n", argv[optind]);
    xd_jobs_usage();
    return XD_SH_EXIT_CODE_USAGE;
  }

  xd_jobs_print_status_all(detailed, print_pids);
  return EXIT_SUCCESS;
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
  opterr = 0;  // disable getopt() errors
  optind = 0;  // reset getopt()
  for (int i = 0; i < xd_builtins_count; i++) {
    if (strcmp(argv[0], xd_builtins[i].name) == 0) {
      return xd_builtins[i].func(argc, argv);
    }
  }
  fprintf(stderr, "xd-shell: builtins: not a builtin!\n");
  return 3;
}  // xd_builtins_execute()
