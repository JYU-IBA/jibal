#ifndef _JIBAL_PHYS_H_
#define _JIBAL_PHYS_H_

/*
    JIBAL - Library for ion beam analysis
    Copyright (C) 2020 Jaakko Julin <jaakko.julin@jyu.fi>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Elementary physics */

#ifdef CLASSICAL
#define jibal_energy(energy,mass) jibal_energy_classical(energy,mass)
#define jibal_velocity(energy,mass) jibal_velocity_classical(energy,mass)
#define jibal_energy_per_mass(velocity) jibal_em_classical(velocity)
#define jibal_velocity_from_em(em) jibal_velocity_em_classical(em)
#else
#define jibal_energy(velocity,mass) jibal_energy_relativistic(velocity,mass) /* v => E */
#define jibal_velocity(energy,mass) jibal_velocity_relativistic(energy,mass)  /* E => v */
#define jibal_energy_per_mass(velocity) jibal_em_relativistic(velocity) /* v => E/m */
#define jibal_velocity_from_em(em) jibal_velocity_em_relativistic(em) /* E/m => v */
#endif

#include <jibal_units.h>

inline double pow2(double x) {
    return x*x;
}

inline double pow3(double x) {
    return (x*x*x);
}

inline double pow4(double x) {
    return pow2(x*x);
}

inline double jibal_velocity_classical(double E, double m) {
    return sqrt(2*E/m);
}

inline double jibal_energy_classical(double v, double m) {
    return 0.5*m*v*v;
}

inline double jibal_em_classical(double v) {
    return 0.5*v*v;
}

inline double jibal_velocity_em_classical(double em) {
    return sqrt(2*em);
}

inline double jibal_velocity_classical_more_accurate(double E, double m) {
    return (C_C*sqrt(2.0/3.0)*sqrt(-1.0+sqrt(6*E/m/C_C2+1)));
}

inline double jibal_energy_classical_more_accurate(double v, double m) {
    return (0.5*m*v*v + (3.0/8.0)*m*pow4(v));
}

inline double jibal_velocity_relativistic(double E, double m) {
    return sqrt((1.0-1.0/pow2((1.0+E/(m*C_C2))))*C_C2);
}

inline double jibal_energy_relativistic(double v, double m) {
    return (m*C_C2*(1.0/sqrt(1-pow2(v/C_C))-1));
}

inline double jibal_em_relativistic(double v) {
    return (C_C2*(1.0/sqrt(1-pow2(v/C_C))-1));
}

inline double jibal_velocity_em_relativistic(double em) {
    return sqrt((1.0-1.0/pow2((1.0+em/C_C2)))*C_C2);
}

inline double jibal_linear_interpolation(double x_low, double x_high, double y_low, double y_high, double x) {
    return y_low+((x-x_low)/(x_high-x_low))*(y_high-y_low);
}

#endif /* _JIBAL_PHYS_H_ */
