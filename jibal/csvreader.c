#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "jibal_units.h"
#include "jibal_generic.h"
#include "jibal_csvreader.h"

jibalcsvreader *jibalcsvreader_init(const char *filename, const jibalcsvreader_settings *settings, const jibalcsvreader_colspec *colspec) {
    if(!filename || !colspec) {
        return NULL;
    }
    FILE *f_in = fopen(filename, "r");
    if(!f_in) {
        return NULL;
    }
    size_t n_cols = 0, colmax = 0;
    for(const jibalcsvreader_colspec *c = colspec; c->type != JGTABLE_DATA_NONE; c++) {
        n_cols++;
        if(c->colnum > colmax) {
            colmax = c->colnum;
        }
    }
    jibalcsvreader *reader = jibalcsvreader_allocate(n_cols, colmax);
    if(!reader)
        return NULL;
    if(settings) { /* Determine settings automatically or make a deep copy */
        jibalcsvreader_set_settings(reader, jibalcsvreader_settings_clone(settings));
    } else {
        jibalcsvreader_set_settings(reader, jibalcsvreader_settings_by_filename(filename));
    }
    reader->f = f_in;
    reader->filename = strdup(filename);

    for(size_t i = 1; i <= reader->n_cols; i++) {
        jibalcsvreader_col *col = &(reader->columns[i]);
        col->colspec = colspec[i - 1]; /* -1 because +1 elsewhere */
        col->colspec.name = strdup(col->colspec.name);
        col->size = jibalcsvreader_type_size(col->colspec.type);
        col->offset += reader->colsize;
        reader->colsize += col->size;
        if(reader->colhits[col->colspec.colnum] != 0) {
            fprintf(stderr, "Multiply defined columns.\n");
        } else {
            reader->colhits[col->colspec.colnum] = i;
        }
#ifdef DEBUG
        fprintf(stderr, "Got col %zu, colnum %zu: type: %s, size: %zu, offset: %zu\n",
                i, col->colspec.colnum,
                jibalcsvreader_type_name(col->colspec.type),
                col->size, col->offset
                );
#endif
    }
#ifdef DEBUG
    fprintf(stderr, "CSV reader: %zu columns, total column size %zu bytes. Will parse %zu columns from input.\n",
            reader->n_cols, reader->colsize, reader->colmax);
    fprintf(stderr, "Column number lookup reader:\n input output\n");
    for(size_t i = 0; i <= reader->colmax; i++) {
        fprintf(stderr, "%6zu %6zu\n", i, reader->colhits[i]);
    }
#endif
    return reader;
}

jibalcsvreader *jibalcsvreader_allocate(size_t n_cols, size_t colmax) {
    jibalcsvreader *reader = calloc(1, sizeof(jibalcsvreader));
    if(!reader)
        return NULL;
    reader->n_cols = n_cols;
    reader->columns = calloc(n_cols + 1, sizeof(jibalcsvreader_col)); /* +1 because numbering starts from 1 */
    reader->colmax = colmax;
    reader->colhits = calloc(colmax + 1, sizeof(size_t));
    return reader;
}

jibalcsvreader_settings *jibalcsvreader_settings_allocate() {
    jibalcsvreader_settings *settings = calloc(1, sizeof(jibalcsvreader_settings));
    if(!settings) {
        return NULL;
    }
    return settings;
}

void jibalcsvreader_close(jibalcsvreader *reader) {
    if(!reader)
        return;
    fclose(reader->f);
    for(size_t i = 1; i <= reader->n_cols; i++) {
        free(reader->columns[i].colspec.name);
    }
    free(reader->columns);
    free(reader->colhits);
    free(reader->line);
    free(reader->filename);
    jibalcsvreader_settings_free(reader->settings);
    free(reader);
}

size_t jibalcsvreader_lineno(const jibalcsvreader *reader) {
    return reader->lineno;
}

const char *jibalcsvreader_error_string(const jibalcsvreader *reader) {
    if(!reader) {
        return "reader is null";
    }
    return jibalcsvreader_error_string_by_code(reader->error);
}

const char *jibalcsvreader_error_string_by_code(jibalcsvreader_error error) {
    switch(error) {
        case JIBALCSVREADER_SUCCESS:
            return "success";
        case JIBALCSVREADER_EOF:
            return "end-of-file";
        case JIBALCSVREADER_NOT_ENOUGH_COLUMNS:
            return "not enough columns";
        case JIBALCSVREADER_ERROR_GENERIC:
            return "generic error";
        default:
            return "unknown error code";
    }
}


int jibalcsvreader_read_line(jibalcsvreader *reader) {
    if(!reader)
        return JIBALCSVREADER_ERROR_GENERIC;
    while(getline(&reader->line, &reader->line_size, reader->f) > 0) {
        reader->lineno++;
        char *line = reader->line;
        line[strcspn(line, "\r\n")] = 0; /* Strips all kinds of newlines! */
        if(reader->settings->skip_empty_lines && *line == '\0') {
            continue;
        }
        if(reader->settings->comment_char && *line == reader->settings->comment_char) {
            continue;
        }
        return JIBALCSVREADER_SUCCESS;
    }
    return JIBALCSVREADER_EOF;
}

