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
            {"version",     no_argument,        NULL, 'v'},
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
            case 'v':
                printf("%s\n", jibal_VERSION);
                exit(EXIT_SUCCESS);
                break; /* Unnecessary */
            case 'F':
                global->format=strdup(optarg);
                break;
            default:
                jibaltool_usage();
                exit(EXIT_FAILURE);
                break;
        }
    }
    int i;
    *argc -= optind;
    *argv += optind;
}

int extract_stop(jibaltool_global *global, int argc, char **argv) {
    int i;

    if (argc < 2) {
        fprintf(stderr, "Usage: jibaltool extract incident target\n");
        return -1;
    }
    jibal *jibal = &global->jibal;
    int Z1;
    jibal_element *incident_elem=NULL; /* e.g. He */
    jibal_isotope *incident=NULL;
    incident = jibal_isotope_find(jibal->isotopes, argv[0], 0, 0); /* e.g. 4He */
    if(!incident) { /* Ok, no isotope found, maybe we were given just the element? */
        incident_elem=jibal_element_find(jibal->elements, argv[0]);
        if(!incident_elem) {
            fprintf(stderr, "%s is not a valid isotope or an element\n", argv[0]);
            return -1;
        }
        Z1=incident_elem->Z;
    } else {
        Z1=incident->Z;
    }
    jibal_element *target = jibal_element_find(jibal->elements, argv[1]);
    if(!target) {
        fprintf(stderr, "No such element: %s\n", argv[1]);
        return -1;
    }
    target = jibal_element_copy(target, 0); /* TODO: free */
    int Z2 = target->Z;
    gsto_file_t *file;
    if(global->stopfile) {
        file = jibal_gsto_get_file(jibal->gsto, global->stopfile);
        if(!file) {
            fprintf(stderr, "No such stopping file: %s\n", global->stopfile);
            return -1;
        }
        jibal_gsto_assign(jibal->gsto, Z1, Z2, file);
    } else {
        jibal_gsto_auto_assign(jibal->gsto, Z1, Z2);
        file = jibal_gsto_get_assigned_file(jibal->gsto, Z1, Z2);
        if(!file) {
            fprintf(stderr, "No stopping file assigned for Z1=%i, Z2=%i.\n", Z1, Z2);
            return -1;
        }
    }
    jibal_gsto_load(jibal->gsto, file);

    FILE *out=jibaltool_open_output(global);
    if(global->format && strcmp(global->format, "srim")==0) {
        if(!incident) {
            fprintf(stderr, "For SRIM style output, please give an isotope\n");
            return -1;
        }
        fprintf(out, "Stopping Units =  eV/(1E15 atoms/cm2)\n"
                        "Energy(keV)  S(Elec)    S(Nuc)\n");
        const double *data=jibal_gsto_file_get_data(file, Z1, Z2);
        for(i=0; i < file->xpoints; i++) {
            double E=energy(file->vel[i], incident->mass);
            fprintf(out, "%e %e %e\n", E/C_KEV, data[i]/C_EV_TFU,
                    jibal_gsto_stop_nuclear_universal(E, Z1, incident->mass, Z2, target->avg_mass)/C_EV_TFU);
        }
    } else {
        jibal_gsto_fprint_file(out, file, Z1, Z1, Z2, Z2);
    }
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

void print_commands(FILE *f, const struct command *commands) {
    const struct command *c;
    fprintf(f, "I recognize the following commands: \n");
    for(c=commands; c->f != NULL; c++) {
        fprintf(f, "%20s    %s\n", c->name, c->help_text);
    }
}

int main(int argc, char **argv) {
    jibaltool_global global = {.Z=0, .outfilename=NULL, .stopfile=NULL, .format=NULL};
    read_options(&global, &argc, &argv);
    static const struct command commands[] = {
            {"extract_stop", &extract_stop, "Extract single stopping (e.g. He in Si) in GSTO compatible ASCII format."},
            {"print_stopfiles", &print_stopfiles, "Print available stopping files."},
            {"print_isotopes", &print_isotopes, "Print a list of isotopes."},
            {"print_elements", &print_elements, "Print a list of elements."},
            {NULL, NULL, NULL}
    };
    if(argc < 1) {
        jibaltool_usage();
        fprintf(stderr, "\n");
        print_commands(stderr, commands);
        exit(EXIT_FAILURE);
    }
    global.jibal = jibal_init(global.config_filename);
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
