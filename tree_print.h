#include <stdio.h>
#include <string.h>
#include "tree_structs.h"

extern struct Program * root;

FILE * file;
int nodeNumber = 0;


void printNodeTree(int nodeId, char* nodeName);
void printRelation(int from, int to, char* label);
int printSimpleNode(int nodeId, char* value, char* label);

void printProgram(void);
void printProgramList(struct ProgramList *progList, int nodeId);
void printStatement(struct Statement *stmt, int nodeId, char* label);
void printStatementList(struct StatementList *stmtList, int nodeId, char* label);
void printClass(struct ClassDeclaration *classDecl, int nodeId, char* label);
void printClassMemberList(struct ClassMemberList *classMemberList, int nodeId, char* label);
void printClassMember(struct ClassMember *elem, int nodeId, char* label);
void printFunction(struct FunctionDefinition *funcDef, int nodeId, char* label);
void printFunctionHeader(struct FunctionHeader *head, int nodeId, char* label);
void printVariablesList(struct VariablesList *list, int nodeId, char* label);
void printVariableElement(struct VariableElement *elem, int nodeId, char* label);

void printExpression(struct Expression *expr, int nodeId, char* label);
void printExpressionList(struct ExpressionList* list, int nodeId, char* label);

void printIf(struct IfStatement *ifStmt, int nodeId, char* label);
void printElseIfList(struct ElseIfStatementList *elseIfBlock, int isAlt, int nodeId, char* label);
void printElseIf(struct ElseIfStatement *elseIfBlock, int isAlt, int nodeId, char* label);
void printWhile(struct WhileStatement *whileStmt, int nodeId, char* label);
void printFor(struct ForStatement *forStmt, int nodeId, char* label);
void printForeach(struct ForeachStatement *foreachStmt, int nodeId, char* label);
void printSwitch(struct SwitchStatement *switchStmt, int nodeId, char* label);
void printCaseStatements(struct CaseStatements *caseStmts, int nodeId, char* label);

void printNodeTree(int nodeId, char* nodeName)
{
	char num[1000];
	_itoa(nodeId, num, 10);
	fputs("node", file);
	fputs(num, file);
	fputs(" [label=\"", file);
	fputs(nodeName, file);
	fputs("\"];\n", file);
}

void printRelation(int from, int to, char* label)
{
	char num[1000];
	_itoa(from, num, 10);
	fputs("node", file);
	fputs(num, file);
	fputs("->node", file);
	_itoa(to, num, 10);
	fputs(num, file);
	fputs(" [label=\"", file);
	fputs(label, file);
	fputs("\"];\n", file);
}

int printSimpleNode(int nodeId, char* value, char* label)
{
	int node = nodeNumber++;
	printRelation(nodeId, node, label);
	printNodeTree(node, value);
	return node;
}

void printProgram(char* filename)
{
	int node = nodeNumber++;
	if (root == NULL)
	{
		printf("Empty tree!\n");
		return;
	}
	
	file = fopen(filename, "w");
	fputs("digraph G {\n", file);
	
	printNodeTree(node, "Program");
	
	if (root->progList != NULL)
	{
		printProgramList(root->progList, node);
	}
	
	fputs("}", file);
}

void printProgramList(struct ProgramList *progList, int nodeId)
{
	int node = nodeNumber++;
	if (progList == NULL)
	{
		printf("Empty program list!\n");
		return;
	}
	
	printNodeTree(node, "Program List");
	printRelation(nodeId, node, "");

	if (progList->first != NULL)
	{
		int current = 1;
		char buf[100];
		struct ProgramElement *elem = progList->first;
		for (; ; elem = elem->next) 
		{
			_itoa(current, buf, 10);
			switch (elem->type)
			{
				case PET_STATEMENT:
					printStatement(elem->stmt, node, buf);
					break;
				case PET_CLASS:
					printClass(elem->classDecl, node, buf);
					break;
				case PET_FUNCTION:
					printFunction(elem->funcDef, node, buf);
					break;
			}
			if (elem == progList->last)
				break;
			current++;
		} 
	}
}

