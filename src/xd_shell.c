/*
 * ==============================================================================
 * File: xd_shell.c
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

#include "xd_shell.h"

#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "xd_aliases.h"
#include "xd_arg_expander.h"
#include "xd_command.h"
#include "xd_comp_generator.h"
#include "xd_job.h"
#include "xd_jobs.h"
#include "xd_readline.h"
#include "xd_string.h"
#include "xd_utils.h"
#include "xd_vars.h"

// ========================
// Function Declarations
// ========================

static void xd_sh_ascii_art();
static void xd_sh_usage();
static void xd_sh_help();

static void xd_sh_init(int argc, char **argv);
static void xd_sh_destroy() __attribute__((destructor));
static void xd_sh_sigint_handler(int signum);
static void xd_sh_sigchld_handler(int signum);
static int xd_sh_setup_signal_handlers();
static const char *xd_sh_resolve_home();
static int xd_sh_source_file(const char *path);
static void xd_sh_source_startup_files();
static void xd_sh_set_default_env();
static int xd_sh_run();

// flex and bison functions
extern void yylex_scan_string(char *str);
extern void yylex_scan_file(FILE *file);
extern void yylex_scan_stdin_interactive();
extern void yylex_scan_stdin_noninteractive();
extern void yyparse_initialize();
extern int yyparse();
extern void yyparse_cleanup();

// ========================
// Variables
// ========================

// ========================
// Public Variables
// ========================

int xd_sh_is_login = 0;
int xd_sh_is_interactive = 0;
int xd_sh_is_subshell = 0;
char xd_sh_prompt[XD_SH_PROMPT_MAX_LENGTH] = {0};
char xd_sh_path[PATH_MAX] = {0};
pid_t xd_sh_pid = 0;
pid_t xd_sh_pgid = 0;
volatile sig_atomic_t xd_sh_readline_running = 0;
volatile sig_atomic_t xd_sh_is_interrupted = 0;
int xd_sh_last_exit_code = 0;
pid_t xd_sh_last_bg_job_pid = 0;
struct termios xd_sh_tty_modes = {0};

// ========================
// Function Definitions
// ========================

/**
 * @brief Prints the ASCII art banner that introduces xd-shell.
 */
static void xd_sh_ascii_art() {
  printf(
      "\n"
      ".-----------------------------------------------------------------------"
      "-------------.\n"
      "|                                                                       "
      "             |\n"
      "|                                                                       "
      "             |\n"
      "|                                                                       "
      "             |\n"
      "|                        88                       88                    "
      "   88  88    |\n"
      "|                        88                       88                    "
      "   88  88    |\n"
      "|                        88                       88                    "
      "   88  88    |\n"
      "|   8b,     ,d8  ,adPPYb,88            ,adPPYba,  88,dPPYba,    "
      ",adPPYba,  88  88    |\n"
      "|    `Y8, ,8P'  a8\"    `Y88  aaaaaaaa  I8[    \"\"  88P'    \"8a  "
      "a8P_____88  88  88    |\n"
      "|      )888(    8b       88  \"\"\"\"\"\"\"\"   `\"Y8ba,   88       88  "
      "8PP\"\"\"\"\"\"\"  88  88    |\n"
      "|    ,d8\" \"8b,  \"8a,   ,d88            aa    ]8I  88       88  \"8b, "
      "  ,aa  88  88    |\n"
      "|   8P'     `Y8  `\"8bbdP\"Y8            `\"YbbdP\"'  88       88   "
      "`\"Ybbd8\"'  88  88    |\n"
      "|                                                                       "
      "             |\n"
      "|                                                                       "
      "             |\n"
      "|                                                                       "
      "             |\n"
      "|  xd-shell v0.1.0                                                      "
      "             |\n"
      "|  Author: Duraid Maihoub | MIT License                                 "
      "             |\n"
      "|  GitHub: https://github.com/xduraid/xd-shell                          "
      "             |\n"
      "|                                                                       "
      "             |\n"
      "'-----------------------------------------------------------------------"
      "-------------'\n\n");
}

/**
 * @brief Prints usage information for the shell executable.
 */
static void xd_sh_usage() {
  fprintf(stderr, "xd_shell: usage: xd_shell [-l] [-c string | -f file]\n");
}  // xd_sh_usage()

/**
 * @brief Prints detailed help information for the shell executable.
 */
static void xd_sh_help() {
  xd_sh_ascii_art();
  printf(
      "usage: xd_shell [-l] [-c string | -f file]\n"
      "  -l          run as a login shell\n"
      "  -c string   execute the commands provided in the string argument\n"
      "  -f file     execute commands by parsing the specified file\n"
      "\n"
      "Without options, xd-shell reads from standard input. When both stdin "
      "and\n"
      "stdout are terminals it starts in interactive mode and uses  "
      "xd_readline,\n"
      "otherwise it processes input non-interactively.\n");
}  // xd_sh_help()

