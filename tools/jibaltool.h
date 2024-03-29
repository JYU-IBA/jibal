/*
    Jyväskylä Ion Beam Analysis Library (JIBAL)
    Copyright (C) 2020-2022 Jaakko Julin <jaakko.julin@jyu.fi>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef JIBAL_JIBALTOOL_H
#define JIBAL_JIBALTOOL_H

#include <jibal.h>

#define JIBAL_TOOL_HELP_STRING "Usage: jibaltool [--version] [--help] <command> [<args>]\n"


typedef  struct {
    jibal *jibal;
    int Z;
    int verbose;
    char *config_filename;
    char *outfilename;
    char *stopfile;
    char *format;
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
