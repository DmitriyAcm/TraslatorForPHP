%{
	#include "tree_structs.h"
	#include <stdio.h>
	#include <malloc.h>
	void yyerror(char const *s);
	extern int yylex(void);
	struct Program *createProgram(struct StatementList *list);
	struct StatementList *appendStatementToList(struct StatementList *list, struct Statement *stmt);
	struct StatementList *createStatementList(struct Statement *stmt);
	struct Statement *createExpressionStatement(struct Expression *expr);
	struct Statement *createIfStatement(struct IfStatement *ifStmt);
	struct Statement *createWhileStatement(struct WhileStatement *whileStmt);
	struct Statement *createForStatement(struct ForStatement *forStmt);
	struct Statement *createForeachStatement(struct ForeachStatement *foreachStmt);
	
	struct Expression *createExpression(enum ExpressionType type, struct Expression *left, struct Expression *right);
	struct Expression *createSimpleExpression(enum ExpressionType type, int intValue, float floatValue, char* stringValue);
	
	struct WhileStatement *createWhile(enum WhileType type, struct Expression *condition, struct Statement *whileBlock);
	struct ForStatement *createFor(struct Expression *initializer, struct Expression *control, struct Expression *endOfLoop, struct Statement *forBlock);
	struct ForeachStatement *createForeach(struct Expression *collection, struct Expression *foreachKey, struct Expression *foreachValue, struct Statement *foreachBlock);
%}

%union {
	int int_const;
	float float_const;
	char* string_const;
	char* id_const;

	struct Expression *expr;
	struct Statement *stmt;
	struct Program *prog;
	struct StatementList *list;
	struct IfStatement *ifStmt;
	struct WhileStatement *whileStmt;
	struct ForStatement *forStmt;
	struct ForeachStatement *foreachStmt;
}

%type <expr> expression
%type <stmt> statement
%type <prog> program
%type <list> statement_list
%type <whileStmt> while_statement
%type <forStmt> for_statement
%type <foreachStmt> foreach_statement

%token <int_const> INT
%token <int_const> BOOL
%token <float_const> FLOAT
%token <string_const> STRING
%token <id_const> ID
%token NIL
%token CONST
%token CLASS
%token EXTENDS
%token PUBLIC
%token PROTECTED
%token PRIVATE
%token NEW
%token AS
%token BREAK
%token CONTINUE
%token DO
%token WHILE
%token ENDWHILE
%token FOR
%token ENDFOR
%token FOREACH
%token ENDFOREACH
%token ECHO
%token IF
%token ELSEIF
%token ELSE
%token ENDIF
%token SWITCH
%token CASE
%token DEFAULT
%token ENDSWITCH
%token FUNCTION
%token INSTANCEOF
%token RETURN
%token STATIC
%token VAR
%token INCREMENT
%token DECREMENT
%token KEY_ACCESS
%token PROPERTY_ACCESS
%token STATIC_PROPERTY_ACCESS
%token START_TAG
%token END_TAG

%left LOGIC_OR_2
%left LOGIC_XOR
%left LOGIC_AND_2
%right '=' PLUS_ASSIGN MINUS_ASSIGN MULT_ASSIGN POW_ASSIGN DIV_ASSIGN CONCAT_ASSIGN MOD_ASSIGN LEFT_SHIFT_ASSIGN RIGHT_SHIFT_ASSIGN
%left LOGIC_OR_1
%left LOGIC_AND_1
%nonassoc EQUAL NOT_EQUAL_1 IDENTICALLY_EQUAL IDENTICALLY_NOT_EQUAL NOT_EQUAL_2
%nonassoc '<' LESSER_EQUAL '>' GREATER_EQUAL
%left LEFT_SHIFT RIGHT_SHIFT
%left '+' '-' '.'
%left '*' '/' '%'
%right '!'
%nonassoc INSTANCEOF
%left UMINUS
%left UPLUS
%right POW

%%

