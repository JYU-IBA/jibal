#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#ifdef WIN32
#include <win_compat.h>
#include <jibal_registry.h>
#include <io.h>
#else
#include <unistd.h>
#include <libgen.h>
#endif
#include <jibal_generic.h>
#include <jibal_config.h>
#include <jibal_defaults.h>


char *make_path_and_check_if_exists(const char *directory, const char *file) {
    /* Return the constructed path (which can be later freed) on success, NULL otherwise. */
    char *filename;
    if(!directory || !file) {
        return NULL;
    }
    if(asprintf(&filename, "%s/%s", directory, file) < 0) {
        return NULL;
    }
    filename=jibal_path_cleanup(filename);
    if(!filename)
        return NULL;
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
    const jibal_config_var vars[] = {
            {JIBAL_CONFIG_VAR_PATH,   "datadir",           0, 0, &config->datadir,          NULL, "GSTO data directory"},
            {JIBAL_CONFIG_VAR_PATH,   "userdatadir",       0, 0, &config->userdatadir,      NULL, "GSTO data directory (user)"},
            {JIBAL_CONFIG_VAR_PATH,   "masses_file",       0, 0, &config->masses_file,      NULL, "Atomic masses files" },
            {JIBAL_CONFIG_VAR_PATH,   "abundances_file",   0, 0, &config->abundances_file,  NULL, "Isotopic abundances files"},
            {JIBAL_CONFIG_VAR_PATH,   "files_file",        0, 0, &config->files_file,       NULL, "GSTO stopping files file"},
            {JIBAL_CONFIG_VAR_PATH,   "assignments_file",  0, 0, &config->assignments_file, NULL, "GSTO stopping assignments file"},
            {JIBAL_CONFIG_VAR_INT,    "Z_max",             0, 0, &config->Z_max,            NULL, "Maximum element number (Z)"},
            {JIBAL_CONFIG_VAR_BOOL,   "extrapolate",       0, 0, &config->extrapolate,      NULL, "Extrapolate stopping"},
            {JIBAL_CONFIG_VAR_OPTION, "rbs_cross_section", 0, 0, &config->cs_rbs, jibal_cs_types, "RBS cross section default"},
            {JIBAL_CONFIG_VAR_OPTION, "erd_cross_section", 0, 0, &config->cs_erd, jibal_cs_types, "ERD cross section default"},
            {0,                       0,                   0, 0, NULL,                      NULL, NULL}
    }; /* null terminated, we use .type == 0 to stop a loop */
    int n_vars;
    for(n_vars = 0; vars[n_vars].type != 0; n_vars++);
    size_t s = sizeof(jibal_config_var)*(n_vars+1); /* +1 because the null termination didn't count */
    jibal_config_var *vars_out = malloc(s);
    if(!vars_out)
        return NULL;
    memcpy(vars_out, vars, s);
    return vars_out;
}

const char *jibal_config_var_type_name(jibal_config_var_type type) {
    switch(type) {
        case JIBAL_CONFIG_VAR_NONE:
            return JIBAL_OPTION_STR_NONE;
        case JIBAL_CONFIG_VAR_STRING:
            return "string";
        case JIBAL_CONFIG_VAR_PATH:
            return "path";
        case JIBAL_CONFIG_VAR_BOOL:
            return "boolean";
        case JIBAL_CONFIG_VAR_INT:
            return "integer";
        case JIBAL_CONFIG_VAR_DOUBLE:
            return "float";
        case JIBAL_CONFIG_VAR_UNIT:
            return "unit";
        case JIBAL_CONFIG_VAR_OPTION:
            return "option";
        case JIBAL_CONFIG_VAR_SIZE:
            return "size";
    }
    return NULL;
}

const char *jibal_config_option_string(const jibal_config_var *var) {
    if(var->option_list == NULL)
        return "Null";
    int i = *((int *)var->variable);
    if(i < 0)
        return "Negative";
    return jibal_option_get_string(var->option_list, i);
}

