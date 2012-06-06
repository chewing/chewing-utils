#include "zhuindict.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_CAPACITY 32

struct entry
{
	char *p_char;
	char *p_zhuin;
};

/* include default zhuin dictionary */
#include "zhuin_data.h"

struct zhuindict
{
	struct entry *arr;
	int size;
	int capacity;
};

static struct entry*
entryarr_new(int count)
{
	struct entry *ptr = calloc(count, sizeof(struct entry));
	if (!ptr)
		return NULL;
	return ptr;
}

static struct entry*
entryarr_realloc(struct entry *ptr, int count)
{
	return realloc(ptr, sizeof(struct entry) * count);
}

static int
entry_compare(const void *e1, const void *e2)
{
	return strcmp(((struct entry*)e1)->p_char, ((struct entry*)e2)->p_char);
}

static void
entryarr_free(struct entry *ptr, int len)
{
	if (ptr) {
		int i;
		for (i=0; i<len; i++) {
			if (ptr[i].p_char)
				free(ptr[i].p_char);
			if (ptr[i].p_zhuin)
				free(ptr[i].p_zhuin);
		}
		free(ptr);
	}
}

static struct zhuindict*
zhuindict_new()
{
	struct zhuindict *ptr = malloc(sizeof(struct zhuindict));
	if (!ptr)
		return NULL;
	ptr->arr = entryarr_new(INIT_CAPACITY);
	ptr->size = 0;
	ptr->capacity = INIT_CAPACITY;
	return ptr;
}

static void
zhuindict_addentry(struct zhuindict *ptr, const char *c, const char *z)
{
	struct entry *e;
	if (ptr->size == ptr->capacity) { /* extend capacity */
		ptr->arr = entryarr_realloc(ptr->arr, ptr->capacity << 1);
		ptr->capacity <<= 1;
	}
	e = &ptr->arr[ptr->size];
	e->p_char = strdup(c);
	e->p_zhuin = strdup(z);
	ptr->size++;
}

/* this function must be called before searching */
static void
zhuindict_sort(struct zhuindict *ptr)
{
	qsort(ptr->arr, ptr->size, sizeof(struct entry), entry_compare);
}

/* search the pronunciation of the chinese character s encoded in utf-8 */
static struct entry*
zhuindict_search(struct zhuindict *ptr, const char *s)
{
	struct entry cmp;
	cmp.p_char = (char*)s;
	cmp.p_zhuin = NULL;
	return bsearch(&cmp, ptr->arr, ptr->size, sizeof(struct entry), entry_compare);
}

void
zhuindict_free(struct zhuindict *ptr)
{
	if (!ptr)
		return;
	entryarr_free(ptr->arr, ptr->size);
	free(ptr);
}

struct zhuindict*
zhuindict_loadfromfile(const char *filename)
{
	char charbuf[100];
	char zhuinbuf[100];
	FILE *f = fopen(filename, "r");
	if (!f)
		return NULL;
	struct zhuindict *ptr = zhuindict_new();
	while (fscanf(f, "%s %s", charbuf, zhuinbuf) != EOF)
		zhuindict_addentry(ptr, charbuf, zhuinbuf);
	zhuindict_sort(ptr);

	return ptr;
}

struct zhuindict*
zhuindict_load_default_dict()
{
	int i;
	struct entry *p_entry = default_data;
	struct zhuindict *ptr = zhuindict_new();
	for (i=0; i<default_data_len; i++) {
		zhuindict_addentry(ptr, p_entry->p_char, p_entry->p_zhuin);
		++p_entry;
	}
	zhuindict_sort(ptr);
	return ptr;
}

const char*
find_zhuin(struct zhuindict* dict, const char *s)
{
	struct entry *e = zhuindict_search(dict, s);
	return e ? e->p_zhuin : NULL;
}
