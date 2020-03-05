#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <jibal_material.h>

const char *parse_element(const char *start, const char **end_ptr, int *A_out, char **name_out, double *conc_out) {
    /* Given string start, e.g. "28Si0.333", A_out, name_out and conc_out are outputs 28 (int), "Si" and 0.333 (double),
     * respectively. end_ptr will be assigned to NULL on errors and otherwise until end of conversion */
    const char *a=start;
    *end_ptr=NULL;
    int A;
    while(*a != '\0') {
        if (*a == '\n' || *a == ' ') {
            a++;
            continue;
        }
        for (A=0; isdigit(*a); a++) {
            A *= 10;
            A += (*a - '0');
        }
#ifdef DEBUG
        fprintf(stderr, "A=%i\n", A);
#endif
        if (!isupper(*a)) {
#ifdef DEBUG
            fprintf(stderr, "Names of elements start with capitals.\n");
#endif
            return NULL;
        }
        const char *elem_start = a;
        a++;
        for (; islower(*a); a++); /* Advance until we run out of lower case elements */
        size_t name_size=a-elem_start;
        char *elem=malloc(name_size * sizeof(char));
        strncpy(elem, elem_start, a - elem_start);
#ifdef DEBUG
        fprintf(stderr, "Element = \"%s\"\n", elem);
#endif
        char *end;
        double conc;
        if(*a == '\0' || *a == ' ' || isupper(*a)) { /* End of the string or a new element (space or capital) before we had a chance to read the concentration */
            conc = 1.0;
        } else {
            conc = strtod(a, &end);
            if (end == a || !end) { /* This shouldn't happen */
#ifdef DEBUG
                fprintf(stderr, "The unexpected has happened.\n");
#endif
                conc = 0.0;
                free(elem);
                return NULL;
            }
            a = end;
        }
#ifdef DEBUG
        fprintf(stderr, "Concentration = %g\n", conc);
#endif
        *A_out=A;
        *conc_out=conc;
        *end_ptr=a;
        *name_out=elem;
        return start;
    }
    return  NULL; /* We shouldn't reach this point */
}

jibal_material *jibal_material_create(const char *formula) {
    jibal_material *material=malloc(sizeof(jibal_material));

    const char *a=formula;
    const char *b;
    char *name;
    const char *line_end=formula+strlen(formula);
    while(a < line_end) {
        int A;
        double conc;
        if(!parse_element(a, &b, &A, &name, &conc)) {
            return NULL;
        }
        fprintf(stderr, "Parsed. Name = %s, A = %i, conc = %g. Remaining to be parsed: \"%s\"\n", name, A, conc, b);
        a=b;
    }
    return material;
}

void jibal_material_free(jibal_material *material) {
    if(material) {
        free(material);
    }
    /* TODO: free names of elements and formulae */
}