/*
 * ==============================================================================
 * File: xd_signals.c
 * Author: Duraid Maihoub
 * Date: 10 Aug 2025
 * Description: Part of the xd-shell project.
 * Repository: https://github.com/xduraid/xd-shell
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-shell is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#include "xd_signals.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xd_utils.h"

// ========================
// Macros
// ========================

/**
 * @brief Small buffer size for storing signal names.
 */
#define XD_SIGNAME_BUF_SIZE (32)

/**
 * @brief Number of columns when printing the list of signals.
 */
#define XD_SIGNALS_PRINT_ALL_COLS (5)

// ========================
// Typedefs
// ========================

/**
 * @brief Represents a signal.
 */
typedef struct xd_signal_t {
  int num;
  const char *name;
} xd_signal_t;

// ========================
// Function Declarations
// ========================

static char *xd_signals_name(int num);

// ========================
// Variables
// ========================

/**
 * @brief Array of defined signals (non-real-time).
 */
static const xd_signal_t xd_signals[] = {
    {SIGHUP,    "SIGHUP"   },
    {SIGINT,    "SIGINT"   },
    {SIGQUIT,   "SIGQUIT"  },
    {SIGILL,    "SIGILL"   },
    {SIGTRAP,   "SIGTRAP"  },
    {SIGABRT,   "SIGABRT"  },
    {SIGBUS,    "SIGBUS"   },
    {SIGFPE,    "SIGFPE"   },
    {SIGKILL,   "SIGKILL"  },
    {SIGUSR1,   "SIGUSR1"  },
    {SIGSEGV,   "SIGSEGV"  },
    {SIGUSR2,   "SIGUSR2"  },
    {SIGPIPE,   "SIGPIPE"  },
    {SIGALRM,   "SIGALRM"  },
    {SIGTERM,   "SIGTERM"  },
    {SIGSTKFLT, "SIGSTKFLT"},
    {SIGCHLD,   "SIGCHLD"  },
    {SIGCONT,   "SIGCONT"  },
    {SIGSTOP,   "SIGSTOP"  },
    {SIGTSTP,   "SIGTSTP"  },
    {SIGTTIN,   "SIGTTIN"  },
    {SIGTTOU,   "SIGTTOU"  },
    {SIGURG,    "SIGURG"   },
    {SIGXCPU,   "SIGXCPU"  },
    {SIGXFSZ,   "SIGXFSZ"  },
    {SIGVTALRM, "SIGVTALRM"},
    {SIGPROF,   "SIGPROF"  },
    {SIGWINCH,  "SIGWINCH" },
    {SIGIO,     "SIGIO"    },
    {SIGPWR,    "SIGPWR"   },
    {SIGSYS,    "SIGSYS"   },
};

/**
 * @brief Number of defined signals.
 */
static const int xd_signals_count = sizeof(xd_signals) / sizeof(xd_signals[0]);

/**
 * @brief Buffer to store the signal name to be returned.
 */
static char xd_signame_buf[XD_SIGNAME_BUF_SIZE] = {0};

// ========================
// Public Variables
// ========================

// ========================
// Function Definitions
// ========================

/**
 * @brief Returns the name of the signal with the passed number.
 *
 * @param num Number of the signal to return its name.
 *
 * @return A static string containing the signal name or `NULL` if not found.
 */
static char *xd_signals_name(int num) {
  for (int i = 0; i < xd_signals_count; i++) {
    if (xd_signals[i].num == num) {
      snprintf(xd_signame_buf, XD_SIGNAME_BUF_SIZE, "%s", xd_signals[i].name);
      return xd_signame_buf;
    }
  }

  if (num < SIGRTMIN || num > SIGRTMAX) {
    return NULL;
  }

  if (num == SIGRTMIN) {
    snprintf(xd_signame_buf, XD_SIGNAME_BUF_SIZE, "%s", "SIGRTMIN");
  }
  else if (num == SIGRTMAX) {
    snprintf(xd_signame_buf, XD_SIGNAME_BUF_SIZE, "%s", "SIGRTMAX");
  }
  else {
    int min = num - SIGRTMIN;
    int max = SIGRTMAX - num;
    if (min <= max) {
      snprintf(xd_signame_buf, XD_SIGNAME_BUF_SIZE, "SIGRTMIN+%d", min);
    }
    else {
      snprintf(xd_signame_buf, XD_SIGNAME_BUF_SIZE, "SIGRTMAX-%d", max);
    }
  }
  return xd_signame_buf;
}  // xd_signals_name()

// ========================
// Public Functions
// ========================

void xd_signals_print_all() {
  int counter = 0;
  for (int i = 1; i <= SIGRTMAX; i++) {
    char *signame = xd_signals_name(i);
    if (signame != NULL) {
      printf("%2d) %-11s ", i, signame);
      counter++;
    }

    if (counter % XD_SIGNALS_PRINT_ALL_COLS == 0 || i == SIGRTMAX) {
      printf("\n");
    }
  }
}  // xd_signals_print_all()

int xd_signals_signal_number(const char *sig) {
  if (sig == NULL) {
    return -1;
  }

  // check if passed string is an int signal number
  long signum = -1;
  if (xd_utils_strtol(sig, &signum) != -1 && signum > 0 && signum <= SIGRTMAX) {
    return (int)signum;
  }

  // skip prefix
  if (strncasecmp(sig, "SIG", 3) == 0) {
    sig += 3;
  }

  // lookup in signal table
  for (int i = 0; i < xd_signals_count; i++) {
    if (strcasecmp(sig, xd_signals[i].name + 3) == 0) {
      return xd_signals[i].num;
    }
  }

  // check RT signals

  if (strcasecmp(sig, "RTMIN") == 0) {
    return SIGRTMIN;
  }
  if (strcasecmp(sig, "RTMAX") == 0) {
    return SIGRTMAX;
  }

  const int len_rtmin = strlen("RTMIN+");
  if (strncasecmp(sig, "RTMIN+", len_rtmin) == 0 &&
      xd_utils_strtol(sig + len_rtmin, &signum) != -1 &&
      SIGRTMIN + signum <= SIGRTMAX) {
    return SIGRTMIN + signum;
  }

  const int len_rtmax = strlen("RTMAX-");
  if (strncasecmp(sig, "RTMAX-", len_rtmax) == 0 &&
      xd_utils_strtol(sig + len_rtmax, &signum) != -1 &&
      SIGRTMAX - signum >= SIGRTMIN) {
    return SIGRTMAX - signum;
  }

  // invalid signal string
  return -1;
}  // xd_signals_signal_number()
