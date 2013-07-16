/*
 * libduck: a Daisy 2.02 parser API.
 * Copyright (C) 2010 Mathieu Schroeter <mathieu@schroetersa.ch>
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
#include "logs.h"
#include "utils.h"
#include "chk.h"
#include "osdep.h"
#include "nccsmilparser.h"

#define BUFFER_STR  4096


static node_t *
node_append (daisydata_t *data)
{
  node_t *node;

  node = calloc (1, sizeof (node_t));
  if (!node)
    return NULL;

  if (data->node_tail)
  {
    node->prev = data->node_tail;
    data->node_tail->next = node;
    data->node_tail = node;

    node->item_id = node->prev->item_id + 1;
  }
  else
  {
    data->node_tail = node;
    data->node_head = node;
    data->node_pos  = node;

    node->item_id = 1;
  }

  return node;
}

static void
node_cancel (daisydata_t *data)
{
  node_t *node;

  if (!data->node_tail)
    return;
  
  node = data->node_tail->prev;
  
  if (!node)
    data->node_head = NULL;
  
  if (data->node_pos == data->node_tail)
    data->node_pos = NULL;
  
  dd_node_free (data->node_tail);
  data->node_tail = node;
}

static int
smil_parse_text (xmlTextReaderPtr reader,
                 daisydata_t *data, node_t *tmp_node, dd_unused chk_t *chk)
{
  xmlChar *attr = NULL;
  const xmlChar *name = NULL;
  char *textsrc = NULL, *fragment = NULL, *tok = NULL;
  int ret = 1;
  /*
   * Attributes:
   *  src
   *  id
   *  region
   */

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  /* go to the fragment identifier point and parses from there. */
  if (!data->smilfound)
  {
    xmlChar *tmp;

    tmp = xmlTextReaderGetAttribute (reader, (xmlChar *) "id");

    /*
     * If the text tag has an id attribute, read it and check wheter it matches
     * the fragmentid. If there is none id attribute, set alreadyfound to 1 and
     * parse the whole smilfile.
     */
    if (tmp)
    {
      if (data->smil_pos->fragment_identifier)
      {
        if (!strcmp ((char *) tmp, data->smil_pos->fragment_identifier))
          data->smilfound = 1;
        else
          dd_log (DUCK_MSG_VERBOSE,
                  "fragment id (%s) not found in this TEXT tag",
                  data->smil_pos->fragment_identifier);
      }

      xmlFree (tmp);
    }
    else
      data->smilfound = 1;
  }

  if (data->smilfound != 1)
    return xmlTextReaderNext (reader);

  name = xmlTextReaderConstName (reader);
  if (!name)
    return 0;

  if (!strcasecmp ((char *) name, "#text"))
  {
    ret = xmlTextReaderRead (reader);
    if (ret != 1)
      return ret;
  }

  attr = xmlTextReaderGetAttribute (reader, (xmlChar *) "src");
  if (!attr)
  {
    dd_log (DUCK_MSG_WARNING, "mal-formed <text> tag");
    return -1;
  }

  /*
   * splits the text on #.
   * get the anchor url
   */
  textsrc = strtok_r ((char *) attr, "#", &tok);
  if (!textsrc)
    goto err;

  dd_log (DUCK_MSG_INFO, "smil parsing <text> XMLFile: %s", textsrc);

  /* storing the file_name of the text file */
  tmp_node->text_filename = strdup (textsrc);

  /* get the fragment identifier */
  fragment = strtok_r (NULL, "#", &tok);
  if (!fragment)
    goto err;

  dd_log (DUCK_MSG_VERBOSE,
          "smil parsing <text> fragment identifier: %s", fragment);

  /* storing the fragment identifier for the passage in the text file */
  tmp_node->fragment_identifier = strdup (fragment);

  /* parse text file */
#if 0
  if (parseXHTML(data, textsrc, fragment, tmp_node) != 1)
    ; /* error */
#else
  dd_log (DUCK_MSG_WARNING, "XHTML parser for SMIL (unimplemented)");
#endif /* !0 */

  xmlFree (attr);
  return ret;

 err:
  dd_log (DUCK_MSG_WARNING, "mal-formed <text> tag");
  xmlFree (attr);
  return -1;
}

