%{
	#include "tree_structs.h"
	#include <stdio.h>
	#include <malloc.h>
	void yyerror(char const *s);
	extern int yylex(void);
	struct Program *createProgram(struct ProgramList *progList);
	struct ProgramList *appendProgramToList(struct ProgramList *list, struct ProgramElement *elem);
	struct ProgramList *createProgramList(struct ProgramElement *elem);
	struct ProgramElement *createStatementElement(struct Statement *stmt);
	struct ProgramElement *createClassDeclarationElement(struct ClassDeclaration *classDecl);
	struct ProgramElement *createFunctionDefinitionElement(struct FunctionDefinition *funcDef);
	
	struct StatementList *appendStatementToList(struct StatementList *list, struct Statement *stmt);
	struct StatementList *createStatementList(struct Statement *stmt);
	struct Statement *createExpressionStatement(struct Expression *expr);
	struct Statement *createIfStatement(struct IfStatement *ifStmt);
	struct Statement *createWhileStatement(struct WhileStatement *whileStmt);
	struct Statement *createForStatement(struct ForStatement *forStmt);
	struct Statement *createForeachStatement(struct ForeachStatement *foreachStmt);
	struct Statement *createSwitchStatement(struct SwitchStatement *switchStmt);
	struct Statement *createCompoundStatement(struct StatementList *compoundStmt);
	struct Statement *createEchoStatement(struct ExpressionList *echoExprList);
	struct Statement *createReturnStatement(struct Expression *expr);
	struct Statement *createJumpStatement(enum StatementType type);
	
	struct Expression *createExpression(enum ExpressionType type, struct Expression *left, struct Expression *right);
	struct Expression *createExpressionWithRightList(enum ExpressionType type, struct Expression *left, struct ExpressionList *rightExprList);
	struct Expression *createSimpleExpression(enum ExpressionType type, int intValue, float floatValue, char* stringValue);
	struct ExpressionList *appendExpressionToList(struct ExpressionList *list, struct Expression *expr);
	struct ExpressionList *createExpressionList(struct Expression *expr);
	
	struct IfStatement *createIf(struct Expression *condition, struct Statement *ifBlock, struct StatementList *altIfBlock, struct ElseIfStatementList *elseIfBlock, struct Statement *elseBlock, struct StatementList *altElseBlock);
	struct ElseIfStatementList *appendElseIfStatementToList(struct ElseIfStatementList *list, struct ElseIfStatement *elseIf);
	struct ElseIfStatementList *createElseIfStatementList(struct ElseIfStatement *elseIf);
	struct ElseIfStatement *createElseIf(struct Expression *condition, struct Statement *elseIfBlock, struct StatementList *altElseIfBlock);
	
	struct SwitchStatement *createSwitch(struct Expression *condition, struct CaseStatements *caseStmts);
	struct CaseStatements *appendCaseToList(struct CaseStatements *list, struct CaseStatement *caseStmt);
	struct CaseStatements *createCaseList(struct CaseStatement *caseStmt);
	struct CaseStatement *createCase(struct Expression *label, struct StatementList *stmtList);
	
	struct WhileStatement *createWhile(enum WhileType type, struct Expression *condition, struct Statement *whileBlock, struct StatementList *altWhileBlock);
	struct ForStatement *createFor(struct ExpressionList *initializer, struct ExpressionList *control, struct ExpressionList *endOfLoop, struct Statement *forBlock, struct StatementList *altForBlock);
	struct ForeachStatement *createForeach(struct Expression *collection, struct Expression *foreachKey, struct Expression *foreachValue, struct Statement *foreachBlock, struct StatementList *altForeachBlock);
	

	struct ClassDeclaration *createClass(char* className, char* classNameExtended, struct ClassMemberList *classMemberList);
	struct ClassMemberList *appendClassMemberToList(struct ClassMemberList *list, struct ClassMember *classMember);
	struct ClassMemberList *createClassMemberList(struct ClassMember *classMember);
	struct ClassMember *createClassMember(enum ClassMemberType cmType, enum AccessType aType, bool isStatic, struct VariablesList *constList, struct VariablesList *propList, struct FunctionDefinition *funcDef, struct FunctionHeader *abstractFuncDef);
	struct VariableElement *createVariable(char* type, char* name, struct Expression *value);	
	struct VariablesList *appendVariableToList(struct VariablesList *list, struct VariableElement *constElement);
	struct VariablesList *createVariablesList(enum VariableElementType type, struct VariableElement *constElement);
	
	struct FunctionDefinition *createFunctionDefinition(struct FunctionHeader *head, struct Statement *body);
	struct FunctionHeader *createFunctionHeader(char* name, struct VariablesList *parameters, char* type);
	
	
	struct Program *prog;
%}

