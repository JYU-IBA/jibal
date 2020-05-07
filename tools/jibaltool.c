#include "jibaltool.h"
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <jibal_config.h>
#include <inttypes.h>
#include <jibal.h>
#include <jibal_stop.h>
#ifdef WIN32
#include <jibal_registry.h>
#endif

void jibaltool_global_free(jibaltool_global *global) {
    if(global->outfilename) {
        free(global->outfilename);
    }
    if(global->stopfile) {
        free(global->stopfile);
    }
    if(global->format) {
        free(global->format);
    }
    jibal_free(&global->jibal);
    /* Doesn't free "options" itself. */
}

void jibaltool_usage() {
    fprintf(stderr, JIBAL_TOOL_HELP_STRING);
}

FILE *jibaltool_open_output(const jibaltool_global *global) {
    FILE *out;
    if(!global->outfilename) {
        return stdout;
    }
    out=fopen(global->outfilename, "w");
    if(!out) {
        exit(EXIT_FAILURE);
    }
    return out;
}

void jibaltool_close_output(FILE *out) {
    if(out != stdin) {
        fclose(out);
    }
}

void read_options(jibaltool_global *global, int *argc, char ***argv) {
    static struct option long_options[] = {
            {"help",        no_argument,        NULL, 'h'},
            {"version",     no_argument,        NULL, 'V'},
            {"verbose",     optional_argument,  NULL, 'v'},
            {"nop",         no_argument,        NULL, 'n'},
            {"out",         required_argument,  NULL, 'o'},
            {"stopfile",    required_argument,  NULL, 's'},
            {"format",      required_argument,  NULL, 'F'},
            {"config",      required_argument,  NULL, 'c'},
            {"z",           required_argument,  NULL, 'z'},
            {NULL,                  0, NULL, 0}
    };
    while (1) {
        int option_index = 0;
        char c = getopt_long(*argc, *argv, "c:hz:o:vVs:", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 'c':
                global->config_filename = strdup(optarg);
                break;
            case 'h':
                jibaltool_usage();
                exit(EXIT_SUCCESS);
                break;
            case 'z':
                global->Z = strtoimax(optarg, NULL, 10);
                break;
            case 'o':
                global->outfilename=strdup(optarg);
                break;
            case 'n':
                fprintf(stderr, "Nop.\n");
                break;
            case 's':
                global->stopfile=strdup(optarg);
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
            case 'F':
                global->format=strdup(optarg);
                break;
            default:
                jibaltool_usage();
                exit(EXIT_FAILURE);
                break;
        }
    }
    *argc -= optind;
    *argv += optind;
}

int extract_stop_material(jibaltool_global *global, int argc, char **argv) {
    if (argc < 2 || !global->stopfile) {
        fprintf(stderr, "Usage: jibaltool [--stopfile=<stopfile>] extract_stop_material incident target\n");
        return -1;
    }
    jibal *jibal = &global->jibal;
    jibal_isotope *incident = jibal_isotope_find(jibal->isotopes, argv[0], 0, 0); /* e.g. 4He */
    if(!incident) {
        fprintf(stderr, "%s is not a valid isotope.\n", argv[0]);
        return -1;
    }
    jibal_material *target = jibal_material_create(global->jibal.elements, argv[1]);
    if(!target) {
        fprintf(stderr, "%s is not a valid material formula.\n", argv[1]);
        return -1;
    }
    gsto_file_t *file=jibal_gsto_get_file(jibal->gsto, global->stopfile);
    if(!file) {
        fprintf(stderr, "No such stopping file: %s\n", global->stopfile);
        return -1;
    }
    if(file->type != GSTO_STO_ELE) {
        fprintf(stderr, "File %s is not an electronic stopping file!\n", file->name);
        return  -1;
    }
    if(!jibal_gsto_assign_material(jibal->gsto, incident, target, file)) {
        fprintf(stderr, "Couldn't assign stopping to file %s. Maybe some Z1,Z2 combination is not in the file?\n",
                file->name);
        return  -1;
    }
    jibal_gsto_load(jibal->gsto, FALSE, file);
    FILE *out=jibaltool_open_output(global);
    int i;
    fprintf(out, "#Stopping Units =  eV/(1E15 atoms/cm2)\n"
                 "#Energy(keV)  S(Elec)    S(Nuc)\n");
    for(i=0; i < file->xpoints; i++) {
        double E=file->em[i]*incident->mass;
        double S_ele=jibal_stop_ele(jibal->gsto, incident, target, E);
        double S_nuc=jibal_stop_nuc(incident, target, E);
        fprintf(out, "%.3e   %.3e   %e\n", E/C_KEV, S_ele/C_EV_TFU, S_nuc/C_EV_TFU);
    }
    jibaltool_close_output(out);
    return 0;
}

