#ifndef _JIBAL_CONFIG_H_
#define _JIBAL_CONFIG_H_

#include <stdio.h>
#include <jibal_units.h>

typedef struct {
    int error;
    char *datadir; /* e.g. /usr/local/share/jibal or C:\Program Files\Jibal\share\jibal */
    char *userdatadir; /* e.g. ~/.jibal/ or %AppData/Jibal */
    char *masses_file;
    char *abundances_file;
    char *files_file;
    char *assignments_file;
    int Z_max;
    int extrapolate; /* this is boolean, see JIBAL_CONFIG_VAR_BOOL */
} jibal_config; /* Some internal configuration (environment etc) */

typedef enum {
    JIBAL_CONFIG_VAR_NONE = 0,
    JIBAL_CONFIG_VAR_STRING = 1, /* C string, aka. NUL terminated char * */
    JIBAL_CONFIG_VAR_PATH = 2, /* Internally same as above, but we assume relative paths are relative to the config file */
    JIBAL_CONFIG_VAR_BOOL = 3, /* Internally an int */
    JIBAL_CONFIG_VAR_INT = 4, /* 32-bit signed int (aka int) */
    JIBAL_CONFIG_VAR_DOUBLE = 5,
    JIBAL_CONFIG_VAR_UNIT = 6 /* Number with a unit. Store number after SI conversion. */
} jibal_config_var_type;

typedef struct {
    jibal_config_var_type type;
    const char *name;
    void *variable;
} jibal_config_var;

char *jibal_config_user_dir();
int jibal_config_user_dir_mkdir_if_necessary();
char *jibal_config_user_config_filename();
jibal_config jibal_config_defaults();
jibal_config jibal_config_init(const jibal_units *units, const char *filename, int seek);
void jibal_config_finalize(jibal_config *config);
char *jibal_config_filename_seek();
int jibal_config_file_read(const jibal_units *units, jibal_config *config, const char *filename);
int jibal_config_file_write(jibal_config *config, FILE *file);
int jibal_path_is_absolute(const char *path);
char *jibal_path_cleanup(char *path);
void jibal_config_free();
#endif /* _JIBAL_CONFIG_H_ */
