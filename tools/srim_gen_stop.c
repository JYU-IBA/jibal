/*
    Copyright (C) 2013-2020 Jaakko Julin <jaakko.julin@jyu.fi>
    See file LICENCE for a copy of the GNU General Public Licence
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#ifdef WIN32
#include <direct.h> // We need _chdir from here
#else
#include <unistd.h>
#endif
#include <errno.h>
#include <jibal_units.h>
#include <jibal_phys.h>
#include <jibal_gsto.h>

#ifdef WIN32
#include "win_compat.h"
#endif



#define SRIM_OUTPUT_N_HEADERS 4 /* this number of rows in the beginning of SR Module output are headers */
#define SRIM_OUTPUT_N_COLS 3
#define SRIM_OUTPUT_LINE_LENGTH 80 /* max length of line */

#ifdef WIN32
#define SR_MODULE_PATH "SRModule.exe"
#else
#define SR_MODULE_PATH "wine SRModule.exe"
#endif
#define SR_FILE_PATH "SR.IN"
#define SR_OUTPUT_FILE "stopping.dat"

#define XSTEPS 101
#define Z_MAX 92

#define INCLUDE_NUCLEAR 0

#define SRIM_PATH_MAXLEN 2048
#define SRIM_GEN_INPUT_MAX_LEN 1000

typedef struct {
    double xmin; /* keV/amu */
    double xmax;
    int xsteps; /* steps numbered 0, 1, 2, ...., vsteps-1 */
    int z1_min;
    int z2_min;
    int z1_max;
    int z2_max;
    int n_combinations;
    char *out_filename;
} srim_gen_settings;

void remove_newline(char *s) {
    size_t i;
    for(i=0; i<strlen(s); i++) {
        if(s[i] == '\n' || s[i] == '\r')
            s[i]='\0';
    }
}

srim_gen_settings *read_settings(FILE *f) {
    srim_gen_settings *settings = calloc(1, sizeof(srim_gen_settings));
    settings->xmin = 10.0; /* keV/amu */
    settings->xmax = 10000.0;
    settings->xsteps = XSTEPS; /* steps numbered 0, 1, 2, ...., vsteps-1 */
    settings->z1_min = 1;
    settings->z2_min = 1;
    settings->z1_max = Z_MAX;
    settings->z2_max = Z_MAX;
    settings->n_combinations = 0;
    char *input = malloc(sizeof(char)*SRIM_GEN_INPUT_MAX_LEN);
    fprintf(stderr, "Please enter output filename, e.g. \"srim2013.ele\": ");
    fgets(input, SRIM_GEN_INPUT_MAX_LEN, f);
    remove_newline(input);
    settings->out_filename = strdup(input);
    fprintf(stderr, "Input minimum energy in keV/u (e.g. 10): ");
    input = fgets(input, SRIM_GEN_INPUT_MAX_LEN, f);
    if(!input) {
        return NULL;
    }
    settings->xmin=strtod(input, NULL);
    fprintf(stderr, "xmin=%g keV/u\n", settings->xmin);
    fprintf(stderr, "Input maximum energy in keV/u (e.g. 10000): ");
    input = fgets(input, SRIM_GEN_INPUT_MAX_LEN, f);
    if(!input) {
        return NULL;
    }
    settings->xmax=strtod(input, NULL);
    fprintf(stderr, "xmax=%g keV/u\n", settings->xmax);
    fprintf(stderr, "Input number of stopping steps to calculate between xmin and xmax in log scale (e.g. 101): ");
    input = fgets(input, SRIM_GEN_INPUT_MAX_LEN, f);
    if(!input) {
        return NULL;
    }
    settings->xsteps=strtol(input, NULL, 10);
    fprintf(stderr, "Input Z1 minimum (e.g. 1): ");
    input = fgets(input, SRIM_GEN_INPUT_MAX_LEN, f);
    if(!input) {
        return NULL;
    }
    settings->z1_min=strtol(input, NULL, 10);
    fprintf(stderr, "Input Z1 maximum (e.g. 92): ");
    input = fgets(input, SRIM_GEN_INPUT_MAX_LEN, f);
    if(!input) {
        return NULL;
    }
    settings->z1_max=strtol(input, NULL, 10);
    fprintf(stderr, "Input Z2 minimum (e.g. 1): ");
    input = fgets(input, SRIM_GEN_INPUT_MAX_LEN, f);
    if(!input) {
        return NULL;
    }
    settings->z2_min=strtol(input, NULL, 10);
    fprintf(stderr, "Input Z2 maximum (e.g. 92): ");
    input = fgets(input, SRIM_GEN_INPUT_MAX_LEN, f);
    if(!input) {
        return NULL;
    }
    settings->z2_max=strtol(input, NULL, 10);
    settings->n_combinations = (settings->z1_max - settings->z1_min + 1)*(settings->z2_max - settings->z2_min + 1);
    free(input);
    return settings;
}

