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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <math.h>
#include <jibal/jibal_masses.h>
#include "defaults.h"
//#include "win_compat.h"

int isotope_set(jibal_isotope *isotope, int Z, int N, int A, double mass, isotope_name name) {
    int i;
    if(!isotope) {
        return -1;
    }
    isotope->N=N;
    isotope->Z=Z;
    isotope->A=A;
    isotope->mass=mass*C_U;
    isotope->abundance=0.0; /* Change this later */
    if(N+Z != A) {
        fprintf(stderr, "Mass number A=%i does not match with N=%i and Z=%i\n", A, N, Z);
    }
    strncpy(isotope->name, name, sizeof(isotope_name));
    return 0;
}


jibal_isotope *isotopes_load(const char *filename) {
    char *line, *line_split;
    char *columns[6];
    char **col;
    isotope_name name;
    if(!filename) {
        filename=JIBAL_MASSES_FILE;
    } 
    FILE *in_file=fopen(filename, "r");
    if(!in_file) {
        fprintf(stderr, "Could not load isotope table from file %s\n", filename);
        return NULL;
    }
    line=malloc(sizeof(char)*JIBAL_MASSES_LINE_LENGTH);
    if(!line) 
        return NULL;
    jibal_isotope *isotopes=malloc(sizeof(jibal_isotope)*(JIBAL_MASSES_ISOTOPES+1));
    int n=0;
    while(fgets(line, JIBAL_MASSES_LINE_LENGTH, in_file) != NULL) {
        line_split=line; /* strsep will screw up line_split, reset for every new line */
        for (col = columns; (*col = strsep(&line_split, " \t")) != NULL;)
            if (**col != '\0')
                if (++col >= &columns[6])
                    break;
        snprintf(name, sizeof(isotope_name), "%i%s", (int)strtol(columns[4], NULL, 10), columns[1]);
        isotope_set(isotopes+n,
                                strtoimax(columns[3], NULL, 10), 
                                strtoimax(columns[2], NULL, 10), 
                                strtoimax(columns[4], NULL, 10), 
                                strtod(columns[5], NULL),
                                name);
        if(n>=JIBAL_MASSES_ISOTOPES) {
            fprintf(stderr, "Too many isotopes! I was expecting a maximum of %i.\n", JIBAL_MASSES_ISOTOPES);
            return NULL;
        }
        n++;
    }
    isotopes[n].A=0; /* "Null terminate" */
    fclose(in_file);
    return isotopes;
}

jibal_element *elements_populate(jibal_isotope *isotopes) {
    jibal_element *elements=calloc(JIBAL_ELEMENTS+1, sizeof(jibal_element));
    jibal_isotope *isotope;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z < 0 || isotope->Z > JIBAL_ELEMENTS) {
            continue;
        }
        jibal_element *e=&elements[isotope->Z];
        if(e->n_isotopes == 0) { /* We encounter this element for the first time */
            char *isotope_name;
            for(isotope_name=isotope->name; isdigit(*isotope_name); isotope_name++);
            strncpy(e->name, isotope_name, sizeof(element_name)-1);
        }

        e->n_isotopes++;
    }
    int Z;
    for(Z=0; Z <= JIBAL_ELEMENTS; Z++) {
        if(elements[Z].n_isotopes > 0) {
            elements[Z].isotopes=malloc(sizeof(jibal_isotope)*elements[Z].n_isotopes);
        }
#ifdef DEBUG
        fprintf(stderr, "Element %i, name %s, %i isotopes\n", Z, elements[Z].name, elements[Z].n_isotopes);
#endif
    }
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z < 0 || isotope->Z >= JIBAL_ELEMENTS) {
            continue;
        }
        jibal_element *e=&elements[isotope->Z];
        int i;
        for(i=0; i < e->n_isotopes && e->isotopes[i] == NULL; i++);
        e->isotopes[i]=isotope;
    }
    return elements;
}


jibal_isotope *find_first_isotope(jibal_isotope *isotopes, int Z) {
    int i;
    jibal_isotope *isotope;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z == Z) { /* The right element */
            return isotope;
        }
    }
    return NULL; /* Nothing found */
}


double find_mass(jibal_isotope *isotopes, int Z, int A) { /* if A=0 calculate average mass, otherwise return isotope mass */
    double mass=0.0;
    jibal_isotope *isotope;
    int i;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z == Z) {
            if(isotope->A == A) {
                return isotope->mass;
            }
            if(!A) {
                mass += isotope->mass*isotope->abundance;
            }
        }
    }
    return mass;
}

int jibal_find_Z_by_name(jibal_isotope *isotopes, char *name) { /* Give just element name e.g. "Cu" */
    jibal_isotope *isotope;
    char *isotope_name;
    int i;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        isotope_name=isotope->name;
        while(isdigit(*isotope_name)) /* Skip numbers */
            isotope_name++;
        if(*isotope_name == '-') /* and dash */
            isotope_name++;
        if(strcmp(isotope_name, name) == 0) {
            return isotope->Z;
        }
    }
    return 0;
}

jibal_isotope *find_most_abundant_isotope(jibal_isotope *isotopes, int Z) {
    int i;
    jibal_isotope *isotope;
    jibal_isotope *most_abundant_isotope=NULL;
    double abundance=0;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z == Z) { /* The right element */
            if(isotope->abundance > abundance) { /* Has higher abundance than anything found so far */
                abundance=isotope->abundance;
                most_abundant_isotope=isotope;
            }
        }
    }
    return most_abundant_isotope;
}

jibal_isotope *find_isotope(jibal_isotope *isotopes, int Z, int A) {
    jibal_isotope *isotope;
    int i;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z == Z && isotope->A == A) {
            return isotope;
        }
    }
    return NULL;
}

jibal_isotope *tisotopes_find_all(jibal_isotope *isotopes, int Z) {
    jibal_isotope *isotope;
    jibal_isotope *isotopes_out; /*!< Table, size to be determined */
    int i, j;
    int n_isotopes=0;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z == Z)
           n_isotopes++;
    }
    if(!n_isotopes) {
        fprintf(stderr, "Did not find any isotopes matching Z=%i\n", Z);
        return 0;
    }
    isotopes_out=malloc(sizeof(jibal_isotope)*n_isotopes);
    j=0;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z == Z) {
            if(j>=n_isotopes)
                break;
            memcpy(isotopes_out+j, isotope, sizeof(jibal_isotope));
            j++;
        }
    } 
    return isotopes_out;
}
jibal_isotope *isotope_find(jibal_isotope *isotopes, const char *name) {
    jibal_isotope *isotope;
    int i=0;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(strcmp(isotope->name, name) == 0) {
            return isotope;
        }
    }
    return NULL;
}
