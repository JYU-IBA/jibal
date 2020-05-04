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
#include <jibal_stop.h>
#include <stdlib.h>
#include <jibal_stragg.h>

typedef struct {
    jibal_isotope *incident;
    jibal_layer *target; /* TODO: array, now just fixed one pointer */
} experiment;

void print_stopping_range(jibal *jibal, experiment *exp, double E_low, double E_step, double E_high) {
    double E;
    int i;
    int i_max=floor((E_high-E_low)/E_step);
    if(i_max > 1000000) {
        i_max=1000000;
    }
    if(i_max < 1) {
        i_max = 1;
    }
    for(i=0; i < i_max; i++) {
        E=E_low + i*E_step;
        double S_ele=jibal_stop_ele(jibal->gsto, exp->incident, exp->target->material, E);
        double S_nuc=jibal_stop_nuc(exp->incident, exp->target->material, E);
        double S_stragg=jibal_stragg(jibal->gsto, exp->incident, exp->target->material, E);
        fprintf(stdout, "%e %e %e %e\n", E/C_KEV, S_ele/C_EV_TFU, S_nuc/C_EV_TFU, sqrt(S_stragg*C_TFU)/C_EV);

    }
}
int main(int argc, char **argv) {
    if(argc < 4) {
        fprintf(stderr, "Usage: get_stop <incident ion> <target material> <energy>\n");
        return EXIT_FAILURE;
    }
    experiment exp;
    jibal jibal=jibal_init(NULL);
    if(jibal.error) {
        fprintf(stderr, "Initializing JIBAL failed with error code: %i (%s)\n", jibal.error,
                jibal_error_string(jibal.error));
        return EXIT_FAILURE;
    }
    exp.incident=jibal_isotope_find(jibal.isotopes, argv[1], 0,0 );
    if(!exp.incident) {
        fprintf(stderr, "No such isotope: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "Z1 = %i\nm1=%g kg (%g u)\n", exp.incident->Z, exp.incident->mass, exp.incident->mass/C_U);

    exp.target=jibal_layer_new(jibal_material_create(jibal.elements, argv[2]), 0.0);
    if(!exp.target) {
        fprintf(stderr, "Error in creating layer \"%s\"\n", argv[2]);
        return -1;
    }
    jibal_material_print(stderr, exp.target->material);

    if(!jibal_gsto_auto_assign_material(jibal.gsto, exp.incident, exp.target->material)) /* TODO: loop over layers */
        return -1;
    jibal_gsto_load_all(jibal.gsto);
    jibal_gsto_print_assignments(jibal.gsto);
    jibal_gsto_print_files(jibal.gsto, TRUE);
    double E;
    if(argc >= 4) {
        E=jibal_get_val(jibal.units, UNIT_TYPE_ENERGY, argv[3]);
    }
    if(argc == 4) {
        print_stopping_range(&jibal, &exp, E, E, E);
    } else if(argc == 5) {
        fprintf(stdout, "E = %g keV\n", E/C_KEV);
        exp.target->thickness=jibal_get_val(jibal.units, UNIT_TYPE_LAYER_THICKNESS, argv[4]);
        fprintf(stdout, "thickness = %g tfu (1e15 at./cm2)\n", exp.target->thickness/C_TFU);
        double S=0.0;
        double E_out= jibal_layer_energy_loss_with_straggling(jibal.gsto, exp.incident, exp.target, E, -1.0, &S);
        fprintf(stdout, "E_out = %g keV\n", E_out/C_KEV);
        fprintf(stdout, "delta E = %g keV\n", (E_out-E)/C_KEV);
        fprintf(stdout, "Straggling = %g keV (FWHM)\n", C_FWHM*sqrt(S)/C_KEV);
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