%union {
	int int_const;
	float float_const;
	char* string_const;
	char* id_const;
	char* type_const;

	struct Program *prog;
	struct ProgramList *progList;
	struct ProgramElement *progElem;
	struct Expression *expr;
	struct ExpressionList *exprList;
	struct Statement *stmt;
	struct StatementList *stmtList;
	struct IfStatement *ifStmt;
	struct ElseIfStatementList *elseIfStmtList;
	struct ElseIfStatement *elseIfStmt;
	struct SwitchStatement *switchStmt;
	struct CaseStatements *caseStmts;
	struct CaseStatement *caseStmt;
	struct WhileStatement *whileStmt;
	struct ForStatement *forStmt;
	struct ForeachStatement *foreachStmt;
	struct ClassDeclaration *classDecl;
	struct ClassMemberList *classMemberList;
	struct ClassMember *classMember;
	struct VariablesList *varList;
	struct VariableElement *varElem;
	struct FunctionDefinition *funcDef;
	struct FunctionHeader *funcHead;
}

%type <prog> program
%type <progList> program_list
%type <progElem> program_element
%type <expr> expression
%type <exprList> expression_list
%type <stmt> statement compound_statement expression_statement echo_statement return_statement jump_statement
%type <stmtList> statement_list
%type <ifStmt> if_statement
%type <elseIfStmtList> elseif_statements_1 elseif_statements_2
%type <elseIfStmt> elseif_statement_1 elseif_statement_2
%type <switchStmt> switch_statement
%type <caseStmts> case_statements
%type <caseStmt> case_statement
%type <whileStmt> while_statement
%type <forStmt> for_statement
%type <foreachStmt> foreach_statement
%type <classDecl> class_declaration
%type <classMemberList> class_member_declarations
%type <classMember> class_member_declaration class_const_elements property_declaration method_declaration
%type <varList> const_elements property_elements parameter_function_list
%type <varElem> const_element property_element parameter_function

%type <funcDef> function_definition
%type <funcHead> function_definition_header

%token <int_const> INT
%token <int_const> BOOL
%token <float_const> FLOAT
%token <string_const> STRING
%token <id_const> ID
%token NIL
%token TYPE
%token CONST
%token CLASS
%token EXTENDS
%token PUBLIC
%token PROTECTED
%token PRIVATE
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
%right '='
%left LOGIC_OR_1
%left LOGIC_AND_1
%nonassoc EQUAL NOT_EQUAL_1 IDENTICALLY_EQUAL IDENTICALLY_NOT_EQUAL NOT_EQUAL_2
%nonassoc '<' LESSER_EQUAL '>' GREATER_EQUAL
%left LEFT_SHIFT RIGHT_SHIFT
%left '+' '-' '.'
%left '*' '/' '%'
%right '!'
%nonassoc INSTANCEOF
%right UMINUS UPLUS
%right POW
%nonassoc ']'
%nonassoc ')'
%nonassoc NEW

%%

program : START_TAG program_list END_TAG	{ $$ = createProgram($2); }
		| START_TAG END_TAG					{ $$ = createProgram(NULL); }
		;
		
program_list : program_element				{ $$ = createProgramList($1); }
			 | program_list program_element	{ $$ = appendProgramToList($1,$2); }
			 ;
			 
program_element : statement					{ $$ = createStatementElement($1); }
				| function_definition		{ $$ = createClassDeclarationElement($1); }
				| class_declaration			{ $$ = createFunctionDefinitionElement($1); }
				;

