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

typedef struct {
    char *datadir;
    char *masses_file;
    char *abundances_file;
    char *stoppings_file;
    int Z_max;
    int extrapolate; /* this is boolean, see JIBAL_CONFIG_VAR_BOOL */
} jibal_config; /* Some internal configuration (environment etc) */

typedef struct {
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
jibal_config jibal_config_init(const jibal_units *units, const char *filename);
int jibal_config_file_read(const jibal_units *units, jibal_config *config, const char *filename);
int jibal_config_file_write(jibal_config *config, FILE *file);
void jibal_config_free();


#endif //JIBAL_JIBAL_H
