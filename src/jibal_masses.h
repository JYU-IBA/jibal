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

#ifndef _JIBAL_MASSES_H_
#define _JIBAL_MASSES_H_

#include <jibal/jibal_units.h>
#include <jibal/jibal_phys.h>

#define JIBAL_MASSES_LINE_LENGTH 80
#define JIBAL_MASSES_ELEMENT_LENGTH 8 /* AAAxx, e.g. 241Am, max size = 3+2+1 = 6 (zero terminated). Round up to 8. */

#ifndef JIBAL_MASSES_ISOTOPES
#define JIBAL_MASSES_ISOTOPES 4095 /* Maximum number of isotopes.  */
#endif

typedef char isotope_name[JIBAL_MASSES_ELEMENT_LENGTH] ;

typedef struct {
    isotope_name name; /* "A-Xx eg. 239-Pu" */
    int N;
    int Z;
    int A; /* A=Z+N */
    double mass;
    double abundance;
} jibal_isotope;

typedef struct {
    double v_max;
    int vsteps;
    int z_max;
    double ***sto; /* sto[Z1][Z2][v_index] */
    double *v; /* v=v[v_index] */
} jibal_stopping;


double jibal_find_average_mass(jibal_isotope *isotopes, int Z);
int jibal_find_Z_by_name(jibal_isotope *isotopes, char *name);
double jibal_find_mass(jibal_isotope *isotope, int Z, int A); /* find isotope mass, but if A=0 calculate average mass of elem. */

jibal_isotope *isotopes_load(const char *filename);
jibal_isotope *find_first_isotope(jibal_isotope *isotopes, int Z);
jibal_isotope *find_most_abundant_isotope(jibal_isotope *isotopes, int Z);
jibal_isotope *find_isotope(jibal_isotope *isotopes, int Z, int A);
jibal_isotope *isotopes_find_all(jibal_isotope *isotopes, int Z);
jibal_isotope *isotope_find(jibal_isotope *isotopes, const char *name);
jibal_stopping *init_stopping_table(char *filename);
int delete_stopping_table(jibal_stopping);


double velocity(double E, double mass); /* Use SI units */
double energy(double v, double mass);
#endif
