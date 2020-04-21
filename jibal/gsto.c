#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <jibal_units.h>
#include <jibal_phys.h>
#include <jibal_gsto.h>
#include <defaults.h>
#include "win_compat.h"


int gsto_header_n(const gsto_header *header) {
    const gsto_header *h;
    int n=0;
    if (!header)
        return 0;
    for (h = header; h->s; h++) {
        n++;
    }
    return n;
}

int gsto_get_header_value(const gsto_header *header, const char *s) {
    const gsto_header *h;
    int i=0;
    if(!s)
        return 0; /* TODO: is returning zero a sane fail safe? */
    for(h=header; h->s; h++) {
        if (strcmp(h->s, s) == 0) {
            return i;
        }
        i++;
    }
    return 0;
}

const char *gsto_get_header_string(const gsto_header *header, int val) {
    int n = gsto_header_n(header);
    if (val >= n)
        return "";
    else
        return header[val].s;
}

int gsto_add_file(jibal_gsto *table, const char *name, const char *filename) {
    int success=0;
#ifdef DEBUG
    fprintf(stderr, "Adding file %s (%s).\n", name, filename);
#endif
    table->files = realloc(table->files, sizeof(gsto_file_t)*(table->n_files+1));
    gsto_file_t *new_file=&table->files[table->n_files];
    memset(new_file, 0, sizeof(gsto_file_t));
    new_file->name = strdup(name);
    new_file->filename = strdup(filename);
    success=jibal_gsto_load(table, new_file);

    if(success) {
        table->n_files++;
    } else {
        fprintf(stderr, "Error in adding stopping file %s (%s).\n", name, filename);
        free(new_file->name);
        free(new_file->filename);
    }
    return success;
    
}

jibal_gsto *gsto_allocate(int Z1_max, int Z2_max) {
    jibal_gsto *table = malloc(sizeof(jibal_gsto));
    table->Z1_max=Z1_max;
    table->Z2_max=Z2_max;
    table->n_files=0;
    table->files=NULL; /* These will be allocated by gsto_new_file */
    table->n_comb=Z1_max*Z2_max; /* From 1..Z1_max inclusive and from 1..Z2_max inclusive */
    table->assignments = calloc(table->n_comb, sizeof(gsto_file_t *));
    return table;
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
    free(workspace->assignments);
}

