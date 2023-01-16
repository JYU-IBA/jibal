#ifndef _JIBAL_GSTO_H_
#define _JIBAL_GSTO_H_

#include <stdio.h>
#include <jibal_masses.h>
#include <jibal_option.h>

#define GSTO_STR_NONE JIBAL_OPTION_STR_NONE


typedef enum {
    GSTO_STO_NONE=0,
    GSTO_STO_NUCL=1,
    GSTO_STO_ELE=2,
    GSTO_STO_TOT=3,
    GSTO_STO_STRAGG=4
} gsto_stopping_type; /* TODO: rename */

static const jibal_option gsto_stopping_types[] = {/* TODO: rename */
        {GSTO_STR_NONE, GSTO_STO_NONE},
        {"nuclear", GSTO_STO_NUCL},
        {"electronic", GSTO_STO_ELE},
        {"total", GSTO_STO_TOT},
        {"stragg", GSTO_STO_STRAGG},
        {NULL, 0}
};

typedef enum {
    GSTO_STO_UNIT_NONE=0,
    GSTO_STO_UNIT_EV15CM2=1, /* eV/(1e15 at/cm^2)*/
    GSTO_STO_UNIT_JM2=2, /* J m^2, the SI unit for stopping cross sections... */
} gsto_stounit;

static const jibal_option gsto_sto_units[] = {
        {GSTO_STR_NONE, GSTO_STO_UNIT_NONE},
        {"eV/(1e15 atoms/cm2)", GSTO_STO_UNIT_EV15CM2},
        {"Jm2", GSTO_STO_UNIT_JM2},
        {NULL, 0}
};

typedef enum {
    GSTO_STRAGG_UNIT_NONE=0,
    GSTO_STRAGG_UNIT_BOHR=1,
    GSTO_STRAGG_UNIT_J2M2, /* J^2 m^2, the SI unit for energy loss straggling variance (omega squared) */
} gsto_straggunit;

static const jibal_option gsto_stragg_units[] = {
        {GSTO_STR_NONE, GSTO_STRAGG_UNIT_NONE},
        {"bohr", GSTO_STRAGG_UNIT_BOHR},
        {"J2m2", GSTO_STRAGG_UNIT_J2M2},
        {NULL, 0}
};

typedef enum {
    GSTO_DF_NONE=0,
    GSTO_DF_ASCII=1,
    GSTO_DF_DOUBLE=2
} gsto_data_format;

