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

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "xd_aliases.h"
#include "xd_command.h"
#include "xd_job.h"
#include "xd_jobs.h"
#include "xd_vars.h"

// ========================
// Function Declarations
// ========================

static void xd_sh_init() __attribute__((constructor));
static void xd_sh_destroy() __attribute__((destructor));
static void xd_sh_sigint_handler(int signum);
static void xd_sh_sigchld_handler(int signum);
static int xd_sh_setup_signal_handlers();
static int xd_sh_run();

// flex and bison functions

extern void yyparse_initialize();
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
pid_t xd_sh_pid = 0;
pid_t xd_sh_pgid = 0;
volatile sig_atomic_t xd_sh_readline_running = 0;
int xd_sh_last_exit_code = 0;
pid_t xd_sh_last_bg_job_pid = 0;
struct termios xd_sh_tty_modes = {0};

// ========================
// Function Definitions
// ========================

/**
 * @brief Constructor, runs before main to initialize the shell.
 */
static void xd_sh_init() {
  xd_sh_is_interactive = (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO));
  pid_t pid = getpid();
  pid_t pgid = getpgrp();

  if (xd_sh_setup_signal_handlers() == -1) {
    fprintf(stderr, "xd-shell: failed to setup signal handlers\n");
    exit(EXIT_FAILURE);
  }

  if (xd_sh_is_interactive) {
    // wait until the shell process group is in foreground
    while (tcgetpgrp(STDIN_FILENO) != (pgid = getpgrp())) {
      kill(-pgid, SIGTTIN);
    }

    // put the shell in its own process group if needed
    if (pid != pgid) {
      if (setpgid(pid, pid) == -1) {
        fprintf(stderr, "xd-shell: setpgid: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
    }

    // ensure the shell controls the terminal
    pgid = getpgrp();
    if (tcsetpgrp(STDIN_FILENO, pgid) == -1) {
      fprintf(stderr, "xd-shell: tcsetpgrp: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    strncpy(xd_sh_prompt, "\e[0;94mxd-shell\e[0m$ ", XD_SH_PROMPT_MAX_LENGTH);

    if (tcgetattr(STDIN_FILENO, &xd_sh_tty_modes) == -1) {
      fprintf(stderr, "xd_readline: failed to get tty attributes\n");
      exit(EXIT_FAILURE);
    }
  }

  xd_sh_pid = pid;
  xd_sh_pgid = pgid;
  xd_jobs_init();
  xd_aliases_init();
  xd_vars_init();
  yyparse_initialize();
}  // xd_sh_init()

/**
 * @brief Destructor, runs before exit to cleanup after the shell.
 */
static void xd_sh_destroy() {
  yyparse_cleanup();
  xd_jobs_destroy();
  xd_aliases_destroy();
  xd_vars_destroy();
}  // xd_sh_destroy()

/**
 * @brief Handles `SIGINT` signal.
 *
 * @param signum The signal number.
 */
static void xd_sh_sigint_handler(int signum) {
  (void)signum;
  int saved_errno = errno;
  if (xd_sh_readline_running) {
    write(STDERR_FILENO, "^C", 2);
  }
  else {
    write(STDERR_FILENO, "\n", 1);
  }
  errno = saved_errno;
}  // xd_sh_sigint_handler()

static void xd_sh_sigchld_handler(int signum) {
  (void)signum;
  int status;
  pid_t pid;
  int saved_errno = errno;
  while (1) {
    pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
    if (pid <= 0) {
      if (pid == -1 && errno == EINTR) {
        continue;
      }
      break;
    }

    xd_job_t *job = xd_jobs_get_with_pid(pid);
    xd_command_t *command = xd_job_get_command_with_pid(job, pid);
    if (job == NULL || command == NULL) {
      continue;
    }

    int was_stopped = WIFSTOPPED(command->wait_status);
    command->wait_status = status;

    if (job->commands[job->command_count - 1] == command) {
      job->wait_status = status;
    }

    if (WIFCONTINUED(status)) {
      if (was_stopped) {
        job->stopped_count--;
      }
    }
    else if (WIFSTOPPED(status)) {
      if (!was_stopped) {
        job->stopped_count++;
      }
    }
    else if (WIFEXITED(status) || WIFSIGNALED(status)) {
      if (was_stopped) {
        job->stopped_count--;
      }
      job->unreaped_count--;
    }

    if (!xd_job_is_alive(job) || xd_job_is_stopped(job)) {
      job->notify = 1;
    }

    struct timespec time_spec;
    clock_gettime(CLOCK_MONOTONIC, &time_spec);
    job->last_active =
        (uint64_t)time_spec.tv_sec * XD_SH_NANOSECONDS_PER_SECOND +
        (uint64_t)time_spec.tv_nsec;
  }
  errno = saved_errno;
}  // xd_sh_sigchld_handler()

/**
 * @brief Sets up the signal handlers.
 *
 * @return `0` on success, `-1` on failure.
 */
static int xd_sh_setup_signal_handlers() {
  struct sigaction signal_action;
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_flags = 0;

  if (xd_sh_is_interactive) {
    signal_action.sa_handler = SIG_IGN;
    if (sigaction(SIGTERM, &signal_action, NULL) == -1) {
      return -1;
    }
    if (sigaction(SIGQUIT, &signal_action, NULL) == -1) {
      return -1;
    }
    if (sigaction(SIGTSTP, &signal_action, NULL) == -1) {
      return -1;
    }
    if (sigaction(SIGTTIN, &signal_action, NULL) == -1) {
      return -1;
    }
    if (sigaction(SIGTTOU, &signal_action, NULL) == -1) {
      return -1;
    }
    signal_action.sa_handler = xd_sh_sigint_handler;
    if (sigaction(SIGINT, &signal_action, NULL) == -1) {
      return -1;
    }
  }

  signal_action.sa_flags = SA_RESTART;
  signal_action.sa_handler = xd_sh_sigchld_handler;
  if (sigaction(SIGCHLD, &signal_action, NULL) == -1) {
    return -1;
  }
  return 0;
}  // xd_sh_setup_signal_handlers()

/**
 * @brief Runs the shell.
 *
 * @return returns the shell's exit code.
 */
static int xd_sh_run() {
  yyparse();
  return xd_sh_last_exit_code;
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
