#include "group_cache.h"
#include "../rmutil/vector.h"

void InitGroupCache() {
    __groupCache = raxNew();
}

void CacheGroupAdd(char *key, Group *group) {
    raxInsert(__groupCache, (unsigned char *)key, strlen(key), group, NULL);
}

// Retrives a group,
// Sets group to NULL if key is missing.
void CacheGroupGet(char *key, Group **group) {
    *group = raxFind(__groupCache, (unsigned char *)key, strlen(key));
    if (*group == raxNotFound) {
        *group = NULL;
    }
}

void FreeGroupCache() {
    raxFreeWithCallback(__groupCache, (void (*)(void *))FreeGroup);
}

// Returns an iterator to scan entire group cache
void CacheGroupIter(CacheGroupIterator *it) {
    raxStart(it, __groupCache);
    raxSeek(it, "^", NULL, 0);
}

// Advance iterator and returns key & value in current position.
int CacheGroupIterNext(CacheGroupIterator *it, char **key, Group **group) {
    *key = NULL;
    *group = NULL;

    int res = raxNext(it);
    if(res) {
        *key = (char*)it->key;
        *group = it->data;
    }

    return res;
}
