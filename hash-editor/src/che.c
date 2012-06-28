#include "che.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* FIXME: use portable description */
#define CHEWING_HASH_PATH ".chewing"
#define PLAT_SEPARATOR "/"
#define HASH_FILE  "uhash.dat"

static char *hashfilename;

GtkWidget *main_window;

int main(int argc, char *argv[])
{
  GtkWidget *menu;
  GtkWidget *tree;
  GtkWidget *scroll;
  GtkWidget *vbox;

  /* load zhuin dictionary */
  zhuin_dictionary = zhuindict_load_default_dict();

  gtk_init(&argc, &argv);

  main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (main_window), "delete_event",
		    G_CALLBACK (che_quit), NULL);
  gtk_window_set_default_size(GTK_WINDOW(main_window), 500, 400);
  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(main_window), vbox);

  menu = che_create_menu(GTK_WINDOW(main_window));
  gtk_box_pack_start(GTK_BOX(vbox), menu, FALSE, TRUE, 0);
	
  /* new scrolled window */
  scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll), GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

  tree = che_create_tree(GTK_WINDOW(main_window));
  //gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), tree); /* this breaks scrolling */
  /* GtkTreeView has its native scrolling mechanism, so we don't need a viewport */
  gtk_container_add(scroll, tree); 

  is_file_saved = FALSE;

  if (argc > 1)
    che_read_hash(argv[1]);
  else {
    if (getenv("HOME")) {
      hashfilename = g_strdup_printf("%s%s", getenv("HOME"),
                         PLAT_SEPARATOR CHEWING_HASH_PATH
                         PLAT_SEPARATOR HASH_FILE);
      /* writable uhash? */
      if (access(hashfilename, W_OK) != 0) {
        /* specify another uhash */
        g_free(hashfilename);
        file_open(main_window);
      }
    }
    else {
      file_open(main_window);
    }
    che_read_hash(hashfilename);
    g_free(hashfilename);
  }

  gtk_widget_show_all( main_window );
  gtk_main();

  zhuindict_free(zhuin_dictionary);

  return 0;
}

GtkWidget *
che_create_tree( GtkWindow *parient )
{
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkWidget *tree;
  void treeview_keypress_callback(GtkWidget*, GdkEvent*, gpointer);
  void treeview_row_activated_callback(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);

  store = gtk_tree_store_new (N_COLUMNS,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_INT,
			      G_TYPE_INT,
			      G_TYPE_INT,
			      G_TYPE_INT);
	
  tree = main_tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
  g_object_unref (G_OBJECT (store));

  g_signal_connect(G_OBJECT(tree), "key-press-event",
          G_CALLBACK (treeview_keypress_callback), NULL);
  g_signal_connect(G_OBJECT(tree), "row-activated",
          G_CALLBACK (treeview_row_activated_callback), NULL);

  /* PhoneSeq */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (G_OBJECT (renderer),
		"foreground", "darkblue",
		NULL);
  column = gtk_tree_view_column_new_with_attributes ("語詞", renderer,
						     "text", SEQ_COLUMN,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  /* Zuin */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("注音", renderer,
						     "text", ZUIN_COLUMN,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  /* UserFreq */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set(renderer, "editable", TRUE, NULL);
  g_signal_connect (G_OBJECT (renderer), "edited",
		    G_CALLBACK (cell_edited), (gpointer)"2");
  column = gtk_tree_view_column_new_with_attributes ("UserFreq",
						     renderer,
						     "text", USERFREQ_COLUMN,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  /* Time */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set(renderer, "editable", TRUE, NULL);
  g_signal_connect (G_OBJECT (renderer), "edited",
		    G_CALLBACK (cell_edited), (gpointer)"3");
  column = gtk_tree_view_column_new_with_attributes ("Time",
						     renderer,
						     "text", TIME_COLUMN,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  /* MaxFreq */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set(renderer, "editable", TRUE, NULL);
  g_signal_connect (G_OBJECT (renderer), "edited",
		    G_CALLBACK (cell_edited), (gpointer)"4");
  column = gtk_tree_view_column_new_with_attributes ("MaxFreq",
						     renderer,
						     "text", MAXFREQ_COLUMN,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
	
  /* OrigFreq */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set(renderer, "editable", TRUE, NULL);
  g_signal_connect (G_OBJECT (renderer), "edited",
		    G_CALLBACK (cell_edited), (gpointer)"5");
  column = gtk_tree_view_column_new_with_attributes ("OrigFreq",
						     renderer,
						     "text", ORIGFREQ_COLUMN,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  return tree;
}

/* gtk radio menu items to choose the file format */
static GtkWidget *format_menu_text;
static GtkWidget *format_menu_binary;

GtkWidget *
che_create_menu( GtkWindow *parient )
{
  GtkWidget *menu_bar;
  GtkWidget *menu_bar_file;
  GtkWidget *menu_bar_edit;
  GtkWidget *menu_bar_about;
  GtkWidget *file_menu;
  GtkWidget *file_menu_open;
  GtkWidget *file_menu_save;
  GtkWidget *file_menu_saveas;
  GtkWidget *file_menu_quit;
  GtkWidget *file_menu_export;
  GtkWidget *file_menu_import;
  GtkWidget *edit_menu;
  GtkWidget *edit_menu_newtsi;
  GtkWidget *edit_menu_remove;
  GtkWidget *edit_menu_search;
  GtkWidget *edit_menu_format;
  GtkWidget *format_menu;
  GSList    *format_group = NULL;
  GtkWidget *separate;
  GtkWidget *separate2;
  GtkAccelGroup *accel_group = NULL;

  accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(main_window), accel_group);
	
  file_menu = gtk_menu_new();
  file_menu_open = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN, NULL);
  g_signal_connect_swapped (G_OBJECT (file_menu_open), "activate",
			    G_CALLBACK (file_open), parient);
  gtk_widget_add_accelerator(file_menu_open, "activate", accel_group,
             GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), file_menu_open);
  file_menu_save = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE, NULL);
  g_signal_connect_swapped (G_OBJECT (file_menu_save), "activate",
			    G_CALLBACK (file_save), NULL);
  gtk_widget_add_accelerator(file_menu_save, "activate", accel_group,
             GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), file_menu_save);
  file_menu_saveas = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE_AS, NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), file_menu_saveas);
  g_signal_connect_swapped (G_OBJECT (file_menu_saveas), "activate",
			    G_CALLBACK (file_save_as), parient);
  file_menu_export = gtk_menu_item_new_with_mnemonic ("匯出(_E)");
  g_signal_connect_swapped (G_OBJECT (file_menu_export), "activate",
		       G_CALLBACK (file_export), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), file_menu_export);
  file_menu_import = gtk_menu_item_new_with_mnemonic ("匯入(_I)");
  g_signal_connect_swapped (G_OBJECT (file_menu_import), "activate",
		       G_CALLBACK (file_import), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), file_menu_import);
  separate = gtk_separator_menu_item_new();
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), separate);
  file_menu_quit = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
  g_signal_connect (G_OBJECT (file_menu_quit), "activate", G_CALLBACK (che_quit), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), file_menu_quit);

  edit_menu = gtk_menu_new();
  edit_menu_newtsi = gtk_menu_item_new_with_mnemonic ("新增語詞(_N)");
  gtk_widget_add_accelerator(edit_menu_newtsi, "activate", accel_group,
             GDK_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (edit_menu_newtsi), "activate", G_CALLBACK (che_new_phrase_dlg), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (edit_menu), edit_menu_newtsi);
  edit_menu_remove = gtk_menu_item_new_with_mnemonic ("移除語詞(_R)");
  g_signal_connect (G_OBJECT (edit_menu_remove), "activate", G_CALLBACK (che_remove_phrase), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (edit_menu), edit_menu_remove);
  edit_menu_search = gtk_menu_item_new_with_mnemonic ("搜尋(_S)");
  gtk_widget_add_accelerator(edit_menu_search, "activate", accel_group,
             GDK_f, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (edit_menu_search), "activate", G_CALLBACK (che_show_search_dlg), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (edit_menu), edit_menu_search);
