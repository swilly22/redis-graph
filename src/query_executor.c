#include "assert.h"
#include "graph/node.h"
#include "stores/store.h"
#include "rmutil/vector.h"
#include "rmutil/util.h"
#include "query_executor.h"
#include "parser/grammar.h"
#include "arithmetic/agg_ctx.h"
#include "arithmetic/repository.h"
#include "hexastore/triplet.h"
#include "hexastore/hexastore.h"
#include "parser/parser_common.h"
#include "./dep/rax/rax.h"

void BuildGraph(Graph *graph, Vector *entities) {
    /* Introduce nodes first. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);

        if(entity->t != N_ENTITY) continue;

        /* No duplicates. */
        if(Graph_GetNodeByAlias(graph, entity->alias) != NULL) continue;

        Node *n = NewNode(INVALID_ENTITY_ID, entity->label);

        /* Add properties. */
        if(entity->properties) {
            size_t prop_count = Vector_Size(entity->properties);
            for(int prop_idx = 0; prop_idx < prop_count; prop_idx+=2) {
                SIValue *key;
                SIValue *value;

                Vector_Get(entity->properties, prop_idx, &key);
                Vector_Get(entity->properties, prop_idx+1, &value);
                char *k = strdup(key->stringval);
                /* TODO: clone value. */
                Node_Add_Properties(n, 1, &k, value);
            }
        }

        Graph_AddNode(graph, n, entity->alias);
    }

    /* Introduce edges. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);

        if(entity->t != N_LINK) continue;

        /* No duplicates. */
        if(Graph_GetEdgeByAlias(graph, entity->alias) != NULL) continue;

        AST_LinkEntity* edge = (AST_LinkEntity*)entity;
        AST_NodeEntity *src_node;
        AST_NodeEntity *dest_node;

        if(edge->direction == N_LEFT_TO_RIGHT) {
            Vector_Get(entities, i-1, &src_node);
            Vector_Get(entities, i+1, &dest_node);
        } else {
            Vector_Get(entities, i+1, &src_node);
            Vector_Get(entities, i-1, &dest_node);
        }

        Node *src = Graph_GetNodeByAlias(graph, src_node->alias);
        Node *dest = Graph_GetNodeByAlias(graph, dest_node->alias);

        Edge *e = NewEdge(INVALID_ENTITY_ID, src, dest, edge->ge.label);

        /* Add properties. */
        if(entity->properties) {
            size_t prop_count = Vector_Size(entity->properties);
            for(int prop_idx = 0; prop_idx < prop_count; prop_idx+=2) {
                SIValue *key;
                SIValue *value;

                Vector_Get(entity->properties, prop_idx, &key);
                Vector_Get(entity->properties, prop_idx+1, &value);
                char *k = strdup(key->stringval);
                /* TODO: clone value. */
                Edge_Add_Properties(e, 1, &k, value);
            }
        }

        Graph_ConnectNodes(graph, src, dest, e, edge->ge.alias);
    }
}

/* Construct an expression tree foreach none aggregated term.
 * Returns a vector of none aggregated expression trees. */
void Build_None_Aggregated_Arithmetic_Expressions(AST_ReturnNode *return_node, AR_ExpNode ***expressions, int *expressions_count, Graph *g) {
    *expressions = malloc(sizeof(AR_ExpNode *) * Vector_Size(return_node->returnElements));
    *expressions_count = 0;

    for(int i = 0; i < Vector_Size(return_node->returnElements); i++) {
        AST_ReturnElementNode *returnElement;
        Vector_Get(return_node->returnElements, i, &returnElement);

        AR_ExpNode *expression = AR_EXP_BuildFromAST(returnElement->exp, g);
        if(!AR_EXP_ContainsAggregation(expression, NULL)) {
            (*expressions)[*expressions_count] = expression;
            (*expressions_count)++;
        }
    }
}

