#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#define HEADER_LEN 39
#define ELEM_LEN 4 /* Max 3 bytes for element name and null termination */
#define MAX_ISOTOPES 3500

#ifndef AMEFILE
#define AMEFILE "mass16.txt"
#endif

typedef struct {
    char el[ELEM_LEN];
    int N;
    int Z;
    int A;
    double mass;
} isotope_t;

isotope_t *populate_isotopes(const char *filename, int *n_isotopes) {
    int n_cols=17;
    char *line;
    int i, i_col, i_name;
    static unsigned char col_offsets[] = {0, 2, 6, 11, 16, 20, 24, 29, 42, 55, 65, 73, 76, 87, 96, 100, 114, 123};
    int len=col_offsets[n_cols]+1;
    int headers=39;
    int lineno=0;
    int terminates=1, newline=0;
    char **cols;
    FILE *in;

    in=fopen(filename, "r");
    if(!in)
        return NULL;

    cols=malloc(sizeof(char *)*n_cols);
    isotope_t *isotopes=malloc(sizeof(isotope_t)*MAX_ISOTOPES);
    line=malloc(sizeof(char)*len);

    for(i_col=0; i_col < n_cols; i_col++) {
        cols[i_col]=line+col_offsets[i_col];
    }
    i=0;
    while(fgets(line, len, in)) {
        if(terminates) {
            newline=1;
            lineno++;
        } else {
            newline=0;
        }
        if(line[strlen(line)-1] == '\n') {
            line[strlen(line)-1] = '\0';
            terminates=1;
        } else {
            terminates=0;
        }
        if(!newline) {
            continue;
        }/*
        fprintf(stderr, "%i: %s\n", lineno, line);*/
        if(lineno <= headers)
            continue;
        i_col=0;
        /*
        for(i=0; i < len; i++) {
            if(!line[i])
                break;
            if(i==col_offsets[i_col]) {
                fprintf(stdout, "\n%i: ", i_col);
                i_col++;
            }
            putchar(line[i]);

        }*/
        /* TODO: check length of line (validity of data) */
        char *c;
        i_name=0;
        isotopes[i].N=strtoimax(cols[2], NULL, 10);
        isotopes[i].Z=strtoimax(cols[3], NULL, 10);
        isotopes[i].A=strtoimax(cols[4], NULL, 10);
        for(c=cols[5]; c < cols[5]+ELEM_LEN-1; c++) {
            if(isspace(*c))
                continue;
            if(!isalpha(*c))
                break;
            isotopes[i].el[i_name++] = *c;
        }
        isotopes[i].el[i_name]='\0';
        double m=strtod(cols[14], NULL)+strtod(cols[15], NULL)/1e6;
        isotopes[i].mass=m;
        i++;
    }
    *n_isotopes=i;
    free(cols);
    free(line);
    fclose(in);
    return isotopes;
}


void print_isotopes(isotope_t *isotopes, int n_isotopes) {
    int i;
    isotope_t *isotope;
    for(i=0; i < n_isotopes; i++) {
        isotope=&(isotopes[i]);
        printf("%4i %4s %3i %3i %3i %10.6lf\n", i, isotope->el, isotope->N, isotope->Z, isotope->A, isotope->mass);
    }
}

int main(int argc, char **argv) {
    int n_isotopes;
    isotope_t *isotopes;
    isotopes=populate_isotopes(AMEFILE, &n_isotopes);
    if(!isotopes) {
        fprintf(stderr, "Couldn't load file %s\n", AMEFILE);
        return -1;
    }
    fprintf(stderr, "Masses from file %s\n", AMEFILE);
    print_isotopes(isotopes, n_isotopes);
    return 0;
}


