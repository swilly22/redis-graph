#ifndef __OP_INDEX_SCAN_H
#define __OP_INDEX_SCAN_H

#include "op.h"
#include "../../graph/graph.h"
#include "../../graph/node.h"
#include "../../index/index.h"


typedef struct {
    OpBase op;
    Node **node;            /* node being scanned */
    Node *_node;
    IndexIterator *iter;
} IndexScan;

/* Creates a new IndexScan operation */
OpBase *NewIndexScanOp(Graph *g, Node **node, IndexIterator *iter);
IndexScan* NewIndexScan(Graph *g, Node **node, IndexIterator *iter);

/* IndexScan next operation
 * called each time a new ID is required */
OpResult IndexScanConsume(OpBase *opBase, Graph* graph);

/* Restart iterator */
OpResult IndexScanReset(OpBase *ctx);

/* Frees IndexScan */
void IndexScanFree(OpBase *ctx);

#endif