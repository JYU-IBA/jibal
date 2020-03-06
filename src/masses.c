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
#include <jibal_masses.h>
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
    isotope->abundance=0.0; /* TODO: Change this later */
    if(N+Z != A) {
        fprintf(stderr, "Mass number A=%i does not match with N=%i and Z=%i\n", A, N, Z);
    }
    strncpy(isotope->name, name, sizeof(isotope_name));
    return 0;
}


jibal_isotope *isotopes_load(const char *filename) {
    char *line_split;
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
    char line[JIBAL_LINE_LENGTH];
    jibal_isotope *isotopes=malloc(sizeof(jibal_isotope)*(JIBAL_MASSES_ISOTOPES+1));
    int n=0;
    while(fgets(line, JIBAL_LINE_LENGTH, in_file) != NULL) {
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
            free(line);
            return NULL;
        }
        n++;
    }
    isotopes[n].A=0; /* "Null terminate" */
#ifdef DEBUG
    fprintf(stderr, "Loaded %i isotopes (maximum set at %i) from %s\n", n, JIBAL_MASSES_ISOTOPES, filename);
#endif
    fclose(in_file);
    return isotopes;
}

int abundances_load(jibal_isotope *isotopes, const char *filename) {
    int n;
    if(!filename) {
        filename=JIBAL_ABUNDANCES_FILE;
    }
    FILE *in_file=fopen(filename, "r");
    if(!in_file) {
        fprintf(stderr, "Could not load isotope table from file %s\n", filename);
        return 0;
    }
    char line[JIBAL_LINE_LENGTH];
    for(n=0; fgets(line, JIBAL_LINE_LENGTH, in_file) != NULL; n++) {
        int Z, A;
        double abundance;
        if(sscanf(line, "%i %i %lf", &Z, &A, &abundance) != 3) { /* Failure (problem with data). Let's keep the good data anyway. */
            return n;
        }
        jibal_isotope *isotope=isotope_find(isotopes, NULL, Z, A);
        if(!isotope) {
#ifdef DEBUG
            fprintf(stderr, "Couldn't find isotope with Z=%i and A=%i\n", Z, A);
#endif
            return n;
        }
        isotope->abundance=abundance;
#ifdef DEBUG
        fprintf(stderr, "Abundance of %s is now %.8lf.%s\n", isotope->name, abundance, abundance<ABUNDANCE_THRESHOLD?" WARNING: This is below threshold!":"");
#endif
    }
#ifdef DEBUG
    fprintf(stderr, "Successfully loaded %i abundances from %s\n", n, filename);
#endif
    return n;
}

void isotopes_free(jibal_isotope *isotopes) {
    free(isotopes);
}

jibal_element *elements_populate(const jibal_isotope *isotopes) {
    jibal_element *elements=calloc(JIBAL_ELEMENTS+1, sizeof(jibal_element));
    const jibal_isotope *isotope;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z < 0 || isotope->Z > JIBAL_ELEMENTS) {
            continue;
        }
        jibal_element *e=&elements[isotope->Z];
        if(e->n_isotopes == 0) { /* We encounter this element for the first time */
            const char *isotope_name;
            for(isotope_name=isotope->name; isdigit(*isotope_name); isotope_name++);
            strncpy(e->name, isotope_name, sizeof(element_name)-1);
        }

        e->n_isotopes++;
    }
    int Z;
    for(Z=0; Z <= JIBAL_ELEMENTS; Z++) {
        elements[Z].Z = Z;
        if(elements[Z].n_isotopes > 0) {
            //elements[Z].isotopes=calloc(elements[Z].n_isotopes, sizeof(jibal_isotope));
            elements[Z].isotopes=calloc(elements[Z].n_isotopes, sizeof(jibal_isotope *));
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
        for(i=0; i < e->n_isotopes && e->isotopes[i] != NULL; i++); /* Find next free slot */
        e->isotopes[i]=isotope; /* Shallow copy */
#ifdef DEBUG
        fprintf(stderr, "Element %i isotope %i is A=%i\n", isotope->Z, i, isotope->A);
#endif
    }
    return elements;
}

void elements_free(jibal_element *elements) {
    int Z;
    for(Z=0; Z <= JIBAL_ELEMENTS; Z++) {
        if(elements[Z].n_isotopes > 0 && elements[Z].isotopes) {
            free(elements[Z].isotopes);
        }
    }
    free(elements);
}

