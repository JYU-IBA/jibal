#include <jibal_cs.h>

double jibal_cs_rbs(const jibal_config *config, const jibal_isotope *incident, const jibal_isotope *target, double theta, double E) {
    return jibal_cross_section_rbs(incident, target, theta, E, config->cs_rbs);
}
double jibal_cs_erd(const jibal_config *config, const jibal_isotope *incident, const jibal_isotope *target, double phi, double E) {
    return jibal_cross_section_erd(incident, target, phi, E, config->cs_erd);
}

const char *jibal_cs_rbs_name(const jibal_config *config) {
    return jibal_cross_section_name(config->cs_rbs);
}

const char *jibal_cs_erd_name(const jibal_config *config) {
    return jibal_cross_section_name(config->cs_erd);
}
