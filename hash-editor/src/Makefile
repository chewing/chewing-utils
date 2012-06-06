CFLAGS=-Wall -g
LIB_GTK=`pkg-config gtk+-2.0 --cflags --libs` 

all: che

che: che.o key2pho-utf8.o chewing-utf8-util.o zhuindict.o
	gcc che.o key2pho-utf8.o chewing-utf8-util.o zhuindict.o -o che $(LIB_GTK)

zhuindict.o: zhuindict.c zhuindict.h zhuin_data.h
	gcc $(CFLAGS) -c zhuindict.c

zhuin_data.h: zhuindict_compile
	./zhuindict_compile

zhuindict_compile: zhuindict_compile.c
	gcc -o zhuindict_compile zhuindict_compile.c

%.o: %.c %.h
	gcc $(CFLAGS) $(LIB_GTK) -c $*.c

clean:
	rm -f *.o
	rm -f che
	rm -f zhuindict_compile
	rm -f zhuin_data.h
