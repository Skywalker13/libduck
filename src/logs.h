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

#ifndef DUCK_LOGS_H
#define DUCK_LOGS_H

void dd_log_verb (duck_verb_t level);
int dd_log_test (duck_verb_t level);
void dd_log_orig (duck_verb_t level, const char *format, ...);

#define dd_log(level, format, arg...) \
  dd_log_orig (level, format, __FILE__, __LINE__, ##arg)

#endif /* DUCK_LOGS_H */
