/*
    LibIBA - Library for ion beam analysis
    Copyright (C) 2020 Jaakko Julin <jaakko.julin@jyu.fi>

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

#ifndef JIBAL_JIBAL_H
#define JIBAL_JIBAL_H

#include <jibal_units.h>
#include <jibal_masses.h>
#include <jibal_material.h>
#include <jibal_gsto.h>

typedef enum {
    JIBAL_ERROR_NONE = 0,
    JIBAL_ERROR_CONFIG = 1,
    JIBAL_ERROR_UNITS = 2,
    JIBAL_ERROR_MASSES = 3,
    JIBAL_ERROR_ABUNDANCES = 4,
    JIBAL_ERROR_ELEMENTS = 5,
    JIBAL_ERROR_GSTO = 6
} jibal_error;

typedef struct {
    int error;
    char *datadir; /* e.g. /usr/local/share/jibal or C:\Program Files\Jibal\share\jibal */
    char *userdatadir; /* e.g. ~/.jibal/ or %AppData/Jibal */
    char *masses_file;
    char *abundances_file;
    char *files_file;
    char *assignments_file;
    int Z_max;
    int extrapolate; /* this is boolean, see JIBAL_CONFIG_VAR_BOOL */
} jibal_config; /* Some internal configuration (environment etc) */

typedef struct {
    jibal_error error;
    jibal_units *units;
    jibal_isotope *isotopes;
    jibal_element *elements;
    jibal_gsto *gsto;
    jibal_config config;
} jibal; /* All in one solution */


typedef enum {
    JIBAL_CONFIG_VAR_NONE = 0,
    JIBAL_CONFIG_VAR_STRING = 1,
    JIBAL_CONFIG_VAR_BOOL = 2, /* Internally an int */
    JIBAL_CONFIG_VAR_INT = 3, /* 32-bit signed int (aka int) */
    JIBAL_CONFIG_VAR_DOUBLE = 4,
    JIBAL_CONFIG_VAR_UNIT = 5 /* Number with a unit. Store number after SI conversion. */
} jibal_config_var_type;

typedef struct {
    jibal_config_var_type type;
    const char *name;
    void *variable;
} jibal_config_var;


jibal jibal_init(const char *config_filename);
void jibal_free(jibal *jibal);
char *jibal_config_user_dir();
char *jibal_config_user_config_filename();
jibal_config jibal_config_defaults();
jibal_config jibal_config_init(const jibal_units *units, const char *filename, int seek);
void jibal_config_finalize(jibal_config *config);
char *jibal_config_filename_seek();
int jibal_config_file_read(const jibal_units *units, jibal_config *config, const char *filename);
int jibal_config_file_write(jibal_config *config, FILE *file);
int jibal_path_is_absolute(const char *path);
char *jibal_path_cleanup(char *path);
const char *jibal_error_string(jibal_error err);
void jibal_config_free();


#endif //JIBAL_JIBAL_H
