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

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xd_aliases.h"
#include "xd_jobs.h"
#include "xd_shell.h"
#include "xd_signals.h"
#include "xd_utils.h"
#include "xd_vars.h"

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

static void xd_kill_usage();
static void xd_kill_help();
static int xd_kill(int argc, char **argv);

static void xd_fg_usage();
static void xd_fg_help();
static int xd_fg(int argc, char **argv);

static void xd_bg_usage();
static void xd_bg_help();
static int xd_bg(int argc, char **argv);

static void xd_alias_usage();
static void xd_alias_help();
static int xd_alias(int argc, char **argv);

static void xd_unalias_usage();
static void xd_unalias_help();
static int xd_unalias(int argc, char **argv);

static void xd_set_usage();
static void xd_set_help();
static int xd_set(int argc, char **argv);

// ========================
// Variables
// ========================

/**
 * @brief Array of defined builtins.
 */
static const xd_builtin_mapping_t xd_builtins[] = {
    {"jobs",    xd_jobs   },
    {"kill",    xd_kill   },
    {"fg",      xd_fg     },
    {"bg",      xd_bg     },
    {"alias",   xd_alias  },
    {"unalias", xd_unalias},
    {"set",     xd_set    },
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

/**
 * @brief Prints usage information for the `kill` builtin.
 */
static void xd_kill_usage() {
  fprintf(
      stderr,
      "kill: usage: kill [-s sigspec | -n signum] pid | jobspec ... or kill "
      "-l\n");
}  // xd_kill_usage()

/**
 * @brief Prints detailed help information for the `kill` builtin.
 */
static void xd_kill_help() {
  printf(
      "kill: kill [-s sigspec | -n signum] pid | jobspec ... or kill -l\n"
      "    Send a signal to a job or process.\n"
      "\n"
      "    Send the processes specified by pid or jobspec the signal named by\n"
      "    sigspec or signum. If neither sigspec nor signum is given, then\n"
      "    SIGTERM is assumed.\n"
      "\n"
      "    Options:\n"
      "      -s sig    sig is a signal name\n"
      "      -n sig    sig is a signal number\n"
      "      -l        list the signal names and their numbers\n"
      "\n"
      "    Exit Status:\n"
      "    Returns success unless invalid option is given or error occurs.\n");
}  // xd_kill_help()

/**
 * @brief Executor of `kill` builtin command.
 */
static int xd_kill(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      xd_kill_help();
      return EXIT_SUCCESS;
    }
  }

  int print_sigs = 0;
  char *sigspec = NULL;

  // parse options
  int opt;
  int getopt_done = 0;
  int idx = 1;
  while (!getopt_done && (opt = getopt(argc, argv, "+:ls:n:")) != -1) {
    switch (opt) {
      case 'l':
        print_sigs = 1;
        break;
      case 's':
      case 'n':
        sigspec = optarg;
        break;
      case ':':
        fprintf(stderr, "xd-shell: kill: -%c: option requires an argument\n",
                optopt);
        xd_kill_usage();
        return XD_SH_EXIT_CODE_USAGE;
      case '?':
      default:
        if (isdigit(argv[idx][1])) {
          getopt_done = 1;
          break;
        }
        fprintf(stderr, "xd-shell: kill: -%c: invalid option\n",
                optopt != 0 ? optopt : '?');
        xd_kill_usage();
        return XD_SH_EXIT_CODE_USAGE;
    }
    if (!getopt_done) {
      idx = optind;
    }
  }

  // list signals
  if (print_sigs) {
    xd_signals_print_all();
    return EXIT_SUCCESS;
  }

  // parse sigspec if provided
  int signum = SIGTERM;
  if (sigspec != NULL) {
    signum = xd_signals_signal_number(sigspec);
    if (signum == -1) {
      fprintf(stderr, "xd-shell: kill: %s: invalid signal specification\n",
              sigspec);
      return EXIT_FAILURE;
    }
  }

  int operand_count = argc - idx;
  if (operand_count < 1) {
    fprintf(stderr, "xd-shell: kill: missing pid or jobspec\n");
    xd_kill_usage();
    return XD_SH_EXIT_CODE_USAGE;
  }

  // parse pids and jobspecs

  int success_count = 0;
  for (int i = 0; i < operand_count; i++) {
    const char *operand = argv[idx++];
    xd_job_t *job = NULL;
    pid_t pid = -1;
    if (*operand == '%') {
      if (strcmp(operand, "%%") == 0 || strcmp(operand, "%+") == 0) {
        job = xd_jobs_get_current();
      }
      else if (strcmp(operand, "%-") == 0) {
        job = xd_jobs_get_previous();
      }
      else {
        long job_id = -1;
        xd_utils_strtol(operand + 1, &job_id);
        job = xd_jobs_get_with_id((int)job_id);
      }

      if (job == NULL) {
        fprintf(stderr, "xd-shell: kill: %s: no such job\n", operand);
        continue;
      }
      pid = -job->pgid;
    }
    else {
      long temp_pid = -1;
      if (xd_utils_strtol(operand, &temp_pid) == -1) {
        fprintf(stderr,
                "xd-shell: kill: %s: arguments must be process or job IDs\n",
                operand);
        continue;
      }

      pid = (int)temp_pid;
    }

    // kill to jobspec in non-interactive mode, kill each pid separately
    if (job != NULL && !xd_sh_is_interactive) {
      if (xd_jobs_kill(job, signum) == -1) {
        continue;
      }
      success_count++;
    }
    else if (kill(pid, signum) == -1) {
      fprintf(stderr, "xd-shell: kill: (%s) - %s\n", operand, strerror(errno));
      continue;
    }
    success_count++;
  }

  return success_count == operand_count ? EXIT_SUCCESS : EXIT_FAILURE;
}  // xd_kill()

