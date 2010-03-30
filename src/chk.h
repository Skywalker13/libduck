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

#ifndef DUCK_CHK_H
#define DUCK_CHK_H

typedef struct chk_s chk_t;


chk_t *dd_chk_new (void);
void dd_chk_free (chk_t *chk);
void dd_chk_flush (chk_t *chk, const char *tag);
void dd_chk_add (chk_t *chk, const char *tag);
int dd_chk_ok (chk_t *chk, const char *tag, uint8_t pos);
uint8_t dd_chk_read (chk_t *chk, const char *tag, uint8_t pos);

#endif /* DUCK_CHK_H */