int jibal_config_write_to_file(const jibal_units *units, jibal_config *config, const char *filename) {
    jibal_config_file *cf = jibal_config_file_init(units);
    if(!cf)
        return 2;
    jibal_config_var *vars = make_config_vars(config);
    if(!vars)
        return 3;
    int retval = 0;
    jibal_config_file_set_vars(cf, vars);
    if(jibal_config_file_write(cf, filename)) {
        fprintf(stderr, WARNING_STRING "Could not write configuration file \"%s\"\n", filename);
        retval = 1;
    }
    jibal_config_file_free(cf);
    return retval;
}

int jibal_path_is_absolute(const char *path) { /* TRUE if it looks absolute, FALSE if it doesn't. Can give false negatives, but is unlikely to give false positives. Just a guess really. */
    if(!path)
        return FALSE;
#ifdef WIN32
    if(path[0] == '\\') /* An absolute path from the root of the current drive, e.g. \Program Files\ is ok. UNC paths and other stuff starting with \\ is always absolute too. */
        return TRUE;
    if(strlen(path) < 3) /* Absolute paths can't really be this short, can they? Lets assume so. */
        return FALSE;
    if(isupper(path[0]) && path[1] == ':' && (path[2] == '/' || path[2] == '\\')) /* Looks like "C:\" or even "C:/" is okay for me */
        return TRUE;
    return FALSE; /* Doesn't look absolute, must be something else */
#else
    return (path[0]=='/');
#endif
}

char *jibal_path_cleanup(char *path) { /* In place removal of repeated back or forward slashes. */
    const char *src = path;
    char *dst = path;
    if(!path)
        return NULL;
    if(*src == '/' || *src == '\\') { /* Allow repeated '//' or '\\' in the beginning by skipping over the first character */
        src++;
        dst++;
    }
    while ((*dst = *src) != '\0') {
        do {
            src++;
        } while ((*dst == '/' || *dst == '\\')  && (*src == '/' || *src == '\\'));
        dst++;
    }
#ifdef WIN32
    if(strlen(path) > 2) {
        for(dst = path+(jibal_path_is_absolute(path)?2:0); *dst != '\0'; dst++) { /* Skip initial 2 characters for absolute paths, just to be sure UNC paths starting with e.g. "\\" will not be converted to "//". */
            if(*dst == '/') /* Replace forwards slashes with backlashes */
                *dst = '\\';
        }
    }
#endif
    return path;
}

int jibal_config_option_get(const jibal_config_var *var, const char *value) {
    jibal_option_get_value(var->option_list, value);
    return 0;
}

int jibal_config_read_from_file(const jibal_units *units, jibal_config *config, const char *filename) { /* Memory leaks in
 * config shouldn't happen (strings are freed and allocated as is necessary, so it is possible to read multiple
 * configuration files. */
    int retval = 0;
    if(!filename) {
        return 1;
    }
    jibal_config_file *cf = jibal_config_file_init(units);
    if(!cf)
        return 2;
    jibal_config_var *vars = make_config_vars(config);
    if(!vars)
        return 3;
    jibal_config_file_set_vars(cf, vars);
    if(jibal_config_file_read(cf, filename)) {
        fprintf(stderr, WARNING_STRING "Could not read configuration file \"%s\"\n", filename);
        retval = 1;
    }
    jibal_config_file_free(cf);
    return retval;
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
    if(asprintf(&out, "%s/%s/", dir, "jibal") < 0)
#else
    if(asprintf(&out, "%s/%s/", dir, ".jibal") < 0)
#endif
        return NULL;
    return jibal_path_cleanup(out); /* Remember to free when done, memory allocated by asprintf*/
}

int jibal_config_user_dir_mkdir_if_necessary() {
    /* Returns 0 if jibal_config_user_dir() directory exists or if it could be created. */
    /* Limitation: can only make directories one level deep at least on Windows, so if the parent directory doesn't
     * exists, is fails. */
    char *path = jibal_config_user_dir();
    if(!path)
        return -1;
    int retval=0;  /* Success */
    if(access(path, F_OK) == 0) {/* Something exists */
        struct stat status;
        stat(path, &status );
        if((status.st_mode & S_IFDIR) == 0) {
            fprintf(stderr, "There is something in %s, but it is not a directory.\n", path);
            retval = -2;
        }
    } else { /* Nothing exits, create! */
#ifdef WIN32
        int status = _mkdir(path);
#else
        int status = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
        if(status != 0) {
            fprintf(stderr, "Making directory %s failed, got error %i.\n", path, status);
            retval = -3;
        }
    }
    return retval;
}

