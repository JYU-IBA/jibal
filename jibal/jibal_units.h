/*
    JIBAL - Library for ion beam analysis
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


#ifndef _JIBAL_UNITS_H_
#define _JIBAL_UNITS_H_

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#include <stdio.h>

#define C_US 1.0e-6
#define C_NS 1.0e-9
#define C_PS 1.0e-12
#define C_CM2 1.0e-4
#define C_CM3 1.0e-6
#define C_CM 1.0e-2
#define C_MM 1.0e-3
#define C_UM 1.0e-6
#define C_NM 1.0e-9
#define C_ANGSTROM 1.0e-10
#define C_G 1.0e-3
#define C_UG 1.0e-9
#define C_G_CM3 1.0e3
#define C_MSR 1.0e-3


#define C_PI M_PI
#define C_DEG (2.0*C_PI/360.0) /* degree */
#define C_FWHM 2.35482004503
#define C_PERCENT 0.01
#define C_U (1.66053906660e-27) /* atomic mass unit, kg */
#define C_C (299792458.0) /* speed of light, in m/s, exact. */
#define C_C2 (C_C*C_C) /* speed of light, squared */
#define C_E (1.602176634e-19) /* elementary charge, in C, exact. */
#define C_UC (1.0e-6/C_E)
#define C_EV (C_E) /* electronvolt, in J */
#define C_KEV (1.0e3*C_EV) /* keV */
#define C_MEV (1.0e6*C_EV) /* MeV */

#define C_H (6.62607015e-34) /* Planck constant, exact. */
#define C_HBAR (C_H/(2*C_PI))
#define C_ALPHA (0.0072973525693) /* fine structure constant. Using CODATA 2018 recommended value. */
#define C_MU0  (2*C_ALPHA*C_H/(C_E*C_E*C_C))  /* vacuum permeability */
#define C_EPSILON0 (1.0/(C_MU0*C_C2)) /* vacuum permittivity */
#define C_ME (9.1093837015e-31) /* electron mass, kg */
#define C_BOHR_RADIUS (C_HBAR/(C_ALPHA*C_ME*C_C)) /* Bohr radius */

#define C_TFU (1.0e19) /* Thin film units, i.e. 1e15 at./cm2 is actually 1e19/m^2 in SI-units */
#define C_EV_TFU (C_EV/C_TFU) /* Units for stopping cross sections (eV/(1e15 at./cm2)) */

#define C_BARN (1.0e-28)
#define C_MB_SR (1.0e-3*C_BARN) /* millibarns/sr */

#define UNIT_TYPE_ENERGY 'E'
#define UNIT_TYPE_ANGLE 'A'
#define UNIT_TYPE_DISTANCE 'L'
#define UNIT_TYPE_MASS 'M'
#define UNIT_TYPE_LAYER_THICKNESS 'X'
#define UNIT_TYPE_TIME 'T'
#define UNIT_TYPE_CHARGE 'Q'
#define UNIT_TYPE_VOLTAGE 'U'
#define UNIT_TYPE_CURRENT 'I'
#define UNIT_TYPE_MAGNETIC_FIELD 'B' /* aka flux density */

typedef struct jibal_units {
    double f; /*!< Factor, i.e. this unit in Si units (e.g. 1.66e-27 for the atomic mass unit) */
    char type; /*!< Physically relevant type, use defines above. UNIT_TYPE_MASS for a unit of mass. */
    char *name; /*!< E.g. "u" */
    struct jibal_units *next; /*!< Linked list */
} jibal_units;

jibal_units *jibal_units_add(jibal_units *units, double f, char type, char *name);
jibal_units *jibal_units_default();
int jibal_units_count(const jibal_units *units);
int jibal_units_print(FILE *out, const jibal_units *units);

/*!
 Get a unit
 
 @param units Linked list of units
 @param type Type of unit (see defines)
 @param name Name of the unit
 */
double jibal_units_get(const jibal_units *units, char type, const char *name);
double jibal_get_val(const jibal_units *units, char type, const char *value);
void jibal_units_free(jibal_units *units);
#endif /* _JIBAL_UNITS_H_ */
