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
#include "xd_list.h"
#include "xd_shell.h"

// ========================
// Macros
// ========================

// ========================
// Function Declarations
// ========================

static void *xd_job_copy_func(void *data);
static void xd_job_destroy_func(void *data);
static int xd_job_comp_func(const void *data1, const void *data2);

static void xd_remove_finished();

// ========================
// Variables
// ========================

/**
 * @brief Tracks the nesting depth of `SIGCHLD` blocking.
 */
static int xd_sigchld_block_count = 0;

/**
 * @brief List of jobs.
 */
static xd_list_t *xd_jobs = NULL;

// ========================
// Function Definitions
// ========================

/**
 * @brief Implementation of `xd_gens_copy_func` for `xd_job_t` objects.
 */
static void *xd_job_copy_func(void *data) {
  return data;
}
// xd_job_copy_func()

/**
 * @brief Implementation of `xd_gens_destroy_func` for `xd_job_t` objects.
 */
static void xd_job_destroy_func(void *data) {
  xd_job_destroy(data);
}
// xd_job_destroy_func()

/**
 * @brief Implementation of `xd_gens_comp_func` for `xd_job_t` objects.
 */
static int xd_job_comp_func(const void *data1, const void *data2) {
  if (data1 == NULL && data2 == NULL) {
    return 0;
  }
  if (data1 == NULL) {
    return -1;
  }
  if (data2 == NULL) {
    return -1;
  }
  xd_job_t *job1 = (xd_job_t *)data1;
  xd_job_t *job2 = (xd_job_t *)data2;
  if (job1->job_id < job2->job_id) {
    return -1;
  }
  if (job1->job_id > job2->job_id) {
    return 1;
  }
  return 0;
}  // xd_job_comp_func()

/**
 * @brief Remove all finished jobs from the jobs list.
 */
static void xd_remove_finished() {
  if (xd_jobs == NULL) {
    return;
  }
  xd_list_node_t *curr = xd_jobs->head;
  xd_list_node_t *next = NULL;
  xd_job_t *curr_job = NULL;
  while (curr != NULL) {
    next = curr->next;
    curr_job = curr->data;
    if (curr_job->unreaped_count == 0) {
      xd_list_remove_node(xd_jobs, curr);
    }
    curr = next;
  }
}  // xd_remove_finished()

// ========================
// Public Functions
// ========================

void xd_jobs_init() {
  xd_jobs =
      xd_list_create(xd_job_copy_func, xd_job_destroy_func, xd_job_comp_func);
}  // xd_jobs_init()

void xd_jobs_destroy() {
  xd_list_destroy(xd_jobs);
}  // xd_jobs_destroy()

void xd_jobs_add(xd_job_t *job) {
  if (xd_jobs == NULL) {
    return;
  }

  int job_id = 1;
  if (xd_jobs->length > 0) {
    job_id = ((xd_job_t *)xd_jobs->tail->data)->job_id + 1;
  }
  job->job_id = job_id;
  xd_list_add_last(xd_jobs, job);
}  // xd_jobs_add()

xd_job_t *xd_jobs_get_with_pid(pid_t pid) {
  if (xd_jobs == NULL) {
    return NULL;
  }

  for (xd_list_node_t *node = xd_jobs->head; node != NULL; node = node->next) {
    xd_job_t *job = node->data;
    if (xd_job_get_command_with_pid(job, pid) != NULL) {
      return job;
    }
  }
  return NULL;
}  // xd_jobs_get_with_pid()

xd_job_t *xd_jobs_get_with_id(int job_id) {
  if (xd_jobs == NULL) {
    return NULL;
  }

  for (xd_list_node_t *node = xd_jobs->head; node != NULL; node = node->next) {
    xd_job_t *job = node->data;
    if (job->job_id == job_id) {
      return job;
    }
  }
  return NULL;
}  // xd_jobs_get_with_id()

void xd_jobs_refresh() {
  if (xd_jobs == NULL) {
    return;
  }
  xd_remove_finished();
}  // xd_jobs_refresh()

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

  if (xd_job_is_stopped(job)) {
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
