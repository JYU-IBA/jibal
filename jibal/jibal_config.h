/*
    Jyväskylä Ion Beam Analysis Library (JIBAL)
    Copyright (C) 2020 - 2023 Jaakko Julin <jaakko.julin@jyu.fi>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef JIBAL_CONFIG_H
#define JIBAL_CONFIG_H

#include <stdio.h>
#include <jibal_units.h>
#include <jibal_cross_section.h>

typedef struct {
    int error;
    char *config_file; /* set to actual file that was read successfully */
    char *datadir; /* e.g. /usr/local/share/jibal or C:\Program Files\JIBAL\share\jibal */
    char *userdatadir; /* e.g. ~/.jibal/ or %AppData/JIBAL */
    char *masses_file;
    char *abundances_file;
    char *files_file;
    char *assignments_file;
    int Z_max;
    int extrapolate; /* this is boolean, see JIBAL_CONFIG_VAR_BOOL */
    jibal_cross_section_type cs_rbs;
    jibal_cross_section_type cs_erd;
} jibal_config; /* Some internal configuration (environment etc) */

typedef enum {
    JIBAL_CONFIG_VAR_NONE = 0,
    JIBAL_CONFIG_VAR_STRING = 1, /* C string, aka. NUL terminated char *. */
    JIBAL_CONFIG_VAR_PATH = 2, /* Internally same as above, but we assume relative paths are relative to the config file */
    JIBAL_CONFIG_VAR_BOOL = 3, /* Internally an int */
    JIBAL_CONFIG_VAR_INT = 4, /* 32-bit signed int (aka int) */
    JIBAL_CONFIG_VAR_DOUBLE = 5,
    JIBAL_CONFIG_VAR_UNIT = 6, /* Number with a unit. Store number after SI conversion. */
    JIBAL_CONFIG_VAR_OPTION = 7, /* Internally an int, input/output as string */
    JIBAL_CONFIG_VAR_SIZE   = 8 /* size_t, unsigned typically 32 or 64-bit, we never use more than 32 bit. */
} jibal_config_var_type;

typedef struct {
    jibal_config_var_type type;
    const char *name;
    const char *unit; /* for JIBAL_CONFIG_VAR_UNIT */
    char unit_type; /* preferred unit, can't be changed (const), but no allocation issues */
    void *variable; /* this is the pointer to data. We don't free these, so memory must be allocated for the duration of use. Note that for strings variable is supposed to be char ** and we allocate new char * (and free old ones) as is necessary. */
    const jibal_option *option_list; /* Only used with type == JIBAL_CONFIG_VAR_OPTION */
    const char *description;
} jibal_config_var;

typedef struct {
    char *filename; /* filename is needed because of JIBAL_CONFIG_VAR_PATH, can be NULL (no path) */
    const jibal_units *units; /* units are needed for vars with units */
    jibal_config_var *vars; /* Terminated with an entry with type==JIBAL_CONFIG_VAR_NONE */
} jibal_config_file;

const char *jibal_config_var_type_name(jibal_config_var_type type);
int jibal_config_option_get(const jibal_config_var *var, const char *value); /* For var->type == JIBAL_CONFIG_VAR_OPTION */
const char *jibal_config_option_string(const jibal_config_var *var); /* For var->type == JIBAL_CONFIG_VAR_OPTION */
char *jibal_config_user_dir();
int jibal_config_user_dir_mkdir_if_necessary();
char *jibal_config_user_config_filename();
jibal_config jibal_config_defaults();
jibal_config *jibal_config_init(const jibal_units *units, const char *filename, int seek);
void jibal_config_free(jibal_config *config);
char *jibal_config_filename_seek();
int jibal_config_read_from_file(const jibal_units *units, jibal_config *config, const char *filename);
void jibal_config_finalize(jibal_config *config);
int jibal_config_write_to_file(const jibal_units *units, jibal_config *config, const char *filename);

jibal_config_file *jibal_config_file_init(const jibal_units *units);
int jibal_config_file_set_vars(jibal_config_file *cf, jibal_config_var *vars); /* the vars will now be "owned" by the config file and free'd with jibal_config_file_free(). */
void jibal_config_file_free(jibal_config_file *cf);
int jibal_config_file_read(jibal_config_file *cf, const char *filename);
int jibal_config_file_write(const jibal_config_file *cf, const char *filename);
int jibal_config_file_var_set(jibal_config_file *cf, const char *var, const char *val);
int jibal_config_var_set(const jibal_units *units, jibal_config_var *var, const char *val, const char *filename);
const jibal_config_var *jibal_config_var_find(jibal_config_var *var, const char *str);
int jibal_path_is_absolute(const char *path);
char *jibal_path_cleanup(char *path);
#endif /* JIBAL_CONFIG_H */
