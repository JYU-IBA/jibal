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

#include <stdio.h>
#include "masses.h"
#include "units.h"

#include "gsto.h"

int get_single_stop(gsto_table_t *table, iba_isotope *incident, double E, int Z2) {
    double v;
    v=velocity(E, incident->mass);
    fprintf(stderr, "Printing stopping for %i in %i at v=%e m/s from file %s.\n", incident->Z, Z2, v, table->assigned_files[incident->Z][Z2]->name);
    printf("%e\n", gsto_sto_v(table, incident->Z, Z2, v));
    return 1; 
}


int main(int argc, char **argv) {
    iba_isotope *isotopes=isotopes_load(NULL);
    iba_units *units=iba_units_default();
    gsto_table_t *table; /* TODO: rename */
    if(!isotopes) {
        fprintf(stderr, "Couldn't load isotopes.\n");
    }
    iba_isotope *incident=isotope_find(isotopes, argv[1]);
    printf("mass=%g (%g u)\n", incident->mass, incident->mass/C_U);

    double E=0.0;
    if(argc<=2) {
        return -1;

    }
    char *target_name;
 
    target_name=argv[2];
    int Z2=iba_find_Z_by_name(isotopes, target_name);
    if(!Z2) {
        fprintf(stderr, "No element %s found\n", target_name);
            return -1;
    }
    table=gsto_init(91, "stoppings.txt");
    if(!table)
        return -1;
    if(!gsto_auto_assign(table, incident->Z, Z2))
            return -1;
    gsto_load(table);

    int i;
    for(i=3; i<argc; i++) {
        E=iba_get_val(units, UNIT_TYPE_ENERGY, argv[i]);
        printf("E=%g keV\n", E/C_KEV);
        get_single_stop(table, incident, E, Z2);
    }
    iba_units_free(units);
}
