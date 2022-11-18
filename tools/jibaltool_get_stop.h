/*
    Jyväskylä Ion Beam Analysis Library (JIBAL)
    Copyright (C) 2020-2022 Jaakko Julin <jaakko.julin@jyu.fi>

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
#include "jibaltool.h"

typedef struct {
    jibal *jibal;
    const jibal_isotope *incident;
    jibal_layer **target; /* Array of pointers, size n_layers, reallocated as necessary */
    int verbose;
    int n_layers;
} get_stop_global;

int print_stop(jibaltool_global *global, int argc, char **argv);
