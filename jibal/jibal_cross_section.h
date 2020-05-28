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

#ifndef _JIBAL_CROSS_SECTION_H_
#define _JIBAL_CROSS_SECTION_H_

#include <jibal_masses.h>
typedef enum {
    JIBAL_CS_NONE=0,
    JIBAL_CS_RUTHERFORD=1,
    JIBAL_CS_ANDERSEN=2
} jibal_cross_section_type;


double jibal_cs_rbs(const jibal_isotope *incident, const jibal_isotope *target, double theta, double E, jibal_cross_section_type type);
double jibal_cs_erd(const jibal_isotope *incident, const jibal_isotope *target, double phi, double E, jibal_cross_section_type type);

const char *jibal_cs_name(jibal_cross_section_type type);

double jibal_andersen_correction(int z1, int z2, double E_cm, double theta_cm);

#endif /* _JIBAL_CROSS_SECTION_H_ */
