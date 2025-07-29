/*
 * ==============================================================================
 * File: xd_job_executor.c
 * Author: Duraid Maihoub
 * Date: 28 July 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xd_command.h"
#include "xd_job.h"
#include "xd_jobs.h"
#include "xd_shell.h"

// ========================
// Macros
// ========================

/**
 * @brief Default access mode for created files.
 */
#define XD_FILE_ACCESS_MODE (0664)

// ========================
// Function Declarations
// ========================

static int xd_reset_signal_handlers();

static int xd_redirect_input();
static int xd_redirect_output();
static int xd_redirect_error();

static void xd_execute_command();

static void xd_failure_cleanup();

// ========================
// Variables
// ========================

/**
 * @brief The current job being executed.
 */
static xd_job_t *xd_job = NULL;

/**
 * @brief The current command being executed.
 */
static xd_command_t *xd_command = NULL;

/**
 * @brief The fd of the current pipe's read end.
 */
static int xd_pipe_read_fd = -1;

/**
 * @brief The fd of the current pipe's write end.
 */
static int xd_pipe_write_fd = -1;

/**
 * @brief The fd of the previous pipe's read end.
 */
static int xd_prev_pipe_read_fd = -1;

/**
 * @brief Indicates whether the command being executed is the first command in
 * the job (non-zero), or not (zero).
 */
static int xd_is_first_command = 0;

/**
 * @brief Indicates whether the command being executed is the last command in
 * the job (non-zero), or not (zero).
 */
static int xd_is_last_command = 0;

// ========================
// Function Definitions
// ========================

/**
 * @brief Resets the signal handlers changed by `xd-shell`'s main process to
 * the default action (`SIG_DFL`).
 *
 * @return `0` on sucess or `-1` on failure.
 */