expression : expression LOGIC_OR_2 expression				{ $$ = createExpression(ET_LOGIC_OR_2,$1,$3); }
		   | expression LOGIC_XOR expression				{ $$ = createExpression(ET_LOGIC_XOR,$1,$3); }
		   | expression LOGIC_AND_2 expression				{ $$ = createExpression(ET_LOGIC_AND_2,$1,$3); }
		   | expression '=' expression						{ $$ = createExpression(ET_ASSIGN,$1,$3); }
		   | expression LOGIC_OR_1 expression				{ $$ = createExpression(ET_LOGIC_OR_1,$1,$3); }
		   | expression LOGIC_AND_1 expression				{ $$ = createExpression(ET_LOGIC_AND_1,$1,$3); }
		   | expression EQUAL expression					{ $$ = createExpression(ET_EQUAL,$1,$3); }
		   | expression NOT_EQUAL_1 expression				{ $$ = createExpression(ET_NOT_EQUAL_1,$1,$3); }
		   | expression NOT_EQUAL_2 expression				{ $$ = createExpression(ET_NOT_EQUAL_2,$1,$3); }
		   | expression IDENTICALLY_EQUAL expression		{ $$ = createExpression(ET_IDENTICALLY_EQUAL,$1,$3); }
		   | expression IDENTICALLY_NOT_EQUAL expression	{ $$ = createExpression(ET_IDENTICALLY_NOT_EQUAL,$1,$3); }
		   | expression '<' expression						{ $$ = createExpression(ET_LESSER,$1,$3); }
		   | expression '>' expression						{ $$ = createExpression(ET_GREATER,$1,$3); }
		   | expression LESSER_EQUAL expression				{ $$ = createExpression(ET_LESSER_EQUAL,$1,$3); }
		   | expression GREATER_EQUAL expression			{ $$ = createExpression(ET_GREATER_EQUAL,$1,$3); }
		   | expression '+' expression						{ $$ = createExpression(ET_PLUS,$1,$3); }
		   | expression '-' expression						{ $$ = createExpression(ET_MINUS,$1,$3); }
		   | expression '.' expression						{ $$ = createExpression(ET_CONCAT,$1,$3); }
		   | expression '*' expression						{ $$ = createExpression(ET_MULT,$1,$3); }
		   | expression '/' expression						{ $$ = createExpression(ET_DIV,$1,$3); }
		   | '!' expression									{ $$ = createExpression(ET_NOT,NULL,$2); }
		   | '+' expression %prec UPLUS						{ $$ = createExpression(ET_PLUS,NULL,$2); }
		   | '-' expression %prec UMINUS					{ $$ = createExpression(ET_MINUS,NULL,$2); }
		   | expression INSTANCEOF expression				{ $$ = createExpression(ET_INSTANCEOF,$1,$3; }
		   | expression POW expression						{ $$ = createExpression(ET_POW,$1,$3); }
		   | expression PROPERTY_ACCESS expression			{ $$ = createExpression(ET_PROPERTY_ACCESS,$1,$3); }
		   | expression STATIC_PROPERTY_ACCESS expression	{ $$ = createExpression(ET_STATIC_PROPERTY_ACCESS,$1,$3); }
		   | '$' expression									{ $$ = createExpression(ET_DOLLAR,NULL,$2); }
		   | expression '[' expression ']'					{ $$ = createExpression(ET_ARRAY,$1,$3); }
		   | expression '[' ']'								{ $$ = createExpression(ET_ARRAY,$1,NULL); }
		   | '(' expression ')'								{ $$ = $2; }
		   | expression '(' expression_list ')'				{ $$ = createExpressionWithRightList(ET_FUNCTION_CALL,$1,$3); }
		   | expression '(' ')'								{ $$ = createExpressionWithRightList(ET_FUNCTION_CALL,$1,NULL); }
		   | NEW expression									{  }
		   | ID												{ $$ = createSimpleExpression(ET_ID,-1,-1,id_const)); }
		   | INT											{ $$ = createSimpleExpression(ET_INT,int_const,-1,NULL)); }
		   | FLOAT											{ $$ = createSimpleExpression(ET_FLOAT,-1,float_const,NULL)); }
		   | STRING											{ $$ = createSimpleExpression(ET_STRING,-1,-1,string_const)); }
		   | BOOL											{ $$ = createSimpleExpression(ET_BOOL,-1,-1,id_const)); }
		   | NIL											{ $$ = createSimpleExpression(ET_NULL,-1,-1,NULL)); }
		   ;
				   
expression_list : expression								{ $$ = createExpressionList($1); }
				| expression_list ',' expression			{ $$ = appendExpressionToList($1,$3); }
				;
						  
		
statement : compound_statement		{ $$ = $1; }
		  | expression_statement	{ $$ = $1; }
		  | while_statement			{ $$ = createWhileStatement($1); }
		  | for_statement			{ $$ = createForStatement($1); }
		  | foreach_statement		{ $$ = createForeachStatement($1); }
		  | if_statement			{ $$ = createIfStatement($1); }
		  | switch_statement		{ $$ = createSwitchStatement($1); }
		  | echo_statement			{ $$ = $1; }
		  | return_statement		{ $$ = $1; }
		  | jump_statement			{ $$ = $1; }
		  ;
		  
compound_statement : '{' statement_list '}'		{ $$ = createCompoundStatement($2); }
				   | '{' '}'					{ $$ = createCompoundStatement(NULL); }
				   ;
				   
expression_statement : expression ';'			{ $$ = createExpressionStatement($1); }
					 ;

echo_statement : ECHO expression_list ';'		{ $$ = createEchoStatement($2); }
			   ;
			   
return_statement : RETURN expression ';'	{ $$ = createReturnStatement($2); }
				 | RETURN ';'				{ $$ = createReturnStatement(NULL); }
				 ;
				 
jump_statement : BREAK ';'					{ $$ = createJumpStatement(ST_BREAK); }
			   | CONTINUE ';'				{ $$ = createJumpStatement(ST_CONTINUE); }

statement_list : statement					{ $$ = createStatementList($1); }
			   | statement_list statement	{ $$ = appendStatementToList($1,$2); }
			   ;
				 
			   
while_statement : WHILE '(' expression ')' statement						{ $$ = createWhile(WT_WHILE,$3,$5,NULL); }
				| WHILE '(' expression ')' ':' statement_list ENDWHILE ';'	{ $$ = createWhile(WT_WHILE,$3,$6); }
				| DO statement WHILE '(' expression ')' ';'					{ $$ = createWhile(WT_DO_WHILE,$5,$2,NULL); }
				;
			 
