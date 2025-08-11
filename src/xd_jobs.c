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
#include <time.h>
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
static int xd_job_is_newer(const xd_job_t *job1, const xd_job_t *job2);

static void xd_notify_status_change();
static void xd_remove_finished();
static void xd_update_current_job();

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

/**
 * @brief Current job (`+`).
 */
static xd_job_t *xd_current_job = NULL;

/**
 * @brief Previous job (`-`).
 */
static xd_job_t *xd_previous_job = NULL;

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
 * @brief Used while updating the current (`+`) and previous (`-`) jobs, to
 * check whether the first passed job is newer than the second passed job.
 *
 * @param job1 The first job, to check if newer.
 * @param job2 The second job, to check if older.
 *
 * @return `1` if the first passed job is newer than the second passed job, or
 * `0` otherwise.
 */
static int xd_job_is_newer(const xd_job_t *job1, const xd_job_t *job2) {
  if (job1 == NULL) {
    return 0;
  }
  if (job2 == NULL) {
    return 1;
  }

  int job1_stopped = xd_job_is_stopped(job1);
  int job2_stopped = xd_job_is_stopped(job2);

  if (job1_stopped != job2_stopped) {
    return job1_stopped;
  }
  if (job1->last_active != job2->last_active) {
    return job1->last_active > job2->last_active;
  }
  return job1->job_id > job2->job_id;
}  // xd_job_is_newer()

static void xd_notify_status_change() {
  if (xd_jobs == NULL) {
    return;
  }

  for (xd_list_node_t *node = xd_jobs->head; node != NULL; node = node->next) {
    xd_job_t *job = node->data;
    if (!job->notify) {
      continue;
    }
    char marker = ' ';
    if (job == xd_current_job) {
      marker = '+';
    }
    else if (job == xd_previous_job) {
      marker = '-';
    }
    xd_job_print_status(job, marker, 0, 0);
    job->notify = 0;
  }
}  // xd_notify_status_change()

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

/**
 * @brief Updates the current (`+`) and previous jobs (`-`).
 */
static void xd_update_current_job() {
  if (xd_jobs == NULL) {
    return;
  }

  xd_job_t *first = NULL;
  xd_job_t *second = NULL;
  for (xd_list_node_t *node = xd_jobs->head; node != NULL; node = node->next) {
    xd_job_t *job = node->data;
    if (!xd_job_is_alive(job)) {
      continue;
    }

    if (xd_job_is_newer(job, first)) {
      second = first;
      first = job;
    }
    else if (job != first && xd_job_is_newer(job, second)) {
      second = job;
    }
  }
  xd_current_job = first;
  xd_previous_job = second;
}  // xd_update_current_job()

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

xd_job_t *xd_jobs_get_current() {
  if (xd_jobs == NULL) {
    return NULL;
  }
  return xd_current_job;
}  // xd_jobs_get_current()

xd_job_t *xd_jobs_get_previous() {
  if (xd_jobs == NULL) {
    return NULL;
  }
  return xd_previous_job;
}  // xd_jobs_get_previous()

void xd_jobs_print_status_all(int detailed, int print_pids) {
  if (xd_jobs == NULL) {
    return;
  }

  for (xd_list_node_t *node = xd_jobs->head; node != NULL; node = node->next) {
    xd_job_t *job = node->data;
    char marker = ' ';
    if (job == xd_current_job) {
      marker = '+';
    }
    else if (job == xd_previous_job) {
      marker = '-';
    }
    xd_job_print_status(job, marker, detailed, print_pids);
    job->notify = 0;
  }
}  // xd_jobs_print_status_all()

void xd_jobs_refresh() {
  if (xd_jobs == NULL) {
    return;
  }
  if (xd_sh_is_interactive) {
    xd_notify_status_change();
  }
  xd_remove_finished();
  xd_update_current_job();
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

int xd_jobs_kill(xd_job_t *job, int signum) {
  if (job == NULL) {
    return -1;
  }
  for (int i = 0; i < job->command_count; i++) {
    pid_t pid = job->commands[i]->pid;
    if (pid == 0) {
      continue;  // failure before fork in xd_job_executor()
    }
    if (kill(pid, signum) == -1) {
      fprintf(stderr, "xd-shell: kill: %s\n", strerror(errno));
      return -1;
    }
  }
  return 0;
}  // xd_jobs_kill()

int xd_jobs_wait(xd_job_t *job) {
  if (job == NULL) {
    return EXIT_FAILURE;
  }

  int status;
  while (xd_job_is_alive(job) && !xd_job_is_stopped(job)) {
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

  struct timespec time_spec;
  clock_gettime(CLOCK_MONOTONIC, &time_spec);
  job->last_active = (uint64_t)time_spec.tv_sec * XD_SH_NANOSECONDS_PER_SECOND +
                     (uint64_t)time_spec.tv_nsec;

  int exit_code = EXIT_SUCCESS;
  if (WIFEXITED(job->wait_status)) {
    exit_code = WEXITSTATUS(job->wait_status);
  }
  else if (WIFSIGNALED(job->wait_status)) {
    exit_code = XD_SH_EXIT_CODE_SIGNAL_OFFSET + WTERMSIG(job->wait_status);
  }
  else if (WIFSTOPPED(job->wait_status)) {
    exit_code = XD_SH_EXIT_CODE_SIGNAL_OFFSET + WSTOPSIG(job->wait_status);
  }

  if (!xd_sh_is_interactive) {
    return exit_code;
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
  return exit_code;
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
