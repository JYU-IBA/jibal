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
#include <assert.h>
#include "win_compat.h"
#include "defaults.h"

//#include "win_compat.h"

int isotope_set(jibal_isotope *isotope, int Z, int N, int A, double mass, isotope_name name) {
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


jibal_isotope *jibal_isotopes_load(const char *filename) {
    char *line_split;
    char *columns[6];
    char **col;
    isotope_name name;
    if(!filename) {
        filename=JIBAL_MASSES_FILE;
    } 
    FILE *in_file=fopen(filename, "r");
    if(!in_file) {
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

int jibal_abundances_load(jibal_isotope *isotopes, const char *filename) {
    int n;
    if(!filename) {
        filename=JIBAL_ABUNDANCES_FILE;
    }
    FILE *in_file=fopen(filename, "r");
    if(!in_file) {
        fprintf(stderr, "Could not load isotope abundances table from file %s\n", filename);
        return -1;
    }
    char line[JIBAL_LINE_LENGTH];
    for(n=0; fgets(line, JIBAL_LINE_LENGTH, in_file) != NULL; n++) {
        int Z, A;
        double abundance;
        if(sscanf(line, "%i %i %lf", &Z, &A, &abundance) != 3) { /* Failure (problem with data). Let's keep the good data anyway. */
            return n;
        }
        jibal_isotope *isotope=jibal_isotope_find(isotopes, NULL, Z, A);
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

void jibal_isotopes_free(jibal_isotope *isotopes) {
    free(isotopes);
}

jibal_element *jibal_elements_populate(const jibal_isotope *isotopes) {
    if(!isotopes)
        return NULL;
    const jibal_isotope *isotope;
    int Z_max=0;
    for(isotope=isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z > Z_max) {
            Z_max=isotope->Z;
        }
    }
    jibal_element *elements=calloc(Z_max+2, sizeof(jibal_element)); /* +1 because 0..Z_MAX incl. and +1 because of null termination */
    for(isotope=isotopes; isotope->A != 0; isotope++) { /* Count isotopes and set name of element */
        if(isotope->Z < 0) {
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
    for(Z=0; Z <= Z_max; Z++) {
        elements[Z].Z = Z;
        if(elements[Z].n_isotopes > 0) {
            elements[Z].isotopes=calloc(elements[Z].n_isotopes, sizeof(jibal_isotope *));
        }
#ifdef DEBUG
        fprintf(stderr, "Element %i, name %s, %i isotopes\n", Z, elements[Z].name, elements[Z].n_isotopes);
#endif
    }
    for(isotope=isotopes; isotope->A != 0; isotope++) { /* Copy pointers to isotopes */
        if(isotope->Z < 0) {
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
    for(Z=0; Z <= Z_max; Z++) {
        jibal_element *element = &elements[Z];
        int i;
        /* Note that we don't have a concentration table for "bare" elements. We can still calculate the average
         * mass. */
        element->avg_mass=0.0;
        for(i=0; i < element->n_isotopes; i++) {
            element->avg_mass += element->isotopes[i]->abundance*element->isotopes[i]->mass;
        }
        if(*element->name == '\0') {
            fprintf(stderr, "WARNING: Element %i does not have a name. Naming it element \"X\".\n", Z);
            *element->name = 'X'; /* This is important.
 * The last member, i.e. elements[Z_max+1] has an empty name due to the calloc above and can be used to stop a loop over
 * all of the elements. */
        }
    }
    return elements;
}

int jibal_elements_Zmax(const jibal_element *elements) {
    const jibal_element *e;
    int n=0;
    for(e=elements+1; e->Z != 0; e++) {
        n++;
        assert(n == e->Z);
    } /* Skipping elements[0], because elements[0].Z=0 */
    return n;
}

void jibal_elements_free(jibal_element *elements) {
    jibal_element *e;
    for(e=elements; e->name[0] != '\0'; e++) {
        if(e->n_isotopes > 0 && e->isotopes) {
            free(e->isotopes);
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

const jibal_element * jibal_element_find(const jibal_element *elements, element_name name) {
    /* Element name is typically something like "Si", but if the name looks like a number (only digits 0-9) and is in
     * the valid range, we assume it is the proton number. */
    int Z;
    char *n=name;
    if(*name == '\0')
        return NULL;
    for(Z=0; isdigit(*n);  Z = Z*10+*(n++)-'0');

    int Z_max = jibal_elements_Zmax(elements);

    if(Z >= 0 && Z <= Z_max && !*n) {
        assert(Z == elements[Z].Z); /* TODO: could be removed */
        return &elements[Z];
    }
    const jibal_element *e;
    for(e=elements; e->name[0] != '\0'; e++) {
        if(strncmp(e->name, name, sizeof(element_name))==0) {
            return e;
        }
    }
    return NULL;
}

int jibal_element_number_of_isotopes(const jibal_element *element, double abundance_threshold) {
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

jibal_element *jibal_element_copy(const jibal_element *element, int A) {
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
        case JIBAL_ALL_ISOTOPES:
            n=element->n_isotopes;
            break;
        case JIBAL_NAT_ISOTOPES:
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
            case JIBAL_ALL_ISOTOPES: /* All isotopes */
                e->isotopes[i] = element->isotopes[i];
                e->concs[i] = element->isotopes[i]->abundance;
                j++;
                break;
            case JIBAL_NAT_ISOTOPES: /* Natural isotopes (above threshold) */
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
    if(!element->concs) { /* This array doesn't exist unless the element was created with jibal_element_new() */
        return;
    }
    for(i=0; i < element->n_isotopes; i++) {
        sum += element->concs[i];
    }
    if(sum == 0.0) { /* TODO: threshold? Shouldn't be necessary, but you never know. */
        return;
    }
    element->avg_mass=0.0;
    for(i=0; i < element->n_isotopes; i++) {
        element->concs[i] /= sum;
        element->avg_mass += element->concs[i]*element->isotopes[i]->mass;
    }
}

jibal_isotope *jibal_isotope_find(jibal_isotope *isotopes, const char *name, int Z, int A) {
    jibal_isotope *isotope;
    if(name != NULL) {
        if (isdigit(*name)) { /* Isotope names usually start with a mass number */
            for (isotope = isotopes; isotope->A != 0; isotope++) {
                if (strcmp(isotope->name, name) == 0) {
                    return isotope;
                }
            }
            return NULL; /* No match */
        }
        switch (*name) { /* Couple of exceptions (hard coded one character nicknames) */
            case 'n': /* Not sure we should support neutrons, but here we are */
                Z = 0;
                A = 1;
                break;
            case 'p':
                Z = 1;
                A = 1;
                break;
            case 'D':
                Z = 1;
                A = 2;
                break;
            case 'T':
                Z = 1;
                A = 3;
                break;
            case 'a':
                Z = 2;
                A = 4;
                break;
            default:
                return NULL;
        }
    }
    for (isotope = isotopes; isotope->A != 0; isotope++) {
        if(isotope->Z == Z && isotope->A == A) {
            return isotope;
        }
    }
    return NULL;
}

const char *jibal_element_name(const jibal_element *elements, int Z) {
    if(Z == JIBAL_ANY_Z)
        return "Any";
    const jibal_element *e;
    for(e=elements; e->name[0] != '\0'; e++) {
        if(e->Z == Z) {
            return e->name;
        }
    }
    return "Err";
}
