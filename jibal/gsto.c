#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <jibal.h>
#include <jibal_units.h>
#include <jibal_phys.h>
#include <jibal_gsto.h>
#include <defaults.h>
#ifdef WIN32
#include "win_compat.h"
#else
#include <libgen.h>
#endif
#include "jibal_stragg.h"

const char *gsto_get_header_string(const jibal_option *header, int val) {
    const jibal_option *h;
    for(h=header; h->s; h++) {
        if(h->val == val) {
            return h->s;
        }
    }
    return GSTO_STR_NONE;
}

int gsto_add_file(jibal_gsto *workspace, const char *name, const char *filename) {
    int success=0;
    if(!filename || !name) {
        fprintf(stderr, "Tried to add a file %s from %s. I need both a name and a filename.\n",
                name?name:"without a name",
                filename?filename:"unspecified location");
        return 0;
    }
#ifdef DEBUG
    fprintf(stderr, "Adding file %s (%s).\n", name, filename);
#endif
    workspace->files = realloc(workspace->files, sizeof(gsto_file_t) * (workspace->n_files + 1));
    gsto_file_t *new_file=&workspace->files[workspace->n_files];
    memset(new_file, 0, sizeof(gsto_file_t));
    new_file->valid = TRUE; /* Will be set to FALSE if there are errors later */
    new_file->name = strdup(name);
    new_file->filename = strdup(filename);
    success=jibal_gsto_load(workspace, TRUE, new_file);

    if(success) {
        workspace->n_files++;
    } else {
        fprintf(stderr, "Error in adding stopping file %s (%s).\n", name, filename);
        free(new_file->name);
        free(new_file->filename);
    }
    return success;
    
}

jibal_gsto *gsto_allocate(int Z1_max, int Z2_max) {
    jibal_gsto *workspace = malloc(sizeof(jibal_gsto));
    workspace->Z1_max=Z1_max;
    workspace->Z2_max=Z2_max;
    workspace->n_files=0;
    workspace->files=NULL; /* These will be allocated by gsto_new_file */
    workspace->n_comb=Z1_max*Z2_max; /* From 1..Z1_max inclusive and from 1..Z2_max inclusive */
    workspace->stop_assignments = calloc(workspace->n_comb, sizeof(gsto_file_t *));
    workspace->stragg_assignments = calloc(workspace->n_comb, sizeof(gsto_file_t *));
    workspace->overrides = NULL;
    return workspace;
}

const char *jibal_gsto_file_source(gsto_file_t *file) {
    return file->source;
}

void jibal_gsto_file_free(gsto_file_t *file) {
    int i;
    if(file->data) {
        for(i=0; i < file->n_comb; i++) {
            if(file->data[i]) {
                free(file->data[i]);
            }
        }
        free(file->data);
    }
    if(file->em) {
        free(file->em);
    }
    free(file->name);
    if(file->source) {
        free(file->source);
    }
    free(file->filename);
}

void jibal_gsto_free(jibal_gsto *workspace) {
    int i;
    if(workspace->files) {
        for (i = 0; i < workspace->n_files; i++) {
            gsto_file_t *file = &workspace->files[i];
            jibal_gsto_file_free(file);
        }
        free(workspace->files);
    }
    free(workspace->stop_assignments);
    free(workspace->stragg_assignments);
    if(workspace->overrides) {
        free(workspace->overrides);
    }
}

int jibal_gsto_file_has_combination(gsto_file_t *file, int Z1, int Z2) {
    if(file->Z1_min == JIBAL_ANY_Z && file->Z2_min == JIBAL_ANY_Z) {
        return 1;
    }
    if (file->Z1_min != JIBAL_ANY_Z && (Z1 < file->Z1_min || Z1 > file->Z1_max)) {
        return 0; /* Z1 out of range of the file */
    }
    if (file->Z2_min != JIBAL_ANY_Z && (Z2 < file->Z2_min || Z2 > file->Z2_max)) {
        return 0; /* Z2 out of range of the file */
    }
    return 1;
}

int jibal_gsto_assign(jibal_gsto *workspace, int Z1, int Z2, gsto_file_t *file) { /* Used internally, can be used after init to override autoinit */
    if(Z1 > workspace->Z1_max || Z2 > workspace->Z2_max) {
        return 0; /* Z1 or Z2 out of range of our workspace */
    }
    if(!jibal_gsto_file_has_combination(file, Z1, Z2)) {
        return 0;
    }
    int i=jibal_gsto_table_get_index(workspace, Z1, Z2);
    assert(i >= 0 && i <= workspace->n_comb);
    if(file->type == GSTO_STO_ELE) {
        workspace->stop_assignments[i] = file;
        return 1;
    }
    if(file->type == GSTO_STO_STRAGG) {
        workspace->stragg_assignments[i] = file;
        return 1;
    }
    return 0;
}

void jibal_gsto_assign_clear_all(jibal_gsto *workspace) {
    memset(workspace->stop_assignments, 0, workspace->n_comb*sizeof(gsto_file_t *));
    memset(workspace->stragg_assignments, 0, workspace->n_comb*sizeof(gsto_file_t *));
}

int jibal_gsto_assign_range(jibal_gsto *workspace, int Z1_min, int Z1_max, int Z2_min, int Z2_max, gsto_file_t
*file) {
    int Z1, Z2;
    for (Z1=Z1_min; Z1 <= Z1_max; Z1++) {
        for (Z2 = Z2_min; Z2 <= Z2_max; Z2++) {
            if(!jibal_gsto_assign(workspace, Z1, Z2, file)) {
                return 0;
            }
        }
    }
    return 1;
}