static int
smil_parse_audio (xmlTextReaderPtr reader, dd_unused daisydata_t *data,
                  node_t *tmp_node, dd_unused chk_t *chk)
{
  xmlChar *attr = NULL;
  char *value = NULL, *tok = NULL;
  int ret = 1;
  /*
   * Attributes:
   *  src
   *  id
   *  clip-begin
   *  clip-end
   */

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  /* fetch the url anchor to the audio file */
  attr = xmlTextReaderGetAttribute (reader, (xmlChar *) "src");
  if (!attr)
    goto err;

  dd_log (DUCK_MSG_INFO, "smil parsing <audio> audio uri: %s", attr);

  /* storing the audio file_name */
  tmp_node->audio_uri = strdup ((char *) attr);

  /* fetch clip-begin */
  xmlFree (attr);
  attr = xmlTextReaderGetAttribute (reader, (xmlChar *) "clip-begin");
  if (!attr)
    goto out;
  value = strtok_r ((char *) attr, "npt=s", &tok);

  dd_log (DUCK_MSG_INFO, "smil parsing <audio> clip-begin: %s", value);

  /* storing clip-begin */
  tmp_node->audio_pos_start = (int) (dd_atof (value) * 1000);
  xmlFree (attr);

  /* fetch clip-end */
  attr = xmlTextReaderGetAttribute (reader, (xmlChar *) "clip-end");
  if (!attr)
    goto err;
  value = strtok_r ((char *) attr, "npt=s", &tok);

  dd_log (DUCK_MSG_INFO, "smil parsing <audio> clip-end: %s", value);

  /* storing clip-end */
  tmp_node->audio_pos_stop = (int) (dd_atof (value) * 1000);

 out:
  xmlFree (attr);
  return ret;

 err:
  dd_log (DUCK_MSG_WARNING, "mal-formed <audio> tag");
  xmlFree (attr);
  return -1;
}

static int
smil_parse_region (xmlTextReaderPtr reader, dd_unused daisydata_t *data)
{
  int ret = 1;
  /*
   * Attributes:
   *  id
   */

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  ret = xmlTextReaderRead (reader);
  return ret;
}

static void
meta_timeinthissmil (daisydata_t *data, xmlChar *value)
{
  data->smil_pos->time = strdup ((char *) value);
}

static void
meta_totalelapsedtime (daisydata_t *data, xmlChar *value)
{
  data->smil_pos->elapsed_time = strdup ((char *) value);
}

/* The number of items in this array must be <= 64 */
static const struct {
  const char *str[4]; /* last ptr must be NULL */
  dd_type_t type;
  int qty;
  void (*fct) (daisydata_t *data, xmlChar *value);
} dd_metamap[] = {
  { { "dc:format",
      "format"                },  TYPE_MANDATORY,   1, NULL                   },
  { { "dc:identifier"         },  TYPE_RECOMMENDED, 1, NULL                   },
  { { "dc:title"              },  TYPE_OPTIONAL,    1, NULL                   },
  { { "ncc:generator"         },  TYPE_OPTIONAL,    1, NULL                   },
  { { "ncc:timeInThisSmil",
      "time-in-this-smil"     },  TYPE_RECOMMENDED, 1, meta_timeinthissmil    },
  { { "ncc:totalElapsedTime",
      "total-elapsed-time"    },  TYPE_RECOMMENDED, 1, meta_totalelapsedtime  },
  { { "title"                 },  TYPE_OPTIONAL,    1, NULL                   },
};

/* TODO: this function _must_ be factorized with ncc_parse_meta()! */
static int
smil_parse_meta (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk)
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

static int smil_parse_par (xmlTextReaderPtr reader,
                           daisydata_t *data, chk_t *chk);

