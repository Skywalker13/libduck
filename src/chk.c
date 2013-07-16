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

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "duck.h"
#include "logs.h"
#include "chk.h"

#define CHK_LIMIT 64

struct chk_s {
  struct tag_s {
    char     *tag;
    uint8_t   chk[CHK_LIMIT];
    uint8_t   nb;
  } *tags;
};


chk_t *
dd_chk_new (void)
{
  return calloc (1, sizeof (chk_t));
}

void
dd_chk_free (chk_t *chk)
{
  if (!chk)
    return;

  if (chk->tags)
  {
    struct tag_s *it;
    for (it = chk->tags; it->tag; it++)
      free (it->tag);
    free (chk->tags);
  }
  free (chk);
}

void
dd_chk_flush (chk_t *chk, const char *tag)
{
  struct tag_s *it;

  if (!chk || !chk->tags)
    return;

  for (it = chk->tags; it->tag; it++)
  {
    if (!tag || !strcasecmp (tag, it->tag))
    {
      memset (it->chk, 0, sizeof (it->chk));
      if (tag)
        break;
    }
  }
}

void
dd_chk_add (chk_t *chk, const char *tag)
{
  size_t size = 0;
  struct tag_s *tmp;
  
  if (!dd_log_test (DUCK_MSG_WARNING))
    return;

  if (!tag)
    return;

  for (tmp = chk->tags; tmp && tmp->tag; tmp++)
    size++;
  size++;

  tmp = chk->tags;
  chk->tags = realloc (chk->tags, (size + 1) * sizeof (*chk->tags));
  if (chk->tags)
  {
    chk->tags[size].tag = NULL;
    chk->tags[size - 1].tag = strdup (tag);
    memset (chk->tags[size - 1].chk, 0, sizeof (chk->tags[size - 1].chk));
  }
  else
    chk->tags = tmp; /* restore */
}

int
dd_chk_ok (chk_t *chk, const char *tag, uint8_t pos)
{
  struct tag_s *it;

  if (!chk || !chk->tags || !tag)
    return 0;

  if (pos >= CHK_LIMIT)
    return 0;

  for (it = chk->tags; it->tag; it++)
    if (!strcasecmp (tag, it->tag))
    {
      it->chk[pos]++;
      return 1;
    }

  return 0;
}

uint8_t
dd_chk_read (chk_t *chk, const char *tag, uint8_t pos)
{
  struct tag_s *it;

  if (!chk || !chk->tags || !tag)
    return 0;

  if (pos >= CHK_LIMIT)
    return 0;

  for (it = chk->tags; it->tag; it++)
    if (!strcasecmp (tag, it->tag))
      return it->chk[pos];

  return 0;
}