/**
 * @brief Prints usage information for the `fg` builtin.
 */
static void xd_fg_usage() {
  fprintf(stderr, "fg: usage: fg [job_spec]\n");
}  // xd_fg_usage()

/**
 * @brief Prints detailed help information for the `fg` builtin.
 */
static void xd_fg_help() {
  printf(
      "fg: fg [job_spec]\n"
      "    Move job to the foreground.\n"
      "\n"
      "    Place the job identified by job_spec in foreground, making it the\n"
      "    current job. If job_spec is not present, the shell's notion of the\n"
      "    current job is used.\n"
      "\n"
      "    Exit Status:\n"
      "    Status of command placed in foreground unless an error occurs.\n");
}  // xd_fg_help()

/**
 * @brief Executor of `fg` builtin command.
 */
static int xd_fg(int argc, char **argv) {
  if (!xd_sh_is_interactive || !isatty(STDIN_FILENO) || getpid() != xd_sh_pid) {
    fprintf(stderr, "xd-shell: fg: no job control\n");
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      xd_fg_help();
      return EXIT_SUCCESS;
    }
  }

  if (argc > 2) {
    fprintf(stderr, "xd-shell: fg: too many arguments\n");
    xd_fg_usage();
    return XD_SH_EXIT_CODE_USAGE;
  }

  int opt;
  while ((opt = getopt(argc, argv, "")) != -1) {
    switch (opt) {
      case '?':
      default:
        fprintf(stderr, "xd-shell: fg: -%c: invalid option\n",
                optopt != 0 ? optopt : '?');
        xd_fg_usage();
        return XD_SH_EXIT_CODE_USAGE;
    }
  }

  // get the target job
  const char *operand = (argc > 1) ? argv[1] : NULL;
  xd_job_t *job = NULL;

  if (operand == NULL) {
    job = xd_jobs_get_current();
  }
  else if (*operand == '%') {
    if (strcmp(operand, "%%") == 0 || strcmp(operand, "%+") == 0) {
      job = xd_jobs_get_current();
    }
    else if (strcmp(operand, "%-") == 0) {
      job = xd_jobs_get_previous();
    }
    else {
      long job_id = -1;
      xd_utils_strtol(operand + 1, &job_id);
      job = xd_jobs_get_with_id((int)job_id);
    }
  }

  if (job == NULL) {
    fprintf(stderr, "xd-shell: fg: %s: no such job\n",
            operand == NULL ? "current" : operand);
    return EXIT_FAILURE;
  }

  xd_job_print_string(job);

  if (xd_jobs_put_in_foreground(job->pgid) == -1) {
    return EXIT_FAILURE;
  }
  while (job->has_tty_modes &&
         tcsetattr(STDIN_FILENO, TCSADRAIN, &job->tty_modes) == -1) {
    if (errno == EINTR) {
      continue;
    }
    break;
  }

  if (kill(-job->pgid, SIGCONT) == -1) {
    xd_jobs_put_in_foreground(xd_sh_pgid);
    fprintf(stderr, "xd-shell: fg: %s: %s\n",
            operand == NULL ? "current" : operand, strerror(errno));
    return EXIT_FAILURE;
  }
  xd_jobs_wait_non_blocking(job);

  int exit_status = xd_jobs_wait(job);

  xd_jobs_put_in_foreground(xd_sh_pgid);

  if (xd_job_is_alive(job)) {
    job->notify = 1;
    if (tcgetattr(STDIN_FILENO, &job->tty_modes) == 0) {
      job->has_tty_modes = 1;
    }
  }

  while (tcsetattr(STDIN_FILENO, TCSADRAIN, &xd_sh_tty_modes) == -1) {
    if (errno == EINTR) {
      continue;
    }
    break;
  }

  return exit_status;
}  // xd_fg()