static int
smil_parse_nestedseq (xmlTextReaderPtr reader,
                      daisydata_t *data, node_t *tmp_node, chk_t *chk)
{
  const xmlChar *name = NULL;
  int ret = 1, type;
  /*
   * Attributes:
   *  dur
   *  id
   */

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  type = xmlTextReaderNodeType (reader);
  if (type != 1)
  {
    dd_log (DUCK_MSG_ERROR, "failed parsing SMIL nested <seq>");
    return -1;
  }

  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  name = xmlTextReaderConstName (reader);
  if (!name)
    return 0;

  /* while we haven't come to the seq closetag */
  while (strcasecmp ((char *) name, "seq"))
  {
    /* <par> 2 */
    if (!strcasecmp ((char *) name, "par"))
      ret = smil_parse_par (reader, data, chk);
    /* <audio> + */
    else if (tmp_node && !strcasecmp ((char *) name, "audio"))
    {
      if (tmp_node->audio_uri)
        tmp_node = node_append (data);

      ret = smil_parse_audio (reader, data, tmp_node, chk);
    }

    if (ret != 1)
      return ret;

    ret = xmlTextReaderRead (reader);
    if (ret != 1)
      return ret;

    name = xmlTextReaderConstName (reader);
    if (!name)
      return 0;
  }

  type = xmlTextReaderNodeType (reader);
  if (type != 15)
  {
    dd_log (DUCK_MSG_ERROR,
            "failed parsing SMIL nested <seq>, endtag expected");
    return -1;
  }

  return ret;
}

static int
smil_parse_nestedseq_n (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk)
{
  return smil_parse_nestedseq (reader, data, NULL, chk);
}

static int
skiptag (xmlTextReaderPtr reader, const char *tag)
{
  const xmlChar *name = NULL;
  int ret = 1, type;

  /* while we haven't come to the par closetag */
  do
  {
    ret = xmlTextReaderRead (reader);
    if (ret != 1)
      return ret;
    
    name = xmlTextReaderConstName (reader);
    if (!name)
      return 0;
  }
  while (strcasecmp ((char *) name, tag));
  
  type = xmlTextReaderNodeType (reader);
  if (type != 15)
  {
    dd_log (DUCK_MSG_ERROR, "failed parsing SMIL <%s>, endtag expected", tag);
    return -1;
  }
  
  return 1;
}

static const struct {
  const char *str;
  dd_type_t type;
  int qty;
  int (*fct) (xmlTextReaderPtr reader,
              daisydata_t *data, node_t *tmp_node, chk_t *chk);
} dd_parmap[] = {
  { "text",     TYPE_MANDATORY,  1,  smil_parse_text      },
  { "audio",    TYPE_OPTIONAL,   1,  smil_parse_audio     },
  { "seq",      TYPE_OPTIONAL,  -1,  smil_parse_nestedseq },
};

static int
smil_parse_par (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk)
{
  const xmlChar *name = NULL;
  xmlChar *tmp = NULL;
  int ret = 1, type;
  node_t *tmp_node;
  /*
   * Attributes:
   *  endsync
   *  system-required
   *  id
   */

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);
  
  if (data->smilfound)
    return skiptag (reader, "par");

  type = xmlTextReaderNodeType (reader);
  if (type != 1)
  {
    dd_log (DUCK_MSG_ERROR, "failed parsing SMIL <par>");
    return -1;
  }

  /* go to the fragment identifier point and parses from there. */
  tmp = xmlTextReaderGetAttribute (reader, (xmlChar *) "id");

  /*
   * If the par tag has an id attribute, read it and check wheter it matches
   * the fragmentid. If there is none id attribute, set alreadyfound to 1 and
   * parse the whole smilfile.
   *
   * Note that a fragment identifier can exist in the text tag of this par tag.
   */
  if (tmp)
  {
    if (data->smil_pos->fragment_identifier)
    {
      if (!strcmp ((char *) tmp, data->smil_pos->fragment_identifier))
        data->smilfound = 1;
      else
        dd_log (DUCK_MSG_VERBOSE,
                "fragment id (%s) not found in this PAR tag",
                data->smil_pos->fragment_identifier);
    }

    xmlFree (tmp);
  }
  else
    data->smilfound = 1;

  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  name = xmlTextReaderConstName (reader);
  if (!name)
    return 0;

  tmp_node = node_append (data);

  /* while we haven't come to the par closetag */
  while (strcasecmp ((char *) name, "par"))
  {
    uint8_t i;

    NCC_CHECK_FOR (dd_parmap, i)
    {
      /* we skip the following tags if not found */
      /* FIXME: bugged if the text tag is not the first */
      if (   !strcmp (dd_parmap[i].str, "text")
          || (strcmp (dd_parmap[i].str, "text") && data->smilfound))
        ret = dd_parmap[i].fct (reader, data, tmp_node, chk);
      dd_chk_ok (chk, "par", i);
      break;
    }

    NCC_CHECK_OUT (dd_parmap, name, i)

    if (ret != 1)
      return ret;

    ret = xmlTextReaderRead (reader);
    if (ret != 1)
      return ret;

    name = xmlTextReaderConstName (reader);
    if (!name)
      return 0;
  }

  NCC_CHECK (chk, "par", dd_parmap, )
  dd_chk_flush (chk, "par");

  type = xmlTextReaderNodeType (reader);
  if (type != 15)
  {
    dd_log (DUCK_MSG_ERROR, "failed parsing SMIL <par>, endtag expected");
    return -1;
  }
  
  if (!data->smilfound)
    node_cancel (data);
  
  return ret;
}