int jibal_gsto_load_binary_file(jibal_gsto *workspace, gsto_file_t *file) {
    int Z1, Z2;
#ifdef DEBUG
    fprintf(stderr, "Loading binary data.\n");
#endif
    for (Z1=file->Z1_min; Z1<=file->Z1_max; Z1++) {
        for (Z2=file->Z2_min; Z2<=file->Z2_max; Z2++) {
            if (file == jibal_gsto_get_assigned_file(workspace, file->type, Z1, Z2)) { /* OK */
                double *data = jibal_gsto_file_allocate_data(file, Z1, Z2);
                size_t n = fread(data, sizeof(double), file->xpoints, file->fp);
                if((int) n != file->xpoints) {
                    file->valid=FALSE;
                    return 0;
                }
            } else {
                if(fseek(file->fp, sizeof(double)*file->xpoints, SEEK_CUR)) {
                    file->valid=FALSE;
                    return 0;
                }
            }
        }
    }
    return 1;
}
void jibal_gsto_fprint_header_property(FILE *f, gsto_header_type h, int val) {
    const jibal_option *properties;
    switch(h) {
        case GSTO_HEADER_TYPE:
            properties=gsto_stopping_types;
            break;
        case GSTO_HEADER_STOUNIT:
            properties=gsto_sto_units;
            break;
        case GSTO_HEADER_STRAGGUNIT:
            properties=gsto_stragg_units;
            break;
        case GSTO_HEADER_XUNIT:
            properties=gsto_xunits;
            break;
        case GSTO_HEADER_FORMAT:
            properties=gsto_data_formats;
            break;
        case GSTO_HEADER_XSCALE:
            properties=gsto_xscales;
            break;
        default:
            properties=NULL;
            break;
    }
    if(!properties) {
        fprintf(stderr, "GSTO Warning: header type %i doesn't correspond to a header with properties\n", h);
        return;
    }
    if(val < jibal_option_n(properties))
        fprintf(f, "%s=%s\n", gsto_headers[h].s, properties[val].s);
    else
        fprintf(stderr, "Warning: can't print GSTO header. val=%i (should be < %i)\n",
                val, jibal_option_n(properties));
}
void jibal_gsto_fprint_header_int(FILE *f, gsto_header_type h, int i) {
    fprintf(f, "%s=%i\n", gsto_get_header_string(gsto_headers, h), i);
}
void jibal_gsto_fprint_header_string(FILE *f, gsto_header_type h, const char *str) {
    fprintf(f, "%s=%s\n", gsto_get_header_string(gsto_headers, h), str);
}

void jibal_gsto_fprint_header_scientific(FILE *f, gsto_header_type h, double val) {
    fprintf(f, "%s=%e\n", gsto_get_header_string(gsto_headers, h), val);
}

void jibal_gsto_fprint_header(FILE *f, gsto_header_type h, void *val) { /* Value is interpreted based on
 * header to be int, double or char *. In the last case void * is char **!. */
    char type;
    int n = jibal_option_n(gsto_headers);
    if((int) h >= n) {
        fprintf(stderr, "GSTO Error: %i is not v valid header type. This shouldn't happen.\n", h);
        return;
    }
    const char *header=gsto_headers[h].s;
    switch(h) {
        case GSTO_HEADER_NONE:
            type=0;
            break;
        case GSTO_HEADER_SOURCE:
            type='s';
            break;
        case GSTO_HEADER_XPOINTS:
        case GSTO_HEADER_Z1:
        case GSTO_HEADER_Z1MIN:
        case GSTO_HEADER_Z1MAX:
        case GSTO_HEADER_Z2:
        case GSTO_HEADER_Z2MIN:
        case GSTO_HEADER_Z2MAX:
            type='i';
            break;
        case GSTO_HEADER_TYPE:
        case GSTO_HEADER_STOUNIT:
        case GSTO_HEADER_STRAGGUNIT:
        case GSTO_HEADER_XUNIT:
        case GSTO_HEADER_FORMAT:
        case GSTO_HEADER_XSCALE:
            type='p';
            break;
        case GSTO_HEADER_XMIN:
        case GSTO_HEADER_XMAX:
            type='e';
            break;
        default:
            type=0;
            break;
    }
    switch(type) {
        case 'i':
            fprintf(f, "%s=%i\n", header, *((int *)val));
            break;
        case 'e':
            fprintf(f, "%s=%e\n", header, *((double *) val));
            break;
        case 'f': /* Note that this is still a double, not a float */
            fprintf(f, "%s=%lf\n", header, *((double *) val));
            break;
        case 's':
            fprintf(f, "%s=%s\n", header, *((char **) val));
            break;
        case 'p':
            jibal_gsto_fprint_header_property(f, h, *(int *)val);
            break;
        default:
            fprintf(stderr, "Unknown header type %c (%i)\n", type, type);
            break;
    }
}

