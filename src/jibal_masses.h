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

#include <jibal_units.h>
#include <jibal_phys.h>

#define JIBAL_MASSES_LINE_LENGTH 80

#ifndef JIBAL_MASSES_ISOTOPES
#define JIBAL_MASSES_ISOTOPES 4095 /* Maximum number of isotopes.  */
#endif

#ifndef JIBAL_ELEMENTS
#define JIBAL_ELEMENTS 118
#endif

#define ABUNDANCE_THRESHOLD (1e-6)

typedef char isotope_name[8]; /* These should be null terminated */
typedef char element_name[4];

typedef struct {
    isotope_name name; /* "A-Xx eg. 239-Pu" */
    int N;
    int Z;
    int A; /* A=Z+N */
    double mass;
    double abundance; /* Do not change this. It is not a concentration. */
} jibal_isotope; /* All jibal_isotopes are supposed to be static data */

typedef struct {
    element_name name;
    int Z;
    int n_isotopes;
    jibal_isotope const **isotopes; /* Array of length n_isotopes, contents are pointers to isotopes */
    double *concs; /* This is NULL in "elements" table, but when used by jibal_material the concentrations of isotopes goes in an array here. */
} jibal_element;

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
void isotopes_free(jibal_isotope *isotopes);
jibal_element *elements_populate(const jibal_isotope *isotopes);
void elements_free(jibal_element *elements);
jibal_element *jibal_element_find(jibal_element *elements, element_name name);
int jibal_element_number_of_isotopes(jibal_element *element, double abundance_threshold);
jibal_element *jibal_element_copy(jibal_element *element, int A); /* Create a copy of a single element, either with all known isotopes (A=-1), naturally abundant isotopes (A=0) or a single isotope (A = mass number) */
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
