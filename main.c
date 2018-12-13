// flex.cpp: определяет точку входа для консольного приложения.
//

//#include "stdafx.h"
//#include "flex.yy.c"
//#include <locale.h>
//#include <string>
//
//
//void _tmain(int argc, _TCHAR* argv[])
//{
//	freopen("output.txt", "w", stdout);
//	yyin = fopen("test.php", "r");
//	yylex();
//}

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <process.h>
#include <locale.h>
#include "tree_print.h"
#include "semantics.h"

//extern "C" FILE * yyin;
//extern "C" int yyparse(void);
extern FILE * yyin;
extern int yyparse(void);

void main(void)
{
	yyin = fopen("test.php", "r");
	yyparse();
	printProgram("startTree.dot");
	doSemantic();
	printProgram("finalTree.dot");
	system("pause");
}