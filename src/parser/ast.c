#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../graph/graph_entity.h"

void _Get_All_Aliases_From_Expression(const AST_ArithmeticExpressionNode *exp, Vector *aliases) {
    /* Find all the aliases in an expression */

    if(exp->type == AST_AR_EXP_OP) {
        /* Process operands. */
        for(int i = 0; i < Vector_Size(exp->op.args); i++) {
            AST_ArithmeticExpressionNode *child;
            Vector_Get(exp->op.args, i, &child);
            _Get_All_Aliases_From_Expression(child, aliases);
        }
    } else {
		/* Check specific operand */
        if(exp->operand.type == AST_AR_EXP_VARIADIC) {
            Vector_Push(aliases, (void*)exp->operand.variadic.alias);
        }
    }
}

AST_Query* New_AST_Query(AST_MatchNode *matchNode, AST_WhereNode *whereNode,
												     AST_CreateNode *createNode, AST_MergeNode *mergeNode,
													 AST_SetNode *setNode, AST_DeleteNode *deleteNode,
													 AST_ReturnNode *returnNode, AST_OrderNode *orderNode,
													 AST_LimitNode *limitNode) {
	AST_Query *queryExpressionNode = (AST_Query*)malloc(sizeof(AST_Query));
	
	queryExpressionNode->matchNode = matchNode;
	queryExpressionNode->whereNode = whereNode;
	queryExpressionNode->createNode = createNode;
	queryExpressionNode->mergeNode = mergeNode;
	queryExpressionNode->setNode = setNode;
	queryExpressionNode->deleteNode = deleteNode;
	queryExpressionNode->returnNode = returnNode;
	queryExpressionNode->orderNode = orderNode;
	queryExpressionNode->limitNode = limitNode;

	return queryExpressionNode;
}

void Free_AST_Query(AST_Query *queryExpressionNode) {
	Free_AST_MatchNode(queryExpressionNode->matchNode);
	Free_AST_CreateNode(queryExpressionNode->createNode);
	Free_AST_MergeNode(queryExpressionNode->mergeNode);
	Free_AST_DeleteNode(queryExpressionNode->deleteNode);
	Free_AST_SetNode(queryExpressionNode->setNode);
	Free_AST_WhereNode(queryExpressionNode->whereNode);
	Free_AST_ReturnNode(queryExpressionNode->returnNode);
	Free_AST_OrderNode(queryExpressionNode->orderNode);
	free(queryExpressionNode);
}

AST_Validation Validate_Aliases_In_Match_Clause(const Vector* aliasesToCheck,
										         const Vector* matchAliases,
										         char** undefined_alias) {
    /* Check that all the aliases that are in aliasesToCheck Vector exists in the match clause */

	for (int i = 0; i < Vector_Size(aliasesToCheck); i++) {
        char *alias_to_check = NULL;
        Vector_Get(aliasesToCheck, i, &alias_to_check);

		int exists = 0;
		for (int j = 0; j < Vector_Size(matchAliases); j++) {
			AST_GraphEntity *match_entity = NULL;
			Vector_Get(matchAliases, j, &match_entity);
			if (strcmp(alias_to_check, match_entity->alias) == 0) {
				exists = 1;
				break;
			}
		}

		if (!exists) {
		    *undefined_alias = (char*)malloc(sizeof(char) *
				(strlen(" not defined" ) + strlen(alias_to_check) + 1));
			sprintf(*undefined_alias, "%s not defined", alias_to_check);
			return AST_INVALID;
		}
    }

	return AST_VALID;
}

AST_Validation _Validate_CREATE_Clause(const AST_Query* ast, char **reason) {
	return AST_VALID;
}

AST_Validation _Validate_DELETE_Clause(const AST_Query* ast, char **reason) {
	char* undefined_alias;

	if (!ast->deleteNode) {
		return AST_VALID;
	}

	if (!ast->matchNode) {
		return AST_INVALID;
	}

	if (Validate_Aliases_In_Match_Clause(ast->deleteNode->graphEntities,
			ast->matchNode->graphEntities, &undefined_alias) != AST_VALID) {
		*reason = undefined_alias;
		return AST_INVALID;
	}

	return AST_VALID;
}

AST_Validation _Validate_MATCH_Clause(const AST_Query* ast, char **reason) {
	return AST_VALID;
}

AST_Validation _Validate_RETURN_Clause(const AST_Query* ast, char **reason) {
	char* undefined_alias;

	if (!ast->returnNode) {
		return AST_VALID;
	}

	if (!ast->matchNode) {
		return AST_INVALID;
	}

	Vector *return_aliases = NewVector(char*, Vector_Size(ast->returnNode->returnElements));
	for (int i=0; i < Vector_Size(ast->returnNode->returnElements); i++) {
		AST_ReturnElementNode *return_entity = NULL;
        Vector_Get(ast->returnNode->returnElements, i, &return_entity);
		_Get_All_Aliases_From_Expression(return_entity->exp, return_aliases);
	}

	AST_Validation res = Validate_Aliases_In_Match_Clause(
		return_aliases, ast->matchNode->graphEntities, &undefined_alias);
	Vector_Free(return_aliases);
	
	if (res != AST_VALID) {
		*reason = undefined_alias;
		return AST_INVALID;
	}

	return AST_VALID;
}

AST_Validation _Validate_SET_Clause(const AST_Query* ast, char **reason) {
	char* undefined_alias;

	if (!ast->setNode) {
		return AST_VALID;
	}

	if (!ast->matchNode) {
		return AST_INVALID;
	}

    Vector *update_aliases = NewVector(char*, Vector_Size(ast->setNode->set_elements));
	for (int i=0; i < Vector_Size(ast->setNode->set_elements); i++) {
		AST_SetElement *update_entity = NULL;
        Vector_Get(ast->setNode->set_elements, i, &update_entity);
		Vector_Push(update_aliases, update_entity->entity->alias);
	}

	AST_Validation res = Validate_Aliases_In_Match_Clause(
		update_aliases, ast->matchNode->graphEntities, &undefined_alias);
	Vector_Free(update_aliases);

	if (res != AST_VALID) {
		*reason = undefined_alias;
		return AST_INVALID;
	}

	return AST_VALID;
}

AST_Validation _Validate_WHERE_Clause(const AST_Query* ast, char **reason) {
	return AST_VALID;
}

AST_Validation Validate_AST(const AST_Query* ast, char **reason) {
	/* AST must include either a MATCH or CREATE clause. */
	if(!(ast->matchNode || ast->createNode || ast->mergeNode)) {
		*reason = "Query must specify either MATCH or CREATE clause.";
		return AST_INVALID;
	}

	if(_Validate_MATCH_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	if(_Validate_WHERE_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	if(_Validate_CREATE_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	if(_Validate_SET_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	if(_Validate_DELETE_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	if(_Validate_RETURN_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	return AST_VALID;
}