void jibal_gsto_fprint_file(FILE *file_out, jibal_gsto *workspace, gsto_file_t *file, gsto_data_format format, int
Z1_min, int Z1_max, int Z2_min, int Z2_max) {
    int i, Z1, Z2;
    /* TODO: this is an absolute mess! We have to consider the Z1, Z2 range we are given, the Z1, Z2 range of the
     * file, and also the global Z1, Z2 range (from workspace), since only that data can ever be loaded!
     * To make it even more interesting the we also have the special value JIBAL_ANY_Z.
     * */
    if(file->Z1_min == JIBAL_ANY_Z) {
        Z1_min = Z1_max = JIBAL_ANY_Z;
    } else if(Z1_min == JIBAL_ANY_Z) {
        Z1_min = file->Z1_min;
        Z2_max = file->Z1_max;
    } else {
        Z1_min = Z1_min < file->Z1_min ? file->Z1_min : Z1_min;
        Z1_max = Z1_max > file->Z1_max ? file->Z1_max : Z1_max;
    }
    if(file->Z2_min == JIBAL_ANY_Z) {
        Z2_min = Z2_max = JIBAL_ANY_Z;
    } else if(Z2_min == JIBAL_ANY_Z) {
        Z2_min = file->Z2_min;
        Z2_max = file->Z2_max;
    } else {
        Z2_min = Z2_min < file->Z2_min ? file->Z2_min : Z2_min;
        Z2_max = Z2_max > file->Z2_max ? file->Z2_max : Z2_max;
    }

    jibal_gsto_fprint_header(file_out, GSTO_HEADER_TYPE, &file->type);
    if(file->source) {
        jibal_gsto_fprint_header(file_out, GSTO_HEADER_SOURCE, &file->source);
    }
    if(Z1_min == JIBAL_ANY_Z) {
        jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z1, &file->Z1_min);
    } else {
        if(Z1_min == Z1_max) {
            jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z1, &Z1_min);
        } else {
            jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z1MIN, &Z1_min);
            jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z1MAX, &Z1_max);
        }
    }
    if(Z2_min == JIBAL_ANY_Z) {
        jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z2, &file->Z2_min);
    } else {
        if(Z2_min == Z2_max) {
            jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z2, &Z2_min);
        } else {
            jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z2MIN, &Z2_min);
            jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z2MAX, &Z2_max);
        }
    }
    if(file->type == GSTO_STO_ELE) {
        jibal_gsto_fprint_header_property(file_out, GSTO_HEADER_STOUNIT,
                                          (format == GSTO_DF_ASCII) ? GSTO_STO_UNIT_EV15CM2 : GSTO_STO_UNIT_JM2);
    }
    if(file->type == GSTO_STO_STRAGG) {
        jibal_gsto_fprint_header_property(file_out, GSTO_HEADER_STRAGGUNIT,
                                          (format == GSTO_DF_ASCII) ? GSTO_STRAGG_UNIT_BOHR : GSTO_STRAGG_UNIT_J2M2);
    }
    /* We convert numbers if we are outputting ASCII, but binary stays as internal binary (SI units) */
    if(file->xscale == GSTO_XSCALE_ARBITRARY) { /* Arbitrary scales are converted */
        jibal_gsto_fprint_header_property(file_out,GSTO_HEADER_XUNIT, GSTO_X_UNIT_KEV_U);
        jibal_gsto_fprint_header_scientific(file_out,GSTO_HEADER_XMIN, file->em[0]/(C_KEV/C_U));
        jibal_gsto_fprint_header_scientific(file_out,GSTO_HEADER_XMAX, file->em[file->xpoints - 1]/(C_KEV/C_U));
        jibal_gsto_fprint_header(file_out,GSTO_HEADER_XPOINTS, &file->xpoints);
        jibal_gsto_fprint_header(file_out,GSTO_HEADER_XSCALE, &file->xscale);
    } else {
        jibal_gsto_fprint_header_property(file_out, GSTO_HEADER_XUNIT, file->xunit_original);
        jibal_gsto_fprint_header(file_out, GSTO_HEADER_XMIN, &file->xmin_original);
        jibal_gsto_fprint_header(file_out, GSTO_HEADER_XMAX, &file->xmax_original);
        jibal_gsto_fprint_header(file_out,GSTO_HEADER_XPOINTS, &file->xpoints);
        jibal_gsto_fprint_header(file_out,GSTO_HEADER_XSCALE, &file->xscale);
    }
    jibal_gsto_fprint_header(file_out, GSTO_HEADER_FORMAT, &format);
    fprintf(file_out, "\n");
    if(file->xscale == GSTO_XSCALE_ARBITRARY) {
        for(i=0; i < file->xpoints; i++) {
            fprintf(file_out, "%e\n", file->em[i]/(C_KEV/C_U));
        }
    }
    for (Z1=Z1_min; Z1 <= Z1_max  && Z1 <= workspace->Z1_max; Z1++) {
        for (Z2 = Z2_min; Z2 <= Z2_max && Z2 <= workspace->Z2_max; Z2++) {
            const double *data=jibal_gsto_file_get_data(file, Z1, Z2);
            if(!data) {
                fprintf(stderr, "Error: no data for Z1=%i and Z2=%i in %s.%s\n",
                        Z1, Z2, file->name, file->data?"":"This file isn't loaded yet.");
                return;
            }
            if(format == GSTO_DF_ASCII) {
                for (i = 0; i < file->xpoints; i++) {
                    double out = data[i];
                    switch (file->stounit) {
                        case GSTO_STO_UNIT_JM2:
                            out /=  C_EV_TFU; /* TODO: should output always be in these units? */
                            break;
                        case GSTO_STO_UNIT_EV15CM2:
                            break;
                        default:
                            break;
                    }
                    switch (file->straggunit) {
                        case GSTO_STRAGG_UNIT_J2M2:
                            out /= jibal_stragg_bohr(Z1, Z2);
                        default:
                            break;
                    }
                    fprintf(file_out, "%e\n", out);
                }
            } else if(format == GSTO_DF_DOUBLE) {
                fwrite(data, sizeof(double), file->xpoints, file_out);
            }
        }
    }
}

int jibal_gsto_table_get_index(jibal_gsto *workspace, int Z1, int Z2) {
    if(Z1 < 1 || Z1 > workspace->Z1_max || Z2 < 1 || Z2 > workspace->Z2_max) {
        return -1;
    }
    int z1 = (Z1 - 1);
    int z2 = (Z2 - 1);
    int i = (workspace->Z2_max * z1 + z2);
    assert(i >= 0 && i < workspace->n_comb);
    return i;
}

int jibal_gsto_file_get_data_index(gsto_file_t *file, int Z1, int Z2) {
    if(file->Z1_min == JIBAL_ANY_Z) {
        if(file->Z2_min == JIBAL_ANY_Z) {
            return 0; /* Only one index is possible Z1=any, Z2=any */
        } else {
            assert(Z2 <= file->Z2_max);
            assert(Z2 >= file->Z2_min);
            return Z2-file->Z2_min;
        }
    }
    if(file->Z2_min == JIBAL_ANY_Z) {
        assert(Z1 <= file->Z1_max);
        assert(Z1 >= file->Z1_min);
        return Z1-file->Z1_min;
    }
    assert(Z1 >= file->Z1_min);
    assert(Z2 >= file->Z2_min);
    assert(Z1 <= file->Z1_max);
    assert(Z2 <= file->Z2_max);
    int z1=(Z1 - file->Z1_min);
    int z2=(Z2 - file->Z2_min);
    int n_z2=(file->Z2_max-file->Z2_min+1);
    int i=(n_z2*z1+z2);
    assert(i < file->n_comb);
    return i;
}

const double *jibal_gsto_file_get_data(gsto_file_t *file, int Z1, int Z2) {
    if(!file->data)
        return NULL;
    return file->data[jibal_gsto_file_get_data_index(file, Z1, Z2)];
}

double *jibal_gsto_file_allocate_data(gsto_file_t *file, int Z1, int Z2) {
    int i=jibal_gsto_file_get_data_index(file, Z1, Z2);
    assert(i >= 0 && i < file->n_comb);
    if(file->data[i]) {
        free(file->data[i]);
    }
    file->data[i] = calloc(file->xpoints, sizeof(double));
    return file->data[i];
}

