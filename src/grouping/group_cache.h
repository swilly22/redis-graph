#ifndef GROUP_CACHE_H_
#define GROUP_CACHE_H_

#include "group.h"
#include "../dep/rax/rax.h"
#include "../rmutil/vector.h"

typedef rax CacheGroup;
typedef raxIterator CacheGroupIterator;

static CacheGroup *__groupCache = NULL;

void InitGroupCache();

void CacheGroupAdd(char *key, Group *group);

// Retrives a group,
// Sets group to NULL if key is missing.
void CacheGroupGet(char *key, Group **group);

void FreeGroupCache();

// Initialize an iterator to scan groups.
void CacheGroupIter(CacheGroupIterator *it);

// Advance iterator and returns key & value in current position.
int CacheGroupIterNext(CacheGroupIterator *iter, char **key, Group **group);

#endif