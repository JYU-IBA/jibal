#include <jibal.h>
#include <jibal_stragg.h>


double jibal_stragg(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_material *target, double E) {
    int i;
    double sum=0.0;
    double em = E/incident->mass;
    for (i = 0; i < target->n_elements; i++) {
        jibal_element *element = &target->elements[i];
        sum += target->concs[i]*jibal_stragg_bohr(incident->Z, element->Z)*jibal_gsto_get_em(workspace, GSTO_STO_STRAGG, incident->Z, element->Z, em);
    }
    return sum;
}

double jibal_stragg_bohr(int Z1, int Z2) {
    const double bohr = 4.0 * C_PI * pow(C_E, 4.0) / pow(4.0 * C_PI * C_EPSILON0, 2.0);
    //const double bohr = 6.6741219e-55;
    return  Z1 * Z1 * Z2 * bohr;
}

double jibal_layer_stragg(jibal_gsto *workspace, const jibal_isotope *incident, const jibal_layer *layer, double
E_0, double factor) {
    int i;
    double s=0.0;
    for(i = 0; i < layer->material->n_elements; i++) {
        double f = 1.0; /* TODO: replace with straggling model correction to Bohr */
        s += f*jibal_stragg_bohr(incident->Z, layer->material->elements[i].Z)*layer->material->concs[i];
    }
    s *= layer->thickness; /* TODO: loop over steps. This is more what jibal_layer_energy_loss() should do and we need
 * dE/dx as well...*/
    s *= factor;
    return sqrt(s);
}
