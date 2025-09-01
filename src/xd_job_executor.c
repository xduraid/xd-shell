/*
 * ==============================================================================
 * File: xd_job_executor.c
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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "xd_builtins.h"
#include "xd_command.h"
#include "xd_job.h"
#include "xd_jobs.h"
#include "xd_shell.h"
#include "xd_vars.h"

// ========================
// Macros
// ========================

/**
 * @brief Default access mode for created files.
 */
#define XD_FILE_ACCESS_MODE (0664)

/**
 * @brief Fallback path when the environment variable `PATH` isn't defined.
 */
#define XD_PATH_DEFAULT "/bin:/usr/bin"

// ========================
// Function Declarations
// ========================

static int xd_reset_signal_handlers();

static int xd_backup_fds();
static void xd_restore_fds();
static int xd_redirect_input();
static int xd_redirect_output();
static int xd_redirect_error();

static char *xd_path_search(const char *name);
static void xd_execute_command();

static void xd_execute_builtin_no_fork();

static void xd_failure_cleanup();

// ========================
// Variables
// ========================

/**
 * @brief The current job being executed.
 */
static xd_job_t *xd_job = NULL;

/**
 * @brief The current command being executed.
 */
static xd_command_t *xd_command = NULL;

/**
 * @brief Original `stdin` fd before redirection.
 */
static int xd_original_input_fd = -1;

/**
 * @brief Original `stdout` fd before redirection.
 */
static int xd_original_output_fd = -1;

/**
 * @brief Original `stderr` fd before redirection.
 */
static int xd_original_error_fd = -1;

/**
 * @brief The fd of the current pipe's read end.
 */
static int xd_pipe_read_fd = -1;

/**
 * @brief The fd of the current pipe's write end.
 */
static int xd_pipe_write_fd = -1;

/**
 * @brief The fd of the previous pipe's read end.
 */
static int xd_prev_pipe_read_fd = -1;

/**
 * @brief Indicates whether the command being executed is the first command in
 * the job (non-zero), or not (zero).
 */
static int xd_is_first_command = 0;

/**
 * @brief Indicates whether the command being executed is the last command in
 * the job (non-zero), or not (zero).
 */
static int xd_is_last_command = 0;

// ========================
// Function Definitions
// ========================

/**
 * @brief Resets the signal handlers changed by `xd-shell`'s main process to
 * the default action (`SIG_DFL`).
 *
 * @return `0` on sucess or `-1` on failure.
 */
static int xd_reset_signal_handlers() {
  if (signal(SIGTERM, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGQUIT, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGTSTP, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGTTIN, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGTTOU, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGCHLD, SIG_DFL) == SIG_ERR) {
    return -1;
  }
  return 0;
}  // xd_reset_signal_handlers()

/**
 * @brief Saves a copy of the original `stdin`, `stdout`, and `stderr` fds for
 * restoration later.
 *
 * @return `0` on success, `-1` on failure.
 */
static int xd_backup_fds() {
  if (xd_command->input_file != NULL) {
    xd_original_input_fd = dup(STDIN_FILENO);
    if (xd_original_input_fd == -1) {
      fprintf(stderr, "xd-shell: failed to backup stdin fd: %s\n",
              strerror(errno));
      return -1;
    }
  }

  if (xd_command->output_file != NULL) {
    xd_original_output_fd = dup(STDOUT_FILENO);
    if (xd_original_output_fd == -1) {
      fprintf(stderr, "xd-shell: failed to backup stdout fd: %s\n",
              strerror(errno));
      close(xd_original_input_fd);
      return -1;
    }
  }

  if (xd_command->error_file != NULL) {
    xd_original_error_fd = dup(STDERR_FILENO);
    if (xd_original_error_fd == -1) {
      fprintf(stderr, "xd-shell: failed to backup stderr fd: %s\n",
              strerror(errno));
      close(xd_original_input_fd);
      close(xd_original_output_fd);
      return -1;
    }
  }
  return 0;
}  // xd_backup_fds()

/**
 * @brief Restore the original `stdin`, `stdout`, and `stderr` fds.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` if restoring original fds
 * failed.
 */
