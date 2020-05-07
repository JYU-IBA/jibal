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

#include <stdlib.h>
char *dirname(char *path) {
    size_t s = strlen(path)+1;
    size_t convertedChars = 0;
    wchar_t *pszPath = malloc(sizeof(wchar_t)*s);
    mbstowcs_s(&convertedChars, pszPath, s, path, _TRUNCATE); /* convert multibyte to wide */
    if(!pszPath)
        return NULL;
    HRESULT result = PathCchRemoveFileSpec(pszPath, convertedChars); /* remove trailing file name or directory */
    if(result != S_OK)
        return NULL;
    size_t s_out = 2*(wcslen(pszPAth)+1);
    char *path_out = malloc(s_out);
    wcstombs_s(&convertedChars, path_out, s_out, pszPath, _TRUNCATE);
    free(pszPath);
    return path_out; /* Could be NULL */
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
