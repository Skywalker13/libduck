/*
 * libduck: a Daisy 2.02 parser API.
 * Copyright (C) 2010 Mathieu Schroeter <mathieu.schroeter@gamesover.ch>
 *
 * This parser is freely and fully inspired of the DaisyProject library
 * http://developer.skolelinux.no/info/studentgrupper/2006-hig-daisyplayer/
 * written by
 *  André Lindhjem <belgarat@sdf.lonestar.org>
 *  Kjetil Holien <kjetil.holien@gmail.com>
 *  Terje Risa <terje.risa@gmail.com>
 *  Øyvind Nerbråten <oyvind@nerbraten.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/xmlreader.h>

#include "duck.h"
#include "duck_internals.h"
#include "utils.h"
#include "logs.h"
#include "chk.h"
#include "osdep.h"
#include "nccparser.h"


static smilnode_t *
smilnode_append (daisydata_t *data)
{
  smilnode_t *smilnode;

  smilnode = calloc (1, sizeof (smilnode_t));
  if (!smilnode)
    return NULL;

  if (data->smil_tail)
  {
    smilnode->prev = data->smil_tail;
    data->smil_tail->next = smilnode;
    data->smil_tail = smilnode;

    smilnode->item_id = smilnode->prev->item_id + 1;
  }
  else
  {
    data->smil_tail = smilnode;
    data->smil_head = smilnode;
    data->smil_pos  = smilnode;

    smilnode->item_id = 1;
  }

  return smilnode;
}

static void
meta_creator (daisydata_t *data, xmlChar *value)
{
  if (!data->book_info.author)
    data->book_info.author = strdup ((char *) value);
}

static void
meta_title (daisydata_t *data, xmlChar *value)
{
  if (!data->book_info.title_text)
    data->book_info.title_text = strdup ((char *) value);
}

static void
meta_narrator (daisydata_t *data, xmlChar *value)
{
  if (!data->book_info.narrator)
   data->book_info.narrator = strdup ((char *) value);
}

static void
meta_totaltime (daisydata_t *data, xmlChar *value)
{
  data->book_info.total_time = strdup ((char *) value);
}

/* The number of items in this array must be <= 64 */
static const struct {
  const char *str[4]; /* last ptr must be NULL */
  dd_type_t type;
  int qty;
  void (*fct) (daisydata_t *data, xmlChar *value);
} dd_metamap[] = {
  /* TODO: check all entries in the array */
  { { "dc:contributor"      },  TYPE_OPTIONAL,     1, NULL                  },
  { { "dc:creator"          },  TYPE_MANDATORY,    1, meta_creator          },
  { { "dc:coverage"         },  TYPE_OPTIONAL,     1, NULL                  },
  { { "dc:date"             },  TYPE_MANDATORY,    1, NULL                  },
  { { "dc:description"      },  TYPE_OPTIONAL,     1, NULL                  },
  { { "dc:format",
      "ncc:format"          },  TYPE_MANDATORY,    1, NULL                  },
  { { "dc:identifier",
      "ncc:identifier"      },  TYPE_MANDATORY,    1, NULL                  },
  { { "dc:language"         },  TYPE_MANDATORY,    1, NULL                  },
  { { "dc:publisher"        },  TYPE_MANDATORY,    1, NULL                  },
  { { "dc:relation"         },  TYPE_OPTIONAL,     1, NULL                  },
  { { "dc:rights"           },  TYPE_OPTIONAL,     1, NULL                  },
  { { "dc:source"           },  TYPE_RECOMMENDED,  1, NULL                  },
  { { "dc:subject"          },  TYPE_RECOMMENDED,  1, NULL                  },
  { { "dc:title"            },  TYPE_MANDATORY,    1, meta_title            },
  { { "dc:type"             },  TYPE_OPTIONAL,     1, NULL                  },
  { { "ncc:charset"         },  TYPE_MANDATORY,    1, NULL                  },
  { { "ncc:depth"           },  TYPE_RECOMMENDED,  1, NULL                  },
  { { "ncc:files"           },  TYPE_RECOMMENDED,  1, NULL                  },
  /* FIXME: 'ncc:footnotes' is mandatory if footnote is used */
  { { "ncc:footnotes"       },  TYPE_OPTIONAL,     1, NULL                  },
  { { "ncc:generator"       },  TYPE_OPTIONAL,     1, NULL                  },
  { { "ncc:kByteSize"       },  TYPE_OPTIONAL,     1, NULL                  },
  { { "ncc:maxPageNormal"   },  TYPE_RECOMMENDED,  1, NULL                  },
  { { "ncc:multimediaType"  },  TYPE_RECOMMENDED,  1, NULL                  },
  { { "ncc:narrator"        },  TYPE_RECOMMENDED,  1, meta_narrator         },
  { { "ncc:pageFront"       },  TYPE_MANDATORY,    1, NULL                  },
  { { "ncc:pageNormal"      },  TYPE_MANDATORY,    1, NULL                  },
  /* FIXME: 'ncc:prodNotes' mandatory if producer's note */
  { { "ncc:prodNotes"       },  TYPE_OPTIONAL,     1, NULL                  },
  { { "ncc:producer"        },  TYPE_OPTIONAL,     1, NULL                  },
  { { "ncc:producedDate"    },  TYPE_OPTIONAL,     1, NULL                  },
  { { "ncc:revision"        },  TYPE_OPTIONAL,     1, NULL                  },
  { { "ncc:revisionDate"    },  TYPE_OPTIONAL,     1, NULL                  },
  /* FIXME: 'ncc:setInfo' mandatory if >1 volume */
  { { "ncc:setInfo"         },  TYPE_RECOMMENDED,  1, NULL                  },
  /* FIXME: 'ncc:sidebars' mandatory if sidebar */
  { { "ncc:sidebars"        },  TYPE_RECOMMENDED,  1, NULL                  },
  { { "ncc:sourceDate"      },  TYPE_RECOMMENDED,  1, NULL                  },
  { { "ncc:sourceEdition"   },  TYPE_RECOMMENDED,  1, NULL                  },
  { { "ncc:sourcePublisher" },  TYPE_RECOMMENDED,  1, NULL                  },
  { { "ncc:sourceRights"    },  TYPE_OPTIONAL,     1, NULL                  },
  { { "ncc:tocItems",
      "ncc:tocitems",
      "ncc:TOCitems"        },  TYPE_MANDATORY,    1, NULL                  },
  { { "ncc:totalTime",
      "ncc:totaltime"       },  TYPE_MANDATORY,    1, meta_totaltime        },
  { { "http-equiv"          },  TYPE_OPTIONAL,     1, NULL                  },
};