static void xd_restore_fds() {
  int failed = 0;

  while (xd_command->input_file != NULL &&
         dup2(xd_original_input_fd, STDIN_FILENO) == -1) {
    if (errno == EINTR) {
      continue;
    }
    fprintf(stderr, "xd-shell: failed to restore stdin fd: %s\n",
            strerror(errno));
    failed = 1;
    break;
  }

  while (xd_command->output_file != NULL &&
         dup2(xd_original_output_fd, STDOUT_FILENO) == -1) {
    if (errno == EINTR) {
      continue;
    }
    fprintf(stderr, "xd-shell: failed to restore stdout fd: %s\n",
            strerror(errno));
    failed = 1;
    break;
  }

  while (xd_command->error_file != NULL &&
         dup2(xd_original_error_fd, STDERR_FILENO) == -1) {
    if (errno == EINTR) {
      continue;
    }
    fprintf(stderr, "xd-shell: failed to restore stderr fd: %s\n",
            strerror(errno));
    failed = 1;
    break;
  }

  if (xd_original_input_fd != -1) {
    close(xd_original_input_fd);
  }
  if (xd_original_output_fd != -1) {
    close(xd_original_output_fd);
  }
  if (xd_original_error_fd != -1) {
    close(xd_original_error_fd);
  }

  if (failed) {
    fprintf(
        stderr,
        "xd-shell: fatal error: couldn't restore original fds... exiting\n");
    exit(EXIT_FAILURE);
  }
}  // xd_restore_fds()

/**
 * @brief Handles `stdin` redirection for the current command to be executed
 * (`xd_command`).
 *
 * @return `0` on success or `-1` on failure.
 */
