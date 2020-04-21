/* 
    Copyright (C) 2013-2020 Jaakko Julin <jaakko.julin@jyu.fi>
    See file LICENCE for a copy of the GNU General Public Licence
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <jibal_units.h>
#include <jibal_phys.h>

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
        double E = x/(C_KEV/C_U);
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
        if(S_elec == 0.0) {
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

void remove_newline(char *s) {
    int i;
    for(i=0; i<strlen(s); i++) {
        if(s[i] == '\n' || s[i] == '\r')
            s[i]='\0';
    }
}

int main (int argc, char **argv) {
    int Z1, Z2, i;
    double xmin=10.0; /* keV/amu */
    double xmax=10000.0; 
    int xsteps=XSTEPS; /* steps numbered 0, 1, 2, ...., vsteps-1 */
    int z1_min=1;
    int z2_min=1;
    int z1_max=Z_MAX; 
    int z2_max=Z_MAX;
    int n_combinations;
    FILE *stopping_output_file;
    i=0;
    char *input=malloc(sizeof(char)*1000);
    fprintf(stderr, "Please enter output filename, e.g. \"srim2013.ele\": ");
    fgets(input, 1000, stdin);
    remove_newline(input); 
    stopping_output_file = fopen(input, "w");
    if(!stopping_output_file) {
        fprintf(stderr, "Could not open file \"%s\" for output", input);
        return 0;
    }
#ifdef WIN32 
    fprintf(stderr, "Please enter SRIM path, e.g. \"C:\\SRIM\\SR Module\\\": ");
#else
    fprintf(stderr, "Since you are not running Windows, I'll try to use wine to run SRModule.exe. I need to know where it is.\nPlease enter SRIM path, e.g. \"%s/.wine/drive_c/SRIM/SR Module/\"\n> ", getenv("HOME")?getenv("HOME"):"/home/user/");
#endif
    char *srim_path=malloc(sizeof(char)*SRIM_PATH_MAXLEN);
    fgets(srim_path, SRIM_PATH_MAXLEN, stdin);
    remove_newline(srim_path);
    fprintf(stderr, "Attempting to chdir to \"%s\"\n", srim_path);
    if(chdir(srim_path) != 0) {
        fprintf(stderr, "Could not chdir to given path. Error number %i.\n", errno);
        return 0;
    }
    fprintf(stderr, "Input minimum energy in keV/u (e.g. 10): ");
    fgets(input, 1000, stdin);
    xmin=strtod(input, NULL);
    xmin=xmin*C_KEV/C_U;
    fprintf(stderr, "xmin=%g keV/u\n", xmin/(C_KEV/C_U));
    fprintf(stderr, "Input maximum energy in keV/u (e.g. 10000): ");
    fgets(input, 1000, stdin);
    xmax=strtod(input, NULL);
    xmax=xmax*C_KEV/C_U;
    fprintf(stderr, "xmax=%g keV/u\n", xmax/(C_KEV/C_U));
    fprintf(stderr, "Input number of stopping steps to calculate between xmin and xmax in log scale (e.g. 101): ");
    fgets(input, 1000, stdin);
    xsteps=strtol(input, NULL, 10);
    fprintf(stderr, "Input Z1 minimum (e.g. 1): ");
    fgets(input, 1000, stdin);
    z1_min=strtol(input, NULL, 10);
    fprintf(stderr, "Input Z1 maximum (e.g. 92): ");
    fgets(input, 1000, stdin);
    z1_max=strtol(input, NULL, 10);
    fprintf(stderr, "Input Z2 minimum (e.g. 1): ");
    fgets(input, 1000, stdin);
    z2_min=strtol(input, NULL, 10);
    fprintf(stderr, "Input Z2 maximum (e.g. 92): ");
    fgets(input, 1000, stdin);
    z2_max=strtol(input, NULL, 10);
    n_combinations = (z1_max-z1_min+1)*(z2_max-z2_min+1);
    fprintf(stopping_output_file, "source=srim\nz1-min=%i\nz1-max=%i\nz2-min=%i\nz2-max=%i\n"
                                  "sto-unit=eV/(1e15 atoms/cm2)\nx-unit=keV/u\nformat=ascii\n"
                                  "x-min=%e\nx-max=%e\nx-points=%i\nx-scale=log10\n"
                                  "==END-OF-HEADER==\n", z1_min, z1_max, z2_min, z2_max, xmin, xmax, xsteps);
    i=0;
    fprintf(stderr, "\n");
    for(Z1=z1_min; Z1<=z1_max; Z1++) {
        for(Z2=z2_min; Z2<=z2_max; Z2++) {
            i++;
            generate_sr_in(SR_FILE_PATH, Z1, Z2, xsteps, xmin, xmax);
            if(run_srim(SR_MODULE_PATH)) {
                if(parse_output(SR_OUTPUT_FILE, stopping_output_file, xsteps)) {
                    fprintf(stderr, "\rZ1=%i. Z2=%i.     OK. %3i/%i.\n", Z1, Z2, i, n_combinations);
                } else {
                    fprintf(stderr, "\rZ1=%i. Z2=%i. Not OK. %3i/%i.\n", Z1, Z2, i, n_combinations);
                    exit(0);
                }
            } else {
                fprintf(stderr, "\nError in running SRModule.\n");
                exit(0);
            }
        }
        sleep(1);
    }
    fprintf(stderr, "\n");
    return 1;
}
