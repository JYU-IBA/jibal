#include "jibaltool.h"
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <jibal_config.h>
#include <inttypes.h>
#include <jibal.h>


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
        char c = getopt_long(*argc, *argv, "hvz:s:", long_options, &option_index);
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
    if(!jibal_gsto_assign_material(jibal->gsto, incident, target, file)) {
        fprintf(stderr, "Couldn't assign stopping to file %s. Maybe some Z1,Z2 combination is not in the file?\n",
                file->name);
        return  -1;
    }
    jibal_gsto_load(jibal->gsto, file);
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

int extract_stop(jibaltool_global *global, int argc, char **argv) {
    if (argc < 2 || !global->stopfile) {
        if(argc < 2) {
            fprintf(stderr, "ERROR: Too few arguments!\n");
        }
        if(!global->stopfile) {
            fprintf(stderr, "ERROR: No stopfile given!\n");
        }

        fprintf(stderr, "Usage: jibaltool --stopfile=<stopfile> [--format=<format>] extract_stop incident target "
                        "[incident high] [target high]\n\n\tIncident and targets are elements (e.g. He or Si).\n\tYou "
                        "can give a range of incident elements too.\n\n\tExample: jibaltool --stopfile=srim2013 "
                        "extract_stop He H He U\n");
        return -1;
    }
    jibal *jibal = &global->jibal;
    jibal_element *incident=jibal_element_find(jibal->elements, argv[0]);; /* e.g. He */
    if(!incident)  {
            fprintf(stderr, "%s is not a valid element\n", argv[0]);
            return -1;
    }
    int Z1_low=incident->Z;
    int Z1_high=Z1_low;
    jibal_element *target = jibal_element_find(jibal->elements, argv[1]);
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
    jibal_gsto_load(jibal->gsto, file);
    FILE *out=jibaltool_open_output(global);
    gsto_data_format format=GSTO_DF_ASCII;
    if(global->format && strcmp(global->format, "bin")==0) {
        format=GSTO_DF_DOUBLE;
    }
    jibal_gsto_fprint_file(out, file, format, Z1_low, Z1_high, Z2_low, Z2_high);
    jibaltool_close_output(out);
    return 0;
}

int print_stopfiles(jibaltool_global *global, int argc, char **argv) {
    jibal_gsto_print_files(global->jibal.gsto);
    return 0;
}

int print_isotopes(jibaltool_global *global, int argc, char **argv) {
    jibal_isotope *isotopes=global->jibal.isotopes;
    jibal_isotope *i;
    FILE *out=jibaltool_open_output(global);
    for(i = isotopes; i->A != 0; i++) {
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

int main(int argc, char **argv) {
    jibaltool_global global = {.Z=0, .outfilename=NULL, .stopfile=NULL, .format=NULL, .verbose=0};
    read_options(&global, &argc, &argv);
    static const struct command commands[] = {
            {"extract_stop", &extract_stop, "Extract stopping (e.g. He in Si or a range) in GSTO"
                                            " compatible format."},
            {"extract_stop_material", &extract_stop_material, "Extract stopping from a single stopping"
                                                              " file for a given ion and material. (e.g. 4He in SiO2)"},
            {"stopfiles", &print_stopfiles, "Print available stopping files."},
            {"isotopes", &print_isotopes, "Print a list of isotopes."},
            {"elements", &print_elements, "Print a list of elements."},
            {"config", &print_config, "Print current configuration (config file)."},
            {NULL, NULL, NULL}
    };
    if(argc < 1) {
        jibaltool_usage();
        fprintf(stderr, "\n");
        print_commands(stderr, commands);
        return EXIT_FAILURE;
    }
    global.jibal = jibal_init(global.config_filename);
    if(global.jibal.error) {
        fprintf(stderr, "Initializing JIBAL failed with error code: %i (%s)\n", global.jibal.error,
                jibal_error_string(global.jibal.error));
        return EXIT_FAILURE;
    }
    const struct command *c;
    int found=0;
    for(c=commands; c->f != NULL; c++) {
        if(strcmp(c->name, argv[0])==0) {
            found=1;
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
