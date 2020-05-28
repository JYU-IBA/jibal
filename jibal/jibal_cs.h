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

#ifndef _JIBAL_CS_H_
#define _JIBAL_CS_H_

#include <jibal_config.h>
#include <jibal_cross_section.h>

/* This file is a simplified interface to cross sections (global configuration) */

double jibal_cs_rbs(const jibal_config *config, const jibal_isotope *incident, const jibal_isotope *target, double theta, double E);
double jibal_cs_erd(const jibal_config *config, const jibal_isotope *incident, const jibal_isotope *target, double phi, double E);

const char *jibal_cs_rbs_name(const jibal_config *config); /* Name of used cross sections */
const char *jibal_cs_erd_name(const jibal_config *config); /* Name of used cross sections */
#endif /* _JIBAL_CS_H_ */