#ifdef ENABLE_TEXT_HASH
  separate2 = gtk_separator_menu_item_new();
  gtk_menu_shell_append (GTK_MENU_SHELL (edit_menu), separate2);
  edit_menu_format = gtk_menu_item_new_with_mnemonic ("格式(_F)");
  gtk_menu_shell_append (GTK_MENU_SHELL (edit_menu), edit_menu_format);

  format_menu = gtk_menu_new();
  format_menu_text = gtk_radio_menu_item_new_with_mnemonic(format_group, "文字(_T)");
  format_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (format_menu_text));
  gtk_menu_shell_append (GTK_MENU_SHELL (format_menu), format_menu_text);
  format_menu_binary = gtk_radio_menu_item_new_with_mnemonic(format_group, "二進位(_B)");
  format_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (format_menu_binary));
  gtk_menu_shell_append (GTK_MENU_SHELL (format_menu), format_menu_binary);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM (edit_menu_format), format_menu);
#endif

  menu_bar = gtk_menu_bar_new ();
  menu_bar_file = gtk_menu_item_new_with_mnemonic ("檔案(_F)");
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_file), file_menu);
  menu_bar_edit = gtk_menu_item_new_with_mnemonic ("編輯(_E)");
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_edit), edit_menu);
  menu_bar_about = gtk_menu_item_new_with_mnemonic ("關於(_A)");
  g_signal_connect (G_OBJECT (menu_bar_about), "activate", G_CALLBACK (che_show_about_dlg), NULL);

  gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), menu_bar_file);
  gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), menu_bar_edit);
  gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), menu_bar_about);

  return menu_bar;
}

enum { HF_TEXT, HF_BINARY };

void
che_set_hash_format(int fmt)
{
	switch (fmt) {
	case HF_TEXT:
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (format_menu_text), TRUE);
		break;
	case HF_BINARY:
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (format_menu_binary), TRUE);
		break;
	default:
		assert(!"Incorrect hash format");
	}
}

int
che_get_hash_format()
{
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (format_menu_text)))
		return HF_TEXT;
	else if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (format_menu_binary)))
		return HF_BINARY;
	else
		assert(!"Either text or binary should be selected.");
}

void
che_read_hash_txt(gchar *filename)
{
  FILE *hash;
  gchar buffer[126];
  gchar buffer2[126];
  gchar zuin[256];
  gchar *ubuffer;
  gint ls, rs;
  gint userfreq, time, maxfreq, origfreq;
  gint i, tmp;

  ls = 126;
  rs = ls * 3 / 2 + 1;
  memset(buffer, 0, ls);
  ubuffer = (gchar *)calloc(1, sizeof(gchar) * rs);
  hash = fopen(filename, "rb");
  chewing_lifetime = 0;

  setlocale(LC_CTYPE, "zh_TW.UTF-8");
  while(1)
    {
      memset(buffer, 0, ls);
      memset(ubuffer, 0, rs);
      memset(zuin, '\0', sizeof(zuin));
      if ( fscanf( hash, "%s", buffer) != 1 )
	break;
      convert(buffer, ubuffer, strlen( buffer ));

      /* read phoneSeq */
      for ( i = 0; i < strlen( buffer ) / 2; i++ )
	{
	  memset(buffer2, '\0', ls);
	  fscanf( hash, "%d", &tmp);
	  Uint2PhoneUTF8(buffer2, tmp);
	  strcat( zuin, buffer2 );
	  strcat( zuin, " " );
	}
      /* read userfreq & recentTime */
      if ( fscanf( hash, "%d %d %d %d",
		   &(userfreq),
		   &(time),
		   &(maxfreq),
		   &(origfreq) ))
	{
	  gtk_tree_store_append (store, &iter, NULL);
	  gtk_tree_store_set (store, &iter,
			      SEQ_COLUMN, ubuffer,
			      ZUIN_COLUMN, zuin,
			      USERFREQ_COLUMN, userfreq,
			      TIME_COLUMN, time,
			      MAXFREQ_COLUMN, maxfreq,
			      ORIGFREQ_COLUMN, origfreq,
			      -1);
	}
    }
	
  fclose(hash);
}

