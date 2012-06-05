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

GtkTreeStore *store;
GtkTreeIter iter;
gboolean is_editing_existing_phrase;
GtkTreeSelection *selection;
GtkTreeView *main_tree_view;
gchar current_filename[1024];
GtkWidget *editor_dialog;
GtkWidget *search_dialog;

GtkWidget *che_create_tree( GtkWindow* );
GtkWidget *che_create_menu( GtkWindow* );
void che_read_hash(gchar *filename);
void convert(char*, char*, int);

struct zhuindict *zhuin_dictionary;

gboolean is_file_saved;
gchar prev_search_text[0xff] = {};
uint32_t chewing_lifetime = 0;

/* callback */
void file_open( GtkWindow* );
void file_save_as( GtkWindow* );
void file_save( gchar* );
void file_export(void*);
void file_import(void*);
void cell_edited(GtkCellRendererText *cellrenderertext, gchar *arg1, gchar *arg2, gpointer column);
void che_new_phrase_dlg( GtkWidget* );
void che_save_phrase(GtkWidget *obj, gpointer vbox);
void che_remove_phrase(GtkWidget *menu);
void che_show_search_dlg(GtkWidget *widget);
void che_show_about_dlg(GtkWidget *widget);