jibal_element *jibal_element_new(const element_name name, int Z, int n_isotopes) { /* Makes a BLANK element (no isotopes) */
    jibal_element *e=malloc(sizeof(jibal_element));
    e->Z=Z;
    e->n_isotopes=n_isotopes;
    e->isotopes=calloc(n_isotopes, sizeof(jibal_isotope *));
    e->concs=calloc(n_isotopes, sizeof(double));
    strncpy(e->name, name, sizeof(element_name)-1);
    return e;
}

jibal_element *jibal_element_find(jibal_element *elements, element_name name) {
    int Z;
    for(Z=0; Z <= JIBAL_ELEMENTS; Z++) {
        if(strncmp(elements[Z].name, name, sizeof(element_name))==0) {
            return &elements[Z];
        }
    }
    return NULL;
}

int jibal_element_number_of_isotopes(jibal_element *element, double abundance_threshold) {
    if(!element)
        return 0;
    int i, n=0;
    for(i=0; i < element->n_isotopes; i++) {
        if(element->isotopes[i]->abundance >= abundance_threshold) {
            n++;
#ifdef DEBUG
            fprintf(stderr, "A=%i above threshold, since %g > %g\n", element->isotopes[i]->A, element->isotopes[i]->abundance, abundance_threshold);
#endif
        }
    }
    return n;
}

jibal_element *jibal_element_copy(jibal_element *element, int A) {
    if(!element || A < -1) {
        return NULL;
    }
    jibal_element *e=malloc(sizeof(jibal_element)); /* output */
    e->Z=element->Z;
    strncpy(e->name, element->name, sizeof(element_name)-1);
    int n=0;
#ifdef DEBUG
    fprintf(stderr, "Trying to figure out based on A=%i how many of the %i isotopes to include.\n", A, element->n_isotopes);
#endif
    switch (A) {
        case -1:
            n=element->n_isotopes;
            break;
        case 0:
            n=jibal_element_number_of_isotopes(element, ABUNDANCE_THRESHOLD);
            break;
        default: /* Single isotope */
            n=1;
            break;
    }
#ifdef DEBUG
    fprintf(stderr, "The answer is %i isotopes\n", n);
#endif
    e=jibal_element_new(element->name, element->Z, n);
    int i,j=0;
    for(i=0; i < element->n_isotopes; i++) {
        switch(A) {
            case -1: /* All isotopes */
                e->isotopes[i] = element->isotopes[i];
                e->concs[i] = element->isotopes[i]->abundance;
                j++;
                break;
            case 0: /* Natural isotopes (above threshold) */
                if(element->isotopes[i]->abundance >= ABUNDANCE_THRESHOLD && j < n) {
                    e->isotopes[j] = element->isotopes[i];
                    e->concs[j] = element->isotopes[i]->abundance;
#ifdef DEBUG
                    fprintf(stderr, "Assigned isotope %i from isotope %i (element %s), abundance %.6f\n", j, i, element->name, element->isotopes[i]->abundance);
#endif
                    j++;
                }
                break;
            default: /* Single isotope case */
                if(element->isotopes[i]->A == A) {
                    e->isotopes[0] = element->isotopes[i];
                    e->concs[0] = 1.0;
                    j++;
                }
                break;
        }
    }
#ifdef DEBUG
    fprintf(stderr, "Found %i isotopes out of %i\n", j, n);
#endif
    if(j==n) { /* We found all the isotopes we were looking for */
        jibal_element_normalize(e);
        return e;
    } else {
        free(e);
        return NULL;
    }
}

void jibal_element_normalize(jibal_element *element) {
    double sum=0.0;
    int i;
    for(i=0; i < element->n_isotopes; i++) {
        sum += element->concs[i];
    }
    if(sum == 0.0) {
        return;
    }
    for(i=0; i < element->n_isotopes; i++) {
        element->concs[i] /= sum;
    }
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

jibal_isotope *isotope_find(jibal_isotope *isotopes, const char *name, int Z, int A) {
    jibal_isotope *isotope;
    int i=0;
    if(name != NULL) {
        for (isotope = isotopes; isotope->A != 0; isotope++) {
            if (strcmp(isotope->name, name) == 0) {
                return isotope;
            }
        }
        return  NULL;
    } else {
        for (isotope = isotopes; isotope->A != 0; isotope++) {
            if(isotope->Z == Z && isotope->A == A) {
                return isotope;
            }
        }
        return NULL;
    }
}
