#include <jibal_registry.h>
#ifdef WIN32
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

char *jibal_registry_string_get(const char *value) {
    DWORD buf_size=256;
    char *buf = NULL;
    LSTATUS status = ERROR_SUCCESS;
    do {
        buf=realloc(buf, sizeof(char)*buf_size);
        if(!buf) {
            return NULL;
        }
        RegGetValueA(HKEY_LOCAL_MACHINE, JIBAL_SUBKEY, value, RRF_RT_REG_SZ, NULL, buf, &buf_size);
    } while(status==ERROR_MORE_DATA);
    if(status == ERROR_SUCCESS) {
        return buf;
    } else {
        free(buf);
        return NULL;
    }
}
#endif /* WIN32 */
