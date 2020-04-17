#ifndef _JIBAL_GSTO_H_
#define _JIBAL_GSTO_H_

#include <stdio.h>

#define GSTO_MAX_LINE_LEN 1024
#define GSTO_END_OF_HEADERS "==END-OF-HEADER=="

typedef struct {
    const char *s;
    int val;
} gsto_header; /* Association between strings and ints (ints preferably from enums). */

typedef enum {
    GSTO_STO_NONE=0,
    GSTO_STO_NUCL=1,
    GSTO_STO_ELE=2,
    GSTO_STO_TOT=3
} gsto_stopping_type;

static const gsto_header gsto_stopping_types[] = {
        {"none", GSTO_STO_NONE},
        {"nuclear", GSTO_STO_NUCL},
        {"electronic", GSTO_STO_ELE},
        {"total", GSTO_STO_TOT},
        {NULL, 0}
};

typedef enum {
    GSTO_STO_UNIT_NONE=0,
    GSTO_STO_UNIT_EV15CM2=1, /* eV/(1e15 at/cm^2)*/
    GSTO_STO_UNIT_JM2=2, /* J m^2, the SI unit for stopping cross sections... */
} gsto_stounit;

static const gsto_header gsto_sto_units[] = {
        {"none", GSTO_STO_UNIT_NONE},
        {"eV/(1e15 atoms/cm2)", GSTO_STO_UNIT_EV15CM2},
        {"Jm2", GSTO_STO_UNIT_JM2},
        {NULL, 0}
};

typedef enum {
    GSTO_DF_NONE=0,
    GSTO_DF_ASCII=1,
    GSTO_DF_DOUBLE=2
} gsto_data_format;

static const gsto_header gsto_data_formats[] = {
        {"none", GSTO_DF_NONE},
        {"ascii", GSTO_DF_ASCII},
        {"binary", GSTO_DF_DOUBLE},
        {NULL, 0}
};

typedef enum {
    GSTO_XSCALE_NONE=0,
    GSTO_XSCALE_LINEAR=1,
    GSTO_XSCALE_LOG10=2,
    GSTO_XSCALE_ARBITRARY=3
} gsto_stopping_xscale;

static const gsto_header gsto_xscales[] = {
        {"none", GSTO_XSCALE_NONE},
        {"linear", GSTO_XSCALE_LINEAR},
        {"log10", GSTO_XSCALE_LOG10},
        {"arb", GSTO_XSCALE_ARBITRARY},
        {NULL, 0}
};

typedef enum {
    GSTO_X_UNIT_NONE=0,
    GSTO_X_UNIT_M_S=1, /* m/s */
    GSTO_X_UNIT_KEV_U=2, /* keV/u */
    GSTO_X_UNIT_MEV_U=3, /* MeV/u */
    GSTO_X_UNIT_J_KG=4, /* J/kg = m^2/s^2 (N.B. this is NOT velocity squared!)*/
} gsto_xunit;

static const gsto_header gsto_xunits[] = {
        {"none", GSTO_X_UNIT_NONE},
        {"m/s", GSTO_X_UNIT_M_S},
        {"keV/u", GSTO_X_UNIT_KEV_U},
        {"MeV/u", GSTO_X_UNIT_MEV_U},
        {"J/kg", GSTO_X_UNIT_J_KG},
        {NULL, 0}
};

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
} gsto_header_type;
#define GSTO_N_HEADER_TYPES 13

static const gsto_header gsto_headers[] = {
        {"      ", GSTO_HEADER_NONE},
        {"source", GSTO_HEADER_SOURCE},
        {"z1-min", GSTO_HEADER_Z1MIN},
        {"z1-max", GSTO_HEADER_Z1MAX},
        {"z2-min", GSTO_HEADER_Z2MIN},
        {"z2-max", GSTO_HEADER_Z2MAX},
        {"sto-unit", GSTO_HEADER_STOUNIT},
        {"x-unit", GSTO_HEADER_XUNIT},
        {"format", GSTO_HEADER_FORMAT},
        {"x-min", GSTO_HEADER_XMIN},
        {"x-max", GSTO_HEADER_XMAX},
        {"x-points", GSTO_HEADER_XPOINTS},
        {"x-scale", GSTO_HEADER_XSCALE},
        {NULL, 0}
};