void printStatement(struct Statement *stmt, int nodeId, char* label)
{
	if (stmt == NULL)
	{
		printf("Empty statement!\n");
		return;
	}

	switch (stmt->type)
	{
		case ST_EXPRESSION:
			printExpression(stmt->expr, nodeId, label);
			break;
		case ST_IF:
			printIf(stmt->ifStmt, nodeId, label);
			break;
		case ST_SWITCH:
			printSwitch(stmt->switchStmt, nodeId, label);
			break;
		case ST_WHILE:
			printWhile(stmt->whileStmt, nodeId, label);
			break;
		case ST_FOR:
			printFor(stmt->forStmt, nodeId, label);
			break;
		case ST_FOREACH:
			printForeach(stmt->foreachStmt, nodeId, label);
			break;
		case ST_COMPOUND:
			printStatementList(stmt->compoundStmt, nodeId, label);
			break;
		case ST_ECHO: 
			{
				int nodeEcho = printSimpleNode(nodeId, "Echo", label);
				printExpressionList(stmt->echoExprList, nodeEcho, "");
				break;
			}
		case ST_RETURN:
			{
				int nodeReturn = printSimpleNode(nodeId, "Return", label);
				if (stmt->expr != NULL)
					printExpression(stmt->expr, nodeReturn, "");
				break;
			}
		case ST_BREAK:
			{
				printSimpleNode(nodeId, "Break", label);
				break;
			}
		case ST_CONTINUE:
			{
				printSimpleNode(nodeId, "Continue", label);
				break;
			}

	}
}

void printStatementList(struct StatementList *stmtList, int nodeId, char* label)
{
	int node = nodeNumber++;
	
	printNodeTree(node, "Statement List");
	printRelation(nodeId, node, label);

	if (stmtList != NULL && stmtList->first != NULL)
	{
		int current = 1;
		char buf[100];
		struct Statement* elem = stmtList->first;
		for (; ; elem = elem->next)
		{
			_itoa(current, buf, 10);
			printStatement(elem, node, buf);
			if (elem == stmtList->last)
				break;

			current++;
		}
	}
}

void printClass(struct ClassDeclaration *classDecl, int nodeId, char* label)
{
	int node = nodeNumber++;
	if (classDecl == NULL)
	{
		printf("Empty Class Declaration!\n");
		return;
	}
	
	printNodeTree(node, "Class Declaration");
	printRelation(nodeId, node, label);

	printSimpleNode(node, classDecl->className, "name");
	if (classDecl->classNameExtended != NULL)
		printSimpleNode(node, classDecl->classNameExtended, "extended name");

	printClassMemberList(classDecl->classMemberList, node, "members");
}

void printClassMemberList(struct ClassMemberList *classMemberList, int nodeId, char* label)
{
	int node = nodeNumber++;
	printNodeTree(node, "Class Member List");
	printRelation(nodeId, node, label);

	if (classMemberList == NULL)
	{
		printf("Empty Class Member List!\n");
		return;
	}

	if (classMemberList->first != NULL)
	{
		int current = 1;
		char buf[100];
		struct ClassMember* elem = classMemberList->first;
		for (; ; elem = elem->next)
		{
			_itoa(current, buf, 10);
			printClassMember(elem, node, buf);
			if (elem == classMemberList->last)
				break;

			current++;
		}
	}
}

void printClassMember(struct ClassMember *elem, int nodeId, char* label)
{
	int node = nodeNumber++;
	int nodeAccess;
	char memberData[100];

	if (elem == NULL)
	{
		printf("Empty Class Member!\n");
		return;
	}

	switch (elem->aType)
	{
		case AT_PRIVATE:
			strcpy(memberData, "private");
			break;
		case AT_PROTECTED:
			strcpy(memberData, "protected");
			break;
		case AT_PUBLIC:
			strcpy(memberData, "public");
			break;
	}

	if (elem->isStatic == 1)
		strcat(memberData, " static");

	nodeAccess = printSimpleNode(node, memberData, "");

	switch (elem->cmType)
	{
		case CMT_CONST:
			printNodeTree(node, "Class Const");
			printRelation(nodeId, node, label);
			printVariablesList(elem->constList, nodeAccess, "");
			break;
		case CMT_PROPERTY:
			printNodeTree(node, "Class Property");
			printRelation(nodeId, node, label);
			printVariablesList(elem->propList, nodeAccess, "");
			break;
		case CMT_METHOD:
			printNodeTree(node, "Class Method");
			printRelation(nodeId, node, label);
			if (elem->funcDef != NULL)
				printFunction(elem->funcDef, nodeAccess, "");
			break;
	}
}

