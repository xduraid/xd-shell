/*
 * ==============================================================================
 * File: xd_jobs.c
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

#include "xd_jobs.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "xd_command.h"
#include "xd_job.h"
#include "xd_shell.h"

// ========================
// Macros
// ========================

// ========================
// Function Declarations
// ========================

// ========================
// Variables
// ========================

/**
 * @brief Tracks the nesting depth of `SIGCHLD` blocking.
 */
static int xd_sigchld_block_count = 0;

// ========================
// Function Definitions
// ========================

// ========================
// Public Functions
// ========================

int xd_jobs_put_in_foreground(pid_t pgid) {
  if (!xd_sh_is_interactive) {
    return -1;
  }
  if (tcsetpgrp(STDIN_FILENO, pgid) == -1) {
    fprintf(stderr, "xd-shell: tcsetpgrp: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}  // xd_jobs_put_in_foreground()

void xd_jobs_kill(xd_job_t *job) {
  if (job == NULL) {
    return;
  }
  for (int i = 0; i < job->command_count; i++) {
    pid_t pid = job->commands[i]->pid;
    if (pid == 0) {
      continue;  // failure before fork in xd_job_executor()
    }
    kill(pid, SIGKILL);
  }
}  // xd_jobs_kill()

void xd_jobs_wait(xd_job_t *job) {
  if (job == NULL) {
    return;
  }

  int status;
  while (job->unreaped_count - job->stopped_count > 0) {
    for (int i = 0; i < job->command_count; i++) {
      xd_command_t *command = job->commands[i];
      pid_t pid = command->pid;
      if (pid == 0) {
        continue;  // failure before fork in xd_job_executor()
      }

      pid_t ret = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      if (ret <= 0) {
        if (ret < 0 && errno == EINTR) {
          i--;
        }
        continue;
      }

      int was_stopped = WIFSTOPPED(command->wait_status);
      command->wait_status = status;
      job->wait_status = status;

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
    }
  }

  if (!xd_sh_is_interactive) {
    return;
  }

  if (job->stopped_count > 0 && job->stopped_count == job->unreaped_count) {
    fprintf(stderr, "\n");
  }
  else if (WIFSIGNALED(job->wait_status)) {
    int termsig = WTERMSIG(job->wait_status);
    if (termsig != SIGINT) {
      fprintf(stderr, "%s", strsignal(termsig));
      if (WCOREDUMP(job->wait_status)) {
        fprintf(stderr, "%s", " (core dumped)");
      }
    }
    fprintf(stderr, "\n");
  }
}  // xd_jobs_wait()

void xd_jobs_sigchld_block() {
  if (xd_sigchld_block_count == 0) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);
  }
  xd_sigchld_block_count++;
}  // xd_jobs_sigchld_block()

void xd_jobs_sigchld_unblock() {
  if (xd_sigchld_block_count > 0) {
    xd_sigchld_block_count--;

    if (xd_sigchld_block_count == 0) {
      sigset_t mask;
      sigemptyset(&mask);
      sigaddset(&mask, SIGCHLD);
      sigprocmask(SIG_UNBLOCK, &mask, NULL);
    }
  }
}  // xd_jobs_sigchld_unblock()
