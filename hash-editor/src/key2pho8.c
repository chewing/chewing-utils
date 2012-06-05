#include "key2pho8.h"

const static int shift[] = { 9, 7, 3, 0 };
const static int sb[] = { 31, 3, 15, 7 };
const char *ph_pho[] = {				/* number of bits */
	"  ㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙ",	/* 5 */
	"  ㄧㄨㄩ",					/* 2 */
	"  ㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦ",			/* 4 */
	"  ˙ˊˇˋ"					/* 3 */
};

/*
   Translate a single phone into a single uint
   assume phone is a big5 null-terminated string with no spaces
   */
uint16_t PhoneWc2Uint( const wchar_t *phone )
{
	int i, j, len = wcslen( phone );
	uint16_t result = 0;
	wchar_t *pc;
	wchar_t temp;
	wchar_t *wcPho;


	for ( i = 0, j = 0; i < len && j < 4; j++ ) {
		temp = phone[ i ];
		wcPho = malloc(sizeof(wchar_t)*22);
		mbstowcs( wcPho, ph_pho[ j ], 22);
		pc = wcschr( wcPho, temp );
		if ( pc ) {
			result |= ( ( pc - wcPho ) - 1 ) << shift[ j ];
			i++;
		}
		free( wcPho );
	}
	return result;
}

uint16_t Uint2PhoneWc( wchar_t *phone, uint16_t seq )
{
	int i, j;
	wchar_t *wcPho;
	for ( i = 0, j = 0; i < 4; i++) {
		wcPho = malloc(sizeof(wchar_t)*22);
		mbstowcs( wcPho, ph_pho[ i ], 22);
		if ( wcPho[ ((seq >> shift[ i ]) & sb[ i ]) + 1 ] != ' ' )
		{
			phone[ j ] = wcPho[ ((seq >> shift[ i ]) & sb[ i ]) + 1 ];
			j++;
		}
		free (wcPho);
	}
	return j;
}

//int main(int argc, char *argv[])
//{
//	wchar_t phone[100];
//	char bphone[100];
//	if (argc < 2)
//	{
//		fprintf(stderr, "Usage: key2pho <number>\n");
//		exit(1);
//	}
//	setlocale(LC_CTYPE, "zh_TW.UTF-8");
////	mbstowcs( phone, "ㄊㄡˊ", 3 );
////	printf("ㄊㄡˊ : %d\n", PhoneWc2Uint( phone ));
//	memset(phone, 0, sizeof(phone));
//	Uint2PhoneWc(phone, atoi(argv[1]));
//	wcstombs( bphone, phone, sizeof(bphone) );
//	printf("%d: %s\n",atoi(argv[1]), bphone);
//
//	return 0;
//}
