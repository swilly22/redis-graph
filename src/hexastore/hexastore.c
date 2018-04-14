#include "triplet.h"
#include "hexastore.h"
#include "../dep/rax/rax_type.h"

HexaStore *_NewHexaStore() {
	return raxNew();
}

HexaStore *GetHexaStore(RedisModuleCtx *ctx, const char *id) {
	HexaStore *hexaStore = NULL;
	
	RedisModuleString *rmId = RedisModule_CreateString(ctx, id, strlen(id));
	RedisModuleKey *key = RedisModule_OpenKey(ctx, rmId, REDISMODULE_WRITE);
	RedisModule_FreeString(ctx, rmId);
	
	int type = RedisModule_KeyType(key);

	if (type == REDISMODULE_KEYTYPE_EMPTY) {
		hexaStore = _NewHexaStore();
		RedisModule_ModuleTypeSetValue(key, RaxRedisModuleType, hexaStore);
	}

	hexaStore = RedisModule_ModuleTypeGetValue(key);
	RedisModule_CloseKey(key);
	return hexaStore;
}

void HexaStore_InsertAllPerm(HexaStore* hexaStore, Triplet *t) {
	char triplet[128] 	= {0};
	char subject[32] 	= {0};
	char predicate[64] 	= {0};
	char object[32] 	= {0};
	size_t tripletLength;

	snprintf(subject, 32, "%ld", t->subject->id);
	snprintf(predicate, 64, "%s%s%ld", t->predicate->relationship, TRIPLET_PREDICATE_DELIMITER, t->predicate->id);
	snprintf(object, 32, "%ld", t->object->id);

	tripletLength = snprintf(triplet, 128, "SPO:%s:%s:%s", subject, predicate, object);
	raxInsert(hexaStore, (unsigned char *)triplet, tripletLength, (void*)t, NULL);

	tripletLength = snprintf(triplet, 128, "SOP:%s:%s:%s", subject, object, predicate);
	raxInsert(hexaStore, (unsigned char *)triplet, tripletLength, (void*)t, NULL);

	tripletLength = snprintf(triplet, 128, "PSO:%s:%s:%s", predicate, subject, object);
	raxInsert(hexaStore, (unsigned char *)triplet, tripletLength, (void*)t, NULL);

	tripletLength = snprintf(triplet, 128, "POS:%s:%s:%s", predicate, object, subject);
	raxInsert(hexaStore, (unsigned char *)triplet, tripletLength, (void*)t, NULL);

	tripletLength = snprintf(triplet, 128, "OSP:%s:%s:%s", object, subject, predicate);
	raxInsert(hexaStore, (unsigned char *)triplet, tripletLength, (void*)t, NULL);

	tripletLength = snprintf(triplet, 128, "OPS:%s:%s:%s", object, predicate, subject);
	raxInsert(hexaStore, (unsigned char *)triplet, tripletLength, (void*)t, NULL);
}

void HexaStore_RemoveAllPerm(HexaStore *hexaStore, const Triplet *t) {
	char triplet[128] 	= {0};
	char subject[32] 	= {0};
	char predicate[64] 	= {0};
	char object[32] 	= {0};
	size_t tripletLength;

	snprintf(subject, 32, "%ld", t->subject->id);
	snprintf(predicate, 64, "%s%s%ld", t->predicate->relationship, TRIPLET_PREDICATE_DELIMITER, t->predicate->id);
	snprintf(object, 32, "%ld", t->object->id);
    
	tripletLength = snprintf(triplet, 128, "SPO:%s:%s:%s", subject, predicate, object);
	raxRemove(hexaStore, (unsigned char *)triplet, tripletLength, NULL);

	tripletLength = snprintf(triplet, 128, "SOP:%s:%s:%s", subject, object, predicate);
	raxRemove(hexaStore, (unsigned char *)triplet, tripletLength, NULL);

	tripletLength = snprintf(triplet, 128, "PSO:%s:%s:%s", predicate, subject, object);
	raxRemove(hexaStore, (unsigned char *)triplet, tripletLength, NULL);

	tripletLength = snprintf(triplet, 128, "POS:%s:%s:%s", predicate, object, subject);
	raxRemove(hexaStore, (unsigned char *)triplet, tripletLength, NULL);

	tripletLength = snprintf(triplet, 128, "OSP:%s:%s:%s", object, subject, predicate);
	raxRemove(hexaStore, (unsigned char *)triplet, tripletLength, NULL);

	tripletLength = snprintf(triplet, 128, "OPS:%s:%s:%s", object, predicate, subject);

	Triplet *removedTriplet;
	raxRemove(hexaStore, (unsigned char *)triplet, tripletLength, (void**)&removedTriplet);
	FreeTriplet(removedTriplet);
}

void HexaStore_Search(HexaStore* hexastore, const char *prefix, TripletIterator *it) {
	raxStart(it, hexastore);
	raxSeek(it, ">=", (unsigned char *)prefix, strlen(prefix));
}

void HexaStore_Search_Iterator(HexaStore* hexastore, sds prefix, TripletIterator *it) {
	raxStart(it, hexastore);
	raxSeek(it, ">=", (unsigned char *)prefix, sdslen(prefix));
}
