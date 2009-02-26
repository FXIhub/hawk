/* ----------------------------------------------------------------------------
   libconfig - A structured configuration file parsing library
   Copyright (C) 2005  Mark A Lindner
 
   This file is part of libconfig.
    
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.
    
   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
   ----------------------------------------------------------------------------
*/

#ifndef __libconfig_h
#define __libconfig_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#if defined(DLL_EXPORT) || defined(LIBCONFIG_EXPORTS)
#define LIBCONFIG_API __declspec(dllexport)
#else // ! DLL_EXPORT
#define LIBCONFIG_API __declspec(dllimport)
#endif // DLL_EXPORT
#else // ! WIN32
#define LIBCONFIG_API
#endif // WIN32

#include <stdio.h>

#define CONFIG_TYPE_NONE    0
#define CONFIG_TYPE_GROUP   1
#define CONFIG_TYPE_INT     2
#define CONFIG_TYPE_FLOAT   3
#define CONFIG_TYPE_STRING  4
#define CONFIG_TYPE_BOOL    5
#define CONFIG_TYPE_ARRAY   6
#define CONFIG_TYPE_LIST    7

#define CONFIG_FORMAT_DEFAULT  0
#define CONFIG_FORMAT_HEX      1

#define CONFIG_TRUE  (1)
#define CONFIG_FALSE (0)

typedef union config_value_t
{
  long ival;
  double fval;
  char *sval;
  int bval;
  struct config_list_t *list;
} config_value_t;

typedef struct config_setting_t
{
  char *name;
  int type;
  int format;
  config_value_t value;
  struct config_setting_t *parent;
  struct config_t *config;
  void *hook;
} config_setting_t;

typedef struct config_list_t
{
  unsigned int length;
  unsigned int capacity;
  config_setting_t **elements;
} config_list_t;

typedef struct config_t
{
  config_setting_t *root;
  void (*destructor)(void *);
  const char *error_text;
  int error_line;
} config_t;

extern LIBCONFIG_API int config_read(config_t *config, FILE *stream);
extern LIBCONFIG_API void config_write(const config_t *config, FILE *stream);

extern LIBCONFIG_API int config_read_file(config_t *config,
                                          const char *filename);
extern LIBCONFIG_API int config_write_file(config_t *config,
                                           const char *filename);

extern LIBCONFIG_API void config_set_destructor(config_t *config,
                                                void (*destructor)(void *));

extern LIBCONFIG_API void config_init(config_t *config);
extern LIBCONFIG_API void config_destroy(config_t *config);

extern LIBCONFIG_API long config_setting_get_int(
  const config_setting_t *setting);
extern LIBCONFIG_API double config_setting_get_float(
  const config_setting_t *setting);
extern LIBCONFIG_API int config_setting_get_bool(
  const config_setting_t *setting);
extern LIBCONFIG_API const char *config_setting_get_string(
  const config_setting_t *setting);

extern LIBCONFIG_API int config_setting_set_int(config_setting_t *setting,
                                                long value);
extern LIBCONFIG_API int config_setting_set_float(config_setting_t *setting,
                                                  double value);
extern LIBCONFIG_API int config_setting_set_bool(config_setting_t *setting,
                                                 int value);
extern LIBCONFIG_API int config_setting_set_string(config_setting_t *setting,
                                                   const char *value);

extern LIBCONFIG_API int config_setting_set_format(config_setting_t *setting,
                                                   int format);
extern LIBCONFIG_API int config_setting_get_format(config_setting_t *setting);

extern LIBCONFIG_API long config_setting_get_int_elem(
  const config_setting_t *setting, int index);
extern LIBCONFIG_API double config_setting_get_float_elem(
  const config_setting_t *setting, int index);
extern LIBCONFIG_API int config_setting_get_bool_elem(
  const config_setting_t *setting, int index);
extern LIBCONFIG_API const char *config_setting_get_string_elem(
  const config_setting_t *setting, int index);

extern LIBCONFIG_API config_setting_t *config_setting_set_int_elem(
  config_setting_t *setting, int index, long value);
extern LIBCONFIG_API config_setting_t *config_setting_set_float_elem(
  config_setting_t *setting, int index, double value);
extern LIBCONFIG_API config_setting_t *config_setting_set_bool_elem(
  config_setting_t *setting, int index, int value);
extern LIBCONFIG_API config_setting_t *config_setting_set_string_elem(
  config_setting_t *setting, int index, const char *value);

#define /* int */ config_setting_type(/* const config_setting_t * */ S) \
  ((S)->type)

#define /* const char */ config_setting_name(/* const config_setting_t * */ S) \
  ((S)->name)

extern LIBCONFIG_API int config_setting_length(
  const config_setting_t *setting);
extern LIBCONFIG_API config_setting_t *config_setting_get_elem(
  const config_setting_t *setting, unsigned int index);

extern LIBCONFIG_API config_setting_t *config_setting_get_member(
  const config_setting_t *setting, const char *name);

extern LIBCONFIG_API config_setting_t *config_setting_add(
  config_setting_t *parent, const char *name, int type);
extern LIBCONFIG_API int config_setting_remove(config_setting_t *parent,
                                               const char *name);
extern LIBCONFIG_API void config_setting_set_hook(config_setting_t *setting,
                                                  void *hook);

#define config_setting_get_hook(S) ((S)->hook)

extern LIBCONFIG_API config_setting_t *config_lookup(const config_t *config,
                                                     const char *path);

extern LIBCONFIG_API long config_lookup_int(const config_t *config,
                                            const char *path);
extern LIBCONFIG_API double config_lookup_float(const config_t *config,
                                                const char *path);
extern LIBCONFIG_API int config_lookup_bool(const config_t *config,
                                            const char *path);
extern LIBCONFIG_API const char *config_lookup_string(const config_t *config,
                                                      const char *path);

#define /* config_setting_t * */ config_root_setting(/* const config_t * */ C) \
  ((C)->root)

#define /* const char * */ config_error_text(/* const config_t * */ C)    \
  ((C)->error_text)

#define /* int */ config_error_line(/* const config_t * */ C)     \
  ((C)->error_line)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __libconfig_h */
