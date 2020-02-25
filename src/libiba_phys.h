/* Elementary physics */

#define energy(a,b) energy_relativistic(a,b)
#define velocity(a,b) energy_relativistic(a,b)


double velocity_relativistic(double E, double mass);
double energy_relativistic(double v, double mass);

double velocity_classical(double E, double mass);
double velocity_classical(double v, double mass);
