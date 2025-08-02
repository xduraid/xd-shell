/*
 * ==============================================================================
 * File: xd_job.h
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

#ifndef XD_JOB_H
#define XD_JOB_H

#include <inttypes.h>
#include <sys/types.h>

#include "xd_command.h"

// ========================
// Typedefs
// ========================

/**
 * @brief Represents a shell job (pipeline of commands).
 */
typedef struct xd_job_t {
  xd_command_t **commands;  // Array of commands in the job
  int command_count;        // Number of commands in the job
  int is_background;        // Whether to run as a background process
  pid_t pgid;               // PGID of the processes executing the job
  int unreaped_count;       // Number of unreaped child processes
  int stopped_count;        // Number of stopped child processes
  int wait_status;          // Last wait status of children
  int job_id;               // Id of the job (in jobs list)
  uint64_t last_active;     // Last time job recived a signal
} xd_job_t;

// ========================
// Function Declarations
// ========================

/**
 * @brief Creates and initializes a new `xd_job_t` structure.
 *
 * @return A pointer to the newly created `xd_job_t`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `xd_job_destroy()` and passing it the returned pointer.
 */
xd_job_t *xd_job_create();

/**
 * @brief Frees the memory allocated for the passed `xd_job_t` structure.
 *
 * @param job A pointer to the `xd_job_t` to be freed.
 *
 * @note If the passed pointer is `NULL` no action shall occur.
 */
void xd_job_destroy(xd_job_t *job);

/**
 * @brief Adds the passed command to the commands array of the passed `xd_job_t`
 * structure.
 *
 * @param job A pointer to the `job_t` structure to which the command will be
 * added.
 * @param command A pointer to the `xd_command_t` structure to be added.
 *
 * @return `0` on success or `-1` if any of the passed parameters (`job` or
 * `command`) is `NULL`.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 */
int xd_job_add_command(xd_job_t *job, xd_command_t *command);

/**
 * @brief Returns the `xd_command_t` structure with the passed pid from the
 * passed job.
 *
 * @param job A pointer to the `job_t` to be searched.
 * @param pid The pid to search for.
 *
 * @return A pointer to the `xd_command_t` structure with the passed PID, or
 * `NULL` if not found.
 */
xd_command_t *xd_job_get_command_with_pid(const xd_job_t *job, pid_t pid);

/**
 * @brief Checks whether the passed job is fully stopped.
 *
 * @param job A pointer to the `job_t` to be checked.
 *
 * @return `1` if the passed job is stopped, `0` otherwise.
 */
int xd_job_is_stopped(const xd_job_t *job);

/**
 * @brief Checks whether the passed job is alive (not fully reaped).
 *
 * @param job A pointer to the `job_t` to be checked.
 *
 * @return `1` if the passed job is alive, `0` otherwise.
 */
int xd_job_is_alive(const xd_job_t *job);

/**
 * @brief Executes the passed job.
 *
 * @param job A pointer to the `xd_job_t` structure to be executed.
 */
void xd_job_execute(xd_job_t *job);

#endif  //  XD_JOB_H