int extract(jibaltool_global *global, int argc, char **argv) {
    if (argc < 2 || !global->stopfile) {
        if(argc < 2) {
            fprintf(stderr, "ERROR: Too few arguments!\n");
        }
        if(!global->stopfile) {
            fprintf(stderr, "ERROR: No stopfile given!\n");
        }

        fprintf(stderr, "Usage: jibaltool --stopfile=<stopfile> [--format=<format>] extract incident target "
                        "[incident high] [target high]\n\n\tIncident and targets are elements (e.g. He or Si).\n\tYou "
                        "can give a range of incident elements too.\n\n\tExample: jibaltool --stopfile=srim2013 "
                        "extract He H He U\n");
        return -1;
    }
    jibal *jibal = &global->jibal;
    const jibal_element *incident=jibal_element_find(jibal->elements, argv[0]);; /* e.g. He */
    if(!incident)  {
            fprintf(stderr, "%s is not a valid element\n", argv[0]);
            return -1;
    }
    int Z1_low=incident->Z;
    int Z1_high=Z1_low;
    const jibal_element *target = jibal_element_find(jibal->elements, argv[1]);
    if(!target) {
        fprintf(stderr, "No such element: %s\n", argv[1]);
        return -1;
    }
    int Z2_low = target->Z;
    int Z2_high=Z2_low;
    if(argc >= 3) {
        incident=jibal_element_find(jibal->elements, argv[2]);
        if(incident) {
            Z1_high = incident->Z;
        }
        if(Z1_high < Z1_low) {
            fprintf(stderr, "Z1 higher bound is lower than lower (%i < %i)\n", Z1_high, Z1_low);
            return -1;
        }
    }
    if(argc >= 4) {
        target=jibal_element_find(jibal->elements, argv[3]);
        if(target) {
            Z2_high = target->Z;
        }
        if(Z2_high < Z2_low) {
            fprintf(stderr, "Z2 higher bound is lower than lower (%i < %i)\n", Z2_high, Z2_low);
            return -1;
        }
    }


    gsto_file_t *file = jibal_gsto_get_file(jibal->gsto, global->stopfile);
    if(!file) {
        fprintf(stderr, "No such stopping file: %s\n", global->stopfile);
        return -1;
    }
    if(!jibal_gsto_assign_range(jibal->gsto, Z1_low, Z1_high, Z2_low, Z2_high, file)) {
        fprintf(stderr, "Could not assign this range to this file.\n");
        return -1;
    }
    jibal_gsto_load(jibal->gsto, FALSE, file);
    FILE *out=jibaltool_open_output(global);
    gsto_data_format format=GSTO_DF_ASCII;
    if(global->format && strcmp(global->format, "bin")==0) {
        format=GSTO_DF_DOUBLE;
    }
    jibal_gsto_fprint_file(out, jibal->gsto, file, format, Z1_low, Z1_high, Z2_low, Z2_high);
    jibaltool_close_output(out);
    return 0;
}

int print_gstofiles(jibaltool_global *global, int argc, char **argv) {
    jibal_gsto_print_files(global->jibal.gsto, FALSE);
    return 0;
}

int print_isotopes(jibaltool_global *global, int argc, char **argv) {
    jibal_isotope *isotopes=global->jibal.isotopes;
    jibal_isotope *i;
    int Z=JIBAL_ANY_Z;
    double threshold=0.0;
    if(argc >= 1) {
        const jibal_element *e=jibal_element_find(global->jibal.elements, argv[0]);
        if(e)
            Z=e->Z;
    }
    if(argc >= 2) {
        if(strcmp(argv[1], "nat")==0) {
            threshold=ABUNDANCE_THRESHOLD;
        } else {
            char *le;
            threshold=strtod(argv[1], &le);
            if(*le == '%')
                threshold /= 100.0;
        }
    }
    FILE *out=jibaltool_open_output(global);
    for(i = isotopes; i->A != 0; i++) {
        if((Z == JIBAL_ANY_Z || Z == i->Z) && i->abundance >= threshold)
            fprintf(out, "%5s %3i %3i %3i %9.5lf %8.6lf\n", i->name, i->Z, i->N, i->A, i->mass/C_U, i->abundance);
    }
    jibaltool_close_output(out);
    return 0;
}

