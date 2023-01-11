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

#ifndef JIBAL_CSVREADER_H
#define JIBAL_CSVREADER_H

#include <stdio.h>
#define JIBALCSVREADER_DEFAULT_DELIM " \t"
#define JIBALCSVREADER_CSV_DELIM ","
#define JIBALCSVREADER_TSV_DELIM "\t"

typedef enum jibalcsvreader_error {
    JIBALCSVREADER_SUCCESS = (0),
    JIBALCSVREADER_ERROR_GENERIC = (-1),
    JIBALCSVREADER_EOF = (-2),
    JIBALCSVREADER_NOT_ENOUGH_COLUMNS = (-3)
} jibalcsvreader_error;

typedef struct jibalcsvreader_settings {
    int skip_empty_lines;
    int skip_multiple_separators;
    int parse_csv_style_quotes;
    char *delim;
    char comment_char;
} jibalcsvreader_settings;

typedef enum jibalcsvreader_data_types {
    JGTABLE_DATA_NONE = 0,
    JGTABLE_DATA_DOUBLE = 1,
    JGTABLE_DATA_INT = 2,
    JGTABLE_DATA_STR = 3
} jibalcsvreader_data_types;

typedef struct jibalcsvreader_colspec {
    size_t colnum;
    char *name;
    jibalcsvreader_data_types type;
} jibalcsvreader_colspec;

typedef struct jibalcsvreader_col {
    jibalcsvreader_colspec colspec;
    size_t offset; /* Used for binary output mode (unimplemented) */
    size_t size; /* Used for binary output mode (unimplemented) */
    char *strdata; /* Pointer to string data (somewhere in ((jibalcsvreader *)reader)->line) stored here temporarily. Don't free this! */
} jibalcsvreader_col;

typedef struct jibalcsvreader {
    jibalcsvreader_settings *settings;
    char *(*strsep)(char **stringp, const char *delim); /* Separator function pointer. */
    char *filename;
    FILE *f;
    char *line;
    size_t line_size;
    size_t lineno;
    size_t n_cols; /* number of columns given in colspec on init */
    jibalcsvreader_col *columns; /* array has n_cols + 1 elements. Plus 1 because we start numbering from 1. */
    size_t colmax; /* largest column number, i.e. how many columns to parse from input */
    size_t *colhits; /* array has colmax+1 elements */
    size_t colsize;
    jibalcsvreader_error error;
} jibalcsvreader;

jibalcsvreader *jibalcsvreader_init(const char *filename, const jibalcsvreader_settings *settings, const jibalcsvreader_colspec *colspec); /* settings may be a NULL pointer (automagics are used)*/
int jibalcsvreader_scan(jibalcsvreader *reader, ...);
void jibalcsvreader_close(jibalcsvreader *reader);
size_t jibalcsvreader_lineno(const jibalcsvreader *reader);
const char *jibalcsvreader_error_string(const jibalcsvreader *reader);
const char *jibalcsvreader_error_string_by_code(jibalcsvreader_error error);

/* Settings may be null (see above). These all return allocated settings, which can be freed automatically (see below) */
jibalcsvreader_settings *jibalcsvreader_settings_default();
jibalcsvreader_settings *jibalcsvreader_settings_csv();
jibalcsvreader_settings *jibalcsvreader_settings_tsv();
jibalcsvreader_settings *jibalcsvreader_settings_clone(const jibalcsvreader_settings *settings);
jibalcsvreader_settings *jibalcsvreader_settings_by_filename(const char *filename);
int jibalcsvreader_set_settings(jibalcsvreader *reader, jibalcsvreader_settings *settings); /* Assumes ownership of settings, they are freed on jibalcsvreader_close() */
void jibalcsvreader_settings_free(jibalcsvreader_settings *settings);

/* The rest are used internally */
jibalcsvreader *jibalcsvreader_allocate(size_t n_cols, size_t colmax);
jibalcsvreader_settings *jibalcsvreader_settings_allocate();

size_t jibalcsvreader_type_size(jibalcsvreader_data_types type);
const char *jibalcsvreader_type_name(jibalcsvreader_data_types type);
int jibalcsvreader_read_line(jibalcsvreader *reader); /* Reads one line from file to reader->line, skipping comments */
int jibalcsvreader_separate_line(jibalcsvreader *reader); /* Separates reader->line to column strdata pointers appropriately */
#endif // JIBAL_CSVREADER_H
