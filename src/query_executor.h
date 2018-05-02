#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "graph/graph.h"
#include "parser/ast.h"
#include "redismodule.h"
#include "hexastore/triplet.h"
#include "arithmetic/arithmetic_expression.h"

/* Given AST's MATCH node constructs a graph
 * representing queried entities and the relationships
 * between them. */
void BuildGraph(Graph *graph, Vector *entities);

/* Constructs an arithmetic expression tree foreach none aggregated term. */
void Build_None_Aggregated_Arithmetic_Expressions(AST_ReturnNode *return_node, AR_ExpNode ***expressions, int *expressions_count, Graph* g);

/* Checks if query performs write (Create/Delete/Update) */
int Query_Modifies_KeySpace(const AST_Query *ast);

/* AST alterations. */
void Modify_AST(RedisModuleCtx *ctx, AST_Query *ast, const char *graphName);

AST_Query* ParseQuery(const char *query, size_t qLen, char **errMsg);

#endif