void
che_read_hash_bin(gchar *filename)
{
  FILE *hash;
  gchar buffer[126];
  gchar buffer2[126];
  gchar zuin[256];
  gchar *ubuffer;
  gint ls, rs;
  gint userfreq, time, maxfreq, origfreq;
  gint i, tmp;
  gint len;
  unsigned char recbuf[FIELD_SIZE];
  unsigned char *puchar;
  uint16_t *pshort;
  int header_length = strlen(BIN_HASH_SIG) + sizeof(uint32_t);

  ls = 126;
  rs = ls * 3 / 2 + 1;
  memset(buffer, 0, ls);
  ubuffer = (gchar *)calloc(1, sizeof(gchar) * rs);
  hash = fopen(filename, "rb");

  fseek(hash, strlen(BIN_HASH_SIG), SEEK_SET); /* skip signature */
  fread(&chewing_lifetime, sizeof(uint32_t), 1, hash);

  setlocale(LC_CTYPE, "zh_TW.UTF-8");
  while(1)
    {
      if ( fread( recbuf, FIELD_SIZE, 1, hash) != 1 )
			break; /* EOF or error */

      memset(buffer, 0, ls);
      memset(ubuffer, 0, rs);
      memset(zuin, '\0', sizeof(zuin));

		/* frequency info */
		userfreq = *(uint32_t*)(recbuf + 0);
		time = *(uint32_t*)(recbuf + 4);
		maxfreq = *(uint32_t*)(recbuf + 8);
		origfreq = *(uint32_t*)(recbuf + 12);

		/* string length in number of chinese characters */
		len = (int)recbuf[16];

      /* read phoneSeq */
		pshort = (uint16_t*)(recbuf + 17);
      for ( i = 0; i < len; i++ )
		{
		  memset(buffer2, '\0', ls);
		  tmp = *pshort;
		  ++pshort;
		  Uint2PhoneUTF8(buffer2, tmp);
		  strcat( zuin, buffer2 );
		  strcat( zuin, " " );
		}

		/* phrase length in number of bytes */
		puchar = (unsigned char*)pshort;
		memcpy(ubuffer, puchar+1, (int)*puchar);
		ubuffer[(int)*puchar] = 0;

		/* skip invalid utf8 strings */
		if (!chewing_utf8_is_valid_str(ubuffer))
			continue;

		//printf("str: %s, zhuin: %s\n", ubuffer, zuin);

		/* store item */
	   gtk_tree_store_append (store, &iter, NULL);
	   gtk_tree_store_set (store, &iter,
			      SEQ_COLUMN, ubuffer,
			      ZUIN_COLUMN, zuin,
			      USERFREQ_COLUMN, userfreq,
			      TIME_COLUMN, time,
			      MAXFREQ_COLUMN, maxfreq,
			      ORIGFREQ_COLUMN, origfreq,
			      -1);
    }
	
  fclose(hash);
}

void
che_read_hash(gchar *filename)
{
	/* check if the file is a binary hash */
	FILE *f = fopen(filename, "rb");
	if (f) {
		char head[strlen(BIN_HASH_SIG)];
		fread(head, sizeof(head), 1, f);
		fclose(f);
		if (memcmp(head, BIN_HASH_SIG, strlen(BIN_HASH_SIG)) == 0) /* binary hash */
			che_read_hash_bin(filename), che_set_hash_format(HF_BINARY);
#ifdef ENABLE_TEXT_HASH
		else /* fallback to text hash */
			che_read_hash_txt(filename), che_set_hash_format(HF_TEXT);
#endif
		strcpy(current_filename, filename);
	}
}

/* import database from text file
 * The file must contain at least two tab-separated fields.
 * The first field is the phrase, and the second field is
 * its zhuin symbol.
 **/
void
che_import_txt(gchar *filename)
{
  FILE *file;
  char line_buffer[1024];
  file = fopen(filename, "rb");

  setlocale(LC_CTYPE, "zh_TW.UTF-8");
  while(fgets(line_buffer, 1024, file))
    {
	   char *pos;
		int index;
	   gchar phrase[32] = {};
	   gchar zhuin[256] = {};
	   gint userfreq = 0, phrase_time = 0, maxfreq = 0, origfreq = 0;
		pos = strtok(line_buffer, "\t");
		index = 0;
		while (pos != NULL) {
			switch (index) {
				case 0: /* phrase */
					strcpy(phrase, pos);
					break;
				case 1: /* zhuin */
					strcpy(zhuin, pos);
					break;
				case 2: /* userfreq */
					userfreq = atoi(pos);
					break;
				case 3: /* time */
					phrase_time = atoi(pos);
					break;
				case 4: /* maxfreq */
					maxfreq = atoi(pos);
					break;
				case 5: /* origfreq */
					origfreq = atoi(pos);
					break;
				default:
					break;
			}
			pos = strtok(NULL, "\t");
			++index;
		}
      /* read userfreq & recentTime */
		if (strlen(phrase) > 0 && strlen(zhuin) > 0)
      {
		  gtk_tree_store_append (store, &iter, NULL);
		  gtk_tree_store_set (store, &iter,
						SEQ_COLUMN, phrase,
						ZUIN_COLUMN, zhuin,
						USERFREQ_COLUMN, userfreq,
						TIME_COLUMN, phrase_time,
						MAXFREQ_COLUMN, maxfreq,
						ORIGFREQ_COLUMN, origfreq,
						-1);
	   }
    }
	
  fclose(file);
}

