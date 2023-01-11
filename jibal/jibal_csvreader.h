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

typedef enum jibal_csvreader_error {
    JIBALCSVREADER_SUCCESS = (0),
    JIBALCSVREADER_ERROR_GENERIC = (-1),
    JIBALCSVREADER_EOF = (-2),
    JIBALCSVREADER_NOT_ENOUGH_COLUMNS = (-3)
} jibal_csvreader_error;

typedef struct jibal_csvreader_settings {
    int skip_empty_lines;
    int skip_multiple_separators;
    int parse_csv_style_quotes;
    char *delim;
    char comment_char;
} jibal_csvreader_settings;

typedef enum jibal_csvreader_data_types {
    JGTABLE_DATA_NONE = 0,
    JGTABLE_DATA_DOUBLE = 1,
    JGTABLE_DATA_INT = 2,
    JGTABLE_DATA_STR = 3
} jibal_csvreader_data_types;

typedef struct jibal_csvreader_colspec {
    size_t colnum;
    char *name;
    jibal_csvreader_data_types type;
} jibal_csvreader_colspec;

typedef struct jibal_csvreader_col {
    jibal_csvreader_colspec colspec;
    size_t offset; /* Used for binary output mode (unimplemented) */
    size_t size; /* Used for binary output mode (unimplemented) */
    char *strdata; /* Pointer to string data (somewhere in ((jibal_csvreader *)reader)->line) stored here temporarily. Don't free this! */
} jibal_csvreader_col;

typedef struct jibal_csvreader {
    jibal_csvreader_settings *settings;
    char *(*strsep)(char **stringp, const char *delim); /* Separator function pointer. */
    char *filename;
    FILE *f;
    char *line;
    size_t line_size;
    size_t lineno;
    size_t n_cols; /* number of columns given in colspec on init */
    jibal_csvreader_col *columns; /* array has n_cols + 1 elements. Plus 1 because we start numbering from 1. */
    size_t colmax; /* largest column number, i.e. how many columns to parse from input */
    size_t *colhits; /* array has colmax+1 elements */
    size_t colsize;
    jibal_csvreader_error error;
} jibal_csvreader;

jibal_csvreader *jibal_csvreader_init(const char *filename, const jibal_csvreader_settings *settings, const jibal_csvreader_colspec *colspec); /* settings may be a NULL pointer (automagics are used)*/
int jibal_csvreader_scan(jibal_csvreader *reader, ...);
void jibal_csvreader_close(jibal_csvreader *reader);
size_t jibal_csvreader_lineno(const jibal_csvreader *reader);
size_t jibal_csvreader_column_strlen(jibal_csvreader *reader, size_t i_col); /* Length of last read column string */
const char *jibal_csvreader_error_string(const jibal_csvreader *reader);
const char *jibal_csvreader_error_string_by_code(jibal_csvreader_error error);

/* Settings may be null (see above). These all return allocated settings, which can be freed automatically (see below) */
jibal_csvreader_settings *jibal_csvreader_settings_default();
jibal_csvreader_settings *jibal_csvreader_settings_csv();
jibal_csvreader_settings *jibal_csvreader_settings_tsv();
jibal_csvreader_settings *jibal_csvreader_settings_clone(const jibal_csvreader_settings *settings);
jibal_csvreader_settings *jibal_csvreader_settings_by_filename(const char *filename);
int jibal_csvreader_set_settings(jibal_csvreader *reader, jibal_csvreader_settings *settings); /* Assumes ownership of settings, they are freed on jibal_csvreader_close() */
void jibal_csvreader_settings_free(jibal_csvreader_settings *settings);

/* The rest are used internally */
jibal_csvreader *jibal_csvreader_allocate(size_t n_cols, size_t colmax);
jibal_csvreader_settings *jibal_csvreader_settings_allocate();

size_t jibal_csvreader_type_size(jibal_csvreader_data_types type);
const char *jibal_csvreader_type_name(jibal_csvreader_data_types type);
int jibal_csvreader_read_line(jibal_csvreader *reader); /* Reads one line from file to reader->line, skipping comments */
int jibal_csvreader_separate_line(jibal_csvreader *reader); /* Separates reader->line to column strdata pointers appropriately */
#endif // JIBAL_CSVREADER_H
