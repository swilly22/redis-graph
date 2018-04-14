#include "./rax.h"
#include "rax_type.h"

/* declaration of the type for redis registration. */
RedisModuleType *RaxRedisModuleType;

void *RaxType_RdbLoad(RedisModuleIO *rdb, int encver) {
    if (encver != 0) {
        return NULL;
    }
    
    // Determin how many elements are in the trie
    uint64_t elements = RedisModule_LoadUnsigned(rdb);
    rax *r = raxNew();
    
    while (elements--) {
        size_t len;
        char *key = RedisModule_LoadStringBuffer(rdb, &len);
        // TODO: Load value from RDB.
        raxInsert(r, (unsigned char *)key, len, NULL, NULL);
    }
    return r;
}

void RaxType_RdbSave(RedisModuleIO *rdb, void *value) {
    rax *r = (rax *)value;
    int count = raxSize(r);

    RedisModule_SaveUnsigned(rdb, count);
    
    // Scan entire trie.
    raxIterator it;
    raxStart(&it, r);
    raxSeek(&it, "^", NULL, 0);

    while(raxNext(&it)) {
        RedisModule_SaveStringBuffer(rdb, (const char *)it.key, it.key_len);
    }
    raxStop(&it);
}

void RaxType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    // TODO: implement.
}

void RaxType_Free(void *value) {
  rax *r = value;
  raxFree(r);
}

int RaxType_Register(RedisModuleCtx *ctx) {
    RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                                 .rdb_load = RaxType_RdbLoad,
                                 .rdb_save = RaxType_RdbSave,
                                 .aof_rewrite = RaxType_AofRewrite,
                                 .free = RaxType_Free};
    
    RaxRedisModuleType = RedisModule_CreateDataType(ctx, "raxtype01", RAX_TYPE_ENCODING_VERSION, &tm);
    if (RaxRedisModuleType == NULL) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}