void
che_quit(GtkWindow *ptr /* reserved */)
{
	file_save(NULL);
	gtk_main_quit();
}

/* callback */
void
file_open( GtkWindow *parient )
{
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new ("Open File",
					parient,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
  gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog), TRUE);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      gtk_tree_store_clear( store );
      hashfilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    }

  gtk_widget_destroy (dialog);
}

void
file_save_as( GtkWindow *parient )
{
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new ("Save As",
					parient,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					NULL);
  gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog), TRUE);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
	   gchar *filename;
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      file_save( filename );
      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}

void
file_save_txt( gchar *fname )
{
  gchar *zuin;
  gchar *buffer, buf[125], str[125], bbuf[125];
  gchar *pos;
  gint userfreq, time, maxfreq, origfreq;
  FILE *file;

  if ( fname == NULL )
    file = fopen( current_filename, "w" );
  else
    file = fopen( fname, "w" );

  fprintf(file,"%-125s", "0");
  if (gtk_tree_model_get_iter_first( GTK_TREE_MODEL(store), &iter)) {
	  do {
		 gtk_tree_model_get (GTK_TREE_MODEL(store), &iter,
				SEQ_COLUMN, &buffer,
				ZUIN_COLUMN, &zuin,
				USERFREQ_COLUMN, &userfreq,
				TIME_COLUMN, &time,
				MAXFREQ_COLUMN, &maxfreq,
				ORIGFREQ_COLUMN, &origfreq,
				-1);
		 convertu2b(buffer, bbuf, 125);
		 sprintf(str,"%s ", bbuf);
		 pos = strtok(zuin, " "); 
		 while( pos != NULL ) {
			sprintf(buf, "%d ", zhuin_to_inx(pos));
			strcat(str, buf);
			pos = strtok(NULL, " ");
		 }
		 sprintf( buf, "%d %d %d %d", userfreq, time, maxfreq, origfreq );
		 strcat(str, buf);
		 fprintf(file,"%-125s", str);
		 g_free(buffer);
		 g_free(zuin);
	  } while( gtk_tree_model_iter_next( GTK_TREE_MODEL(store), &iter ) );
  }
  fclose(file);
}

void
file_save_bin( gchar *fname )
{
  gchar *zuin;
  gchar *buffer, buf[125], str[125], bbuf[125];
  gchar *pos;
  uint32_t userfreq, time, maxfreq, origfreq;
  uint8_t phrase_length; /* zuin string length in chinese characters */
  uint8_t string_length; /* string length in bytes */
  uint16_t *pshort;
  uint8_t *pchar;
  FILE *file;
  int i;

  if ( fname == NULL )
    file = fopen( current_filename, "wb" );
  else
    file = fopen( fname, "wb" );

  /* write header: signature + chewing lifetime */
  fwrite(BIN_HASH_SIG, strlen(BIN_HASH_SIG), 1, file);
  fwrite(&chewing_lifetime, sizeof(uint32_t), 1, file); /* TODO: the value of chewing_lifetime */

  if (gtk_tree_model_get_iter_first( GTK_TREE_MODEL(store), &iter)) {
	  do {
		 char writebuf[FIELD_SIZE] = {};
		 gtk_tree_model_get (GTK_TREE_MODEL(store), &iter,
				SEQ_COLUMN, &buffer,
				ZUIN_COLUMN, &zuin,
				USERFREQ_COLUMN, &userfreq,
				TIME_COLUMN, &time,
				MAXFREQ_COLUMN, &maxfreq,
				ORIGFREQ_COLUMN, &origfreq,
				-1);

		 /* frequency info */
		 *(uint32_t*)(writebuf + 0)  = userfreq;
		 *(uint32_t*)(writebuf + 4)  = time;
		 *(uint32_t*)(writebuf + 8)  = maxfreq;
		 *(uint32_t*)(writebuf + 12) = origfreq;

		 /* phone seq */
		 phrase_length = chewing_utf8_strlen(buffer);
		 writebuf[16] = phrase_length;
		 pshort = (uint16_t*)(writebuf + 17);
		 pos = strtok(zuin, " ");
		 for (i=0; i<phrase_length && pos != NULL; i++) {
			/* TODO: check for errors, phrase length may differ from zhuin length */
			*pshort = zhuin_to_inx(pos);
			pos = strtok(NULL, " ");
			++pshort;
		 }

		 /* phrase length in bytes */
		 
		 string_length = strlen(buffer);
		 pchar = (uint8_t*)pshort;
		 *pchar = string_length;
		 memcpy(pchar+1, buffer, string_length);

		 /* write to file */
		 fwrite(writebuf, sizeof(writebuf), 1, file);

		 g_free(buffer);
		 g_free(zuin);
	  } while( gtk_tree_model_iter_next( GTK_TREE_MODEL(store), &iter ) );
  }
  fclose(file);
}