int ReturnClause_ExpandCollapsedNodes(RedisModuleCtx *ctx, AST_Query *ast, const char *graphName) {
    int modified = 0;
    if(!ReturnClause_ContainsCollapsedNodes(ast->returnNode)) return modified;

    /* Create a new return clause */
    Vector *expandReturnElements = NewVector(AST_ReturnElementNode*, Vector_Size(ast->returnNode->returnElements));

    /* Scan return clause, search for collapsed nodes. */
    for(int i = 0; i < Vector_Size(ast->returnNode->returnElements); i++) {
        AST_ReturnElementNode *ret_elem;
        Vector_Get(ast->returnNode->returnElements, i, &ret_elem);
        AST_ArithmeticExpressionNode *exp = ret_elem->exp;

        /* Detect collapsed entity,
         * A collapsed entity is represented by an arithmetic expression
         * of AST_AR_EXP_OPERAND type,
         * The operand type should be AST_AR_EXP_VARIADIC,
         * lastly property should be missing. */
        if(exp->type == AST_AR_EXP_OPERAND &&
                exp->operand.type == AST_AR_EXP_VARIADIC &&
                exp->operand.variadic.property == NULL) {

            /* Return clause doesn't contains entity's label,
             * Find collapsed entity's label. */
            AST_GraphEntity *collapsed_entity = NULL;
            for(int j = 0; j < Vector_Size(ast->matchNode->graphEntities); j++) {
                AST_GraphEntity *ge;
                Vector_Get(ast->matchNode->graphEntities, j, &ge);
                if(strcmp(ge->alias, exp->operand.variadic.alias) == 0) {
                    collapsed_entity = ge;
                    break;
                }
            }
            assert(collapsed_entity != NULL);

            /* Find label's properties. */
            LabelStoreType store_type = (collapsed_entity->t == N_ENTITY) ? STORE_NODE : STORE_EDGE;
            unsigned char *prop = NULL;     /* Entity property. */
            uint16_t prop_len = 0;          /* Length of entity's property. */
            raxIterator it;

            /* Collapsed entity has a label. */
            if(collapsed_entity->label) {
                LabelStore *store = LabelStore_Get(ctx, store_type, graphName, collapsed_entity->label);
                raxStart(&it, store->stats.properties);
                raxSeek(&it, "^", NULL, 0);
                while(raxNext(&it)) {
                    prop = it.key;
                    prop_len = it.key_len;

                    prop[prop_len] = 0;
                    /* Create a new return element foreach property. */
                    AST_ArithmeticExpressionNode *expanded_exp =
                        New_AST_AR_EXP_VariableOperandNode(collapsed_entity->alias, (char*)prop);

                    AST_ReturnElementNode *retElem =
                        New_AST_ReturnElementNode(expanded_exp, ret_elem->alias);

                    Vector_Push(expandReturnElements, retElem);
                }
                raxStop(&it);
            } else {
                /* Entity doesn't have a label.
                 * We don't have a choice but to retrieve all known properties. */
                size_t stores_len = 128;    /* Limit number of labels we'll consider. */
                LabelStore *stores[128];    /* Label stores. */

                LabelStore_Get_ALL(ctx, store_type, graphName, stores, &stores_len);
                rax *properties = raxNew(); /* Holds all properties, discards duplicates. */

                /* Get properties out of label store. */
                for(int store_idx = 0; store_idx < stores_len; store_idx++) {
                    LabelStore *s = stores[store_idx];
                    if(!s->label) continue; /* No label, (this is 'ALL' store). */
                    raxStart(&it, s->stats.properties);
                    raxSeek(&it, "^", NULL, 0);
                    /* Add property to properties triemap. */
                    while(raxNext(&it)) {
                        prop = it.key;
                        prop_len = it.key_len;
                        prop[prop_len] = 0;
                        raxInsert(properties, prop, prop_len, prop, NULL);
                    }
                    raxStop(&it);
                }

                /* Add each property to return clause. */
                raxStart(&it, properties);
                raxSeek(&it, "^", NULL, 0);
                while(raxNext(&it)) {
                    prop = it.key;
                    prop_len = it.key_len;

                    /* Create a new return element foreach property. */
                    prop[prop_len] = 0;
                    AST_ArithmeticExpressionNode *expanded_exp =
                        New_AST_AR_EXP_VariableOperandNode(collapsed_entity->alias, (char*)prop);

                    AST_ReturnElementNode *retElem =
                        New_AST_ReturnElementNode(expanded_exp, ret_elem->alias);

                    Vector_Push(expandReturnElements, retElem);
                }
                raxStop(&it);
                raxFree(properties);
            }
            /* Discard collapsed return element. */
            Free_AST_ReturnElementNode(ret_elem);
            modified = 1;
        } else {
            Vector_Push(expandReturnElements, ret_elem);
        }
    }
    /* Override previous return clause. */
    Vector_Free(ast->returnNode->returnElements);
    ast->returnNode->returnElements = expandReturnElements;
    return modified;
}

