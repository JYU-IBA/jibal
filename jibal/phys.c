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


extern inline double pow4(double x);
extern inline double jibal_velocity_relativistic(double E, double mass);
extern inline double jibal_energy_relativistic(double v, double mass);
extern inline double jibal_em_relativistic(double v);
extern inline double jibal_velocity_em_relativistic(double em);
extern inline double jibal_velocity_classical(double E, double mass);
extern inline double jibal_energy_classical(double v, double mass);
extern inline double jibal_em_classical(double v);
extern inline double jibal_velocity_em_classical(double em);
extern inline double jibal_velocity_classical_more_accurate(double E, double mass);
extern inline double jibal_velocity_classical_more_accurate(double v, double mass);
extern inline double jibal_linear_interpolation(double x_low, double x_high, double y_low, double y_high, double x);