void
file_save( gchar *fname )
{
	const char tmp_suffix[] = ".tmp";
	char *tmp_filename;

	if (fname)
		strcpy(current_filename, fname);

	/* write to <current_filename>.tmp */
	tmp_filename = malloc(strlen(current_filename) + sizeof(tmp_suffix) + 1);
	sprintf(tmp_filename, "%s%s", current_filename, tmp_suffix);

#ifdef ENABLE_TEXT_HASH
	switch (che_get_hash_format())
	{
	case HF_TEXT:
		file_save_txt(tmp_filename);
		break;
	case HF_BINARY:
		file_save_bin(tmp_filename);
		break;
	}
#else
	file_save_bin(tmp_filename); /* always save as binary hash */
#endif

	/* when done, replace <current_filename> with <current_filename>.tmp */
	if (rename(tmp_filename, current_filename) != 0) {
		fprintf(stderr, "error: failed to save file to %s\n", current_filename);
	} else {
		is_file_saved = TRUE;
	}

	free(tmp_filename);
}

/* export the database to a text file */
void
file_export_as_text( gchar *fname )
{
  gchar *zuin;
  gchar *buffer, buf[125], str[125];
  gchar *pos;
  gint userfreq, time, maxfreq, origfreq;
  FILE *file;

  if ( fname == NULL )
    return;

  file = fopen( fname, "w" );

  gtk_tree_model_get_iter_first( GTK_TREE_MODEL(store), &iter);
  do {
    gtk_tree_model_get (GTK_TREE_MODEL(store), &iter,
			SEQ_COLUMN, &buffer,
			ZUIN_COLUMN, &zuin,
			USERFREQ_COLUMN, &userfreq,
			TIME_COLUMN, &time,
			MAXFREQ_COLUMN, &maxfreq,
			ORIGFREQ_COLUMN, &origfreq,
			-1);
    sprintf(str, "%s\t", buffer);
	 sprintf(buf, "%s\t", zuin);
	 strcat(str,buf);
    sprintf( buf, "%d\t%d\t%d\t%d", userfreq, time, maxfreq, origfreq );
    strcat(str, buf);
    fprintf(file,"%s\n", str);
    g_free(buffer);
    g_free(zuin);
  } while( gtk_tree_model_iter_next( GTK_TREE_MODEL(store), &iter ) );
  fclose(file);
}

void file_export(void *ptr)
{
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new ("匯出",
					main_window,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					NULL);
  gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog), TRUE);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
	   gchar *filename;
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      file_export_as_text( filename );
      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}

void file_import(void *ptr)
{
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new ("匯入",
					main_window,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					NULL);
  gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog), TRUE);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
	   gchar *filename;
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      che_import_txt(filename);
      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}

void
che_remove_phrase(GtkWidget *widget)
{
  gboolean valid = gtk_tree_selection_get_selected(selection,
				  NULL,
				  &iter);
  if (valid)
    valid = gtk_tree_store_remove(store, &iter);
  if (valid)
    gtk_tree_selection_select_iter(selection, &iter); /* select the next item */
}

void
che_select_prev_phrase()
{
	GtkTreePath *path;
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
	if (path && gtk_tree_path_prev(path)) {
		gtk_tree_model_get_iter(store, &iter, path);
		gtk_tree_selection_select_iter(selection, &iter);
	}
	gtk_tree_path_free(path);
}

void
che_select_next_phrase()
{
	gboolean valid = gtk_tree_selection_get_selected(selection,
				  NULL,
				  &iter);
	if (valid) {
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
		if (valid)
			gtk_tree_selection_select_iter(selection, &iter); /* select the next item */
	}
}

void
convert(char *input, char *output, int n_char)
{
  const char *inptr = input;
  size_t inbytesleft = n_char;
  size_t outbytesleft = n_char / 2 * 3 + 1;

  char *outptr = output;
  iconv_t cd;

  cd = iconv_open("UTF-8", "BIG-5");
  iconv (cd, (char **)&inptr, &inbytesleft, &outptr, &outbytesleft);

  iconv_close(cd);
}

void
convertu2b(char *input, char *output, int n_char)
{
  const char *inptr = input;
  size_t inbytesleft = n_char;
  size_t outbytesleft = n_char / 3 * 2 + 1;

  char *outptr = output;
  iconv_t cd;

  cd = iconv_open("BIG5", "UTF-8");
  iconv (cd, (char **)&inptr, &inbytesleft, &outptr, &outbytesleft);

  iconv_close(cd);
}

void
cell_edited(GtkCellRendererText *cellrenderertext, gchar *p, gchar *value, gpointer column)
{
  GtkTreePath *path;
  path = gtk_tree_path_new_from_string( p );
  gtk_tree_model_get_iter( GTK_TREE_MODEL(store), &iter, path );
  gtk_tree_store_set (store, &iter,
		      atoi((gpointer)column), atoi(value),
		      -1);
}
/* ------------------------------- */
struct _widget_pair {
  GtkWidget *w1;
  GtkWidget *w2;
};
typedef struct _widget_pair wp_t;

GtkWidget *che_new_phrase_box();
void entry_active_callback(GtkWidget *obj, gpointer vbox);
void button_click_callback(GtkWidget *btn, gpointer parent);
/*
int main(int argc, char *argv[])
{
  GtkWidget *main_window;
  GtkWidget *vbox;
  GtkWidget *testbox;
  
  gtk_init(&argc, &argv);

  main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title( GTK_WINDOW (main_window), "Test" );
  g_signal_connect (G_OBJECT (main_window), "delete_event",
		    G_CALLBACK (gtk_main_quit), NULL);
  vbox = gtk_vbox_new(FALSE, 1);
  testbox = (GtkWidget*)che_new_phrase_box();
  gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(testbox),
		      FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(vbox));
  gtk_widget_show_all(main_window);

  gtk_main();
  return 0;
}
*/

/* Show the phrase editor dialog for adding new phrases. */
void che_new_phrase_dlg(GtkWidget *widget)
{
  is_editing_existing_phrase = FALSE;
  che_phrase_dlg("新增語詞");
}

