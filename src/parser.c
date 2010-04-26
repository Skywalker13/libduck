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
#include "logs.h"
#include "utils.h"
#include "osdep.h"
#include "nccparser.h"
#include "nccsmilparser.h"
#include "parser.h"


int
dd_parser_smilload (daisydata_t *data)
{
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!data)
    return -1;

  dd_node_flush (data);

  switch (data->format)
  {
  case DUCK_FORMAT_NCC:
    return dd_nccsmil_parse (data);

  case DUCK_FORMAT_NCX:
  default:
    break;
  }

  return -1;
}

#define NCX_PATTERN_L ".ncx"
#define NCX_PATTERN_U ".NCX"
#define NCC_PATTERN_L "ncc.html"
#define NCC_PATTERN_U "NCC.HTML"

#define NCX_STRLEN (sizeof (NCX_PATTERN_L) - 1)
#define NCC_STRLEN (sizeof (NCC_PATTERN_L) - 1)

daisydata_t *
dd_parser_load (const char *path, duck_format_t format)
{
  daisydata_t *data;
  size_t len;
  unsigned int i;
  static const struct {
    const char   *pattern;
    size_t        min_size;
    duck_format_t format;
  } mapping[] = {
    { NCX_PATTERN_L,  NCX_STRLEN,  DUCK_FORMAT_NCX },
    { NCX_PATTERN_U,  NCX_STRLEN,  DUCK_FORMAT_NCX },
    { NCC_PATTERN_L,  NCC_STRLEN,  DUCK_FORMAT_NCC },
    { NCC_PATTERN_U,  NCC_STRLEN,  DUCK_FORMAT_NCC },
  };

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  data = calloc (1, sizeof (daisydata_t));
  if (!data)
    return NULL;

  data->format = format;
  len = strlen (path);

  if (format == DUCK_FORMAT_AUTO)
    for (i = 0; i < ARRAY_NB_ELEMENTS (mapping); i++)
      if (len >= mapping[i].min_size
          && !strcmp (path + len - mapping[i].min_size, mapping[i].pattern))
      {
        data->format = mapping[i].format;
        break;
      }

  switch (data->format)
  {
  /* TODO */
  case DUCK_FORMAT_NCX:
    return NULL;

  /* 2.02 Daisy book */
  case DUCK_FORMAT_NCC:
  {
    int rc;
    char *it;

    it = strrchr (path, '/');
    data->path = it ? strndup (path, it - path + 1) : strdup ("./");
    if (!data->path)
      return NULL;

    rc = dd_ncc_parse (data, path);
    data->integrity = !!rc;
    break;
  }

  default:
    dd_log (DUCK_MSG_WARNING, "the file \"%s\" is unknown", path);
    return NULL;
  }

  return data;
}
