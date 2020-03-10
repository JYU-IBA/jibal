#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <jibal_units.h>
#include <jibal_phys.h>
#include <jibal_gsto.h>
#include <defaults.h>

static char *gsto_stopping_types[] ={ /* The first three characters are tested with e.g. strncmp(stopping_types[i], "tot", 3*sizeof(char)). So make them unique. */ 
    "none",
    "nuclear",
    "electronic",
    "total"
};

static char *sto_units[] = {
    "none",
    "eV/(1e15 atoms/cm2)"
};

static char *formats[] = {
    "none",
    "ascii",
    "binary"
};

static char *xscales[] = {
    "none",
    "linear",
    "log10"
};

static char *xunits[] = {
    "none",
    "m/s",
    "keV/u"
};

static char *gsto_headers[] = {
    "      ",
    "source",
    "z1-min",
    "z1-max",
    "z2-min",
    "z2-max",
    "sto-unit",
    "x-unit",
    "format",
    "x-min",
    "x-max",
    "x-points",
    "x-scale"
};

header_properties_t gsto_get_headers_property(const char *property) {
    
    return 0;
}

int gsto_add_file(jibal_gsto *table, char *name, char *filename, int Z1_min, int Z1_max, int Z2_min, int Z2_max, char *type) {
    int success=1;
    int i;
#ifdef DEBUG
    fprintf(stderr, "Adding file %s (%s), %i<=Z1<=%i, %i<=Z2<=%i to database.\n", name, filename, Z1_min, Z1_max, Z2_min, Z2_max);
#endif
    table->files = realloc(table->files, sizeof(gsto_file_t)*(table->n_files+1));
    gsto_file_t *new_file=&table->files[table->n_files];
    new_file->name = strdup(name);
    if(*filename == '/') { /* Absolute path, just copy the file name */
        new_file->filename=strdup(filename);
    } else { /* Relative path. Append to datadir path. */
        new_file->filename=calloc(strlen(JIBAL_DATADIR)+strlen(filename)+1, sizeof(char));
        strcat(new_file->filename, JIBAL_DATADIR);
        strcat(new_file->filename, filename);
    }
    for(i=GSTO_N_STOPPING_TYPES-1; i >=0; i--) {
        if(strncmp(gsto_stopping_types[i], type, 3*sizeof(char))==0) {
            break;
        }
    }
    if(i<=0) {
        return 0; /* No stopping type specified or "none" specified. Perhaps it is not necessary to take this file into account... */
    }
    
    new_file->Z1_min=Z1_min; /* These will be rewritten when the file is loaded. */
    new_file->Z1_max=Z1_max;
    new_file->Z2_min=Z2_min;
    new_file->Z2_max=Z2_max;
    new_file->xmin=0; /* This will be read from the file when loaded */
    new_file->xmax=0; /* Same as this */
    new_file->xscale=0; /* and this */
    new_file->stounit=0;
    new_file->xunit=0;
    new_file->data=NULL; /* this is allocated also when the file is loaded */

    if(Z1_min > Z1_max) {
        success=0;
    }
    if(Z2_min > Z2_max) {
        success=0;
    }
    if(success) {
        table->n_files++;
    } else {
        fprintf(stderr, "Error in adding stopping file %s (%s) to database\n", name, filename);
        free(new_file->name);
        free(new_file->filename);
    }
    return success;
    
}

jibal_gsto *gsto_allocate(int Z1_max, int Z2_max) {
    int Z1;
    jibal_gsto *table = malloc(sizeof(jibal_gsto));
    table->Z1_max=Z1_max;
    table->Z2_max=Z2_max;
    table->n_files=0;
    table->files=NULL; /* These will be allocated by gsto_new_file */
    table->n_comb=Z1_max*Z2_max; /* From 1..Z1_max inclusive and from 1..Z2_max inclusive */
    table->assignments = calloc(table->n_comb, sizeof(gsto_file_t *));
    return table;
}


void jibal_gsto_file_free(gsto_file_t *file) {
    int i;
    if(file->data) {
        for(i=0; i < file->n_comb; i++) {
            if(file->data[i] != NULL) {
                free(file->data[i]);
            }
        }
        free(file->data);
    }
    free(file->name);
    free(file->filename);
    free(file);
}

