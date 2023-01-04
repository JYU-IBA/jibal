#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <jibal_config.h>
#include <inttypes.h>
#include <jibal.h>
#include <jibal_stop.h>
#include <jibal_defaults.h>
#include <jibal_cs.h>
#include <jibal_kin.h>

#ifdef WIN32
#include <jibal_registry.h>
#include <win_compat.h>
#endif
#include "jibaltool.h"
#include "jibaltool_get_stop.h"

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
    jibal_free(global->jibal);
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
        char c = getopt_long(*argc, *argv, "+c:hz:o:vVs:", long_options, &option_index);
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
                printf("jibaltool %s JIBAL library version %s\n", JIBAL_VERSION, jibal_version());
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
        for(int i = 0; i < argc; i++) {
            fprintf(stderr, "argv[%i] = %s\n", i, argv[i]);
        }
        fprintf(stderr, "Usage: jibaltool [--stopfile=<stopfile>] extract_stop_material incident target\n");
        return -1;
    }
    jibal *jibal = global->jibal;
    const jibal_isotope *incident = jibal_isotope_find(jibal->isotopes, argv[0], 0, 0); /* e.g. 4He */
    if(!incident) {
        fprintf(stderr, "%s is not a valid isotope.\n", argv[0]);
        return -1;
    }
    jibal_material *target = jibal_material_create(global->jibal->elements, argv[1]);
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
    size_t i;
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
    jibal *jibal = global->jibal;
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
    if(jibal_gsto_print_files(global->jibal->gsto, FALSE) != 0) {
        fprintf(stderr, "Current configuration for files is in %s\n", global->jibal->config->files_file);
    }
    return 0;
}

