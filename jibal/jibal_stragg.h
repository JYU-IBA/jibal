#ifndef _JIBAL_STRAGG_H_
#define _JIBAL_STRAGG_H_

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

double jibal_stragg(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E);
double jibal_stragg_bohr(int Z1, int Z2);
double jibal_layer_stragg(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_layer *layer, double
E_0, double factor);

#endif // _JIBAL_STRAGG_H_
