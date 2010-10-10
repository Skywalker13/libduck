/*
 * Duck: a Daisy 2.02 parser API.
 * Copyright (C) 2010 Mathieu Schroeter <mathieu@schroetersa.ch>
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

#ifndef DUCK_OSDEP_H
#define DUCK_OSDEP_H

#ifdef OSDEP_STRNDUP
char *dd_strndup (const char *s, size_t n);
#undef  strndup
#define strndup dd_strndup
#endif /* OSDEP_STRNDUP */
#ifdef OSDEP_STRCASESTR
char *dd_strcasestr (const char *haystack, const char *needle);
#undef  strcasestr
#define strcasestr dd_strcasestr
#endif /* OSDEP_STRCASESTR */
#ifdef OSDEP_STRTOK_R
char *dd_strtok_r (char *str, const char *delim, char **saveptr);
#undef  strtok_r
#define strtok_r dd_strtok_r
#endif /* OSDEP_STRTOK_R */

#endif /* DUCK_OSDEP_H */