void printFunction(struct FunctionDefinition *funcDef, int nodeId, char* label)
{
	int node = nodeNumber++;
	if (funcDef == NULL)
	{
		printf("Empty Function Definition!\n");
		return;
	}
	
	printNodeTree(node, "Function Definition");
	printRelation(nodeId, node, label);

	printFunctionHeader(funcDef->head, node, "header");
	printStatement(funcDef->body, node, "body");
}

void printFunctionHeader(struct FunctionHeader *head, int nodeId, char* label)
{
	int node = nodeNumber++;
	if (head == NULL)
	{
		printf("Empty Function Header!\n");
		return;
	}
	
	printNodeTree(node, "Function Header");
	printRelation(nodeId, node, label);

	printSimpleNode(node, head->name, "name");
	printVariablesList(head->parameters, node, "arguments");
	if (head->type != NULL) 
		printSimpleNode(node, head->type, "return type");
}

void printVariablesList(struct VariablesList *list, int nodeId, char* label)
{
	int node = nodeNumber++;
	
	printNodeTree(node, "Variable List");
	printRelation(nodeId, node, label);

	if (list != NULL && list->first != NULL)
	{
		int current = 1;
		char buf[100];
		struct VariableElement* elem = list->first;
		for (; ; elem = elem->next)
		{
			_itoa(current, buf, 10);
			printVariableElement(elem, node, buf);
			if (elem == list->last)
				break;
			current++;
		}
	}
}

void printVariableElement(struct VariableElement *elem, int nodeId, char* label)
{
	int node = nodeNumber++;
	if (elem == NULL)
	{
		printf("Empty Variable!\n");
		return;
	}
	
	printNodeTree(node, "Variable");
	printRelation(nodeId, node, label);

	if (elem->type != NULL)
		printSimpleNode(node, elem->type, "type");

	printSimpleNode(node, elem->name, "name");
	
	if (elem->value != NULL)
		printExpression(elem->value, node, "value");
}

