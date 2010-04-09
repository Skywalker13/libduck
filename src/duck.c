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

#include "duck.h"
#include "duck_internals.h"
#include "parser.h"
#include "utils.h"
#include "logs.h"


unsigned int
libduck_version (void)
{
  return LIBDUCK_VERSION_INT;
}

void
duck_verbosity (duck_verb_t level)
{
  dd_log_verb (level);
}

void
duck_uninit (duck_t *handle)
{
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!handle)
    return;

  dd_daisydata_free (handle->data);
  free (handle);
}

duck_t *
duck_init (duck_init_param_t *param)
{
  duck_t *handle;
  duck_init_param_t p;
  const duck_init_param_t *pp = &p;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (param)
    pp = param;
  else
    memset (&p, 0, sizeof (p));

  handle = calloc (1, sizeof (duck_t));
  if (!handle)
    return NULL;

  return handle;
}

int
duck_getpos (duck_t *handle, int *smilpos, int *nodepos)
{
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!handle || !handle->data)
    return -1;

  if (smilpos)
  {
    if (!handle->data->smil_pos)
      return -1;
    *smilpos = handle->data->smil_pos->item_id;
  }

  if (nodepos)
  {
    if (!handle->data->node_pos)
      return -1;
    *nodepos = handle->data->node_pos->item_id;
  }

  return 0;
}

int
duck_getheading (duck_t *handle, duck_hx_t *hx)
{
  const smilnode_t *smil_n;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!handle || !handle->data || !hx)
    return -1;

  memset (hx, 0, sizeof (*hx));

  smil_n = handle->data->smil_head;
  for (; smil_n; smil_n = smil_n->next)
  {
    if (smil_n->type != DUCK_SMILNODE_HEADING)
      continue;

    switch (smil_n->level)
    {
    case 1:
      hx->h1++;
      hx->h2 = hx->h3 = hx->h4 = hx->h5 = hx->h6 = 0;
      break;

    case 2:
      hx->h2++;
      hx->h3 = hx->h4 = hx->h5 = hx->h6 = 0;
      break;

    case 3:
      hx->h3++;
      hx->h4 = hx->h5 = hx->h6 = 0;
      break;

    case 4:
      hx->h4++;
      hx->h5 = hx->h6 = 0;
      break;

    case 5:
      hx->h5++;
      hx->h6 = 0;
      break;

    case 6:
      hx->h6++;
      break;

    default:
      return -1;
    }

    if (smil_n == handle->data->smil_pos)
      break;
  }

  return 0;
}

int
duck_walk (duck_t *handle, int smilpos, int nodepos)
{
  unsigned int direction;
  smilnode_t *smil_n;
  node_t *node_n;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!handle || !handle->data)
    return -1;

  smil_n = handle->data->smil_pos;
  if (!smil_n)
    return -1;

  /* goto SMIL position */
  if (smilpos && smil_n->item_id != smilpos)
  {
    if (smilpos > smil_n->item_id)
      direction = 1;
    else
      direction = 0;

    while (smil_n && smil_n->item_id != smilpos)
      smil_n = direction ? smil_n->next : smil_n->prev;

    if (!smil_n) /* out of bound */
      return -1;

    handle->data->smil_pos = smil_n;
    /* the node is loaded on demand, then flush it */
    dd_node_flush (handle->data);
  }

  if (!nodepos)
    return 0;

  if (!handle->data->node_pos)
    dd_parser_smilload (handle->data);

  node_n = handle->data->node_pos;
  if (!node_n)
    return -1;

  /* goto NODE position */
  if (node_n->item_id != nodepos)
  {
    if (nodepos > node_n->item_id)
      direction = 1;
    else
      direction = 0;

    while (node_n && node_n->item_id != nodepos)
      node_n = direction ? node_n->next : node_n->prev;

    if (!node_n) /* out of bound */
      return -1;

    handle->data->node_pos = node_n;
  }

  return 0;
}

int
duck_load (duck_t *handle, const char *path, duck_format_t format)
{
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!handle || !path)
    return -1;

  if (handle->data)
    dd_daisydata_free (handle->data);

  handle->data = dd_parser_load (path, format);
  if (!handle->data)
    return -1;

  return 0;
}

int
duck_smilnode_number (duck_t *handle)
{
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!handle || !handle->data || !handle->data->smil_tail)
    return -1;

  return handle->data->smil_tail->item_id;
}

int
duck_book_getinfo (duck_t *handle, duck_book_info_t sel, duck_value_t *res)
{
  bookinfo_t *book;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!handle || !handle->data || !res)
    return -1;

  book = &handle->data->book_info;

  switch (sel)
  {
  case DUCK_BOOK_S_NARRATOR:
    if (book->narrator)
      res->s = strdup (book->narrator);
    break;

  case DUCK_BOOK_S_TITLE:
    if (book->title_text)
      res->s = strdup (book->title_text);
    break;

  default:
    return -1;
  }

  return 0;
}

int
duck_smilnode_getinfo (duck_t *handle,
                       duck_smilnode_info_t sel, duck_value_t *res)
{
  smilnode_t *smil_n;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!handle || !handle->data || !res)
    return -1;

  smil_n = handle->data->smil_pos;
  if (!smil_n)
    return -1;

  switch (sel)
  {
  case DUCK_SMILNODE_S_ANCHOR:
    if (smil_n->anchor)
      res->s = strdup (smil_n->anchor);
    break;

  case DUCK_SMILNODE_S_HEADER:
    if (smil_n->header)
      res->s = strdup (smil_n->header);
    break;

  case DUCK_SMILNODE_I_INDEX:
    res->i = smil_n->item_id;
    break;

  case DUCK_SMILNODE_I_LEVEL:
    res->i = smil_n->level;
    break;

  case DUCK_SMILNODE_I_TYPE:
    res->i = smil_n->type;
    break;

  default:
    return -1;
  }

  return 0;
}

int
duck_node_getinfo (duck_t *handle,
                   duck_node_info_t sel, duck_value_t *res)
{
  node_t *node_n;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!handle || !handle->data || !res)
    return -1;

  node_n = handle->data->node_pos;
  if (!node_n)
    return -1;

  switch (sel)
  {
  case DUCK_NODE_S_AUDIO_URI:
    if (node_n->audio_uri)
      res->s = strdup (node_n->audio_uri);
    break;

  case DUCK_NODE_I_AUDIO_POS_START:
    res->i = node_n->audio_pos_start;
    break;

  case DUCK_NODE_I_AUDIO_POS_STOP:
    res->i = node_n->audio_pos_stop;
    break;

  case DUCK_NODE_I_INDEX:
    res->i = node_n->item_id;
    break;

  default:
    return -1;
  }

  return 0;
}
