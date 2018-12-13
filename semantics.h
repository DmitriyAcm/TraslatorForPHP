#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tree_structs.h"

extern struct Program * root;

void doSemantic(void);
void doSemanticFunction(struct FunctionDefinition* func);
void doSemanticStatement(struct Statement* stmt);
void doSemanticStatementList(struct StatementList* stmtList);
void doSemanticElseIf(struct ElseIfStatementList* elseIfList);
void doSemanticExpression(struct Expression* expr);
void doSemanticExpressionList(struct ExpressionList* exprList);
void switch2if(struct SwitchStatement *switchStmt, struct Statement* parent);
void toLowerCase(char* string);
void checkSimpleExpression(struct Expression* expr);
void checkForeachElement(struct Expression* element);
void checkDefaultRepeat(struct SwitchStatement* switchStmt);

void toLowerCase(char* string) {
	int i = 0;
	int size = strlen(string);
	char* result = (char*)malloc(size);
	while(string[i]) {
		result[i] = (char)tolower(string[i]);
		i++;
	}
	result[i] = '\0';
	strcpy(string, result);
}

void doSemantic(void) {
	if (root != NULL && root->progList != NULL) {
		struct ProgramElement* pe;
		for (pe = root->progList->first; pe != NULL; pe = pe->next) {
			switch (pe->type) {
				case PET_STATEMENT:
					doSemanticStatement(pe->stmt);
					break;
				case PET_FUNCTION:
					doSemanticFunction(pe->funcDef);
					break;
				case PET_CLASS:
					toLowerCase(pe->classDecl->className);
					break;
			}
		}
	}
}

void doSemanticFunction(struct FunctionDefinition* func) {
	toLowerCase(func->head->name);
	doSemanticStatement(func->body);
}

void checkDefaultRepeat(struct SwitchStatement* switchStmt) {
	struct CaseStatements* cases = switchStmt->caseStmts;
	struct CaseStatement* c;
	int isFindDefault = 0;
	for (c = cases->first; c != NULL; c = c->next) {
		if (c->label == NULL) {
			if (isFindDefault == 0) {
				isFindDefault = 1;
			} else {
				printf("parse error! incorrect repeat default statement in switch!\n");
				root = NULL;
				return;
			}
		}

	}
}

void doSemanticStatement(struct Statement* stmt) {
	if (stmt != NULL) {
		switch (stmt->type) {
			case ST_SWITCH:
				checkDefaultRepeat(stmt->switchStmt);
				//switch2if(stmt->switchStmt, stmt);
				break;
			case ST_FOR:
				doSemanticExpressionList(stmt->forStmt->initializer);
				doSemanticExpressionList(stmt->forStmt->control);
				doSemanticExpressionList(stmt->forStmt->endOfLoop);
				if (stmt->forStmt->altForBlock != NULL) {
					doSemanticStatementList(stmt->forStmt->altForBlock);
				} else {
					doSemanticStatement(stmt->forStmt->forBlock);
				}
				break;
			case ST_IF:
				doSemanticExpression(stmt->ifStmt->condition);
				if (stmt->ifStmt->altIfBlock != NULL) {
					doSemanticStatementList(stmt->ifStmt->altIfBlock);
				} else {
					doSemanticStatement(stmt->ifStmt->ifBlock);
				}
				if (stmt->ifStmt->elseIfBlock != NULL) {
					doSemanticElseIf(stmt->ifStmt->elseIfBlock);
				}
				if (stmt->ifStmt->altElseBlock != NULL) {
					doSemanticStatementList(stmt->ifStmt->altElseBlock);
				} else {
					doSemanticStatement(stmt->ifStmt->elseBlock);
				}
				break;
			case ST_WHILE:
				doSemanticExpression(stmt->whileStmt->condition);
				if (stmt->whileStmt->altWhileBlock != NULL) {
					doSemanticStatementList(stmt->whileStmt->altWhileBlock);
				} else {
					doSemanticStatement(stmt->whileStmt->whileBlock);
				}
				break;
			case ST_COMPOUND:
				doSemanticStatementList(stmt->compoundStmt);
				break;
			case ST_FOREACH:
				doSemanticExpression(stmt->foreachStmt->collection);
				checkForeachElement(stmt->foreachStmt->element);
				doSemanticExpression(stmt->foreachStmt->element);
				if (stmt->foreachStmt->altForeachBlock != NULL) {
					doSemanticStatementList(stmt->foreachStmt->altForeachBlock);
				} else {
					doSemanticStatement(stmt->foreachStmt->foreachBlock);
				}
				break;
			case ST_ECHO:
				doSemanticExpressionList(stmt->echoExprList);
				break;
			case ST_EXPRESSION:
			case ST_RETURN:
				doSemanticExpression(stmt->expr);
				break;
		}
	}
}

void checkForeachElement(struct Expression* element) {
	checkSimpleExpression(element);
	if (element->type != ET_DOLLAR) {
			printf("parse error! incorrect foreach element values!\n");
			root = NULL;
			return;
	}
}

