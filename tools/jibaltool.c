#include "jibaltool.h"
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <jibal_config.h>
#include <inttypes.h>
#include <jibal.h>
#include <jibal_stop.h>
#include <defaults.h>

#ifdef WIN32
#include <jibal_registry.h>
#include <win_compat.h>
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
    if(jibal_gsto_print_files(global->jibal.gsto, FALSE) != 0) {
        fprintf(stderr, "Current configuration for files is in %s\n", global->jibal.config.files_file);
    }
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

char read_user_response(const char *question) {
    char *line=NULL;
    size_t line_size=0;
    //ssize_t linelen;
    char r=0;
    const char *template = "%s [yes/no]: ";
    const char *snarky_comebacks[]={"Please answer either yes or no.",
                                    "Yes or no. Or actually you can say exit too.",
                                    "Could you make up your mind. \"yes\", \"no\" or \"exit\". Easy.",
                                    "Let me spell you your options: Y E S or N O.",
                                    "There are two options. Yes or no. And exit. There are three options: yes, no and exit. Did you expect a Spanish Inquisition?",
                                    "NOBODY EXPECTS THE SPANISH INQUISITION (ha ha, same joke again)",
                                    "Oh come on! ENGLISH! YES OR NO. Well, don't shout or I won't listen to you.",
                                    "I've just about had it with you. Last warning.",
                                    "No, really.",
                                    "Fine! That's it.",
                                    NULL};
    int comeback=0;
    if(question) {
        fprintf(stderr, template, question);
    }
    while(getline(&line, &line_size, stdin) > 0) {
        line[strcspn(line, "\r\n")] = 0; /* Strip newlines */
        if(strcmp(line, "yes") == 0) {
            r = 'y';
        }
        if(strcmp(line, "maybe") == 0) {
            fprintf(stderr, "I take that as a yes.\n");
            r = 'y';
        }
        if(strcmp(line, "no") == 0) {
            r = 'n';
        }
        if(strcmp(line, "exit") == 0) {
            r = 'x';
        }
        if(strcmp(line, "quit") == 0) {
            r = 'x';
        }
        if(strcmp(line, "abort") == 0) {
            r = 'x';
        }
        if(r != 0)
            break;
        if(strlen(line) == 1) {
            fprintf(stderr, "Single letter response? What are you, an animal?\n");
        } else if(isupper(line[0])) {
            fprintf(stderr, "Please no upper case. I don't like capital letters.\n");
        } else {
            int i=((comeback%2==0)?0:comeback/2);
            fprintf(stderr, "%s\n", snarky_comebacks[i]);
            comeback++;
            if(!snarky_comebacks[i+1]) { /* Nothing clever left to say, exit */
                exit(EXIT_FAILURE);
            }
        }
        if(question) {
            fprintf(stderr, template, question);
        } else {
            fprintf(stderr, "[yes/no]: ");
        }

    }
    fprintf(stderr, "\n");
    free(line);
    switch (r) {
        case 'x':
            exit(EXIT_SUCCESS);
        case 0:
            exit(EXIT_FAILURE);
        default:
            return r;
    }
}

int bootstrap_write_user_config(jibal_config *config) {
    int retval=0;
    if(jibal_config_user_dir_mkdir_if_necessary() != 0) {
        char *user_dir=jibal_config_user_dir();
        fprintf(stderr, "Directory %s doesn't exist and can not be created. Try creating it manually. Now.\n", user_dir);
        free(user_dir);
        while(read_user_response("Are you ready to continue?") != 'y') {}; /* Infinite loop */
    }
    char *user_configfile=jibal_config_user_config_filename();
    FILE *config_out = fopen(user_configfile, "w");
    if(!config_out) {
        fprintf(stderr, "I can't write to that damn file.\n");
        retval = -1;
    } else {
        fprintf(stderr, "You are a brave soul. Writing.\n");
        jibal_config_file_write(config, config_out);
        fprintf(stderr, "Writing complete.\n");
        fclose(config_out);
    }
    return retval;
}

void bootstrap_make_blanks(const char *user_dir, const char *filename) { /* Silently creates empty files if they don't
 * exist, filename is relative to user_dir */
    char *path;
    asprintf(&path, "%s/%s", user_dir, filename);
    if(!path)
        return;
    path = jibal_path_cleanup(path);
    if(access(path, F_OK) == -1) { /* Doesn't exist */
        FILE *f=fopen(path, "w");
        if(f) {
            fclose(f);
        }
    }
    free(path);
}

