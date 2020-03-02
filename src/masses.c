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
//#include "win_compat.h"

int isotope_set(iba_isotope *isotope, int Z, int N, int A, double mass, isotope_name name) {
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


iba_isotope *isotopes_load(const char *filename) {
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
    iba_isotope *isotopes=malloc(sizeof(iba_isotope)*(JIBAL_MASSES_ISOTOPES+1));
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

iba_isotope *find_first_isotope(iba_isotope *isotopes, int Z) {
    int i;
    iba_isotope *isotope;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z == Z) { /* The right element */
            return isotope;
        }
    }
    return NULL; /* Nothing found */
}


double find_mass(iba_isotope *isotopes, int Z, int A) { /* if A=0 calculate average mass, otherwise return isotope mass */
    double mass=0.0;
    iba_isotope *isotope;
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

int iba_find_Z_by_name(iba_isotope *isotopes, char *name) { /* Give just element name e.g. "Cu" */
    iba_isotope *isotope;
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

iba_isotope *find_most_abundant_isotope(iba_isotope *isotopes, int Z) {
    int i;
    iba_isotope *isotope;
    iba_isotope *most_abundant_isotope=NULL;
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

iba_isotope *find_isotope(iba_isotope *isotopes, int Z, int A) {
    iba_isotope *isotope;
    int i;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z == Z && isotope->A == A) {
            return isotope;
        }
    }
    return NULL;
}

iba_isotope *tisotopes_find_all(iba_isotope *isotopes, int Z) {
    iba_isotope *isotope;
    iba_isotope *isotopes_out; /*!< Table, size to be determined */
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
    isotopes_out=malloc(sizeof(iba_isotope)*n_isotopes);
    j=0;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z == Z) {
            if(j>=n_isotopes)
                break;
            memcpy(isotopes_out+j, isotope, sizeof(iba_isotope));
            j++;
        }
    } 
    return isotopes_out;
}
iba_isotope *isotope_find(iba_isotope *isotopes, const char *name) {
    iba_isotope *isotope;
    int i=0;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(strcmp(isotope->name, name) == 0) {
            return isotope;
        }
    }
    return NULL;
}