int jibal_gsto_load_ascii_file(jibal_gsto *workspace, gsto_file_t *file) {
    int Z1, Z2, previous_Z1=file->Z1_min, previous_Z2=file->Z2_min-1, skip, i;
    char *line = calloc(GSTO_DATAFILES_MAX_LINE_LEN, sizeof(char));
    int actually_skipped=0;
#ifdef DEBUG
    fprintf(stderr, "Loading ascii data.\n");
#endif

    for (Z1 = file->Z1_min; Z1 <= file->Z1_max && Z1 <= workspace->Z1_max; Z1++) {
        for (Z2 = file->Z2_min; Z2 <= file->Z2_max && Z2 <= workspace->Z2_max; Z2++) {
            if (file == jibal_gsto_get_assigned_file(workspace, file->type, Z1, Z2) || Z1 == JIBAL_ANY_Z || Z2 == JIBAL_ANY_Z) {
                /* This file is assigned to this Z1, Z2 combination, so we have to load the stopping in.
                 * EXCEPTION: if either Z1 or Z2 is "ANY_Z" we ignore this whole skipping thing and just always load
                 * in everything. */
                if (Z1 == JIBAL_ANY_Z || Z2 == JIBAL_ANY_Z) {
                    skip = 0;
                } else {
                    skip = file->xpoints * ((Z1 - previous_Z1) * (file->Z2_max - file->Z2_min + 1) +
                                            (Z2 - previous_Z2 - 1)); /* Not sure if correct, but it works. */
                }
#ifdef DEBUG
                fprintf(stderr, "Skipping %i*(%i*%i+%i)=%i lines.\n", file->xpoints, Z1-previous_Z1, file->Z1_max-file->Z2_min+1, Z2-previous_Z2-1, skip);
                actually_skipped=0;
#endif

                while (skip--) {
                    file->lineno++;
                    if(!fgets(line, GSTO_DATAFILES_MAX_LINE_LEN, file->fp))
                        break;
                    
                    if(*line == '#') {
                        skip++; /* Undoing skip-- */
#ifdef DEBUG
                        fprintf(stderr, "Comment on line %i: %s", file->lineno, line+1);
#endif
                    } 
                    actually_skipped++;
                }
#ifdef DEBUG
                fprintf(stderr, "actually skipped %i lines\n", actually_skipped);
#endif
                double *data = jibal_gsto_file_allocate_data(file, Z1, Z2);
                for(i=0; i<file->xpoints; i++) {
                    if(!fgets(line, GSTO_DATAFILES_MAX_LINE_LEN, file->fp)) {
                        fprintf(stderr, "ERROR: File %s ended prematurely when reading Z1=%i Z2=%i stopping point=%i/%i"
                                        ".\n", file->filename, Z1, Z2, i+1, file->xpoints);
                        file->valid = FALSE;
                        break;
                    }
                    file->lineno++;
                    if(*line == '#') { /* This line is a comment. Ignore. */
                        i--;
                    } else {
#ifdef DEBUG
                        fprintf(stderr, "Loaded stopping [%i][%i][%i] from line %i.\n", Z1, Z2, i, file->lineno);
#endif
                        data[i] = strtod(line, NULL);
                    }
                }
                previous_Z1=Z1;
                previous_Z2=Z2;
                
            }
        }
    }
    free(line);
    return 1;
}

void jibal_gsto_file_calculate_ncombs(gsto_file_t *file) {
    if(file->Z1_min == JIBAL_ANY_Z) {
        file->Z1_max = JIBAL_ANY_Z; /* For sanity, we can't have any Z1 but with a maximum. Then we should also have
 * a clearly defined minimum. */
        if(file->Z2_min == JIBAL_ANY_Z) {
            file->Z2_max = JIBAL_ANY_Z;
            file->n_comb = 1; /* Any Z1, Z2, so maybe just one? */
            return;
        }
        file->n_comb = (file->Z2_max - file->Z2_min + 1); /* Any Z1, some Z2 range */
        return;
    }
    if(file->Z2_min == JIBAL_ANY_Z) {
        file->Z2_max = JIBAL_ANY_Z;
        file->n_comb = (file->Z1_max - file->Z1_min + 1); /* Any Z2, some Z1 range */
        return;
    }
    file->n_comb = (file->Z1_max - file->Z1_min + 1) * (file->Z2_max - file->Z2_min + 1); /* Z1 and Z2 range */
}

int jibal_gsto_load(jibal_gsto *workspace, int headers_only, gsto_file_t *file) {
    char *line;
    char *line_split;
    char *columns[3];
    char **col;
    int n_errors=0;
    if (!file) {
        return 0;
    }
    file->fp = fopen(file->filename, "rb");
    if (!file->fp) {
        fprintf(stderr, "Could not open file \"%s\".\n", file->filename);
        file->valid = FALSE;
        return 0;
    }
    file->lineno = 0;
    line = calloc(GSTO_DATAFILES_MAX_LINE_LEN, sizeof(char));
    /* parse headers, stop when end of headers found */
    while (fgets(line, GSTO_DATAFILES_MAX_LINE_LEN, file->fp) != NULL && file->valid) {
        file->lineno++;
        if(*line == '#') /* Comments are allowed */
            continue;
        if(*line == '\r' || *line == '\n')
            break;
        line_split = line;
        for (col = columns; (*col = strsep(&line_split, "=\n\r\t")) != NULL;)
            if (**col != '\0')
                if (++col >= &columns[3])
                    break;
        switch (jibal_option_get_value(gsto_headers, columns[0])) {
            case GSTO_HEADER_TYPE:
                file->type = jibal_option_get_value(gsto_stopping_types, columns[1]);
                break;
            case GSTO_HEADER_SOURCE:
                file->source = strdup(columns[1]);
                break;
            case GSTO_HEADER_FORMAT:
                file->data_format = jibal_option_get_value(gsto_data_formats, columns[1]);
                break;
            case GSTO_HEADER_STOUNIT:
                file->stounit = jibal_option_get_value(gsto_sto_units, columns[1]);
                file->stounit_original = file->stounit;
                break;
            case GSTO_HEADER_STRAGGUNIT:
                file->straggunit = jibal_option_get_value(gsto_stragg_units, columns[1]);
                file->straggunit_original = file->straggunit;
                break;
            case GSTO_HEADER_XSCALE:
                file->xscale = jibal_option_get_value(gsto_xscales, columns[1]);
                break;
            case GSTO_HEADER_XUNIT:
                file->xunit = jibal_option_get_value(gsto_xunits, columns[1]);
                file->xunit_original = file->xunit;
                break;
            case GSTO_HEADER_XPOINTS:
                file->xpoints = strtol(columns[1], NULL, 10);
                break;
            case GSTO_HEADER_XMIN:
                file->xmin = strtod(columns[1], NULL);
                file->xmin_original = file->xmin;
                break;
            case GSTO_HEADER_XMAX:
                file->xmax = strtod(columns[1], NULL);
                file->xmax_original = file->xmax;
                break;
            case GSTO_HEADER_Z1:
               // element = jibal_element_find(workspace->elements, columns[1]);
                file->Z1_min = strtol(columns[1], NULL, 10);
                file->Z1_max = file->Z1_min;
                break;
            case GSTO_HEADER_Z1MIN:
                file->Z1_min = strtol(columns[1], NULL, 10);
                break;
            case GSTO_HEADER_Z1MAX:
                file->Z1_max = strtol(columns[1], NULL, 10);
                break;
            case GSTO_HEADER_Z2:
                file->Z2_min = strtol(columns[1], NULL, 10);
                file->Z2_max = file->Z2_min;
                break;
            case GSTO_HEADER_Z2MIN:
                file->Z2_min = strtol(columns[1], NULL, 10);
                break;
            case GSTO_HEADER_Z2MAX:
                file->Z2_max = strtol(columns[1], NULL, 10);
                break;
            case GSTO_HEADER_NONE:
            default:
                fprintf(stderr, "WARNING: unknown header \"%s\" on line %i of file %s.\n", columns[0],
                        file->lineno,
                        file->filename);
                n_errors++;
                break;
        }
        if(n_errors >= 10) { /* This prevents us from flooding the output with error messages if the headers are not
 * properly terminated */
            fprintf(stderr, "WARNING: too many errors, considering the file invalid.\n");
            file->valid = FALSE;
            break;
        }
    } /* End of headers */
    free(line);
    if(file->type != GSTO_STO_ELE && file->type != GSTO_STO_STRAGG) {
        fprintf(stderr, "WARNING: GSTO files of type %s are not supported. Missing/wrong type header in file %s?\n",
                gsto_get_header_string(gsto_stopping_types, file->type), file->name);
        file->valid = FALSE;
    }
    if((file->straggunit && file->stounit) || (!file->stounit && !file->straggunit)) {
        fprintf(stderr, "WARNING: You can must specify exactly one stopping or straggling unit. Issue with file %s.\n", file->name);
        file->valid = FALSE;
    }

    jibal_gsto_file_calculate_ncombs(file);
    if(headers_only || file->valid == FALSE) {
        fclose(file->fp);
        return file->valid;
    }
    if(file->data) {
        free(file->data);
    }
    file->data = calloc(file->n_comb, sizeof(double *));
    if(file->em) {
        free(file->em);
    }
    file->em = jibal_gsto_em_table(file);
    if(!file->em) {
        fprintf(stderr, "WARNING: Could not make E/m table for file %s\n", file->name);
        file->valid = FALSE;
        fclose(file->fp);
        return file->valid;
    }
    switch (file->data_format) {
        case GSTO_DF_DOUBLE:
            jibal_gsto_load_binary_file(workspace, file);
            break;
        case GSTO_DF_ASCII:
        default:
            jibal_gsto_load_ascii_file(workspace, file);
            break;
    }
    fclose(file->fp);
#ifndef GSTO_DONT_CONVERT_TO_SI
    jibal_gsto_convert_file_to_SI(file);
#endif
    jibal_gsto_calculate_speedups(file);
    return 1;
}

