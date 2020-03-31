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
    jibal_free(&global->jibal);
    /* Doesn't free "options" itself. */
}

void jibaltool_usage() {
    fprintf(stderr, JIBAL_TOOL_HELP_STRING);
    exit(EXIT_SUCCESS);
}

void read_options(jibaltool_global *global, int *argc, char ***argv) {
    static struct option long_options[] = {
            {"help",        no_argument,        NULL, 'h'},
            {"version",     no_argument,        NULL, 'v'},
            {"nop",         no_argument,        NULL, 'n'},
            {"out",         required_argument,  NULL, 'o'},
            {"stopfile",    required_argument,  NULL, 's'},
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
            default:
                jibaltool_usage();
                break;
        }
    }
    int i;
    *argc -= optind;
    *argv += optind;
}

int extract(jibaltool_global *global, int argc, char **argv) {
    int i;
    if (argc < 2) {
        fprintf(stderr, "Usage: jibaltool extract incident target\n");
        return -1;
    }
    jibal *jibal = &global->jibal;
    jibal_element *incident = jibal_element_find(jibal->elements, argv[0]);
    jibal_element *target = jibal_element_find(jibal->elements, argv[1]);
    int Z1 = incident->Z;
    int Z2 = target->Z;
    gsto_file_t *file;
    if(global->stopfile) {
        file = jibal_gsto_get_file(jibal->gsto, global->stopfile);
    } else {
        jibal_gsto_auto_assign(jibal->gsto, Z1, Z2);
        file = jibal_gsto_get_assigned_file(jibal->gsto, Z1, Z2);
    }
    if(!file) {
        return -1;
    }
    jibal_gsto_load(jibal->gsto, file);

    FILE *out=stdout;
    if(global->outfilename) {
        out=fopen(global->outfilename, "w");
        if(!out) {
            return 0;
        }
    }
    jibal_gsto_fprint_file(out, file, Z1, Z1, Z2, Z2);
    if(out != stdout) {
        fclose(out);
    }
    return 0;
}

int main(int argc, char **argv) {
    jibaltool_global global = {.Z=0, .outfilename=NULL, .stopfile=NULL};
    read_options(&global, &argc, &argv);
    struct command {
        const char *name;
        int (*f)(jibaltool_global *, int, char **);
    };
    static struct command commands[] = {
            {"extract", &extract},
            {NULL, NULL}
    };
    if(argc < 1) {
        jibaltool_usage();
    }
    global.jibal = jibal_init(global.config_filename);
    struct command *c;
    int found=0;
    for(c=commands; c->f != NULL; c++) {
        if(strcmp(c->name, argv[0])==0) {
            found=1;
            c->f(&global, argc-1, argv+1);
            break;
        }
    }
    if(!found) {
        fprintf(stderr, "No such command: %s\n", argv[0]);
    }
    jibaltool_global_free(&global);
    return EXIT_SUCCESS;
}
