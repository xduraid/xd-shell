/*
 * ==============================================================================
 * File: xd_command.c
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

#include "xd_command.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========================
// Public Functions
// ========================

xd_command_t *xd_command_create() {
  xd_command_t *command = (xd_command_t *)malloc(sizeof(xd_command_t));
  if (command == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  command->argc = 0;
  command->argv = NULL;

  command->input_file = NULL;
  command->output_file = NULL;
  command->error_file = NULL;
  command->append_output = 0;
  command->append_error = 0;
  command->pid = 0;
  command->wait_status = -1;
  command->str = NULL;

  return command;
}  // xd_command_create()

void xd_command_destroy(xd_command_t *command) {
  if (command == NULL) {
    return;
  }
  free(command->input_file);
  free(command->output_file);
  free(command->error_file);
  for (int i = 0; i < command->argc; i++) {
    free(command->argv[i]);
  }
  free((void *)command->argv);
  free(command->str);
  free(command);
}  // xd_command_destroy()

int xd_command_add_arg(xd_command_t *command, const char *arg) {
  if (command == NULL || arg == NULL) {
    return -1;
  }

  char *argument = strdup(arg);
  if (argument == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  int new_argc = command->argc + 1;
  char **new_argv =
      (char **)realloc((void *)command->argv, sizeof(char *) * (new_argc + 1));
  if (new_argv == NULL) {
    fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  new_argv[new_argc - 1] = argument;
  new_argv[new_argc] = NULL;

  command->argc = new_argc;
  command->argv = new_argv;

  return 0;  // success
}  // xd_command_add_arg()
