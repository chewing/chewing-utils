#ifndef CHEWING_UTF8_UTIL_H
#define CHEWING_UTF8_UTIL_H

int chewing_utf8_strlen(char *str);
int chewing_utf8_byte2len(unsigned char b);
int chewing_utf8_strncpy(char *dest, const char *src, int n, int end);
int chewing_utf8_is_valid_str(char *str);
int chewing_utf8_strlen(char *str);

#endif /* CHEWING_UTF8_UTIL_H */
