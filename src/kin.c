#include <math.h>
#include <jibal_kin.h>


double jibal_kin_rbs(double m1, double m2, double theta, int plus) {
    double sign=plus?1.0:-1.0;
    return (pow(m1, 2.0)/pow(m1+m2, 2.0))*pow(cos(theta)+sign*sqrt(pow(m2/m1, 2.0)-pow(sin(theta), 2.0)), 2.0);
}

double jibal_kin_erd(double m1, double m2, double theta) {
    return (4.0*m1*m2*pow(cos(theta), 2.0)/pow(m1+m2, 2.0));
}