int _nameAnonymousNodes(Vector *entities, int *entity_id) {
    int modified = 0;

    /* Foreach graph entity: node/edge. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);

        if (entity->alias == NULL) {
            asprintf(&entity->alias, "anon_%d", *entity_id);
            (*entity_id)++;
            modified = 1;
        }
    }

    return modified;
}

int nameAnonymousNodes(AST_Query *ast) {
    int modified = 0;
    int entity_id = 0;

    if(ast->matchNode)
        modified |= _nameAnonymousNodes(ast->matchNode->graphEntities, &entity_id);

    if(ast->createNode)
        modified |= _nameAnonymousNodes(ast->createNode->graphEntities, &entity_id);

    if(ast->mergeNode)
        modified |= _nameAnonymousNodes(ast->mergeNode->nodes, &entity_id);

    return modified;
}

int inlineProperties(AST_Query *ast) {
    int modified = 0;

    /* Migrate inline filters to WHERE clause. */
    if(!ast->matchNode) return modified;

    Vector *entities = ast->matchNode->graphEntities;
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);

        Vector *properties = entity->properties;
        if(properties == NULL) continue;
        modified = 1;

        SIValue *key;
        SIValue *val;
        /* Create a WHERE condition from each property. */
        for(int i = 0; i < Vector_Size(properties); i+=2) {
            Vector_Get(properties, i, &key);    /* TODO: key might be leaking. */
            Vector_Get(properties, i+1, &val);
            const char *alias = entity->alias;
            const char *property = key->stringval;

            AST_FilterNode *filterNode = New_AST_ConstantPredicateNode(alias, property, EQ, *val);

            /* Create WHERE clause if missing. */
            if(ast->whereNode == NULL) {
                ast->whereNode = New_AST_WhereNode(filterNode);
            } else {
                /* Introduce filter with AND operation. */
                AST_FilterNode *left = ast->whereNode->filters;
                AST_FilterNode *right = filterNode;
                ast->whereNode->filters = New_AST_ConditionNode(left, AND, right);
            }
        }
        Vector_Free(entity->properties);
        entity->properties = NULL;
    }

    return modified;
}

int Query_Modifies_KeySpace(const AST_Query *ast) {
    return (ast->createNode != NULL || ast->deleteNode != NULL);
}

/* Merge clause is replaced by MATCH and CREATE clauses. */
int MergeClauseTransform(AST_Query *ast) {
    int modified = 0;
    if(!ast->mergeNode) return modified;

    /* Clone merge nodes, this is done due to match/create clause free. */
    size_t node_count = Vector_Size(ast->mergeNode->nodes);
    Vector *match_elements = NewVector(AST_GraphEntity*, node_count);
    Vector *create_elements = NewVector(AST_GraphEntity*, node_count);

    AST_GraphEntity *node = NULL;
    while(Vector_Pop(ast->mergeNode->nodes, &node)) {
        /* Clone node's properties. */
        size_t prop_count = Vector_Size(node->properties);
        Vector *clone_properties = NewVector(SIValue*, prop_count);

        for(int i = 0; i < prop_count; i++) {
            SIValue *prop;
            SIValue *clone_prop = malloc(sizeof(SIValue));
            Vector_Get(node->properties, i, &prop);
            *clone_prop = SI_Clone(*prop);
            Vector_Push(clone_properties, clone_prop);
        }

        AST_NodeEntity *clone_node = New_AST_NodeEntity(strdup(node->alias),
                                     strdup(node->label),
                                     clone_properties);

        Vector_Push(match_elements, node); /* Reuse merge node. */
        Vector_Push(create_elements, clone_node);
    }

    /* Introduce new MATCH and CREATE clauses, remove MERGE clause. */
    ast->matchNode = New_AST_MatchNode(match_elements);
    ast->createNode = New_AST_CreateNode(create_elements);
    Free_AST_MergeNode(ast->mergeNode);
    ast->mergeNode = NULL;
    modified = 1;

    return modified;
}

void Modify_AST(RedisModuleCtx *ctx, AST_Query *ast, const char *graphName) {
    /* Each AST modification must return a value indicating if it has modified
     * the AST in some way, the behavior should be such that a modification function F,
     * if F is called more then once, consecutively, F() F() than the second call MUST NOT
     * perform any modifications on the AST. */
    while(1) {
        if(nameAnonymousNodes(ast)) continue;
        if(inlineProperties(ast)) continue;
        if(ReturnClause_ExpandCollapsedNodes(ctx, ast, graphName)) continue;
        if(MergeClauseTransform(ast)) continue;
        break;
    }
}

AST_Query* ParseQuery(const char *query, size_t qLen, char **errMsg) {
    AST_Query *ast = Query_Parse(query, qLen, errMsg);
    return ast;
}