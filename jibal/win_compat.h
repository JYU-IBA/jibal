#ifdef WIN32
#define F_OK 0
#define W_OK 2
#define R_OK 4
char *strsep(char **, const char *);
char *dirname(char *path);
#endif
