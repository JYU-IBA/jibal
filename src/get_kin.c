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
#include <jibal_masses.h>
#include <jibal_units.h>
#include <jibal_gsto.h>
#include <jibal_material.h>
#include <jibal_kin.h>
#include <jibal_cross_section.h>

typedef struct {
    jibal_isotope *isotopes;
    jibal_element *elements;
    jibal_units *units;
} generic_data;


int main(int argc, char **argv) {
    generic_data data;
    data.isotopes=jibal_isotopes_load(NULL);
    jibal_abundances_load(data.isotopes, NULL);
    data.elements=jibal_elements_populate(data.isotopes);
    data.units=jibal_units_default();
    if(argc<=4) {
        return -1;

    }
    jibal_isotope *incident = jibal_isotope_find(data.isotopes, argv[1], 0, 0);
    jibal_isotope *target = jibal_isotope_find(data.isotopes, argv[2], 0, 0);
    double theta = jibal_get_val(data.units, UNIT_TYPE_ANGLE, argv[3]);
    double E = jibal_get_val(data.units, UNIT_TYPE_ENERGY, argv[4]);
    double E_erd = jibal_kin_erd(incident->mass, target->mass, theta) * E;
    double cs_erd =  jibal_erd_cross_section(incident, target, theta, E);
    fprintf(stderr, "E = %g keV\n", E/C_KEV);
    fprintf(stderr, "E_erd = %g keV\n", E_erd/C_KEV);
    fprintf(stderr, "ERD cross section = %g mb/sr\n", cs_erd/C_MB_SR);
    return 0;
}