void jibal_gsto_free(jibal_gsto *workspace) {
    int i;
    for(i=0; i < workspace->n_files; i++) {
        gsto_file_t *file=&workspace->files[i];
        jibal_gsto_file_free(file);
    }
    free(workspace->assignments);
}

int jibal_gsto_assign(jibal_gsto *workspace, int Z1, int Z2, gsto_file_t *file) { /* Used internally, can be used after init to override autoinit */
    int i=jibal_gsto_table_get_index(workspace, Z1, Z2);
    if(i < 0) {
        return 0;
    }
    workspace->assignments[jibal_gsto_table_get_index(workspace, Z1, Z2)]=file;
    return 1;
}

int gsto_load_binary_file(jibal_gsto *workspace, gsto_file_t *file) {
    int Z1, Z2;
#ifdef DEBUG
    fprintf(stderr, "Loading binary data.\n");
#endif
    for (Z1=file->Z1_min; Z1<=file->Z1_max; Z1++) {
        for (Z2=file->Z2_min; Z2<=file->Z2_max; Z2++) {
            if (file == jibal_gsto_get_file(workspace, Z1, Z2)) {
                double *data = jibal_gsto_file_allocate_data(file, Z1, Z2);
                fread(data, sizeof(double), file->xpoints, file->fp);
            } else {
                fseek(file->fp, sizeof(double)*file->xpoints, SEEK_CUR);
            }
        }
    }
    return 1;
}

int jibal_gsto_table_get_index(jibal_gsto *workspace, int Z1, int Z2) {
    if(Z1 < 1 || Z1 > workspace->Z1_max || Z2 < 1 || Z2 > workspace->Z2_max) {
        return -1;
    }
    int z1 = (Z1 - 1);
    int z2 = (Z2 - 1);
    int n_z1 = (workspace->Z1_max - 1 + 1);
    int n_z2 = (workspace->Z2_max - 1 + 1);
    int i = (n_z2 * z1 + z2);
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
    int n_z1=(file->Z1_max-file->Z1_min+1);
    int n_z2=(file->Z2_max-file->Z2_min+1);
    int i=(n_z2*z1+z2);
    assert(i < file->n_comb);
    return i;
}

const double *jibal_gsto_file_get_data(gsto_file_t *file, int Z1, int Z2) {
    return file->data[jibal_gsto_file_get_data_index(file, Z1, Z2)];
}

double *jibal_gsto_file_allocate_data(gsto_file_t *file, int Z1, int Z2) {
    double *data;
    int i=jibal_gsto_file_get_data_index(file, Z1, Z2);
#ifdef DEBUG
    fprintf(stderr, "i=%i for Z1=%i and Z2=%i\n", i, Z1, Z2);
#endif
    if(file->data[i] == NULL) { /* Allocate only if null! */
        file->data[i] = calloc(file->xpoints, sizeof(double));
    }
    return file->data[i];
}

