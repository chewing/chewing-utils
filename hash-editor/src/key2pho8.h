#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdint.h>
#include <wchar.h>
uint16_t PhoneWc2Uint( const wchar_t *phone );
uint16_t Uint2PhoneWc( wchar_t *phone, uint16_t seq );
