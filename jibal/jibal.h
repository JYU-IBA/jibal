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

#ifndef JIBAL_JIBAL_H
#define JIBAL_JIBAL_H

#include <jibal_units.h>
#include <jibal_masses.h>
#include <jibal_material.h>
#include <jibal_gsto.h>

typedef struct {
    jibal_units *units;
    jibal_isotope *isotopes;
    jibal_element *elements;
    jibal_gsto *gsto;
} jibal; /* All in one solution */

jibal jibal_init();
void jibal_free();

#endif //JIBAL_JIBAL_H
