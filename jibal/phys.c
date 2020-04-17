/*
    LibIBA - Library for ion beam analysis
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

#include <math.h>
#include <jibal_phys.h>
#include <jibal_units.h>

double jibal_velocity_classical(double E, double m) {
    return sqrt(2*E/m);
}

double jibal_energy_classical(double v, double m) {
    return 0.5*m*v*v;
}

double jibal_em_classical(double v) {
    return 0.5*v*v;
}

double jibal_velocity_em_classical(double em) {
    return sqrt(2*em);
}

double jibal_velocity_classical_more_accurate(double E, double m) {
    return (C_C*sqrt(2.0/3.0)*sqrt(-1.0+sqrt(6*E/m/C_C2+1)));
}

double jibal_energy_classical_more_accurate(double v, double m) {
    return (0.5*m*v*v + (3.0/8.0)*m*pow(v, 4.0));
}

double jibal_velocity_relativistic(double E, double m) {
    return sqrt((1-pow((1.0+E/(m*C_C2)),-2.0))*C_C2);
}

double jibal_energy_relativistic(double v, double m) {
    return (m*C_C2*(pow(1-pow(v/C_C,2.0),-0.5)-1));
}

double jibal_em_relativistic(double v) {
    return (C_C2*(pow(1-pow(v/C_C,2.0),-0.5)-1));
}

double jibal_velocity_em_relativistic(double em) {
    return sqrt((1-pow((1.0+em/C_C2),-2.0))*C_C2);
}

double jibal_linear_interpolation(double x_low, double x_high, double y_low, double y_high, double x) {
    return y_low+((x-x_low)/(x_high-x_low))*(y_high-y_low);
}