/* Show the phrase editor dialog for editing an existing phrase. */
void che_edit_phrase_dlg(GtkWidget *widget)
{
  is_editing_existing_phrase = TRUE;
  che_phrase_dlg("編輯語詞");
}

void che_phrase_dlg(const char *title)
{
  GtkWidget *dlg = gtk_dialog_new_with_buttons (title,
						main_window,
						GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
						NULL);
  GtkWidget *dlgbox = (GtkWidget*)che_new_phrase_box();
  editor_dialog = dlg;
  gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dlg)->vbox), dlgbox);
  g_signal_connect_swapped (dlg,
			    "response",
			    G_CALLBACK (gtk_widget_destroy),
			    dlg);
  gtk_widget_show_all(GTK_WIDGET(dlg));
}

GtkWidget *che_new_phrase_box()
{
  GtkWidget *vbox_top;
  GtkWidget *vbox0;
  GtkWidget *hbox0;
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *btn, *btn2;
  
  vbox_top = gtk_vbox_new(FALSE, 1);
  vbox0 = gtk_vbox_new(FALSE, 1);
  hbox0 = gtk_hbox_new(FALSE, 1);
  vbox1 = gtk_vbox_new(FALSE, 1);
  gtk_box_pack_start( GTK_BOX(vbox_top), GTK_WIDGET(vbox0),
		      FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox0), GTK_WIDGET(hbox0),
		      FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox_top), GTK_WIDGET(vbox1),
		      FALSE, FALSE, 0);
  label = gtk_label_new("新詞：");
  entry = gtk_entry_new_with_max_length(20);
  btn = gtk_button_new_with_label("轉成注音");
  btn2 = gtk_button_new_from_stock(GTK_STOCK_SAVE);
  gtk_box_pack_start_defaults( GTK_BOX(hbox0), GTK_WIDGET(label) );
  gtk_box_pack_start_defaults( GTK_BOX(hbox0), GTK_WIDGET(entry) );
  gtk_box_pack_start_defaults( GTK_BOX(hbox0), GTK_WIDGET(btn) );
  gtk_box_pack_start_defaults( GTK_BOX(vbox_top), GTK_WIDGET(btn2));

  if (is_editing_existing_phrase) { /* fill in the original data */
    gchar *buffer, *zhuin;
    gboolean valid = gtk_tree_selection_get_selected(selection,
	      NULL,
         &iter);
    if (valid) {
	   int length;
      gtk_tree_model_get (GTK_TREE_MODEL(store), &iter,
  			SEQ_COLUMN, &buffer,
  			ZUIN_COLUMN, &zhuin,
  			-1);
  	   gtk_entry_set_text(entry, buffer);
		entry_active(entry, (gpointer)vbox1, zhuin);

  	   g_free(buffer);
  	   g_free(zhuin);
	 }
  }

  g_signal_connect( G_OBJECT(entry), "activate",
		    G_CALLBACK(entry_active_callback), (gpointer)vbox1);
  g_signal_connect( G_OBJECT(btn), "clicked", G_CALLBACK(entry_active_callback), (gpointer)entry);
  g_signal_connect( G_OBJECT(btn2), "clicked", G_CALLBACK(che_save_phrase), (gpointer)vbox1);
  
  return vbox_top;
}

GtkWidget *che_new_label_button_box(char *tsi, char *zhuin)
{
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *button;
  
  hbox = gtk_hbox_new(FALSE, 5);
  label = gtk_label_new(tsi);
  gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET (label),
		    FALSE, FALSE, 10);
  button = gtk_button_new_with_label(zhuin);
  gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET (button),
		    FALSE, FALSE, 0);
  g_signal_connect( button, "clicked", G_CALLBACK(button_click_callback), (gpointer)(GTK_WIDGET(hbox)->parent));
  return hbox;  
}

void entry_active(GtkWidget *obj, gpointer vbox, const char *zhuin)
{
  int i, length;
  char *chr_zhuin;
  char zhuin_buffer[10];
  GtkWidget *box = vbox;
  GtkWidget *bl;
  gchar buf[4];
  gchar *text = gtk_entry_get_text(GTK_ENTRY(obj));
  length = chewing_utf8_strlen(text);

  gtk_container_foreach(GTK_CONTAINER(box), gtk_widget_destroy, NULL);

  if (zhuin)
    chr_zhuin = strtok(zhuin, " ");
  else
    chr_zhuin = "      ";
  for(i = 0; i < length; i++) {
    chewing_utf8_strncpy(buf, chewing_utf8_strseek(text, i), 1, 1);
	 /* try to find the zhuin for the chinese character */
    if (zhuin_dictionary && find_zhuin(zhuin_dictionary, buf, zhuin_buffer)) {
    	chr_zhuin = zhuin_buffer;
    } else {
      chr_zhuin = "      ";
    }
    bl = che_new_label_button_box(buf, chr_zhuin);
    gtk_widget_show_all(GTK_WIDGET(bl));
    gtk_box_pack_start_defaults( GTK_BOX(box), GTK_WIDGET (bl));
	 if (zhuin)
	   chr_zhuin = strtok(NULL, " ");
  }
}

void entry_active_callback(GtkWidget *obj, gpointer vbox)
{
  if(!GTK_IS_ENTRY(obj))
    {
      g_signal_emit_by_name(G_OBJECT(vbox), "activate", G_TYPE_NONE);
      return;
    }
  entry_active(obj, vbox, NULL);
}