program : START_TAG statement_list END_TAG { $$ = createProgram($2); }
		| START_TAG END_TAG				   { $$ = createProgram(NULL); }
		;
		

expression : expression LOGIC_OR_2 expression				{ $$ = createExpression(ET_LOGIC_OR_2, $1, $3); }
		   | expression LOGIC_XOR expression				{ $$ = createExpression(ET_LOGIC_XOR, $1, $3); }
		   | expression LOGIC_AND_2 expression				{ $$ = createExpression(ET_LOGIC_AND_2, $1, $3); }
		   | expression POW_ASSIGN expression				{ $$ = createExpression(ET_POW_ASSIGN, $1, $3); }
		   | expression MULT_ASSIGN expression				{ $$ = createExpression(ET_MULT_ASSIGN, $1, $3); }
		   | expression DIV_ASSIGN expression				{ $$ = createExpression(ET_DIV_ASSIGN, $1, $3); }
		   | expression MOD_ASSIGN expression				{ $$ = createExpression(ET_MOD_ASSIGN, $1, $3); }
		   | expression PLUS_ASSIGN expression				{ $$ = createExpression(ET_PLUS_ASSIGN, $1, $3); }
		   | expression MINUS_ASSIGN expression				{ $$ = createExpression(ET_MINUS_ASSIGN, $1, $3); }
		   | expression CONCAT_ASSIGN expression			{ $$ = createExpression(ET_CONCAT_ASSIGN, $1, $3); }
		   | expression LEFT_SHIFT_ASSIGN expression		{ $$ = createExpression(ET_LEFT_SHIFT_ASSIGN, $1, $3); }
		   | expression RIGHT_SHIFT_ASSIGN expression		{ $$ = createExpression(ET_RIGHT_SHIFT_ASSIGN, $1, $3); }
		   | expression '=' expression						{ $$ = createExpression(ET_ASSIGN, $1, $3); }
		   | expression LOGIC_OR_1 expression				{ $$ = createExpression(ET_LOGIC_OR_1, $1, $3); }
		   | expression LOGIC_AND_1 expression				{ $$ = createExpression(ET_LOGIC_AND_1, $1, $3); }
		   | expression EQUAL expression					{ $$ = createExpression(ET_EQUAL, $1, $3); }
		   | expression NOT_EQUAL_1 expression				{ $$ = createExpression(ET_NOT_EQUAL_1, $1, $3); }
		   | expression NOT_EQUAL_2 expression				{ $$ = createExpression(ET_NOT_EQUAL_2, $1, $3); }
		   | expression IDENTICALLY_EQUAL expression		{ $$ = createExpression(ET_IDENTICALLY_EQUAL, $1, $3); }
		   | expression IDENTICALLY_NOT_EQUAL expression	{ $$ = createExpression(ET_IDENTICALLY_NOT_EQUAL, $1, $3); }
		   | expression '<' expression						{ $$ = createExpression(ET_LESSER, $1, $3); }
		   | expression '>' expression						{ $$ = createExpression(ET_GREATER, $1, $3); }
		   | expression LESSER_EQUAL expression				{ $$ = createExpression(ET_LESSER_EQUAL, $1, $3); }
		   | expression GREATER_EQUAL expression			{ $$ = createExpression(ET_GREATER_EQUAL, $1, $3); }
		   | expression LEFT_SHIFT expression				{ $$ = createExpression(ET_LEFT_SHIFT, $1, $3); }
		   | expression RIGHT_SHIFT expression				{ $$ = createExpression(ET_RIGHT_SHIFT, $1, $3); }
		   | expression '+' expression						{ $$ = createExpression(ET_PLUS, $1, $3); }
		   | expression '-' expression						{ $$ = createExpression(ET_MINUS, $1, $3); }
		   | expression '.' expression						{ $$ = createExpression(ET_CONCAT, $1, $3); }
		   | expression '*' expression						{ $$ = createExpression(ET_MULT, $1, $3); }
		   | expression '/' expression						{ $$ = createExpression(ET_DIV, $1, $3); }
		   | expression '%' expression						{ $$ = createExpression(ET_MOD, $1, $3); }
		   | '!' expression									{ $$ = createExpression(ET_NOT, NULL, $2); }
		   | '+' expression %prec UPLUS						{ $$ = createExpression(ET_PLUS, NULL, $2); }
		   | '-' expression %prec UMINUS					{ $$ = createExpression(ET_MINUS, NULL, $2); }
		   | expression INSTANCEOF ID						{ $$ = createExpression(ET_INSTANCEOF, $1, createSimpleExpression(ET_ID, -1, -1, id_const)); }
		   | expression POW expression						{ $$ = createExpression(ET_POW, $1, $3); }
		   | expression INCREMENT							{ $$ = createExpression(ET_INCREMENT, $1, NULL); }
		   | expression DECREMENT							{ $$ = createExpression(ET_DECREMENT, $1, NULL); }
		   | INCREMENT expression							{ $$ = createExpression(ET_INCREMENT, NULL, $2); }
		   | DECREMENT expression							{ $$ = createExpression(ET_DECREMENT, NULL, $2); }
		   | '$' expression									{ $$ = createExpression(ET_DOLLAR, NULL, $2); }
		   | '$' '{' expression '}'							{ $$ = createExpression(ET_DOLLAR, NULL, $3); }
		   | '(' expression ')'								{ $$ = $2; }
		   | ID												{ $$ = createSimpleExpression(ET_ID, -1, -1, id_const)); }
		   | INT											{ $$ = createSimpleExpression(ET_INT, int_const, -1, NULL)); }
		   | FLOAT											{ $$ = createSimpleExpression(ET_FLOAT, -1, float_const, NULL)); }
		   | STRING											{ $$ = createSimpleExpression(ET_STRING, -1, -1, string_const)); }
		   ;
				   

						  
		
