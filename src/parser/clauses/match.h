#ifndef _CLAUSE_MATCH_H
#define _CLAUSE_MATCH_H

#include "../../rmutil/vector.h"

typedef struct {
	Vector *graphEntities;
} AST_MatchNode;

AST_MatchNode* New_AST_MatchNode(Vector *elements);
void Free_AST_MatchNode(AST_MatchNode *matchNode);

#endif