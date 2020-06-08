/*
    Jyväskylä Ion Beam Analysis Library (JIBAL)
    Copyright (C) 2020 Jaakko Julin <jaakko.julin@jyu.fi>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <jibal.h>
#include <jibal_config.h>
#include <jibal_defaults.h>

jibal jibal_init(const char *config_filename) {
    jibal jibal;
    jibal.error = JIBAL_ERROR_NONE;
    jibal.units=jibal_units_default();
    if(!jibal.units) {
        jibal.error = JIBAL_ERROR_UNITS;
        return jibal;
    }
    jibal.config = jibal_config_init(jibal.units, config_filename, TRUE);
    if(jibal.config.error) {
        jibal.error = JIBAL_ERROR_CONFIG;
        return jibal;
    }
    jibal.isotopes = jibal_isotopes_load(jibal.config.masses_file);
    if(!jibal.isotopes) {
        fprintf(stderr, "Could not load isotope table from file %s.\n", jibal.config.masses_file);
        jibal.error = JIBAL_ERROR_MASSES;
        return jibal;
    }
    if(jibal_abundances_load(jibal.isotopes, jibal.config.abundances_file) < 0) {
        jibal.error = JIBAL_ERROR_ABUNDANCES;
        return jibal;
    }
    jibal.elements=jibal_elements_populate(jibal.isotopes);
#ifdef DEBUG
    fprintf(stderr, "The Z_max of elements array is %i\n", jibal_elements_Zmax(jibal.elements));
#endif
    if(!jibal.elements) {
        jibal.error = JIBAL_ERROR_ELEMENTS;
        return jibal;
    }
    jibal.gsto= jibal_gsto_init(jibal.elements, jibal.config.Z_max, jibal.config.files_file,
                                jibal.config.assignments_file);
    if(!jibal.gsto) {
        fprintf(stderr, "Could not initialize GSTO.\n");
        jibal.error = JIBAL_ERROR_GSTO;
        return jibal;
    }
    jibal.gsto->extrapolate = jibal.config.extrapolate;
    return jibal;
}

void jibal_free(jibal *jibal) {
    if(jibal->units)
        jibal_units_free(jibal->units);
    if(jibal->elements)
        jibal_elements_free(jibal->elements);
    if(jibal->isotopes)
        jibal_isotopes_free(jibal->isotopes);
    if(jibal->gsto)
        jibal_gsto_free(jibal->gsto);
    jibal_config_free(&jibal->config);
    /* Note, not freeing jibal itself! */
}

const char *jibal_error_string(jibal_error err) {
    switch(err) {
        case JIBAL_ERROR_NONE:
            return "ERROR_NONE";
        case JIBAL_ERROR_CONFIG:
            return "ERROR_CONFIG";
        case JIBAL_ERROR_UNITS:
            return "ERROR_UNITS";
        case JIBAL_ERROR_MASSES:
            return "ERROR_MASSES";
        case JIBAL_ERROR_ABUNDANCES:
            return "ERROR_ABUNDANCES";
        case JIBAL_ERROR_ELEMENTS:
            return "ERROR_ELEMENTS";
        case JIBAL_ERROR_GSTO:
            return "ERROR_GSTO";
        default:
            return "ERROR_UNKNOWN";
    }
}

const char *jibal_version() {
    return jibal_VERSION;
}
