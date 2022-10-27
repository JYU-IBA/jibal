/*
    Jyväskylä Ion Beam Analysis Library (JIBAL)
    Copyright (C) 2020-2022 Jaakko Julin <jaakko.julin@jyu.fi>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
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
#include <jibal_cs.h>
#include <stdlib.h>


int main(int argc, char **argv) {
    jibal *jibal = jibal_init(NULL);
    if(jibal->error) {
        fprintf(stderr, "Initializing JIBAL failed with error code: %i (%s)\n", jibal->error,
                jibal_error_string(jibal->error));
        return EXIT_FAILURE;
    }
    if(argc<=4) {
        return -1;

    }
    const jibal_isotope *incident = jibal_isotope_find(jibal->isotopes, argv[1], 0, 0);
    if(!incident) {
        fprintf(stderr, "There is no isotope %s in my database.\n", argv[1]);
        return -1;
    }
    jibal_material *target_material = jibal_material_create(jibal->elements, argv[2]);
    if(!target_material) {
        fprintf(stderr, "Error in creating material from expression %s\n", argv[2]);
        return -1;
    }
    double theta = jibal_get_val(jibal->units, UNIT_TYPE_ANGLE, argv[3]); /* RBS */
    double E = jibal_get_val(jibal->units, UNIT_TYPE_ENERGY, argv[4]);

    double cs_rbs = 0.0;

    for(size_t i_elem = 0; i_elem < target_material->n_elements; i_elem++) {
        jibal_element *e = &target_material->elements[i_elem];
        for(size_t i_isotope = 0; i_isotope < e->n_isotopes; i_isotope++) {
            const jibal_isotope *isotope = e->isotopes[i_elem];
            double theta_max=asin(isotope->mass/incident->mass);
            if(incident->mass >= isotope->mass && theta > theta_max) {
                continue; /* Scattering not possible */
            }
            double sigma = jibal_cs_rbs(jibal->config, incident, isotope, theta, E);
#ifdef DEBUG
            fprintf(stderr, "Isotope %zu conc %lf\n", i_isotope, e->concs[i_isotope]);
#endif
            cs_rbs += e->concs[i_isotope] * sigma;
        }
    }
    fprintf(stderr, "RBS cross section is %g mb/sr (%s)\n", cs_rbs/C_MB_SR, jibal_cs_rbs_name(jibal->config));
    jibal_material_free(target_material);
    jibal_free(jibal);
    return EXIT_SUCCESS;
}
