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
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "osdep.h"


#ifdef OSDEP_STRNDUP
char *
dd_strndup (const char *s, size_t n)
{
  char *res;
  size_t length;

  length = strlen (s);
  if (length > n)
    length = n;

  res = malloc (length + 1);
  if (!res)
    return NULL;

  memcpy (res, s, length);
  res[length] = '\0';

  return res;
}
#endif /* OSDEP_STRNDUP */

#ifdef OSDEP_STRCASESTR
char *
dd_strcasestr (const char *haystack, const char *needle)
{
  size_t length;

  length = strlen (needle);
  if (!length)
    return (char *) haystack;

  for (; *haystack; haystack++)
    if (   tolower ((int) (unsigned char) *haystack)
        == tolower ((int) (unsigned char) *needle))
      if (!strncasecmp (haystack, needle, length))
        return (char *) haystack;

  return NULL;
}
#endif /* OSDEP_STRCASESTR */

#ifdef OSDEP_STRTOK_R
char *
dd_strtok_r (char *str, const char *delim, char **saveptr)
{
  char *token;

  if (str)
    *saveptr = str;
  token = *saveptr;

  if (!token)
    return NULL;

  token += strspn (token, delim);
  *saveptr = strpbrk (token, delim);
  if (*saveptr)
    *(*saveptr)++ = '\0';

  return *token ? token : NULL;
}
#endif /* OSDEP_STRTOK_R */
