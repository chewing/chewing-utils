#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <iconv.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "key2pho8.h"
#include "key2pho-utf8.h"
#include "chewing-utf8-util.h"
#include "zhuindict.h"

#define FIELD_SIZE 125
#define BIN_HASH_SIG "CBiH"
#define ZHUIN_DICTIONARY_FILE "zhuin.txt"
//#define ENABLE_TEXT_HASH

enum {
	SEQ_COLUMN,
	ZUIN_COLUMN,
	USERFREQ_COLUMN,
	TIME_COLUMN,
	MAXFREQ_COLUMN,
	ORIGFREQ_COLUMN,
	N_COLUMNS
};
