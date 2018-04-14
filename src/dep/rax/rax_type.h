#ifndef __RAX_TYPE_H__
#define __RAX_TYPE_H__

#include "../../redismodule.h"

extern RedisModuleType *RaxRedisModuleType;

#define RAX_TYPE_ENCODING_VERSION 1

/* Commands related to the redis rax registration */
int RaxType_Register(RedisModuleCtx *ctx);
void* RaxType_RdbLoad(RedisModuleIO *rdb, int encver);
void RaxType_RdbSave(RedisModuleIO *rdb, void *value);
void RaxType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value);
void RaxType_Free(void *value);

#endif