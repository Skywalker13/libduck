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

#ifndef DUCK_UITLS_H
#define DUCK_UTILS_H

char *dd_trimwhitespaces (const char *str);
double dd_atof (const char *nptr);
int dd_strtime2int (const char *time);

void dd_smilnode_free (smilnode_t *smilnode);
void dd_node_free (node_t *node);
void dd_daisydata_free (daisydata_t *data);

void dd_node_flush (daisydata_t *data);
void dd_smilnode_flush (daisydata_t *data);

#endif /* DUCK_UTILS_H */