void che_save_phrase(GtkWidget *obj, gpointer vbox)
{
  GtkWidget *box = vbox;
  GtkWidget *hbox;
  char zuin[256];
  char phrase[256];
  gboolean valid;

  GList *hboxes = gtk_container_get_children(GTK_CONTAINER(box));
  hboxes = g_list_first(hboxes);
  if (!hboxes)
	  return;
  memset(phrase, '\0', sizeof(phrase));
  memset(zuin, '\0', sizeof(zuin));
  do
    {
      GtkWidget *lab = g_list_nth_data(gtk_container_get_children(GTK_CONTAINER(hboxes->data)), 0);
      GtkWidget *btn = g_list_nth_data(gtk_container_get_children(GTK_CONTAINER(hboxes->data)), 1);
      gchar *label = gtk_button_get_label(btn);
      strcat(zuin, label);
      strcat(zuin, " ");
      strcat(phrase, GTK_LABEL(lab)->label);
    }
  while((hboxes = g_list_next(hboxes)) != NULL);
  

  if (is_editing_existing_phrase)
    valid = gtk_tree_selection_get_selected(selection, NULL, &iter);
  else
    gtk_tree_store_prepend (store, &iter, NULL), valid = TRUE;

  if (valid) {
    gtk_tree_store_set (store, &iter,
		      SEQ_COLUMN, phrase,
		      ZUIN_COLUMN, zuin,
		      -1);
    if (!is_editing_existing_phrase) {
      gtk_tree_store_set (store, &iter,
		      USERFREQ_COLUMN, 0,
		      TIME_COLUMN, 0,
		      MAXFREQ_COLUMN, 0,
		      ORIGFREQ_COLUMN, 0, -1);
	 }
  }
  /* close the editor dialog */
  g_signal_emit_by_name(G_OBJECT(editor_dialog), "response", G_TYPE_NONE);
  is_file_saved = FALSE;
}

void append_text(GtkWidget *btn, gpointer entry)
{
  gchar *label = gtk_button_get_label(GTK_BUTTON(btn));
  gtk_entry_append_text(GTK_ENTRY(entry), label);
}

void che_set_context(GtkWidget *dlg, gint arg, wp_t *pwp)
{
  wp_t *wp = (wp_t*)pwp;
  gchar *context = gtk_entry_get_text(GTK_ENTRY(wp->w2));
  gtk_button_set_label(GTK_BUTTON(wp->w1), context);
  gtk_widget_destroy(dlg);
  free(pwp);
}

void button_click_callback(GtkWidget *cbtn, gpointer parent)
{
  const char *zhuin_tab[] = {
    "ㄅ", "ㄆ", "ㄇ", "ㄈ", "ㄉ", "ㄊ", "ㄋ", "ㄌ", "ㄍ", "ㄎ", "ㄏ",
    "ㄐ", "ㄑ", "ㄒ", "ㄓ", "ㄔ", "ㄕ", "ㄖ", "ㄗ", "ㄘ", "ㄙ", "ㄧ",
    "ㄨ", "ㄩ", "ㄚ", "ㄛ", "ㄜ", "ㄝ", "ㄞ", "ㄟ", "ㄠ", "ㄡ", "ㄢ",
    "ㄣ", "ㄤ", "ㄥ", "ㄦ", "˙", "ˊ", "ˇ", "ˋ", " ",
  };
  int i = 0;

  GtkWidget *window = parent;
  wp_t *wp;
  GtkWidget *hbox, *btn, *entry;
  GtkWidget *dlg = gtk_dialog_new_with_buttons ("設定注音",
						GTK_WINDOW(window),
						GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
						GTK_STOCK_OK,
						GTK_RESPONSE_OK,
						NULL);
  entry = gtk_entry_new();
  gtk_entry_set_text(entry, gtk_button_get_label(cbtn));
  gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dlg)->vbox), entry);
  hbox = gtk_hbox_new(FALSE, 0);
  for(i = 0; i < 11; i++)
    {
      btn = gtk_button_new_with_label(zhuin_tab[i]);
      gtk_signal_connect(GTK_WIDGET(btn), "clicked", G_CALLBACK(append_text), (gpointer)entry);
      gtk_box_pack_start_defaults(GTK_BOX(hbox), btn);
    }
  gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dlg)->vbox), hbox);
  hbox = gtk_hbox_new(FALSE, 0);
  for(i = 11; i < 22; i++)
    {
      btn = gtk_button_new_with_label(zhuin_tab[i]);
      gtk_signal_connect(GTK_WIDGET(btn), "clicked", G_CALLBACK(append_text), (gpointer)entry);
      gtk_box_pack_start_defaults(GTK_BOX(hbox), btn);
    }
  gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dlg)->vbox), hbox);
  hbox = gtk_hbox_new(FALSE, 0);
  for(i = 22; i < 33; i++)
    {
      btn = gtk_button_new_with_label(zhuin_tab[i]);
      gtk_signal_connect(btn, "clicked", G_CALLBACK(append_text), (gpointer)entry);
      gtk_box_pack_start_defaults(GTK_BOX(hbox), btn);
    }
  gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dlg)->vbox), hbox);
  hbox = gtk_hbox_new(FALSE, 0);
  for(i = 33; i < 42; i++)
    {
      btn = gtk_button_new_with_label(zhuin_tab[i]);
      gtk_signal_connect(btn, "clicked", G_CALLBACK(append_text), (gpointer)entry);
      gtk_box_pack_start_defaults(GTK_BOX(hbox), btn);
    }
  gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dlg)->vbox), hbox);
  wp = (void*)malloc(sizeof(wp_t));
  wp->w1 = cbtn;
  wp->w2 = entry;
  g_signal_connect (dlg,
		    "response", 
		    G_CALLBACK (che_set_context),
		    wp);
  gtk_widget_show_all(GTK_WIDGET(dlg));
}

