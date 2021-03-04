#include <jibal.h>
#include <jibal_stragg.h>
#include "jibal_stop.h"


double jibal_stragg(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E) {
    size_t i;
    double sum=0.0;
    double em = E/incident->mass;
    int Z1=incident->Z;
    for (i = 0; i < target->n_elements; i++) {
        int Z2 = target->elements[i].Z;
        sum += target->concs[i]*jibal_gsto_get_em(workspace, GSTO_STO_STRAGG, Z1, Z2, em);
    }
    return sum;
}

double jibal_stragg_bohr(int Z1, int Z2) {
    const double bohr = 4.0 * C_PI * pow(C_E, 4.0) / pow(4.0 * C_PI * C_EPSILON0, 2.0);
    //const double bohr = 6.6741219e-55;
    return  Z1 * Z1 * Z2 * bohr;
}

double jibal_layer_energy_loss_with_straggling(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_layer *layer, double E_0, double factor, double *S) {
    double E = E_0;
    double dE;
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
        dE = (h / 6) * (k1 + 2 * k2 + 2 * k3 + k4); /* Energy change in thickness "h" */
#ifndef NO_NON_STATISTICAL_BROADENING
        double s_ratio = jibal_stop(workspace, incident, layer->material, E+dE)/(k1/factor); /* Ratio of stopping */
        *S *= (s_ratio)*(s_ratio); /* Non-statistical broadening due to energy dependent stopping */
#endif
        *S += h*jibal_stragg(workspace, incident, layer->material, (E+dE/2)); /* Straggling, calculate at mid-energy */
        E += dE;
#else
        dE = factor*h*jibal_stop(workspace, incident, layer->material, E);
        E += factor*h*jibal_stop(workspace, incident, layer->material, E);
        /* TODO: straggling? */
#endif
        if(!isnormal(E))
            return 0.0;
    }
    return E;
}
