/* Example of C++ using Jibal */

#include <iostream>
extern "C" {
#include <jibal.h>
#include <jibal_masses.h>
};

int main() {
    jibal *jibal = jibal_init(nullptr);
    if(jibal->error) {
        std::cerr << "Initializing JIBAL failed with error code: "
            << jibal->error
            << " (" << jibal_error_string(jibal->error) << ")"
            << std::endl;
        return 1;
    }
    const jibal_isotope *alpha=jibal_isotope_find(jibal->isotopes, "4He", 0, 0);
    std::cout << "The mass of " << alpha->name << " is " << alpha->mass/C_U << " u" << std::endl;
    const jibal_material *si = jibal_material_create(jibal->elements, "Si");
    double E = jibal_get_val(jibal->units, UNIT_TYPE_ENERGY, "2MeV");
    if(!si) {
        return EXIT_FAILURE;
    }
    int Z2 = si->elements[0].Z;
    if(!jibal_gsto_auto_assign(jibal->gsto, alpha->Z, Z2)) {
        return EXIT_FAILURE;
    }
    jibal_gsto_load_all(jibal->gsto);
    double S = jibal_gsto_get_em(jibal->gsto, GSTO_STO_ELE, alpha->Z, Z2, E/alpha->mass);
    std::cout << "The electronic stopping of " << alpha->name << " in Si at " << E/C_MEV << " MeV is " << S/C_EV_TFU << " eV/tfu\n";
    jibal_free(jibal);
    return 0;
}
