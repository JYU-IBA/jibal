#ifndef _JIBAL_GSTO_H_
#define _JIBAL_GSTO_H_

#include <jibal_masses.h>
#include <jibal_material.h>

#define GSTO_MAX_LINE_LEN 1024
#define GSTO_END_OF_HEADERS "==END-OF-HEADER=="
#define GSTO_DATA_PATH_ENV_VARIABLE "POTKU_DATA_PATH"
#define GSTO_DATA_DEFAULT_FILE JIBAL_DATADIR "stoppings.txt"
#define GSTO_N_STOPPING_TYPES 4
typedef enum {
    STO_NONE=0,
    STO_NUCL=1,
    STO_ELE=2,
    STO_TOT=3
} stopping_type_t;

#define GSTO_N_DATA_FORMATS 2
typedef enum {
    GSTO_DF_NONE=0,
    GSTO_DF_ASCII=1,
    GSTO_DF_DOUBLE=2
} stopping_data_format_t;

#define GSTO_N_X_SCALES 3
typedef enum {
    GSTO_XSCALE_NONE=0,
    GSTO_XSCALE_LINEAR=1,
    GSTO_XSCALE_LOG10=2
} stopping_xscale_t;

#define GSTO_N_X_UNITS 3
typedef enum {
    GSTO_X_UNIT_NONE=0,
    GSTO_X_UNIT_M_S=1, /* m/s */
    GSTO_X_UNIT_KEV_U=2, /* keV/u */
} stopping_xunit_t;

#define GSTO_N_STO_UNITS 2
typedef enum {
    GSTO_STO_UNIT_NONE=0,
    GSTO_STO_UNIT_EV15CM2=1, /* eV/(1e15 at/cm^2)*/
} stopping_stounit_t;

#define GSTO_N_HEADER_TYPES 13
typedef enum {
    GSTO_HEADER_NONE=0,
    GSTO_HEADER_SOURCE=1,
    GSTO_HEADER_Z1MIN=2,
    GSTO_HEADER_Z1MAX=3,
    GSTO_HEADER_Z2MIN=4,
    GSTO_HEADER_Z2MAX=5,
    GSTO_HEADER_STOUNIT=6,
    GSTO_HEADER_XUNIT=7,
    GSTO_HEADER_FORMAT=8,
    GSTO_HEADER_XMIN=9,
    GSTO_HEADER_XMAX=10,
    GSTO_HEADER_XPOINTS=11,
    GSTO_HEADER_XSCALE=12
} header_properties_t;

typedef struct {
    int lineno; /* Keep track of how many lines read */
    /* file contains stopping for Z1 = Z1_min .. Z1_max inclusive in Z2 = Z2_min .. Z2_max inclusive i.e. (Z1_max-Z1_min+1)*(Z2_max-Z2_min+1) combinations */
    int Z1_min; 
    int Z2_min;
    int Z1_max;
    int Z2_max;
    int n_comb;
    int xpoints; /* How many points of stopping per Z1, Z2 combination */
    double xmin; /* The first point of stopping corresponds to x=xmin */
    double xmax; /* The last point of stopping corresponds to x=xmax */
    stopping_xscale_t xscale; /* The scale specifies how stopping points are spread between min and max (linear, log...) */
    stopping_xunit_t xunit; /* Stopping as a function of what? */
    stopping_stounit_t stounit; /* Stopping unit */
    stopping_type_t type; /* does this file contain nuclear, electronic or total stopping? */
    stopping_data_format_t data_format; /* What does the data look like (after headers) */
    FILE *fp;
    char *name; /* Descriptive name of the file, from the settings file */
    char *filename; /* Filename (relative or full path, whatever fopen can chew) */
    double **data; /* Data is stored here. Array of pointers. Access with functions. */
} gsto_file_t;

typedef struct {
    int Z1_max;
    int Z2_max;
    int n_files;
    int n_comb;
    gsto_file_t *files; /* table of gsto_file_t */
    gsto_file_t **assignments; /* TODO: what is this */
} jibal_gsto;


jibal_gsto *gsto_init(int Z_max, char *stoppings_file_name);
int gsto_add_file(jibal_gsto *workspace, char *name, char *filename, int Z1_min, int Z1_max, int Z2_min, int Z2_max, char *type);
int jibal_gsto_assign(jibal_gsto *workspace, int Z1, int Z2, gsto_file_t *file);
int gsto_load_binary_file(jibal_gsto *workspace, gsto_file_t *file);
int gsto_load_ascii_file(jibal_gsto *workspace, gsto_file_t *file);
int gsto_load(jibal_gsto *workspace);

int jibal_gsto_table_get_index(jibal_gsto *workspace, int Z1, int Z2);
int jibal_gsto_print_files(jibal_gsto *workspace);
int jibal_gsto_print_assignments(jibal_gsto *workspace);


double jibal_gsto_scale_velocity_to_x(const gsto_file_t *file, double v); /* from SI units */
double jibal_gsto_scale_y_to_stopping(const gsto_file_t *file, double y); /* to SI units */
double gsto_sto_v(jibal_gsto *workspace, int Z1, int Z2, double v);
double *gsto_sto_v_table(jibal_gsto *workspace, int Z1, int Z2, double v_min, double v_max, int points);
double gsto_sto_nuclear_universal(double E, int Z1, double m1, int Z2, double m2);


int jibal_gsto_file_get_data_index(gsto_file_t *file, int Z1, int Z2);
const double *jibal_gsto_file_get_data(gsto_file_t *file, int Z1, int Z2);
double *jibal_gsto_file_allocate_data(gsto_file_t *file, int Z1, int Z2);

gsto_file_t *jibal_gsto_get_file(jibal_gsto *workspace, int Z1, int Z2);

double jibal_stop(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E);
double jibal_stop_ele(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E);
double jibal_stop_nuc(const jibal_isotope *incident, const jibal_material *target, double E);
int jibal_stop_auto_assign(jibal_gsto *workspace, const jibal_isotope *incident, jibal_material *target); /* TODO: energy range */

void jibal_gsto_file_free(gsto_file_t *file);
void jibal_gsto_free(jibal_gsto *workspace);


/* The following are mostly internal */
jibal_gsto *gsto_allocate(int Z1_max, int Z2_max);
#endif
