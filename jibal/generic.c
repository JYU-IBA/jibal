#include <string.h>
#include "jibal_generic.h"

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

char *jibal_remove_double_quotes(char *s) {
    char *src = s, *dst = s;
    while((*dst = *src) != '\0') {
        if ((*dst == '"') && (*src == '"')) {
            src++;
        }
        src++;
        dst++;
    }
    return s;
}

char *jibal_strsep(char **stringp, const char *delim) {
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

char *jibal_strsep_with_quotes(char **stringp, const char *delim) {
    char *s = *stringp;
    if(!s) {
        return NULL;
    }
    if(*s != '"') {
        return jibal_strsep(stringp, delim);
    }
    *s = '\0'; /* Replace initial quote with a terminator */
    char *end = s + 1;
    while(1) {
        end += strcspn(end, "\""); /* Find ending quote, end of string. If *s is already a quote, this does nothing. */
        if(*end == '"' && *(end+1) == '"') { /* Skip over double-double quotes (input might have escaped " -> "") */
            end += 2;
            continue;
        }
        break;
    }
    if(*end == '"') { /* True end quote found */
        *end = 0; /* Terminate string */
        end++; /* Could be first real character of the next token, delimeter or terminator */
        if(*end == '\0') {
            *stringp = NULL;
        } else {
            for(const char *d = delim; *d; d++) { /* Find out if there is a delimeter after the final quote... */
                if(*end == *d) {
                    *end = '\0'; /* Set it to zero */
                    end++; /* ...and skip over it */
                    break;
                }
            }
            *stringp = end;
        }
        return jibal_remove_double_quotes(s + 1); /* Returns the original token, but skipping over the initial quote and unescaping double quotes */
    } else { /* No terminating quote found, the initial quote has been replaced with a terminator so this should look like an empty field. */
        *stringp = NULL; /* Signal end-of-string */
        return s;
    }
}