void printExpression(struct Expression *expr, int nodeId, char* label)
{
	int node = nodeNumber++;
	if (expr == NULL)
	{
		printf("Empty expression!\n");
		return;
	}
	
	printRelation(nodeId, node, label);

	switch (expr->type)
	{
		char buf[1000];
		case ET_INT:
			_itoa(expr->intValue, buf, 10);
			printNodeTree(node, buf);
			break;
		case ET_FLOAT:
			sprintf(buf, "%f", expr->floatValue);
			printNodeTree(node, buf);
			break;
		case ET_STRING:
			strcpy(buf, "\'");
			strcat(buf, expr->stringValue);
			strcat(buf, "\'");
			printNodeTree(node, buf);
			break;
		case ET_BOOL:
			printNodeTree(node, "BOOL");
			expr->intValue == 1 ? strcpy(buf, "true") : strcpy(buf, "false");
			printNodeTree(node, buf);
			break;
		case ET_NULL:
			printNodeTree(node, "NULL");
			break;
		case ET_ID:
			printNodeTree(node, "ID");
			printSimpleNode(node, expr->stringValue, "");
			break;
		case ET_POW:	
			printNodeTree(node, "**");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_INSTANCEOF:
			printNodeTree(node, "instanceof");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_DOLLAR:
			printNodeTree(node, "$");
			printExpression(expr->right, node, "right");
			break;
		case ET_ARRAY:
			printNodeTree(node, "Array Element");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_NOT:
			printNodeTree(node, "!");
			printExpression(expr->right, node, "right");
			break;
		case ET_PLUS:
			printNodeTree(node, "+");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_MINUS:
			printNodeTree(node, "-");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_MULT:
			printNodeTree(node, "*");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_DIV:
			printNodeTree(node, "/");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_CONCAT:
			printNodeTree(node, ".");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_MOD:
			printNodeTree(node, "%");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_LESSER:
			printNodeTree(node, "<");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_LESSER_EQUAL:
			printNodeTree(node, "<=");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_GREATER:
			printNodeTree(node, ">");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_GREATER_EQUAL:
			printNodeTree(node, ">=");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_EQUAL:
			printNodeTree(node, "==");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_NOT_EQUAL_1:
		case ET_NOT_EQUAL_2:
			printNodeTree(node, "!=");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_IDENTICALLY_EQUAL:
			printNodeTree(node, "===");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_IDENTICALLY_NOT_EQUAL:
			printNodeTree(node, "!==");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_LOGIC_AND_1:
			printNodeTree(node, "&&");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_LOGIC_OR_1:
			printNodeTree(node, "||");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_LOGIC_XOR:
			printNodeTree(node, "xor");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_LOGIC_AND_2:
			printNodeTree(node, "and");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_LOGIC_OR_2:
			printNodeTree(node, "or");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_ASSIGN:
			printNodeTree(node, "=");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_PROPERTY_ACCESS:
			printNodeTree(node, "->");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_STATIC_PROPERTY_ACCESS:
			printNodeTree(node, "::");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_KEY_ACCESS:
			printNodeTree(node, "=>");
			printExpression(expr->left, node, "left");
			printExpression(expr->right, node, "right");
			break;
		case ET_FUNCTION_CALL:
			printNodeTree(node, "Function Call");
			printExpression(expr->left, node, "left");
			printExpressionList(expr->rightExprList, node, "right");
			break;
		case ET_OBJECT_CREATION:
			printNodeTree(node, "Object Creation");
			printExpression(expr->left, node, "");
			break;
	}
}

void printExpressionList(struct ExpressionList* list, int nodeId, char* label)
{
	int node = nodeNumber++;
	
	printNodeTree(node, "Expression List");
	printRelation(nodeId, node, label);

	if (list != NULL && list->first != NULL)
	{
		int current = 1;
		char buf[100];
		struct Expression* elem = list->first;
		for (; ; elem = elem->next)
		{
			_itoa(current, buf, 10);
			printExpression(elem, node, buf);
			if (elem == list->last)
				break;

			current++;
		}
	}
}

void printIf(struct IfStatement *ifStmt, int nodeId, char* label)
{
	int node = nodeNumber++;
	int isAlt = 0;

	if (ifStmt == NULL)
	{
		printf("Empty If statement!\n");
		return;
	}

	printNodeTree(node, "If Statement");
	printRelation(nodeId, node, label);

	printExpression(ifStmt->condition, node, "condition");
	if (ifStmt->ifBlock != NULL)
	{
		printStatement(ifStmt->ifBlock, node, "if block");
	}
	else 
	{
		printStatementList(ifStmt->altIfBlock, node, "if block");
		isAlt = 1;
	}

	if (ifStmt->elseIfBlock != NULL)
		printElseIfList(ifStmt->elseIfBlock, isAlt, node, "else if block");

	if (isAlt == 0 && ifStmt->elseBlock != NULL)
		printStatement(ifStmt->elseBlock, node, "else block");
	else if (isAlt == 1 && ifStmt->altElseBlock != NULL)
		printStatementList(ifStmt->altElseBlock, node, "else block");
}

void printElseIfList(struct ElseIfStatementList *elseIfBlock, int isAlt, int nodeId, char* label)
{
	int node = nodeNumber++;

	if (elseIfBlock == NULL)
	{
		printf("Empty Else If statement!\n");
		return;
	}

	printNodeTree(node, "Else If Statement List");
	printRelation(nodeId, node, label);

	if (elseIfBlock->first != NULL)
	{
		int current = 1;
		char buf[100];
		struct ElseIfStatement *elem = elseIfBlock->first;
		for (; ; elem = elem->next)
		{
			_itoa(current, buf, 10);
			printElseIf(elem, isAlt, node, buf);
			if (elem == elseIfBlock->last)
				break;

			current++;
		}
	}
}

