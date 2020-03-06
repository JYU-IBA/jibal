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
#include <jibal_masses.h>
#include <jibal_units.h>
#include <jibal_gsto.h>
#include <jibal_material.h>

int main(int argc, char **argv) {
    jibal_isotope *isotopes=isotopes_load(NULL);
    abundances_load(isotopes, NULL);
    jibal_element *elements=elements_populate(isotopes);
    jibal_units *units=jibal_units_default();
    gsto_table_t *table; /* TODO: rename */
    if(!isotopes) {
        fprintf(stderr, "Couldn't load isotopes.\n");
        return -1;
    }
    jibal_isotope *incident=isotope_find(isotopes, argv[1], 0,0 );
    fprintf(stderr, "Z1=%i\nm1=%g kg (%g u)\n", incident->Z, incident->mass, incident->mass/C_U);

    if(argc<=2) {
        return -1;

    }
    char *target_string=argv[2];

    jibal_material *target=jibal_material_create(elements, target_string);
    if(!target) {
        fprintf(stderr, "\"%s\" is not a valid material formula\n", target_string);
        return -1;
    }
    jibal_material_print(stderr, target);
    table=gsto_init(91, NULL);
    if(!table)
        return -1;
    if(!jibal_stop_auto_assign(table, incident, target))
        return -1;
    gsto_load(table);


    int i;
    double E, E_low, E_step=0.0, E_high=0.0;
    if(argc >= 4) {
        E_low=jibal_get_val(units, UNIT_TYPE_ENERGY, argv[3]);
    }
    if(argc >= 6) {
        E_step=jibal_get_val(units, UNIT_TYPE_ENERGY, argv[4]);
        E_high=jibal_get_val(units, UNIT_TYPE_ENERGY, argv[5]);
    }
    fprintf(stderr, "E_low=%g, E_high=%g, E_step=%g\n", E_low/C_KEV, E_high/C_KEV, E_step/C_KEV);
    for(E=E_low; E <= E_high; E += E_step) {
        double S=jibal_stop(table, incident, target, E);
        fprintf(stdout, "%e %e\n", E/C_KEV, S/C_EV_TFU);
    }
    jibal_material_free(target);
    jibal_units_free(units);
    elements_free(elements);
    isotopes_free(isotopes);
    return 0;
}
