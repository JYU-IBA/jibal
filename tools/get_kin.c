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
#include <jibal.h>
#include <jibal_kin.h>
#include <jibal_cross_section.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    jibal jibal = jibal_init(NULL);
    if(jibal.error) {
        fprintf(stderr, "Initializing JIBAL failed with error code: %i (%s)\n", jibal.error,
                jibal_error_string(jibal.error));
        return EXIT_FAILURE;
    }
    if(argc<=4) {
        return -1;

    }
    const jibal_isotope *incident = jibal_isotope_find(jibal.isotopes, argv[1], 0, 0);
    if(!incident) {
        fprintf(stderr, "There is no isotope %s in my database.\n", argv[1]);
        return -1;
    }
    const jibal_isotope *target = jibal_isotope_find(jibal.isotopes, argv[2], 0, 0);
    if(!target) {
        fprintf(stderr, "There is no isotope %s in my database.\n", argv[2]);
        return -1;
    }
    double theta = jibal_get_val(jibal.units, UNIT_TYPE_ANGLE, argv[3]);
    double E = jibal_get_val(jibal.units, UNIT_TYPE_ENERGY, argv[4]);
    double E_erd = jibal_kin_erd(incident->mass, target->mass, theta) * E;
    double E_rbs = jibal_kin_rbs(incident->mass, target->mass, theta, '+') * E;
    double cs_erd =  jibal_erd_cross_section(incident, target, theta, E);
    double cs_rbs = jibal_rbs_cross_section(incident, target, theta, E);
    fprintf(stderr, "E = %g keV\n", E/C_KEV);
    fprintf(stderr, "E_rbs = %g keV\n", E_rbs/C_KEV);
    fprintf(stderr, "E_erd = %g keV\n", E_erd/C_KEV);
    fprintf(stderr, "ERD cross section = %g mb/sr\n", cs_erd/C_MB_SR);
    fprintf(stderr, "RBS cross section = %g mb/sr\n", cs_rbs/C_MB_SR);
    return EXIT_SUCCESS;
}