int print_isotopes(jibaltool_global *global, int argc, char **argv) {
    jibal_isotope *isotopes=global->jibal->isotopes;
    const jibal_isotope *i;
    int Z=JIBAL_ANY_Z;
    double threshold=0.0;
    if(argc >= 1) {
        const jibal_element *e=jibal_element_find(global->jibal->elements, argv[0]);
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
    int Z_max = jibal_elements_Zmax(global->jibal->elements);
    for(Z=0; Z <= Z_max; Z++) {
        jibal_element *e=&global->jibal->elements[Z];
        fprintf(out, "%2s %3i %2lu %2lu %9.5lf\n", e->name, e->Z, jibal_element_number_of_isotopes(e, 0.0),
                jibal_element_number_of_isotopes(e, ABUNDANCE_THRESHOLD), e->avg_mass/C_U);
    }
    jibaltool_close_output(out);
    return 0;
}

int print_config(jibaltool_global *global, int argc, char **argv) {
    jibal_config_write_to_file(global->jibal->units, global->jibal->config, global->outfilename);
    return 0;
}


void print_commands(FILE *f, const struct command *commands) {
    const struct command *c;
    fprintf(f, "I recognize the following commands: \n");
    for(c=commands; c->f != NULL; c++) {
        fprintf(f, "%22s    %s\n", c->name, c->help_text);
    }
}

int print_status(jibaltool_global *global, int argc, char **argv) {
    FILE *out=jibaltool_open_output(global);
    jibal_status_print(out, global->jibal);
    jibaltool_close_output(out);
    return 0;
}

int print_units(jibaltool_global *global, int argc, char **argv) {
    FILE *out = jibaltool_open_output(global);
    if(argc == 0) {
        jibal_units_print(out, global->jibal->units);
        return 0;
    } else {
        double f_in, f_out;
        f_in = jibal_get_val(global->jibal->units, 0, argv[0]);
        if(argc >=2) {
            f_out = jibal_get_val(global->jibal->units, 0, argv[1]);
        } else {
            f_out = 1.0;
        }
        fprintf(out, "%.10lg\n", f_in/f_out);
    }
    jibaltool_close_output(out);
    return 0;
}

int print_cs(jibaltool_global *global, int argc, char **argv) {
    jibal *jibal = global->jibal;
    if(argc < 4) {
        fprintf(stderr, "Usage: jibaltool cs <incident ion> <target> <energy> <angle>\n\nTarget can be an isotope, element or compound.\nExample: jibaltool cs 4He Si 170deg 2MeV\n");
        return EXIT_FAILURE;
    }
    const jibal_isotope *incident = jibal_isotope_find(jibal->isotopes, argv[0], 0, 0);
    if(!incident) {
        fprintf(stderr, "There is no isotope %s in my database.\n", argv[0]);
        return EXIT_FAILURE;
    }
    jibal_material *target_material = jibal_material_create(jibal->elements, argv[1]);
    if(!target_material) {
        fprintf(stderr, "Error in creating material from expression %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    double theta = jibal_get_val(jibal->units, UNIT_TYPE_ANGLE, argv[2]);
    double E = jibal_get_val(jibal->units, UNIT_TYPE_ENERGY, argv[3]);

    double cs_rbs = 0.0;
    double cs_erd = 0.0;
    int erd = theta < (90.0*C_DEG);
    for(size_t i_elem = 0; i_elem < target_material->n_elements; i_elem++) {
        jibal_element *e = &target_material->elements[i_elem];
        for(size_t i_isotope = 0; i_isotope < e->n_isotopes; i_isotope++) {
            const jibal_isotope *isotope = e->isotopes[i_elem];
            double theta_max=asin(isotope->mass/incident->mass);

            double c = e->concs[i_isotope];
            if(!(incident->mass > isotope->mass && theta > theta_max)) { /* Scattering possible */
                double sigma_rbs = jibal_cs_rbs(jibal->config, incident, isotope, theta, E);
                cs_rbs += c * sigma_rbs;
            }
            if(erd) {
                double sigma_erd = jibal_cs_erd(jibal->config, incident, isotope, theta, E);
                cs_erd += c * sigma_erd;
            }
#ifdef DEBUG
            fprintf(stderr, "Isotope %zu conc %lf\n", i_isotope, e->concs[i_isotope]);
#endif
        }
    }
    fprintf(stderr, "RBS cross section is %g mb/sr (%s)\n", cs_rbs/C_MB_SR, jibal_cs_rbs_name(jibal->config));
    if(erd) {
        fprintf(stderr, "ERD cross section is %g mb/sr (%s)\n", cs_erd / C_MB_SR, jibal_cs_erd_name(jibal->config));
    }
    jibal_material_free(target_material);
    return EXIT_SUCCESS;
}


void print_kin_rbs(jibal *jibal, const jibal_isotope *incident, const jibal_isotope *target, double theta, double E) {
    double r = incident->mass/target->mass;
    double theta_max=asin(target->mass/incident->mass);
    double theta_cm = theta + asin(r * sin(theta)); /* RBS */
    double E_rbs = jibal_kin_rbs(incident->mass, target->mass, theta, '+') * E;
    double cs_rbs = jibal_cs_rbs(jibal->config, incident, target, theta, E);
    fprintf(stderr, "RBS (%s scattered by %s to lab angle theta)\n", incident->name, target->name);
    if(incident->mass >= target->mass && theta > theta_max) {
        fprintf(stderr, "theta_max = %g deg (scattering not possible)\n", theta_max/C_DEG);
        return;
    }
    fprintf(stderr, "theta = %g deg\n", theta/C_DEG);
    fprintf(stderr, "theta_cm = %g deg\n", theta_cm/C_DEG);

    if(incident->mass <= target->mass) {
        fprintf(stderr, "E_rbs = %g keV\n", E_rbs/C_KEV);
    } else {
        fprintf(stderr, "E_rbs = %g keV (plus-sign solution)\n", E_rbs/C_KEV);
        fprintf(stderr, "E_rbs = %g keV (minus-sign solution)\n", jibal_kin_rbs(incident->mass, target->mass, theta, '-')*E/C_KEV);
    }
    fprintf(stderr, "RBS cross section = %g mb/sr (%s)\n", cs_rbs/C_MB_SR, jibal_cs_rbs_name(jibal->config));
}

void print_kin_erd(jibal *jibal, const jibal_isotope *incident, const jibal_isotope *target, double phi, double E) {
    double cs_erd = jibal_cs_erd(jibal->config, incident, target, phi, E);
    double E_erd = jibal_kin_erd(incident->mass, target->mass, phi) * E;
    double theta_cm= C_PI - 2 * phi;
    double theta = atan2(sin(theta_cm), (cos(theta_cm) + target->mass / incident->mass));

    fprintf(stderr, "ERD (%s recoiled by %s to lab angle phi)\n", target->name, incident->name);
    if(phi >= C_PI/2.0) {
        fprintf(stderr, "Not possible.\n");
        return;
    }
    fprintf(stderr, "phi = %g deg\n", phi/C_DEG);
    fprintf(stderr, "theta_cm = %g deg\n", theta_cm/C_DEG);
    fprintf(stderr, "E_erd = %g keV\n", E_erd/C_KEV);
    fprintf(stderr, "v_erd = %g m/s\n", jibal_velocity(E_erd, target->mass));
    fprintf(stderr, "ERD cross section = %g mb/sr (%s)\n", cs_erd/C_MB_SR, jibal_cs_erd_name(jibal->config));
    double inverse_scaling = 4.0 * pow(sin(theta), 2.0) * cos(theta_cm - theta) * cos(phi) / (pow(sin(theta_cm), 2.0));
    double E_inv = (target->mass/incident->mass) * E;
    if(inverse_scaling > 0.0) {
        fprintf(stderr, "ERD cross section is %g times the RBS cross section for %g MeV %s scattering from %s to an angle of %g deg\n",
                inverse_scaling,
                E_inv/C_MEV,
                target->name,
                incident->name,
                theta/C_DEG
        );
    }
}

int print_kin(jibaltool_global *global, int argc, char **argv) {
    jibal *jibal = global->jibal;
    if(argc < 4) {
        fprintf(stderr, "Usage: jibaltool kin <incident ion> <target isotope> <energy> <angle>\n\nExample: jibaltool kin 4He 28Si 170deg 2MeV\n");
        return EXIT_FAILURE;
    }
    const jibal_isotope *incident = jibal_isotope_find(jibal->isotopes, argv[0], 0, 0);
    if(!incident) {
        fprintf(stderr, "There is no isotope %s in my database.\n", argv[0]);
        return -1;
    }
    const jibal_isotope *target = jibal_isotope_find(jibal->isotopes, argv[1], 0, 0);
    if(!target) {
        fprintf(stderr, "There is no isotope %s in my database.\n", argv[1]);
        return -1;
    }
    double angle = jibal_get_val(jibal->units, UNIT_TYPE_ANGLE, argv[2]); /* RBS */
    if(angle < 0.0 || angle > 180.0*C_DEG) {
        fprintf(stderr, "Please give an angle in range of [0, 180 deg]. You gave %g deg.\n", angle/C_DEG);
        return EXIT_FAILURE;
    }
    double E = jibal_get_val(jibal->units, UNIT_TYPE_ENERGY, argv[3]);
    if(E < 0.0 || E > 10000.0*C_MEV) { /* I guess 10 GeV is quite high */
        fprintf(stderr, "Please check the energy (give a unit if you like to, e.g. \"2.0MeV\"). You gave %g MeV.\n",
                E / C_MEV);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "E_lab = %g keV\n", E/C_KEV);
    double E_cm = target->mass*E/(incident->mass + target->mass);
    fprintf(stderr, "E_cm = %g keV\n", E_cm/C_KEV);
    print_kin_rbs(jibal, incident, target, angle, E);
    fprintf(stderr, "\n");
    print_kin_erd(jibal, incident, target, angle, E);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    jibaltool_global global = {.Z=0, .outfilename=NULL, .stopfile=NULL, .format=NULL, .verbose=0, .jibal = NULL};
    read_options(&global, &argc, &argv);
#ifdef DEBUG
    fprintf(stderr, "Argument vector after 1st round:\n");
    for(int i = 0; i < argc; i++) {
        fprintf(stderr, "argv[%i] = %s\n", i, argv[i]);
    }
#endif
    static const struct command commands[] = {
            {"extract", &extract,"Extract values (e.g. He in Si or a range) in GSTO compatible format."},
            {"extract_stop_material", &extract_stop_material,"Extract stopping from a single stopping file for a given ion and material. (e.g. 4He in SiO2)"},
            {"files", &print_gstofiles, "Print available GSTO files."},
            {"isotopes", &print_isotopes, "Print a list of isotopes."},
            {"elements", &print_elements, "Print a list of elements."},
            {"config", &print_config, "Print current configuration (config file)."},
            {"status", &print_status, "Print JIBAL status."},
            {"units", &print_units, "Print recognized units."},
            {"cs", &print_cs, "Calculate cross sections."},
            {"kin", &print_kin, "Calculate kinematics."},
            {"stop", &print_stop, "Calculate stopping and energy loss."},
            {NULL, NULL, NULL}
    };
    if(argc < 1) {
        jibaltool_usage();
        fprintf(stderr, "\n");
        print_commands(stderr, commands);
        return EXIT_FAILURE;
    }

    const struct command *c, *c_found = NULL;

    for(c = commands; c->f != NULL; c++) {
        if(strcmp(c->name, argv[0]) == 0) {
            c_found = c;
            break;
        }
    }
    if(c_found) {
        global.jibal = jibal_init(global.config_filename);
        if (global.jibal->error) {
            fprintf(stderr, "Initializing JIBAL failed with error code: %i (%s)\n",
                    global.jibal->error, jibal_error_string(global.jibal->error));
            return EXIT_FAILURE;
        }
        c_found->f(&global, argc - 1, argv + 1);
        jibaltool_global_free(&global);
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "No such command: %s\n\n", argv[0]);
        print_commands(stderr, commands);
        jibaltool_global_free(&global);
        return EXIT_FAILURE;
    }
}
