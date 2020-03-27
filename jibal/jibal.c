#include <jibal.h>
#include <defaults.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

jibal jibal_init() {
    jibal jibal;
    jibal.config=jibal_config_init(NULL);
    jibal.isotopes=jibal_isotopes_load(jibal.config.masses_file);
    if(!jibal.isotopes) {
        fprintf(stderr, "Could not load isotope table from file %s.\n", jibal.config.masses_file);
        return jibal;
    }
    jibal_abundances_load(jibal.isotopes, jibal.config.abundances_file);
    jibal.elements=jibal_elements_populate(jibal.isotopes);
    jibal.units=jibal_units_default();
    jibal.gsto=jibal_gsto_init(jibal.config.Z_max, NULL);
    return jibal;
}

void jibal_free(jibal *jibal) {
    if(jibal->units)
        jibal_units_free(jibal->units);
    if(jibal->elements)
        jibal_elements_free(jibal->elements);
    if(jibal->isotopes)
        jibal_isotopes_free(jibal->isotopes);
    if(jibal->gsto)
        jibal_gsto_free(jibal->gsto);
    jibal_config_free(&jibal->config);
    /* Note, not freeing jibal itself! */
}

char *make_path_and_check_if_exists(const char *directory, const char *subdirectory, const char *file) {
    /* TODO: move somewhere and rename
     * TODO: va_list and arbitrary directories?
     * The subdirectory can be NULL. Return the constructed path (which can be later freed) on success, NULL
     * otherwise. *
     */
    char *filename;
    FILE *f;
    if(!directory || !file) {
        return NULL;
    }
    if(subdirectory) {
        if(asprintf(&filename, "%s/%s/%s", directory, subdirectory, file) < 0) {
            return NULL;
        }
    } else {
        if(asprintf(&filename, "%s/%s", directory, file) < 0) {
            return NULL;
        }
    }
#if 0
    size_t i=strlen(directory);
    while(i--) {
        if(directory.datadir[i] == '/') {
            directory.datadir[i] = '\0';
        } else {
            break;
        }
    }
#endif
    if(access(filename, R_OK) != 0)  { /* File doesn't exist */
#ifndef DEBUG
        fprintf(stderr, "File \"%s\" doesn't exist.\n", filename);
#endif
        free(filename);
        return NULL;
    }
    return filename;
}

int read_config_file(jibal_config *config, const char *filename) { /* Memory leaks in config shouldn't happen (strings
 * are freed and allocated as is necessary, so it is possible to read multiple configuration files. */
    if(!filename) {
        return -2;
    }
    FILE *f=fopen(filename, "r");
    if(!f) {
        fprintf(stderr, WARNING_STRING "Could not read configuration file \"%s\"\n", filename);
        return -1;
    }
    jibal_config_var vars[]={{JIBAL_CONFIG_VAR_STRING, "masses_file", &config->masses_file},
                             {JIBAL_CONFIG_VAR_STRING, "abundances_file", &config->abundances_file},
                             {JIBAL_CONFIG_VAR_INT, "Z_max", &config->Z_max},
                             0}; /* null terminated, we use .type == 0 to stop a loop */
    jibal_config_var *var;
    unsigned int lineno=0;
    char *line_orig=malloc(sizeof(char)*JIBAL_CONFIG_MAX_LINE_LEN);
    while(fgets(line_orig, JIBAL_CONFIG_MAX_LINE_LEN, f)) {
        char *line=line_orig;
        lineno++;
        if(line[0] == '#') {
            if(line[1] == '#') {
                fprintf(stderr, "Comment on line %i: %s", lineno, line+2);
            }
            continue;
        }
        line[strcspn(line, "\r\n")] = 0; /* Strips all kinds of newlines! */
        while(isspace(*line)) {line++;} /* Ignore leading whitespace on lines */
        if(strlen(line)==0)
            continue; /* Empty lines (even after stripping newline and whitespace) are ignored */
        char *line_var=line;
        size_t eq_pos=strcspn(line, "="); /* Finds equality sign */
        if(line[eq_pos] == '\0' || eq_pos == 0) {
            fprintf(stderr,  WARNING_STRING "Malformed configuration file line %i: \"%s\"\n", lineno, line);
            continue;
        }
        line[eq_pos]='\0'; /* Separate argument name from value by replacing the first '=' with a null */
        size_t s;
        for(s=eq_pos-1; s >= 0 && isspace(line[s]); s--) {line[s]='\0';} /* Replace whitespace before '=' with nulls */
        char *line_val=line+eq_pos+1; /* First character after the '=', could also be a '\0'! */
        while(isspace(*line_val)) {line_val++;} /* Ignore leading whitespace in values */
#ifdef DEBUG
        fprintf(stderr, "line %i, key=\"%s\", val=\"%s\"\n", lineno, line_var, line_val);
#endif
        for(var=vars; var->type != 0; var++) {
            if(strcmp(var->name, line_var)!=0) {
                continue; /* Doesn't match */
            }
            switch(var->type) {
                case JIBAL_CONFIG_VAR_NONE: /* Execution never reaches this */
                    break;
                case JIBAL_CONFIG_VAR_STRING:
                    if(*((char **)var->variable)) { /*  Our void * is actually char ** */
                        free(*((char **)var->variable));
                    }
                    *((char **)var->variable)=strdup(line_val);
                    break;
                case JIBAL_CONFIG_VAR_DOUBLE:
                    *((double *)var->variable)=strtod(line_val, NULL);
                    break;
                case JIBAL_CONFIG_VAR_INT:
                    *((int *)var->variable)=(int)strtol(line, NULL, 0); /* Unsafe for large integers */
                    break;
            }
            break;
        }
        if(var->type == 0) {
            fprintf(stderr, "JIBAL WARNING: line %i of configuration file: unknown variable \"%s\" with value \"%s\" "
                            "ignored\n",
                    lineno, line_var, line_val);
        }
    }
    free(line_orig);
    fclose(f);
    return 0;
}

