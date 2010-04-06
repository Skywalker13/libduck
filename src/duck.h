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

#ifndef DUCK_H
#define DUCK_H

/**
 * \file duck.h
 *
 * Duck public API header.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DD_STRINGIFY(s) #s
#define DD_TOSTRING(s) DD_STRINGIFY(s)

#define DD_VERSION_INT(a, b, c) (a << 16 | b << 8 | c)
#define DD_VERSION_DOT(a, b, c) a ##.## b ##.## c
#define DD_VERSION(a, b, c) DD_VERSION_DOT(a, b, c)

#define LIBDUCK_VERSION_MAJOR  0
#define LIBDUCK_VERSION_MINOR  0
#define LIBDUCK_VERSION_MICRO  1

#define LIBDUCK_VERSION_INT DD_VERSION_INT(LIBDUCK_VERSION_MAJOR, \
                                           LIBDUCK_VERSION_MINOR, \
                                           LIBDUCK_VERSION_MICRO)
#define LIBDUCK_VERSION     DD_VERSION(LIBDUCK_VERSION_MAJOR, \
                                       LIBDUCK_VERSION_MINOR, \
                                       LIBDUCK_VERSION_MICRO)
#define LIBDUCK_VERSION_STR DD_TOSTRING(LIBDUCK_VERSION)
#define LIBDUCK_BUILD       LIBDUCK_VERSION_INT

#include <inttypes.h>

/**
 * \brief Return LIBDUCK_VERSION_INT constant.
 */
unsigned int libduck_version (void);


/******************************************************************************/
/*                                                                            */
/* Duck Handling                                                              */
/*                                                                            */
/******************************************************************************/

/** \brief Main handle. */
typedef struct duck_s duck_t;

/** \brief Verbosity level. */
typedef enum duck_verb {
  DUCK_MSG_NONE,     /**< No error messages.                            */
  DUCK_MSG_VERBOSE,  /**< Super-verbose mode: mostly for debugging.     */
  DUCK_MSG_INFO,     /**< Working operations.                           */
  DUCK_MSG_WARNING,  /**< Harmless failures.                            */
  DUCK_MSG_ERROR,    /**< May result in hazardous behavior.             */
  DUCK_MSG_CRITICAL, /**< Prevents lib from working.                    */
} duck_verb_t;

/** \brief Format of Digital Talking Book. */
typedef enum duck_format {
  DUCK_FORMAT_AUTO    = -1,
  DUCK_FORMAT_NCC     =  0, /**< Mostly supported (Daisy 2.02).         */
  DUCK_FORMAT_NCX     =  1, /**< Unimplemented.                         */
} duck_format_t;

/** \brief Type of result for duck_smilnode_getinfo(). */
typedef enum duck_smilnode_info {
  DUCK_SMILNODE_S_HEADER, /**< Heading text.                            */
  DUCK_SMILNODE_I_INDEX,  /**< Index (smilpos).                         */
  DUCK_SMILNODE_I_LEVEL,  /**< Level of heading (1-6).                  */
  DUCK_SMILNODE_I_TYPE,   /**< Type of smilnode ::duck_smilnode_type_t. */
} duck_smilnode_info_t;

/** \brief Type of result for duck_node_getinfo(). */
typedef enum duck_node_info {
  DUCK_NODE_S_AUDIO_URI,        /**< URI for the node.                  */
  DUCK_NODE_I_AUDIO_POS_START,  /**< Start position of the node [ms].   */
  DUCK_NODE_I_AUDIO_POS_STOP,   /**< Stop position of the node [ms].    */
  DUCK_NODE_I_INDEX,            /**< Index (smilnode).                  */
} duck_node_info_t;

typedef enum duck_smilnode_type {
  DUCK_SMILNODE_HEADING,
  DUCK_SMILNODE_PAGE,
  DUCK_SMILNODE_BLOCK,
} duck_smilnode_type_t;

typedef union duck_value {
  char *s;
  int   i;
} duck_value_t;

#define DUCK_MAX_LEVEL 6

typedef struct duck_hx_s {
  int h1, h2, h3, h4, h5, h6;
} duck_hx_t;

/** \brief Parameters for duck_init(). */
typedef struct duck_init_param_s {
  int dummy;
} duck_init_param_t;

/**
 * \name Duck Handling.
 * @{
 */

/**
 * \brief Init an handle.
 *
 * For a description of each parameters supported by this function:
 * \see ::duck_init_param_t
 *
 * When a parameter in \p param is 0 (or NULL), its default value is used.
 * If \p param is NULL, then all default values are forced for all parameters.
 *
 * \param[in] param       Parameters, NULL for default values.
 * \return The handle.
 */
duck_t *duck_init (duck_init_param_t *param);

/**
 * \brief Uninit an handle.
 *
 * \param[in] handle      Handle.
 */
void duck_uninit (duck_t *handle);

/**
 * \brief Change verbosity level.
 *
 * Default value is DUCK_MSG_INFO.
 *
 * \warning This function can be called in anytime.
 * \param[in] level       Level provided by duck_verb_t.
 */
void duck_verbosity (duck_verb_t level);

/**
 * \brief Load a Daisy book.
 *
 * \param[in] handle      Handle.
 * \param[in] format      File format.
 * \return 0 on success, != 0 on error.
 */
int duck_load (duck_t *handle, const char *path, duck_format_t format);

/**
 * \brief Retrieve the current smilnode (chapter) and the node (paragraph).
 *
 * \param[in] handle      Handle.
 * \param[out] smilpos    Chapter position.
 * \param[out] nodepos    Paragraph position.
 * \return 0 for success, != 0 on error.
 */
int duck_getpos (duck_t *handle, int *smilpos, int *nodepos);

/**
 * \brief Retrieve the current heading (a.b.c.d.e.f).
 *
 * \param[in] handle      Handle.
 * \param[out] hx         Heading.
 * \return 0 for success, != 0 on error.
 */
int duck_getheading (duck_t *handle, duck_hx_t *hx);

/**
 * \brief Go to a specific chapter and paragraph.
 *
 * \param[in] handle      Handle.
 * \param[in] smilpos     Chapter position.
 * \param[in] nodepos     Paragraph position.
 * \return 0 for success, != 0 on error.
 */
int duck_walk (duck_t *handle, int smilpos, int nodepos);

/**
 * \brief Retrieve the number of Smil node.
 *
 * \param[in] handle      Handle.
 * \return the number.
 */
int duck_smilnode_number (duck_t *handle);

/**
 * \brief Retrieve the informations on a Smil node.
 *
 * \param[in] handle      Handle.
 * \param[in] sel         Type of information.
 * \param[out] res        Result.
 * \return 0 for success, != 0 on error.
 */
int duck_smilnode_getinfo (duck_t *handle,
                           duck_smilnode_info_t sel, duck_value_t *res);

/**
 * \brief Retrieve the informations on a node.
 *
 * \param[in] handle      Handle.
 * \param[in] sel         Type of information.
 * \param[out] res        Result.
 * \return 0 for success, != 0 on error.
 */
int duck_node_getinfo (duck_t *handle,
                       duck_node_info_t sel, duck_value_t *res);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DUCK_H */
