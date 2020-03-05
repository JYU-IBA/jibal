#ifndef _JIBAL_MATERIAL_H_
#define _JIBAL_MATERIAL_H_

#include <jibal_masses.h>

typedef struct {
    char *name;
    int n_elements;
    jibal_element *elements; /* Array of elements. N.B. we store our own "copies" of each element so that we can change things (such as isotopic concentrations)! */
    double *abundances;
} jibal_material;


jibal_material *jibal_material_create(const char *formula);

void jibal_material_free(jibal_material *material);

#endif