int bootstrap_config(jibaltool_global *global, int argc, char **argv) {
    char r = 0;
    fprintf(stderr, "Welcome to Jibal user configuration bootstrap procedure, I will be your guide.\n\n");
    fprintf(stderr, "The rules are simple: I ask the questions and you answer.\n");
    fprintf(stderr, "The questions will be mostly yes/no questions. Let's start with a simple one.\n");
    r = read_user_response("Do you want to continue?");
    if(r != 'y') {
        fprintf(stderr, "Ok, maybe next time.\n");
        return EXIT_SUCCESS;
    }
    global->jibal.units=jibal_units_default();
    if(!global->jibal.units) {
        fprintf(stderr, "Bootstrapping failed because of issues with units. This is unheard of.\n");
        exit(EXIT_FAILURE);
    }
    jibal_config config = jibal_config_init(global->jibal.units, NULL, FALSE); /* Initialize config without any configuration files */
    if(config.error) {
        fprintf(stderr, "Can't initialize configuration.\n");
        exit(EXIT_FAILURE);
    }
    char *user_dir = jibal_config_user_dir();
    if(!user_dir) {
        fprintf(stderr, "I can't figure out which directory to put data on your platform.\n");
        exit(EXIT_FAILURE);
    }
    char *user_configfile = jibal_config_user_config_filename();
    if(!user_configfile) {
        fprintf(stderr, "I can't figure out where to put a configuration file on your platform.\n");
        exit(EXIT_FAILURE);
    }

    if(config.files_file)
        free(config.files_file);
    if(config.assignments_file)
        free(config.assignments_file);
    config.files_file=strdup(JIBAL_FILES_FILE); /* Just default names, not so important. We want relative paths to user_dir! */
    config.assignments_file=strdup(JIBAL_ASSIGNMENTS_FILE);

    fprintf(stderr, "A blank user configuration would look like this (with my best guesses):\n\n");
    fprintf(stderr, "======================================================================\n");
    jibal_config_file_write(&config, stderr);
    fprintf(stderr, "======================================================================\n");
    fprintf(stderr, "\nI could write it to %s, where Jibal could find it.\n", user_configfile);

    if(access(user_configfile, F_OK) == -1) { /* Doesn't exist, good */
        r = read_user_response("Do you want me to write this file?");
    } else { /* Exists */
        if(access(user_configfile, W_OK) == 0) {
            r = read_user_response("\nWARNING! VAROITUS! WARNUNG! VARNING!\nThe file exists!\nDo you want me to overwrite this file?");
        } else {
            fprintf(stderr, "\nThe file exists but is not writable. Can't do shit. :( :( Sorry.\n");
            r = 0;
        }
    }
    if(r == 'y') {
        if(bootstrap_write_user_config(&config) == 0) { /* Success */
            bootstrap_make_blanks(user_dir, config.files_file);
            bootstrap_make_blanks(user_dir, config.assignments_file);
        }
    } else if (r != 0) {
        fprintf(stderr, "Ok, I didn't do anything, I swear! :) :)\n");
    }
    free(user_dir);
    free(user_configfile);
    jibal_units_free(global->jibal.units);
    fprintf(stderr, "Bootstrap complete. That's it.\n");
    r = read_user_response("Do you want to do it again?");
    if(r == 'y') {
        fprintf(stderr, "YESSS! Oh boy it will be much more fun the next round!\n\n\n");
        return bootstrap_config(global, argc, argv);
    } else {
        fprintf(stderr, "I can see why, it's not really that much fun. Come back again later! I'll be here for you if you need me.\n");
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char **argv) {
    jibaltool_global global = {.Z=0, .outfilename=NULL, .stopfile=NULL, .format=NULL, .verbose=0};
    read_options(&global, &argc, &argv);
    static const struct command commands[] = {
            {"extract", &extract,
             "Extract values (e.g. He in Si or a range) in GSTO compatible format."},
            {"extract_stop_material", &extract_stop_material,
             "Extract stopping from a single stopping file for a given ion and material. (e.g. 4He in SiO2)"},
            {"files", &print_gstofiles, "Print available GSTO files."},
            {"isotopes", &print_isotopes, "Print a list of isotopes."},
            {"elements", &print_elements, "Print a list of elements."},
            {"config", &print_config, "Print current configuration (config file)."},
            {"bootstrap", &bootstrap_config,
             "Set up user configuration and download data files interactively."},
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