int jibalcsvreader_separate_line(jibalcsvreader *reader) {
    int ret = jibalcsvreader_read_line(reader);
    if(ret) {
        return ret;
    }
    char *line = reader->line;
    char *line_split = line, *col_str;
    size_t colnum = 0;

    while ((col_str = reader->strsep(&line_split, reader->settings->delim)) != NULL) {
        if(reader->settings->skip_multiple_separators && *col_str == '\0') {
            continue;
        }
        colnum++;
#ifdef NO_COLHITS_SPEEDUP
        for(size_t i = 1; i <= reader->n_cols; i++) {
                if(reader->columns[i].colspec.colnum == colnum) {
                    reader->columns[i].strdata = col_str;
                }
            }
#else
        reader->columns[reader->colhits[colnum]].strdata = col_str; /* We can look up the column (spec) number based on the column number as long as columns are defined only once */
#endif
        if(colnum >= reader->colmax) { /* Read enough columns */
            return JIBALCSVREADER_SUCCESS;
        }
    }
    if(colnum < reader->colmax) {
        return JIBALCSVREADER_NOT_ENOUGH_COLUMNS;
    }
    return JIBALCSVREADER_SUCCESS;
}

size_t jibalcsvreader_type_size(jibalcsvreader_data_types type) {
    switch(type) {
        case JGTABLE_DATA_NONE:
            return 0;
        case JGTABLE_DATA_DOUBLE:
            return sizeof(double);
        case JGTABLE_DATA_INT:
            return sizeof(int);
        case JGTABLE_DATA_STR:
            return sizeof(char *);
        default:
            return 0;
    }
}

const char *jibalcsvreader_type_name(jibalcsvreader_data_types type) {
    switch(type) {
        case JGTABLE_DATA_NONE:
            return "none";
        case JGTABLE_DATA_DOUBLE:
            return "double";
        case JGTABLE_DATA_INT:
            return "int";
        case JGTABLE_DATA_STR:
            return "string";
        default:
            return NULL;
    }
}

int jibalcsvreader_scan(jibalcsvreader *reader, ...) {
    va_list ap;
    va_start(ap, reader);
    int ret = jibalcsvreader_separate_line(reader);
    if(ret) {
        reader->error = ret;
        return ret;
        va_end(ap);
    }
    double d;
    double *d_out;
    int i;
    int *i_out;
    char *s_out;
    char *end;
    int n_success = 0;
    for(size_t i_col = 1; i_col <= reader->n_cols; i_col++) {
        jibalcsvreader_col *col = &(reader->columns[i_col]);
        switch(col->colspec.type) {
            case JGTABLE_DATA_DOUBLE:
                d = strtod(col->strdata, &end);
                if(*end != '\0') { /* Conversion was not complete */
                    break;
                }
                d_out = va_arg(ap, double *);
                *d_out = d;
                n_success++;
                break;
            case JGTABLE_DATA_INT:
                i = (int)strtol(col->strdata, &end, 10);
                if(*end != '\0') { /* Conversion was not complete */
                    break;
                }
                i_out = va_arg(ap, int *);
                *i_out = i;
                n_success++;
                break;
            case JGTABLE_DATA_STR:
                s_out = va_arg(ap, char *);
                strcpy(s_out, col->strdata); /* s_out must be large enough */
                n_success++;
            default:
                break;
        }
    }
    va_end(ap);
    return n_success;
}

jibalcsvreader_settings *jibalcsvreader_settings_default() {
    jibalcsvreader_settings *settings = jibalcsvreader_settings_allocate();
    if(!settings) {
        return NULL;
    }
    settings->delim = strdup(JIBALCSVREADER_DEFAULT_DELIM);
    settings->skip_empty_lines = TRUE;
    settings->skip_multiple_separators = TRUE;
    settings->parse_csv_style_quotes = FALSE;
    settings->comment_char = '#';
    return settings;
}

jibalcsvreader_settings *jibalcsvreader_settings_csv() {
    jibalcsvreader_settings *settings = jibalcsvreader_settings_allocate();
    if(!settings) {
        return NULL;
    }
    settings->delim = strdup(JIBALCSVREADER_CSV_DELIM);
    settings->skip_empty_lines = FALSE;
    settings->skip_multiple_separators = FALSE;
    settings->parse_csv_style_quotes = TRUE;
    settings->comment_char = 0;
    return settings;
}

jibalcsvreader_settings *jibalcsvreader_settings_tsv() {
    jibalcsvreader_settings *settings = jibalcsvreader_settings_allocate();
    if(!settings) {
        return NULL;
    }
    settings->delim = strdup(JIBALCSVREADER_TSV_DELIM);
    settings->skip_empty_lines = FALSE;
    settings->skip_multiple_separators = FALSE;
    settings->parse_csv_style_quotes = FALSE;
    settings->comment_char = '#';
    return settings;
}

jibalcsvreader_settings *jibalcsvreader_settings_clone(const jibalcsvreader_settings *settings) {
    jibalcsvreader_settings *out = jibalcsvreader_settings_allocate();
    *out = *settings;
    out->delim = strdup(out->delim);
    return out;
}

jibalcsvreader_settings *jibalcsvreader_settings_by_filename(const char *filename) {
    size_t l = strlen(filename);
    const size_t suffix_len = 4; /* including "." */
    if(l > suffix_len) {
        if(strncasecmp(filename + l - suffix_len, ".csv", 4) == 0) {
            return jibalcsvreader_settings_csv();
        } else if(strncasecmp(filename + l - suffix_len, ".tsv", 4) == 0) {
            return jibalcsvreader_settings_tsv();
        }
    }
    return jibalcsvreader_settings_default();
}

int jibalcsvreader_set_settings(jibalcsvreader *reader, jibalcsvreader_settings *settings) {
    if(!reader || !settings) {
        return JIBALCSVREADER_ERROR_GENERIC;
    }
    reader->settings = settings;
    if(settings->parse_csv_style_quotes) {
        reader->strsep = jibal_strsep_with_quotes;
    } else {
        reader->strsep = strsep;
    }
    return JIBALCSVREADER_SUCCESS;
}

void jibalcsvreader_settings_free(jibalcsvreader_settings *settings) {
    if(!settings)
        return;
    free(settings->delim);
}