void
treeview_keypress_callback(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	if (event->type == GDK_KEY_PRESS) {
		switch (event->key.keyval) {
		case GDK_Delete:
			che_remove_phrase(widget);
			break;
		case GDK_Up: /* select previous phrase */
			che_select_prev_phrase();
			break;
		case GDK_Down: /* select next phrase */
			che_select_next_phrase();
			break;
		}
	}
}

void
treeview_row_activated_callback(GtkTreeView* treeview,
	GtkTreePath* path, GtkTreeViewColumn* column, gpointer ptr)
{
	che_edit_phrase_dlg(treeview);
}

int match_phrase(char *key, char *text)
{
	if (strlen(key) > strlen(text)) return 0;
	return memcmp(key, text, strlen(key)) == 0;
}

/* select the row and make it visible */
void che_select_row(GtkTreeIter *it)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	gtk_tree_selection_select_iter(selection, it);
	gtk_tree_selection_get_selected(selection, &model, it);
	if (model) {
		path = gtk_tree_model_get_path(model, it);
		if (path) {
			gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(main_tree_view), path, NULL, FALSE, 0, 0);
			gtk_tree_path_free(path);
		}
	}
}

void search_phrase_callback(GtkWidget *obj, gpointer entry)
{
  gchar *ptr;
  gchar buffer[256];
  const gchar *entry_text;
  gboolean valid;

  entry_text = gtk_entry_get_text(entry);
  if (strlen(entry_text) == 0) goto finished;

  /* get the selected item */
  valid = gtk_tree_selection_get_selected(selection, NULL, &iter);
  if (valid) /* begin searching with the next item */
    valid = gtk_tree_model_iter_next( GTK_TREE_MODEL(store), &iter);
  else /* if there's no item selected, begin with the first item */
    valid = gtk_tree_model_get_iter_first( GTK_TREE_MODEL(store), &iter);

  if (valid) {
    int i;
	 for (i=0; i<2; i++) {
      do {
        gtk_tree_model_get (GTK_TREE_MODEL(store), &iter,
    			SEQ_COLUMN, &ptr,
    			-1); 
        strcpy(buffer, ptr);
		  //printf("phrase: %s\n", buffer);
        g_free(ptr);
        if (match_phrase(entry_text, buffer)) {
		    che_select_row(&iter);
  		    goto finished;
        }
      } while (gtk_tree_model_iter_next( GTK_TREE_MODEL(store), &iter));
		gtk_tree_model_get_iter_first( GTK_TREE_MODEL(store), &iter); /* wrap search */
    }
	 /* phrase not found */
	 
  }

finished:
  strcpy(prev_search_text, entry_text);

  /* close the search dialog */
  g_signal_emit_by_name(G_OBJECT(search_dialog), "response", G_TYPE_NONE);
}

GtkWidget *che_new_search_box()
{
  GtkWidget *vbox_top;
  GtkWidget *vbox0;
  GtkWidget *hbox0;
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *btn;
  
  vbox_top = gtk_vbox_new(FALSE, 1);
  vbox0 = gtk_vbox_new(FALSE, 1);
  hbox0 = gtk_hbox_new(FALSE, 1);
  vbox1 = gtk_vbox_new(FALSE, 1);
  gtk_box_pack_start( GTK_BOX(vbox_top), GTK_WIDGET(vbox0),
		      FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox0), GTK_WIDGET(hbox0),
		      FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox_top), GTK_WIDGET(vbox1),
		      FALSE, FALSE, 0);
  label = gtk_label_new("搜尋開頭文字：");
  entry = gtk_entry_new_with_max_length(20);
  btn = gtk_button_new_with_label("確定");
  gtk_box_pack_start_defaults( GTK_BOX(hbox0), GTK_WIDGET(label) );
  gtk_box_pack_start_defaults( GTK_BOX(hbox0), GTK_WIDGET(entry) );
  gtk_box_pack_start_defaults( GTK_BOX(hbox0), GTK_WIDGET(btn) );

  gtk_entry_set_text(entry, prev_search_text);

  g_signal_connect( G_OBJECT(entry), "activate",
		    G_CALLBACK(search_phrase_callback), (gpointer)entry);
  g_signal_connect( G_OBJECT(btn), "clicked", G_CALLBACK(search_phrase_callback), (gpointer)entry);
  
  return vbox_top;
}

void che_show_search_dlg(GtkWidget *widget)
{
  GtkWidget *dlg = gtk_dialog_new_with_buttons ("搜尋",
						main_window,
						GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
						NULL);
  GtkWidget *dlgbox = (GtkWidget*)che_new_search_box();
  search_dialog = dlg;
  gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dlg)->vbox), dlgbox);
  g_signal_connect_swapped (dlg,
			    "response",
			    G_CALLBACK (gtk_widget_destroy),
			    dlg);
  gtk_widget_show_all(GTK_WIDGET(dlg));
}

void che_show_about_dlg(GtkWidget *widget)
{
	const char *authors[] = {
		"Kan-Ru Chen <kanru@kanru.info>",
		"Timothy Lin <lzh9102@gmail.com>",
		NULL };

	GtkAboutDialog *dialog = gtk_about_dialog_new();

	gtk_about_dialog_set_program_name(dialog, "新酷音詞庫編輯器");
	gtk_about_dialog_set_authors(dialog, authors);
	gtk_about_dialog_set_version(dialog, "2.1");
	gtk_about_dialog_set_comments(dialog, "編輯新酷音(libchewing 1.3.x)的二進位詞庫\n"
	                                      "使用者詞庫位於~/.chewing/uhash.dat");
	gtk_about_dialog_set_website(dialog, "http://chewing.csie.net/");
	gtk_about_dialog_set_website_label(dialog, "開啟專案首頁");

	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_transient_for(dialog, main_window);

	gtk_dialog_run(dialog);

	gtk_widget_destroy(dialog);
}
