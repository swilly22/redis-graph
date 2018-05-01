#include "delete.h"

AST_DeleteNode* New_AST_DeleteNode(Vector *elements) {
	AST_DeleteNode *deleteNode = (AST_DeleteNode*)malloc(sizeof(AST_DeleteNode));
	deleteNode->graphEntities = elements;
	return deleteNode;
}

void Free_AST_DeleteNode(AST_DeleteNode *deleteNode) {
	if(!deleteNode)	return;
	Vector_Free(deleteNode->graphEntities);
	free(deleteNode);
}