void doSemanticStatementList(struct StatementList* stmtList) {
	if (stmtList != NULL) {
		struct Statement* stmt;
		for (stmt = stmtList->first; stmt != NULL; stmt = stmt->next) {
			doSemanticStatement(stmt);
		}
	}
}

void doSemanticElseIf(struct ElseIfStatementList* elseIfList) {
	struct ElseIfStatement* stmt;
	for (stmt = elseIfList->first; stmt != NULL; stmt = stmt->next) {
		doSemanticExpression(stmt->condition);
		if (stmt->altElseIfBlock != NULL) {
			doSemanticStatementList(stmt->altElseIfBlock);
		} else {
			doSemanticStatement(stmt->elseIfBlock);
		}
	}
}

void doSemanticExpressionList(struct ExpressionList* exprList) {
	struct Expression* expr;
	for (expr = exprList->first; expr != NULL; expr = expr->next) {
		doSemanticExpression(expr);
	}
}

void checkSimpleExpression(struct Expression* expr) {
	if (expr->type == ET_INT ||
		expr->type == ET_FLOAT ||
		expr->type == ET_STRING ||
		expr->type == ET_BOOL || 
		expr->type == ET_NULL) {
			printf("parse error! incorrect left operand <function call>!\n");
			root = NULL;
			return;
	}
}

void doSemanticExpression(struct Expression* expr) {
	if (expr != NULL) {
		switch (expr->type) {
			case ET_FUNCTION_CALL:
				checkSimpleExpression(expr->left);
				if (expr->left->type == ET_ID) {
					toLowerCase(expr->left->stringValue);
				}
				break;
			case ET_ARRAY:
				checkSimpleExpression(expr->left);
				break;
			case ET_PROPERTY_ACCESS:
			case ET_STATIC_PROPERTY_ACCESS:
				checkSimpleExpression(expr->left);
				checkSimpleExpression(expr->right);
				break;
		}
	}
	if (expr->left != NULL) {
		doSemanticExpression(expr->left);
	}
	if (expr->right != NULL) {
		doSemanticExpression(expr->right);
	}
	if (expr->rightExprList != NULL) {
		doSemanticExpressionList(expr->rightExprList);
	}
}

void switch2if(struct SwitchStatement *switchStmt, struct Statement* parent) {
	//ÑÄÅËÀÒÜ ÏÐÎÂÅÐÊÓ ÍÀ BREAK
	struct IfStatement* ifStmt = (struct IfStatement*)malloc(sizeof(struct IfStatement));
	struct CaseStatements* cases = switchStmt->caseStmts;
	struct CaseStatement* c = cases->first->next;
	struct Expression* ifCondition = (struct Expression*)malloc(sizeof(struct Expression));
	ifCondition->type = ET_EQUAL;
	ifCondition->left = switchStmt->condition;
	ifCondition->right = cases->first->label;

	ifStmt->condition = ifCondition;
	ifStmt->ifBlock = (struct Statement*)malloc(sizeof(struct Statement));
	ifStmt->ifBlock->type = ST_COMPOUND;
	ifStmt->ifBlock->compoundStmt = cases->first->stmtList;
	ifStmt->elseIfBlock = NULL;
	if (c != NULL && c->label != NULL) {
		struct ElseIfStatementList* elseifs = (struct ElseIfStatementList*)malloc(sizeof(struct ElseIfStatementList));
		int iter = 1;
		for (; c != NULL && c->label != NULL; c = c->next, iter++) {
			struct ElseIfStatement* elseIf = (struct ElseIfStatement*)malloc(sizeof(struct ElseIfStatement));
			struct Expression* elseIfCondition = (struct Expression*)malloc(sizeof(struct Expression));
			elseIfCondition->type = ET_EQUAL;
			elseIfCondition->left = switchStmt->condition;
			elseIfCondition->right = c->label;
			elseIf->condition = elseIfCondition;
			elseIf->elseIfBlock = (struct Statement*)malloc(sizeof(struct Statement));
			elseIf->elseIfBlock->type = ST_COMPOUND;
			elseIf->elseIfBlock->compoundStmt = c->stmtList;
			if (iter == 1) {
				elseifs->first = elseifs->last = elseIf;
			} else {
				elseifs->last->next = elseIf;
				elseifs->last->next->next = NULL;
				elseifs->last = elseIf;
			}
		}
		ifStmt->elseIfBlock = elseifs;
	}
	if (c != NULL && c->label == NULL) {
		ifStmt->elseBlock = (struct Statement*)malloc(sizeof(struct Statement));
		ifStmt->elseBlock->type = ST_COMPOUND;
		ifStmt->elseBlock->compoundStmt = c->stmtList;
	}
	parent->type = ST_IF;
	parent->ifStmt = ifStmt;
	free(parent->switchStmt);
	parent->switchStmt = NULL;
}