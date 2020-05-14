#include <jibal_cross_section.h>
#include <jibal_units.h>

double jibal_erd_cross_section(const jibal_isotope *incident, const jibal_isotope *target, double theta, double E) {
    double E_cm = target->mass*E/(incident->mass+target->mass);
    double t_sc=C_PI-2*theta;
    double sigma_r = pow(incident->Z*C_E*target->Z*C_E/(8*C_PI*C_EPSILON0*E), 2.0)
            *pow(1.0 + incident->mass/target->mass, 2.0)*pow(cos(theta), -3.0);
    /* TODO: screening */
    return sigma_r;
}

double jibal_rbs_cross_section(const jibal_isotope *incident, const jibal_isotope *target, double theta, double E) {
    double E_cm = target->mass*E/(incident->mass + target->mass);
    double r = incident->mass/target->mass;
    double t_cm = theta + asin(r*sin(theta));;
    double sigma_cm = pow((incident->Z*C_E*target->Z*C_E)/(4.0*C_PI*C_EPSILON0), 2.0)*pow(1.0/(4.0*E_cm), 2.0)*pow(1.0/sin(t_cm/2.0),4.0);
    double sigma_r = (sigma_cm*pow(sin(t_cm), 2.0))/(pow(sin(theta), 2.0)*cos(t_cm - theta));
    /* TODO: screening */
    return(sigma_r);
}
