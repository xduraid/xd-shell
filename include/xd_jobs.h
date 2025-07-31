/*
 * ==============================================================================
 * File: xd_jobs.h
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

#ifndef XD_JOBS_H
#define XD_JOBS_H

#include <sys/types.h>

#include "xd_job.h"

/**
 * @brief Initialize the jobs list.
 */
void xd_jobs_init();

/**
 * @brief Frees the memory allocated for the jobs list.
 */
void xd_jobs_destroy();

/**
 * @brief Adds the passed job to the end of the jobs list and gives it a job id.
 *
 * @param job A pointer to the job to be added.
 */
void xd_jobs_add(xd_job_t *job);

/**
 * @brief Returns the job that has a child process with the passed PID.
 *
 * @param pid pid The process id to look for.
 *
 * @return A pointer to the job that has a child with the passed PID, or `NULL`
 * if not found or if the jobs list is not initialized yet.
 */
xd_job_t *xd_jobs_get_with_pid(pid_t pid);

/**
 * @brief Returns the job with the passed job id.
 *
 * @param job_id The id of the job.
 *
 * @return A pointer to the job with the passed job id, or `NULL` if not found
 * or if the jobs list is not initialized yet.
 */
xd_job_t *xd_jobs_get_with_id(int job_id);

void xd_jobs_refresh();

/**
 * @brief Puts the process group with the passed id in control of the terminal.
 *
 * @param pgid Id of the process group to be put in control of the terminal.
 *
 * @return `0` on success or `-1` on failure.
 *
 * @note This function will fail if the shell is not in interactive mode.
 */
int xd_jobs_put_in_foreground(pid_t pgid);

/**
 * @brief Kills all processes of the passed job.
 *
 * @param job The job to kill its processes.
 */
void xd_jobs_kill(xd_job_t *job);

/**
 * @brief Waits for the passed job to terminate or stop.
 *
 * @param job The job to wait for.
 *
 * @note This function should be used only for foreground jobs.
 */
void xd_jobs_wait(xd_job_t *job);

/**
 * @brief Blocks delivery of the `SIGCHLD` signal.
 *
 * @note This function must be paired with a corresponding call to
 * `xd_jobs_sigchld_unblock()`.
 */
void xd_jobs_sigchld_block();

/**
 * @brief Unblocks delivery of the `SIGCHLD` signal.
 *
 * @note This function must be called only after a matching call to
 * `xd_jobs_sigchld_block()`.
 */
void xd_jobs_sigchld_unblock();

#endif  // XD_JOBS_H
