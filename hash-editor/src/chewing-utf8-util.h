#include <stdlib.h>

#ifndef CHEWING_UTF8_UTIL_H
#define CHEWING_UTF8_UTIL_H

size_t chewing_utf8_strlen(const char *str);
size_t chewing_utf8_byte2len(unsigned char b);
size_t chewing_utf8_strncpy(char *dest, const char *src, int n, int end);
int chewing_utf8_is_valid_str(const char *str);
char *chewing_utf8_strseek(const char *src, size_t n);

#endif /* CHEWING_UTF8_UTIL_H */
