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

#include "xd_command.h"
#include "xd_job_executor.h"

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

void xd_job_execute(xd_job_t *job) {
  (void)job;
#ifndef XD_TESTING_MODE
  xd_job_executor(job);
#endif  // XD_TESTING_MODE
}  // xd_job_execute()