static int xd_redirect_input() {
  // setup fd
  int input_fd = -1;
  if (xd_command->input_file != NULL) {
    // input from file (input redirection)
    while ((input_fd = open(xd_command->input_file, O_RDONLY)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      fprintf(stderr, "xd-shell: %s: %s\n", xd_command->input_file,
              strerror(errno));
      return -1;
    }
    if (!xd_is_first_command) {
      // pipe input isn't needed
      close(xd_prev_pipe_read_fd);
    }
  }
  else if (!xd_is_first_command) {
    // input from pipe
    input_fd = xd_prev_pipe_read_fd;
  }
  else {
    // input from stdin (default)
    return 0;
  }

  // redirect
  int ret = 0;
  while (dup2(input_fd, STDIN_FILENO) == -1) {
    if (errno == EINTR) {
      continue;
    }
    fprintf(stderr, "xd-shell: dup2: %s\n", strerror(errno));
    ret = -1;
    break;
  }
  close(input_fd);
  return ret;
}  // xd_redirect_input()

/**
 * @brief Handles `stdout` redirection for the current command to be executed
 * (`xd_command`).
 *
 * @return `0` on success or `-1` on failure.
 */
static int xd_redirect_output() {
  // setup fd
  int output_fd = -1;
  if (xd_command->output_file != NULL) {
    // output to file (output redirection)
    int flags = O_WRONLY | O_CREAT;
    flags |= (xd_command->append_output ? O_APPEND : O_TRUNC);
    while ((output_fd = open(xd_command->output_file, flags,
                             XD_FILE_ACCESS_MODE)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      fprintf(stderr, "xd-shell: %s: %s\n", xd_command->output_file,
              strerror(errno));
      return -1;
    }
    if (!xd_is_last_command) {
      // pipe output isn't needed
      close(xd_pipe_write_fd);
    }
  }
  else if (!xd_is_last_command) {
    // output to pipe
    output_fd = xd_pipe_write_fd;
  }
  else {
    // output to stdout (default)
    return 0;
  }

  // redirect
  int ret = 0;
  while (dup2(output_fd, STDOUT_FILENO) == -1) {
    if (errno == EINTR) {
      continue;
    }
    fprintf(stderr, "xd-shell: dup2: %s\n", strerror(errno));
    ret = -1;
    break;
  }
  close(output_fd);
  return ret;
}  // xd_redirect_output()

/**
 * @brief Handles `stderr` redirection for the current command to be executed
 * (`xd_command`).
 *
 * @return `0` on success or `-1` on failure.
 */
static int xd_redirect_error() {
  if (xd_command->error_file != NULL && xd_command->output_file != NULL &&
      strcmp(xd_command->error_file, xd_command->output_file) == 0) {
    // error file is same as output file
    while (dup2(STDOUT_FILENO, STDERR_FILENO) == -1) {
      if (errno == EINTR) {
        continue;
      }
      fprintf(stderr, "xd-shell: dup2: %s\n", strerror(errno));
      return -1;
    }
    return 0;
  }

  // setup fd
  int error_fd = -1;
  if (xd_command->error_file != NULL) {
    // error to file (error redirection)
    int flags = O_WRONLY | O_CREAT;
    flags |= (xd_command->append_error ? O_APPEND : O_TRUNC);

    while ((error_fd = open(xd_command->error_file, flags,
                            XD_FILE_ACCESS_MODE)) == -1) {
      if (errno == EINTR) {
        continue;
      }
      fprintf(stderr, "xd-shell: %s: %s\n", xd_command->error_file,
              strerror(errno));
      return -1;
    }
  }
  else {
    // error to stderr (default)
    return 0;
  }

  // redirect
  int ret = 0;
  while (dup2(error_fd, STDERR_FILENO) == -1) {
    if (errno == EINTR) {
      continue;
    }
    fprintf(stderr, "xd-shell: dup2: %s\n", strerror(errno));
    ret = -1;
    break;
  }
  close(error_fd);
  return ret;
}  // xd_redirect_error()

/**
 * @brief Search `PATH` for an executable with the passed name.
 *
 * @param name The executable name to search for.
 *
 * @return A newly allocated string containing the matched path, or `NULL` if
 * not found or if `name` is invalid.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` on allocation failure.
 *
 * @note The caller is responsible for freeing the allocated memory by calling
 * `free()` and passing it the returned pointer.
 */
static char *xd_path_search(const char *name) {
  if (name == NULL || *name == '\0') {
    return NULL;
  }
  if (strchr(name, '/')) {
    return NULL;
  }

  const char *PATH = xd_vars_get("PATH");
  if (PATH == NULL) {
    PATH = XD_PATH_DEFAULT;
  }

  int name_len = (int)strlen(name);
  const char *cursor = PATH;
  char path_buffer[PATH_MAX];

  while (1) {
    const char *colon = strchr(cursor, ':');
    int segment_len =
        (colon == NULL) ? (int)strlen(cursor) : (int)(colon - cursor);

    const char *dir_path;
    int dir_len;
    if (segment_len == 0) {
      // empty, used current directory
      dir_path = ".";
      dir_len = 1;
    }
    else {
      dir_path = cursor;
      dir_len = segment_len;
    }

    int need_slash = (dir_path[dir_len - 1] != '/');
    int total_len = dir_len + need_slash + name_len;

    if (total_len < PATH_MAX) {
      memcpy(path_buffer, dir_path, dir_len);
      if (need_slash) {
        path_buffer[dir_len] = '/';
      }
      memcpy(path_buffer + dir_len + need_slash, name, name_len);
      path_buffer[total_len] = '\0';

      if (access(path_buffer, X_OK) == 0) {
        struct stat file_stat;
        if (stat(path_buffer, &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
          char *ret = strdup(path_buffer);
          if (ret == NULL) {
            fprintf(stderr, "xd-shell: failed to allocate memory: %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
          }
          return ret;
        }
      }
    }

    if (colon == NULL) {
      break;
    }
    cursor = colon + 1;
  }

  return NULL;
}  // xd_path_search()

/**
 * @brief Executes the current command (`xd_command`).
 */
static void xd_execute_command() {
  if (xd_sh_is_interactive) {
    // interactive shell setup process group
    xd_command->pid = getpid();
    if (xd_job->pgid == 0) {
      xd_job->pgid = xd_command->pid;
    }
    if (setpgid(xd_command->pid, xd_job->pgid) == -1) {
      fprintf(stderr, "xd-shell: setpgid: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (!xd_job->is_background) {
      if (xd_jobs_put_in_foreground(xd_job->pgid) == -1) {
        exit(EXIT_FAILURE);
      }
    }
  }

  if (xd_reset_signal_handlers() == -1) {
    fprintf(stderr, "xd-shell: failed to reset signal handlers\n");
    exit(EXIT_FAILURE);
  }

  if (!xd_is_last_command) {
    close(xd_pipe_read_fd);
  }

  if (xd_redirect_input() == -1 || xd_redirect_output() == -1 ||
      xd_redirect_error() == -1) {
    exit(EXIT_FAILURE);
  }

  const char *executable = xd_command->argv[0];

  if (xd_builtins_is_builtin(executable)) {
    int builtin_exit_code =
        xd_builtins_execute(xd_command->argc, xd_command->argv);
    exit(builtin_exit_code);
  }

  int slash_found = (strchr(executable, '/') != NULL);
  char *resolved_path = NULL;
  if (!slash_found) {
    resolved_path = xd_path_search(executable);
  }

  char **envp = xd_vars_create_envp();

  const char *exec_path = resolved_path != NULL ? resolved_path : executable;
  int exec_has_slash = (strchr(exec_path, '/') != NULL);
  execve(exec_path, xd_command->argv, envp);

  // execvp failed

  // check if the target is a directory
  struct stat file_stat;
  if (stat(exec_path, &file_stat) == 0 && S_ISDIR(file_stat.st_mode)) {
    fprintf(stderr, "xd-shell: %s: Is a directory\n", executable);
    free(resolved_path);
    xd_vars_destroy_envp(envp);
    exit(XD_SH_EXIT_CODE_CANNOT_EXECUTE);
  }

  // check if not found
  if (errno == ENOENT) {
    if (!exec_has_slash) {
      fprintf(stderr, "xd-shell: %s: command not found\n", executable);
    }
    else {
      fprintf(stderr, "xd-shell: %s: %s\n", executable, strerror(errno));
    }
    free(resolved_path);
    xd_vars_destroy_envp(envp);
    exit(XD_SH_EXIT_CODE_NOT_FOUND);
  }

  // cannot be executed
  fprintf(stderr, "xd-shell: %s: %s\n", executable, strerror(errno));
  free(resolved_path);
  xd_vars_destroy_envp(envp);
  exit(XD_SH_EXIT_CODE_CANNOT_EXECUTE);
}  // xd_execute_command()

/**
 * @brief Handles the execution of a builtin command in the parent process
 * without fork.
 *
 * This function is used when the job is foreground (no &), and contains a
 * signal command which is a builtin command.
 *
 * @warning This function calls `exit(EXIT_FAILURE)` if restoring original fds
 * failed after redirection.
 */
static void xd_execute_builtin_no_fork() {
  xd_command = xd_job->commands[0];

  xd_is_first_command = 1;
  xd_is_last_command = 1;

  xd_original_input_fd = -1;
  xd_original_output_fd = -1;
  xd_original_error_fd = -1;

  if (xd_backup_fds() == -1) {
    xd_sh_last_exit_code = EXIT_FAILURE;
    return;
  }

  if (xd_redirect_input() == -1 || xd_redirect_output() == -1 ||
      xd_redirect_error() == -1) {
    xd_sh_last_exit_code = EXIT_FAILURE;
  }
  else {
    xd_sh_last_exit_code =
        xd_builtins_execute(xd_command->argc, xd_command->argv);
  }

  fflush(stdout);
  fflush(stderr);
  xd_restore_fds();
}  // xd_execute_builtin_no_fork()

/**
 * @brief Performs cleanup actions after a job execution failure.
 */
static void xd_failure_cleanup() {
  if (xd_prev_pipe_read_fd != -1) {
    close(xd_prev_pipe_read_fd);
  }
  if (xd_pipe_read_fd != -1) {
    close(xd_pipe_read_fd);
  }
  if (xd_pipe_write_fd != -1) {
    close(xd_pipe_write_fd);
  }
  xd_jobs_kill(xd_job, SIGKILL);
  xd_jobs_wait(xd_job);
  if (xd_sh_is_interactive) {
    xd_jobs_put_in_foreground(xd_sh_pgid);
    while (tcsetattr(STDIN_FILENO, TCSADRAIN, &xd_sh_tty_modes) == -1) {
      if (errno == EINTR) {
        continue;
      }
      break;
    }
  }
  xd_job_destroy(xd_job);
  xd_sh_last_exit_code = EXIT_FAILURE;
}  // xd_failure_cleanup()

// ========================
// Public Functions
// ========================

void xd_job_executor(xd_job_t *job) {
  if (xd_sh_is_interactive) {
    tcgetattr(STDIN_FILENO, &xd_sh_tty_modes);
  }

  xd_job = job;

  if (xd_job->command_count == 1 && !xd_job->is_background &&
      xd_builtins_is_builtin(xd_job->commands[0]->argv[0])) {
    xd_execute_builtin_no_fork();
    xd_job_destroy(xd_job);
    return;
  }

  xd_job->pgid = 0;
  xd_pipe_read_fd = -1;
  xd_pipe_write_fd = -1;
  int pipe_fd[2] = {-1, -1};

  for (int i = 0; i < xd_job->command_count; i++) {
    xd_command = xd_job->commands[i];
    xd_command->pid = 0;
    xd_is_first_command = (i == 0);
    xd_is_last_command = (i == xd_job->command_count - 1);
    xd_prev_pipe_read_fd = xd_pipe_read_fd;

    // create a pipe between each two consecutive commands in the job
    if (i < xd_job->command_count - 1) {
      if (pipe(pipe_fd) == -1) {
        fprintf(stderr, "xd-shell: pipe: %s\n", strerror(errno));
        xd_failure_cleanup();
        return;
      }
      xd_pipe_read_fd = pipe_fd[0];
      xd_pipe_write_fd = pipe_fd[1];
    }

    // fork a child process to execute the current command
    pid_t child_pid = fork();
    if (child_pid == -1) {
      fprintf(stderr, "xd-shell: fork: %s\n", strerror(errno));
      xd_failure_cleanup();
      return;
    }

    if (child_pid == 0) {
      xd_execute_command();  // This calls `exit()`
    }

    xd_command->pid = child_pid;
    xd_job->unreaped_count++;

    if (xd_sh_is_interactive) {
      if (xd_job->pgid == 0) {
        xd_job->pgid = child_pid;
      }
      setpgid(xd_command->pid, xd_job->pgid);
    }
    else {
      xd_job->pgid = xd_sh_pgid;
    }

    if (!xd_is_first_command) {
      close(xd_prev_pipe_read_fd);
    }
    if (!xd_is_last_command) {
      close(xd_pipe_write_fd);
    }
  }  // for()

  struct timespec time_spec;
  clock_gettime(CLOCK_MONOTONIC, &time_spec);
  job->last_active = (uint64_t)time_spec.tv_sec * XD_SH_NANOSECONDS_PER_SECOND +
                     (uint64_t)time_spec.tv_nsec;

  if (!xd_job->is_background) {
    if (xd_sh_is_interactive) {
      xd_jobs_put_in_foreground(xd_job->pgid);
      xd_sh_last_exit_code = xd_jobs_wait(xd_job);
      xd_jobs_put_in_foreground(xd_sh_pgid);
    }
    else {
      xd_sh_last_exit_code = xd_jobs_wait(xd_job);
    }

    if (!xd_job_is_alive(xd_job)) {
      xd_job_destroy(xd_job);
    }
    else {
      xd_job->notify = 1;
      xd_jobs_add(xd_job);
      if (tcgetattr(STDIN_FILENO, &xd_job->tty_modes) == 0) {
        xd_job->has_tty_modes = 1;
      }
    }

    while (tcsetattr(STDIN_FILENO, TCSADRAIN, &xd_sh_tty_modes) == -1) {
      if (errno == EINTR) {
        continue;
      }
      break;
    }
  }
  else {
    xd_jobs_add(xd_job);
    if (xd_sh_is_interactive) {
      printf("[%d] %d\n", xd_job->job_id, xd_command->pid);
    }
    xd_sh_last_exit_code = EXIT_SUCCESS;
  }
}  // xd_job_executor()
