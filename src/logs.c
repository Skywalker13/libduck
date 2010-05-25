/*
 * Duck: a Daisy 2.02 parser API.
 * Copyright (C) 2010 Mathieu Schroeter <mathieu.schroeter@gamesover.ch>
 *
 * This file is part of libduck.
 *
 * libduck is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libduck is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libduck; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "duck.h"
#include "duck_internals.h"

#ifdef USE_LOGCOLOR
#define NORMAL   "\033[0m"
#define COLOR(x) "\033[" #x ";1m"
#define BOLD     COLOR(1)
#define F_RED    COLOR(31)
#define F_GREEN  COLOR(32)
#define F_YELLOW COLOR(33)
#define F_BLUE   COLOR(34)
#define B_RED    COLOR(41)
#endif /* USE_LOGCOLOR */

static duck_verb_t g_verbosity = DUCK_MSG_INFO;


void
dd_log_verb (duck_verb_t level)
{
  g_verbosity = level;
}

int
dd_log_test (duck_verb_t level)
{
  duck_verb_t verbosity;

  verbosity = g_verbosity;

  /* do we really want logging ? */
  if (verbosity == DUCK_MSG_NONE)
    return 0;

  if (level < verbosity)
    return 0;

  return 1;
}

void
dd_log_orig (duck_verb_t level, const char *format, ...)
{
#ifdef USE_LOGCOLOR
  static const char *const c[] = {
    [DUCK_MSG_VERBOSE]  = F_BLUE,
    [DUCK_MSG_INFO]     = F_GREEN,
    [DUCK_MSG_WARNING]  = F_YELLOW,
    [DUCK_MSG_ERROR]    = F_RED,
    [DUCK_MSG_CRITICAL] = B_RED,
  };
#endif /* USE_LOGCOLOR */
  static const char *const l[] = {
    [DUCK_MSG_VERBOSE]  = "Verb",
    [DUCK_MSG_INFO]     = "Info",
    [DUCK_MSG_WARNING]  = "Warn",
    [DUCK_MSG_ERROR]    = "Err",
    [DUCK_MSG_CRITICAL] = "Crit",
  };
  char fmt[256];
  va_list va;

  if (!format || !dd_log_test (level))
    return;

#ifdef USE_LOGCOLOR
  snprintf (fmt, sizeof (fmt),
            "[" BOLD "libduck" NORMAL "] [%%s:%%i] %s%s" NORMAL ": %s\n",
            c[level], l[level], format);
#else
  snprintf (fmt, sizeof (fmt),
            "[libduck] [%%s:%%i] %s: %s\n", l[level], format);
#endif /* USE_LOGCOLOR */

  va_start (va, format);
  vfprintf (stderr, fmt, va);
  va_end (va);
}
