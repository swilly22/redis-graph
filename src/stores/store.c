#include "store.h"
#include "../rmutil/strings.h"
#include "../util/triemap/triemap_type.h"

/* Creates a new LabelStore. */
LabelStore *__new_Store(const char *label) {
    LabelStore *store = calloc(1, sizeof(LabelStore));
    store->items = NewTrieMap();
    store->stats.properties = NewTrieMap();
    if(label) store->label = strdup(label);

    return store;
}

void LabelStore_Free(LabelStore *store, void (*freeCB)(void *)) {
    TrieMap_Free(store->items, freeCB);
    TrieMap_Free(store->stats.properties, freeCB);
    if(store->label) free(store->label);
    free(store);
}

int LabelStore_Id(char **id, LabelStoreType type, const char *graph, const char *label) {
    if(label == NULL) {
        label = "ALL";
    }

    const char* storeType = NULL;

    switch(type) {
        case STORE_NODE:
            storeType = "NODE";
            break;
        case STORE_EDGE:
            storeType = "EDGE";
            break;
        default:
            // Unexpected store type.
            break;
    }
    
    return asprintf(id, "%s_%s_%s_%s", LABELSTORE_PREFIX, graph, storeType, label);
}

LabelStore *LabelStore_Get(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, const char* label) {
	LabelStore *store = NULL;
    char *strKey;
    LabelStore_Id(&strKey, type, graph, label);

    RedisModuleString *rmStoreId = RedisModule_CreateString(ctx, strKey, strlen(strKey));
    free(strKey);
    
	RedisModuleKey *key = RedisModule_OpenKey(ctx, rmStoreId, REDISMODULE_WRITE);
    RedisModule_FreeString(ctx, rmStoreId);

	if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
		store = __new_Store(label);
		RedisModule_ModuleTypeSetValue(key, TrieRedisModuleType, store);
	}

	store = RedisModule_ModuleTypeGetValue(key);
    RedisModule_CloseKey(key);
	return store;
}

/* Get all stores of given type. */
void LabelStore_Get_ALL(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, LabelStore **stores, size_t *stores_len) {
    char *strKey;
    LabelStore_Id(&strKey, type, graph, "*");

    int scan_idx = 0;               /* SCAN cursor. */
    size_t total_stores = 0;        /* Number of stoers retrieved. */
    RedisModuleCallReply *reply;

    /* Consume SCAN */
    do {
        reply = RedisModule_Call(ctx, "SCAN", "lcc", scan_idx, "MATCH", strKey);

        /* First element is the scan cursor, 0 indicates end of SCAN. */
        RedisModuleCallReply *element = RedisModule_CallReplyArrayElement(reply, 0);
        scan_idx = RedisModule_CallReplyInteger(element);

        /* Process SCAN results. */
        RedisModuleCallReply *scan_results = RedisModule_CallReplyArrayElement(reply, 1);
        /* Number of elements in replay. */
        size_t keys_count = RedisModule_CallReplyLength(scan_results);

        /* Extract SCAN result elements. */
        for(int idx = 0; idx < keys_count && *stores_len > total_stores; idx++) {
            element = RedisModule_CallReplyArrayElement(scan_results, idx);
            RedisModuleString *store_key = RedisModule_CreateStringFromCallReply(element);

            RedisModuleKey *key = RedisModule_OpenKey(ctx, store_key, REDISMODULE_WRITE);
            RedisModule_FreeString(ctx, store_key);

            stores[total_stores] = RedisModule_ModuleTypeGetValue(key);
            RedisModule_CloseKey(key);
            total_stores++;
        }
    } while(scan_idx != 0);

    /* Update number of stores fetched. */
    *stores_len = total_stores;
    free(strKey);
}

int LabelStore_Cardinality(LabelStore *store) {
    return store->items->cardinality;
}

void LabelStore_Insert(LabelStore *store, char *id, GraphEntity *entity) {
    if (TrieMap_Add(store->items, id, strlen(id), entity, NULL)) {
        /* Entity is new to the store,
         * update store's entity schema. */
        if(store->label) {
            /* Store has a label, not an 'ALL' store, where there are
             * multiple entities with different labels.
             * Add each of the entity's attribute names to store's stats,
             * We'll be using this information whenever we're required to
             * expand a collapsed entity. */
            int prop_count = entity->prop_count;
            for(int idx = 0; idx < prop_count; idx++) {
                char *prop_name = entity->properties[idx].name;
                TrieMap_Add(store->stats.properties, prop_name, strlen(prop_name), NULL, NULL);
            }
        }
    }
}

int LabelStore_Remove(LabelStore *store, char *id, void (*freeCB)(void *)) {
    return TrieMap_Delete(store->items, id, strlen(id), freeCB);
}

LabelStoreIterator *LabelStore_Search(LabelStore *store, const char *id) {
    char* prefix_dup = strdup(id);
	LabelStoreIterator *iter = TrieMap_Iterate(store->items, prefix_dup, strlen(prefix_dup));
    return iter;
}

int LabelStoreIterator_Next(LabelStoreIterator *cursor, char **key, tm_len_t *len, void **value) {
    return TrieMapIterator_Next(cursor, key, len, value);
}

void LabelStoreIterator_Free(LabelStoreIterator* iterator) {
	TrieMapIterator_Free(iterator);
}