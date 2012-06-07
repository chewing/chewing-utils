/* Compile zhuin.txt into C header file */
#include <stdio.h>
#include "key2pho-utf8.h"

int main()
{
	char charbuf[100];
	char zhuinbuf[100];
	FILE *fin;
	FILE *fout;
	int count = 0;

	fin = fopen("zhuin.txt", "r");
	if (!fin)
		return -1;
	fout = fopen("zhuin_data.h", "w");
	if (!fout)
		return -1;

	fprintf(fout, "/* this file is generated from zhuin.txt. */\n");
	fprintf(fout, "static const struct entry default_data[] = {\n");

	while (fscanf(fin, "%s %s", charbuf, zhuinbuf) != EOF) {
		fprintf(fout, "\t{\"%s\", %d},\n", charbuf, zhuin_to_inx(zhuinbuf));
		++count;
	}

	fprintf(fout, "\t{0, 0}\n");
	fprintf(fout, "};\n\n");
	fprintf(fout, "static const int default_data_len = %d;\n", count);

	fclose(fin);
	fclose(fout);

	return 0;
}
