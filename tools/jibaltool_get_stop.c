/*
    Jyväskylä Ion Beam Analysis Library (JIBAL)
    Copyright (C) 2020-2022 Jaakko Julin <jaakko.julin@jyu.fi>

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
#include "jibaltool_get_stop.h"

void jibaltool_stop_usage() {
    fprintf(stderr, "Usage: jibaltool stop <incident ion> <energy> -l <material> -t <thickness> [-l <material> -t <thickness>] ...\n\n");
    fprintf(stderr, "Example: jibaltool stop 4He 2MeV -l Si -t 1000tfu\n\n");
    fprintf(stderr, "If you give only one layer and omit the thickness, stopping data will outputted instead of layer stopping.\n");
    fprintf(stderr, "Usage: jibaltool stop <incident ion> <energy> [<energy step> <energy high>]\n");
    fprintf(stderr, "Example: jibaltool stop 4He 1MeV 100keV 3MeV -l Si\n");
}

int jibaltool_get_stop_read_options(get_stop_global *global, int *argc, char ***argv) {
    static struct option long_options[] = {
            {"layer",       required_argument,  NULL, 'l'},
            {"thickness",   required_argument,  NULL, 't'},
            {NULL,                  0,  NULL, 0}
    };
    while (1) {
        int option_index = 0;
        char c = getopt_long(*argc, *argv, "l:t:", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 'l':
                global->n_layers++;
                global->target = realloc(global->target, global->n_layers*sizeof(jibal_layer *));
                if(!global->target)
                    return EXIT_FAILURE;
                global->target[global->n_layers - 1]=jibal_layer_new(jibal_material_create(global->jibal->elements, optarg), 0.0);
                if(!global->target[global->n_layers - 1]) {
                    fprintf(stderr, "Could not create layer with material %s\n", optarg);
                    return EXIT_FAILURE;
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
                continue;
        }
    }
    *argc -= optind;
    *argv += optind;
    return EXIT_SUCCESS;
}

void print_stopping_range(jibal *jibal, get_stop_global *global, const jibal_material *material, double E_low, double E_step, double E_high) {
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
int print_stop(jibaltool_global *global, int argc, char **argv) {
    get_stop_global g = {NULL, NULL, NULL, 0, 0};
    double E;
    jibal *jibal=jibal_init(NULL);
    if(jibal->error) {
        fprintf(stderr, "Initializing JIBAL failed with error code: %i (%s)\n", jibal->error,
                jibal_error_string(jibal->error));
        return EXIT_FAILURE;
    }
    g.jibal = global->jibal;
#ifdef DEBUG
    fprintf(stderr, "Argument vector on entry to print_stop():\n");
    for(int i = 0; i < argc; i++) {
        fprintf(stderr, "argv[%i] = %s\n", i, argv[i]);
    }
#endif
    argc++;
    argv--;
    if(jibaltool_get_stop_read_options(&g, &argc, &argv)) {
        return EXIT_FAILURE;
    }
    if(argc < 2) {
        fprintf(stderr, "Not enough arguments!\n", argc);
        jibaltool_stop_usage();
        return EXIT_FAILURE;
    }
    g.incident=jibal_isotope_find(jibal->isotopes, argv[0], 0,0 );
    if(!g.incident) {
        fprintf(stderr, "No such isotope: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    E=jibal_get_val(jibal->units, UNIT_TYPE_ENERGY, argv[1]);
    if(g.verbose > 0) {
        fprintf(stderr, "Z1 = %i\n", g.incident->Z);
        fprintf(stderr, "m1 = %g u\n", g.incident->mass / C_U);
    }

    if(!g.target) {
        fprintf(stderr, "No layers/material given. Try -l material, e.g. -l SiO2.\n");
        return -1;
    }

    int i;
    for(i=0; i < g.n_layers; i++) {
        jibal_layer *layer = g.target[i];
        if(g.verbose > 1) {
            fprintf(stderr, "Layer %i/%i. Thickness %g tfu\n", i+1, g.n_layers, layer->thickness/C_TFU);
            jibal_material_print(stderr, layer->material);
        }
        if (!jibal_gsto_auto_assign_material(jibal->gsto, g.incident, layer->material))
            return -1;
    }
    jibal_gsto_load_all(jibal->gsto);
    if(g.verbose > 0) {
        jibal_gsto_print_assignments(jibal->gsto);
    }
    if(g.verbose > 1) {
        jibal_gsto_print_files(jibal->gsto, TRUE);
    }
    if(g.n_layers == 1 && g.target[0]->thickness == 0.0) {
        jibal_material *material = g.target[0]->material;
        if(argc == 2) {
            print_stopping_range(jibal, &g, material, E, E, E);
        } else if(argc == 4) {
            double E_low, E_step, E_high;
            E_low=E;
            E_step=jibal_get_val(jibal->units, UNIT_TYPE_ENERGY, argv[2]);
            E_high=jibal_get_val(jibal->units, UNIT_TYPE_ENERGY, argv[3]);
            fprintf(stderr, "E_low = %g keV\n", E_low/C_KEV);
            fprintf(stderr, "E_step = %g keV\n", E_step/C_KEV);
            fprintf(stderr, "E_high = %g keV\n", E_high/C_KEV);
            print_stopping_range(jibal, &g, material, E_low, E_step, E_high);
        }
    } else {
        fprintf(stderr, "E = %g keV\n", E/C_KEV);
        double S=0.0;
        double E_0=E;
        for(i=0; i < g.n_layers; i++) {
            E = jibal_layer_energy_loss_with_straggling(jibal->gsto, g.incident, g.target[i], E, -1.0, &S);
        }
        fprintf(stdout, "E_out = %g keV\n", E/C_KEV);
        fprintf(stdout, "delta E = %g keV\n", (E-E_0)/C_KEV);
        fprintf(stdout, "Straggling = %g keV (FWHM)\n", C_FWHM*sqrt(S)/C_KEV);
    }


   // jibal_material_free(get_stop_global.target->material);
    jibal_free(jibal);
    return EXIT_SUCCESS;
}