statement : '{' statement_list '}'	{ $$ = $2; }
		  | '{' '}'					{ $$ = NULL; }
		  | expression ';'			{ $$ = createExpressionStatement($1); }
		  | ';'						{ $$ = createExpressionStatement(NULL); }
		  | while_statement			{ $$ = createWhileStatement($1); }
		  | for_statement			{ $$ = createForStatement($1); }
		  | foreach_statement		{ $$ = createForeachStatement($1); }
		  ;

statement_list : statement					{ $$ = createStatementList($1); }
			   | statement_list statement	{ $$ = appendStatementToList($1, $2); }
			   ;
				 
			   
while_statement : WHILE '(' expression ')' statement						{ $$ = createWhile(WT_WHILE,$3,$5); }
				| DO statement WHILE '(' expression ')' ';'					{ $$ = createWhile(WT_DO_WHILE,$5,$2); }
				;
			 
for_statement : FOR '(' expression ';' expression ';' expression ')' statement									{ $$ = createFor($3,$5,$7,$9); }
			  | FOR '(' ';' expression ';' expression ')' statement												{ $$ = createFor(NULL,$4,$6,$8); }
			  | FOR '(' expression ';' ';' expression ')' statement												{ $$ = createFor($3,NULL,$6,$8); }
			  | FOR '(' expression ';' expression ';' ')' statement												{ $$ = createFor($3,$5,NULL,$8); }
			  | FOR '(' ';' ';' expression ')' statement														{ $$ = createFor(NULL,NULL,$5,$7); }
			  | FOR '(' expression ';' ';' ')' statement														{ $$ = createFor($3,NULL,NULL,$7); }
			  | FOR '(' ';' expression ';' ')' statement														{ $$ = createFor(NULL,$4,NULL,$7); }
			  | FOR '(' ';' ';' ')' statement																	{ $$ = createFor(NULL,NULL,NULL,$6); }
			  ;
			 
			   