for_statement : FOR '(' expression_list ';' expression_list ';' expression_list ')' statement						{ $$ = createFor($3,$5,$7,$9,NULL); }
			  | FOR '(' ';' expression_list ';' expression_list ')' statement										{ $$ = createFor(NULL,$4,$6,$8,NULL); }
			  | FOR '(' expression_list ';' ';' expression_list ')' statement										{ $$ = createFor($3,NULL,$6,$8,NULL); }
			  | FOR '(' expression_list ';' expression_list ';' ')' statement										{ $$ = createFor($3,$5,NULL,$8,NULL); }
			  | FOR '(' ';' ';' expression_list ')' statement														{ $$ = createFor(NULL,NULL,$5,$7,NULL); }
			  | FOR '(' expression_list ';' ';' ')' statement														{ $$ = createFor($3,NULL,NULL,$7,NULL); }
			  | FOR '(' ';' expression_list ';' ')' statement														{ $$ = createFor(NULL,$4,NULL,$7,NULL); }
			  | FOR '(' ';' ';' ')' statement																		{ $$ = createFor(NULL,NULL,NULL,$6,NULL); }
			  | FOR '(' expression_list ';' expression_list ';' expression_list ')' ':' statement_list	ENDFOR ';'	{ $$ = createFor($3,$5,$7,NULL,$10); }
			  | FOR '(' ';' expression_list ';' expression_list ')' ':' statement_list ENDFOR ';'					{ $$ = createFor(NULL,$4,$6,NULL,$9); }
			  | FOR '(' expression_list ';' ';' expression_list ')' ':' statement_list ENDFOR ';'					{ $$ = createFor($3,NULL,$6,NULL,$9); }
			  | FOR '(' expression_list ';' expression_list ';' ')' ':' statement_list ENDFOR ';'					{ $$ = createFor($3,$5,NULL,NULL,$9); }
			  | FOR '(' ';' ';' expression_list ')' ':' statement_list ENDFOR ';'									{ $$ = createFor(NULL,NULL,$5,NULL,$8); }
			  | FOR '(' expression_list ';' ';' ')' ':' statement_list ENDFOR ';'									{ $$ = createFor($3,NULL,NULL,NULL,$8); }
			  | FOR '(' ';' expression_list ';' ')' ':' statement_list ENDFOR ';'									{ $$ = createFor(NULL,$4,NULL,NULL,$8); }
			  | FOR '(' ';' ';' ')' ':' statement_list ENDFOR ';'													{ $$ = createFor(NULL,NULL,NULL,NULL,$7); }
			  ;
			   
foreach_statement : FOREACH '(' expression AS expression ')' statement													{ $$ = createForeach($3,NULL,$5,$7,NULL); }
				  | FOREACH '(' expression AS expression KEY_ACCESS expression ')' statement							{ $$ = createForeach($3,$5,$7,$9,NULL); }
				  | FOREACH '(' expression AS expression ')' ':' statement_list ENDFOREACH ';'							{ $$ = createForeach($3,NULL,$5,NULL,$8); }
				  | FOREACH '(' expression AS expression KEY_ACCESS expression ')' ':' statement_list ENDFOREACH ';'	{ $$ = createForeach($3,$5,$7,NULL,$10); }
				  ;
				  
if_statement : IF '(' expression ')' statement elseif_statements_1 ELSE statement								{ $$ = createIf($3,$5,NULL,$6,$8,NULL); }
			 | IF '(' expression ')' statement elseif_statements_1												{ $$ = createIf($3,$5,NULL,$6,NULL,NULL); }
			 | IF '(' expression ')' statement ELSE statement													{ $$ = createIf($3,$5,NULL,NULL,$7,NULL); }
			 | IF '(' expression ')' statement																	{ $$ = createIf($3,$5,NULL,NULL,NULL,NULL); }
			 | IF '(' expression ')' ':' statement_list elseif_statements_2 ELSE ':' statement_list ENDIF ';'	{ $$ = createIf($3,NULL,$6,$7,NULL,$10); }
			 | IF '(' expression ')' ':' statement_list elseif_statements_2 ENDIF ';'							{ $$ = createIf($3,NULL,$6,$7,NULL,NULL); }
			 | IF '(' expression ')' ':' statement_list ELSE ':' statement_list ENDIF ';'						{ $$ = createIf($3,NULL,$6,NULL,NULL,$9); }
			 | IF '(' expression ')' ':' statement_list ENDIF ';'												{ $$ = createIf($3,NULL,$6,NULL,NULL,NULL); }
			 ;

elseif_statements_1 : elseif_statement_1													{ $$ = createElseIfStatementList($1); }
					| elseif_statements_1 elseif_statement_1								{ $$ = appendElseIfStatementToList($1,$2); }
					;
					
elseif_statement_1 : ELSEIF '(' expression ')' statement								{ $$ = createElseIf($3,$5,NULL); }
				   ;
				   
				 
elseif_statements_2 : elseif_statement_2												{ $$ = createElseIfStatementList($1); }
					| elseif_statements_2 elseif_statement_2							{ $$ = appendElseIfStatementToList($1,$2); }
					;
					
elseif_statement_2 : ELSEIF '(' expression ')' ':' statement_list					{ $$ = createElseIf($3,NULL,$6); }
				   ;
					
switch_statement : SWITCH '(' expression ')' '{' case_statements '}'				{ $$ = createSwitch($3,$6); }
				 | SWITCH '(' expression ')' '{' '}'								{ $$ = createSwitch($3,NULL); }
				 | SWITCH '(' expression ')' ':' case_statements ENDSWITCH ';'		{ $$ = createSwitch($3,$6); }
				 | SWITCH '(' expression ')' ':' ENDSWITCH ';'						{ $$ = createSwitch($3,NULL); }
				 ;

