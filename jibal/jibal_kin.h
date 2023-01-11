/*
    Jyväskylä Ion Beam Analysis Library (JIBAL)
    Copyright (C) 2020 - 2023 Jaakko Julin <jaakko.julin@jyu.fi>

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

#ifndef JIBAL_KIN_H
#define JIBAL_KIN_H

#include <jibal_units.h>
#include <jibal_masses.h>

double jibal_kin_rbs(double m1, double m2, double theta, char sign);
double jibal_kin_erd(double m1, double m2, double theta);

#endif /* JIBAL_KIN_H */
