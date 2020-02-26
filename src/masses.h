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

#ifndef _LIBIBA_MASSES_H_
#define _LIBIBA_MASSES_H_

#include "defaults.h"
#include "units.h"
#include "phys.h"


#define MASSES_LINE_LENGTH 80
#define MASSES_ELEMENT_LENGTH 8 /* AAAxx, e.g. 241Am, max size = 3+2+1 = 6 (zero terminated). Round up to 8. */

#ifndef MASSES_ISOTOPES
#define MASSES_ISOTOPES 4096 /* Maximum number of isotopes. Real number in the database is less than 3500. Don't assume this number is exact.  (see isotopes_t) */
#endif

#define STOPPING_DATA DATAPATH/stoppings.txt
#define MASSES_DATA DATAPATH/masses.dat
#define XSTR(x) STR(x)
#define STR(x) #x


typedef struct {
    char name[MASSES_ELEMENT_LENGTH]; /* "A-Xx eg. 239-Pu" */
    int N;
    int Z;
    int A; /* A=Z+N */
    double mass;
    double abundance;
} isotope_t;

typedef struct {
    int n_isotopes;
    isotope_t *i;
} isotopes_t;

typedef struct {
    double v_max;
    int vsteps;
    int z_max;
    double ***sto; /* sto[Z1][Z2][v_index] */
    double *v; /* v=v[v_index] */
} stopping_t;


double find_average_mass(isotopes_t *isotopes, int Z);
int find_Z_by_name(isotopes_t *isotopes, char *name); 
double find_mass(isotopes_t *isotope, int Z, int A); /* find isotope mass, but if A=0 calculate average mass of elem. */

isotopes_t *isotopes_load(const char *filename);
isotope_t *find_first_isotope(isotopes_t *isotopes, int Z);
isotope_t *find_most_abundant_isotope(isotopes_t *isotopes, int Z);
isotope_t *find_isotope(isotopes_t *isotopes, int Z, int A);
int find_all_isotopes(isotopes_t *isotopes, isotope_t **isotopes_out, int Z);   
isotope_t *isotope_find(isotopes_t *isotopes, const char *name);
stopping_t *init_stopping_table(char *filename);
int delete_stopping_table(stopping_t *stopping);


double velocity(double E, double mass); /* Use SI units */
double energy(double v, double mass);
#endif
