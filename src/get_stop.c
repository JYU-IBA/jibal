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
#include <jibal/jibal_masses.h>
#include <jibal/jibal_units.h>

#include <jibal/jibal_gsto.h>

int main(int argc, char **argv) {
    jibal_isotope *isotopes=isotopes_load(NULL);
    jibal_element *elements=elements_populate(isotopes);
    jibal_units *units=jibal_units_default();
    gsto_table_t *table; /* TODO: rename */
    if(!isotopes) {
        fprintf(stderr, "Couldn't load isotopes.\n");
        return -1;
    }
    jibal_isotope *incident=isotope_find(isotopes, argv[1]);
    fprintf(stderr, "m_1=%g kg (%g u)\n", incident->mass, incident->mass/C_U);

    double E=0.0;
    if(argc<=2) {
        return -1;

    }
    char *target_name;
 
    target_name=argv[2];
    int Z2=jibal_find_Z_by_name(isotopes, target_name);
    if(!Z2) {
        fprintf(stderr, "No element %s found\n", target_name);
            return -1;
    }
    fprintf(stderr, "Z1=%i, Z2=%i\n", incident->Z, Z2);
    table=gsto_init(91, NULL);
    if(!table)
        return -1;
    if(!gsto_auto_assign(table, incident->Z, Z2))
            return -1;
    fprintf(stderr, "Stopping data from file %s (%s)\n", table->assigned_files[incident->Z][Z2]->name, table->assigned_files[incident->Z][Z2]->filename);
    gsto_load(table);

    if(argc==3) {
        while(fscanf(stdin, "%lg\n", &E)==1) {
            E *= C_KEV; 
            printf("%e %e\n", E/C_KEV, gsto_sto_v(table, incident->Z, Z2, velocity(E, incident->mass)));
        }
        return 0;
    }
    int i;
    for(i=3; i<argc; i++) {
        E=jibal_get_val(units, UNIT_TYPE_ENERGY, argv[i]);
        fprintf(stderr, "E=%g keV\n", E/C_KEV);
        fprintf(stderr, "Nuclear stopping: %g (eV/(10^15 at./cm2))\n", gsto_sto_nuclear_universal(E, incident->Z, incident->mass, Z2, 26.982*C_U)/C_EV_TFU);
        printf("%e %e\n", E/C_KEV, gsto_sto_v(table, incident->Z, Z2, velocity(E, incident->mass)));
    }
    jibal_units_free(units);
    elements_free(elements);
    isotopes_free(isotopes);
}
