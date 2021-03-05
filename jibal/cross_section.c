#include <jibal_cross_section.h>
#include <jibal_units.h>
#include <jibal_phys.h>

double jibal_cross_section_rbs(const jibal_isotope *incident, const jibal_isotope *target, double theta, double E, jibal_cross_section_type type) {
    double E_cm = target->mass*E/(incident->mass + target->mass);
    double r = incident->mass/target->mass;
    double theta_cm = theta + asin(r*sin(theta));;
    double sigma_cm = pow2((incident->Z*C_E*target->Z*C_E)/(4.0*C_PI*C_EPSILON0))*pow2(1.0/(4.0*E_cm))*pow4(1.0/sin(theta_cm/2.0));
    double sigma_r = (sigma_cm*pow2(sin(theta_cm)))/(pow2(sin(theta))*cos(theta_cm - theta));
    switch (type) {
        case JIBAL_CS_RUTHERFORD:
            return sigma_r;
        case JIBAL_CS_ANDERSEN:
            return jibal_andersen_correction(incident->Z, target->Z, E_cm, theta_cm)*sigma_r;
        default:
            return sigma_r;
    }
}

double jibal_cross_section_erd(const jibal_isotope *incident, const jibal_isotope *target, double phi, double E, jibal_cross_section_type type) {
    double E_cm, theta_cm;
    double sigma_r = pow2(incident->Z*C_E*target->Z*C_E/(8*C_PI*C_EPSILON0*E))
            * pow2(1.0 + incident->mass/target->mass) * pow(cos(phi), -3.0);
    switch (type) {
        case JIBAL_CS_RUTHERFORD:
            return sigma_r;
        case JIBAL_CS_ANDERSEN:
            E_cm = target->mass*E/(incident->mass+target->mass);
            theta_cm = C_PI- 2 * phi;
            return jibal_andersen_correction(incident->Z, target->Z, E_cm, theta_cm)*sigma_r;
        default:
            return sigma_r;
    }
}

const char *jibal_cross_section_name(jibal_cross_section_type type) {
    return jibal_option_get_string(jibal_cs_types, type);
}

double jibal_andersen_correction(int z1, int z2, double E_cm, double theta_cm) {
    double r_VE = 48.73 * C_EV * z1 * z2 * sqrt(pow(z1, 2.0 / 3.0) + pow(z2, 2.0 / 3.0)) / E_cm;
    double F = pow2(1 + 0.5 * r_VE) / pow2(1 + r_VE + pow2(0.5 * r_VE / (sin(theta_cm / 2.0))));
    return F;
}
