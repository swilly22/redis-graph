#ifndef __LABEL_STORE_H__
#define __LABEL_STORE_H__

#include "../redismodule.h"
#include "../graph/graph_entity.h"
#include "../dep/rax/rax.h"

#define LABELSTORE_PREFIX "redis_graph_store"

typedef enum {
  STORE_NODE,
  STORE_EDGE,
} LabelStoreType;

/* Keeps track on label schema. */
typedef struct {
  rax *properties;  /* Entities under this Label, have a subset of properties */
} LabelStatistics;

typedef struct {
  rax *items;
  LabelStatistics stats;
  char *label;
} LabelStore;

typedef raxIterator LabelStoreIterator;

/* Generates an ID for a new LabelStore. */
int LabelStore_Id(char **id, LabelStoreType type, const char *graph, const char *label);

/* Get a label store. */
LabelStore *LabelStore_Get(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, const char* label);

/* Get all stores of given type. */
void LabelStore_Get_ALL(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, LabelStore **stores, size_t *stores_len);

/* Returns the number of items within the store */
int LabelStore_Cardinality(LabelStore *store);

/* Inserts a new graph entity. */
void LabelStore_Insert(LabelStore *store, char *id, GraphEntity *entity);

/* Removes entity with ID. */
int LabelStore_Remove(LabelStore *store, char *id);

/* Scans through the entire store. */
void LabelStore_Scan(LabelStore *store, LabelStoreIterator *it);

/* Free store. */
void LabelStore_Free(LabelStore *store);

/* Returns the next id from cursor. */
int LabelStoreIterator_Next(LabelStoreIterator *cursor, char **key, uint16_t *len, void **value);

/* Free iterator. */
void LabelStoreIterator_Free(LabelStoreIterator* iterator);

#endif /* __LABEL_STORE_H__ */