char *jibal_config_user_config_filename() {
    char *filename;
    char *dir = jibal_config_user_dir();
    if(!dir)
        return NULL;
    if(asprintf(&filename, "%s/%s", dir, JIBAL_CONFIG_FILE) < 0) {
        free(dir);
        return NULL;
    }
    free(dir);
    return jibal_path_cleanup(filename);
}

jibal_config jibal_config_defaults() {
    jibal_config config = {.Z_max = JIBAL_MAX_Z, .extrapolate = FALSE, .error = 0, .config_file = NULL, .cs_rbs = JIBAL_CS_ANDERSEN, .cs_erd = JIBAL_CS_ANDERSEN};
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
#ifdef WIN32
    if(!config->datadir) {
        config->datadir=jibal_registry_string_get("RootDirectory");
        if(config->datadir) {
            config->datadir = realloc(config->datadir, strlen(config->datadir) + 1 + strlen(JIBAL_DATADIR) + 1 + 1);
            if (!config->datadir) {
                fprintf(stderr, "Allocation issues with config->datadir.\n");
                return;
            }
            config->datadir = strcat(config->datadir, "/");
            config->datadir = strcat(config->datadir, JIBAL_DATADIR);
            config->datadir = strcat(config->datadir, "/");
            config->datadir = jibal_path_cleanup(config->datadir);
            /* TODO: check if exists */
        }
    }
#endif
    if(!config->datadir) {
        config->datadir=strdup(JIBAL_DATADIR_FULL); /* Directory set by CMake is normal if "make install" has been used. */
    }
    const char *dirs[] = {".", config->userdatadir, config->datadir, NULL};
    const char **dir;
    for(dir=dirs; *dir != NULL; dir++) {
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

char *jibal_config_filename_seek() {
    unsigned int attempt=1;
    /* Lets look for a configuration file!
     * 1. Environmental variable (JIBAL_CONFIG_DIR)
     * 2. Current working directory
     * 3. Under home (~/.jibal/) or %AppData%/jibal (usually something like C:\Users\Jaakko\AppData\Roaming\jibal)
     * 4. Prefix (set at compile time, e.g. /usr/local/etc/jibal/),
     * 5. Platform specific place (UNIX: /etc) on Windows we look one directory up, (useful if the program is in bin directory)
     *
     * On first success (file exist) a full path to the file is returned. The memory is allocated so you have to free it.
     *
     * Returns NULL on failure to find any files.
     *
     * */
    char *filename=NULL;
    while (!filename && attempt) { /* Loop until somebody sets attempt = 0 (no success) or filename candidate is
 * found */
        char *dir;
        switch (attempt) {
            case 1:
                dir = getenv("JIBAL_CONFIG_DIR");
                if(dir) {
                    if(asprintf(&filename, "%s/%s", dir, JIBAL_CONFIG_FILE) < 0)
                        filename=NULL;
                }
                break;
            case 2:
                filename = strdup(JIBAL_CONFIG_FILE);
                break;
            case 3:
                filename = jibal_config_user_config_filename();
                break;
            case 4:
                /* TODO: Installers might put everything in a different directory */
                if(asprintf(&filename, "%s/etc/jibal/%s", JIBAL_INSTALL_PREFIX, JIBAL_CONFIG_FILE) < 0)
                    return NULL;
                break;
            case 5:
#ifdef WIN32
                if(asprintf(&filename, "../%s", JIBAL_CONFIG_FILE) < 0)
#else
                if(asprintf(&filename, "/etc/jibal/%s", JIBAL_CONFIG_FILE) < 0)
#endif
                    return NULL;
                break;
            default:
                attempt = 0;
                break;
        }
        if (attempt == 0) {/* We ran out of places to look for */
            break;
        }
        if(filename) {
#ifdef DEBUG
            fprintf(stderr, "Config attempt %i file %s\n", attempt, filename);
#endif
            if (access(filename, R_OK) != 0) { /* Not found, bad candidate, but let's continue */
                free(filename);
                filename = NULL;
            }
        }
        attempt++;
    }
    if(filename) {
#ifdef DEBUG
        fprintf(stderr, "Returning final candidate %s\n", filename);
#endif
        return jibal_path_cleanup(filename);
    }
    return NULL;
}

jibal_config *jibal_config_init(const jibal_units *units, const char *filename, int seek) {
    jibal_config *config = malloc(sizeof(jibal_config));
    *config = jibal_config_defaults();
    char *f = NULL;
    if(filename) {
        f = jibal_path_cleanup(strdup(filename)); /* No memory leak here, trust me */
    } else if(seek) {
        f = jibal_config_filename_seek();
    }
    if(f) {
        config->error = jibal_config_read_from_file(units, config, f);
        config->config_file = f;
    }
    jibal_config_finalize(config);
    return config; /* Note: configuration is not validated. We only set config.error if we TRIED to read a configuration file but failed.  */
}

void jibal_config_free(jibal_config *config) {
    free(config->config_file);
    free(config->datadir);
    free(config->userdatadir);
    free(config->masses_file);
    free(config->abundances_file);
    free(config->files_file);
    free(config->assignments_file);
    free(config);
}

jibal_config_file *jibal_config_file_init(const jibal_units *units) {
    jibal_config_file *cf = calloc(1, sizeof(jibal_config_file));
    cf->units = units;
    return cf;
}

int jibal_config_file_set_vars(jibal_config_file *cf, jibal_config_var *vars) {
    cf->vars = vars;
    return 0;
}

void jibal_config_file_free(jibal_config_file *cf) {
    if(!cf)
        return;
    free(cf->vars);
    free(cf->filename);
    free(cf);
}

int jibal_config_file_read(jibal_config_file *cf, const char *filename) {
    unsigned int lineno=0;
    char *line_orig = NULL;
    size_t line_size=0;
    if(!cf)
        return 1;
    assert(cf->units);
    if(!cf->vars) {
        return 0; /* No variables, nothing read (no attempt even made) */
    }
    free(cf->filename);
    cf->filename = strdup(filename);
    FILE *f = jibal_fopen(filename, "r");
    if(!f)
        return 1;
    while(getline(&line_orig, &line_size, f) > 0) {
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
        if(line[eq_pos] == '\0' || eq_pos <= 1) {
            fprintf(stderr,  WARNING_STRING "Malformed configuration file %s line %i: \"%s\"\n", filename, lineno, line_orig);
            continue;
        }
        line[eq_pos]='\0'; /* Separate argument name from value by replacing the first '=' with a null */
        size_t s;
        for(s=eq_pos-1; s > 0 && isspace(line[s]); s--) {line[s]='\0';} /* Replace whitespace before '=' with nulls */
        char *line_val=line+eq_pos+1; /* First character after the '=', could also be a '\0'! */
        while(isspace(*line_val)) {line_val++;} /* Ignore leading whitespace in values */
#ifdef DEBUG
        fprintf(stderr, "line %i, key=\"%s\", val=\"%s\"\n", lineno, line_var, line_val);
#endif
        if(jibal_config_file_var_set(cf, line_var, line_val)) {
            fprintf(stderr,
                    "JIBAL WARNING: line %i of configuration file: variable \"%s\" unknown or other error. The ignored value was \"%s\"\n",
                    lineno, line_var, line_val);
        }
    }
    free(line_orig);
    jibal_fclose(f);
    return 0;
}

int jibal_config_file_write(const jibal_config_file *cf, const char *filename) {
    if(!cf)
        return 1;
    assert(cf->units);
    if(!cf->vars)
        return 0; /* No variables, nothing written (no attempt even made) */
    FILE *f = jibal_fopen(filename, "w");
    if(!f)
        return 1;
    const jibal_config_var *var;
    for(var = cf->vars; var->type != 0; var++) {
        if(var->variable == NULL)
            continue;
        switch(var->type) {
            case JIBAL_CONFIG_VAR_NONE:
                break;
            case JIBAL_CONFIG_VAR_PATH:
            case JIBAL_CONFIG_VAR_STRING:
                if (*((void **)var->variable) == NULL)
                    continue;
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
            case JIBAL_CONFIG_VAR_OPTION:
                fprintf(f, "%s = %s\n", var->name, jibal_option_get_string(var->option_list, *((int *)var->variable)));
                break;
            case JIBAL_CONFIG_VAR_SIZE:
                fprintf(f, "%s = %zu\n", var->name, *((size_t *)var->variable));
                break;
        }
    }
    jibal_fclose(f);
    return 0;
}

int jibal_config_file_var_set(jibal_config_file *cf, const char *var_str, const char *val) {

    jibal_config_var *var;
    for(var = cf->vars; var->type != 0; var++) {
        if(strcmp(var->name, var_str) == 0) {
            break;
        }
    }
    if(var->type == 0) { /* No matching var was found */
        return 1;
    }
#ifdef DEBUG
    fprintf(stderr, "Setting \"%s\" (type %i) to \"%s\" (filename = %s)\n", var->name, var->type, val, cf->filename);
#endif
    return jibal_config_var_set(cf->units, var, val, cf->filename);
}

int jibal_config_var_set(const jibal_units *units, jibal_config_var *var, const char *val, const char *filename) {
    if(var->variable == NULL) { /* No data pointer has been set */
        return 1;
    }
    char *end;
    switch(var->type) {
        case JIBAL_CONFIG_VAR_NONE:
            return 1;
            break;
        case JIBAL_CONFIG_VAR_STRING:
            if(*((char **)var->variable)) { /*  Our void * is actually char ** */
                free(*((char **)var->variable));
            }
            *((char **)var->variable)=strdup(val);
            break;
        case JIBAL_CONFIG_VAR_PATH:
            if(*((char **)var->variable)) { /*  Our void * is actually char ** */
                free(*((char **)var->variable));
            }
            if(jibal_path_is_absolute(val) || !filename) {
#ifdef DEBUG
                fprintf(stderr, "Path is absolute or no filename was given.\n");
#endif
                *((char **)var->variable)=strdup(val);
            } else {
                char *tmp=strdup(filename); /* Get a char we can mutilate */
                char *tmp2=dirname(tmp);
                char *out;
                if(!tmp2 || strlen(tmp2) == 0) { /* Windows kludge. */
                    out=strdup(val);
                } else {
                    if(asprintf(&out, "%s/%s", tmp2, val) < 0)
                        out = NULL;
                }
                jibal_path_cleanup(out);
                *((char **)var->variable)=out;
                free(tmp);
            }
            break;
        case JIBAL_CONFIG_VAR_BOOL:
            *((int *)var->variable)=!strcmp(val, "true"); /* exactly "true" is 1, everything else is 0 */
            break;
        case JIBAL_CONFIG_VAR_INT:
            *((int *)var->variable)=(int)strtol(val, &end, 0); /* Unsafe for large integers */
            break;
        case JIBAL_CONFIG_VAR_DOUBLE:
            *((double *)var->variable)=strtod(val, &end);
            break;
        case JIBAL_CONFIG_VAR_UNIT: /* TODO: error checking */
            strtod(val, &end);
            *((double *)var->variable) = jibal_get_val(units, var->unit_type, val);
            break;
        case JIBAL_CONFIG_VAR_OPTION:
            *((int *)var->variable)=jibal_option_get_value(var->option_list, val);
            break;
        case JIBAL_CONFIG_VAR_SIZE:
            *((int *)var->variable)=(int)strtoul(val, NULL, 0); /* Handles up to 32 bit probably ok. */
            break;
    }
    return 0;
}

const jibal_config_var *jibal_config_var_find(jibal_config_var *vars, const char *str) {
    const jibal_config_var *var, *var_found;
    size_t len = strlen(str);
    int found = 0;
    for(var = vars; var->type != 0; var++) {
        if(strncmp(var->name, str, len) == 0) {
            found++;
            var_found = var;
        }
    }
    if(found == 1) {
        return var_found;
    } else {
        return NULL;
    }
}
