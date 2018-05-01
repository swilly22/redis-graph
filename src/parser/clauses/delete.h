#ifndef _CLAUSE_DELETE_H
#define _CLAUSE_DELETE_H

#include "../../rmutil/vector.h"

typedef struct {	
	Vector *graphEntities; /* Vector of char pointers. */
} AST_DeleteNode;

AST_DeleteNode* New_AST_DeleteNode(Vector *elements);
void Free_AST_DeleteNode(AST_DeleteNode *deleteNode);

#endif