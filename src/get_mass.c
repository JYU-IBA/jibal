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

#include <stdio.h>
#include "masses.h"
#include "units.h"

int main(int argc, char **argv) {
    iba_isotope *isotopes=isotopes_load(NULL);
    iba_units *units=iba_units_default();
    if(!isotopes) {
        fprintf(stderr, "Couldn't load isotopes.\n");
    }
    iba_isotope *incident=isotope_find(isotopes, argv[1]);
    printf("mass=%g (%g u)\n", incident->mass, incident->mass/C_U);

    double E=0.0;
    if(argc>2) {
        E=iba_get_val(units, UNIT_TYPE_ENERGY, argv[2]);
        printf("E=%g keV\n", E/C_KEV);
    }
}
