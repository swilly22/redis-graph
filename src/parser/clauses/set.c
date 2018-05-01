#include "./set.h"

AST_SetNode* New_AST_SetNode(Vector *elements) {
	AST_SetNode *set_node = (AST_SetNode*)malloc(sizeof(AST_SetNode));
	set_node->set_elements = elements;
	return set_node;
}

AST_SetElement* New_AST_SetElement(AST_Variable *updated_entity, AST_ArithmeticExpressionNode *exp) {
	AST_SetElement *set_element = malloc(sizeof(AST_SetElement));
	set_element->entity = updated_entity;
	set_element->exp = exp;
	return set_element;
}

void Free_AST_SetNode(AST_SetNode *setNode) {
    if(!setNode) return;

    AST_SetElement *elem = NULL;
    while(Vector_Pop(setNode->set_elements, elem)) {
        Free_AST_Variable(elem->entity);
        Free_AST_ArithmeticExpressionNode(elem->exp);
        free(elem);
    }

    Vector_Free(setNode->set_elements);
    free(setNode);
}
