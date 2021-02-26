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

#ifndef JIBAL_MASSES_ISOTOPES_INITIAL_ALLOC
#define JIBAL_MASSES_ISOTOPES_INITIAL_ALLOC 4096 /* Guess for number of isotopes. Helps with allocation. Doesn't need to be exact. */
#endif

#define ABUNDANCE_THRESHOLD (1e-6)

#define JIBAL_ANY_Z (-1) /* Z number that has a special meaning (Z is anything!). This is used when we want to have
 * 1D arrays in stead of 2D arrays */

#define JIBAL_NAT_ISOTOPES 0 /* Special meaning of A=0, natural (abundance > 0.0) isotopes. Useful for natural elements. */
#define JIBAL_ALL_ISOTOPES -1  /* Special meaning of A=-1, all known isotopes */

#define JIBAL_ISOTOPE_NAME_LENGTH 8

typedef char isotope_name[JIBAL_ISOTOPE_NAME_LENGTH]; /* These should be null terminated */
typedef char element_name[JIBAL_ISOTOPE_NAME_LENGTH];

typedef struct {
    isotope_name name; /* "AAAXx eg. 239Pu" */
    int N;
    int Z;
    int A; /* A=Z+N */
    double mass;
    double abundance; /* Do not change this. It is not a concentration. */
} jibal_isotope; /* All jibal_isotopes are supposed to be static data */

typedef struct {
    element_name name; /* by default something like "Si", but "natSi", "28Si" are valid names too after jibal_element_copy() */
    int Z;
    int n_isotopes;
    const jibal_isotope **isotopes; /* Array of length n_isotopes, contents are pointers to isotopes */
    double *concs; /* This is NULL in "elements" table, but when used by jibal_material the concentrations of isotopes goes in an array here. */
    double avg_mass; /* Average mass */
} jibal_element;

jibal_isotope *jibal_isotopes_load(const char *filename);
int jibal_abundances_load(jibal_isotope *isotopes, const char *filename);
void jibal_isotopes_free(jibal_isotope *isotopes);
jibal_element *jibal_elements_populate(const jibal_isotope *isotopes);
int jibal_elements_Zmax(const jibal_element *elements);
void jibal_elements_free(jibal_element *elements);
const jibal_element * jibal_element_find(const jibal_element *elements, const char *name);
int jibal_element_number_of_isotopes(const jibal_element *element, double abundance_threshold);
jibal_element *jibal_element_copy(const jibal_element *element, int A); /* Create a copy of a single element, either with all known isotopes (A=-1), naturally abundant isotopes (A=0) or a single isotope (A = mass number) */
void jibal_element_normalize(jibal_element *element);
const jibal_isotope * jibal_isotope_find(const jibal_isotope *isotopes, const char *name, int Z, int A); /* Give either name or Z and A. If name is NULL Z and A are used. */
const char *jibal_element_name(const jibal_element *elements, int Z);
#endif /* _JIBAL_MASSES_H_ */
