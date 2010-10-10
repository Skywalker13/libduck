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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "duck.h"
#include "duck_internals.h"
#include "logs.h"
#include "osdep.h"
#include "utils.h"


char *
dd_trimwhitespaces (const char *str)
{
  size_t len;
  const char *it, *s;

  if (!str)
    return NULL;

  it = str;
  len = strlen (str);

  /* start */
  while (*it == ' ' || *it == '\t' || *it == '\n' || *it == '\r')
    it++;

  s = it;

  /* end */
  it = str + len - 1;
  while (*it == ' ' || *it == '\t' || *it == '\n' || *it == '\r')
    it--;

  return strndup (s, it - s + 1);
}

double
dd_atof (const char *nptr)
{
  double div = 1.0;
  int res, integer;
  unsigned int frac = 0, start = 0, end = 0;

  while (*nptr && !isdigit ((int) (unsigned char) *nptr) && *nptr != '-')
  nptr++;

  if (!*nptr)
    return 0.0;

  res = sscanf (nptr, "%i.%n%u%n", &integer, &start, &frac, &end);
  if (res < 1)
    return 0.0;

  if (!frac)
    return (double) integer;

  if (integer < 0)
    div = -div;

  div *= pow (10.0, end - start);
  return integer + frac / div;
}

int
dd_strtime2int (const char *time)
{
  int rc, h, m, s;

  if (!time)
    return 0;

  rc = sscanf (time, "%u:%u:%u", &h, &m, &s);
  if (rc != 3)
    return 0;

  return h * 3600 + m * 60 + s;
}

void
dd_smilnode_free (smilnode_t *smilnode)
{
  smilnode_t *tmp;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  while (smilnode)
  {
    tmp = smilnode->next;
    if (smilnode->id)
      free (smilnode->id);
    if (smilnode->anchor)
      free (smilnode->anchor);
    if (smilnode->header)
      free (smilnode->header);
    if (smilnode->fragment_identifier)
      free (smilnode->fragment_identifier);
    if (smilnode->text_passage)
      free (smilnode->text_passage);
    if (smilnode->image)
      free (smilnode->image);
    if (smilnode->time)
      free (smilnode->time);
    if (smilnode->elapsed_time)
      free (smilnode->elapsed_time);
    free (smilnode);
    smilnode = tmp;
  }
}

void
dd_smilnode_flush (daisydata_t *data)
{
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!data || !data->smil_head)
    return;

  dd_smilnode_free (data->smil_head);

  data->smil_head = NULL;
  data->smil_tail = NULL;
  data->smil_pos  = NULL;
}

void
dd_node_free (node_t *node)
{
  node_t *tmp;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  while (node)
  {
    tmp = node->next;
    if (node->audio_uri)
      free (node->audio_uri);
    if (node->text_filename)
      free (node->text_filename);
    if (node->fragment_identifier)
      free (node->fragment_identifier);
    if (node->text_passage)
      free (node->text_passage);
    if (node->image)
      free (node->image);
    free (node);
    node = tmp;
  }
}

void
dd_node_flush (daisydata_t *data)
{
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!data || !data->node_head)
    return;

  dd_node_free (data->node_head);

  data->node_head = NULL;
  data->node_tail = NULL;
  data->node_pos  = NULL;
}

void
dd_daisydata_free (daisydata_t *data)
{
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!data)
    return;

  if (data->smil_head)
    dd_smilnode_free (data->smil_head);
  if (data->node_head)
    dd_node_free (data->node_head);

  if (data->book_info.title_text)
    free (data->book_info.title_text);
  if (data->book_info.total_time)
    free (data->book_info.total_time);
  if (data->book_info.narrator)
    free (data->book_info.narrator);
  if (data->book_info.author)
    free (data->book_info.author);

  if (data->path)
    free (data->path);

  free (data);
}

