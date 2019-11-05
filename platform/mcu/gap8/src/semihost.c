#include "gap_semihost.h"

// roughly this is the last stage of printf:
// print a string until '\0'
void gap8_semihost_write0(const char *print_string)
{
    __internal_semihost(SEMIHOSTING_SYS_WRITE0, (long) print_string);
}

int gap8_semihost_open(const char *name, int mode)
{
    uint32_t len = strlen(name);
    uint32_t args[3] = {name,mode,len};
    return __internal_semihost(SEMIHOSTING_SYS_OPEN, (long) args);
}

int gap8_semihost_close(int fd)
{
    //uint32_t args[3] = {name,mode,len};
    return __internal_semihost(SEMIHOSTING_SYS_CLOSE, (long) fd);
}

int gap8_semihost_read(int fd, uint8_t *buffer, int len)
{
    uint32_t args[3] = {(uint32_t)fd,(uint32_t)buffer,(uint32_t)len};
    return __internal_semihost(SEMIHOSTING_SYS_READ, (long) args);
}

int gap8_semihost_write(int fd, uint8_t *buffer, int len)
{
    uint32_t args[3] = {(uint32_t)fd,(uint32_t)buffer,(uint32_t)len};
    return __internal_semihost(SEMIHOSTING_SYS_WRITE, (long) args);
}

int gap8_semihost_seek(int fd, uint32_t pos)
{
    uint32_t args[2] = {(uint32_t)fd,pos};
    return __internal_semihost(SEMIHOSTING_SYS_WRITE, (long) args);
}

int gap8_semihost_flen(int fd)
{
    return __internal_semihost(SEMIHOSTING_SYS_FLEN, (long) fd);
}

void gap8_semihost_exit(int code)
{
    return __internal_semihost(SEMIHOSTING_SYS_EXIT, (long) code);
}