typedef struct {
    int lineno; /* Keep track of how many lines read */
    /* file contains stopping for Z1 = Z1_min .. Z1_max inclusive in Z2 = Z2_min .. Z2_max inclusive i.e. (Z1_max-Z1_min+1)*(Z2_max-Z2_min+1) combinations */
    int Z1_min; 
    int Z2_min;
    int Z1_max;
    int Z2_max;
    int n_comb;
    int xpoints; /* How many points of stopping per Z1, Z2 combination */
    double xmin; /* The first point of stopping corresponds to x=xmin. N.B.: in units of the file, not converted to SI*/
    double xmax; /* The last point of stopping corresponds to x=xmax */
    double xmin_speedup; /* xmin or log10(xmin) */
    double xdiv; /* speedup variable, calculated from xpoints, xmin and xmax */
    double *em; /* Array of energy/mass (size: xpoints). In SI units! */
    int em_index_accel; /* Store the energy/mass bin (of the array above) that was last found in an attempt to
 * accelerate successive seeks */
    gsto_stopping_xscale xscale; /* The scale specifies how stopping points are spread between min and max (linear, log...) */
    gsto_xunit xunit; /* Stopping as a function of what? */
    gsto_stounit stounit; /* Stopping unit */
    gsto_stopping_type type; /* does this file contain nuclear, electronic or total stopping? TODO: only electronic
 * makes sense, remove others */
    gsto_data_format data_format; /* What does the data look like (after headers) */
    FILE *fp;
    char *name; /* Descriptive name of the file, from the settings file */
    char *source; /* Source of data (meta data from the file) */
    char *filename; /* Filename (relative or full path, whatever fopen can chew) */
    double **data; /* Data is stored here. Array of pointers. Access with functions. */
} gsto_file_t;

typedef struct {
    int Z1_max;
    int Z2_max;
    int n_files;
    int n_comb;
    gsto_file_t *files; /* table of gsto_file_t. Note: not a table of pointers. */
    gsto_file_t **assignments; /* array of n_comb gsto_file_t pointers. For each Z1 and Z2 combination there can be a
 * file assigned. Access with functions. */
    double stop_step; /* as stopping cross section */
    int extrapolate; /* boolean */
} jibal_gsto;



#include <jibal_masses.h>
#include <jibal_material.h>
#include <jibal_layer.h>



jibal_gsto *jibal_gsto_init(int Z_max, const char *datadir, const char *stoppings_file_name);
int gsto_add_file(jibal_gsto *workspace, const char *name, const char *filename);
int jibal_gsto_assign(jibal_gsto *workspace, int Z1, int Z2, gsto_file_t *file);
int jibal_gsto_assign_range(jibal_gsto *workspace, int Z1_min, int Z1_max, int Z2_min, int Z2_max, gsto_file_t *file);
int jibal_gsto_assign_material(jibal_gsto *workspace, const jibal_isotope *incident, jibal_material *target,
        gsto_file_t *file);
int jibal_gsto_auto_assign(jibal_gsto *workspace, int Z1, int Z2);
int jibal_gsto_auto_assign_material(jibal_gsto *workspace, const jibal_isotope *incident, jibal_material *target);
int jibal_gsto_print_files(jibal_gsto *workspace);
int jibal_gsto_print_assignments(jibal_gsto *workspace);
const char *jibal_gsto_file_source(gsto_file_t *file);
void jibal_gsto_file_free(gsto_file_t *file);
void jibal_gsto_free(jibal_gsto *workspace);

int jibal_gsto_load(jibal_gsto *workspace, gsto_file_t *file);
int jibal_gsto_load_all(jibal_gsto *workspace);

double jibal_stop(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E);
double jibal_stop_ele(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E);
double jibal_stop_nuc(const jibal_isotope *incident, const jibal_material *target, double E); /* TODO: energy range */

double jibal_layer_energy_loss(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_layer *layer, double
E, double
factor);

/* The following are mostly internal */

jibal_gsto *jibal_gsto_allocate(int Z1_max, int Z2_max);
int jibal_gsto_load_ascii_file(jibal_gsto *workspace, gsto_file_t *file);
int jibal_gsto_load_binary_file(jibal_gsto *workspace, gsto_file_t *file);
void jibal_gsto_fprint_file(FILE *file_out, gsto_file_t *file, gsto_data_format format, int Z1_min, int Z1_max,
        int Z2_min, int Z2_max);

int jibal_gsto_file_get_data_index(gsto_file_t *file, int Z1, int Z2);
const double *jibal_gsto_file_get_data(gsto_file_t *file, int Z1, int Z2);
double *jibal_gsto_file_allocate_data(gsto_file_t *file, int Z1, int Z2);
gsto_file_t *jibal_gsto_get_assigned_file(jibal_gsto *workspace, int Z1, int Z2);
gsto_file_t *jibal_gsto_get_file(jibal_gsto *workspace, const char *name);

int jibal_gsto_table_get_index(jibal_gsto *workspace, int Z1, int Z2);
double jibal_gsto_em_from_file_units(double x, const gsto_file_t *file);
double *jibal_gsto_em_table(const gsto_file_t *file);
int jibal_gsto_velocity_to_index(const gsto_file_t *file, double v);
double jibal_gsto_scale_y_to_stopping(const gsto_file_t *file, double y); /* to SI units */
double jibal_gsto_stop_v(jibal_gsto *workspace, int Z1, int Z2, double v);
double jibal_gsto_stop_em(jibal_gsto *workspace, int Z1, int Z2, double Em);
double jibal_gsto_stop_nuclear_universal(double E, int Z1, double m1, int Z2, double m2);

#endif /* _JIBAL_GSTO_H_ */