case_statements : case_statement						{ $$ = createCaseList($1); }
				| case_statements case_statement		{ $$ = appendCaseToList($1,$2); }
				;
				 
case_statement : CASE expression ':' statement_list		{ $$ = createCase($2,$4); }
			   | CASE expression ':'					{ $$ = createCase($2,NULL); }
			   | CASE expression ';' statement_list		{ $$ = createCase($2,$4); }
			   | CASE expression ';'					{ $$ = createCase($2,NULL); }
			   | DEFAULT ':' statement_list				{ $$ = createCase(NULL,$3); }
			   | DEFAULT ':'							{ $$ = createCase(NULL,NULL); }
			   | DEFAULT ';' statement_list				{ $$ = createCase(NULL,$3); }
			   | DEFAULT ';'							{ $$ = createCase(NULL,NULL); }
			   ;

function_definition : function_definition_header compound_statement					{ $$ = createFunctionDefinition($1,$2); }
					;
					
function_definition_header : FUNCTION ID '(' parameter_function_list ')'			{ $$ = createFunctionHeader(id_const,$4,NULL); }
						   | FUNCTION ID '(' parameter_function_list ')' ':' TYPE	{ $$ = createFunctionHeader(id_const,$4,type_const); }
						   | FUNCTION ID '(' ')'									{ $$ = createFunctionHeader(id_const,NULL,NULL); }
						   | FUNCTION ID '(' ')' ':' TYPE							{ $$ = createFunctionHeader(id_const,NULL,type_const); }
						   ;
					
parameter_function_list : parameter_function										{ $$ = createVariablesList(VET_ARG,$1); }
						| parameter_function_list ',' parameter_function			{ $$ = appendVariableToList($1,$3); }
						;
						
parameter_function : '$' ID							{ $$ = createVariable(NULL,id_const,NULL); }
				   | '$' ID '=' expression			{ $$ = createVariable(NULL,id_const,$4); }
				   | TYPE '$' ID					{ $$ = createVariable(type_const,id_const,NULL); }
				   | TYPE '$' ID '=' expression		{ $$ = createVariable(type_const,id_const,$5); }
				   ;
				   


class_declaration : CLASS ID '{' class_member_declarations '}'					{ $$ = createClass(id_const,NULL,$4); }
				  | CLASS ID EXTENDS ID '{' class_member_declarations '}'		{ $$ = createClass(id_const,id_const,$6); }
				  ;

class_member_declarations : class_member_declaration							{ $$ = createClassMemberList($1); }
						  | class_member_declarations class_member_declaration	{ $$ = appendClassMemberToList($1,$2); }
						  ;
						  
class_member_declaration : class_const_elements	{ $$ = $1; }
						 | property_declaration	{ $$ = $1; }
						 | method_declaration	{ $$ = $1; }
						 ;

class_const_elements : CONST const_elements ';'				{ $$ = createClassMember(CMT_CONST,AT_PUBLIC,false,$2,NULL,NULL,NULL); }
					 | PUBLIC CONST const_elements ';'		{ $$ = createClassMember(CMT_CONST,AT_PUBLIC,false,$3,NULL,NULL,NULL); }
					 | PROTECTED CONST const_elements ';'	{ $$ = createClassMember(CMT_CONST,AT_PROTECTED,false,$3,NULL,NULL,NULL); }
					 | PRIVATE CONST const_elements ';'		{ $$ = createClassMember(CMT_CONST,AT_PRIVATE,false,$3,NULL,NULL,NULL); }
					 ;

const_elements : const_element								{ $$ = createVariablesList(VET_CONST,$1); }
			   | const_elements ',' const_element			{ $$ = appendVariableToList($1,$3); }
			   ;
			   
const_element : ID '=' expression							{ $$ = createVariable(NULL,id_const,$3); }
			  ;
			   
property_declaration : VAR property_elements				{ $$ = createClassMember(CMT_PROPERTY,AT_PUBLIC,false,NULL,$2,NULL,NULL); }
					 | PUBLIC property_elements				{ $$ = createClassMember(CMT_PROPERTY,AT_PUBLIC,false,NULL,$2,NULL,NULL); }
					 | PROTECTED property_elements			{ $$ = createClassMember(CMT_PROPERTY,AT_PROTECTED,false,NULL,$2,NULL,NULL); }
					 | PRIVATE property_elements			{ $$ = createClassMember(CMT_PROPERTY,AT_PRIVATE,false,NULL,$2,NULL,NULL); }
					 | PUBLIC STATIC property_elements		{ $$ = createClassMember(CMT_PROPERTY,AT_PUBLIC,true,NULL,$3,NULL,NULL); }
					 | PROTECTED STATIC property_elements	{ $$ = createClassMember(CMT_PROPERTY,AT_PROTECTED,true,NULL,$3,NULL,NULL); }
					 | PRIVATE STATIC property_elements		{ $$ = createClassMember(CMT_PROPERTY,AT_PRIVATE,true,NULL,$3,NULL,NULL); }
					 | STATIC property_elements				{ $$ = createClassMember(CMT_PROPERTY,AT_PUBLIC,true,NULL,$2,NULL,NULL); }
					 | STATIC PUBLIC property_elements		{ $$ = createClassMember(CMT_PROPERTY,AT_PUBLIC,true,NULL,$3,NULL,NULL); }
					 | STATIC PROTECTED property_elements	{ $$ = createClassMember(CMT_PROPERTY,AT_PROTECTED,true,NULL,$3,NULL,NULL); }
					 | STATIC PRIVATE property_elements		{ $$ = createClassMember(CMT_PROPERTY,AT_PRIVATE,true,NULL,$3,NULL,NULL); }
					 ;
					 
