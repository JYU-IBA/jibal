#ifndef _JIBAL_MATERIAL_H_
#define _JIBAL_MATERIAL_H_

#include <stdio.h>
#include <jibal_masses.h>

typedef struct jibal_material {
    char *name;
    size_t n_elements;
    jibal_element *elements; /* Array of elements. N.B. we store our own "copies" of each element so that we can change things (such as isotopic concentrations)! */
    double *concs;
} jibal_material;


jibal_material *jibal_material_create(jibal_element *elements, const char *formula);
void jibal_material_normalize(jibal_material *material);
void jibal_material_print(FILE *stream, jibal_material *material);
void jibal_material_free(jibal_material *material);
jibal_material *jibal_material_copy(const jibal_material *material);

#endif /* _JIBAL_MATERIAL_H_ */
