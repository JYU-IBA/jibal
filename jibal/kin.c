#include <math.h>
#include <jibal_kin.h>


double jibal_kin_rbs(double m1, double m2, double theta, char sign) {
    double factor=(sign=='-'?-1.0:1.0);
    return (pow2(m1)/pow2(m1+m2))*pow2(cos(theta)+factor*sqrt(pow2(m2/m1)-pow2(sin(theta))));
}

double jibal_kin_erd(double m1, double m2, double theta) {
    return (4.0*m1*m2*pow2(cos(theta))/pow2(m1+m2));
}
