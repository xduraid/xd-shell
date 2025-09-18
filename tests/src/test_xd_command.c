/*
 * ==============================================================================
 * File: test_xd_command.c
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

#include <stddef.h>
#include <string.h>

#include "xd_command.h"
#include "xd_ctest.h"

static int test_xd_command_create() {
  XD_TEST_START;

  // Arrange - Act
  xd_command_t *command = xd_command_create();

  // Assert
  XD_TEST_ASSERT(command != NULL);
  XD_TEST_ASSERT(command->argv == NULL);
  XD_TEST_ASSERT(command->argc == 0);

  XD_TEST_ASSERT(command->input_file == NULL);
  XD_TEST_ASSERT(command->output_file == NULL);
  XD_TEST_ASSERT(command->error_file == NULL);
  XD_TEST_ASSERT(command->append_output == 0);
  XD_TEST_ASSERT(command->append_output == 0);
  XD_TEST_ASSERT(command->pid == 0);

xd_test_cleanup:
  xd_command_destroy(command);
  XD_TEST_END;
}  // test_xd_command_create()

static int test_xd_command_add_arg1() {
  XD_TEST_START;

  // Arrange
  xd_command_t *command = xd_command_create();

  // Act
  int ret = xd_command_add_arg(command, "foo");

  // Assert
  XD_TEST_ASSERT(command != NULL);
  XD_TEST_ASSERT(ret == 0);

  XD_TEST_ASSERT(command->argv != NULL);
  XD_TEST_ASSERT(command->argc == 1);
  XD_TEST_ASSERT(strcmp(command->argv[0], "foo") == 0);
  XD_TEST_ASSERT(command->argv[1] == NULL);

  XD_TEST_ASSERT(command->input_file == NULL);
  XD_TEST_ASSERT(command->output_file == NULL);
  XD_TEST_ASSERT(command->error_file == NULL);
  XD_TEST_ASSERT(command->append_output == 0);
  XD_TEST_ASSERT(command->append_output == 0);
  XD_TEST_ASSERT(command->pid == 0);

xd_test_cleanup:
  xd_command_destroy(command);
  XD_TEST_END;
}  // test_xd_command_add_arg1()

static int test_xd_command_add_arg2() {
  XD_TEST_START;

  // Arrange
  xd_command_t *command = xd_command_create();

  // Act
  int ret1 = xd_command_add_arg(command, "foo");
  int ret2 = xd_command_add_arg(command, "bar");

  // Assert
  XD_TEST_ASSERT(command != NULL);
  XD_TEST_ASSERT(ret1 == 0);
  XD_TEST_ASSERT(ret2 == 0);

  XD_TEST_ASSERT(command->argv != NULL);
  XD_TEST_ASSERT(command->argc == 2);
  XD_TEST_ASSERT(strcmp(command->argv[0], "foo") == 0);
  XD_TEST_ASSERT(strcmp(command->argv[1], "bar") == 0);
  XD_TEST_ASSERT(command->argv[2] == NULL);

  XD_TEST_ASSERT(command->input_file == NULL);
  XD_TEST_ASSERT(command->output_file == NULL);
  XD_TEST_ASSERT(command->error_file == NULL);
  XD_TEST_ASSERT(command->append_output == 0);
  XD_TEST_ASSERT(command->append_output == 0);
  XD_TEST_ASSERT(command->pid == 0);

xd_test_cleanup:
  xd_command_destroy(command);
  XD_TEST_END;
}  // test_xd_command_add_arg2()

static int test_xd_command_add_arg3() {
  XD_TEST_START;

  // Arrange
  xd_command_t *command = xd_command_create();

  // Act
  int ret1 = xd_command_add_arg(NULL, "bar");
  int ret2 = xd_command_add_arg(command, NULL);
  int ret3 = xd_command_add_arg(NULL, NULL);

  // Assert
  XD_TEST_ASSERT(command != NULL);
  XD_TEST_ASSERT(ret1 == -1);
  XD_TEST_ASSERT(ret2 == -1);
  XD_TEST_ASSERT(ret3 == -1);

xd_test_cleanup:
  xd_command_destroy(command);
  XD_TEST_END;
}  // test_xd_command_add_arg3()

static xd_test_case test_suite[] = {
    XD_TEST_CASE(test_xd_command_create),
    XD_TEST_CASE(test_xd_command_add_arg1),
    XD_TEST_CASE(test_xd_command_add_arg2),
    XD_TEST_CASE(test_xd_command_add_arg3),
};

int main() {
  XD_TEST_RUN_ALL(test_suite);
}  // main()
