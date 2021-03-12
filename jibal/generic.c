#include "jibal_generic.h"

int jibal_isdigit(const char c) {
    if(c >= '0' && c <= '9')
        return 1;
    return 0;
}