foreach_statement : FOREACH '(' expression AS expression ')' statement													{ $$ = createForeach($3,NULL,$5,$7); }
				  | FOREACH '(' expression AS expression KEY_ACCESS expression ')' statement							{ $$ = createForeach($3,$5,$7,$9); }
				  ;





				
				
				
				
				
%%

void yyerror(char const *s)
{
	printf("%s",s);
}				
				
struct Program *createProgram(struct StatementList *list)
{
	struct Program *result = (struct Program *)malloc(sizeof(struct Program));
	result->stmtList=list;
	return result;
}

struct StatementList *appendStatementToList(struct StatementList *list, struct Statement *stmt)
{
	StatementList* result = list;
	while (result->next != NULL)
	{
		result = result->next;
	}
	result->next = createStatementList(stmt);
	return list;
}
struct StatementList *createStatementList(struct Statement *stmt)
{
	struct StatementList *result = (struct StatementList *)malloc(sizeof(struct StatementList));
	result->stmt = stmt;
	return result;
}

struct Statement *createExpressionStatement(struct Expression *expr)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	enum StatementType type = ST_EXPRESSION;
	result->type = type;
	result->expr = expr;
	return result;
}

struct Statement *createIfStatement(struct IfStatement *ifStmt)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	enum StatementType type = ST_IF;
	result->type = type;
	result->ifStmt = ifStmt;
	return result;
}

struct Statement *createWhileStatement(struct WhileStatement *whileStmt)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	enum StatementType type = ST_WHILE;
	result->type = type;
	result->whileStmt = whileStmt;
	return result;
}

struct Statement *createForStatement(struct ForStatement *forStmt)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	enum StatementType type = ST_FOR;
	result->type = type;
	result->forStmt = forStmt;
	return result;
}

struct Statement *createForeachStatement(struct ForeachStatement *foreachStmt)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	enum StatementType type = ST_FOREACH;
	result->type = type;
	result->foreachStmt = foreachStmt;
	return result;
}

struct WhileStatement *createWhile(enum WhileType type, struct Expression *condition, struct Statement *whileBlock)
{
	struct WhileStatement *result = (struct WhileStatement *)malloc(sizeof(struct WhileStatement));
	result->type = type;
	result->condition = condition;
	result->whileBlock = whileBlock;
	return result;
}

struct ForStatement *createFor(struct Expression *initializer, struct Expression *control, struct Expression *endOfLoop, struct Statement *forBlock)
{
	struct ForStatement *result = (struct ForStatement *)malloc(sizeof(struct ForStatement));
	result->initializer = initializer;
	result->control = control;
	result->endOfLoop = endOfLoop;
	result->forBlock = forBlock;
	return result;
}

struct ForeachStatement *createForeach(struct Expression *collection, struct Expression *foreachKey, struct Expression *foreachValue, struct Statement *foreachBlock)
{
	struct ForeachStatement *result = (struct ForeachStatement *)malloc(sizeof(struct ForeachStatement));
	result->collection = collection;
	result->foreachKey = foreachKey;
	result->foreachValue = foreachValue;
	result->foreachBlock = foreachBlock;
	return result;
}


struct Expression *createExpression(enum ExpressionType type, struct Expression *left, struct Expression *right)
{
	struct Expression *result = (struct Expression *)malloc(sizeof(struct Expression));
	result->type = type;
	result->left = left;
	result->right = right;
	return result;
}

struct Expression *createSimpleExpression(enum ExpressionType type, int intValue, float floatValue, char* stringValue)
{
	struct Expression *result = (struct Expression *)malloc(sizeof(struct Expression));
	result->type = type;
	
	switch (type)
	{
		case ET_INT:
			result->intValue = intValue;
			break;
		case ET_FLOAT:
			result->floatValue = floatValue;
			break;
		case ET_STRING:
		case ET_ID:
			result->stringValue = stringValue;
			break;
		default:
			break;
	}
	
	return result;
}







