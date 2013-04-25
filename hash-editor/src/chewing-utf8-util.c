#include <stdio.h>
#include <string.h>
#include "chewing-utf8-util.h"

/* Table of UTF-8 length */
static char utf8len_tab[256] =
  {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /*bogus*/
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /*bogus*/
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1,
  };

/* Return length of UTF-8 string */
size_t
chewing_utf8_strlen(const char *str)
{
  size_t length = 0;
  const char *strptr = str;

  while (strptr[0] != '\0')
    {
      strptr += chewing_utf8_byte2len(strptr[0]);
      ++length;
    }
  return length;
}

/* Return bytes of a UTF-8 character */
size_t
chewing_utf8_byte2len(unsigned char b)
{
  return utf8len_tab[b];
}

/* Return how many bytes was copied */
size_t
chewing_utf8_strncpy(char *dest, const char *src, int n, int end)
{
   int i = 0, len = 0;
   const char *iter = src;
   for( i = 0; i < n; i++ )
   {
       len += chewing_utf8_byte2len(iter[len]);
   }
   memcpy(dest, iter, len);
   if( end == 1)
      dest[len] = '\0';
   return len;
}

char*
chewing_utf8_strseek(const char *src, size_t n)
{
    int i = 0;
    const char *iter = src;
    for( i = 0; i < n; i++ )
    {
        iter += chewing_utf8_byte2len(iter[0]);
    }
    return (char*)iter;
}


int chewing_utf8_is_valid_str(const char *str)
{
    if ( str == NULL || *str == '\0' ) {
        return 0;
    }
    while ( *str != '\0' )  {
        int len = utf8len_tab[(unsigned char)*str];
        if ( len <= 1 ) {
            return 0;
        }
        str += len;
    };
    return 1;
}
