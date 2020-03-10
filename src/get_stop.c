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

typedef struct {
    jibal_isotope *isotopes;
    jibal_element *elements;
    jibal_units *units;
} generic_data;

typedef struct {
    jibal_gsto  *workspace;
    jibal_isotope *incident;
    jibal_layer *target; /* TODO: array, now just fixed one pointer */
} experiment;

void print_stopping_range(experiment *exp, double E_low, double E_step, double E_high) {
    double E;
    for(E=E_low; E <= E_high; E += E_step) {
        double S_ele=jibal_stop_ele(exp->workspace, exp->incident, exp->target->material, E);
        double S_nuc=jibal_stop_nuc(exp->incident, exp->target->material, E);
        fprintf(stdout, "%e %e %e\n", E/C_KEV, S_ele/C_EV_TFU, S_nuc/C_EV_TFU);
    }
}
int main(int argc, char **argv) {
    experiment exp;
    generic_data data;
    data.isotopes=isotopes_load(NULL);
    abundances_load(data.isotopes, NULL);
    data.elements=elements_populate(data.isotopes);
    data.units=jibal_units_default();
    if(argc<=2) {
        return -1;

    }
    if(!data.isotopes) {
        fprintf(stderr, "Couldn't load isotopes.\n");
        return -1;
    }
    exp.incident=isotope_find(data.isotopes, argv[1], 0,0 );
    fprintf(stderr, "Z1=%i\nm1=%g kg (%g u)\n", exp.incident->Z, exp.incident->mass, exp.incident->mass/C_U);

    char *target_string=argv[2];

    exp.target=jibal_layer_new(jibal_material_create(data.elements, target_string), 0.0);
    if(!exp.target) {
        fprintf(stderr, "Error in creating layer \"%s\"", target_string);
        return -1;
    }
    jibal_material_print(stderr, exp.target->material);
    exp.workspace=gsto_init(91, NULL);
    if(!exp.workspace)
        return -1;
    if(!jibal_stop_auto_assign(exp.workspace, exp.incident, exp.target->material)) /* TODO: loop over layers */
        return -1;
    gsto_load(exp.workspace);


    int i;
    double E;
    if(argc >= 4) {
        jibal_get_val(data.units, UNIT_TYPE_ENERGY, argv[3]);
    }

    if(argc == 4) {
        print_stopping_range(&exp, E, E, E);
    } else if(argc == 5) {
        fprintf(stderr, "E = %g keV\n", E/C_KEV);
        exp.target->thickness=jibal_get_val(data.units, UNIT_TYPE_LAYER_THICKNESS, argv[4]);
        fprintf(stderr, "Layer thickness = %g tfu (1e15 at./cm2)\n", exp.target->thickness/C_TFU);
        double E_out= jibal_layer_energy_loss(exp.workspace, exp.incident, exp.target, E);
        fprintf(stderr, "E_out = %g keV\n", E_out/C_KEV);
        fprintf(stderr, "delta E = %g keV\n", (E_out-E)/C_KEV);
    } else if(argc == 6) {
        double E_low, E_step, E_high;
        E_low=E;
        E_step=jibal_get_val(data.units, UNIT_TYPE_ENERGY, argv[4]);
        E_high=jibal_get_val(data.units, UNIT_TYPE_ENERGY, argv[5]);
        fprintf(stderr, "E_low=%g keV, E_high=%g keV, E_step=%g keV\n", E_low/C_KEV, E_high/C_KEV, E_step/C_KEV);
        print_stopping_range(&exp, E_low, E_step, E_high);
    }
    jibal_material_free(exp.target->material);
    jibal_units_free(data.units);
    elements_free(data.elements);
    isotopes_free(data.isotopes);
    jibal_gsto_free(exp.workspace);
    return 0;
}