int print_elements(jibaltool_global *global, int argc, char **argv) {
    int Z;
    FILE *out=jibaltool_open_output(global);
    for(Z=0; Z <= JIBAL_ELEMENTS; Z++) {
        jibal_element *e=&global->jibal.elements[Z];
        fprintf(out, "%2s %3i %2i %2i %9.5lf\n", e->name, e->Z, jibal_element_number_of_isotopes(e, 0.0),
                jibal_element_number_of_isotopes(e, ABUNDANCE_THRESHOLD), e->avg_mass/C_U);
    }
    jibaltool_close_output(out);
    return 0;
}

int print_config(jibaltool_global *global, int argc, char **argv) {
    FILE *out=jibaltool_open_output(global);
    jibal_config_file_write(&global->jibal.config, out);
    jibaltool_close_output(out);
    return 0;
}


void print_commands(FILE *f, const struct command *commands) {
    const struct command *c;
    fprintf(f, "I recognize the following commands: \n");
    for(c=commands; c->f != NULL; c++) {
        fprintf(f, "%22s    %s\n", c->name, c->help_text);
    }
}

int bootstrap_config(jibaltool_global *global, int argc, char **argv) {
    char *user_dir=jibal_config_user_dir();
    if(!user_dir) {
        fprintf(stderr, "User configuration path can not be created. There is something odd in your platform.\n");
        exit(EXIT_FAILURE);
    }
    jibal_config config = jibal_config_defaults();
    global->jibal.units=jibal_units_default();
    global->jibal.config=jibal_config_init(global->jibal.units, NULL, FALSE); /* Initialize config without any configuration files */
    config.masses_file = strdup(global->jibal.config.masses_file);
    config.abundances_file = strdup(global->jibal.config.abundances_file);
    config.datadir = strdup(user_dir); /* TODO: try to guess and then ask verification from user */
    jibal_config_finalize(&config);
    fprintf(stdout, "User configuration will be created in %s\n", user_dir);
#ifdef WIN32
    char *install_root_from_registry = jibal_registry_string_get("RootDirectory");
    if(install_root_from_registry) {
        fprintf(stdout, "Root from registry: %s\n", install_root_from_registry);
        free(install_root_from_registry);
    }
#endif
    FILE *out=jibaltool_open_output(global); /* TODO: wrong place */
    jibal_config_file_write(&config, out);
    jibaltool_close_output(out);
    free(user_dir);
    return 0;
}

int main(int argc, char **argv) {
    jibaltool_global global = {.Z=0, .outfilename=NULL, .stopfile=NULL, .format=NULL, .verbose=0};
    read_options(&global, &argc, &argv);
    static const struct command commands[] = {
            {"extract", &extract, "Extract values (e.g. He in Si or a range) in GSTO"
                                            " compatible format."},
            {"extract_stop_material", &extract_stop_material, "Extract stopping from a single stopping"
                                                              " file for a given ion and material. (e.g. 4He in SiO2)"},
            {"files", &print_gstofiles, "Print available GSTO files."},
            {"isotopes", &print_isotopes, "Print a list of isotopes."},
            {"elements", &print_elements, "Print a list of elements."},
            {"config", &print_config, "Print current configuration (config file)."},
            {"bootstrap", &bootstrap_config, "Set up user configuration and download data files interactively."},
            {NULL, NULL, NULL}
    };
    if(argc < 1) {
        jibaltool_usage();
        fprintf(stderr, "\n");
        print_commands(stderr, commands);
        return EXIT_FAILURE;
    }

    const struct command *c;
    int found=0;
    for(c=commands; c->f != NULL; c++) {
        if(strcmp(c->name, argv[0])==0) {
            found=1;
            if(c->f != &bootstrap_config) {
                global.jibal = jibal_init(global.config_filename);
                if(global.jibal.error) {
                    fprintf(stderr, "Initializing JIBAL failed with error code: %i (%s)\n", global.jibal.error,
                    jibal_error_string(global.jibal.error));
                    return EXIT_FAILURE;
                }
            }
            c->f(&global, argc-1, argv+1);
            break;
        }
    }
    if(!found) {
        fprintf(stderr, "No such command: %s\n\n", argv[0]);
        print_commands(stderr, commands);
    }
    jibaltool_global_free(&global);
    return EXIT_SUCCESS;
}
