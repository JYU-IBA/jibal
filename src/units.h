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

/* Constants and other units */


#ifndef _LIBIBA_UNITS_H_
#define _LIBIBA_UNITS_H_

#include <math.h>

#define C_PI M_PI
#define C_U (1.66053906660e-27) /* atomic mass unit, kg */
#define C_C (299792458.0) /* speed of light, in m/s */
#define C_C2 (C_C*C_C) /* speed of light, squared */
#define C_E (1.602176634e-19) /* elementary charge, in C */
#define C_EV (C_E) /* electronvolt, in J */
#define C_KEV (1.0e3*C_EV) /* keV */
#define C_MEV (1.0e6*C_EV) /* MeV */
#define C_DEG (2.0*C_PI/360.0)

#define UNIT_TYPE_ENERGY 'E'
#define UNIT_TYPE_ANGLE 'A'
#define UNIT_TYPE_DISTANCE 'L'
#define UNIT_TYPE_MASS 'M'

typedef struct iba_units {
    double f; /*!< Factor, i.e. this unit in Si units (e.g. 1.66e-27 for the atomic mass unit) */
    char type; /*!< Physically relevant type, use defines above. UNIT_TYPE_MASS for a unit of mass. */
    char *name; /*!< E.g. "u" */
    struct iba_units *next; /*!< Linked list */
} iba_units;

iba_units *iba_units_add(iba_units *units, double f, char type, char *name);
iba_units *iba_units_default();

/*!
 Get a unit
 
 @param units Linked list of units
 @param type Type of unit (see defines)
 @param name Name of the unit
 */
double iba_units_get(const iba_units *units, char type, const char *name);
double iba_get_val(const iba_units *units, char type, const char *value);
#endif /* _LIBIBA_UNITS_H_ */