property_elements : property_element						{ $$ = createVariablesList(VET_PROPERTY,$1); }
				  | property_elements ',' property_element	{ $$ = appendVariableToList($1,$3); }
				  ;
				  
property_element : '$' ID ';'					{ $$ = createVariable(NULL,id_const,NULL); }
				 | '$' ID '=' expression ';'	{ $$ = createVariable(NULL,id_const,$4); }
				 ;
				
method_declaration : function_definition								{ $$ = createClassMember(CMT_METHOD,AT_PUBLIC,false,NULL,NULL,$1,NULL); }
				   | PUBLIC function_definition							{ $$ = createClassMember(CMT_METHOD,AT_PUBLIC,false,NULL,NULL,$2,NULL); }
				   | PROTECTED function_definition						{ $$ = createClassMember(CMT_METHOD,AT_PROTECTED,false,NULL,NULL,$2,NULL); }
				   | PRIVATE function_definition						{ $$ = createClassMember(CMT_METHOD,AT_PRIVATE,false,NULL,NULL,$2,NULL); }
				   | PUBLIC STATIC function_definition					{ $$ = createClassMember(CMT_METHOD,AT_PUBLIC,true,NULL,NULL,$3,NULL); }
				   | PROTECTED STATIC function_definition				{ $$ = createClassMember(CMT_METHOD,AT_PROTECTED,true,NULL,NULL,$3,NULL); }
				   | PRIVATE STATIC function_definition					{ $$ = createClassMember(CMT_METHOD,AT_PRIVATE,true,NULL,NULL,$3,NULL); }
				   | STATIC function_definition							{ $$ = createClassMember(CMT_METHOD,AT_PUBLIC,true,NULL,NULL,$2,NULL); }
				   | STATIC PUBLIC function_definition					{ $$ = createClassMember(CMT_METHOD,AT_PUBLIC,true,NULL,NULL,$3,NULL); }
				   | STATIC PROTECTED function_definition				{ $$ = createClassMember(CMT_METHOD,AT_PROTECTED,true,NULL,NULL,$3,NULL); }
				   | STATIC PRIVATE function_definition					{ $$ = createClassMember(CMT_METHOD,AT_PRIVATE,true,NULL,NULL,$3,NULL); }
				   | PUBLIC function_definition_header ';'				{ $$ = createClassMember(CMT_METHOD,AT_PUBLIC,false,NULL,NULL,NULL,$2); }
				   | PROTECTED function_definition_header ';'			{ $$ = createClassMember(CMT_METHOD,AT_PROTECTED,false,NULL,NULL,NULL,$2); }
				   | PRIVATE function_definition_header ';'				{ $$ = createClassMember(CMT_METHOD,AT_PRIVATE,false,NULL,NULL,NULL,$2); }
				   | PUBLIC STATIC function_definition_header ';'		{ $$ = createClassMember(CMT_METHOD,AT_PUBLIC,true,NULL,NULL,NULL,$3); }
				   | PROTECTED STATIC function_definition_header ';'	{ $$ = createClassMember(CMT_METHOD,AT_PROTECTED,true,NULL,NULL,NULL,$3); }
				   | PRIVATE STATIC function_definition_header ';'		{ $$ = createClassMember(CMT_METHOD,AT_PRIVATE,true,NULL,NULL,NULL,$3); }
				   | STATIC function_definition_header ';'				{ $$ = createClassMember(CMT_METHOD,AT_PUBLIC,true,NULL,NULL,NULL,$2); }
				   | STATIC PUBLIC function_definition_header ';'		{ $$ = createClassMember(CMT_METHOD,AT_PUBLIC,true,NULL,NULL,NULL,$3); }
				   | STATIC PROTECTED function_definition_header ';'	{ $$ = createClassMember(CMT_METHOD,AT_PROTECTED,true,NULL,NULL,NULL,$3); }
				   | STATIC PRIVATE function_definition_header ';'		{ $$ = createClassMember(CMT_METHOD,AT_PRIVATE,true,NULL,NULL,NULL,$3); }
				   ;
				
				
				
%%

void yyerror(char const *s)
{
	printf("%s",s);
}				
				
struct Program *createProgram(struct ProgramList *progList)
{
}

struct ProgramList *appendProgramToList(struct ProgramList *list, struct ProgramElement *elem)
{
	list->last->next = stmt;
	list->last = stmt;
	return list;
}

struct ProgramList *createProgramList(struct ProgramElement *elem)
{
	struct ProgramList *result = (struct ProgramList *)malloc(sizeof(struct ProgramList));
	result->first = elem;
	result->last = elem;
	return result;
}

