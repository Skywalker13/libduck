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

#ifndef DUCK_INTERNALS_H
#define DUCK_INTERNALS_H

#include <inttypes.h>

typedef enum dd_type {
  TYPE_MANDATORY,
  TYPE_OPTIONAL,
  TYPE_RECOMMENDED,
} dd_type_t;

typedef struct node_s {
  struct node_s *next;
  struct node_s *prev;

  int item_id;

  char *audio_uri;
  int   audio_pos_start;
  int   audio_pos_stop;
  char *text_filename;
  char *fragment_identifier;
  char *text_passage;
  char *image;
} node_t;

typedef struct smilnode_s {
  struct smilnode_s *next;
  struct smilnode_s *prev;

  int level;
  int item_id;
  duck_smilnode_type_t type;

  char *id;
  char *anchor;
  char *header;
  char *fragment_identifier;
  char *text_passage;
  char *image;
  char *time;                   /* Total time of this smil.               */
  char *elapsed_time;           /* Total elapsed time in this smil.       */
} smilnode_t;

typedef struct bookinfo_s {
  char *title_text;             /* Title of the book.                     */
  char *title_audio_uri;        /* The source to the audio stream         */
  char *title_audio_pos_start;  /* The time offset to start playback.     */
  char *title_audio_pos_stop;   /* The time offset to stop playback.      */
  char *title_image;            /* The source to the book image.          */
  char *total_time;             /* The total time of the book.            */
  char *narrator;
  char *author;
} bookinfo_t;

typedef struct daisydata_s {
  /*
   * Pointers which are the mirror of all Smil files. It represents the
   * spine of the loaded Daisy DTB.
   */
  smilnode_t *smil_head;
  smilnode_t *smil_tail;
  smilnode_t *smil_pos;

  /* All passages nodes of a parsed Smil file. */
  node_t *node_head;
  node_t *node_tail;
  node_t *node_pos;

  /* Metadata of the loaded Daisy DTB. */
  bookinfo_t book_info;

  duck_format_t format; /* Book version (like 2.02). */

  int smilfound;  /* smilparser: check if a Smil file is already presents. */
  char *path;     /* Directory of the loaded DTB. */

  int integrity;  /* 1 if the parsing has failed somewhere, 0 if ok */

} daisydata_t;

struct duck_s {
  daisydata_t *data;
};

#define ARRAY_NB_ELEMENTS(a) (sizeof (a) / sizeof (*a))

#ifndef dd_unused
#if defined(__GNUC__)
#  define dd_unused __attribute__ ((unused))
#else
#  define dd_unused
#endif
#endif

#define NCC_CHECK_FOR(a, i)                                                   \
  for (i = 0; i < ARRAY_NB_ELEMENTS (a); i++)                                 \
    if (!strcasecmp ((char *) name, (a)[i].str))

#define NCC_CHECK_OUT(a, n, i)                                                \
  if (i == ARRAY_NB_ELEMENTS (a) && *n != '#')                                \
    dd_log (DUCK_MSG_WARNING,                                                 \
            "the tag <%s> is not part of Daisy 2.02 specifications", n);

#define NCC_CHECK(c, t, arr, a)                                               \
  {                                                                           \
    uint8_t qty, i;                                                           \
    for (i = 0; i < ARRAY_NB_ELEMENTS (arr); i++)                             \
    {                                                                         \
      qty = dd_chk_read (c, t, i);                                            \
      if (!qty)                                                               \
      {                                                                       \
        switch ((arr)[i].type)                                                \
        {                                                                     \
        case TYPE_MANDATORY:                                                  \
          dd_log (DUCK_MSG_WARNING,                                           \
                  "[check] " t " : %s mandatory but unavailable",             \
                  a(arr)[i].str);                                             \
          break;                                                              \
                                                                              \
        case TYPE_RECOMMENDED:                                                \
          dd_log (DUCK_MSG_VERBOSE,                                           \
                  "[check] " t " : %s is recommended", a(arr)[i].str);        \
          break;                                                              \
                                                                              \
        default:                                                              \
          break;                                                              \
        }                                                                     \
      }                                                                       \
      else if ((arr)[i].qty == 1 && qty > 1)                                  \
        dd_log (DUCK_MSG_WARNING,                                             \
                "[check] " t " : %s exists %u times but only one "            \
                "is specified in the specifications", a(arr)[i].str, qty);    \
    }                                                                         \
  }

#endif /* DUCK_INTERNALS_H */
