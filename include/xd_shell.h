/*
 * ==============================================================================
 * File: xd_shell.h
 * Author: Duraid Maihoub
 * Date: 17 July 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#ifndef XD_SHELL_H
#define XD_SHELL_H

#include <signal.h>
#include <sys/types.h>

// ========================
// Macros
// ========================

/**
 * @brief The maximum length of the shell's input prompt.
 */
#define XD_SH_PROMPT_MAX_LENGTH (5000)

/**
 * @brief Number of nanoseconds in one second.
 */
#define XD_SH_NANOSECONDS_PER_SECOND 1000000000ULL

/**
 * @brief Exit code when command cannot be executed.
 */
#define XD_SH_EXIT_CODE_CANNOT_EXECUTE (126)

/**
 * @brief Exit code when command was not found.
 */
#define XD_SH_EXIT_CODE_NOT_FOUND (127)

/**
 * @brief Offset added to signal numbers to generate a shell-compatible exit
 * status.
 */
#define XD_SH_EXIT_CODE_SIGNAL_OFFSET (128)

// ========================
// Function Declarations
// ========================

// ========================
// Variables
// ========================

/**
 * @brief Indicates whether the shell is interactive (non-zero) or not (zero).
 */
extern int xd_sh_is_interactive;

/**
 * @brief The shell's input prompt string.
 */
extern char xd_sh_prompt[];

/**
 * @brief Process id of the main process.
 */
extern pid_t xd_sh_pid;

/**
 * @brief Process group id of the main process.
 */
extern pid_t xd_sh_pgid;

/**
 * @brief Indicates whether readline is currently running.
 */
extern volatile sig_atomic_t xd_sh_readline_running;

/**
 * @brief Exit code of the last executed command.
 */
extern int xd_sh_last_exit_code;

#endif  // XD_SHELL_H