/**
 * @brief Constructor, runs before main to initialize the shell.
 */

/**
 * @brief Shell initializer function, must be called first thing in main.
 *
 * @param argc Number of arguments in the argument array.
 * @param argv The argument array.
 */
static void xd_sh_init(int argc, char **argv) {
  // parse arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      xd_sh_help();
      exit(EXIT_SUCCESS);
    }
  }

  if (argv[0][0] == '-') {
    xd_sh_is_login = 1;
  }

  char *command_string = NULL;
  char *file_to_parse = NULL;
  FILE *input_file = NULL;

  opterr = 0;
  int opt_char;
  while ((opt_char = getopt(argc, argv, "+:lc:f:")) != -1) {
    switch (opt_char) {
      case 'c':
        command_string = optarg;
        break;
      case 'f':
        file_to_parse = optarg;
        break;
      case 'l':
        xd_sh_is_login = 1;
        break;
      case ':':
        fprintf(stderr, "xd-shell: -%c: option requires an argument\n", optopt);
        xd_sh_usage();
        exit(XD_SH_EXIT_CODE_USAGE);
      case '?':
      default:
        fprintf(stderr, "xd-shell: -%c: invalid option\n",
                optopt != 0 ? optopt : '?');
        xd_sh_usage();
        exit(XD_SH_EXIT_CODE_USAGE);
    }
  }

  if (command_string != NULL && file_to_parse != NULL) {
    fprintf(stderr, "xd-shell: options -c and -f cannot be used together\n");
    xd_sh_usage();
    exit(XD_SH_EXIT_CODE_USAGE);
  }

  if (optind < argc) {
    fprintf(stderr, "xd-shell: unexpected argument: %s\n", argv[optind]);
    xd_sh_usage();
    exit(XD_SH_EXIT_CODE_USAGE);
  }

  if (file_to_parse != NULL) {
    input_file = fopen(file_to_parse, "r");
    if (input_file == NULL) {
      fprintf(stderr, "xd-shell: %s: %s\n", file_to_parse, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  xd_sh_is_interactive = (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO));
  if (command_string != NULL || file_to_parse != NULL) {
    xd_sh_is_interactive = 0;
  }
  pid_t pid = getpid();
  pid_t pgid = getpgrp();

  if (xd_sh_setup_signal_handlers() == -1) {
    fprintf(stderr, "xd-shell: failed to setup signal handlers\n");
    exit(EXIT_FAILURE);
  }

  if (xd_sh_is_interactive) {
    // wait until the shell process group is in foreground
    while (tcgetpgrp(STDIN_FILENO) != (pgid = getpgrp())) {
      kill(-pgid, SIGTTIN);
    }

    // put the shell in its own process group if needed
    if (pid != pgid) {
      if (setpgid(pid, pid) == -1) {
        fprintf(stderr, "xd-shell: setpgid: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
    }

    // ensure the shell controls the terminal
    pgid = getpgrp();
    if (tcsetpgrp(STDIN_FILENO, pgid) == -1) {
      fprintf(stderr, "xd-shell: tcsetpgrp: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    strncpy(xd_sh_prompt, "\e[0;94mxd-shell\e[0m$ ", XD_SH_PROMPT_MAX_LENGTH);
  }

  xd_sh_pid = pid;
  xd_sh_pgid = pgid;
  xd_jobs_init();
  xd_aliases_init();
  xd_vars_init();
  xd_sh_set_default_env();
  yyparse_initialize();
  xd_arg_expander_init();

  if (realpath("/proc/self/exe", xd_sh_path) == NULL) {
    fprintf(stderr, "xd-shell: failed to get shell path\n");
    exit(EXIT_FAILURE);
  }
  xd_vars_put("SHELL", xd_sh_path, 1);

  if (xd_sh_is_interactive) {
    // setup `HISTFILE` variable and load history from file
    char path[PATH_MAX];
    const char *histfile = xd_vars_get("HISTFILE");
    if (histfile == NULL) {
      const char *HOME = xd_sh_resolve_home();

      if (HOME != NULL) {
        snprintf(path, PATH_MAX, "%s/%s", HOME, XD_SH_DEF_HISTFILE_NAME);
      }
      else {
        snprintf(path, PATH_MAX, "%s", XD_SH_DEF_HISTFILE_NAME);
      }
    }
    else {
      snprintf(path, PATH_MAX, "%s", histfile);
    }
    xd_vars_put("HISTFILE", path, 0);
    xd_readline_history_load_from_file(path);

    // setup tab-completion function
    xd_readline_completions_generator = xd_completions_generator;
  }

  if (xd_sh_is_interactive) {
    yylex_scan_stdin_interactive();
  }
  else if (command_string != NULL) {
    yylex_scan_string(command_string);
  }
  else if (input_file != NULL) {
    yylex_scan_file(input_file);
  }
  else {
    yylex_scan_stdin_noninteractive();
  }

  if (xd_sh_is_interactive && xd_sh_is_login) {
    xd_sh_ascii_art();
  }

  xd_sh_source_startup_files();
}  // xd_sh_init()

/**
 * @brief Destructor, runs before exit to cleanup after the shell.
 */
static void xd_sh_destroy() {
  // save history to file
  if (xd_sh_is_interactive && getpid() == xd_sh_pid) {
    char *histfile = xd_vars_get("HISTFILE");
    if (histfile != NULL) {
      xd_readline_history_save_to_file(histfile, 0);
    }
  }
  yyparse_cleanup();
  xd_jobs_destroy();
  xd_aliases_destroy();
  xd_vars_destroy();
  xd_arg_expander_destroy();
}  // xd_sh_destroy()

/**
 * @brief Handles `SIGINT` signal.
 *
 * @param signum The signal number.
 */
static void xd_sh_sigint_handler(int signum) {
  (void)signum;
  int saved_errno = errno;
  if (xd_sh_readline_running) {
    write(STDERR_FILENO, "^C", 2);
  }
  else {
    write(STDERR_FILENO, "\n", 1);
  }
  xd_sh_is_interrupted = 1;
  errno = saved_errno;
}  // xd_sh_sigint_handler()

static void xd_sh_sigchld_handler(int signum) {
  (void)signum;
  int status;
  pid_t pid;
  int saved_errno = errno;
  while (1) {
    pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
    if (pid <= 0) {
      if (pid == -1 && errno == EINTR) {
        continue;
      }
      break;
    }

    xd_job_t *job = xd_jobs_get_with_pid(pid);
    xd_command_t *command = xd_job_get_command_with_pid(job, pid);
    if (job == NULL || command == NULL) {
      continue;
    }

    int was_stopped = WIFSTOPPED(command->wait_status);
    command->wait_status = status;

    if (job->commands[job->command_count - 1] == command) {
      job->wait_status = status;
    }

    if (WIFCONTINUED(status)) {
      if (was_stopped) {
        job->stopped_count--;
      }
    }
    else if (WIFSTOPPED(status)) {
      if (!was_stopped) {
        job->stopped_count++;
      }
    }
    else if (WIFEXITED(status) || WIFSIGNALED(status)) {
      if (was_stopped) {
        job->stopped_count--;
      }
      job->unreaped_count--;
    }

    if (!xd_job_is_alive(job) || xd_job_is_stopped(job)) {
      job->notify = 1;
    }

    struct timespec time_spec;
    clock_gettime(CLOCK_MONOTONIC, &time_spec);
    job->last_active =
        (uint64_t)time_spec.tv_sec * XD_SH_NANOSECONDS_PER_SECOND +
        (uint64_t)time_spec.tv_nsec;
  }
  errno = saved_errno;
}  // xd_sh_sigchld_handler()

/**
 * @brief Sets up the signal handlers.
 *
 * @return `0` on success, `-1` on failure.
 */
static int xd_sh_setup_signal_handlers() {
  struct sigaction signal_action;
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_flags = 0;

  if (xd_sh_is_interactive) {
    signal_action.sa_handler = SIG_IGN;
    if (sigaction(SIGTERM, &signal_action, NULL) == -1) {
      return -1;
    }
    if (sigaction(SIGQUIT, &signal_action, NULL) == -1) {
      return -1;
    }
    if (sigaction(SIGTSTP, &signal_action, NULL) == -1) {
      return -1;
    }
    if (sigaction(SIGTTIN, &signal_action, NULL) == -1) {
      return -1;
    }
    if (sigaction(SIGTTOU, &signal_action, NULL) == -1) {
      return -1;
    }
    signal_action.sa_handler = xd_sh_sigint_handler;
    if (sigaction(SIGINT, &signal_action, NULL) == -1) {
      return -1;
    }
  }

  signal_action.sa_flags = SA_RESTART;
  signal_action.sa_handler = xd_sh_sigchld_handler;
  if (sigaction(SIGCHLD, &signal_action, NULL) == -1) {
    return -1;
  }
  return 0;
}  // xd_sh_setup_signal_handlers()

/**
 * @brief Resolves the user's home directory.
 *
 * @return Pointer to the home directory path or `NULL` when unavailable.
 */
static const char *xd_sh_resolve_home() {
  char *HOME = xd_vars_get("HOME");
  if (HOME == NULL) {
    struct passwd *pwd = getpwuid(getuid());
    if (pwd != NULL) {
      HOME = pwd->pw_dir;
    }
  }
  return HOME;
}  // xd_sh_resolve_home()

/**
 * @brief Pushes a source file onto the lexer's input stack.
 *
 * @param path Path to the file to be pushed.
 *
 * @return `0` on success, `-1` on failure.
 */
static int xd_sh_source_file(const char *path) {
  if (path == NULL) {
    return -1;
  }
  if (xd_utils_is_bin(path) == 1) {
    return -1;
  }
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    return -1;
  }
  xd_sh_is_interactive = 0;
  yylex_scan_file(file);
  return 0;
}  // xd_sh_source_file()

/**
 * @brief Pushes startup script files onto the lexer input stack.
 */
static void xd_sh_source_startup_files() {
  const char *home = xd_sh_resolve_home();
  if (home == NULL) {
    return;
  }
  char path[PATH_MAX];
  if (xd_sh_is_login) {
    snprintf(path, PATH_MAX, "%s/.xdsh_profile", home);
    xd_sh_source_file(path);
  }
  else if (xd_sh_is_interactive) {
    snprintf(path, PATH_MAX, "%s/.xdshrc", home);
    xd_sh_source_file(path);
  }
}  // xd_sh_source_startup_files()

/**
 * @brief Sets the default environment for the shell if not already defined.
 */
static void xd_sh_set_default_env() {
  struct passwd *pwd = getpwuid(getuid());

  if (xd_vars_get("HOME") == NULL && pwd != NULL && pwd->pw_dir != NULL) {
    xd_vars_put("HOME", pwd->pw_dir, 1);
  }
  if (xd_vars_get("USER") == NULL && pwd != NULL && pwd->pw_name != NULL) {
    xd_vars_put("USER", pwd->pw_name, 1);
  }
  if (xd_vars_get("LOGNAME") == NULL && pwd != NULL && pwd->pw_name != NULL) {
    xd_vars_put("LOGNAME", pwd->pw_name, 1);
  }
  if (xd_vars_get("PATH") == NULL) {
    xd_vars_put("PATH", XD_SH_DEF_PATH, 1);
  }

  char *shlvl_str = xd_vars_get("SHLVL");
  long shlvl = 0;
  if (shlvl_str != NULL) {
    xd_utils_strtol(shlvl_str, &shlvl);
  }
  long new_shlvl = xd_sh_is_login ? 1 : shlvl + 1;
  if (new_shlvl < 1) {
    new_shlvl = 1;
  }

  char shlvl_buf[XD_STR_DEF_CAP];
  snprintf(shlvl_buf, XD_STR_DEF_CAP, "%ld", new_shlvl);
  xd_vars_put("SHLVL", shlvl_buf, 1);
}  // xd_sh_set_default_env()

/**
 * @brief Runs the shell.
 *
 * @return returns the shell's exit code.
 */
static int xd_sh_run() {
  yyparse();
  return xd_sh_last_exit_code;
}  // xd_sh_run()

// ========================
// Public Functions
// ========================

void xd_sh_update_prompt() {
  struct passwd *pwd = getpwuid(getuid());
  const char *username = (pwd == NULL) ? "" : pwd->pw_name;
  char prompt_char = (strcmp(username, "root") == 0) ? '#' : '$';

  char hostname[PATH_MAX] = {0};
  if (gethostname(hostname, PATH_MAX) == -1) {
    hostname[0] = '\0';
  }

  char cwd[PATH_MAX] = {0};
  if (getcwd(cwd, PATH_MAX) == NULL) {
    cwd[0] = '\0';
  }

  const char *HOME = xd_vars_get("HOME");
  int home_len = (HOME == NULL) ? 0 : (int)strlen(HOME);

  int use_tilde = 0;
  if (home_len > 0 && HOME[home_len - 1] != '/' &&
      strncmp(HOME, cwd, home_len) == 0 &&
      (cwd[home_len] == '/' || cwd[home_len] == '\0')) {
    use_tilde = 1;
  }
  else {
    home_len = 0;
  }

  snprintf(xd_sh_prompt, XD_SH_PROMPT_MAX_LENGTH,
           XD_UTILS_CNSOL_FG_RED "%s@%s" XD_UTILS_CNSOL_RESET
                                 ":" XD_UTILS_CNSOL_FG_BLUE
                                 "%s%s" XD_UTILS_CNSOL_RESET "%c ",
           username, hostname, use_tilde ? "~" : "", cwd + home_len,
           prompt_char);
}  // xd_sh_update_prompt()

// ========================
// Main
// ========================

/**
 * @brief Program entry point.
 */
int main(int argc, char **argv) {
  xd_sh_init(argc, argv);
  return xd_sh_run();
}  // main()
