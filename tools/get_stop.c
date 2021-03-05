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
#include <jibal_defaults.h>

#ifdef WIN32
#include <win_compat.h>
#endif


typedef struct {
    jibal *jibal;
    const jibal_isotope *incident;
    jibal_layer **target; /* Array of pointers, size n_layers, reallocated as necessary */
    int verbose;
    int n_layers;
} global;

void usage() {
    fprintf(stderr, "Usage: get_stop <incident ion> <energy> -l <material> -t <thickness> [-l <material> -t <thickness>] ...\n\n");
    fprintf(stderr, "Example: get_stop 4He 2MeV -l Si -t 1000tfu\n\n");
    fprintf(stderr, "If you give only one layer and omit the thickness, stopping data will outputted instead of layer stopping.\n");
    fprintf(stderr, "Usage: get_stop <incident ion> <energy> [<energy step> <energy high>]\n");
    fprintf(stderr, "Example: get_stop 4He 1MeV 100keV 3MeV -l Si\n");
}

void read_options(global *global, int *argc, char ***argv) {
    static struct option long_options[] = {
            {"help",        no_argument,        NULL, 'h'},
            {"version",     no_argument,        NULL, 'V'},
            {"verbose",     optional_argument,  NULL, 'v'},
            {"layer",       required_argument,  NULL, 'l'},
            {"thickness",   required_argument,  NULL, 't'},
            {"out",         required_argument,  NULL, 'o'},
            {"z",           required_argument,  NULL, 'z'},
            {NULL,                  0,  NULL, 0}
    };
    while (1) {
        int option_index = 0;
        char c = getopt_long(*argc, *argv, "hvVl:t:", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
                break;
            case 'V':
                printf("%s\n", JIBAL_VERSION);
                exit(EXIT_SUCCESS);
                break; /* Unnecessary */
            case 'v':
                if(optarg)
                    global->verbose = atoi(optarg);
                else
                    global->verbose++;
                break;
            case 'l':
                global->n_layers++;
                global->target = realloc(global->target, global->n_layers*sizeof(jibal_layer *));
                if(!global->target)
                    exit(EXIT_FAILURE);
                global->target[global->n_layers - 1]=jibal_layer_new(jibal_material_create(global->jibal->elements, optarg), 0.0);
                if(!global->target[global->n_layers - 1]) {
                    fprintf(stderr, "Could not create layer with material %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 't':
                if (global->n_layers) {
                    global->target[global->n_layers - 1]->thickness = jibal_get_val(global->jibal->units, UNIT_TYPE_LAYER_THICKNESS, optarg);
                } else {
                    fprintf(stderr, "Layer thickness %s given, but there is no layer.\n", optarg);
                }
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

void print_stopping_range(jibal *jibal, global *global, const jibal_material *material, double E_low, double E_step, double E_high) {
    double E;
    int i;
    int i_max=ceil((E_high-E_low)/E_step);
    if(i_max > 1000000) {
        i_max=1000000;
    }
    for(i=0; i <= i_max; i++) {
        E=E_low + i*E_step;
        double S_ele=jibal_stop_ele(jibal->gsto, global->incident, material, E);
        double S_nuc=jibal_stop_nuc(global->incident, material, E);
        double S_stragg=jibal_stragg(jibal->gsto, global->incident, material, E);
        fprintf(stdout, "%e %e %e %e\n", E/C_KEV, S_ele/C_EV_TFU, S_nuc/C_EV_TFU, sqrt(S_stragg*C_TFU)/C_EV);

    }
}
int main(int argc, char **argv) {
    global global = {NULL, NULL, NULL, 0, 0};
    double E;
    jibal *jibal=jibal_init(NULL);
    if(jibal->error) {
        fprintf(stderr, "Initializing JIBAL failed with error code: %i (%s)\n", jibal->error,
                jibal_error_string(jibal->error));
        return EXIT_FAILURE;
    }
    global.jibal = jibal;
    read_options(&global, &argc, &argv);
    if(argc < 2) {
        usage();
        return EXIT_FAILURE;
    }
    global.incident=jibal_isotope_find(jibal->isotopes, argv[0], 0,0 );
    if(!global.incident) {
        fprintf(stderr, "No such isotope: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    E=jibal_get_val(jibal->units, UNIT_TYPE_ENERGY, argv[1]);
    if(global.verbose > 0) {
        fprintf(stderr, "Z1 = %i\n", global.incident->Z);
        fprintf(stderr, "m1 = %g u\n", global.incident->mass / C_U);
    }

    if(!global.target) {
        fprintf(stderr, "No layers/material given. Try -l material, e.g. -l SiO2.\n");
        return -1;
    }

    int i;
    for(i=0; i < global.n_layers; i++) {
        jibal_layer *layer = global.target[i];
        if(global.verbose > 1) {
            fprintf(stderr, "Layer %i/%i. Thickness %g tfu\n", i+1, global.n_layers, layer->thickness/C_TFU);
            jibal_material_print(stderr, layer->material);
        }
        if (!jibal_gsto_auto_assign_material(jibal->gsto, global.incident, layer->material))
            return -1;
    }
    jibal_gsto_load_all(jibal->gsto);
    if(global.verbose > 0) {
        jibal_gsto_print_assignments(jibal->gsto);
    }
    if(global.verbose > 1) {
        jibal_gsto_print_files(jibal->gsto, TRUE);
    }
    if(global.n_layers == 1 && global.target[0]->thickness == 0.0) {
        jibal_material *material = global.target[0]->material;
        if(argc == 2) {
            print_stopping_range(jibal, &global, material, E, E, E);
        } else if(argc == 4) {
            double E_low, E_step, E_high;
            E_low=E;
            E_step=jibal_get_val(jibal->units, UNIT_TYPE_ENERGY, argv[2]);
            E_high=jibal_get_val(jibal->units, UNIT_TYPE_ENERGY, argv[3]);
            fprintf(stderr, "E_low = %g keV\n", E_low/C_KEV);
            fprintf(stderr, "E_step = %g keV\n", E_step/C_KEV);
            fprintf(stderr, "E_high = %g keV\n", E_high/C_KEV);
            print_stopping_range(jibal, &global, material, E_low, E_step, E_high);
        }
    } else {
        fprintf(stderr, "E = %g keV\n", E/C_KEV);
        double S=0.0;
        double E_0=E;
        for(i=0; i < global.n_layers; i++) {
            E = jibal_layer_energy_loss_with_straggling(jibal->gsto, global.incident, global.target[i], E, -1.0, &S);
        }
        fprintf(stdout, "E_out = %g keV\n", E/C_KEV);
        fprintf(stdout, "delta E = %g keV\n", (E-E_0)/C_KEV);
        fprintf(stdout, "Straggling = %g keV (FWHM)\n", C_FWHM*sqrt(S)/C_KEV);
    }


   // jibal_material_free(global.target->material);
    jibal_free(jibal);
    return EXIT_SUCCESS;
}
