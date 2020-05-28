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
#include <jibal_cs.h>
#include <stdlib.h>

void print_kin_rbs(jibal *jibal, const jibal_isotope *incident, const jibal_isotope *target, double theta, double E) {
    double r = incident->mass/target->mass;
    double theta_max=asin(target->mass/incident->mass);
    double theta_cm = theta + asin(r * sin(theta)); /* RBS */
    double E_rbs = jibal_kin_rbs(incident->mass, target->mass, theta, '+') * E;
    double cs_rbs = jibal_cs_rbs(&jibal->config, incident, target, theta, E);
    fprintf(stderr, "RBS (%s scattered by %s to lab angle theta)\n", incident->name, target->name);
    if(theta > theta_max) {
        fprintf(stderr, "theta_max = %g deg (scattering not possible)\n", theta_max/C_DEG);
        return;
    }
    fprintf(stderr, "theta = %g deg\n", theta/C_DEG);
    fprintf(stderr, "theta_cm = %g deg\n", theta_cm/C_DEG);
    fprintf(stderr, "E_rbs = %g keV\n", E_rbs/C_KEV);
    fprintf(stderr, "RBS cross section = %g mb/sr (%s)\n", cs_rbs/C_MB_SR, jibal_cs_name(&jibal->config));
}

void print_kin_erd(jibal *jibal, const jibal_isotope *incident, const jibal_isotope *target, double phi, double E) {
    double cs_erd = jibal_cs_erd(&jibal->config, incident, target, phi, E);
    double E_erd = jibal_kin_erd(incident->mass, target->mass, phi) * E;
    double theta_cm= C_PI - 2 * phi;
    double theta = atan2(sin(theta_cm), (cos(theta_cm) + target->mass / incident->mass));

    fprintf(stderr, "ERD (%s recoiled by %s to lab angle phi)\n", target->name, incident->name);
    if(phi >= C_PI/2.0) {
        fprintf(stderr, "Not possible.\n");
        return;
    }
    fprintf(stderr, "phi = %g deg\n", phi/C_DEG);
    fprintf(stderr, "theta_cm = %g deg\n", theta_cm/C_DEG);
    fprintf(stderr, "E_erd = %g keV\n", E_erd/C_KEV);
    fprintf(stderr, "v_erd = %g m/s\n", velocity(E_erd, target->mass));
    fprintf(stderr, "ERD cross section = %g mb/sr (%s)\n", cs_erd/C_MB_SR, jibal_cs_name(&jibal->config));
    double inverse_scaling = 4.0 * pow(sin(theta), 2.0) * cos(theta_cm - theta) * cos(phi) / (pow(sin(theta_cm), 2.0));
    double E_inv = (target->mass/incident->mass) * E;
    fprintf(stderr, "ERD cross section is %g times the RBS cross section for %g MeV %s scattering from %s to an angle of %g deg\n",
            inverse_scaling,
            E_inv/C_MEV,
            target->name,
            incident->name,
            theta/C_DEG
    );
}

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
    double angle = jibal_get_val(jibal.units, UNIT_TYPE_ANGLE, argv[3]); /* RBS */
    double E = jibal_get_val(jibal.units, UNIT_TYPE_ENERGY, argv[4]);
    double E_cm = target->mass*E/(incident->mass + target->mass);
    fprintf(stderr, "E_cm = %g keV\n", E_cm/C_KEV);
    print_kin_rbs(&jibal, incident, target, angle, E);
    fprintf(stderr, "\n");
    print_kin_erd(&jibal, incident, target, angle, E);
    return EXIT_SUCCESS;
}
