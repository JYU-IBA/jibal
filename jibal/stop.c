#include <jibal_stop.h>


double jibal_gsto_stop_em(jibal_gsto *workspace, int Z1, int Z2, double em) {
    return jibal_gsto_get_em(workspace, GSTO_STO_ELE, Z1, Z2, em);
}

double jibal_gsto_stop_v(jibal_gsto *workspace, int Z1, int Z2, double v) {
    return jibal_gsto_get_em(workspace, GSTO_STO_ELE, Z1, Z2, jibal_energy_per_mass(v));
}

double jibal_stop(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E) {
    /* TODO: make a variable in the workspace that determines whether we are interested in total stopping or just
     * electronic stopping. At the moment this is fixed to total. */
    return jibal_stop_nuc(incident, target, E) + jibal_stop_ele(workspace, incident, target, E); /* This returns a POSITIVE value (-dE/dx) in SI units for stopping cross section, e.g. J/(1/m^2) = J m^2 */
}

double jibal_stop_nuc(const jibal_isotope *incident, const jibal_material *target, double E) {
    int i;
    double sum = 0.0;
    for (i = 0; i < target->n_elements; i++) {
        jibal_element *element = &target->elements[i];
#ifndef NUCLEAR_STOPPING_ISOTOPES
        sum += target->concs[i]*jibal_gsto_stop_nuclear_universal(E, incident->Z, incident->mass, element->Z, element->avg_mass);
#else
        int j;
        for(j=0; j < element->n_isotopes; j++) { /* It would probably suffice to calculate the nuclear stopping with an average mass... */
            const jibal_isotope *isotope = element->isotopes[j];
            sum += target->concs[i]*element->concs[j]*gsto_sto_nuclear_universal(E, incident->Z, incident->mass, element->Z, isotope->mass);
        }
#endif
    }
    return sum;
}

double jibal_layer_energy_loss(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_layer *layer, double
E_0, double factor) {
    double E = E_0;
    double x;
    double h = workspace->stop_step;
#ifdef DEBUG
    fprintf(stderr, "Thickness %g, stop step %g\n", layer->thickness, h);
#endif
    for (x = 0.0; x <= layer->thickness; x += h) {
        if(x+h > layer->thickness) { /* Last step may be partial */
            h=layer->thickness-x;
            if(h < workspace->stop_step/1e6) {
                break;
            }
        }
#ifndef NO_RUNGE_KUTTA
        double k1, k2, k3, k4;
        k1 = factor*jibal_stop(workspace, incident, layer->material, E);
        k2 = factor*jibal_stop(workspace, incident, layer->material, E + (h / 2) * k1);
        k3 = factor*jibal_stop(workspace, incident, layer->material, E + (h / 2) * k2);
        k4 = factor*jibal_stop(workspace, incident, layer->material, E + h * k3);
        E += (h / 6) * (k1 + 2 * k2 + 2 * k3 + k4);
#else
        E += factor*h*jibal_stop(workspace, incident, layer->material, E);
#endif
        if(!isnormal(E))
            return 0.0;
    }
    return E;
}

double jibal_stop_ele(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E) {
    int i;
    double sum = 0.0;
    double em=E/incident->mass;
    for (i = 0; i < target->n_elements; i++) {
        jibal_element *element = &target->elements[i];
        sum += target->concs[i]*jibal_gsto_get_em(workspace, GSTO_STO_ELE, incident->Z, element->Z, em);
    }
    return sum;
}

