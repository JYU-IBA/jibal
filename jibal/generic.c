#include "jibal_generic.h"

#include <string.h>

int jibal_isdigit(const char c) {
    if(c >= '0' && c <= '9')
        return 1;
    return 0;
}

FILE *jibal_fopen(const char *filename, const char *mode) {
    FILE *f;
    if(!filename) {
        if(*mode == 'r')
            f = stdin;
        else
            f = stderr;
    } else if(strlen(filename) == 1 && *filename == '-')
        f = stdout;
    else {
        f = fopen(filename, mode);
    }
    if(!f && filename) {
        fprintf(stderr, "Can not open file \"%s\" (mode %s)\n", filename, mode);
    }
    return f;
}

void jibal_fclose(FILE *f) {
    if(f == stdout || f == stderr || f == stdin)
        return;
    fclose(f);
}
