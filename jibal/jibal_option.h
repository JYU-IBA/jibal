#ifndef _JIBAL_OPTION_H_
#define _JIBAL_OPTION_H_

#define JIBAL_OPTION_STR_NONE "none"

typedef struct {
    const char *s;
    int val;
} jibal_option; /* Association between strings and ints (ints preferably from enums). */


int jibal_option_n(const jibal_option *option);
int jibal_option_get_value(const jibal_option *option, const char *s);
const char *jibal_option_get_string(const jibal_option *option, int value);

#endif /* _JIBAL_OPTION_H_ */
