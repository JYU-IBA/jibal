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
#include <jibal.h>
#include <stdlib.h>

typedef struct {
    jibal_isotope *incident;
    jibal_layer *target; /* TODO: array, now just fixed one pointer */
} experiment;

void print_stopping_range(jibal *jibal, experiment *exp, double E_low, double E_step, double E_high) {
    double E;
    for(E=E_low; E <= E_high; E += E_step) {
        double S_ele=jibal_stop_ele(jibal->gsto, exp->incident, exp->target->material, E);
        double S_nuc=jibal_stop_nuc(exp->incident, exp->target->material, E);
        fprintf(stdout, "%e %e %e\n", E/C_KEV, S_ele/C_EV_TFU, S_nuc/C_EV_TFU);
    }
}
int main(int argc, char **argv) {
    if(argc < 2)
        return -1;
    experiment exp;
    jibal jibal=jibal_init(NULL);
    exp.incident=jibal_isotope_find(jibal.isotopes, argv[1], 0,0 );
    if(!exp.incident) {
        fprintf(stderr, "No such isotope: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "Z1=%i\nm1=%g kg (%g u)\n", exp.incident->Z, exp.incident->mass, exp.incident->mass/C_U);

    char *target_string=argv[2];

    exp.target=jibal_layer_new(jibal_material_create(jibal.elements, target_string), 0.0);
    if(!exp.target) {
        fprintf(stderr, "Error in creating layer \"%s\"", target_string);
        return -1;
    }
    jibal_material_print(stderr, exp.target->material);
    if(!jibal.gsto)
        return -1;
    if(!jibal_gsto_auto_assign_material(jibal.gsto, exp.incident, exp.target->material)) /* TODO: loop over layers */
        return -1;
    jibal_gsto_load_all(jibal.gsto);


    int i;
    double E;
    if(argc >= 4) {
        E=jibal_get_val(jibal.units, UNIT_TYPE_ENERGY, argv[3]);
    }

    if(argc == 4) {
        print_stopping_range(&jibal, &exp, E, E, E);
    } else if(argc == 5) {
        fprintf(stderr, "E = %g keV\n", E/C_KEV);
        exp.target->thickness=jibal_get_val(jibal.units, UNIT_TYPE_LAYER_THICKNESS, argv[4]);
        fprintf(stderr, "Layer thickness = %g tfu (1e15 at./cm2)\n", exp.target->thickness/C_TFU);
        double E_out= jibal_layer_energy_loss(jibal.gsto, exp.incident, exp.target, E, -1.0);
        fprintf(stderr, "E_out = %g keV\n", E_out/C_KEV);
        fprintf(stderr, "delta E = %g keV\n", (E_out-E)/C_KEV);
    } else if(argc == 6) {
        double E_low, E_step, E_high;
        E_low=E;
        E_step=jibal_get_val(jibal.units, UNIT_TYPE_ENERGY, argv[4]);
        E_high=jibal_get_val(jibal.units, UNIT_TYPE_ENERGY, argv[5]);
        fprintf(stderr, "E_low=%g keV, E_high=%g keV, E_step=%g keV\n", E_low/C_KEV, E_high/C_KEV, E_step/C_KEV);
        print_stopping_range(&jibal, &exp, E_low, E_step, E_high);
    }
    jibal_material_free(exp.target->material);
    jibal_free(&jibal);
    return EXIT_SUCCESS;
}
