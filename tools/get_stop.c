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
#include <stdlib.h>
#include <getopt.h>
#include <jibal.h>
#include <jibal_stop.h>
#include <jibal_stragg.h>
#include <defaults.h>

#ifdef WIN32
#include <win_compat.h>
#endif


typedef struct {
    jibal_isotope *incident;
    jibal_layer *target; /* TODO: array, now just fixed one pointer */
    int verbose;
} global;

void usage() {
    fprintf(stderr, "");
}

void read_options(global *global, int *argc, char ***argv) {
    static struct option long_options[] = {
            {"help",        no_argument,        NULL, 'h'},
            {"version",     no_argument,        NULL, 'V'},
            {"verbose",     optional_argument,  NULL, 'v'},
            {"nop",         no_argument,        NULL, 'n'},
            {"out",         required_argument,  NULL, 'o'},
            {"stopfile",    required_argument,  NULL, 's'},
            {"z",           required_argument,  NULL, 'z'},
            {NULL,                  0, NULL, 0}
    };
    while (1) {
        int option_index = 0;
        char c = getopt_long(*argc, *argv, "hvV", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
                break;
            case 'n':
                fprintf(stderr, "Nop.\n");
                break;
            case 'V':
                printf("%s\n", jibal_VERSION);
                exit(EXIT_SUCCESS);
                break; /* Unnecessary */
            case 'v':
                if(optarg)
                    global->verbose = atoi(optarg);
                else
                    global->verbose++;
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
                break;
        }
    }
    *argc -= optind;
    *argv += optind;
}

void print_stopping_range(jibal *jibal, global *global, double E_low, double E_step, double E_high) {
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
        double S_ele=jibal_stop_ele(jibal->gsto, global->incident, global->target->material, E);
        double S_nuc=jibal_stop_nuc(global->incident, global->target->material, E);
        double S_stragg=jibal_stragg(jibal->gsto, global->incident, global->target->material, E);
        fprintf(stdout, "%e %e %e %e\n", E/C_KEV, S_ele/C_EV_TFU, S_nuc/C_EV_TFU, sqrt(S_stragg*C_TFU)/C_EV);

    }
}
int main(int argc, char **argv) {
    if(argc < 4) {
        fprintf(stderr, "Usage: get_stop <incident ion> <target material> <energy>\n");
        return EXIT_FAILURE;
    }
    global global = {NULL, NULL, 0};
    read_options(&global, &argc, &argv);
    jibal jibal=jibal_init(NULL);
    if(jibal.error) {
        fprintf(stderr, "Initializing JIBAL failed with error code: %i (%s)\n", jibal.error,
                jibal_error_string(jibal.error));
        return EXIT_FAILURE;
    }
    global.incident=jibal_isotope_find(jibal.isotopes, argv[0], 0,0 );
    if(!global.incident) {
        fprintf(stderr, "No such isotope: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "Z1 = %i\n", global.incident->Z);
    fprintf(stderr, "m1 = %g u\n", global.incident->mass/C_U);

    global.target=jibal_layer_new(jibal_material_create(jibal.elements, argv[1]), 0.0);
    if(!global.target) {
        fprintf(stderr, "Error in creating layer \"%s\"\n", argv[1]);
        return -1;
    }
    if(global.verbose) {
        jibal_material_print(stderr, global.target->material);
    }

    if(!jibal_gsto_auto_assign_material(jibal.gsto, global.incident, global.target->material)) /* TODO: loop over layers */
        return -1;
    jibal_gsto_load_all(jibal.gsto);
    jibal_gsto_print_assignments(jibal.gsto);
    if(global.verbose) {
        jibal_gsto_print_files(jibal.gsto, TRUE);
    }
    double E;
    if(argc >= 3) {
        E=jibal_get_val(jibal.units, UNIT_TYPE_ENERGY, argv[2]);
    }
    if(argc == 3) {
        fprintf(stderr, "E = %g keV\n", E/C_KEV);
        print_stopping_range(&jibal, &global, E, E, E);
    } else if(argc == 4) {
        fprintf(stdout, "E = %g keV\n", E/C_KEV);
        global.target->thickness=jibal_get_val(jibal.units, UNIT_TYPE_LAYER_THICKNESS, argv[3]);
        fprintf(stderr, "thickness = %g tfu (1e15 at./cm2)\n", global.target->thickness/C_TFU);
        double S=0.0;
        double E_out= jibal_layer_energy_loss_with_straggling(jibal.gsto, global.incident, global.target, E, -1.0, &S);
        fprintf(stdout, "E_out = %g keV\n", E_out/C_KEV);
        fprintf(stdout, "delta E = %g keV\n", (E_out-E)/C_KEV);
        fprintf(stdout, "Straggling = %g keV (FWHM)\n", C_FWHM*sqrt(S)/C_KEV);
    } else if(argc == 5) {
        double E_low, E_step, E_high;
        E_low=E;
        E_step=jibal_get_val(jibal.units, UNIT_TYPE_ENERGY, argv[4]);
        E_high=jibal_get_val(jibal.units, UNIT_TYPE_ENERGY, argv[5]);
        fprintf(stderr, "E_low=%g keV, E_high=%g keV, E_step=%g keV\n", E_low/C_KEV, E_high/C_KEV, E_step/C_KEV);
        print_stopping_range(&jibal, &global, E_low, E_step, E_high);
    }
    jibal_material_free(global.target->material);
    jibal_free(&jibal);
    return EXIT_SUCCESS;
}