int jibal_gsto_assign(jibal_gsto *workspace, int Z1, int Z2, gsto_file_t *file) { /* Used internally, can be used after init to override autoinit */
    if (Z1 < file->Z1_min || Z1 > file->Z1_max || Z2 < file->Z2_min || Z2 > file->Z2_max) {
        return 0; /* Fail */
    }
    if(Z1 > workspace->Z1_max || Z2 > workspace->Z2_max) {
        return 0;
    }
    int i=jibal_gsto_table_get_index(workspace, Z1, Z2);
    assert(i >= 0 && i <= workspace->n_comb);
    workspace->assignments[jibal_gsto_table_get_index(workspace, Z1, Z2)]=file;
    return 1; /* Success */
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
            if (file == jibal_gsto_get_assigned_file(workspace, Z1, Z2)) {
                double *data = jibal_gsto_file_allocate_data(file, Z1, Z2);
                fread(data, sizeof(double), file->xpoints, file->fp);
                int i;
                for(i=0; i<file->xpoints; i++) {
                    data[i] = jibal_gsto_scale_y_to_stopping(file, data[i]);
                }
            } else {
                fseek(file->fp, sizeof(double)*file->xpoints, SEEK_CUR);
            }
        }
    }
    return 1;
}
void jibal_gsto_fprint_header_property(FILE *f, gsto_header_type h, int val) {
    const gsto_header *properties;
    switch(h) {
        case GSTO_HEADER_STOUNIT:
            properties=gsto_sto_units;
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
    if(val < gsto_header_n(properties))
        fprintf(f, "%s=%s\n", gsto_headers[h].s, properties[val].s);
    else
        fprintf(stderr, "Warning: can't print GSTO header. val=%i (should be < %i)\n",
                val, gsto_header_n(properties));
}
void jibal_gsto_fprint_header_scientific(FILE *f, gsto_header_type h, double val) {
    fprintf(f, "%s=%e\n", gsto_get_header_string(gsto_headers, h), val);
}

void jibal_gsto_fprint_header(FILE *f, gsto_header_type h, void *val) { /* Value is interpreted based on
 * header to be int, double or char *. In the last case void * is char **!. */
    char type;
    if(h >= GSTO_N_HEADER_TYPES) {
        fprintf(stderr, "GSTO Error: %i is not v valid header type. This shouldn't happen, check GSTO_N_HEADER_TYPES\n", h);
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
        case GSTO_HEADER_Z1MIN:
        case GSTO_HEADER_Z1MAX:
        case GSTO_HEADER_Z2MIN:
        case GSTO_HEADER_Z2MAX:
            type='i';
            break;
        case GSTO_HEADER_STOUNIT:
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
    }
}

void jibal_gsto_fprint_file(FILE *file_out, gsto_file_t *file, gsto_data_format format, int Z1_min, int
        Z1_max, int Z2_min, int Z2_max) {
    int i, Z1, Z2;
    Z1_min=Z1_min<file->Z1_min?file->Z1_min:Z1_min;
    Z1_max=Z1_max>file->Z1_max?file->Z1_max:Z1_max;
    Z2_min=Z2_min<file->Z2_min?file->Z2_min:Z2_min;
    Z2_max=Z2_max>file->Z2_max?file->Z2_max:Z2_max;

    if(file->source) {
        fprintf(file_out, "%s=%s\n", gsto_headers[GSTO_HEADER_SOURCE].s, file->source);
    }
    jibal_gsto_fprint_header(file_out, GSTO_HEADER_FORMAT, &format);
    jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z1MIN, &Z1_min);
    jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z1MAX, &Z1_max);
    jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z2MIN, &Z2_min);
    jibal_gsto_fprint_header(file_out, GSTO_HEADER_Z2MAX, &Z2_max);
    jibal_gsto_fprint_header_property(file_out, GSTO_HEADER_STOUNIT,
            (format==GSTO_DF_ASCII)?GSTO_STO_UNIT_EV15CM2:GSTO_STO_UNIT_JM2);
    /* We convert numbers if we are outputting ASCII, but binary stays as internal binary (SI units) */
    if(file->xscale == GSTO_XSCALE_ARBITRARY) { /* Arbitrary scales are converted */
        jibal_gsto_fprint_header_property(file_out,GSTO_HEADER_XUNIT, GSTO_X_UNIT_KEV_U);
        jibal_gsto_fprint_header_scientific(file_out,GSTO_HEADER_XMIN, file->em[0]/(C_KEV/C_U));
        jibal_gsto_fprint_header_scientific(file_out,GSTO_HEADER_XMAX, file->em[file->xpoints - 1]/(C_KEV/C_U));
        jibal_gsto_fprint_header(file_out,GSTO_HEADER_XPOINTS, &file->xpoints);
        jibal_gsto_fprint_header(file_out,GSTO_HEADER_XSCALE, &file->xscale);
    } else {
        jibal_gsto_fprint_header_property(file_out, GSTO_HEADER_XUNIT, file->xunit);
        jibal_gsto_fprint_header(file_out, GSTO_HEADER_XMIN, &file->xmin);
        jibal_gsto_fprint_header(file_out, GSTO_HEADER_XMAX, &file->xmax);
        jibal_gsto_fprint_header(file_out,GSTO_HEADER_XPOINTS, &file->xpoints);
        jibal_gsto_fprint_header(file_out,GSTO_HEADER_XSCALE, &file->xscale);
    }
    fprintf(file_out, "%s\n", GSTO_END_OF_HEADERS);
    if(file->xscale == GSTO_XSCALE_ARBITRARY) {
        for(i=0; i < file->xpoints; i++) {
            fprintf(file_out, "%e\n", file->em[i]/(C_KEV/C_U));
        }
    }
    for (Z1=Z1_min; Z1 <= Z1_max; Z1++) {
        for (Z2 = Z2_min; Z2 <= Z2_max; Z2++) {
            const double *data=jibal_gsto_file_get_data(file, Z1, Z2);
            if(!data) {
                fprintf(stderr, "Error: no data for Z1=%i and Z2=%i in %s.%s\n",
                        Z1, Z2, file->name, file->data?"":"This file isn't loaded yet.");
                return;
            }
            if(format == GSTO_DF_ASCII) {
                int i;
                for (i = 0; i < file->xpoints; i++) {
                    fprintf(file_out, "%e\n", data[i] / C_EV_TFU);
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
    assert(Z1 >= file->Z1_min);
    assert(Z2 >= file->Z2_min);
    assert(Z1 <= file->Z2_max);
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
    char *line = calloc(GSTO_MAX_LINE_LEN, sizeof(char));
    int actually_skipped=0;
#ifdef DEBUG
    fprintf(stderr, "Loading ascii data.\n");
#endif
    
    for (Z1=file->Z1_min; Z1 <= file->Z1_max && Z1 <= workspace->Z1_max; Z1++) {
        for (Z2=file->Z2_min; Z2 <= file->Z2_max && Z2 <= workspace->Z2_max; Z2++) {
            if (file == jibal_gsto_get_assigned_file(workspace, Z1, Z2)) { /* This file is assigned to this Z1, Z2 combination, so we have to load the stopping in. */
                skip=file->xpoints*((Z1-previous_Z1)*(file->Z2_max-file->Z2_min+1)+(Z2-previous_Z2-1)); /* Not sure if correct, but it works. */
#ifdef DEBUG
                fprintf(stderr, "Skipping %i*(%i*%i+%i)=%i lines.\n", file->xpoints, Z1-previous_Z1, file->Z1_max-file->Z2_min+1, Z2-previous_Z2-1, skip);
                actually_skipped=0;
#endif

                while (skip--) {
                    file->lineno++;
                    if(!fgets(line, GSTO_MAX_LINE_LEN, file->fp))
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
                    if(!fgets(line, GSTO_MAX_LINE_LEN, file->fp)) {
                        fprintf(stderr, "ERROR: File %s ended prematurely when reading Z1=%i Z2=%i stopping point=%i/%i"
                                        ".\n", file->filename, Z1, Z2, i+1, file->xpoints);
                        break;
                    }
                    file->lineno++;
                    if(*line == '#') { /* This line is a comment. Ignore. */
                        i--;
                    } else {
#ifdef DEBUG
                        fprintf(stderr, "Loaded stopping [%i][%i][%i] from line %i.\n", Z1, Z2, i, file->lineno);
#endif
                        data[i] = jibal_gsto_scale_y_to_stopping(file, strtod(line, NULL));
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



int jibal_gsto_load(jibal_gsto *workspace, gsto_file_t *file) {
    char *line;
    char *line_split;
    char *columns[3];
    char **col;
    int header = 0;
    if (!file) {
        return 0;
    }
    file->fp = fopen(file->filename, "r");
    if (!file->fp) {
        fprintf(stderr, "Could not open file \"%s\".\n", file->filename);
        return 0;
    }
    file->lineno = 0;
    line = calloc(GSTO_MAX_LINE_LEN, sizeof(char));
    /* parse headers, stop when end of headers found */
    while (fgets(line, GSTO_MAX_LINE_LEN, file->fp) != NULL) {
        file->lineno++;
        if (strncmp(line, GSTO_END_OF_HEADERS, strlen(GSTO_END_OF_HEADERS)) == 0) {
#ifdef DEBUG
            fprintf(stderr, "End of headers on line %i of settings file.\n", file->lineno);
#endif
            break;
        }
        line_split = line;
        for (col = columns; (*col = strsep(&line_split, "=\n\r\t")) != NULL;)
            if (**col != '\0')
                if (++col >= &columns[3])
                    break;

        header = gsto_get_header_value(gsto_headers, columns[0]);
        switch (header) {
            case GSTO_HEADER_SOURCE:
                file->source = strdup(columns[1]);
                break;
            case GSTO_HEADER_FORMAT:
                file->data_format = gsto_get_header_value(gsto_data_formats, columns[1]);
                break;
            case GSTO_HEADER_STOUNIT:
                file->stounit = gsto_get_header_value(gsto_sto_units, columns[1]);
                break;
            case GSTO_HEADER_XSCALE:
                file->xscale = gsto_get_header_value(gsto_xscales, columns[1]);
                break;
            case GSTO_HEADER_XUNIT:
                file->xunit = gsto_get_header_value(gsto_xunits, columns[1]);
                break;
            case GSTO_HEADER_XPOINTS:
                file->xpoints = strtol(columns[1], NULL, 10);
                break;
            case GSTO_HEADER_XMIN:
                file->xmin = strtod(columns[1], NULL);
                break;
            case GSTO_HEADER_XMAX:
                file->xmax = strtod(columns[1], NULL);
                break;
            case GSTO_HEADER_Z1MIN:
                file->Z1_min = strtol(columns[1], NULL, 10);
                break;
            case GSTO_HEADER_Z1MAX:
                file->Z1_max = strtol(columns[1], NULL, 10);
                break;
            case GSTO_HEADER_Z2MIN:
                file->Z2_min = strtol(columns[1], NULL, 10);
                break;
            case GSTO_HEADER_Z2MAX:
                file->Z2_max = strtol(columns[1], NULL, 10);
                break;
            case GSTO_HEADER_NONE:
            default:
                fprintf(stderr, "GSTO Warning: unknown header \"%s\" on line %i of file %s\n", columns[0], file->lineno,
                        file->filename);
                break;
        }
    } /* End of headers */
    file->n_comb = (file->Z1_max - file->Z1_min + 1) * (file->Z2_max - file->Z2_min + 1);
    if(file->data) {
        free(file->data);
    }
    file->data = calloc(file->n_comb, sizeof(double *));
    if(file->em) {
        free(file->em);
    }
    file->em = jibal_gsto_em_table(file);
    file->xmin_speedup = 0.0;
    file->xdiv = 0.0;
    file->em_index_accel = 0;
#ifndef GSTO_DONT_CONVERT_TO_SI
    /* We convert energy per mass units to SI, this will possibly reduce conversions later. Note that we only need
     * recalculate xmin and xmax, since other variables are internally in SI. */
    if(file->xunit == GSTO_X_UNIT_MEV_U) {
        file->xmin *= C_MEV/C_U;
        file->xmax *= C_MEV/C_U;
        file->xunit = GSTO_X_UNIT_J_KG;
    } else if(file->xunit == GSTO_X_UNIT_KEV_U) {
        file->xmin *= C_KEV/C_U;
        file->xmax *= C_KEV/C_U;
        file->xunit = GSTO_X_UNIT_J_KG;
    }
#endif

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
    free(line);
    return 1;
}

int jibal_gsto_load_all(jibal_gsto *workspace) { /* For every file, load combinations from file */
    int i;
    int n_success=0;
    for(i=0; i < workspace->n_files; i++) {
        gsto_file_t *file = &workspace->files[i];
        n_success += jibal_gsto_load(workspace, file);
    }
    return n_success;
}

int jibal_gsto_print_files(jibal_gsto *workspace) {
    int i, Z1, Z2;
    int assignments;
    gsto_file_t *file;
    fprintf(stderr, "List of available stopping files:\n");
    
    for(i=0; i < workspace->n_files; i++) {
        assignments=0;
        file=&workspace->files[i];
        for (Z1=1; Z1 <= workspace->Z1_max; Z1++) {
            for (Z2=1; Z2 <= workspace->Z2_max; Z2++) {
                if(jibal_gsto_get_assigned_file(workspace, Z1, Z2) == file) {
                    assignments++;
                }
            }        
        }
        fprintf(stderr, "%i: %s,\n", i+1, file->name);
        fprintf(stderr, "\tfilename: %s\n", file->filename);
        if(file->source) {
            fprintf(stderr, "\tdata source: %s\n", file->source);
        }
        fprintf(stderr, "\t%i assignments,\n", assignments);
        fprintf(stderr, "\t%i <= Z1 <= %i,\n", file->Z1_min, file->Z1_max);
        fprintf(stderr, "\t%i <= Z2 <= %i,\n", file->Z2_min, file->Z2_max);
        fprintf(stderr, "\tx-points=%i, x-scale=%s, x-unit=%s,\n", file->xpoints,
                gsto_get_header_string(gsto_xscales, file->xscale),
                gsto_get_header_string(gsto_xunits, file->xunit));
        fprintf(stderr, "\tstopping unit=%s, format=%s\n", gsto_get_header_string(gsto_sto_units, file->stounit),
                gsto_get_header_string(gsto_data_formats, file->data_format));
    }
    return 1;
}

int jibal_gsto_print_assignments(jibal_gsto *workspace) {
    int Z1, Z2;
    fprintf(stderr, "LIST OF ASSIGNED STOPPING FILES FOLLOWS\n=====\n");
    for (Z1=1; Z1 <= workspace->Z1_max; Z1++) {
        for (Z2=1; Z2 <= workspace->Z2_max; Z2++) {
            gsto_file_t *file= jibal_gsto_get_assigned_file(workspace, Z1, Z2);
            if(file) {
                fprintf(stderr, "Stopping for Z1=%i in Z2=%i assigned to file %s.\n", Z1, Z2, file->name);
            } else {
#ifdef DEBUG
                fprintf(stderr, "Stopping for Z1=%i in Z2=%i not assigned.\n", Z1, Z2);
#endif
            }
        }        
    }
    fprintf(stderr, "=====\n");
    return 1;
}

int jibal_gsto_auto_assign(jibal_gsto *workspace, int Z1, int Z2) {
    gsto_file_t *file;
    int success=0, i;
    for (i=0; i < workspace->n_files; i++) {
        file=&workspace->files[i];
        if (file->Z1_min<=Z1 && file->Z1_max >= Z1 && file->Z2_min <= Z2 && file->Z2_max >= Z2) { /*File includes this Z1, Z2 combination*/
            jibal_gsto_assign(workspace, Z1, Z2, file);
            success=1;
            break; /* Stop when the first file to include this combination is found */
        }
    }
    return success;
}

jibal_gsto *jibal_gsto_init(int Z_max, const char *datadir, const char *stoppings_file_name) {
    int i=0, n_files=0, n_errors=0;
    char *line=calloc(GSTO_MAX_LINE_LEN, sizeof(char));
    char *line_split;
    char *columns[2];
    char **col;
    int lineno=0;
    FILE *settings_file=NULL;
    jibal_gsto *workspace;
    workspace = gsto_allocate(Z_max, Z_max);
    workspace->stop_step = JIBAL_STEP_SIZE; /* TODO: set this from some configuration. Used only for layer energy
 * loss calculations */
    workspace->extrapolate = FALSE;
    if(!stoppings_file_name) { /* If filename given (not NULL), attempt to load settings file */
        stoppings_file_name=JIBAL_STOPPINGS_FILE;
    }
    settings_file=fopen(stoppings_file_name, "r");
    if(!settings_file) {
        fprintf(stderr, "Can not open file %s\n", stoppings_file_name);
        return NULL;
    }
#ifdef DEBUG
    fprintf(stderr, "Settings from %s\n", stoppings_file_name);
#endif
    while (fgets(line, GSTO_MAX_LINE_LEN, settings_file) != NULL) {
        lineno++;
        i++;
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
        char *filename=columns[1];
        if(name && filename ) {
            if (filename[0] != '/') {
                filename = calloc(strlen(datadir) + strlen(filename) + 1, sizeof(char));
                strcat(filename, datadir);
                strcat(filename, columns[1]); /* Note, filename now starts with '/' so we don't need to add it */
            }


            if (gsto_add_file(workspace, name, filename)) {
                n_files++;
            } else {
                fprintf(stderr, "WARNING: adding file %s failed.\n", filename);
                n_errors++;
            }
        } else {
            fprintf(stderr, "WARNING: adding stopping file failed, since line %i in %s is malformed.\n", lineno, stoppings_file_name);
            n_errors++;
        }
        if(filename != columns[1] && filename) { /* Free filename if it was allocated */
            free(filename);
        }
    }
#ifdef DEBUG
    fprintf(stderr, "GSTO: Read %i lines from settings file, added %i files, attempt to add %i files failed.\n", i, n_files, n_errors);
#endif
    fclose(settings_file);
    return workspace;
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

gsto_file_t *jibal_gsto_get_assigned_file(jibal_gsto *workspace, int Z1, int Z2) {
    int i = jibal_gsto_table_get_index(workspace, Z1, Z2);
    if(i < 0)
        return NULL;
    return workspace->assignments[i];
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
                fscanf(file->fp, "%lf\n", &x);
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
fprintf(stderr, "lo=%i, residual=%e m/s (%.2lf%% of bin)\n", lo, v-file->vel[lo],
            100.0*(v-file->vel[lo])/(file->vel[lo+1]-file->vel[lo]));
#endif
#ifdef GSTO_VELOCITY_BIN_CHECK_STRICT /* Note: due to floating point issues this is too strict */
    if (v < file->vel[lo] || v > file->vel[lo + 1]) { /* Sanity check and out-of-bounds check. */
        return -1;
    }
#else
    if (em < file->em[0] || em > file->em[file->xpoints - 1]) {
        return -1;
    }
#endif

    return lo;
}

double jibal_gsto_scale_y_to_stopping(const gsto_file_t *file, double y) {
    switch (file->stounit) {
        case GSTO_STO_UNIT_NONE:
            return y;
        case GSTO_STO_UNIT_EV15CM2:
            return y*C_EV_TFU;
        case GSTO_STO_UNIT_JM2:
            return y; /* The SI unit, used internally */
        default:
            return y;
    }
}

double jibal_gsto_stop_em(jibal_gsto *workspace, int Z1, int Z2, double em) {
    gsto_file_t *file= jibal_gsto_get_assigned_file(workspace, Z1, Z2);
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
    double sto;
    sto=jibal_linear_interpolation(file->em[lo], file->em[lo+1], data[lo], data[lo+1], em);
    return sto;
}

double jibal_gsto_stop_v(jibal_gsto *workspace, int Z1, int Z2, double v) {
    return jibal_gsto_stop_em(workspace, Z1, Z2, energy_per_mass(v));
}

double jibal_stop(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E) {
    /* TODO: make a variable in the workspace that determines whether we are interested in total stopping or just
     * electronic stopping. At the moment this is fixed to total. */
    return jibal_stop_nuc(incident, target, E) + jibal_stop_ele(workspace, incident, target, E); /* This returns a POSITIVE value (-dE/dx) in SI units for stopping cross section, e.g. J/(1/m^2) = J m^2 */
}

double jibal_stop_nuc(const jibal_isotope *incident, const jibal_material *target, double E) {
    int i;
    double sum = 0.0;
    for (i = 0; i < target->n_elements; i++) {
        jibal_element *element = &target->elements[i];
#ifndef NUCLEAR_STOPPING_ISOTOPES
        sum += target->concs[i]*jibal_gsto_stop_nuclear_universal(E, incident->Z, incident->mass, element->Z, element->avg_mass);
#else
        int j;
        for(j=0; j < element->n_isotopes; j++) { /* It would probably suffice to calculate the nuclear stopping with an average mass... */
            const jibal_isotope *isotope = element->isotopes[j];
            sum += target->concs[i]*element->concs[j]*gsto_sto_nuclear_universal(E, incident->Z, incident->mass, element->Z, isotope->mass);
        }
#endif
    }
    return sum;
}

double jibal_layer_energy_loss(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_layer *layer, double
E_0, double factor) {
    double E = E_0;
    double x;
    double h = workspace->stop_step;
#ifdef DEBUG
    fprintf(stderr, "Thickness %g, stop step %g\n", layer->thickness, h);
#endif
    for (x = 0.0; x <= layer->thickness; x += h) {
        if(x+h > layer->thickness) { /* Last step may be partial */
            h=layer->thickness-x;
            if(h < workspace->stop_step/1e6) {
                break;
            }
        }
#ifndef NO_RUNGE_KUTTA
        double k1, k2, k3, k4;
        k1 = factor*jibal_stop(workspace, incident, layer->material, E);
        k2 = factor*jibal_stop(workspace, incident, layer->material, E + (h / 2) * k1);
        k3 = factor*jibal_stop(workspace, incident, layer->material, E + (h / 2) * k2);
        k4 = factor*jibal_stop(workspace, incident, layer->material, E + h * k3);
        E += (h / 6) * (k1 + 2 * k2 + 2 * k3 + k4);
#else
        E += factor*h*jibal_stop(workspace, incident, layer->material, E);
#endif
        if(!isnormal(E))
            return 0.0;
    }
    return E;
}

double jibal_stop_ele(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E) {
    int i;
    double sum = 0.0;
    double v=velocity(E, incident->mass);
    for (i = 0; i < target->n_elements; i++) {
        jibal_element *element = &target->elements[i];
        sum += target->concs[i]*jibal_gsto_stop_v(workspace, incident->Z, element->Z, v);
    }
    return sum;
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