static const jibal_option gsto_data_formats[] = {
        {GSTO_STR_NONE, GSTO_DF_NONE},
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

static const jibal_option gsto_xscales[] = {
        {GSTO_STR_NONE, GSTO_XSCALE_NONE},
        {"lin", GSTO_XSCALE_LINEAR},
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

static const jibal_option gsto_xunits[] = {
        {GSTO_STR_NONE, GSTO_X_UNIT_NONE},
        {"m/s", GSTO_X_UNIT_M_S},
        {"keV/u", GSTO_X_UNIT_KEV_U},
        {"MeV/u", GSTO_X_UNIT_MEV_U},
        {"J/kg", GSTO_X_UNIT_J_KG},
        {NULL, 0}
};

typedef enum {
    GSTO_HEADER_NONE=0,
    GSTO_HEADER_TYPE=1,
    GSTO_HEADER_SOURCE=2,
    GSTO_HEADER_Z1=3,
    GSTO_HEADER_Z1MIN=4,
    GSTO_HEADER_Z1MAX=5,
    GSTO_HEADER_Z2=6,
    GSTO_HEADER_Z2MIN=7,
    GSTO_HEADER_Z2MAX=8,
    GSTO_HEADER_STOUNIT=9,
    GSTO_HEADER_STRAGGUNIT=10,
    GSTO_HEADER_XUNIT=11,
    GSTO_HEADER_FORMAT=12,
    GSTO_HEADER_XMIN=13,
    GSTO_HEADER_XMAX=14,
    GSTO_HEADER_XPOINTS=15,
    GSTO_HEADER_XSCALE=16
} gsto_header_type;

static const jibal_option gsto_headers[] = {
        {GSTO_STR_NONE, GSTO_HEADER_NONE},
        {"type", GSTO_HEADER_TYPE},
        {"source", GSTO_HEADER_SOURCE},
        {"z1", GSTO_HEADER_Z1},
        {"z1-min", GSTO_HEADER_Z1MIN},
        {"z1-max", GSTO_HEADER_Z1MAX},
        {"z2", GSTO_HEADER_Z2},
        {"z2-min", GSTO_HEADER_Z2MIN},
        {"z2-max", GSTO_HEADER_Z2MAX},
        {"sto-unit", GSTO_HEADER_STOUNIT},
        {"stragg-unit", GSTO_HEADER_STRAGGUNIT},
        {"x-unit", GSTO_HEADER_XUNIT},
        {"format", GSTO_HEADER_FORMAT},
        {"x-min", GSTO_HEADER_XMIN},
        {"x-max", GSTO_HEADER_XMAX},
        {"x-points", GSTO_HEADER_XPOINTS},
        {"x-scale", GSTO_HEADER_XSCALE},
        {NULL, 0}
};

typedef struct {
    int valid;
    int lineno; /* Keep track of how many lines read */
    /* file contains stopping for Z1 = Z1_min .. Z1_max inclusive in Z2 = Z2_min .. Z2_max inclusive i.e. (Z1_max-Z1_min+1)*(Z2_max-Z2_min+1) combinations */
    int Z1_min;
    int Z2_min;
    int Z1_max;
    int Z2_max;
    size_t n_comb;
    size_t xpoints; /* How many points of stopping per Z1, Z2 combination */
    double xmin; /* The first point of stopping corresponds to x=xmin. In units of xunit.*/
    double xmin_original; /* xmin as read from file */
    double xmax; /* The last point of stopping corresponds to x=xmax. In units of xunit. */
    double xmax_original; /* xmax as read from file */
    double xmin_speedup; /* xmin or log10(xmin) */
    double xdiv; /* speedup variable, calculated from xpoints, xmin and xmax */
    double *em; /* Array of energy/mass (size: xpoints). In SI units! */
    int em_index_accel; /* Store the energy/mass bin (of the array above) that was last found in an attempt to
 * accelerate successive seeks */
    gsto_stopping_xscale xscale; /* The scale specifies how stopping points are spread between min and max (linear, log...) */
    gsto_xunit xunit; /* Stopping as a function of what? Can be converted (to SI) or not! */
    gsto_xunit xunit_original; /* As it was read. */
    gsto_stounit stounit; /* Stopping unit */
    gsto_stounit stounit_original;
    gsto_straggunit straggunit; /* Straggling unit */
    gsto_straggunit straggunit_original;
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
    int Z1;
    int Z2;
    gsto_file_t *file;
} gsto_assignment;

typedef struct {
    const jibal_element *elements;
    int Z1_max;
    int Z2_max;
    size_t n_files;
    size_t n_comb;
    gsto_file_t *files; /* table of gsto_file_t. Note: not a table of pointers. */
    gsto_file_t **stop_assignments; /* array of n_comb gsto_file_t pointers. For each Z1 and Z2 combination there can
 * be a file assigned. Access with functions. */
    gsto_file_t **stragg_assignments;
    gsto_assignment *overrides;
    double stop_step; /* as stopping cross section */
    int extrapolate; /* boolean */
} jibal_gsto;



#include <jibal_masses.h>
#include <jibal_material.h>
#include <jibal_layer.h>



jibal_gsto *jibal_gsto_init(const jibal_element *elements, int Z_max, const char *files_file_name,
                            const char *assignments_file_name);
int jibal_gsto_read_settings_file(jibal_gsto *workspace, const char *filename);
gsto_assignment *jibal_gsto_read_assignments_file(jibal_gsto *workspace, const char *filename);
int jibal_gsto_add_file(jibal_gsto *workspace, jibal_element *elements, const char *name, const char *filename);
int jibal_gsto_file_has_combination(gsto_file_t *file, int Z1, int Z2);
int jibal_gsto_assign(jibal_gsto *workspace, int Z1, int Z2, gsto_file_t *file);
void jibal_gsto_assign_clear_all(jibal_gsto *workspace);
int jibal_gsto_assign_range(jibal_gsto *workspace, int Z1_min, int Z1_max, int Z2_min, int Z2_max, gsto_file_t *file);
int jibal_gsto_assign_material(jibal_gsto *workspace, const jibal_isotope *incident, jibal_material *target,
        gsto_file_t *file);
int jibal_gsto_auto_assign(jibal_gsto *workspace, int Z1, int Z2);
int jibal_gsto_auto_assign_material(jibal_gsto *workspace, const jibal_isotope *incident, jibal_material *target);
int  jibal_gsto_file_count_assignments(jibal_gsto *workspace, gsto_file_t *file);
int jibal_gsto_print_files(jibal_gsto *workspace, int used_only);
int jibal_gsto_print_assignments(jibal_gsto *workspace);
const char *jibal_gsto_file_source(gsto_file_t *file);
void jibal_gsto_file_free(gsto_file_t *file);
void jibal_gsto_free(jibal_gsto *workspace);

int jibal_gsto_load(jibal_gsto *workspace, int headers_only, gsto_file_t *file);
int jibal_gsto_load_all(jibal_gsto *workspace);



/* The following are mostly internal */

jibal_gsto *jibal_gsto_allocate(int Z1_max, int Z2_max);
int jibal_gsto_load_ascii_file(jibal_gsto *workspace, gsto_file_t *file);
int jibal_gsto_load_binary_file(jibal_gsto *workspace, gsto_file_t *file);
void jibal_gsto_fprint_file(FILE *file_out, jibal_gsto *workspace, gsto_file_t *file, gsto_data_format format, int Z1_min, int Z1_max, int Z2_min, int Z2_max);

size_t jibal_gsto_file_get_data_index(gsto_file_t *file, int Z1, int Z2);
void jibal_gsto_file_calculate_ncombs(gsto_file_t *file);
double *jibal_gsto_file_allocate_data(gsto_file_t *file, int Z1, int Z2);
gsto_file_t *jibal_gsto_get_assigned_file(jibal_gsto *workspace, gsto_stopping_type type, int Z1, int Z2);
gsto_file_t *jibal_gsto_get_file(jibal_gsto *workspace, const char *name);
double jibal_gsto_em_from_file_units(double x, const gsto_file_t *file);
double *jibal_gsto_em_table(const gsto_file_t *file);
int jibal_gsto_velocity_to_index(const gsto_file_t *file, double v);
void jibal_gsto_calculate_speedups(gsto_file_t *file);
void jibal_gsto_convert_file_to_SI(gsto_file_t *file);
double jibal_gsto_get_em(jibal_gsto *workspace, gsto_stopping_type type, int Z1, int Z2, double em);

void jibal_gsto_fprint_header_property(FILE *f, gsto_header_type h, int val);
void jibal_gsto_fprint_header_int(FILE *f, gsto_header_type h, int i);
void jibal_gsto_fprint_header_string(FILE *f, gsto_header_type h, const char *str);
void jibal_gsto_fprint_header_scientific(FILE *f, gsto_header_type h, double val);
void jibal_gsto_fprint_header(FILE *f, gsto_header_type h, const void *val);
inline size_t jibal_gsto_table_get_index(jibal_gsto *workspace, int Z1, int Z2) {
    return (workspace->Z2_max * (Z1 - 1) + (Z2 - 1));
}
inline const double *jibal_gsto_file_get_data(gsto_file_t *file, int Z1, int Z2) {
    return file->data[jibal_gsto_file_get_data_index(file, Z1, Z2)];
}
#endif /* _JIBAL_GSTO_H_ */