int gsto_load_ascii_file(jibal_gsto *workspace, gsto_file_t *file) {
    int Z1, Z2, previous_Z1=file->Z1_min, previous_Z2=file->Z2_min-1, skip, i;
    char *line = calloc(GSTO_MAX_LINE_LEN, sizeof(char));
    int actually_skipped=0;
#ifdef DEBUG
    fprintf(stderr, "Loading ascii data.\n");
#endif
    
    for (Z1=file->Z1_min; Z1<=file->Z1_max && Z1 <= workspace->Z1_max; Z1++) {
        for (Z2=file->Z2_min; Z2<=file->Z2_max && Z2<=file->Z2_max; Z2++) {
            if (file == jibal_gsto_get_file(workspace, Z1, Z2)) { /* This file is assigned to this Z1, Z2 combination, so we have to load the stopping in. */
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
#ifdef DEBUG
                        fprintf(stderr, "File %s ended prematurely when reading Z1=%i Z2=%i stopping point=%i/%i.\n", file->filename, Z1, Z2, i+1, file->xpoints);
#endif
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





int gsto_load(jibal_gsto *workspace) { /* For every file, load combinations from file */
    int i;
    gsto_file_t *file;
    char *line=calloc(GSTO_MAX_LINE_LEN, sizeof(char));
    char *line_split;
    char *columns[3];
    char **col;
    int header=0, property;
    for(i=0; i < workspace->n_files; i++) {
        file=&workspace->files[i];
        file->lineno=0;
        file->fp=fopen(file->filename, "r");
        if(!file->fp) {
            fprintf(stderr, "Could not open file %s for reading.\n", file->filename);
            return 0;
        }
        /* parse headers, stop when end of headers found */
        while (fgets(line, GSTO_MAX_LINE_LEN, file->fp) != NULL) {
            file->lineno++;
            if(strncmp(line, GSTO_END_OF_HEADERS, strlen(GSTO_END_OF_HEADERS))==0) {
#ifdef DEBUG
                fprintf(stderr, "End of headers on line %i of settings file.\n", file->lineno);
#endif
                break;
            }
            line_split=line;
            for (col = columns; (*col = strsep(&line_split, "=\n\r\t")) != NULL;)
                if (**col != '\0')
                    if (++col >= &columns[3])
                        break;
#ifdef DEBUG
            fprintf(stderr, "Line %i, property %s is %s.\n", file->lineno, columns[0], columns[1]);
#endif 
            for(header=0; header < GSTO_N_HEADER_TYPES; header++) {
#ifdef DEBUG
                fprintf(stderr, "Does \"%s\" match \"%s\"? ", columns[0], gsto_headers[header]);
#endif
                if(strncmp(columns[0], gsto_headers[header], strlen(gsto_headers[header]))==0) {
#ifdef DEBUG
                    fprintf(stderr, "Yes.\n");
#endif
                    switch (header) {
                        case GSTO_HEADER_FORMAT:
                            for(property=0; property<GSTO_N_STO_UNITS; property++) {
                                if(strncmp(formats[property], columns[1], strlen(formats[property]))==0) {
                                    file->data_format=property;
                                }
                            }
                            break;
                        case GSTO_HEADER_STOUNIT:
                            for(property=0; property<GSTO_N_STO_UNITS; property++) {
                                if(strncmp(sto_units[property], columns[1], strlen(sto_units[property]))==0) {
                                    file->stounit=property;
                                }
                            }
                            break;
                        case GSTO_HEADER_XSCALE:
                            for(property=0; property<GSTO_N_X_SCALES; property++) {
                                if(strncmp(xscales[property], columns[1], strlen(xscales[property]))==0) {
                                    file->xscale=property;
                                }
                            }
                            break;
                        case GSTO_HEADER_XUNIT:
                            for(property=0; property<GSTO_N_X_UNITS; property++) {
                                if(strncmp(xunits[property], columns[1], strlen(xunits[property]))==0) {
                                    file->xunit=property;
                                }
                            }
                            break;
                        case GSTO_HEADER_XPOINTS:
                            file->xpoints=strtol(columns[1], NULL, 10);
#ifdef DEBUG
                            fprintf(stderr, "Set number of x points to %i\n", file->xpoints);
#endif
                            break;
                        case GSTO_HEADER_XMIN:
                            file->xmin=strtod(columns[1], NULL);
#ifdef DEBUG
                            fprintf(stderr, "Set minimum value of table to be %lf\n", file->xmin);
#endif
                            break;
                        case GSTO_HEADER_XMAX:
                            file->xmax=strtod(columns[1], NULL);
#ifdef DEBUG
                            fprintf(stderr, "Set maximum value of table to be %lf\n", file->xmax);
#endif
                            break;
                        case GSTO_HEADER_Z1MIN:
                            file->Z1_min=strtol(columns[1], NULL, 10);
                            break;
                        case GSTO_HEADER_Z1MAX:
                            file->Z1_max=strtol(columns[1], NULL, 10);
                            break;
                        case GSTO_HEADER_Z2MIN:
                            file->Z2_min=strtol(columns[1], NULL, 10);
                            break;
                        case GSTO_HEADER_Z2MAX:
                            file->Z2_max=strtol(columns[1], NULL, 10);
                            break;
                        default:
                            break;
                    }
                    break;
                } else {
#ifdef DEBUG
                    fprintf(stderr, "No.\n");
#endif
                }
            } 
        } /* End of headers */
        file->n_comb=(file->Z1_max-file->Z1_min+1)*(file->Z2_max-file->Z2_min+1);
        file->data = calloc(file->n_comb, sizeof(double *));
        switch (file->data_format) {
            case GSTO_DF_DOUBLE:
                gsto_load_binary_file(workspace, file);
                break;
            case GSTO_DF_ASCII:
            default:
                gsto_load_ascii_file(workspace, file);
                break;
        }
        fclose(file->fp);
    }
    free(line);
    return 1;
}

int jibal_gsto_print_files(jibal_gsto *workspace) {
    int i, Z1, Z2;
    int assignments;
    gsto_file_t *file;
    fprintf(stderr, "LIST OF AVAILABLE STOPPING FILES FOLLOWS\n=====\n");
    
    for(i=0; i < workspace->n_files; i++) {
        assignments=0;
        file=&workspace->files[i];
        for (Z1=1; Z1 <= workspace->Z1_max; Z1++) {
            for (Z2=1; Z2 <= workspace->Z2_max; Z2++) {
                if(jibal_gsto_get_file(workspace, Z1, Z2) == file) {
                    assignments++;
                }
            }        
        }
        fprintf(stderr, "%i: %s (%s), %i assignments, %i<=Z1<=%i, %i<=Z2<=%i. x-points=%i, x-scale=%s, x-unit=%s, stopping unit=%s, format=%s\n", i, file->name, file->filename, assignments, file->Z1_min, file->Z1_max, file->Z2_min, file->Z2_max, file->xpoints, xscales[file->xscale], xunits[file->xunit], sto_units[file->stounit], formats[file->data_format]);  
    }
    fprintf(stderr, "=====\n");
    return 1;
}

int jibal_gsto_print_assignments(jibal_gsto *workspace) {
    int Z1, Z2;
    fprintf(stderr, "LIST OF ASSIGNED STOPPING FILES FOLLOWS\n=====\n");
    for (Z1=1; Z1 <= workspace->Z1_max; Z1++) {
        for (Z2=1; Z2 <= workspace->Z2_max; Z2++) {
            gsto_file_t *file=jibal_gsto_get_file(workspace, Z1, Z2);
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

int gsto_auto_assign(jibal_gsto *workspace, int Z1, int Z2) {
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

jibal_gsto *gsto_init(int Z_max, char *stoppings_file_name) {
    int i=0, n_files=0, n_errors=0;
    char *env_path;
    char *line=calloc(GSTO_MAX_LINE_LEN, sizeof(char));
    char *line_split;
    char *columns[8];
    char **col;
    FILE *settings_file=NULL;
    jibal_gsto *workspace;
    workspace = gsto_allocate(Z_max, Z_max);
    workspace->stop_step = JIBAL_STEP_SIZE;
    if(!stoppings_file_name) { /* If filename given (not NULL), attempt to load settings file */
        stoppings_file_name=GSTO_DATA_DEFAULT_FILE;
    }
    settings_file=fopen(stoppings_file_name, "r");
    if(!settings_file) {
        return NULL;
    }
#ifdef DEBUG
    fprintf(stderr, "Settings from %s\n", stoppings_file_name);
#endif
    while (fgets(line, GSTO_MAX_LINE_LEN, settings_file) != NULL) {
        i++;
        if(line[0] == '#') /* Strip comments */
            continue;
        line_split=line; /* strsep will screw up line_split, reset for every new line */
        for (col = columns; (*col = strsep(&line_split, " \t\r\n")) != NULL;)
            if (**col != '\0')
                if (++col >= &columns[8])
                    break;
        if(gsto_add_file(workspace, columns[7], columns[0], strtol(columns[2], NULL, 10), strtol(columns[3], NULL, 10), strtol(columns[4], NULL, 10), strtol(columns[5], NULL, 10), columns[1])) {
            n_files++;
        } else {
            n_errors++;
        }

    }
#ifdef DEBUG
    fprintf(stderr, "GSTO: Read %i lines from settings file, added %i files, attempt to add %i files failed.\n", i, n_files, n_errors);
#endif
    fclose(settings_file);
    return workspace;
}

double gsto_sto_nuclear_universal(double E, int Z1, double m1, int Z2, double m2) {
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

gsto_file_t *jibal_gsto_get_file(jibal_gsto *workspace, int Z1, int Z2) {
    int i = jibal_gsto_table_get_index(workspace, Z1, Z2);
    if(i < 0)
        return NULL;
    return workspace->assignments[i];
}

double jibal_gsto_scale_velocity_to_x(const gsto_file_t *file, double v) {
    double x, gamma;
    /* Scale v to "native" velocity, i.e. units of the file. */
    switch (file->xunit) {
        case GSTO_X_UNIT_KEV_U:
#ifdef CLASSICAL
            x=0.5*(C_U/C_KEV)*pow(v,2.0);
#else
            gamma=1.0/(sqrt(1-pow(v,2.0)/C_C2));
            x=(gamma-1)*C_C2/(C_KEV/C_U);
#endif
            break;
        case GSTO_X_UNIT_M_S:
        default:
            x=v;
            break;
    }
    if(x < file->xmin || x > file->xmax)  {
        return 0.0;
    }
    return x;
}

double jibal_gsto_xscale_to_index(const gsto_file_t *file, double x) {
    double index;
    switch (file->xscale) {
        case GSTO_XSCALE_LOG10:
            index = (log10(x) - log10(file->xmin)) / (log10(file->xmax) - log10(file->xmin)) * (file->xpoints - 1);
            break;
        case GSTO_XSCALE_LINEAR:
        default:
            index = (x - file->xmin) / (file->xmax - file->xmin) * (file->xpoints - 1);
            break;
    }
    return index;
}

double jibal_gsto_scale_y_to_stopping(const gsto_file_t *file, double y) {
    switch (file->stounit) {
        case GSTO_STO_UNIT_NONE:
            return y;
        case GSTO_STO_UNIT_EV15CM2:
            return y*C_EV_TFU;
        default:
            return y;
    }
}



double jibal_gsto_stop_v(jibal_gsto *workspace, int Z1, int Z2, double v) {
    gsto_file_t *file=jibal_gsto_get_file(workspace, Z1, Z2);
    if(!file) {
#ifdef DEBUG
        fprintf(stderr, "No stopping file assigned to Z1=%i Z2=%i\n", Z1, Z2);
#endif
        return 0.0;
    }
    double x=jibal_gsto_scale_velocity_to_x(file, v);
    if(x == 0.0) { /* X out of range. Yes, this kind of comparison is safe. */
        return 0.0;
    }

    /* Scale x to indices of tabulated stopping and interpolate */
    double i_float=jibal_gsto_xscale_to_index(file, x);
    int i = (int) floor(i_float);
    const double *data=jibal_gsto_file_get_data(file, Z1, Z2);
    double sto = jibal_linear_interpolation(1.0*i, 1.0*(i+1), data[i], data[i+1], i_float);
    return jibal_gsto_scale_y_to_stopping(file, sto);
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
        sum += target->concs[i]*gsto_sto_nuclear_universal(E, incident->Z, incident->mass, element->Z, element->avg_mass);
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

double jibal_layer_energy_loss(jibal_gsto *workspace, jibal_isotope *incident, jibal_layer *layer, double E_0) {
    double k1, k2, k3, k4;
    double E = E_0;
    double x;
    double h = workspace->stop_step;
    for (x = 0.0; x <= layer->thickness; x += h) {
        k1 = jibal_stop(workspace, incident, layer->material, E);
        k2 = jibal_stop(workspace, incident, layer->material, E + (h / 2) * k1);
        k3 = jibal_stop(workspace, incident, layer->material, E + (h / 2) * k2);
        k4 = jibal_stop(workspace, incident, layer->material, E + h * k3);
        E -= (h / 6) * (k1 + 2 * k2 + 2 * k3 + k4);
    }
    return E;
}

double jibal_stop_ele(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E) {
    int i;
    double sum = 0.0;
    for (i = 0; i < target->n_elements; i++) {
        jibal_element *element = &target->elements[i];
        sum += target->concs[i]*jibal_gsto_stop_v(workspace, incident->Z, element->Z, velocity(E, incident->mass));
    }
    return sum;
}

int jibal_stop_auto_assign(jibal_gsto *workspace, const jibal_isotope *incident, jibal_material *target) {
    int i;
    int success = 0;
    for (i = 0; i < target->n_elements; i++) {
        success += gsto_auto_assign(workspace, incident->Z, target->elements[i].Z);
    }
    return success;
}
