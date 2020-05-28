#include <jibal_cross_section.h>
#include <jibal_units.h>

double jibal_cs_rbs(const jibal_isotope *incident, const jibal_isotope *target, double theta, double E, jibal_cross_section_type type) {
    double E_cm = target->mass*E/(incident->mass + target->mass);
    double r = incident->mass/target->mass;
    double theta_cm = theta + asin(r*sin(theta));;
    double sigma_cm = pow((incident->Z*C_E*target->Z*C_E)/(4.0*C_PI*C_EPSILON0), 2.0)*pow(1.0/(4.0*E_cm), 2.0)*pow(1.0/sin(theta_cm/2.0),4.0);
    double sigma_r = (sigma_cm*pow(sin(theta_cm), 2.0))/(pow(sin(theta), 2.0)*cos(theta_cm - theta));
    switch (type) {
        case JIBAL_CS_RUTHERFORD:
            return sigma_r;
        case JIBAL_CS_ANDERSEN:
            return jibal_andersen_correction(incident->Z, target->Z, E_cm, theta_cm)*sigma_r;
        default:
            return sigma_r;
    }
}

double jibal_cs_erd(const jibal_isotope *incident, const jibal_isotope *target, double theta, double E, jibal_cross_section_type type) {
    double E_cm, theta_cm;
    double sigma_r = pow(incident->Z*C_E*target->Z*C_E/(8*C_PI*C_EPSILON0*E), 2.0)
            *pow(1.0 + incident->mass/target->mass, 2.0)*pow(cos(theta), -3.0);
    switch (type) {
        case JIBAL_CS_RUTHERFORD:
            return sigma_r;
        case JIBAL_CS_ANDERSEN:
            E_cm = target->mass*E/(incident->mass+target->mass);
            theta_cm = C_PI-2*theta;
            return jibal_andersen_correction(incident->Z, target->Z, E_cm, theta_cm)*sigma_r;
        default:
            return sigma_r;
    }
}

const char *jibal_cs_name(jibal_cross_section_type type) {
    switch (type) {
        case JIBAL_CS_RUTHERFORD:
            return "Rutherford";
        case JIBAL_CS_ANDERSEN:
            return "Andersen";
        default:
            return "Unknown";
    }
}

double jibal_andersen_correction(int z1, int z2, double E_cm, double theta_cm) {
    double r_VE = 48.73 * C_EV * z1 * z2 * sqrt(pow(z1, 2.0 / 3.0) + pow(z2, 2.0 / 3.0)) / E_cm;
    double F = pow(1 + 0.5 * r_VE, 2.0) / pow(1 + r_VE + pow(0.5 * r_VE / (sin(theta_cm / 2.0)), 2.0), 2.0);
    return F;
}