/**
 * @brief Prints usage information for the `bg` builtin.
 */
static void xd_bg_usage() {
  fprintf(stderr, "bg: usage: bg [job_spec ...]\n");
}  // xd_bg_usage()

/**
 * @brief Prints detailed help information for the `bg` builtin.
 */
static void xd_bg_help() {
  printf(
      "bg: bg [job_spec ...]\n"
      "    Move jobs to the background.\n"
      "\n"
      "    Place jobs identified by job_spec's in background, as if they\n"
      "    started with `&`. If job_spec is not present, the shell's notion\n"
      "    of the current job is used.\n"
      "\n"
      "    Exit Status:\n"
      "    Success unless job control is not enabled or an error occurs.\n");
}  // xd_bg_help()

/**
 * @brief Executor of `bg` builtin command.
 */
static int xd_bg(int argc, char **argv) {
  if (!xd_sh_is_interactive || !isatty(STDIN_FILENO) || getpid() != xd_sh_pid) {
    fprintf(stderr, "xd-shell: bg: no job control\n");
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      xd_bg_help();
      return EXIT_SUCCESS;
    }
  }

  int opt;
  while ((opt = getopt(argc, argv, "")) != -1) {
    switch (opt) {
      case '?':
      default:
        fprintf(stderr, "xd-shell: bg: -%c: invalid option\n",
                optopt != 0 ? optopt : '?');
        xd_bg_usage();
        return XD_SH_EXIT_CODE_USAGE;
    }
  }

  int success_count = 0;
  int idx = 1;
  do {
    const char *operand = argv[idx];
    xd_job_t *job = NULL;

    if (operand == NULL) {
      job = xd_jobs_get_current();
    }
    else if (*operand == '%') {
      if (strcmp(operand, "%%") == 0 || strcmp(operand, "%+") == 0) {
        job = xd_jobs_get_current();
      }
      else if (strcmp(operand, "%-") == 0) {
        job = xd_jobs_get_previous();
      }
      else {
        long job_id = -1;
        xd_utils_strtol(operand + 1, &job_id);
        job = xd_jobs_get_with_id((int)job_id);
      }
    }

    if (job == NULL || !xd_job_is_alive(job)) {
      fprintf(stderr, "xd-shell: bg: %s: no such job\n",
              operand == NULL ? "current" : operand);
      continue;
    }

    if (!xd_job_is_stopped(job)) {
      fprintf(stderr, "xd-shell: bg: job %d already in background\n",
              job->job_id);
      success_count++;
      continue;
    }

    if (kill(-job->pgid, SIGCONT) == -1) {
      fprintf(stderr, "xd-shell: bg: %s: %s\n",
              operand == NULL ? "current" : operand, strerror(errno));
      continue;
    }

    xd_jobs_wait_non_blocking(job);
    job->notify = 1;
    job->is_background = 1;

    success_count++;
  } while (++idx < argc);

  return success_count == argc - 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}  // xd_bg()

/**
 * @brief Prints usage information for the `alias` builtin.
 */
static void xd_alias_usage() {
  fprintf(stderr, "alias: usage: alias [name[=value] ... ]\n");
}  // xd_alias_usage()

/**
 * @brief Prints detailed help information for the `alias` builtin.
 */
static void xd_alias_help() {
  printf(
      "alias: alias [name[=value] ... ]\n"
      "    Define or display aliases.\n"
      "\n"
      "    Without arguments, it prints the list of aliases in the reusable\n"
      "    form `alias name=value` to standard output\n"
      "    Otherwise, an alias is defined for each name whose value is given.\n"
      "\n"
      "    Exit Status:\n"
      "    Returns success unless invalid option is given or error occurs.\n");
}  // xd_alias_help()

/**
 * @brief Executor of `alias` builtin command.
 */
static int xd_alias(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      xd_alias_help();
      return EXIT_SUCCESS;
    }
  }

  int opt;
  while ((opt = getopt(argc, argv, "")) != -1) {
    switch (opt) {
      case '?':
      default:
        fprintf(stderr, "xd-shell: alias: -%c: invalid option\n",
                optopt != 0 ? optopt : '?');
        xd_alias_usage();
        return XD_SH_EXIT_CODE_USAGE;
    }
  }

  if (argc == 1) {
    xd_aliases_print_all();
    return EXIT_SUCCESS;
  }

  int success_count = 0;
  for (int i = 1; i < argc; i++) {
    char *name = argv[i];
    char *value = NULL;
    char *equal = strchr(name, '=');
    if (equal == NULL) {
      value = xd_aliases_get(name);
      if (value == NULL) {
        fprintf(stderr, "xd-shell: alias: %s: not found\n", name);
        continue;
      }
      printf("alias %s='%s'\n", name, value);
      success_count++;
      continue;
    }

    *equal = '\0';
    value = equal + 1;

    if (!xd_aliases_is_valid_name(name)) {
      fprintf(stderr, "xd-shell: alias: %s: invalid alias name\n", name);
      continue;
    }

    xd_aliases_put(name, value);
    success_count++;
  }

  return success_count == argc - 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}  // xd_alias()

