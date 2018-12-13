%{
	#include "tree_structs.h"
	#include "php_tab.h"
	#include <math.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include <conio.h>
	#include <locale.h> 
	//#include "tree_print.h"
	extern int yyparse(void);
%}

%option noyywrap
%option never-interactive
%option yylineno

ID				[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
TRUE 			[tT][rR][uU][eE]
FALSE 			[fF][Aa][lL][sS][eE]
NULL 			[Nn][uU][lL]{2}

INT_10 			[1-9][0-9]*|0
INT_16 			0[xX][0-9a-fA-F]+
INT_8 			0[0-7]*
INT_2 			0[bB][01]+

NUM          	[0-9]+
FLOAT          	([0-9]*[\.]{NUM})|({NUM}[\.][0-9]*)
EXPONENT	 	(({NUM}|{FLOAT})[eE][+-]?{NUM})	

%x PHP
%x ONE_LINE_COMMENT
%x MULTILINE_COMMENT
%x HTML_COMMENT
%x SINGLE_QUOTED_STRING
%x DOUBLE_QUOTED_STRING
%x SIMPLE_COMPLEX_INSERT
%x SIMPLE_SQUARE_BRACKETS
%x HARD_COMPLEX_INSERT
%x BEGDOC
%x HEREDOC
%x NOWDOC


%%

%{
	int s = 0;
	char buf[100000];
	char stringID[100000];
	char single_char_str[10];
	char num[20];

	int stackState[100] = {0};
	
	int nState = 0;
	int cntBracket = 0;
%}


"<!--"								{ stackState[++nState] = HTML_COMMENT; BEGIN(stackState[nState]); }
<HTML_COMMENT>[^\-]+				;
<HTML_COMMENT>"-"					;
<HTML_COMMENT>\-{2}\>				{ BEGIN(stackState[--nState]); }

<PHP>("//"|"#").*       			;

<PHP>"/*"							{ stackState[++nState] = MULTILINE_COMMENT; BEGIN(stackState[nState]); }
<MULTILINE_COMMENT>[^*]*			;
<MULTILINE_COMMENT>"*"+[^*/]* 		;
<MULTILINE_COMMENT>"*"+"/" 			{ BEGIN(stackState[--nState]); }

<PHP>\'								{ stackState[++nState] = SINGLE_QUOTED_STRING; BEGIN(SINGLE_QUOTED_STRING); buf[0] = '\0'; s = yylineno; }
<SINGLE_QUOTED_STRING>\\\'			{ strcat(buf, "\'"); }
<SINGLE_QUOTED_STRING>\\\\     		{ strcat(buf, "\\"); }
<SINGLE_QUOTED_STRING>\\[^\\\']     { strcat(buf, yytext); }
<SINGLE_QUOTED_STRING>[^\\\']+		{ strcat(buf, yytext); }
<SINGLE_QUOTED_STRING>\'			{ char *res = (char*)malloc(sizeof(char)*1024); strcpy(res,buf); yylval.string_const = res; BEGIN(PHP); buf[0]='\0'; return STRING; }

<PHP>"<<<"              			{ stackState[++nState] = BEGDOC; BEGIN(BEGDOC); }
<BEGDOC>[ \t]*            			;
<BEGDOC>(\'{ID}\'|{ID})[^\n]        { BEGIN(stackState[--nState]); printf("ERROR! Wrong doc literal!\n"); }
<BEGDOC>\'{ID}\'\n          		{ stackState[nState] = NOWDOC; BEGIN(NOWDOC);  buf[0]='\0'; stringID[0]='\0'; strcpy(stringID,yytext+1); stringID[strlen(stringID)-2]='\0'; s = yylineno; }
<BEGDOC>{ID}\n            			{ stackState[nState] = HEREDOC; BEGIN(HEREDOC);  buf[0]='\0'; stringID[0]='\0'; strcpy(stringID,yytext); stringID[strlen(stringID)-1]='\0'; s = yylineno; }

<NOWDOC,HEREDOC>^{ID}/;\n			{
										char check_end[1000] = "";
										strncat(check_end, yytext, strcspn(yytext,";"));
										if (strcmp(check_end,stringID)==0)
										{
											char *res = (char*)malloc(sizeof(char)*1024);
											//BEGIN(stackState[--nState]);
											buf[strlen(buf)-1] = '\0';
											strcpy(res,buf); 
											yylval.string_const = res; 
											BEGIN(PHP); 
											buf[0]='\0'; 
											return STRING;
										}
										else
										{
											strcat(buf,yytext);
										}
									}
															
<NOWDOC>\n+							{ strcat(buf,yytext); }
<NOWDOC>[^\s\n]+					{ strcat(buf,yytext); }


<PHP>"\""											{ stackState[++nState] = DOUBLE_QUOTED_STRING; BEGIN(stackState[nState]); buf[0]='\0'; s = yylineno; }
<DOUBLE_QUOTED_STRING>[^\\\"\$\{]+					{ strcat(buf,yytext); }

<HEREDOC>[^\\\$\{]									{ strcat(buf,yytext); }

<DOUBLE_QUOTED_STRING,HEREDOC>\\\$					{ strcat(buf,"$"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\\{					{ strcat(buf,"{"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\${ID}				{
														int curline = yylineno;
														printf("Found string literal\n\"%s\"\nfrom line %d to line %d\n", buf, s, curline);
														printf("Found operator \".\" in line %d\n", curline);
														printf("Found operator \"$\" in line %d\n",curline);
														printf("Found identifier \"%s\" in line %d\n",yytext+1,curline);
														buf[0]='\0';

														stackState[++nState] = SIMPLE_COMPLEX_INSERT; 
														BEGIN(stackState[nState]);
													}
<DOUBLE_QUOTED_STRING,HEREDOC>\$					{ strcat(buf,"$"); }
<SIMPLE_COMPLEX_INSERT>[^\[-]						{ BEGIN(stackState[--nState]); printf("Found operator \".\" in line %d\n", yylineno); strcat(buf,yytext); }
<SIMPLE_COMPLEX_INSERT>\[							{ 
														printf("Found symbol \"[\" in line %d\n",yylineno);
														
														stackState[++nState] = SIMPLE_SQUARE_BRACKETS; 
														BEGIN(stackState[nState]);
													}
<SIMPLE_COMPLEX_INSERT>->{ID}						{
														int curline = yylineno;
														printf("Found symbol \"->\" in line %d\n",curline);
														printf("Found identifier \"%s\" in line %d\n",yytext+2,curline);
														printf("Found operator \".\" in line %d\n",yylineno);
														BEGIN(stackState[--nState]);
													}
<SIMPLE_COMPLEX_INSERT>[-]						    { BEGIN(stackState[--nState]); printf("Found operator \".\" in line %d\n", yylineno); strcat(buf,yytext); }
<DOUBLE_QUOTED_STRING,HEREDOC>(\{\$)|(\$\{) 		{ 
														int curline = yylineno; 
														printf("Found string literal\n\"%s\"\nfrom line %d to line %d\n", buf, s, curline);
														printf("Found operator \".\" in line %d\n", curline);
														printf("Found operator \"$\" in line %d\n", yylineno);
														buf[0]='\0';

														stackState[++nState] = HARD_COMPLEX_INSERT;
														BEGIN(stackState[nState]);
													}
<HARD_COMPLEX_INSERT>\}								{  }
<DOUBLE_QUOTED_STRING,HEREDOC>\{					{ strcat(buf,yytext); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\\\					{ strcat(buf,"\\"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\n			 		{ strcat(buf,"\n"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\r			 		{ strcat(buf,"\r"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\t					{ strcat(buf,"\t"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\v			 		{ strcat(buf,"\v"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\e			 		{ strcat(buf,"\e"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\f			 		{ strcat(buf,"\f"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\[0-7]{1,3}  		{ single_char_str[0]=(char)strtol(yytext+1,NULL,8); single_char_str[1] = '\0'; strcat(buf,single_char_str);}
<DOUBLE_QUOTED_STRING,HEREDOC>\\x[0-9A-Fa-f]{1,2}   { single_char_str[0]=(char)strtol(yytext+2,NULL,16); single_char_str[1] = '\0'; strcat(buf,single_char_str);}
<DOUBLE_QUOTED_STRING>\\\"			 				{ strcat(buf,"\""); }
<DOUBLE_QUOTED_STRING>\"			 				{ char *res = (char*)malloc(sizeof(char)*1024); strcpy(res,buf); yylval.string_const = res; BEGIN(PHP); buf[0]='\0'; return STRING;}

<DOUBLE_QUOTED_STRING,HEREDOC>\\					{ strcat(buf,"\\"); }
<HEREDOC>\s.*										{ strcat(buf,yytext); }




\<\?([pP][hH][pP])?									{ BEGIN(PHP); return START_TAG; }
<PHP>\?\>											{ 
														BEGIN(INITIAL);	return END_TAG;
														
													}
<PHP><<EOF>>										{ //BEGIN(stackState[--nState]); 
														BEGIN(INITIAL);	return END_TAG; }

<HTML_COMMENT,MULTILINE_COMMENT,BEGDOC,NOWDOC,HEREDOC,SINGLE_QUOTED_STRING,DOUBLE_QUOTED_STRING,SIMPLE_COMPLEX_INSERT><<EOF>>			{ BEGIN(INITIAL); printf("ERROR! Unexpected end of file!"); }

<PHP>[cC][oO][nN][sS][tT]						{ return CONST; }
<PHP>[cC][lL][aA][sS][sS]						{ return CLASS; }
<PHP>[eE][xX][tT][eE][nN][dD][sS]				{ return EXTENDS; }
<PHP>[pP][uU][bB][lL][iI][cC]					{ return PUBLIC; }
<PHP>[pP][rR][oO][tT][eE][cC][tT][eE][dD]		{ return PROTECTED; }
<PHP>[pP][rR][iI][vV][aA][tT][eE]				{ return PRIVATE; }
<PHP>[nN][eE][wW]								{ return NEW; }
<PHP>[aA][sS]									{ return AS; }
<PHP>[aA][nN][dD]								{ return LOGIC_AND_2; }
<PHP>[oO][rR]									{ return LOGIC_OR_2; }
<PHP>[xX][oO][rR]								{ return LOGIC_XOR; }
<PHP>[bB][rR][eE][aA][kK]						{ return BREAK; }
<PHP>[cC][oO][nN][tT][iI][nN][uU][eE]			{ return CONTINUE; }
<PHP>[dD][oO]									{ return DO; }
<PHP>[wW][hH][iI][lL][eE]						{ return WHILE; }
<PHP>[eE][nN][dD][wW][hH][iI][lL][eE]			{ return ENDWHILE; }
<PHP>[fF][oO][rR]								{ return FOR; }
<PHP>[eE][nN][dD][fF][oO][rR]					{ return ENDFOR; }
<PHP>[fF][oO][rR][eE][aA][cC][hH]				{ return FOREACH; }
<PHP>[eE][nN][dD][fF][oO][rR][eE][aA][cC][hH]	{ return ENDFOREACH; }
<PHP>[eE][cC][hH][oO]							{ return ECHO; }
<PHP>[iI][fF]									{ return IF; }
<PHP>[eE][lL][sS][eE][iI][fF]					{ return ELSEIF; }
<PHP>[eE][lL][sS][eE]							{ return ELSE; }
<PHP>[eE][nN][dD][iI][fF]						{ return ENDIF; }
<PHP>[sS][wW][iI][tT][cC][hH]					{ return SWITCH; }
<PHP>[cC][aA][sS][eE]							{ return CASE; }
<PHP>[dD][eE][fF][aA][uU][lL][tT]				{ return DEFAULT; }
<PHP>[eE][nN][dD][sS][wW][iI][tT][cC][hH]		{ return ENDSWITCH; }
<PHP>[fF][uU][nN][cC][tT][iI][oO][nN]			{ return FUNCTION; }
<PHP>[iI][nN][sS][tT][aA][nN][cC][eE][oO][fF]	{ return INSTANCEOF; }
<PHP>[rR][eE][tT][uU][rR][nN]					{ return RETURN; }
<PHP>[sS][tT][aA][tT][iI][cC]					{ return STATIC; }
<PHP>[vV][aA][rR]								{ return VAR; }
<PHP>bool										{ char *res = (char*)malloc(sizeof(char)*1024); strcpy(res,yytext); yylval.type_const = res; return TYPE; }
<PHP>boolean									{ char *res = (char*)malloc(sizeof(char)*1024); strcpy(res,yytext); yylval.type_const = res; return TYPE; }
<PHP>double										{ char *res = (char*)malloc(sizeof(char)*1024); strcpy(res,yytext); yylval.type_const = res; return TYPE; }
<PHP>int										{ char *res = (char*)malloc(sizeof(char)*1024); strcpy(res,yytext); yylval.type_const = res; return TYPE; }
<PHP>integer									{ char *res = (char*)malloc(sizeof(char)*1024); strcpy(res,yytext); yylval.type_const = res; return TYPE; }
<PHP>float										{ char *res = (char*)malloc(sizeof(char)*1024); strcpy(res,yytext); yylval.type_const = res; return TYPE; }
<PHP>string										{ char *res = (char*)malloc(sizeof(char)*1024); strcpy(res,yytext); yylval.type_const = res; return TYPE; }


<HARD_COMPLEX_INSERT,PHP>"!" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"*" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"**" 						{ return POW; }
<HARD_COMPLEX_INSERT,PHP>"/" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"%" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"+" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"." 						{ return *yytext; }
<PHP>"," 											{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"-" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"<" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"<=" 						{ return LESSER_EQUAL; }
<HARD_COMPLEX_INSERT,PHP>">" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>">=" 						{ return GREATER_EQUAL; }
<HARD_COMPLEX_INSERT,PHP>"==" 						{ return EQUAL; }
<HARD_COMPLEX_INSERT,PHP>"!=" 						{ return NOT_EQUAL_1; }
<HARD_COMPLEX_INSERT,PHP>"<>" 						{ return NOT_EQUAL_2; }
<HARD_COMPLEX_INSERT,PHP>"===" 						{ return IDENTICALLY_EQUAL; }
<HARD_COMPLEX_INSERT,PHP>"!==" 						{ return IDENTICALLY_NOT_EQUAL; }
<HARD_COMPLEX_INSERT,PHP>"&&" 						{ return LOGIC_AND_1; }
<HARD_COMPLEX_INSERT,PHP>"||" 						{ return LOGIC_OR_1; }
<HARD_COMPLEX_INSERT,PHP>"=" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"$" 						{ return *yytext; }

<PHP>";" 											{ return *yytext; }
<PHP>":" 											{ return *yytext; }
<PHP>"?" 											{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"(" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>")" 						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"{" 						{ cntBracket++; return *yytext; }
<HARD_COMPLEX_INSERT,PHP>"}" 						{ return *yytext;
									//if(stackState[nState] == HARD_COMPLEX_INSERT && cntBracket == 0)
									//{
									//	printf("Found operator \".\" in line %d\n",  yylineno);

									//	--nState;
									//	BEGIN(stackState[--nState]);
									//}
									//else 
									//{
									//	printf("Found symbol \"%s\" in line %d\n", yytext, yylineno);
									//	cntBracket--;
									//	if(cntBracket < 0) {
									//		printf("Error Brackets subs");
									//	}
									//}
								}
<HARD_COMPLEX_INSERT,PHP>"["						{ return *yytext; }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>"]" 						{  return *yytext;
									//printf("Found symbol \"%s\" in line %d\n", yytext, yylineno);
														
									//if(stackState[nState - 1] == SIMPLE_COMPLEX_INSERT){
									//	printf("Found operator \".\" in line %d\n", yylineno);

									//	--nState;
									//	BEGIN(stackState[--nState]);
									//}
								}
<HARD_COMPLEX_INSERT,PHP>"::" 						{ return STATIC_PROPERTY_ACCESS; }
<HARD_COMPLEX_INSERT,PHP>"->"						{ return PROPERTY_ACCESS; }
<HARD_COMPLEX_INSERT,PHP>"=>" 						{ return KEY_ACCESS; }

<HARD_COMPLEX_INSERT,PHP>{TRUE}						{ yylval.int_const = 1; return BOOL; }
<HARD_COMPLEX_INSERT,PHP>{FALSE}					{ yylval.int_const = 0; return BOOL; }
<HARD_COMPLEX_INSERT,PHP>{NULL}						{ return NIL; }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>{INT_10}				{ yylval.int_const = (int)strtol(yytext,NULL,10); return INT; }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>{INT_16}				{ yylval.int_const = (int)strtol(yytext,NULL,16); return INT; }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>{INT_8}					{ yylval.int_const = (int)strtol(yytext,NULL,8); return INT; }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>{INT_2}					{ yylval.int_const = (int)strtol(yytext+2,NULL,2); return INT; }
<HARD_COMPLEX_INSERT,PHP>{FLOAT}										{ yylval.float_const = atof(yytext); return FLOAT; }
<HARD_COMPLEX_INSERT,PHP>{EXPONENT}										{ yylval.float_const = atof(yytext); return FLOAT; }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>{ID}					{ char *res = (char*)malloc(sizeof(char)*1024); strcpy(res,yytext); yylval.id_const = res; return ID;}

<HARD_COMPLEX_INSERT,SIMPLE_SQUARE_BRACKETS>.							{ printf("Error simple insert %d\n", yylineno); }
<INITIAL,PHP>[ \t\n]+													;
<INITIAL>.*																;

%%