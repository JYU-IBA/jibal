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
    jibal_free(jibal);
    return 0;
}
