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
        char *elem=calloc(name_size, sizeof(char));
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

jibal_material *jibal_material_create(jibal_element *elements, const char *formula) {
    jibal_material *material=malloc(sizeof(jibal_material));
    material->n_elements=0;
    material->name=strdup(formula); /* Default name is the formula */
    const char *a=formula;
    const char *b;
    char *name;
    const char *line_end=formula+strlen(formula);
    int A;
    double conc;
    int i_element=0;
    for(a=formula; a < line_end; material->n_elements++) { /* Count the number of elements */
        if(!parse_element(a, &b, &A, &name, &conc)) {
            break;
        }
        a=b;
    }
    fprintf(stderr, "%i elements!!\n", material->n_elements);
    material->elements=calloc(material->n_elements, sizeof(jibal_element));
    material->concs=calloc(material->n_elements, sizeof(double));

    for(a=formula; a < line_end; i_element++) {

        if(!parse_element(a, &b, &A, &name, &conc)) {
            return NULL;
        }
#ifdef DEBUG
        fprintf(stderr, "Parsed. Name = %s, A = %i, conc = %g. Remaining to be parsed: \"%s\"\n", name, A, conc, b);
#endif
        jibal_element *element=jibal_element_copy(jibal_element_find(elements, name), A);
        fprintf(stderr, "Element: %p\n", element);
        free(name);
        if(!element) {
            return NULL;
        }
#ifdef DEBUG
        fprintf(stderr, "Found element Z=%i (aka %s)\n", element->Z, element->name);
#endif
        material->elements[i_element]=*element; /* Deep copy */
        material->concs[i_element]=conc;
        free(element);
        a=b;
    }
    return material;
}

void jibal_material_print(FILE * restrict stream, jibal_material *material) {
    int i;
    fprintf(stream, "Material %s has %i elements.\n", material->name, material->n_elements);
    for(i=0; i < material -> n_elements; i++) {
        jibal_element *element = &material->elements[i];
        fprintf(stream, "  %6.3f%% elements[%i]: %s (Z=%i), %i isotopes\n", material->concs[i]/C_PERCENT, i, element->name, element->Z, element->n_isotopes);
        int j;
        for(j=0; j < element->n_isotopes; j++) {
            const jibal_isotope *isotope = element->isotopes[j];
            fprintf(stream, "    %6.3f%% isotopes[%i]: %s (A=%i)\n", element->concs[i]/C_PERCENT, j, isotope->name, isotope->A);
        }
    }
}

void jibal_material_free(jibal_material *material) {
    if(!material) {
        return;
    }
    int i;
    free(material->elements);
    free(material->concs);
    free(material);
    /* TODO: free names of elements and formulae */
}