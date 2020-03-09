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
#define energy(a,b) jibal_energy_classical(a,b)
#define velocity(a,b) jibal_velocity_classical(a,b)
#else
#define energy(a,b) jibal_energy_relativistic(a,b)
#define velocity(a,b) jibal_velocity_relativistic(a,b)
#endif

double jibal_velocity_relativistic(double E, double mass);
double jibal_energy_relativistic(double v, double mass);

double jibal_velocity_classical(double E, double mass);
double jibal_velocity_classical(double v, double mass);

double jibal_linear_interpolation(double x_low, double x_high, double y_low, double y_high, double x);
#endif /* _JIBAL_PHYS_H_ */