void jibal_gsto_calculate_speedups(gsto_file_t *file) {
    file->xmin_speedup = 0.0;
    file->xdiv = 0.0;
    file->em_index_accel = 0;
    switch (file->xscale) {
        case GSTO_XSCALE_LINEAR:
            file->xmin_speedup = file->xmin;
            file->xdiv = (file->xpoints - 1) / (file->xmax - file->xmin);
            break;
        case GSTO_XSCALE_LOG10:
            file->xmin_speedup = log10(file->xmin);
            file->xdiv = (file->xpoints - 1) / (log10(file->xmax) - log10(file->xmin));
            break;
        case GSTO_XSCALE_ARBITRARY:
        default:
            break;
    }
}

void jibal_gsto_convert_file_to_SI(gsto_file_t *file) {
    /* We convert energy per mass units to SI, this will possibly reduce conversions later.
     *
     * Note that we only recalculate xmin and xmax, since other variables are internally in SI.
     *
     * Stopping & straggling data is also converted, if necessary and/or possible
     *
     * xunit, stounit and straggunit will reflect the units after conversion
     *
     * */
    if(file->xunit == GSTO_X_UNIT_MEV_U) {
        file->xmin *= C_MEV/C_U;
        file->xmax *= C_MEV/C_U;
        file->xunit = GSTO_X_UNIT_J_KG;
    } else if(file->xunit == GSTO_X_UNIT_KEV_U) {
        file->xmin *= C_KEV/C_U;
        file->xmax *= C_KEV/C_U;
        file->xunit = GSTO_X_UNIT_J_KG;
    }

    if(file->stounit == GSTO_STO_UNIT_EV15CM2) {
        int i_comb;
        for (i_comb = 0; i_comb < file->n_comb; i_comb++) {
            double *data = file->data[i_comb];
            if (data) {
                int i;
                for (i = 0; i < file->xpoints; i++) {
                    data[i] *= C_EV_TFU;
                }
            }
        }
        file->stounit = GSTO_STO_UNIT_JM2;
    }

    if(file->straggunit == GSTO_STRAGG_UNIT_BOHR && file->Z1_min != JIBAL_ANY_Z && file->Z2_min != JIBAL_ANY_Z) {
        /* We can't convert if Z1 and Z2 and ambiguous. Also the following algorithm doesn't check for overlaps and would
         * convert some data twice. */
        int Z1, Z2;
        for (Z1=file->Z1_min; Z1 <= file->Z1_max; Z1++) {
            for (Z2 = file->Z2_min; Z2 <= file->Z2_max; Z2++) {
                int i_comb=jibal_gsto_file_get_data_index(file, Z1, Z2);
                double *data = file->data[i_comb];
                if (data) {
                    int i;
                    for (i = 0; i < file->xpoints; i++) {
                        data[i] *= jibal_stragg_bohr(Z1, Z2);
                    }
                }
            }
        }
        file->straggunit = GSTO_STRAGG_UNIT_J2M2;
    }

}

int jibal_gsto_load_all(jibal_gsto *workspace) { /* For every file, load combinations from file */
    int i;
    int n_success=0;
    for(i=0; i < workspace->n_files; i++) {
        gsto_file_t *file = &workspace->files[i];
        int assignments = jibal_gsto_file_count_assignments(workspace, file);
        if(!assignments)
            continue;
        n_success += jibal_gsto_load(workspace, FALSE, file);
    }
    return n_success;
}

int jibal_gsto_file_count_assignments(jibal_gsto *workspace, gsto_file_t *file) {
    int Z1, Z2;
    int assignments=0;
    for (Z1=1; Z1 <= workspace->Z1_max; Z1++) {
        for (Z2=1; Z2 <= workspace->Z2_max; Z2++) {
            if(jibal_gsto_get_assigned_file(workspace, file->type, Z1, Z2) == file) {
                assignments++;
            }
        }
    }
    return assignments;
}

