#ifdef WIN32
#include <string.h>
char *strsep(char **stringp, const char *delim) {
	char *start= *stringp;
	char *p;

	p = (start != NULL) ? strpbrk(start, delim) : NULL;

	if(p==NULL) {
		*stringp=NULL;
	} else {
		*p = '\0';
		*stringp=p+1;
	}
	return start;
}
#endif

#ifdef _MSC_VER
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define vscprintf _vscprintf
int vasprintf(char **strp, const char *format, va_list ap)
{
    int len = vscprintf(format, ap);
    if (len == -1)
        return -1;
    char *str = (char*)malloc((size_t) len + 1);
    if (!str)
        return -1;
    int retval = vsnprintf(str, len + 1, format, ap);
    if (retval == -1) {
        free(str);
        return -1;
    }
    *strp = str;
    return retval;
}

int asprintf(char **strp, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int retval = vasprintf(strp, format, ap);
    va_end(ap);
    return retval;
}
#endif
