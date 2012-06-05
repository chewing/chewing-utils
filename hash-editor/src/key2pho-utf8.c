//#include "global.h"
//#include "zhuin.h"
#include "chewing-utf8-util.h"
#include <string.h>

const char *zhuin_tab[] = {
  "ㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙ",
  "ㄧㄨㄩ",
  "ㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦ",
  "˙ˊˇˋ "
};
static const int zhuin_tab_num[] = {
  22, 4, 14, 6
};
static const int shift[] = { 9, 7, 3, 0 };
const static int sb[] = { 31, 3, 15, 7 };

//static const char *ph_str =
//"ㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙㄧㄨㄩㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦ˙ˊˇˋ" ;

//static char *key_str[ MAX_KBTYPE ] = {
//	"1qaz2wsxedcrfv5tgbyhnujm8ik,9ol.0p;/-7634",		/* standard kb */
//	"bpmfdtnlgkhjvcjvcrzasexuyhgeiawomnkllsdfj",		/* hsu */
//	"1234567890-qwertyuiopasdfghjkl;zxcvbn/m,.",		/* IBM */
//	"2wsx3edcrfvtgb6yhnujm8ik,9ol.0p;/-['=1qaz",		/* Gin-yieh */
//	"bpmfdtnlvkhg7c,./j;'sexuaorwiqzy890-=1234",		/* ET  */
//	"bpmfdtnlvkhgvcgycjqwsexuaorwiqzpmntlhdfjk",		/* ET26 */
//	"1'a;2,oq.ejpuk5yixfdbghm8ctw9rnv0lsz[7634",		/* Dvorak */
//	"bpmfdtnlgkhjvcjvcrzasexuyhgeiawomnklldhtn",		/* Dvorak Hsu */
//	"1qaz2wsxedcrfv5tgbyhnujm8ik,9ol.0p;/-7634",		/* pinying */
//} ;

/* Read one zhuin string,
   return the number it means */
int
zhuin_to_inx( char *zhuin )
{
  char *iter = zhuin, *pos;
  char buf[4];
  int len, result = 0;
  int i;
  
  /* Here the constant 4 is the number 
     of zhuin_tab and zhuin_tab_num */
/*  for( i = 0; i < 3; i++ ) */
  for( i = 0; i < 4; i++ )
    {
      len = chewing_utf8_byte2len( iter[0] );
      strncpy( buf, iter, len );
      buf[len] = '\0';
      if(!buf[0])
          continue;
      pos = strstr( zhuin_tab[i], buf );
      if ( pos )
	{
	  result |= (zhuin_tab_num[i] - chewing_utf8_strlen(pos)) << shift[i];
	  iter += len;
	}
    }
  return result;
}

/* Read one zhuin string,
   return the number of tones*/
int
zhuin_to_tone( char *zhuin )
{
  char *iter = zhuin, *pos = 0;
  char buf[4];
  int len, result = 0;
  int i;
  
  /* Here the constant 4 is the number 
     of zhuin_tab and zhuin_tab_num */
  for( i = 0; i < 4; i++ )
    {
      len = chewing_utf8_byte2len( iter[0] );
      strncpy( buf, iter, len );
      buf[len] = '\0';
      pos = strstr( zhuin_tab[3], buf );
      if ( pos )
	{
	  result = zhuin_tab_num[3] - chewing_utf8_strlen(pos) + 1;
          return result;
	}
      iter += len;
    }
  return result;
}

int Uint2PhoneUTF8( char *phone, long seq )
{
    int i, j, k;
    char *pos;
    char buffer[5];
    for ( i = 0, j = 0; i < 4; i++) {
        k = ((seq >> shift[ i ]) & sb[ i ] ) - 1;
        if ( k >= 0 && (pos = chewing_utf8_strseek( zhuin_tab[ i ], k )) )
        {
            chewing_utf8_strncpy(buffer, pos, 1, 1);
            strcat(phone, buffer);
            j++;
        }
    }
    return j;
}

#if 0
int Key2PhoneInxUTF8( int key, int type, int kbtype, int searchTimes )
{
	char keyStr[ 5 ], bgStr[ 10 ], *p;

	keyStr[ 0 ] = key;
	keyStr[ 1 ] = '\0';
	if( !Key2PhoUTF8( bgStr, keyStr, kbtype, searchTimes ) )
            return 0;
	p = strstr( zhuin_tab[ type ], bgStr );
	if ( ! p )
		return 0;
        /*
          This /2 here is because ph_pho[] string array uses
          big5. Every char is 2 bytes.
         */
        /* XXX: Need to modify to satisfy UTF-8 */
	//return ( p - ph_pho[ type ] ) / 2;
        return zhuin_tab_num[type] - chewing_utf8_strlen(p);
}

int Key2PhoUTF8( char *pho, const char *inputkey, int kbtype, int searchTimes )
{
	int len = strlen( inputkey ), i, s;
	char *pTarget;

	pho[ 0 ] = '\0';
	for ( i = 0; i < len; i++ ) {
		char *findptr = NULL;
		int index;

		for ( 
			s = 0, pTarget = key_str[ kbtype ]; 
			s < searchTimes; 
			s++, pTarget = findptr + 1 ) {
			findptr = strchr( pTarget, inputkey[ i ] );
			if ( ! findptr ) {
				return 0;
			}
		}
		index = findptr - key_str[ kbtype ];
                /* XXX: Need to modify to satisfy UTF-8 */
                /*
		pho[ i * 2 ] = ph_str[ index * 2 ];
		pho[ i * 2 + 1 ] = ph_str[ index * 2 + 1 ];
                */
                chewing_utf8_strncpy(
                        chewing_utf8_strseek(pho, i),
                        chewing_utf8_strseek(ph_str, index), 1, 1);
	}
        pTarget = chewing_utf8_strseek(pho, len);
	pTarget[0] = '\0';
	return 1;
}
#endif