static int xd_reset_signal_handlers() {
  if (signal(SIGTERM, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGQUIT, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGTSTP, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGTTIN, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGTTOU, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGCHLD, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  return 0;
}  // xd_reset_signal_handlers()

/**
 * @brief Handles `stdin` redirection for the current command to be executed
 * (`xd_command`).
 *
 * @return `0` on success or `-1` on failure.
 */
static int xd_redirect_input() {
  // setup fd
  int input_fd = -1;
  if (xd_command->input_file != NULL) {
    // input from file (input redirection)
    while ((input_fd = open(xd_command->input_file, O_RDONLY)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      fprintf(stderr, "xd-shell: %s: %s\n", xd_command->input_file,
              strerror(errno));
      return -1;
    }
    if (!xd_is_first_command) {
      // pipe input isn't needed
      close(xd_prev_pipe_read_fd);
    }
  }
  else if (!xd_is_first_command) {
    // input from pipe
    input_fd = xd_prev_pipe_read_fd;
  }
  else {
    // input from stdin (default)
    return 0;
  }

  // redirect
  int ret = 0;
  while (dup2(input_fd, STDIN_FILENO) == -1) {
    if (errno == EINTR) {
      continue;
    }
    fprintf(stderr, "xd-shell: dup2: %s\n", strerror(errno));
    ret = -1;
    break;
  }
  close(input_fd);
  return ret;
}  // xd_redirect_input()

/**
 * @brief Handles `stdout` redirection for the current command to be executed
 * (`xd_command`).
 *
 * @return `0` on success or `-1` on failure.
 */
static int xd_redirect_output() {
  // setup fd
  int output_fd = -1;
  if (xd_command->output_file != NULL) {
    // output to file (output redirection)
    int flags = O_WRONLY | O_CREAT;
    flags |= (xd_command->append_output ? O_APPEND : O_TRUNC);
    while ((output_fd = open(xd_command->output_file, flags,
                             XD_FILE_ACCESS_MODE)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      fprintf(stderr, "xd-shell: %s: %s\n", xd_command->output_file,
              strerror(errno));
      return -1;
    }
    if (!xd_is_last_command) {
      // pipe output isn't needed
      close(xd_pipe_write_fd);
    }
  }
  else if (!xd_is_last_command) {
    // output to pipe
    output_fd = xd_pipe_write_fd;
  }
  else {
    // output to stdout (default)
    return 0;
  }

  // redirect
  int ret = 0;
  while (dup2(output_fd, STDOUT_FILENO) == -1) {
    if (errno == EINTR) {
      continue;
    }
    fprintf(stderr, "xd-shell: dup2: %s\n", strerror(errno));
    ret = -1;
    break;
  }
  close(output_fd);
  return ret;
}  // xd_redirect_output()

/**
 * @brief Handles `stderr` redirection for the current command to be executed
 * (`xd_command`).
 *
 * @return `0` on success or `-1` on failure.
 */
static int xd_redirect_error() {
  if (xd_command->error_file != NULL && xd_command->output_file != NULL &&
      strcmp(xd_command->error_file, xd_command->output_file) == 0) {
    // error file is same as output file
    while (dup2(STDOUT_FILENO, STDERR_FILENO) == -1) {
      if (errno == EINTR) {
        continue;
      }
      fprintf(stderr, "xd-shell: dup2: %s\n", strerror(errno));
      return -1;
    }
    return 0;
  }

  // setup fd
  int error_fd = -1;
  if (xd_command->error_file != NULL) {
    // error to file (error redirection)
    int flags = O_WRONLY | O_CREAT;
    flags |= (xd_command->append_error ? O_APPEND : O_TRUNC);

    while ((error_fd = open(xd_command->error_file, flags,
                            XD_FILE_ACCESS_MODE)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      fprintf(stderr, "xd-shell: %s: %s\n", xd_command->error_file,
              strerror(errno));
      return -1;
    }
  }
  else {
    // error to stderr (default)
    return 0;
  }

  // redirect
  int ret = 0;
  while (dup2(error_fd, STDERR_FILENO) == -1) {
    if (errno == EINTR) {
      continue;
    }
    fprintf(stderr, "xd-shell: dup2: %s\n", strerror(errno));
    ret = -1;
    break;
  }
  close(error_fd);
  return ret;
}  // xd_redirect_error()

/**
 * @brief Executes the current command (`xd_command`).
 */
static void xd_execute_command() {
  if (xd_sh_is_interactive) {
    // interactive shell setup process group
    xd_command->pid = getpid();
    if (xd_job->pgid == 0) {
      xd_job->pgid = xd_command->pid;
    }
    if (setpgid(xd_command->pid, xd_job->pgid) == -1) {
      fprintf(stderr, "xd-shell: setpgid: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (!xd_job->is_background) {
      if (xd_jobs_put_in_foreground(xd_job->pgid) == -1) {
        exit(EXIT_FAILURE);
      }
    }
  }

  if (xd_reset_signal_handlers() == -1) {
    fprintf(stderr, "xd-shell: failed to reset signal handlers\n");
    exit(EXIT_FAILURE);
  }

  if (!xd_is_last_command) {
    close(xd_pipe_read_fd);
  }

  if (xd_redirect_input() == -1) {
    exit(EXIT_FAILURE);
  }
  if (xd_redirect_output() == -1) {
    exit(EXIT_FAILURE);
  }
  if (xd_redirect_error() == -1) {
    exit(EXIT_FAILURE);
  }

  execvp(xd_command->argv[0], xd_command->argv);
  fprintf(stderr, "xd-shell: %s: %s\n", xd_command->argv[0], strerror(errno));
  exit(EXIT_FAILURE);
}  // xd_execute_command()

/**
 * @brief Performs cleanup actions after a job execution failure.
 */
static void xd_failure_cleanup() {
  if (xd_prev_pipe_read_fd != -1) {
    close(xd_prev_pipe_read_fd);
  }
  if (xd_pipe_read_fd != -1) {
    close(xd_pipe_read_fd);
  }
  if (xd_pipe_write_fd != -1) {
    close(xd_pipe_write_fd);
  }
  xd_jobs_kill(xd_job);
  xd_jobs_wait(xd_job);
  if (xd_sh_is_interactive) {
    xd_jobs_put_in_foreground(xd_sh_pgid);
  }
}  // xd_failure_cleanup()

// ========================
// Public Functions
// ========================

void xd_job_executor(xd_job_t *job) {
  xd_job = job;

  xd_job->pgid = 0;
  xd_pipe_read_fd = -1;
  xd_pipe_write_fd = -1;
  int pipe_fd[2] = {-1, -1};

  for (int i = 0; i < xd_job->command_count; i++) {
    xd_command = xd_job->commands[i];
    xd_command->pid = 0;
    xd_is_first_command = (i == 0);
    xd_is_last_command = (i == xd_job->command_count - 1);
    xd_prev_pipe_read_fd = xd_pipe_read_fd;

    // create a pipe between each two consecutive commands in the job
    if (i < xd_job->command_count - 1) {
      if (pipe(pipe_fd) == -1) {
        fprintf(stderr, "xd-shell: pipe: %s\n", strerror(errno));
        xd_failure_cleanup();
        return;
      }
      xd_pipe_read_fd = pipe_fd[0];
      xd_pipe_write_fd = pipe_fd[1];
    }

    // fork a child process to execute the current command
    pid_t child_pid = fork();
    if (child_pid == -1) {
      fprintf(stderr, "xd-shell: fork: %s\n", strerror(errno));
      xd_failure_cleanup();
      return;
    }

    if (child_pid == 0) {
      xd_execute_command();  // This calls `exit()`
    }

    xd_command->pid = child_pid;
    xd_job->unreaped_count++;

    if (xd_sh_is_interactive) {
      if (xd_job->pgid == 0) {
        xd_job->pgid = child_pid;
      }
      setpgid(xd_command->pid, xd_job->pgid);
    }
    else {
      xd_job->pgid = xd_sh_pgid;
    }

    if (!xd_is_first_command) {
      close(xd_prev_pipe_read_fd);
    }
    if (!xd_is_last_command) {
      close(xd_pipe_write_fd);
    }
  }  // for()

  if (!xd_job->is_background) {
    if (xd_sh_is_interactive) {
      xd_jobs_put_in_foreground(xd_job->pgid);
      xd_jobs_wait(xd_job);
      xd_jobs_put_in_foreground(xd_sh_pgid);
    }
    else {
      xd_jobs_wait(xd_job);
    }
  }
}  // xd_job_executor()
