CFLAGS=-Wall -g
LIB_GTK=`pkg-config gtk+-2.0 --cflags --libs` 

all: che

che: che.o key2pho-utf8.o chewing-utf8-util.o zhuindict.o
	gcc che.o key2pho-utf8.o chewing-utf8-util.o zhuindict.o -o che $(LIB_GTK)

zhuindict.o: zhuindict.c
	gcc $(CFLAGS) -c zhuindict.c

%.o: %.c %.h
	gcc $(CFLAGS) $(LIB_GTK) -c $*.c

clean:
	rm -f *.o
	rm -f che