static const struct {
  const char *str;
  dd_type_t type;
  int qty;
  int (*fct) (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk);
} dd_mainseqmap[] = {
  { "par",      TYPE_MANDATORY, -1,  smil_parse_par         },
  { "seq",      TYPE_OPTIONAL,  -1,  smil_parse_nestedseq_n },
};

static int
smil_mainseq (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk)
{
  const xmlChar *name = NULL;
  int ret = 1, type;
  /*
   * Attributes:
   *  dur
   *  id
   */

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  type = xmlTextReaderNodeType (reader);
  if (type != 1)
  {
    dd_log (DUCK_MSG_ERROR, "failed parsing SMIL main <seq>");
    return -1;
  }

  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  name = xmlTextReaderConstName (reader);
  if (!name)
    return 0;

  /* while we haven't reached the seq end-tag */
  while (type != 15 && strcasecmp ((char *) name, "seq"))
  {
    uint8_t i;

    NCC_CHECK_FOR (dd_mainseqmap, i)
    {
      ret = dd_mainseqmap[i].fct (reader, data, chk);
      dd_chk_ok (chk, "mainseq", i);
      break;
    }

    NCC_CHECK_OUT (dd_mainseqmap, name, i)

    if (ret != 1)
      return ret;

    ret = xmlTextReaderRead (reader);
    if (ret != 1)
      return ret;

    name = xmlTextReaderConstName (reader);
    if (!name)
      return 0;

    type = xmlTextReaderNodeType (reader);
  }

  NCC_CHECK (chk, "mainseq", dd_mainseqmap, )
  dd_chk_flush (chk, "mainseq");

  return ret;
}

static const struct {
  const char *str;
  dd_type_t type;
  int qty;
  int (*fct) (xmlTextReaderPtr reader, daisydata_t *data);
} dd_layoutmap[] = {
  { "region",     TYPE_MANDATORY,  1,  smil_parse_region },
};

static int
smil_parse_layout (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk)
{
  const xmlChar *name = NULL;
  int ret = 1, type;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  type = xmlTextReaderNodeType (reader);
  if (type != 1)
  {
    dd_log (DUCK_MSG_ERROR, "failed parsing SMIL <layout>");
    return -1;
  }

  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  name = xmlTextReaderConstName (reader);
  if (!name)
    return 0;

  /* while we haven't reached the layout endtag */
  while (strcasecmp ((char *) name, "layout"))
  {
    uint8_t i;

    NCC_CHECK_FOR (dd_layoutmap, i)
    {
      ret = dd_layoutmap[i].fct (reader, data);
      dd_chk_ok (chk, "layout", i);
      break;
    }

    NCC_CHECK_OUT (dd_layoutmap, name, i)

    if (ret != 1)
      return ret;

    ret = xmlTextReaderRead (reader);
    if (ret != 1)
      return ret;

    name = xmlTextReaderConstName (reader);
    if (!name)
      return 0;
  }

  NCC_CHECK (chk, "layout", dd_layoutmap,  )
  dd_chk_flush (chk, "layout");

  type = xmlTextReaderNodeType (reader);
  if (type != 15)
  {
    dd_log (DUCK_MSG_ERROR, "failed parsing SMIL <layout>, endtag expected");
    return -1;
  }

  return ret;
}

