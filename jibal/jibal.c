#if defined(__GNUC__) && ! defined(_GNU_SOURCE)
#define _GNU_SOURCE // Needed for asprintf (from stdio.h) on GNU/Linux
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#define R_OK 4
#else
#include <unistd.h>
#endif
#include <ctype.h>
#include <jibal.h>
#include <defaults.h>

jibal jibal_init(const char *config_filename) {
    jibal jibal;
    jibal.error = JIBAL_ERROR_NONE;
    jibal.units=jibal_units_default();
    if(!jibal.units) {
        jibal.error = JIBAL_ERROR_UNITS;
        return jibal;
    }
    jibal.config = jibal_config_init(jibal.units, config_filename, TRUE);
    if(jibal.config.error) {
        jibal.error = JIBAL_ERROR_CONFIG;
        return jibal;
    }
    jibal.isotopes = jibal_isotopes_load(jibal.config.masses_file);
    if(!jibal.isotopes) {
        fprintf(stderr, "Could not load isotope table from file %s.\n", jibal.config.masses_file);
        jibal.error = JIBAL_ERROR_MASSES;
        return jibal;
    }
    if(jibal_abundances_load(jibal.isotopes, jibal.config.abundances_file) < 0) {
        jibal.error = JIBAL_ERROR_ABUNDANCES;
        return jibal;
    }
    jibal.elements=jibal_elements_populate(jibal.isotopes);
#ifdef DEBUG
    fprintf(stderr, "The Z_max of elements array is %i\n", jibal_elements_Zmax(jibal.elements));
#endif
    if(!jibal.elements) {
        jibal.error = JIBAL_ERROR_ELEMENTS;
        return jibal;
    }
    jibal.gsto= jibal_gsto_init(jibal.elements, jibal.config.Z_max, jibal.config.datadir, jibal.config.files_file, jibal.config.assignments_file);
    if(!jibal.gsto) {
        fprintf(stderr, "Could not initialize GSTO.\n");
        jibal.error = JIBAL_ERROR_GSTO;
        return jibal;
    }
    jibal.gsto->extrapolate = jibal.config.extrapolate;
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

char *make_path_and_check_if_exists(const char *directory, const char *file) {
    /* Return the constructed path (which can be later freed) on success, NULL otherwise. */
    char *filename;
    if(!directory || !file) {
        return NULL;
    }
    if(asprintf(&filename, "%s/%s", directory, file) < 0) {
        return NULL;
    }
    if(access(filename, R_OK) != 0)  { /* File doesn't exist */
#ifdef DEBUG
        fprintf(stderr, "File \"%s\" doesn't exist.\n", filename);
#endif
        free(filename);
        return NULL;
    }
    return filename;
}

jibal_config_var *make_config_vars(jibal_config *config) { /* Makes a structure that defines config options. Used
 * when config files are read and written. Default values should be handled elsewhere. */
    const jibal_config_var vars[]={
            {JIBAL_CONFIG_VAR_STRING, "datadir", &config->datadir},
            {JIBAL_CONFIG_VAR_STRING, "userdatadir", &config->userdatadir},
            {JIBAL_CONFIG_VAR_STRING, "masses_file", &config->masses_file},
            {JIBAL_CONFIG_VAR_STRING, "abundances_file", &config->abundances_file},
            {JIBAL_CONFIG_VAR_STRING, "files_file", &config->files_file},
            {JIBAL_CONFIG_VAR_STRING, "assignments_file", &config->assignments_file},
            {JIBAL_CONFIG_VAR_INT, "Z_max", &config->Z_max},
            {JIBAL_CONFIG_VAR_BOOL, "extrapolate", &config->extrapolate},
            {0, 0, NULL}
    }; /* null terminated, we use .type == 0 to stop a loop */
            int n_vars;
            for(n_vars=0; vars[n_vars].type != 0; n_vars++);
            size_t s=sizeof(jibal_config_var)*(n_vars+1); /* +1 because the null termination didn't count */
            jibal_config_var *vars_out=malloc(s);
            memcpy(vars_out, vars, s);
            return vars_out;
}

int jibal_config_file_write(jibal_config *config, FILE *f) {
    jibal_config_var *vars=make_config_vars(config);
    const jibal_config_var *var;
    for(var=vars; var->type != 0; var++) {
        switch(var->type) {
            case JIBAL_CONFIG_VAR_NONE:
                break;
            case JIBAL_CONFIG_VAR_STRING:
                fprintf(f, "%s = %s\n", var->name, *((char**)var->variable));
                break;
            case JIBAL_CONFIG_VAR_BOOL:
                fprintf(f, "%s = %s\n", var->name, *((int *)var->variable)?"true":"false");
                break;
            case JIBAL_CONFIG_VAR_INT:
                fprintf(f, "%s = %i\n", var->name, *((int *)var->variable));
                break;
            case JIBAL_CONFIG_VAR_DOUBLE:
                fprintf(f, "%s = %g\n", var->name, *((double *)var->variable));
                break;
            case JIBAL_CONFIG_VAR_UNIT:
                fprintf(f, "%s = %g\n", var->name, *((double *)var->variable));
                break;
        }
    }
    free(vars);
    fclose(f);
    return 0;
}

int jibal_config_file_read(const jibal_units *units, jibal_config *config, const char *filename) { /* Memory leaks in
 * config shouldn't happen (strings are freed and allocated as is necessary, so it is possible to read multiple
 * configuration files. */
    if(!filename) {
        return -2;
    }
    FILE *f=fopen(filename, "rb");
    if(!f) {
        fprintf(stderr, WARNING_STRING "Could not read configuration file \"%s\"\n", filename);
        return -1;
    }
    jibal_config_var *vars=make_config_vars(config);
    const jibal_config_var *var;
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
            fprintf(stderr,  WARNING_STRING "Malformed configuration file %s line %i: \"%s\"\n", filename, lineno, line_orig);
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
                case JIBAL_CONFIG_VAR_BOOL:
                    *((int *)var->variable)=!strcmp(line_val, "true"); /* exactly "true" is 1, everything else is 0 */
                    break;
                case JIBAL_CONFIG_VAR_INT:
                    *((int *)var->variable)=(int)strtol(line_val, NULL, 0); /* Unsafe for large integers */
                    break;
                case JIBAL_CONFIG_VAR_DOUBLE:
                    *((double *)var->variable)=strtod(line_val, NULL);
                    break;
                case JIBAL_CONFIG_VAR_UNIT:
                    *((double *)var->variable)=jibal_get_val(units, 0, line_val);
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
    free(vars);
    return 0;
}

char *jibal_config_user_dir() {
    char *out;
    const char *dir;
#ifdef WIN32
    dir = getenv("AppData");
#else
    dir = getenv("HOME");
#endif
    if(!dir)
        return NULL;
#ifdef WIN32
    asprintf(&out, "%s/%s/", dir, "jibal");
#else
    asprintf(&out, "%s/%s/", dir, ".jibal");
#endif
    return out; /* Remember to free when done, memory allocated by asprintf*/
}

jibal_config jibal_config_defaults() {
    jibal_config config = {.Z_max = JIBAL_MAX_Z, .extrapolate = FALSE, .error = 0};
    const char *c=getenv("JIBAL_DATADIR");
    if(c) {
        config.datadir=strdup(c);
    }
    return config;
}

void jibal_config_finalize(jibal_config *config) { /* Fill in missing defaults based on config so far */
    if(!config->userdatadir) {
        config->userdatadir=jibal_config_user_dir();
    }
    if(!config->datadir) {
        /* TODO: search from /usr or /usr/local or registry on Windows */
        config->datadir=strdup(JIBAL_DATADIR); /* Directory set by CMake is our last hope */
    }
    const char *dirs[] = {config->userdatadir, config->datadir, NULL};
    const char **dir;
    for(dir=dirs; *dir != NULL; dir++) {
        fprintf(stderr, "Trying dir %s\n", *dir);
        if(!config->masses_file) {
            config->masses_file = make_path_and_check_if_exists(*dir, JIBAL_MASSES_FILE);
        }
        if(!config->files_file) {
            config->files_file = make_path_and_check_if_exists(*dir, JIBAL_FILES_FILE);
        }
        if(!config->abundances_file) {
            config->abundances_file = make_path_and_check_if_exists(*dir, JIBAL_ABUNDANCES_FILE);
        }

        if(!config->assignments_file) {
            config->assignments_file = make_path_and_check_if_exists(*dir, JIBAL_ASSIGNMENTS_FILE);
        }
    }
}

jibal_config jibal_config_init(const jibal_units *units, const char *filename, int seek) {
    jibal_config config = jibal_config_defaults();
    unsigned int attempt=1;
    /* Lets look for a configuration file!
     * 0. It is given to us explicitly, or we look for JIBAL_CONFIG_FILE (= jibal.conf) in the following places:
     * 1. Environmental variable (JIBAL_CONFIG_DIR)
     * 2. Current working directory
     * 3. Under home (~/.jibal/) or %AppData%/jibal (usually something like C:\Users\Jaakko\AppData\Roaming\jibal)
     * 4. Prefix (set at compile time, e.g. /usr/local/etc/jibal/),
     * 5. Platform specific place (UNIX: /etc) on Windows we look one directory up, (useful if the program is in bin directory)
     *
     * After this the configuration file (only one!) is read and uninitialized values are set to defaults.
     * It is possible to read another file using jibal_config_file_read()
     * */
    if(filename) {
        config.error=jibal_config_file_read(units, &config, filename);
    } else if(seek) {
        char *filename_attempt=NULL;
        while (!filename_attempt && attempt) { /* Loop until somebody sets attempt = 0 (no success) or filename candidate is
 * found */
            char *dir;
            switch (attempt) {
                case 1:
                    dir = getenv("JIBAL_CONFIG_DIR");
                    if(dir) {
                        asprintf(&filename_attempt, "%s/%s", dir, JIBAL_CONFIG_FILE);
                    }
                    break;
                case 2:
                    filename_attempt = strdup(JIBAL_CONFIG_FILE);
                    break;
                case 3:
                    dir = jibal_config_user_dir();
                    if(dir) {
                        asprintf(&filename_attempt, "%s/%s", dir, JIBAL_CONFIG_FILE);
                    }
                    break;
                case 4:
                    asprintf(&filename_attempt, "%s/etc/jibal/%s", JIBAL_INSTALL_PREFIX, JIBAL_CONFIG_FILE);
                    /* TODO: Installers might put everything in a different directory */
                    break;
                case 5:
#ifdef WIN32
                    asprintf(&filename_attempt, "../%s", JIBAL_CONFIG_FILE);
#else
                    asprintf(&filename_attempt, "/etc/jibal/%s", JIBAL_CONFIG_FILE);
#endif
                    break;
                default:
                    attempt = 0;
                    break;
            }
            if (attempt == 0) {/* We ran out of places to look for */
                break;
            }
            //filename_attempt = make_path_and_check_if_exists(config_dir, config_subdir, JIBAL_CONFIG_FILE);
            if(filename_attempt) {
#ifdef DEBUG
                fprintf(stderr, "Config attempt %i file %s\n", attempt, filename_attempt);
#endif
                if (access(filename_attempt, R_OK) != 0) { /* Not found, bad candidate */
                    free(filename_attempt);
                    filename_attempt = NULL;
                }
            }
            attempt++;
        }
        if(filename_attempt) {
            config.error = jibal_config_file_read(units, &config, filename_attempt);
            free(filename_attempt);
        }
    }
    jibal_config_finalize(&config);
    return config; /* Note: configuration is not validated in any way! */
}

const char *jibal_error_string(jibal_error err) {
    switch(err) {
        case JIBAL_ERROR_NONE:
            return "ERROR_NONE";
        case JIBAL_ERROR_CONFIG:
            return "ERROR_CONFIG";
        case JIBAL_ERROR_UNITS:
            return "ERROR_UNITS";
        case JIBAL_ERROR_MASSES:
            return "ERROR_MASSES";
        case JIBAL_ERROR_ABUNDANCES:
            return "ERROR_ABUNDANCES";
        case JIBAL_ERROR_ELEMENTS:
            return "ERROR_ELEMENTS";
        case JIBAL_ERROR_GSTO:
            return "ERROR_GSTO";
        default:
            return "ERROR_UNKNOWN";
    }
}

void jibal_config_free(jibal_config *config) {
    if(config->datadir)
        free(config->datadir);
    if(config->userdatadir)
        free(config->userdatadir);
    if(config->masses_file)
        free(config->masses_file);
    if(config->abundances_file)
        free(config->abundances_file);
    if(config->files_file)
        free(config->files_file);
    if(config->assignments_file)
        free(config->assignments_file);
    /* Note, not freeing jibal_config itself! */
}