struct ProgramElement *createStatementElement(struct Statement *stmt)
{
	struct ProgramElement *result = (struct ProgramElement *)malloc(sizeof(struct ProgramElement));
	result->type = PET_STATEMENT;
	result->stmt = stmt;
	return result;
}

struct ProgramElement *createClassDeclarationElement(struct ClassDeclaration *classDecl)
{
	struct ProgramElement *result = (struct ProgramElement *)malloc(sizeof(struct ProgramElement));
	result->type = PET_CLASS;
	result->classDecl = classDecl;
	return result;
}

struct ProgramElement *createFunctionDefinitionElement(struct FunctionDefinition *funcDef)
{
	struct ProgramElement *result = (struct ProgramElement *)malloc(sizeof(struct ProgramElement));
	result->type = PET_FUNCTION;
	result->funcDef = funcDef;
	return result;
}


struct StatementList *appendStatementToList(struct StatementList *list, struct Statement *stmt)
{
	list->last->next = stmt;
	list->last = stmt;
	return list;
}
struct StatementList *createStatementList(struct Statement *stmt)
{
	struct StatementList *result = (struct StatementList *)malloc(sizeof(struct StatementList));
	result->first = stmt;
	result->last = stmt;
	return result;
}

struct Statement *createExpressionStatement(struct Expression *expr)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	result->type = ST_EXPRESSION;
	result->expr = expr;
	return result;
}

struct Statement *createIfStatement(struct IfStatement *ifStmt)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	result->type = ST_IF;
	result->ifStmt = ifStmt;
	return result;
}

struct Statement *createWhileStatement(struct WhileStatement *whileStmt)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	result->type = ST_WHILE;
	result->whileStmt = whileStmt;
	return result;
}

struct Statement *createForStatement(struct ForStatement *forStmt)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	result->type = ST_FOR;
	result->forStmt = forStmt;
	return result;
}

struct Statement *createForeachStatement(struct ForeachStatement *foreachStmt)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	result->type = ST_FOREACH;
	result->foreachStmt = foreachStmt;
	return result;
}

struct Statement *createSwitchStatement(struct SwitchStatement *switchStmt)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	result->type = ST_SWITCH;
	result->switchStmt = switchStmt;
	return result;
}

struct Statement *createCompoundStatement(struct StatementList *compoundStmt)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	result->type = ST_COMPOUND;
	result->compoundStmt = compoundStmt;
	return result;
}

struct Statement *createEchoStatement(struct ExpressionList *echoExprList)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	result->type = ST_ECHO;
	result->echoExprList = echoExprList;
	return result;
}

struct Statement *createReturnStatement(struct Expression *expr)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	result->type = ST_RETURN;
	result->expr = expr;
	return result;
}

struct Statement *createJumpStatement(enum StatementType type)
{
	struct Statement *result = (struct Statement *)malloc(sizeof(struct Statement));
	result->type = type;
	return result;
}


struct WhileStatement *createWhile(enum WhileType type, struct Expression *condition, struct Statement *whileBlock, struct StatementList *altWhileBlock)
{
	struct WhileStatement *result = (struct WhileStatement *)malloc(sizeof(struct WhileStatement));
	result->type = type;
	result->condition = condition;
	result->whileBlock = whileBlock;
	result->altWhileBlock = altWhileBlock;
	return result;
}

struct ForStatement *createFor(struct ExpressionList *initializer, struct ExpressionList *control, struct ExpressionList *endOfLoop, struct Statement *forBlock, struct StatementList *altForBlock)
{
	struct ForStatement *result = (struct ForStatement *)malloc(sizeof(struct ForStatement));
	result->initializer = initializer;
	result->control = control;
	result->endOfLoop = endOfLoop;
	result->forBlock = forBlock;
	result->altForBlock = altForBlock;
	return result;
}

