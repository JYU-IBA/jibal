/*
    Jyväskylä Ion Beam Analysis Library (JIBAL)
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

#ifndef _JIBAL_JIBAL_H_
#define _JIBAL_JIBAL_H_

#include <jibal_units.h>
#include <jibal_config.h>
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

typedef struct jibal {
    jibal_error error;
    jibal_units *units;
    jibal_isotope *isotopes;
    jibal_element *elements;
    jibal_gsto *gsto;
    jibal_config *config;
} jibal; /* All in one solution */

jibal *jibal_init(const char *config_filename);
void jibal_status_print(FILE *f, const jibal *jibal);
void jibal_free(jibal *jibal);
const char *jibal_error_string(jibal_error err);
const char *jibal_version();

#endif //_JIBAL_JIBAL_H_
