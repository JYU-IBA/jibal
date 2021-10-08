#ifndef WIN_COMPAT_H
#define WIN_COMPAT_H
#ifdef WIN32
#define F_OK 0
#define W_OK 2
#define R_OK 4
#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif
char *strsep(char **, const char *);
char *dirname(char *path);
char *realpath(const char *restrict file_name, char *restrict resolved_name);
#endif
#ifdef _MSC_VER
#include <stdarg.h>
#define vscprintf _vscprintf
int asprintf(char **strp, const char *format, ...);
int vasprintf(char **strp, const char *format, va_list ap);
#endif
#endif // WIN_COMPAT_H
