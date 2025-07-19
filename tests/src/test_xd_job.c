/*
 * ==============================================================================
 * File: test_xd_job.c
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

#include "xd_command.h"
#include "xd_ctest.h"
#include "xd_job.h"

static int test_xd_job_create() {
  XD_TEST_START;

  // Arrange - Act
  xd_job_t *job = xd_job_create();

  // Assert
  XD_TEST_ASSERT(job != NULL);
  XD_TEST_ASSERT(job->commands == NULL);
  XD_TEST_ASSERT(job->command_count == 0);
  XD_TEST_ASSERT(job->is_background == 0);
  XD_TEST_ASSERT(job->pgid == 0);

xd_test_cleanup:
  xd_job_destroy(job);
  XD_TEST_END;
}  // test_xd_job_create()

static int test_xd_job_add_command1() {
  XD_TEST_START;

  // Arrange
  xd_job_t *job = xd_job_create();
  xd_command_t *command = xd_command_create("foo");

  // Act
  int ret = xd_job_add_command(job, command);

  // Assert
  XD_TEST_ASSERT(job != NULL);
  XD_TEST_ASSERT(job->commands != NULL);
  XD_TEST_ASSERT(ret == 0);
  XD_TEST_ASSERT(job->command_count == 1);
  XD_TEST_ASSERT(job->commands[0] == command);

  XD_TEST_ASSERT(job->is_background == 0);
  XD_TEST_ASSERT(job->pgid == 0);

xd_test_cleanup:
  xd_job_destroy(job);
  XD_TEST_END;
}  // test_xd_job_add_command1()

static int test_xd_job_add_command2() {
  XD_TEST_START;

  // Arrange
  xd_job_t *job = xd_job_create();
  xd_command_t *command1 = xd_command_create("foo");
  xd_command_t *command2 = xd_command_create("bar");

  // Act
  int ret1 = xd_job_add_command(job, command1);
  int ret2 = xd_job_add_command(job, command2);

  // Assert
  XD_TEST_ASSERT(job != NULL);
  XD_TEST_ASSERT(job->commands != NULL);
  XD_TEST_ASSERT(ret1 == 0);
  XD_TEST_ASSERT(ret2 == 0);
  XD_TEST_ASSERT(job->command_count == 2);
  XD_TEST_ASSERT(job->commands[0] == command1);
  XD_TEST_ASSERT(job->commands[1] == command2);

  XD_TEST_ASSERT(job->is_background == 0);
  XD_TEST_ASSERT(job->pgid == 0);

xd_test_cleanup:
  xd_job_destroy(job);
  XD_TEST_END;
}  // test_xd_job_add_command2()

static int test_xd_job_add_command3() {
  XD_TEST_START;

  // Arrange
  xd_job_t *job = xd_job_create();
  xd_command_t *command = xd_command_create("foo");

  // Act
  int ret1 = xd_job_add_command(job, NULL);
  int ret2 = xd_job_add_command(NULL, command);
  int ret3 = xd_job_add_command(NULL, NULL);

  // Assert
  XD_TEST_ASSERT(ret1 == -1);
  XD_TEST_ASSERT(ret2 == -1);
  XD_TEST_ASSERT(ret3 == -1);

xd_test_cleanup:
  xd_command_destroy(command);
  xd_job_destroy(job);
  XD_TEST_END;
}  // test_xd_job_add_command3()

static xd_test_case test_suite[] = {
    XD_TEST_CASE(test_xd_job_create),
    XD_TEST_CASE(test_xd_job_add_command1),
    XD_TEST_CASE(test_xd_job_add_command2),
    XD_TEST_CASE(test_xd_job_add_command3),
};

int main() {
  XD_TEST_RUN_ALL(test_suite);
}  // main()