int jibal_gsto_print_files(jibal_gsto *workspace, int used_only) {
    int i;
    int assignments;
    gsto_file_t *file;
    if(workspace->n_files == 0) {
        fprintf(stderr, "There are no stopping or straggling files installed. Maybe check your configuration or download some data?\n");
        return -1;
    }
    fprintf(stderr, "List of %s GSTO files:\n", used_only?"used":"available");
    
    for(i=0; i < workspace->n_files; i++) {
        file=&workspace->files[i];
        assignments=jibal_gsto_file_count_assignments(workspace, file);
        if(used_only && !assignments) {
            continue;
        }
        fprintf(stderr, "%i: %s,\n", i+1, file->name);
        fprintf(stderr, "\ttype: %s\n", gsto_get_header_string(gsto_stopping_types, file->type));
        fprintf(stderr, "\tfilename: %s\n", file->filename);
        if(file->source) {
            fprintf(stderr, "\tdata source: %s\n", file->source);
        }
        if(assignments) {
            fprintf(stderr, "\t%i assignments,\n", assignments);
        }
        if(file->Z1_min == file->Z1_max) {
            fprintf(stderr, "\tZ1 = %i (%s)\n", file->Z1_min, jibal_element_name(workspace->elements, file->Z1_min));
        } else {
            fprintf(stderr, "\t%i (%s) <= Z1 <= %i (%s),\n", file->Z1_min, jibal_element_name(workspace->elements, file->Z1_min),
                    file->Z1_max, jibal_element_name(workspace->elements, file->Z1_max));
        }
        if(file->Z2_min == file->Z2_max) {
            fprintf(stderr, "\tZ2 = %i (%s)\n", file->Z2_min, jibal_element_name(workspace->elements, file->Z2_min));
        } else {
            fprintf(stderr, "\t%i (%s) <= Z2 <= %i (%s),\n", file->Z2_min, jibal_element_name(workspace->elements, file->Z2_min),
                    file->Z2_max, jibal_element_name(workspace->elements, file->Z2_max));
        }
        fprintf(stderr, "\tx-points=%i\n", file->xpoints);
        fprintf(stderr, "\tx-scale=%s\n", gsto_get_header_string(gsto_xscales, file->xscale));
        if(file->xunit == file->xunit_original) {
            fprintf(stderr, "\tx-unit=%s\n", gsto_get_header_string(gsto_xunits, file->xunit_original));
        } else {
            fprintf(stderr, "\tx-unit=%s (converted to %s)\n", gsto_get_header_string(gsto_xunits, file->xunit_original),
                    gsto_get_header_string(gsto_xunits, file->xunit));

        }
        fprintf(stderr, "\tx-min=%e\n", file->xmin);
        fprintf(stderr, "\tx-max=%e\n", file->xmax);
        if(file->stounit != GSTO_STO_UNIT_NONE) {
            if (file->stounit == file->stounit_original) {
                fprintf(stderr, "\tstopping unit=%s\n", gsto_get_header_string(gsto_sto_units, file->stounit_original));
            } else {
                fprintf(stderr, "\tstopping unit=%s (converted to %s)\n",
                        gsto_get_header_string(gsto_sto_units, file->stounit_original),
                        gsto_get_header_string(gsto_sto_units, file->stounit));
            }
        }
        if(file->straggunit != GSTO_STRAGG_UNIT_NONE) {
            if (file->straggunit == file->straggunit_original) {
                fprintf(stderr, "\tstraggling unit=%s\n", gsto_get_header_string(gsto_stragg_units,
                        file->straggunit_original));
            } else {
                fprintf(stderr, "\tstraggling unit=%s (converted to %s)\n",
                        gsto_get_header_string(gsto_stragg_units, file->straggunit_original),
                        gsto_get_header_string(gsto_stragg_units, file->straggunit));
            }
        }
        fprintf(stderr, "\tformat=%s\n", gsto_get_header_string(gsto_data_formats, file->data_format));
    }
    return 0;
}

int jibal_gsto_print_assignments(jibal_gsto *workspace) {
    int Z1, Z2;
    fprintf(stderr, "\nList of assigned stopping and straggling files:\n");
    for (Z1=1; Z1 <= workspace->Z1_max; Z1++) {
        for (Z2=1; Z2 <= workspace->Z2_max; Z2++) {
            gsto_file_t *file_sto = jibal_gsto_get_assigned_file(workspace, GSTO_STO_ELE, Z1, Z2);
            gsto_file_t *file_stg = jibal_gsto_get_assigned_file(workspace, GSTO_STO_STRAGG, Z1, Z2);
            if(!file_sto && !file_stg)
                continue; /* Nothing to do, nothing assigned */
            fprintf(stderr, "  Z1=%i (%s), Z2=%i (%s): ", Z1, jibal_element_name(workspace->elements, Z1),
                    Z2, jibal_element_name(workspace->elements, Z2));
            if(file_sto) {
                fprintf(stderr, "Stopping file %s.", file_sto->name);
            }
            if(file_stg) {
                fprintf(stderr, "%sStraggling file %s.", file_sto?" ":"", file_stg->name);
            }
            fprintf(stderr, "\n");
        }        
    }
    fprintf(stderr, "\n");
    return 1;
}

int jibal_gsto_auto_assign(jibal_gsto *workspace, int Z1, int Z2) {
    gsto_file_t *file;
    int success=0, i;
    if(workspace->overrides) {
        gsto_assignment *a;
        for (a = workspace->overrides; a->file != NULL; a++) {
            if(a->Z1 == Z1 && a->Z2 == Z2) {
                if(jibal_gsto_assign(workspace, Z1, Z2, a->file)) {
                    success=1;
                } else {
                    fprintf(stderr, "Warning: could not assign Z1=%i, Z2=%i to file %s\n", Z1, Z2, a->file->name);
                }
            }
        }
    }
    for (i=0; i < workspace->n_files; i++) {
        file=&workspace->files[i];
        if(jibal_gsto_file_has_combination(file, Z1, Z2)) {
            if(!jibal_gsto_get_assigned_file(workspace, file->type, Z1, Z2)) {
                if(jibal_gsto_assign(workspace, Z1, Z2, file)) {
                    success = 1;
                }
            } else {
                success=1; /* I guess success by someone else (previous call?) is success too. */
            }
        }
    }
    return success;
}

jibal_gsto *jibal_gsto_init(const jibal_element *elements, int Z_max, const char *files_file_name,
                            const char *assignments_file_name) {
    jibal_gsto *workspace;
    workspace = gsto_allocate(Z_max, Z_max);
    workspace->elements = elements;
    if(jibal_elements_Zmax(elements) < Z_max) {
        fprintf(stderr, "WARNING: Z_max=%i is too large, I don't know this many elements.\n", Z_max);
    }
    workspace->stop_step = JIBAL_STEP_SIZE; /* TODO: set this from some configuration. Used only for layer energy
 * loss calculations */
    workspace->extrapolate = FALSE;
    jibal_gsto_read_settings_file(workspace, files_file_name);
    workspace->overrides = jibal_gsto_read_assignments_file(workspace, assignments_file_name);
    return workspace;
}