int generate_sr_in(char *out_filename, int Z1, int Z2, int xsteps, double xmin, double xmax) {
    FILE *sr_file = fopen(out_filename, "w");
    int i;
    double x;
    if(!sr_file) {
        fprintf(stderr, "Could not open SR.IN for writing!\n");
        return 0;
    }
    fprintf(sr_file, "---Stopping/Range Input Data (Number-format: Period = Decimal Point)\r\n");
    fprintf(sr_file, "---Output File Name\r\n");
    fprintf(sr_file, "\"%s\"\r\n", SR_OUTPUT_FILE);
    fprintf(sr_file, "---Ion(Z), Ion Mass(u)\r\n");
    fprintf(sr_file, "%i   1.0\r\n", Z1);
    fprintf(sr_file, "---Target Data: (Solid=0,Gas=1), Density(g/cm3), Compound Corr.\r\n");
    fprintf(sr_file, "0    1      1\r\n");
    fprintf(sr_file, "---Number of Target Elements\r\n");
    fprintf(sr_file, "1\r\n");
    fprintf(sr_file, "---Target Elements: (Z), Target name, Stoich, Target Mass(u)\r\n");
    fprintf(sr_file, "%i   \"\"   100   1.0\r\n", Z2);
    fprintf(sr_file, "---Output Stopping Units (1-8)\r\n");
    fprintf(sr_file, "7\r\n");
    fprintf(sr_file, "---Ion Energy : E-Min(keV), E-Max(keV)\r\n");
    fprintf(sr_file, "0  0\r\n");
    for(i=0; i<xsteps; i++) {
        x=xmin*pow(xmax/xmin,1.0*i/(xsteps-1)); /* log scale */
        double E = x;
        fprintf(sr_file, "%lf\r\n", E);
    }
    fclose(sr_file);
    return 1;
}

int run_srim(char *sr_module_path) {
    int result;
    result=system(sr_module_path);
    if(result==-1 || result==127)
        return 0;
    return 1;
}


char *fix_exponential_notation(char *str) {
    char *s;
    for(s=str; *s != '\0'; s++) {
        if(*s == ',')
            *s = '.';
        if(*s == 'E')
            *s = 'e';
    }
    return str;
}

int parse_output(char *filename, FILE *stopping_output_file, int xsteps) {
    int lineno=0, i=0;
    FILE *in_file=fopen(filename, "r");
    char *columns[SRIM_OUTPUT_N_COLS];
    char **col;
    if(!in_file)
        return 0;
    char *line=malloc(sizeof(char)*SRIM_OUTPUT_LINE_LENGTH);
    char *line_split;
    while(fgets(line, SRIM_OUTPUT_LINE_LENGTH, in_file) != NULL) {
        lineno++;
        if(lineno<=SRIM_OUTPUT_N_HEADERS) /* Headers */
            continue; 
        line_split=line; /* strsep will screw up line_split, reset for every new line */
        for (col = columns; (*col = strsep(&line_split, " \t")) != NULL;)
            if (**col != '\0')
                if (++col >= &columns[SRIM_OUTPUT_N_COLS])
                    break;
    
        double energy=strtod(fix_exponential_notation(columns[0]), NULL);
        double S_elec=strtod(fix_exponential_notation(columns[1]), NULL);
        if(S_elec == 0.0 || energy == 0.0) {
            fprintf(stderr, "Problems.\n");
            return 0;
        }
#if INCLUDE_NUCLEAR /* We use m1=1u and m2=1u so this doesn't make much sense */
        double S_nuc=strtod(fix_exponential_notation(columns[2]), NULL);
        fprintf(stopping_output_file, "%e\n", S_elec+S_nuc);
#else
        fprintf(stopping_output_file, "%.3e\n", S_elec); /* Three is the magic number of decimals */
#endif
        i++;
    }
    fflush(stopping_output_file);
    free(line);
    fclose(in_file);
    if(i==xsteps) {
        return i;
    } else {
        return 0;
    }
}

