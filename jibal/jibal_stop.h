#ifndef _JIBAL_STOP_H_
#define _JIBAL_STOP_H_

#include <jibal_gsto.h>

double jibal_stop(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E);
double jibal_stop_ele(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E);
double jibal_stop_nuc(const jibal_isotope *incident, const jibal_material *target, double E); /* TODO: energy range */
double jibal_layer_energy_loss(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_layer *layer, double E, double factor);

double jibal_gsto_stop_v(jibal_gsto *workspace, int Z1, int Z2, double v); /* Kinda deprecated */
double jibal_gsto_stop_em(jibal_gsto *workspace, int Z1, int Z2, double em); /* Kinda deprecated */
double jibal_gsto_stop_nuclear_universal(double E, int Z1, double m1, int Z2, double m2);

#endif /* _JIBAL_STOP_H_ */
