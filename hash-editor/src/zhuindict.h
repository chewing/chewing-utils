#ifndef ZHUINDICT_H
#define ZHUINDICT_H


struct zhuindict;

struct zhuindict* zhuindict_loadfromfile(const char *f);

struct zhuindict* zhuindict_load_default_dict();

void zhuindict_free(struct zhuindict *ptr);

const char* find_zhuin(struct zhuindict* dict, const char *s);

#endif /* ZHUINDICT_H */
