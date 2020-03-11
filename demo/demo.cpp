/* Example of C++ using Jibal */

#include <iostream>
extern "C" {
#include <jibal_masses.h>
};

int main() {
    jibal_isotope *isotopes=jibal_isotopes_load(NULL);
    jibal_isotope *alpha=jibal_isotope_find(isotopes, "4He", 0, 0);
    std::cout << "The mass of " << alpha->name << " is " << alpha->mass/C_U << " u" << std::endl;
    jibal_isotopes_free(isotopes);
    return 0;
}