struct ForeachStatement *createForeach(struct Expression *collection, struct Expression *foreachKey, struct Expression *foreachValue, struct Statement *foreachBlock, struct StatementList *altForeachBlock)
{
	struct ForeachStatement *result = (struct ForeachStatement *)malloc(sizeof(struct ForeachStatement));
	result->collection = collection;
	result->foreachKey = foreachKey;
	result->foreachValue = foreachValue;
	result->foreachBlock = foreachBlock;
	result->altForeachBlock = altForeachBlock;
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

struct Expression *createExpressionWithRightList(enum ExpressionType type, struct Expression *left, struct ExpressionList *rightExprList)
{
	struct Expression *result = (struct Expression *)malloc(sizeof(struct Expression));
	result->type = type;
	result->left = left;
	result->rightExprList = rightExprList;
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

struct ExpressionList *appendExpressionToList(struct ExpressionList *list, struct Expression *expr)
{
	list->last->next = expr;
	list->last = expr;
	return list;
}

struct ExpressionList *createExpressionList(struct Expression *expr)
{
	struct ExpressionList *result = (struct ExpressionList *)malloc(sizeof(struct ExpressionList));
	result->first = expr;
	result->last = expr;
	return result;
}


struct IfStatement *createIf(struct Expression *condition, struct Statement *ifBlock, struct StatementList *altIfBlock, struct ElseIfStatements *elseIfBlock, struct Statement *elseBlock, struct StatementList *altElseBlock)
{
	struct IfStatement *result = (struct IfStatement *)malloc(sizeof(struct IfStatement));
	result->condition = condition;
	result->ifBlock = ifBlock;
	result->altIfBlock = altIfBlock;
	result->elseIfBlock = elseIfBlock;
	result->elseBlock = elseBlock;
	result->altElseBlock = altElseBlock;
	return result;
}

struct ElseIfStatementList *appendElseIfStatementToList(struct ElseIfStatementList *list, struct ElseIfStatement *elseIf)
{
	list->last->next = elseIf;
	list->last = elseIf;
	return list;
}

struct ElseIfStatementList *createElseIfStatementList(struct ElseIfStatement *elseIf)
{
	struct ElseIfStatementList *result = (struct ElseIfStatementList *)malloc(sizeof(struct ElseIfStatementList));
	result->first = elseIf;
	result->last = elseIf;
	return result;
}

struct ElseIfStatement *createElseIf(struct Expression *condition, struct Statement *elseIfBlock, struct StatementList *altElseIfBlock)
{
	struct ElseIfStatement *result = (struct ElseIfStatement *)malloc(sizeof(struct ElseIfStatement));
	result->condition = condition;
	result->elseIfBlock = elseIfBlock;
	result->altElseIfBlock = altElseIfBlock;
	return result;
}

struct SwitchStatement *createSwitch(struct Expression *condition, struct CaseStatements *caseStmts)
{
	struct SwitchStatement *result = (struct SwitchStatement *)malloc(sizeof(struct SwitchStatement));
	result->condition = condition;
	result->caseStmts = caseStmts;
	return result;
}

struct CaseStatements *appendCaseToList(struct CaseStatements *list, struct CaseStatement *caseStmt)
{
	list->last->next = caseStmt;
	list->last = caseStmt;
	return list;
}

struct CaseStatements *createCaseList(struct CaseStatement *caseStmt)
{
	struct CaseStatements *result = (struct CaseStatements *)malloc(sizeof(struct CaseStatements));
	result->first = caseStmt;
	result->last = caseStmt;
	return result;
}

struct CaseStatement *createCase(struct Expression *label, struct StatementList *stmtList)
{
	struct CaseStatement *result = (struct CaseStatement *)malloc(sizeof(struct CaseStatement));
	result->label = label;
	result->stmtList = stmtList;
	return result;
}



struct VariableElement *createVariable(char* type, char* name, struct Expression *value)
{
	struct VariableElement *result = (struct VariableElement *)malloc(sizeof(struct VariableElement));
	result->type = type;
	result->name = name;
	result->value = value;
	return result;
}

struct ClassDeclaration *createClass(char* className, char* classNameExtended, struct ClassMemberList *classMemberList)
{
	struct ClassDeclaration *result = (struct ClassDeclaration *)malloc(sizeof(struct ClassDeclaration));
	result->className = className;
	result->classNameExtended = classNameExtended;
	result->classMemberList = classMemberList;
	return result;
}

struct ClassMemberList *appendClassMemberToList(struct ClassMemberList *list, struct ClassMember *classMember)
{
	list->last->next = classMember;
	list->last = classMember;
	return list;
}

struct ClassMemberList *createClassMemberList(struct ClassMember *classMember)
{
	struct ClassMemberList *result = (struct ClassMemberList *)malloc(sizeof(struct ClassMemberList));
	result->first = classMember;
	result->last = classMember;
	return result;
}

struct ClassMember *createClassMember(enum ClassMemberType cmType, enum AccessType aType, bool isStatic, struct VariablesList *constList, struct VariablesList *propList, struct FunctionDefinition *funcDef, struct FunctionHeader *abstractFuncDef)
{
	struct ClassMember *result = (struct ClassMember *)malloc(sizeof(struct ClassMember));
	result->cmType = cmType;
	result->aType = aType;
	result->isStatic = isStatic;
	result->constList = constList;
	result->propList = propList;
	result->funcDef = funcDef;
	result->abstractFuncDef = abstractFuncDef;
	return result;
}

struct VariablesList *appendConstElementToList(struct VariablesList *list, struct VariableElement *constElement)
{
	list->last->next = constElement;
	list->last = constElement;
	return list;
}

struct VariablesList *createVariablesList(enum VariableElementType type, struct VariableElement *constElement)
{
	struct VariablesList *result = (struct VariablesList *)malloc(sizeof(struct VariablesList));
	result->type = type;
	result->first = constElement;
	result->last = constElement;
	return result;
}



struct FunctionDefinition *createFunctionDefinition(struct FunctionHeader *head, struct Statement *body)
{
	struct FunctionDefinition *result = (struct FunctionDefinition *)malloc(sizeof(struct FunctionDefinition));
	result->head = head;
	result->body = body;
	return result;
}

struct FunctionHeader *createFunctionHeader(char* name, struct VariablesList *parameters, char* type)
{
	struct FunctionHeader *result = (struct FunctionHeader *)malloc(sizeof(struct FunctionHeader));
	result->name = name;
	result->parameters = parameters;
	result->type = type;
	return result;
}










