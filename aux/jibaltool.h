#ifndef JIBAL_JIBALTOOL_H
#define JIBAL_JIBALTOOL_H

#include <jibal.h>

#define JIBAL_TOOL_HELP_STRING "Usage: jibaltool [--version] [--help] <command> [<args>]\n"


typedef  struct {
    jibal jibal;
    int Z;
    char *config_filename;
    char *outfilename;
    char *stopfile;
} jibaltool_global;

struct command {
    const char *name;
    int (*f)(jibaltool_global *, int, char **);
    const char *help_text;
};

void jibaltool_global_free(jibaltool_global *options);
void jibaltool_usage();
void read_options(jibaltool_global *global, int *argc, char ***argv);

int extract_stop(jibaltool_global *options, int argc, char **argv);

#endif //JIBAL_JIBALTOOL_H
