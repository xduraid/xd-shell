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
 * @brief The string used as prompt in multi-line input.
 */
#define XD_SH_PROMPT2 "> "

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

/**
 * @brief Exit code due to `SIGINTR`.
 */
#define XD_SH_EXIT_CODE_SIGINTR (130)

/**
 * @brief The exit code when args are invalid.
 */
#define XD_SH_EXIT_CODE_USAGE (2)

/**
 * @brief The default history file name.
 */
#define XD_SH_DEF_HISTFILE_NAME ".xdsh_history"

/**
 * @brief Fallback path when the environment variable `PATH` isn't defined.
 */
#define XD_SH_DEF_PATH \
  "/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin"

// ========================
// Function Declarations
// ========================

/**
 * @brief Rebuilds `xd_sh_prompt` with the current user, host and working
 * directory.
 */
void xd_sh_update_prompt();

/**
 * @brief Search the directories listed in `PATH` for a file with the passed
 * name.
 *
 * @param name The file name to search for.
 *
 * @return A newly allocated string containing the matched path, or `NULL` if
 * not found or if `name` is invalid.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `free()` and passing it the returned pointer.
 */
char *xd_sh_path_search(const char *name);

// ========================
// Variables
// ========================

/**
 * @brief Indicates whether the shell is a login shell (non-zero) or not (zero).
 */
extern int xd_sh_is_login;

/**
 * @brief Indicates whether the shell is interactive (non-zero) or not (zero).
 */
extern int xd_sh_is_interactive;

/**
 * @brief Indicates whether the shell is a sub-shell (non-zero) or not (zero).
 */
extern int xd_sh_is_subshell;

/**
 * @brief The shell's input prompt string.
 */
extern char xd_sh_prompt[];

/**
 * @brief The absolute path of the shell executable.
 */
extern char xd_sh_path[];

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
 * @brief Indicates whether the shell was interrupted.
 */
extern volatile sig_atomic_t xd_sh_is_interrupted;

/**
 * @brief Exit code of the last executed command.
 */
extern int xd_sh_last_exit_code;

/**
 * @brief Process id of the last job lunched in background.
 */
extern pid_t xd_sh_last_bg_job_pid;

/**
 * @brief The current modes for the shell.
 */
extern struct termios xd_sh_tty_modes;

#endif  // XD_SHELL_H
