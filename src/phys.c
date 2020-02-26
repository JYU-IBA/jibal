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
#include "phys.h"
#include "units.h"

double iba_velocity_classical(double E, double m) {
    return sqrt(2*E/m);
}

double iba_energy_classical(double v, double m) {
    return 0.5*m*v*v;
}

double iba_velocity_relativistic(double E, double m) {
    double gamma=1.0+E/(m*C_C2);
    return sqrt((1-pow(gamma,-2.0))*C_C2);
}

double iba_energy_relativistic(double v, double m) {
    return (m*C_C2*(pow(1-pow(v/C_C,2.0),-0.5)-1));
}
