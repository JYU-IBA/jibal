#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <math.h>
#include "masses.h"
//#include "win_compat.h"

int add_isotope_to_table(isotopes_t *isotopes, int Z, int N, int A, double mass, char name[MASSES_ELEMENT_LENGTH]) {
    int i;
    isotope_t *isotope;
    if(!isotopes) {
        return 0;
    }
    i=isotopes->n_isotopes;
    if(i>=MASSES_ISOTOPES || i<0) 
        return 0;
    isotope=&isotopes->i[i];
    isotope->N=N;
    isotope->Z=Z;
    isotope->A=A;
    isotope->mass=mass*C_U;
    isotope->abundance=0.0; /* Change this later */
    if(N+Z != A) {
        fprintf(stderr, "Mass number A=%i does not match with N=%i and Z=%i\n", A, N, Z);
    }
    strncpy(isotope->name, name, MASSES_ELEMENT_LENGTH);
    isotopes->n_isotopes++;
    return 1;
}


isotopes_t *isotopes_load(const char *filename) {
    char *line, *line_split;
    char *columns[6];
    char **col;
    char name[MASSES_ELEMENT_LENGTH];
    if(!filename) {
        filename=MASSES_FILE;
    } 
    FILE *in_file=fopen(filename, "r");
    if(!in_file) {
        fprintf(stderr, "Could not load isotope table from file %s\n", filename);
        return NULL;
    }
    line=malloc(sizeof(char)*MASSES_LINE_LENGTH);
    if(!line) 
        return NULL;
    isotopes_t *isotopes=malloc(sizeof(isotopes_t));
    if(!isotopes)
        return NULL;
    isotopes->n_isotopes=0;
    isotopes->i = malloc(sizeof(isotope_t)*MASSES_ISOTOPES);
    if(!isotopes->i)
        return NULL;
    while(fgets(line, MASSES_LINE_LENGTH, in_file) != NULL) {
        line_split=line; /* strsep will screw up line_split, reset for every new line */
        for (col = columns; (*col = strsep(&line_split, " \t")) != NULL;)
            if (**col != '\0')
                if (++col >= &columns[6])
                    break;
        snprintf(name, MASSES_ELEMENT_LENGTH, "%i%s", (int)strtol(columns[4], NULL, 10), columns[1]);
        add_isotope_to_table(isotopes,
                                strtoimax(columns[3], NULL, 10), 
                                strtoimax(columns[2], NULL, 10), 
                                strtoimax(columns[4], NULL, 10), 
                                strtod(columns[5], NULL),
                                name);
    }
    fclose(in_file);
    return isotopes;
}

isotope_t *find_first_isotope(isotopes_t *isotopes, int Z) {
    int i;
    isotope_t *isotope;
    for(i=0; i<isotopes->n_isotopes; i++) {
        isotope=&isotopes->i[i];
        if(isotope->Z == Z) { /* The right element */
            return isotope;
        }
    }
    return NULL; /* Nothing found */
}


double find_mass(isotopes_t *isotopes, int Z, int A) { /* if A=0 calculate average mass, otherwise return isotope mass */
    double mass=0.0;
    isotope_t *isotope;
    int i;
    for(i=0; i<isotopes->n_isotopes; i++) {
        isotope=&isotopes->i[i];
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

int find_Z_by_name(isotopes_t *isotopes, char *name) { /* Give just element name e.g. "Cu" */
    isotope_t *isotope;
    char *isotope_name;
    int i;
    for(i=0; i<isotopes->n_isotopes; i++) {
        isotope=&isotopes->i[i];
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

isotope_t *find_most_abundant_isotope(isotopes_t *isotopes, int Z) {
    int i;
    isotope_t *isotope;
    isotope_t *most_abundant_isotope=NULL;
    double abundance=0;
    for(i=0; i<isotopes->n_isotopes; i++) {
        isotope=&isotopes->i[i];
        if(isotope->Z == Z) { /* The right element */
            if(isotope->abundance > abundance) { /* Has higher abundance than anything found so far */
                abundance=isotope->abundance;
                most_abundant_isotope=isotope;
            }
        }
    }
    return most_abundant_isotope;
}

isotope_t *find_isotope(isotopes_t *isotopes, int Z, int A) {
    isotope_t *isotope;
    int i;
    for(i=0; i<isotopes->n_isotopes; i++) {
        isotope=&isotopes->i[i];
        if(isotope->Z == Z && isotope->A == A) {
            return isotope;
        }
    }
    return NULL;
}

int find_all_isotopes(isotopes_t *isotopes, isotope_t **isotopes_out, int Z) {
    isotope_t *isotope;
    int i, j;
    int n_isotopes=0;
    for(i=0; i<isotopes->n_isotopes; i++) {
        isotope=&isotopes->i[i];
        if(isotope->Z == Z)
           n_isotopes++;
    }
    if(!n_isotopes) {
        fprintf(stderr, "Did not find any isotopes matching Z=%i\n", Z);
        return 0;
    }
    *isotopes_out=malloc(sizeof(isotope_t)*n_isotopes);
    j=0;
    for(i=0; i<isotopes->n_isotopes; i++) {
        isotope=&isotopes->i[i];
        if(isotope->Z == Z) {
            if(j>=n_isotopes)
                break;
            isotopes_out[j]=isotope;
            j++;
        }
    } 
    return n_isotopes;
}
isotope_t *isotope_find(isotopes_t *isotopes, const char *name) {
    isotope_t *isotope;
    int i;
    for(i=0; i<isotopes->n_isotopes; i++) {
        isotope=&isotopes->i[i];
        if(strcmp(isotope->name, name) == 0) {
            return isotope;
        }
    }
    return NULL;
}
