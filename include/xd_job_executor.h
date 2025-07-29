/*
 * ==============================================================================
 * File: xd_job_executor.h
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

#ifndef XD_JOB_EXECUTOR_H
#define XD_JOB_EXECUTOR_H

#include "xd_job.h"

/**
 * @brief Executes the passed job.
 *
 * @param job A pointer to the `xd_job_t` structure to be executed.
 */
void xd_job_executor(xd_job_t *job);

#endif  // XD_JOB_EXECUTOR_H
