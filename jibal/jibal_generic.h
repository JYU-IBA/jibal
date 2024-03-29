/*
    Jyväskylä Ion Beam Analysis Library (JIBAL)
    Copyright (C) 2020 - 2023 Jaakko Julin <jaakko.julin@jyu.fi>

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
#ifndef JIBAL_GENERIC_H
#define JIBAL_GENERIC_H

#include <stdio.h>

int jibal_isdigit(char c);
FILE *jibal_fopen(const char *filename, const char *mode); /* opens file and returns file pointer (like fopen()), returns NULL if fails, stderr if filename is NULL, stdout if filename is "-" */
void jibal_fclose(FILE *f); /* fclose() unless f is either stdin, stdout or stderr */
char *jibal_strsep(char **stringp, const char *delim); /* Just regular strsep */
char *jibal_strsep_with_quotes(char **stringp, const char *delim);
char *jibal_remove_double_quotes(char *s);
#endif // JIBAL_GENERIC_H