int jibal_gsto_read_settings_file(jibal_gsto *workspace, const char *filename) {
    if(!filename) {
        fprintf(stderr, "WARNING: GSTO files configuration not found. Maybe you don't have %s anywhere?\n", JIBAL_FILES_FILE);
        return 0;
    }
    FILE *f=fopen(filename, "r");
    if(!f) {
        fprintf(stderr, "WARNING: Can not open file \"%s\"\n", filename);
        return 0;
    }
    char *line=malloc(sizeof(char)*GSTO_METADATA_MAX_LINE_LEN);
    char *line_split;
    char *columns[2];
    char **col;
    int lineno=0, n_files=0, n_errors=0;
    while (fgets(line, GSTO_METADATA_MAX_LINE_LEN, f) != NULL) {
        lineno++;
        if (line[0] == '#') /* Strip comments */
            continue;
        line_split = line; /* strsep will screw up line_split, reset for every new line */
        columns[0] = NULL;
        columns[1] = NULL;
        /* We read in two columns (hard coded), first is the name and the second is the filename. */
        for (col = columns; (*col = strsep(&line_split, ",\t\r\n")) != NULL;) {
            if (**col != '\0')
                if (++col >= &columns[2])
                    break;
        }
        char *name=columns[0];
        char *file=columns[1];
        if(name && file ) {
            if(!jibal_path_is_absolute(file)) {
                char *datadir=strdup(filename); /* dirname below may modify the string, therefore strdup */
                char *tmp=strdup(dirname(datadir)); /* dirname might return pointer to somewhere in "datadir" or static memory, therefore another strdup */
                free(datadir); /* This might contain something mutilated by dirname, lets free it */
                datadir=tmp;
                file = calloc(strlen(datadir) + 1 + strlen(file) + 1, sizeof(char));
                strcat(file, datadir);
                strcat(file, "/");
                strcat(file, columns[1]);
                file = jibal_path_cleanup(file);
            }
            if(gsto_add_file(workspace, name, file)) {
                n_files++;
            } else {
                fprintf(stderr, "WARNING: adding file %s failed.\n", file);
                n_errors++;
            }
        } else {
            fprintf(stderr, "WARNING: adding stopping file failed, since line %i in %s is malformed.\n", lineno, file);
            n_errors++;
        }
        if(file != columns[1] && file) { /* Free filename if it was allocated */
            free(file);
        }
    }
#ifdef DEBUG
    fprintf(stderr, "GSTO: Read %i lines from settings file, added %i files, attempt to add %i files failed.\n", lineno, n_files, n_errors);
#endif
    fclose(f);
    return n_files;
}

gsto_assignment *jibal_gsto_read_assignments_file(jibal_gsto *workspace, const char *filename) {
    if(!filename) {
        fprintf(stderr, "WARNING: No assignments filename given. Maybe you don't have %s anywhere? Create an empty file to suppress this warning.\n", JIBAL_ASSIGNMENTS_FILE);
        return NULL;
    }
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "WARNING: Can not open file %s. Create an empty file to suppress this warning.\n", filename);
        return NULL;
    }
    char *line = malloc(sizeof(char) * GSTO_METADATA_MAX_LINE_LEN);
    char *line_split;
    char *columns[3];
    char **col;
    int lineno = 0, n_possible = 0, n_actual = 0;
    while (fgets(line, GSTO_METADATA_MAX_LINE_LEN, f) != NULL) { if (line[0] != '#') { n_possible++; }}
    if (n_possible == 0) {
        return NULL;
    }
    rewind(f);
    gsto_assignment *a = calloc(n_possible, sizeof(gsto_assignment));
    while (fgets(line, GSTO_METADATA_MAX_LINE_LEN, f) != NULL) {
        lineno++;
        if (line[0] == '#') /* Strip comments */
            continue;
        line_split = line; /* strsep will screw up line_split, reset for every new line */
        /* We read in two columns (hard coded), first is the name and the second is the filename. */
        for (col = columns; col < &columns[3]; col++) { *col = NULL; }
        for (col = columns; (*col = strsep(&line_split, ",\t\r\n")) != NULL;) {
            if (**col != '\0')
                if (++col >= &columns[3])
                    break;
        }
        const jibal_element *incident = jibal_element_find(workspace->elements, columns[0]);
        if (!incident) {
            fprintf(stderr, "WARNING: No such element: %s in file %s", columns[0], filename);
            continue;
        }
        const jibal_element *target = jibal_element_find(workspace->elements, columns[1]);
        if (!target) {
            fprintf(stderr, "WARNING: No such element: %s in file %s", columns[1], filename);
            continue;
        }
        gsto_file_t *file = jibal_gsto_get_file(workspace, columns[2]);
        if (!file) {
            fprintf(stderr, "WARNING: No such GSTO file: %s in file %s", columns[2], filename);
            continue;
        }
        gsto_assignment *ass = &a[n_actual];
        ass->Z1 = incident->Z;
        ass->Z2 = target->Z;
        ass->file = file;
        n_actual++;
    }
    free(line);
    return a; /* Remember to free this */
}


double jibal_gsto_stop_nuclear_universal(double E, int Z1, double m1, int Z2, double m2) {
    double a_u=0.8854*C_BOHR_RADIUS/(pow(Z1, 0.23)+pow(Z2, 0.23));
    double gamma = 4.0*m1*m2/pow(m1+m2, 2.0);
    double epsilon=(E/C_KEV)*32.53*m2/(Z1*Z2*(m1+m2)*(pow(Z1, 0.23)+pow(Z2, 0.23)));
    double S_ne;
#ifdef DEBUG
    fprintf(stderr, "Nuclear stopping of Z2=%i (m2=%g) for Z1=%i (m2=%g). a_u=%g, gamma=%g, epsilon=%g\n", Z2, m2, Z1, m2, a_u, gamma, epsilon);
#endif
    if(epsilon <= 30.0) {
         S_ne=log(1+1.1383*epsilon)/(2*(epsilon+0.01321*pow(epsilon, 0.21226)+0.19593*pow(epsilon, 0.5)));
    } else {
         S_ne=log(epsilon)/(2*epsilon);
    }
    double S=S_ne*C_PI*pow(a_u, 2.0)*gamma*E/epsilon;
#ifdef DEBUG
    fprintf(stderr, "Reduced S_ne=%g, S=%g (eV/tfu)\n", S_ne, S/C_EV_TFU);
#endif
    return S;
}

gsto_file_t *jibal_gsto_get_assigned_file(jibal_gsto *workspace, gsto_stopping_type type, int Z1, int Z2) {
    int i = jibal_gsto_table_get_index(workspace, Z1, Z2);
    if(i < 0)
        return NULL;
    if(type == GSTO_STO_ELE)
        return workspace->stop_assignments[i];
    if(type == GSTO_STO_STRAGG)
        return workspace->stragg_assignments[i];
    return NULL;
}

gsto_file_t *jibal_gsto_get_file(jibal_gsto *workspace, const char *name) {
    int i;
    gsto_file_t *file;
    for(i=0; i < workspace->n_files; i++) {
        file = &workspace->files[i];
        if(strcmp(file->name, name) == 0) {
            return file;
        }
    }
    return NULL;
}

