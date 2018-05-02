#ifndef _CLAUSE_CREATE_H
#define _CLAUSE_CREATE_H

#include "../../rmutil/vector.h"

typedef struct {	
	Vector *graphEntities; /* Vector of Vectors of AST_GraphEntity pointers. */
} AST_CreateNode;

AST_CreateNode* New_AST_CreateNode(Vector *elements);
void Free_AST_CreateNode(AST_CreateNode *createNode);

#endif