void printElseIf(struct ElseIfStatement *elseIfBlock, int isAlt, int nodeId, char* label)
{
	int node = nodeNumber++;

	if (elseIfBlock == NULL)
	{
		printf("Empty Else If statement!\n");
		return;
	}

	printNodeTree(node, "Else If Statement");
	printRelation(nodeId, node, label);

	printExpression(elseIfBlock->condition, node, "condition");
	if (isAlt == 0)
		printStatement(elseIfBlock->elseIfBlock, node, "else if block");
	else
		printStatementList(elseIfBlock->altElseIfBlock, node, "else if block");
}

void printWhile(struct WhileStatement *whileStmt, int nodeId, char* label)
{
	int node = nodeNumber++;

	if (whileStmt == NULL)
	{
		printf("Empty While statement!\n");
		return;
	}

	if (whileStmt->type == WT_WHILE)
		printNodeTree(node, "While Statement");
	else
		printNodeTree(node, "Do-While Statement");
	printRelation(nodeId, node, label);

	printExpression(whileStmt->condition, node, "condition");
	if (whileStmt->whileBlock != NULL)
		printStatement(whileStmt->whileBlock, node, "body");
	else
		printStatementList(whileStmt->altWhileBlock, node, "body");
}

void printFor(struct ForStatement *forStmt, int nodeId, char* label)
{
	int node = nodeNumber++;

	if (forStmt == NULL)
	{
		printf("Empty For statement!\n");
		return;
	}

	printNodeTree(node, "For Statement");
	printRelation(nodeId, node, label);

	printExpressionList(forStmt->initializer, node, "initializer");
	printExpressionList(forStmt->control, node, "control");
	printExpressionList(forStmt->endOfLoop, node, "endOfLoop");

	if (forStmt->forBlock != NULL)
		printStatement(forStmt->forBlock, node, "for block");
	else
		printStatementList(forStmt->altForBlock, node, "for block");
}

void printForeach(struct ForeachStatement *foreachStmt, int nodeId, char* label)
{
	int node = nodeNumber++;

	if (foreachStmt == NULL)
	{
		printf("Empty Foreach statement!\n");
		return;
	}

	printNodeTree(node, "Foreach Statement");
	printRelation(nodeId, node, label);

	printExpression(foreachStmt->collection, node, "collection");
	printExpression(foreachStmt->element, node, "element");

	if (foreachStmt->foreachBlock != NULL)
		printStatement(foreachStmt->foreachBlock, node, "foreach block");
	else
		printStatementList(foreachStmt->altForeachBlock, node, "foreach block");
}

void printSwitch(struct SwitchStatement *switchStmt, int nodeId, char* label)
{
	int node = nodeNumber++;

	if (switchStmt == NULL)
	{
		printf("Empty Switch statement!\n");
		return;
	}

	printNodeTree(node, "Switch Statement");
	printRelation(nodeId, node, label);

	printExpression(switchStmt->condition, node, "condition");
	printCaseStatements(switchStmt->caseStmts, node, "cases");
}

void printCaseStatements(struct CaseStatements *caseStmts, int nodeId, char* label)
{
	int node = nodeNumber++;

	printNodeTree(node, "Case Statements");
	printRelation(nodeId, node, label);

	if (caseStmts != NULL && caseStmts->first != NULL)
	{
		struct CaseStatement* elem = caseStmts->first;
		int current = 1;
		char buf[100];
		for (; ; elem = elem->next)
		{
			int nodeCase = nodeNumber++;
			_itoa(current, buf, 10);
			printNodeTree(nodeCase, "Case");
			printRelation(node, nodeCase, buf);

			if (elem->label != NULL)
				printExpression(elem->label, nodeCase, "label");
			printStatementList(elem->stmtList, nodeCase, "");

			current++;
			if (elem == caseStmts->last)
				break;
		}
	}
}