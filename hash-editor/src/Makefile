CFLAGS = -Wall -g
LIB_GTK = `pkg-config gtk+-2.0 --cflags --libs`

TARGET = che
UTF8_OBJS = key2pho-utf8.o chewing-utf8-util.o

all: $(TARGET)

$(TARGET): $(TARGET).o $(UTF8_OBJS) zhuindict.o
	$(CC) $(TARGET).o $(UTF8_OBJS) zhuindict.o -o $(TARGET) $(LIB_GTK)

zhuindict.o: zhuindict.c zhuindict.h zhuin_data.h
	$(CC) $(CFLAGS) -c zhuindict.c

zhuin_data.h: zhuindict_compile
	./zhuindict_compile

zhuindict_compile: zhuindict_compile.c $(UTF8_OBJS)
	$(CC) -o zhuindict_compile zhuindict_compile.c $(UTF8_OBJS)

%.o: %.c %.h
	$(CC) $(CFLAGS) $(LIB_GTK) -c $*.c

clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f zhuindict_compile
	rm -f zhuin_data.h
