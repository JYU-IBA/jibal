#include <stdio.h>
#include "masses.c"

int main(int argc, char **argv) {
    isotopes_t *isotopes=isotopes_load(NULL);
    isotope_t *incident=isotope_find(isotopes, argv[1]);
    printf("mass=%g (%g u)\n", incident->mass, incident->mass/C_U);
}
