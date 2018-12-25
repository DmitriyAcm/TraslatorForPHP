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
void switch2dowhile(struct SwitchStatement *switchStmt, struct Statement* parent);
void toLowerCase(char* string);
void checkSimpleExpression(struct Expression* expr);
void checkForeachElement(struct Expression* element);
void checkDefaultRepeat(struct SwitchStatement* switchStmt);
struct Expression* setFVar(enum ExpressionType type, int value);

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
	struct Statement* returnStmt = (struct Statement*)malloc(sizeof(struct Statement));
	struct Statement* stmt;
	toLowerCase(func->head->name);
	doSemanticStatement(func->body);
	for (stmt = func->body->compoundStmt->first; stmt != func->body->compoundStmt->last; stmt = stmt->next) {}
	returnStmt->type = ST_RETURN;
	returnStmt->expr = (struct Expression *)malloc(sizeof(struct Expression));
	returnStmt->expr->type = ET_NULL;
	stmt->next = returnStmt;
	stmt->next->next = NULL;
	func->body->compoundStmt->last = returnStmt;
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
				switch2dowhile(stmt->switchStmt, stmt);
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
	if (exprList != NULL) {
		for (expr = exprList->first; expr != NULL; expr = expr->next) {
			doSemanticExpression(expr);
		}
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
}

void switch2dowhile(struct SwitchStatement *switchStmt, struct Statement* parent) {
	struct StatementList* stmtList = (struct StatementList*)malloc(sizeof(struct StatementList));
	struct Expression* switchCondition = switchStmt->condition;
	struct WhileStatement* doWhile = (struct WhileStatement*)malloc(sizeof(struct WhileStatement));
	struct CaseStatement* _case;
	struct Statement* currentStmtDoWhile;

	doWhile->whileBlock = (struct Statement*)malloc(sizeof(struct Statement));
	doWhile->whileBlock->type = ST_COMPOUND;
	doWhile->whileBlock->compoundStmt = (struct StatementList*)malloc(sizeof(struct StatementList));
	doWhile->whileBlock->compoundStmt->first = (struct Statement*)malloc(sizeof(struct Statement));
	doWhile->condition = setFVar(ET_NOT_EQUAL_1, 1);

	stmtList->first = (struct Statement*)malloc(sizeof(struct Statement));
	stmtList->first->type = ST_EXPRESSION; 
	stmtList->first->expr = setFVar(ET_ASSIGN, 0);

	stmtList->first->next = (struct Statement*)malloc(sizeof(struct Statement));
	stmtList->first->next->type = ST_WHILE;
	stmtList->first->next->next = NULL;
	stmtList->last = stmtList->first->next;
	doWhile->type = WT_DO_WHILE;
	doWhile->altWhileBlock = NULL;
	
	currentStmtDoWhile = doWhile->whileBlock->compoundStmt->first;
	for (_case = switchStmt->caseStmts->first; _case != NULL; _case = _case->next) {
		struct Statement* f1 = (struct Statement*)malloc(sizeof(struct Statement));
		struct IfStatement* ifStmt = (struct IfStatement*)malloc(sizeof(struct IfStatement));
		struct Statement* firstStmt = (struct Statement*)malloc(sizeof(struct Statement));
		struct StatementList* stmtLoop = (struct StatementList*)malloc(sizeof(struct StatementList));

		ifStmt->condition = (struct Expression*)malloc(sizeof(struct Expression));
		ifStmt->ifBlock = (struct Statement*)malloc(sizeof(struct Statement));
		ifStmt->ifBlock->type = ST_COMPOUND;
		ifStmt->altIfBlock = NULL;
		ifStmt->elseIfBlock = NULL;
		ifStmt->elseBlock = NULL;
		ifStmt->altElseBlock = NULL;

		f1->type = ST_EXPRESSION; 
		f1->expr = setFVar(ET_ASSIGN, 1);

		if (_case->label != NULL) {
			ifStmt->condition->type = ET_LOGIC_OR_1;
			ifStmt->condition->left = (struct Expression*)malloc(sizeof(struct Expression));
			ifStmt->condition->left->type = ET_EQUAL;
			ifStmt->condition->left->left = switchCondition;
			ifStmt->condition->left->right = _case->label;
			ifStmt->condition->right = setFVar(ET_EQUAL, 1);
		} else {
			ifStmt->condition = setFVar(ET_GREATER_EQUAL, 1);
		}
		firstStmt->type = ST_EXPRESSION;
		firstStmt->expr = setFVar(ET_ASSIGN, 1);
		firstStmt->next = _case->stmtList->first;

		stmtLoop->first = firstStmt;
		stmtLoop->last = _case->stmtList->last;
		ifStmt->ifBlock->compoundStmt = stmtLoop;

		currentStmtDoWhile->type = ST_IF;
		currentStmtDoWhile->ifStmt = ifStmt;

		currentStmtDoWhile->next = (struct Statement*)malloc(sizeof(struct Statement));
		currentStmtDoWhile = currentStmtDoWhile->next;
	}
	//currentStmtDoWhile = (struct Statement*)malloc(sizeof(struct Statement));
	currentStmtDoWhile->type = ST_IF;
	currentStmtDoWhile->ifStmt = (struct IfStatement*)malloc(sizeof(struct IfStatement));
	currentStmtDoWhile->ifStmt->altIfBlock = NULL;
	currentStmtDoWhile->ifStmt->elseBlock = NULL;
	currentStmtDoWhile->ifStmt->elseIfBlock = NULL;
	currentStmtDoWhile->ifStmt->altElseBlock = NULL;
	currentStmtDoWhile->ifStmt->condition = setFVar(ET_EQUAL, 0);
	currentStmtDoWhile->ifStmt->ifBlock = (struct Statement*)malloc(sizeof(struct Statement));
	currentStmtDoWhile->ifStmt->ifBlock->type = ST_EXPRESSION;
	currentStmtDoWhile->ifStmt->ifBlock->expr = setFVar(ET_ASSIGN, 2);
	currentStmtDoWhile->next = NULL;
	doWhile->whileBlock->compoundStmt->last = currentStmtDoWhile;

	stmtList->first->next->whileStmt = doWhile;
	parent->compoundStmt = stmtList;
	parent->type = ST_COMPOUND;
	free(parent->switchStmt);
	parent->switchStmt = NULL;
}

struct Expression* setFVar(enum ExpressionType type, int value) {
	struct Expression* expr = (struct Expression*)malloc(sizeof(struct Expression));
	expr->type = type;
	expr->left = (struct Expression*)malloc(sizeof(struct Expression));
	expr->left->type = ET_DOLLAR;
	expr->left->right = (struct Expression*)malloc(sizeof(struct Expression));
	expr->left->right->type = ET_ID;
	expr->left->right->stringValue = "___F___";
	expr->right = (struct Expression*)malloc(sizeof(struct Expression));
	expr->right->type = ET_INT;
	expr->right->intValue = value;
	return expr;
}