/**
 * @brief Prints usage information for the `unalias` builtin.
 */
static void xd_unalias_usage() {
  fprintf(stderr, "unalias: usage: unalias [-a] name [name ...]\n");
}  // xd_unalias_usage()

/**
 * @brief Prints detailed help information for the `unalias` builtin.
 */
static void xd_unalias_help() {
  printf(
      "unalias: unalias [-a] name [name ...]\n"
      "    Remove each name from the list of defined aliases.\n"
      "\n"
      "    Options:\n"
      "      -a        remove all alias definitions\n"
      "\n"
      "    Exit Status:\n"
      "    Returns success unless invalid option is given or error occurs.\n");
}  // xd_unalias_help()

/**
 * @brief Executor of `unalias` builtin command.
 */
static int xd_unalias(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      xd_unalias_help();
      return EXIT_SUCCESS;
    }
  }

  int clear_all = 0;
  int opt;
  while ((opt = getopt(argc, argv, "a")) != -1) {
    switch (opt) {
      case 'a':
        clear_all = 1;
        break;
      case '?':
      default:
        fprintf(stderr, "xd-shell: unalias: -%c: invalid option\n",
                optopt != 0 ? optopt : '?');
        xd_unalias_usage();
        return XD_SH_EXIT_CODE_USAGE;
    }
  }

  if (argc == 1) {
    xd_unalias_usage();
    return XD_SH_EXIT_CODE_USAGE;
  }

  if (clear_all) {
    xd_aliases_clear();
    return EXIT_SUCCESS;
  }

  int operand_count = argc - optind;
  int success_count = 0;
  for (int i = optind; i < argc; i++) {
    char *name = argv[i];

    if (!xd_aliases_is_valid_name(name)) {
      fprintf(stderr, "xd-shell: unalias: %s: invalid alias name\n", name);
      continue;
    }

    if (xd_aliases_remove(name) == -1) {
      fprintf(stderr, "xd-shell: unalias: %s: not found\n", name);
      continue;
    }
    success_count++;
  }

  return success_count == operand_count ? EXIT_SUCCESS : EXIT_FAILURE;
}  // xd_unalias()

/**
 * @brief Prints usage information for the `set` builtin.
 */
static void xd_set_usage() {
  fprintf(stderr, "set: usage: set [name[=value] ... ]\n");
}  // xd_set_usage()

/**
 * @brief Prints detailed help information for the `set` builtin.
 */
static void xd_set_help() {
  printf(
      "set: set [name[=value] ... ]\n"
      "    Define or display variables.\n"
      "\n"
      "    Without arguments, it prints the list of variables in the reusable\n"
      "    form `set name=value` to standard output\n"
      "    Otherwise, a variable is defined for each name whose value is "
      "given.\n"
      "\n"
      "    Exit Status:\n"
      "    Returns success unless invalid option is given or error occurs.\n");
}  // xd_set_help()

/**
 * @brief Executor of `set` builtin command.
 */
static int xd_set(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      xd_set_help();
      return EXIT_SUCCESS;
    }
  }

  int opt;
  while ((opt = getopt(argc, argv, "")) != -1) {
    switch (opt) {
      case '?':
      default:
        fprintf(stderr, "xd-shell: set: -%c: invalid option\n",
                optopt != 0 ? optopt : '?');
        xd_set_usage();
        return XD_SH_EXIT_CODE_USAGE;
    }
  }

  if (argc == 1) {
    xd_vars_print_all();
    return EXIT_SUCCESS;
  }

  int success_count = 0;
  for (int i = 1; i < argc; i++) {
    char *name = argv[i];
    char *value = NULL;
    char *equal = strchr(name, '=');
    if (equal == NULL) {
      value = xd_vars_get(name);
      if (value == NULL) {
        fprintf(stderr, "xd-shell: set: %s: not found\n", name);
        continue;
      }
      printf("set %s='%s'\n", name, value);
      success_count++;
      continue;
    }

    *equal = '\0';
    value = equal + 1;

    if (!xd_vars_is_valid_name(name)) {
      fprintf(stderr, "xd-shell: set: %s: invalid variable name\n", name);
      continue;
    }

    int is_exported = xd_vars_is_exported(name);
    xd_vars_put(name, value, is_exported);
    success_count++;
  }

  return success_count == argc - 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}  // xd_set()

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