static const struct {
  const char *str;
  dd_type_t type;
  int qty;
  int (*fct) (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk);
} dd_bodymap[] = {
  { "seq",      TYPE_MANDATORY,  1,  smil_mainseq    },
};

static int
smil_parse_body (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk)
{
  const xmlChar *name = NULL;
  int ret = 1, type;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  ret = xmlTextReaderRead (reader);
  if (ret != 1)
    return ret;

  name = xmlTextReaderConstName (reader);
  if (!name)
    return 0;

  /* while we haven't reached the body endtag */
  while (strcasecmp ((char *) name, "body"))
  {
    uint8_t i;

    NCC_CHECK_FOR (dd_bodymap, i)
    {
      ret = dd_bodymap[i].fct (reader, data, chk);
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

  NCC_CHECK (chk, "body",   dd_bodymap, )
  dd_chk_flush (chk, "body");

  type = xmlTextReaderNodeType (reader);
  if (type != 15)
  {
    dd_log (DUCK_MSG_ERROR, "failed parsing SMIL <body>, endtag expected");
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
  { "layout",   TYPE_OPTIONAL,   1,  smil_parse_layout },
  { "meta",     TYPE_MANDATORY, -1,  smil_parse_meta   },
};

static int
smil_parse_head (xmlTextReaderPtr reader, daisydata_t *data, chk_t *chk)
{
  const xmlChar *name = NULL;
  int ret = 1, type;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  type = xmlTextReaderNodeType (reader);
  if (type != 1)
  {
    dd_log (DUCK_MSG_ERROR, "failed parsing SMIL <head>");
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

  NCC_CHECK (chk, "head",   dd_headmap, )
  dd_chk_flush (chk, "head");

  type = xmlTextReaderNodeType (reader);
  if (type != 15)
  {
    dd_log (DUCK_MSG_ERROR, "failed parsing SMIL <head>, endtag expected");
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
  { "smil",     TYPE_MANDATORY,  0,  NULL            },
  { "head",     TYPE_MANDATORY,  1,  smil_parse_head },
  { "body",     TYPE_MANDATORY,  1,  smil_parse_body },
};

/*
 * Function for parsing a SMIL 1.0 file used with the daisy 2.02 standard.
 * Collects data and stores it in a linked list in the main data structure.
 * The books meta data is stored in a bookinfo struct in the main data
 * structure. The linked Node list represents the spine of all the passages
 * in a smil file.
 */
int
dd_nccsmil_parse (daisydata_t *data)
{
  const xmlChar *name = NULL;
  char tmp[BUFFER_STR];
  xmlTextReaderPtr reader = NULL;
  int ret = 1;
  chk_t *chk = NULL;

  dd_log (DUCK_MSG_VERBOSE, __FUNCTION__);

  snprintf (tmp, sizeof (tmp), "%s%s", data->path, data->smil_pos->anchor);

  data->smilfound = 0;

  /* open the smil file */
  reader = xmlReaderForFile (tmp, NULL, 0);
  if (!reader)
  {
    dd_log (DUCK_MSG_ERROR,
            "failed to open SMIL file: %s", data->smil_pos->anchor);
    return -1;
  }

  dd_log (DUCK_MSG_INFO, "smil parsing file: %s", data->smil_pos->anchor);

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
  dd_chk_add (chk, "layout");
  dd_chk_add (chk, "mainseq");
  dd_chk_add (chk, "par");

  /* while we haven't reached the html endtag */
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

  NCC_CHECK (chk, "root",   dd_rootmap,    )

  if (ret)
    goto err;

 out:
  dd_chk_free (chk);
  xmlTextReaderClose (reader);
  xmlFreeTextReader (reader);
  return ret;

 err:
  dd_chk_free (chk);
  xmlTextReaderClose (reader);
  xmlFreeTextReader (reader);
  dd_log (DUCK_MSG_WARNING,
          "failed to parse SMIL file: %s", data->smil_pos->anchor);
  return -1;
}
