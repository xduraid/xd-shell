/*
 * ==============================================================================
 * File: xd_command.h
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

#ifndef XD_COMMAND_H
#define XD_COMMAND_H

#include <stdlib.h>
#include <sys/types.h>

// ========================
// Typedefs
// ========================

/**
 * @brief Represents a shell command with its arguments and I/O redirection
 * information.
 */
typedef struct xd_command_t {
  int argc;           // Number of arguments
  char **argv;        // Array of arguments (null-terminated)
  char *input_file;   // File for stdin redirection
  char *output_file;  // File for stdout redirection
  int append_output;  // Whether to append to the output file
  char *error_file;   // File for stderr redirection
  int append_error;   // Whether to append to the error file
  pid_t pid;          // PID of the process executing the command
  int wait_status;    // Status of command process when reaped with wait
  char *str;          // String used to run this command
} xd_command_t;

// ========================
// Function Declarations
// ========================

/**
 * @brief Creates and initializes a new `xd_command_t` structure with the passed
 * executable as the first argument in its argument array.
 *
 * @param exec The name or path of the executable.
 *
 * @return A pointer to the newly created `xd_command_t` structure or `NULL` if
 * the passed string `exec` is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_command_destroy()` and passing it the returned pointer.
 */
xd_command_t *xd_command_create(const char *exec);

/**
 * @brief Frees the memory allocated for the passed `xd_command_t` structure.
 *
 * @param command A pointer to the `xd_command_t` structure to be freed.
 *
 * @note If the passed pointer is `NULL` no action shall occur.
 */
void xd_command_destroy(xd_command_t *command);

/**
 * @brief Adds the passed argument to the arguments array of the passed
 * `xd_command_t` structure.
 *
 * @param command A pointer to the `xd_command_t` structure to which the
 * argument will be added.
 * @param arg The argument to be added.
 *
 * @return `0` on success or `-1` if any of the passed parameters (`command` or
 * `arg`) is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
int xd_command_add_arg(xd_command_t *command, const char *arg);

#endif  // XD_COMMAND_H