void print_headers(FILE *stopping_output_file, const srim_gen_settings *settings) {
    jibal_gsto_fprint_header_property(stopping_output_file, GSTO_HEADER_TYPE, GSTO_STO_ELE);
    jibal_gsto_fprint_header_string(stopping_output_file, GSTO_HEADER_SOURCE, "SRIM");
    jibal_gsto_fprint_header(stopping_output_file, GSTO_HEADER_Z1MIN, &settings->z1_min);
    jibal_gsto_fprint_header(stopping_output_file, GSTO_HEADER_Z1MAX, &settings->z1_max);
    jibal_gsto_fprint_header(stopping_output_file, GSTO_HEADER_Z2MIN, &settings->z2_min);
    jibal_gsto_fprint_header(stopping_output_file, GSTO_HEADER_Z2MAX, &settings->z2_max);
    jibal_gsto_fprint_header_property(stopping_output_file, GSTO_HEADER_STOUNIT, GSTO_STO_UNIT_EV15CM2);
    jibal_gsto_fprint_header_property(stopping_output_file, GSTO_HEADER_XUNIT, GSTO_X_UNIT_KEV_U);
    jibal_gsto_fprint_header_property(stopping_output_file, GSTO_HEADER_FORMAT, GSTO_DF_ASCII);
    jibal_gsto_fprint_header(stopping_output_file, GSTO_HEADER_XMIN, &settings->xmin);
    jibal_gsto_fprint_header(stopping_output_file, GSTO_HEADER_XMAX, &settings->xmax);
    jibal_gsto_fprint_header_property(stopping_output_file, GSTO_HEADER_XSCALE, GSTO_XSCALE_LOG10);
    jibal_gsto_fprint_header(stopping_output_file, GSTO_HEADER_XPOINTS, &settings->xsteps);
    fprintf(stopping_output_file, "\n");
}

int generate_output(FILE *stopping_output_file, srim_gen_settings *settings) {
    int Z1, Z2, i;
    i = 0;
    for(Z1 = settings->z1_min; Z1 <= settings->z1_max; Z1++) {
        for(Z2 = settings->z2_min; Z2 <= settings->z2_max; Z2++) {
            i++;
            generate_sr_in(SR_FILE_PATH, Z1, Z2, settings->xsteps, settings->xmin, settings->xmax);
            if(run_srim(SR_MODULE_PATH)) {
                if(parse_output(SR_OUTPUT_FILE, stopping_output_file, settings->xsteps)) {
                    fprintf(stderr, "\rZ1 = %3i. Z2 = %3i.     OK. %3i/%i.\n", Z1, Z2, i, settings->n_combinations);
                } else {
                    fprintf(stderr, "\rZ1 = %3i. Z2 = %3i. Not OK. %3i/%i.\n", Z1, Z2, i, settings->n_combinations);
                    return EXIT_FAILURE;
                }
            } else {
                fprintf(stderr, "\nError in running SRModule.\n");
                return EXIT_FAILURE;
            }
        }
#ifndef WIN32 /* We are too lazy to sleep on Windows */
        sleep(1);
#endif
    }
    fprintf(stderr, "\n");
    return EXIT_SUCCESS;
}

int main (void) {
    srim_gen_settings *settings = read_settings(stdin);
    if(!settings) {
        fprintf(stderr, "Could not read all required settings.\n");
        return EXIT_FAILURE;
    }
    FILE *stopping_output_file = fopen(settings->out_filename, "w");
    if(!stopping_output_file) {
        fprintf(stderr, "Could not open file \"%s\" for output", settings->out_filename);
        return 0;
    }
#ifdef WIN32 
    fprintf(stderr, "Please enter SRIM path, e.g. \"C:\\SRIM\\SR Module\\\": ");
#else
    fprintf(stderr, "Since you are not running Windows, I'll try to use wine to run SRModule.exe. I need to know where it is.\nPlease enter SRIM path, e.g. \"%s/.wine/drive_c/SRIM/SR Module/\"\n> ", getenv("HOME")?getenv("HOME"):"/home/user/");
#endif
    char *srim_path = malloc(sizeof(char) * SRIM_PATH_MAXLEN);
    fgets(srim_path, SRIM_PATH_MAXLEN, stdin);
    remove_newline(srim_path);
    fprintf(stderr, "Attempting to chdir to \"%s\"\n", srim_path);
    if(chdir(srim_path) != 0) {
        fprintf(stderr, "Could not chdir to given path. Error number %i.\n", errno);
        return EXIT_FAILURE;
    }
    generate_output(stopping_output_file, settings);
    fclose(stopping_output_file);
    free(settings->out_filename);
    free(settings);
    return EXIT_SUCCESS;
}
