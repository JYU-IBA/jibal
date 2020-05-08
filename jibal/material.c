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
        char *elem=calloc(name_size+1, sizeof(char));
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
#ifdef DEBUG
    fprintf(stderr, "Parsing material formula: \"%s\"\n", formula);
#endif
    for(a=formula; a < line_end; material->n_elements++) { /* Count the number of elements */
        if(!parse_element(a, &b, &A, &name, &conc)) {
            break;
        }
        a=b;
    }
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
    jibal_material_normalize(material);
    return material;
}
void jibal_material_normalize(jibal_material *material) {
    double sum=0.0;
    int i;
    for(i=0; i < material->n_elements; i++) {
        sum += material->concs[i];
    }
    if(sum == 0.0) { /* TODO: threshold? */
        return;
    }
    for(i=0; i < material->n_elements; i++) {
        material->concs[i] /= sum;
    }
}

void jibal_material_print(FILE *stream, jibal_material *material) {
    int i;
    fprintf(stream, "Material %s has %i elements.\n", material->name, material->n_elements);
    for(i=0; i < material -> n_elements; i++) {
        jibal_element *element = &material->elements[i];
        fprintf(stream, "    %2i. %s (Z=%i): %9.5f%%  with %i isotopes (avg mass %g u):\n", i+1, element->name, element->Z, material->concs[i]/C_PERCENT, element->n_isotopes, element->avg_mass/C_U);
        int j;
        for(j=0; j < element->n_isotopes; j++) {
            const jibal_isotope *isotope = element->isotopes[j];
            fprintf(stream, "      %2i. %5s: %9.5lf%%  (A=%i, mass=%g u, abundance=%.5lf%%)\n", j+1, isotope->name, element->concs[j]/C_PERCENT,
                    isotope->A, isotope->mass/C_U, isotope->abundance / C_PERCENT);
        }
    }
}

void jibal_material_free(jibal_material *material) {
    if(!material) {
        return;
    }
    free(material->elements);
    free(material->concs);
    free(material);
    /* TODO: free names of elements and formulae */
}