jibal_config jibal_config_init(const char *filename) {
    jibal_config config = {};
    const char *c;
    const char *config_dir;
    const char *config_subdir;
    unsigned int attempt=1;
    int error=0;
    /* Lets look for a configuration file!
     * 1. It is given to us
     * 2. Environmental variable
     * 2. Under home (~/.jibal/)
     * 3. Prefix (set at compile time, e.g. /usr/local/etc/jibal/)
     * 4. Platform specific place (UNIX: /etc, WINDOWS: TODO)
     *
     * After this the configuration file (only one!) is read and uninitialized values are set to defaults
     *
     * */
    if(filename) {
        error=read_config_file(&config, filename);
    } else {
        char *filename_attempt=NULL;
        while (!filename_attempt && attempt) { /* Loop until somebody sets attempt = 0 (no success) or filename candidate is
 * found */
            config_dir = NULL;
            config_subdir = NULL;
            switch (attempt) {
                case 1:
                    config_dir = getenv("JIBAL_CONFIG_DIR");
                    break;
                case 2:
                    config_dir = getenv("HOME");
                    config_subdir = "/.jibal/";
                    break;
                case 3:
                    config_dir = JIBAL_INSTALL_PREFIX;
                    config_subdir = "etc/jibal/";
                    break;
                case 4:
                    config_dir = "/etc/jibal/"; // TODO: WINDOWS?
                    break;
                default:
                    attempt = 0;
                    config_dir = NULL;
                    break;
            }
            if (attempt == 0) {/* We ran out of places to look for */
                break;
            }
            filename_attempt = make_path_and_check_if_exists(config_dir, config_subdir, JIBAL_CONFIG_FILE);
            attempt++;
        }
        if(filename_attempt) {
            error = read_config_file(&config, filename_attempt);
            free(filename_attempt);
        }
    }
    c=getenv("JIBAL_DATADIR");
    if(c) {
        if(config.datadir)
            free(config.datadir);
        config.datadir=strdup(c); /* Environmental variable overrides config. */
    }
    if(!config.datadir) {
        config.datadir=JIBAL_DATADIR; /* Directory set by CMake is our last hope */
    }
    if(!config.masses_file) {
        asprintf(&config.masses_file, "%s/%s", config.datadir, JIBAL_MASSES_FILE);
    }
    if(!config.abundances_file) {
        asprintf(&config.abundances_file, "%s/%s", config.datadir, JIBAL_ABUNDANCES_FILE);
    }
    if(!config.stoppings_file) {
        asprintf(&config.stoppings_file, "%s/%s", config.datadir, JIBAL_STOPPINGS_FILE);
    }
    if(config.Z_max==0) {
        config.Z_max = JIBAL_MAX_Z;
    }
    return config; /* Note: configuration is not validated in any way! */
}

void jibal_config_free(jibal_config *config) {
    if(config->datadir)
        free(config->datadir);
    if(config->masses_file)
        free(config->masses_file);
    if(config->abundances_file)
        free(config->abundances_file);

    /* Note, not freeing jibal_config itself! */
}
