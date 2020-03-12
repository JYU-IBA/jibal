#include <jibal.h>
#include <defaults.h>

jibal jibal_init() {
    jibal jibal;
    jibal.isotopes=jibal_isotopes_load(NULL);
    if(!jibal.isotopes) {
        fprintf(stderr, "Could not load isotope table.\n");
        return jibal;
    }
    jibal_abundances_load(jibal.isotopes, NULL);
    jibal.elements=jibal_elements_populate(jibal.isotopes);
    jibal.units=jibal_units_default();
    jibal.gsto=jibal_gsto_init(JIBAL_MAX_Z, NULL);
    return jibal;
}

void jibal_free(jibal *jibal) {
    if(jibal->units)
        jibal_units_free(jibal->units);
    if(jibal->elements)
        jibal_elements_free(jibal->elements);
    if(jibal->isotopes)
        jibal_isotopes_free(jibal->isotopes);
    if(jibal->gsto)
        jibal_gsto_free(jibal->gsto);
    /* Note, not freeing jibal itself! */
}