static int
ncc_parse_meta (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk)
{
  uint8_t i;
  xmlChar *attrvalue = NULL, *attrcontent = NULL;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  attrvalue = xmlTextReaderGetAttribute (reader, (xmlChar *) "name");
  if (!attrvalue)
    goto out;

  attrcontent = xmlTextReaderGetAttribute (reader, (xmlChar *) "content");
  if (!attrcontent)
    goto out;

  for (i = 0; i < ARRAY_NB_ELEMENTS (dd_metamap); i++)
  {
    uint8_t j;
    for (j = 0; dd_metamap[i].str[j]; j++)
      if (!strcmp (dd_metamap[i].str[j], (char *) attrvalue))
        break;

    if (!dd_metamap[i].str[j])
      continue;

    dd_chk_ok (chk, "meta", i);

    if (!dd_metamap[i].fct)
      break;

    dd_log (DUCK_MSG_INFO,
            "parsing of <meta> %s : %s", attrvalue, attrcontent);
    dd_metamap[i].fct (data, attrcontent);

    if (j) /* syntax obsolete ? */
      dd_log (DUCK_MSG_WARNING,
              "<meta> %s should be rewritten to %s",
              attrvalue, dd_metamap[i].str[j]);
    break;
  }

 out:
  if (attrvalue)
    xmlFree (attrvalue);
  if (attrcontent)
    xmlFree (attrcontent);
  return xmlTextReaderRead (reader);
}

static int
ncc_parse_title (xmlTextReaderPtr reader,
                 daisydata_t *data, dd_unused chk_t *chk)
{
  xmlChar *value = NULL;
  int ret = 1;

  dd_log (DUCK_MSG_VERBOSE, "ncc parsing <title>");

  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  if (xmlTextReaderHasValue (reader))
  {
    value = xmlTextReaderValue (reader);
    data->book_info.title_text = dd_trimwhitespaces ((char *) value);

    if (value)
      xmlFree (value);
  }

  return xmlTextReaderRead (reader);
}