double jibal_gsto_em_from_file_units(double x, const gsto_file_t *file) {
    double em;
    switch (file->xunit) {
        case GSTO_X_UNIT_MEV_U:
            em=x*(C_MEV/C_U);
            break;
        case GSTO_X_UNIT_KEV_U:
            em=x*(C_KEV/C_U);
            break;
        case GSTO_X_UNIT_M_S:
            em=energy_per_mass(x);
            break;
        default:
            em=0.0;
            break;
    }
    return em;
}

double *jibal_gsto_em_table(const gsto_file_t *file) { /* Note: for internal use only, may read the file->fp */
    double *table = malloc(sizeof(double) * file->xpoints);
    int i;
    double x;
#ifdef DEBUG
    fprintf(stderr, "Making velocity table, %i points, xmin=%g, xmax=%g, xscale=%i, xunit=%i\n", file->xpoints,
            file->xmin, file->xmax, file->xscale, file->xunit);
#endif
    for (i = 0; i < file->xpoints; i++) {
        switch (file->xscale) {
            case GSTO_XSCALE_LOG10:
                x = file->xmin * pow(file->xmax / file->xmin, 1.0 * i / (file->xpoints - 1));
                break;
            case GSTO_XSCALE_LINEAR:
                x = file->xmin + (file->xmax - file->xmin) * (1.0 * i / (file->xpoints - 1));
                break;
            case GSTO_XSCALE_NONE:
                x = 0.0;
                break;
            case GSTO_XSCALE_ARBITRARY:
                if(fscanf(file->fp, "%lf\n", &x) != 1) {
                    free(table);
                    return NULL;
                }
                break;
        }
        table[i]=jibal_gsto_em_from_file_units(x, file);
    }
    return table;
}

int jibal_gsto_em_to_index(const gsto_file_t *file, double em) { /* Returns the low bin, i.e. i of em[i], where
 * file->em[i] <= em < file->em[i+1], or -1 if vel is beyond limits of the file. Includes speedups for lin and log
 * scale, but there is a conversion cost to units of the file. keV/u and MeV/u files are by default converted into
 * J/kg, to avoid (the extremely low) conversion cost for these.
 *
 * For arbitrarily spaced X values binary search is employed. Due to floating point issues the log speedup might
 * return i+1 if v is very close to em[i+1]. This shouldn't make much of a difference after (linear) interpolation. */
    double x;
    switch (file->xunit) {
        case GSTO_X_UNIT_J_KG:
            x = em;
            break;
        case GSTO_X_UNIT_MEV_U:
            x = em/(C_MEV/C_U);
            break;
        case GSTO_X_UNIT_KEV_U:
            x = em/(C_KEV/C_U);
            break;
        case GSTO_X_UNIT_M_S:
        default:
            x=velocity_from_em(em);
            break;
    }
    int lo, mi, hi;
    switch (file->xscale) {
        case GSTO_XSCALE_LOG10:
            lo = floor((log10(x) - file->xmin_speedup) * file->xdiv);
            if(lo < 0 || lo >= file->xpoints) {
                return -1;
            }
            break;
        case GSTO_XSCALE_LINEAR:
            lo = floor((x - file->xmin) * file->xdiv);
            if(lo < 0 || lo >= file->xpoints) {
                return -1;
            }
            break;
        case GSTO_XSCALE_ARBITRARY:
        default:
            hi = file->xpoints - 1;
            lo = 0;
            while (hi - lo > 1) {
                mi = (hi + lo) / 2;
                if (em >= file->em[mi]) {
                    lo = mi;
                } else {
                    hi = mi;
                }
            }
    }
#ifdef DEBUG
fprintf(stderr, "lo=%i, residual=%e m/s (%.2lf%% of bin)\n", 
            lo, em - file->em[lo], 100.0*(em - file->em[lo])/(file->em[lo+1] - file->em[lo]));
#endif
#ifdef GSTO_VELOCITY_BIN_CHECK_STRICT /* Note: due to floating point issues this is too strict */
    if (em < file->em[lo] || em > file->em[lo + 1]) { /* Sanity check and out-of-bounds check. */
        return -1;
    }
#else
    if (em < file->em[0] || em > file->em[file->xpoints - 1]) {
        return -1;
    }
#endif

    return lo;
}

double jibal_gsto_get_em(jibal_gsto *workspace, gsto_stopping_type type, int Z1, int Z2, double em) {
    gsto_file_t *file = jibal_gsto_get_assigned_file(workspace, type, Z1, Z2);
    if(!file) {
        return nan(NULL);
    }
    const double *data=jibal_gsto_file_get_data(file, Z1, Z2);
    assert(data);
    int lo;
    if(file->em[file->em_index_accel] <= em && em < file->em[file->em_index_accel+1]) {
        lo = file->em_index_accel;
    } else {
        lo = jibal_gsto_em_to_index(file, em);
        if (lo < 0) { /* Out of bounds */
            if(workspace->extrapolate) {
                if(em >= 0 && em <= file->em[0]) {
                    return jibal_linear_interpolation(0.0, file->em[0], 0.0, data[0], em);
                    /* Linear interpolation from (0, 0) to lowest real data point */
                }
                if(em >= file->em[file->xpoints - 1]) {
                    return data[file->xpoints - 1];
                }
            }
            return nan(NULL);
        }
        file->em_index_accel = lo;
    }
    double out;
    out=jibal_linear_interpolation(file->em[lo], file->em[lo+1], data[lo], data[lo+1], em);
    if(file->straggunit == GSTO_STRAGG_UNIT_BOHR) {
        assert(type == GSTO_STO_STRAGG);
        out *= jibal_stragg_bohr(Z1, Z2);
    }
    if(file->stounit == GSTO_STO_UNIT_EV15CM2) {
        assert(type == GSTO_STO_ELE);
        out *= C_EV_TFU;
    }
    return out;
}

int jibal_gsto_assign_material(jibal_gsto *workspace, const jibal_isotope *incident, jibal_material *target, gsto_file_t *file) {
    int i;
    for (i = 0; i < target->n_elements; i++) {
        if(!jibal_gsto_assign(workspace, incident->Z, target->elements[i].Z, file)) {
            return 0;
        }
    }
    return 1; /* Success */
}

int jibal_gsto_auto_assign_material(jibal_gsto *workspace, const jibal_isotope *incident, jibal_material *target) {
    int i;

    for (i = 0; i < target->n_elements; i++) {
        if(!jibal_gsto_auto_assign(workspace, incident->Z, target->elements[i].Z)) {
            return 0; /* Fail */
        }
    }
    return 1; /* Success */
}
