/*
 * ==============================================================================
 * File: xd_job.c
 * Author: Duraid Maihoub
 * Date: 18 July 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#include "xd_job.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "xd_command.h"
#include "xd_job_executor.h"

// ========================
// Macros
// ========================

/**
 * @brief Size of a small buffer used to format the status when printing the
 * notifications.
 */
#define XD_JOBS_STATUS_BUFFER_SIZE (42)

// ========================
// Function Declarations
// ========================

static void xd_print_status_detailed(xd_job_t *job, char marker, int print_pid);

// ========================
// Function Definitions
// ========================

/**
 * @brief Prints the detailed status of the passed `xd_job_t` structure.
 *
 * @param job A pointer to the `xd_job_t` structure to print its detailed
 * status.
 * @param marker A character to be printed after the job id, could be `'+'` for
 * the most recent job, `'-'` for the second most recent job, or `' '`
 * otherwise.
 * @param print_pid Whether to print the job PID or not.
 */
static void xd_print_status_detailed(xd_job_t *job, char marker,
                                     int print_pid) {
  char state_buf[XD_JOBS_STATUS_BUFFER_SIZE];
  for (int i = 0; i < job->command_count; i++) {
    xd_command_t *command = job->commands[i];

    if (WIFSTOPPED(command->wait_status)) {
      snprintf(state_buf, sizeof(state_buf), "Stopped");
    }
    else if (WIFSIGNALED(command->wait_status)) {
      int termsig = WTERMSIG(command->wait_status);
      if (WCOREDUMP(command->wait_status)) {
        snprintf(state_buf, sizeof(state_buf), "%s (core dumped)",
                 strsignal(termsig));
      }
      else {
        snprintf(state_buf, sizeof(state_buf), "%s", strsignal(termsig));
      }
    }
    else if (WIFEXITED(command->wait_status)) {
      int exit_code = WEXITSTATUS(command->wait_status);
      if (exit_code == 0) {
        snprintf(state_buf, sizeof(state_buf), "Done");
      }
      else {
        snprintf(state_buf, sizeof(state_buf), "Exit %d", exit_code);
      }
    }
    else {
      snprintf(state_buf, sizeof(state_buf), "Running");
    }

    if (i == 0) {
      printf("[%d]%c  ", job->job_id, marker);
    }
    else {
      printf("      ");
    }
    if (print_pid) {
      printf("%u ", command->pid);
    }
    printf("%-42s %s %s", state_buf, i > 0 ? "|" : " ", command->str);
    if (i == job->command_count - 1 && xd_job_is_alive(job) &&
        !xd_job_is_stopped(job)) {
      printf(" &");
    }
    printf("\n");
  }
}  // xd_print_status_detailed()

// ========================
// Public Functions
// ========================

xd_job_t *xd_job_create() {
  xd_job_t *job = (xd_job_t *)malloc(sizeof(xd_job_t));
  if (job == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  job->commands = NULL;
  job->command_count = 0;
  job->is_background = 0;
  job->pgid = 0;
  job->stopped_count = 0;
  job->unreaped_count = 0;
  job->wait_status = -1;
  job->job_id = -1;
  job->notify = 0;

  return job;
}  // xd_job_create()

void xd_job_destroy(xd_job_t *job) {
  if (job == NULL) {
    return;
  }
  for (int i = 0; i < job->command_count; i++) {
    xd_command_destroy(job->commands[i]);
  }
  free((void *)job->commands);
  free(job);
}  // xd_job_destroy()

int xd_job_add_command(xd_job_t *job, xd_command_t *command) {
  if (job == NULL || command == NULL) {
    return -1;
  }

  int new_command_count = job->command_count + 1;
  xd_command_t **new_commands = (xd_command_t **)realloc(
      (void *)job->commands, sizeof(xd_command_t *) * new_command_count);
  if (new_commands == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  new_commands[new_command_count - 1] = command;

  job->command_count = new_command_count;
  job->commands = new_commands;

  return 0;
}  // xd_job_add_command()

xd_command_t *xd_job_get_command_with_pid(const xd_job_t *job, pid_t pid) {
  if (job == NULL) {
    return NULL;
  }
  for (int i = 0; i < job->command_count; i++) {
    if (job->commands[i]->pid == pid) {
      return job->commands[i];
    }
  }
  return NULL;
}  // xd_job_get_command_with_pid()

int xd_job_is_stopped(const xd_job_t *job) {
  if (job == NULL) {
    return 0;
  }
  return job->stopped_count > 0 && job->stopped_count == job->unreaped_count;
}  // xd_job_is_stopped()

int xd_job_is_alive(const xd_job_t *job) {
  if (job == NULL) {
    return 0;
  }
  return job->unreaped_count > 0;
}  // xd_job_is_alive()

void xd_job_print_status(xd_job_t *job, char marker, int detailed,
                         int print_pid) {
  if (job == NULL) {
    return;
  }
  if (detailed) {
    xd_print_status_detailed(job, marker, print_pid);
    return;
  }

  char state_buf[XD_JOBS_STATUS_BUFFER_SIZE];
  if (xd_job_is_stopped(job)) {
    snprintf(state_buf, sizeof(state_buf), "Stopped");
  }
  else if (!xd_job_is_alive(job)) {
    if (WIFSIGNALED(job->wait_status)) {
      int termsig = WTERMSIG(job->wait_status);
      if (WCOREDUMP(job->wait_status)) {
        snprintf(state_buf, sizeof(state_buf), "%s (core dumped)",
                 strsignal(termsig));
      }
      else {
        snprintf(state_buf, sizeof(state_buf), "%s", strsignal(termsig));
      }
    }
    else if (WIFEXITED(job->wait_status)) {
      int exit_code = WEXITSTATUS(job->wait_status);
      if (exit_code == 0) {
        snprintf(state_buf, sizeof(state_buf), "Done");
      }
      else {
        snprintf(state_buf, sizeof(state_buf), "Exit %d", exit_code);
      }
    }
  }
  else {
    snprintf(state_buf, sizeof(state_buf), "Running");
  }

  printf("[%d]%c  ", job->job_id, marker);
  if (print_pid) {
    printf("%u ", job->commands[0]->pid);
  }
  printf("%-42s", state_buf);

  for (int i = 0; i < job->command_count; i++) {
    if (i > 0) {
      printf(" |");
    }
    printf(" %s", job->commands[i]->str);
  }
  if (job->is_background && xd_job_is_alive(job) && !xd_job_is_stopped(job)) {
    printf(" &");
  }
  printf("\n");
}  // xd_job_print_status()

void xd_job_execute(xd_job_t *job) {
  (void)job;
#ifndef XD_TESTING_MODE
  xd_job_executor(job);
#endif  // XD_TESTING_MODE
}  // xd_job_execute()