static int
ncc_parse_a (xmlTextReaderPtr reader, smilnode_t *smilnode)
{
  xmlChar *smilsrc = NULL, *tmp = NULL, *value = NULL;
  int ret = 1;
  char *tok = NULL;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  /* get href */
  smilsrc = xmlTextReaderGetAttribute (reader, (xmlChar *) "href");
  if (!smilsrc)
  {
    dd_log (DUCK_MSG_WARNING, "mal-formed <a> tag");
    return -1;
  }

  tmp = (xmlChar *) strtok_r ((char *) smilsrc, "#", &tok);
  dd_log (DUCK_MSG_INFO, "ncc parsing <a> file anchor: %s", tmp);
  smilnode->anchor = strdup ((char *) tmp);

  /* get identifier */
  tmp = (xmlChar *) strtok_r (NULL, "#", &tok);
  dd_log (DUCK_MSG_INFO, "ncc parsing <a> fragment identifier: %s", tmp);
  smilnode->fragment_identifier = strdup ((char *) tmp);

  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  if (smilsrc)
    xmlFree (smilsrc);

  if (xmlTextReaderHasValue (reader))
  {
    value = xmlTextReaderValue (reader);
    smilnode->header = dd_trimwhitespaces ((char *) value);

    dd_log (DUCK_MSG_INFO, "ncc parsing <a> heading: %s", value);

    if (value)
      xmlFree (value);
  }

  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  ret = xmlTextReaderRead (reader);
  return ret;
}

static int
ncc_parse_common (xmlTextReaderPtr reader, smilnode_t *smilnode)
{
  const xmlChar *name = NULL;
  xmlChar *attrvalue = NULL;
  int ret = 1;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  attrvalue = xmlTextReaderGetAttribute (reader, (xmlChar *) "id");
  if (attrvalue)
    smilnode->id = (char *) attrvalue;

  /* next tag */
  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  name = xmlTextReaderConstName (reader);
  if (!name)
    return 0;

  if (!strcasecmp ((char *) name, "a"))
    ret = ncc_parse_a (reader, smilnode);
  else
    ret = -1;

  return ret;
}

static int
ncc_parse_hx (xmlTextReaderPtr reader,
              smilnode_t *smilnode, const xmlChar *name)
{
  /*
   * TODO: Hx must contain (attributes) id, h1 must contain class,
   * h2-6 might contain class. Not taken care of now
   */
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  smilnode->type  = DUCK_SMILNODE_HEADING;
  smilnode->level = (int) ((char) name[1] - 48);
  return ncc_parse_common (reader, smilnode);
}

static int
ncc_parse_span (xmlTextReaderPtr reader,
                smilnode_t *smilnode, dd_unused const xmlChar *name)
{
  /* Span must contain id and class attributes */
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  smilnode->type = DUCK_SMILNODE_PAGE;
  return ncc_parse_common (reader, smilnode);
}

static int
ncc_parse_div (xmlTextReaderPtr reader,
               smilnode_t *smilnode, dd_unused const xmlChar *name)
{
  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  smilnode->type = DUCK_SMILNODE_BLOCK;
  return ncc_parse_common (reader, smilnode);
}

static const struct {
  const char *str;
  dd_type_t type;
  int qty;
  int (*fct) (xmlTextReaderPtr reader,
              smilnode_t *smilnode, const xmlChar *name);
} dd_bodymap[] = {
  { "h1",       TYPE_MANDATORY, -1,  ncc_parse_hx    },
  { "h2",       TYPE_OPTIONAL,  -1,  ncc_parse_hx    },
  { "h3",       TYPE_OPTIONAL,  -1,  ncc_parse_hx    },
  { "h4",       TYPE_OPTIONAL,  -1,  ncc_parse_hx    },
  { "h5",       TYPE_OPTIONAL,  -1,  ncc_parse_hx    },
  { "h6",       TYPE_OPTIONAL,  -1,  ncc_parse_hx    },
  { "span",     TYPE_OPTIONAL,  -1,  ncc_parse_span  },
  { "div",      TYPE_OPTIONAL,  -1,  ncc_parse_div   },
};

static int
ncc_parse_body (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk)
{
  const xmlChar *name = NULL;
  int ret = 1, type;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  type = xmlTextReaderNodeType (reader);
  if (type != 1)
  {
    dd_log (DUCK_MSG_ERROR, "failed NCC <body>");
    return -1;
  }

  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  name = xmlTextReaderConstName (reader);
  if (!name)
    return 0;

  /* while we haven't come to the body closetag */
  while (strcasecmp ((char *) name, "body"))
  {
    uint8_t i;

    NCC_CHECK_FOR (dd_bodymap, i)
    {
      smilnode_t *smilnode = smilnode_append (data);
      ret = dd_bodymap[i].fct (reader, smilnode, name);
      dd_chk_ok (chk, "body", i);
      break;
    }

    NCC_CHECK_OUT (dd_bodymap, name, i)

    if (ret != 1)
      return ret;

    ret = xmlTextReaderRead (reader);
    if (ret != 1)
      return ret;

    name = xmlTextReaderConstName (reader);
    if (!name)
      return 0;
  }

  NCC_CHECK (chk, "body", dd_bodymap,  )
  dd_chk_flush (chk, "body");

  type = xmlTextReaderNodeType (reader);
  if (type != 15)
  {
    dd_log (DUCK_MSG_WARNING, "failed parsing NCC <body>, endtag expected");
    return -1;
  }

  return ret;
}

