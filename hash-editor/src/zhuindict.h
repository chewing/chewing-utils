#ifndef ZHUINDICT_H
#define ZHUINDICT_H


struct zhuindict;

struct zhuindict* zhuindict_loadfromfile(const char *f);

struct zhuindict* zhuindict_load_default_dict();

void zhuindict_free(struct zhuindict *ptr);

/*  Find <s> in <dict> and save the result to <buffer>.
 *  The function returns 1 if an entry is found.
 *  Otherwise, it returns 0.
 */
int find_zhuin(struct zhuindict* dict, const char *s, char *buffer);

#endif /* ZHUINDICT_H */
