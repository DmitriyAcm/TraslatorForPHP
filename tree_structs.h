#ifndef TREE_STRUCTS
#define TREE_STRUCTS

enum ExpressionType
{
	ET_INT,
	ET_FLOAT,
	ET_STRING,
	ET_BOOL,
	ET_NULL,
	ET_ID,
	ET_POW,
	ET_INSTANCEOF,
	ET_DOLLAR,
	ET_ARRAY,
	ET_NOT,
	ET_PLUS,
	ET_MINUS,
	ET_MULT,
	ET_DIV,
	ET_CONCAT,
	ET_MOD,
	ET_LESSER,
	ET_LESSER_EQUAL,
	ET_GREATER,
	ET_GREATER_EQUAL,
	ET_EQUAL,
	ET_NOT_EQUAL_1,
	ET_IDENTICALLY_EQUAL,
	ET_IDENTICALLY_NOT_EQUAL,
	ET_NOT_EQUAL_2,
	ET_LOGIC_AND_1,
	ET_LOGIC_OR_1,
	ET_LOGIC_XOR,
	ET_LOGIC_AND_2,
	ET_LOGIC_OR_2,
	ET_ASSIGN,
	ET_PROPERTY_ACCESS,
	ET_STATIC_PROPERTY_ACCESS,
	ET_KEY_ACCESS,
	ET_FUNCTION_CALL,
	ET_OBJECT_CREATION
};

enum StatementType
{
	ST_EXPRESSION,
	ST_IF,
	ST_SWITCH,
	ST_WHILE,
	ST_FOR,
	ST_FOREACH,
	ST_COMPOUND,
	ST_ECHO,
	ST_RETURN,
	ST_BREAK,
	ST_CONTINUE
};

enum ClassMemberType
{
	CMT_CONST,
	CMT_PROPERTY,
	CMT_METHOD
};

enum VariableElementType
{
	VET_CONST,
	VET_PROPERTY,
	VET_ARG
};

enum AccessType
{
	AT_PUBLIC,
	AT_PROTECTED,
	AT_PRIVATE
};

enum WhileType
{
	WT_WHILE,
	WT_DO_WHILE
};

enum ProgramElementType
{
	PET_STATEMENT,
	PET_CLASS,
	PET_FUNCTION
};

struct Expression
{
	enum ExpressionType type;
	int intValue;
	float floatValue;
	char* stringValue;
	struct Expression *left;
	struct Expression *right;
	struct ExpressionList *rightExprList;
	struct Expression *next;
};

struct Statement
{
	enum StatementType type;
	struct Expression *expr;
	struct IfStatement *ifStmt;
	struct WhileStatement *whileStmt;
	struct ForStatement *forStmt;
	struct ForeachStatement *foreachStmt;
	struct SwitchStatement *switchStmt;
	struct StatementList *compoundStmt;
	struct ExpressionList *echoExprList;
	struct Statement *next;
};

struct Program
{
	struct ProgramList *progList;
};

struct ProgramList
{
	struct ProgramElement *first;
	struct ProgramElement *last;
};

struct ProgramElement
{
	enum ProgramElementType type;
	struct Statement *stmt;
	struct ClassDeclaration *classDecl;
	struct FunctionDefinition *funcDef;
	struct ProgramElement *next;
};

struct StatementList
{
	struct Statement *first;
	struct Statement *last;
};

struct ExpressionList
{
	struct Expression *first;
	struct Expression *last;
};

struct IfStatement
{
	struct Expression *condition;
	struct Statement *ifBlock;
	struct StatementList *altIfBlock;
	struct ElseIfStatementList *elseIfBlock;
	struct Statement *elseBlock;
	struct StatementList *altElseBlock;
};

struct ElseIfStatementList
{
	struct ElseIfStatement *first;
	struct ElseIfStatement *last;
};

struct ElseIfStatement
{
	struct Expression *condition;
	struct Statement *elseIfBlock;
	struct StatementList *altElseIfBlock;
	struct ElseIfStatement *next;
};

struct SwitchStatement
{
	struct Expression *condition;
	struct CaseStatements *caseStmts;
};

struct CaseStatements
{
	struct CaseStatement *first;
	struct CaseStatement *last;
};

struct CaseStatement
{
	struct Expression *label;
	struct StatementList *stmtList;
	struct CaseStatement *next;
};

struct WhileStatement
{
	enum WhileType type;
	struct Expression *condition;
	struct Statement *whileBlock;
	struct StatementList *altWhileBlock;
};

struct ForStatement
{
	struct ExpressionList *initializer;
	struct ExpressionList *control;
	struct ExpressionList *endOfLoop;
	struct Statement *forBlock;
	struct StatementList *altForBlock;
};

struct ForeachStatement
{
	struct Expression *collection;
	struct Expression *element;
	struct Statement *foreachBlock;
	struct StatementList *altForeachBlock;
};

struct ClassDeclaration
{
	char* className;
	char* classNameExtended;
	struct ClassMemberList *classMemberList;
};

struct ClassMemberList
{
	struct ClassMember *first;
	struct ClassMember *last;
};

struct ClassMember
{
	enum ClassMemberType cmType;
	enum AccessType aType;
	int isStatic;
	struct VariablesList *constList;
	struct VariablesList *propList;
	struct FunctionDefinition *funcDef;
	struct ClassMember *next;
};

struct VariablesList
{
	enum VariableElementType type;
	struct VariableElement *first;
	struct VariableElement *last;
};

struct VariableElement
{
	char* type;
	char* name;
	struct Expression *value;
	struct VariableElement *next;
};

struct FunctionDefinition
{
	struct FunctionHeader *head;
	struct Statement *body;
};

struct FunctionHeader
{
	char* name;
	struct VariablesList *parameters;
	char* type;
};


#endif