static const struct {
  const char *str;
  dd_type_t type;
  int qty;
  int (*fct) (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk);
} dd_headmap[] = {
  /* TODO: check all entries in the array */
  { "title",    TYPE_MANDATORY,  1,   ncc_parse_title },
  { "meta",     TYPE_MANDATORY, -1,   ncc_parse_meta  },
};

static int
ncc_parse_head (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk)
{
  const xmlChar *name = NULL;
  int ret = 1, type;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  type = xmlTextReaderNodeType (reader);
  if (type != 1)
  {
    dd_log (DUCK_MSG_WARNING, "failed NCC <head>");
    return -1;
  }

  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  name = xmlTextReaderConstName (reader);
  if (!name)
    return 0;

  /* while we haven't come to the head closetag */
  while (strcasecmp ((char *) name, "head"))
  {
    uint8_t i;

    NCC_CHECK_FOR (dd_headmap, i)
    {
      ret = dd_headmap[i].fct (reader, data, chk);
      dd_chk_ok (chk, "head", i);
      break;
    }

    NCC_CHECK_OUT (dd_headmap, name, i)

    if (ret != 1)
      return ret;

    ret = xmlTextReaderRead (reader);
    if (ret != 1)
      return ret;

    name = xmlTextReaderConstName (reader);
    if (!name)
      return 0;
  }

  /*
   * Special check for properties, it is necessary to wait that the whole
   * metadata are parsed.
   */
  NCC_CHECK (chk, "meta", dd_metamap, *)
  dd_chk_flush (chk, "meta");

  NCC_CHECK (chk, "head", dd_headmap,  )
  dd_chk_flush (chk, "head");

  type = xmlTextReaderNodeType (reader);
  if (type != 15)
  {
    dd_log (DUCK_MSG_WARNING, "failed parsing NCC <head>, endtag expected");
    return -1;
  }

  return ret;
}

static const struct {
  const char *str;
  dd_type_t type;
  int qty;
  int (*fct) (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk);
} dd_rootmap[] = {
  /* TODO: check all entries in the array */
  { "html",   TYPE_MANDATORY,  0,  NULL            },
  { "head",   TYPE_MANDATORY,  1,  ncc_parse_head  },
  { "body",   TYPE_MANDATORY,  1,  ncc_parse_body  },
};

/*
 * Function for parsing a daisy 2.02 ncc file (ncc.*). Builds a linked list in
 * the main data structure containing information about all smil files. The
 * linked list represents the spine of the daisy DTB.
 */
int
dd_ncc_parse (daisydata_t *data, const char *path)
{
  const xmlChar *name = NULL;
  xmlTextReaderPtr reader = NULL;
  int ret = 1;
  chk_t *chk = NULL;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  if (!data || !path)
    return -1;

  /* open the ncc file */
  reader = xmlReaderForFile (path, NULL, 0);
  if (!reader)
  {
    dd_log (DUCK_MSG_ERROR, "Failed to open NCC file: %s", path);
    return -1;
  }

  dd_log (DUCK_MSG_INFO, "ncc parsing file: %s", path);

  /* read the first tag */
  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    goto err;

  chk = dd_chk_new ();
  if (!chk)
    goto err;

  dd_chk_add (chk, "root");
  dd_chk_add (chk, "head");
  dd_chk_add (chk, "body");
  dd_chk_add (chk, "meta");

  /* while there are more tags, parse the next */
  while (ret == 1)
  {
    uint8_t i;

    /* get name of tag */
    name = xmlTextReaderConstName (reader);
    if (!name)
    {
      ret = 0;
      goto out;
    }

    NCC_CHECK_FOR (dd_rootmap, i)
    {
      if (dd_rootmap[i].fct)
        ret = dd_rootmap[i].fct (reader, data, chk);
      dd_chk_ok (chk, "root", i);
      break;
    }

    NCC_CHECK_OUT (dd_rootmap, name, i)

    if (ret != 1)
      goto err;

    ret = xmlTextReaderRead (reader);
  }

  NCC_CHECK (chk, "root", dd_rootmap,  )

  if (ret)
    goto err;

 out:
  dd_chk_free (chk);
  xmlFreeTextReader (reader);
  return ret;

 err:
  dd_log (DUCK_MSG_WARNING, "Failed to parse NCC file: %s", path);
  dd_chk_free (chk);
  xmlFreeTextReader (reader);
  return -1;
}
