#include <string.h>
#include <stdio.h>
#include <jibal_option.h>


int jibal_option_n(const jibal_option *option) {
    const jibal_option *o;
    int n=0;
    if (!option)
        return 0;
    for (o = option; o->s; o++) {
        n++;
    }
    return n;
}

int jibal_option_get_value(const jibal_option *option, const char *s) {
    const jibal_option *o;
    if(!s)
        return 0; /* TODO: is returning zero a sane fail safe? */
    for(o=option; o->s; o++) {
        if (strcmp(o->s, s) == 0) {
            return o->val;
        }
    }
    fprintf(stderr, "WARNING: \"%s\" is not a valid option.\n", s);
    return 0;
}

const char *jibal_option_get_string(const jibal_option *option, int value) {
    if(!option)
        return JIBAL_OPTION_STR_NONE;
    int n = jibal_option_n(option);
    if(value >= 0 && value < n) {
        return option[value].s;
    }
    return JIBAL_OPTION_STR_NONE;
}
