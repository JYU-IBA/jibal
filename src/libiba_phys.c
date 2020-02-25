#include "libiba_phys.h"

double velocity_classical(double E, double m) {
    return sqrt(2*E/m);
}

double energy_classical(double v, double m) {
    return 0.5*m*v*v;
}

double velocity_relativistic(double E, double m) {
    double gamma=1.0+E/(mass*C_C2);
    return sqrt((1-pow(gamma,-2.0))*C_C2);
}

double energy_relativistic(double v, double m) {
    return (m*C_C2*(pow(1-pow(v/C_C,2.0),-0.5)-1));
}
