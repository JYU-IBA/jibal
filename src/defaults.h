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

#ifndef _JIBAL_DEFAULTS_H_
#define _JIBAL_DEFAULTS_H_

#include <jibal_units.h>

#define JIBAL_STEP_SIZE (10.0*C_TFU)

#ifndef JIBAL_DATADIR
#define JIBAL_DATADIR "../data/"
#endif

#define JIBAL_MASSES_FILE JIBAL_DATADIR "masses.dat"
#define JIBAL_ABUNDANCES_FILE JIBAL_DATADIR "abundances.dat"

#endif /* _JIBAL_DEFAULTS_H_ */
