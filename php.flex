%{
	#include <math.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include <conio.h>
	#include <locale.h> 
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

	int stackState[100000];
	
	stackState[0] = INITIAL;
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

<PHP>\'								{ stackState[++nState] = SINGLE_QUOTED_STRING; BEGIN(stackState[nState]); buf[0] = '\0'; s = yylineno; }
<SINGLE_QUOTED_STRING>\\\'			{ strcat(buf, "\'"); }
<SINGLE_QUOTED_STRING>\\\\     		{ strcat(buf, "\\"); }
<SINGLE_QUOTED_STRING>\\[^\\\']     { strcat(buf, yytext); }
<SINGLE_QUOTED_STRING>[^\\\']+		{ strcat(buf, yytext); }
<SINGLE_QUOTED_STRING>\'			{ BEGIN(stackState[--nState]); printf("Found string literal\n\"%s\"\nfrom line %d to line %d\n", buf, s, yylineno); buf[0]='\0'; }

<PHP>"<<<"              			{ stackState[++nState] = BEGDOC; BEGIN(stackState[nState]); }
<BEGDOC>[ \t]*            			;
<BEGDOC>(\'{ID}\'|{ID})[^\n]        { BEGIN(stackState[--nState]); printf("ERROR! Wrong doc literal!\n"); }
<BEGDOC>\'{ID}\'\n          		{ stackState[nState] = NOWDOC; BEGIN(stackState[nState]);  buf[0]='\0'; stringID[0]='\0'; strcpy(stringID,yytext+1); stringID[strlen(stringID)-2]='\0'; s = yylineno; }
<BEGDOC>{ID}\n            			{ stackState[nState] = HEREDOC; BEGIN(stackState[nState]);  buf[0]='\0'; stringID[0]='\0'; strcpy(stringID,yytext); stringID[strlen(stringID)-1]='\0'; s = yylineno; }

<NOWDOC,HEREDOC>^{ID};\n			{
										char check_end[1000] = "";
										strncat(check_end, yytext, strcspn(yytext,";"));
										if (strcmp(check_end,stringID)==0)
										{
											BEGIN(stackState[--nState]);
											buf[strlen(buf)-1] = '\0';
											printf("Found string literal\n\"%s\"\nfrom line %d to line %d\n", buf, s, yylineno);					
											printf("Found symbol \";\" in line %d\n", yylineno);
											buf[0] = '\0';
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
<DOUBLE_QUOTED_STRING>\"			 				{ BEGIN(stackState[--nState]); printf("Found string literal\n\"%s\"\nfrom line %d to line %d\n", buf, s, yylineno); buf[0]='\0'; }

<DOUBLE_QUOTED_STRING,HEREDOC>\\					{ strcat(buf,"\\"); }
<HEREDOC>\s.*										{ strcat(buf,yytext); }




\<\?([pP][hH][pP])?									{ stackState[++nState] = PHP; BEGIN(stackState[nState]); printf("Found start PHP-script in line %d\n", yylineno); }
<PHP>\?\>											{ 
														if(stackState[nState - 1] == INITIAL)
														{
															BEGIN(stackState[--nState]); 
															printf("Found end PHP-script in line %d\n", yylineno);
														}
														else
														{
															printf("Found symbol \"?\" in line %d\n", yytext, yylineno);
															printf("Found symbol \">\" in line %d\n", yytext, yylineno);
														}
													}
<PHP><<EOF>>										{ BEGIN(stackState[--nState]); printf("Found end PHP-script in line %d\n", yylineno); }

<HTML_COMMENT,MULTILINE_COMMENT,BEGDOC,NOWDOC,HEREDOC,SINGLE_QUOTED_STRING,DOUBLE_QUOTED_STRING,SIMPLE_COMPLEX_INSERT><<EOF>>			{ BEGIN(INITIAL); printf("ERROR! Unexpected end of file!"); }

<PHP>[cC][oO][nN][sS][tT]						{ printf("Found keyword \"const\" in line %d\n", yylineno); }
<PHP>[cC][lL][aA][sS][sS]						{ printf("Found keyword \"class\" in line %d\n", yylineno); }
<PHP>[eE][xX][tT][eE][nN][dD][sS]				{ printf("Found keyword \"extends\" in line %d\n", yylineno); }
<PHP>[pP][uU][bB][lL][iI][cC]					{ printf("Found keyword \"public\" in line %d\n", yylineno); }
<PHP>[pP][rR][oO][tT][eE][cC][tT][eE][dD]		{ printf("Found keyword \"protected\" in line %d\n", yylineno); }
<PHP>[pP][rR][iI][vV][aA][tT][eE]				{ printf("Found keyword \"private\" in line %d\n", yylineno); }
<PHP>[nN][eE][wW]								{ printf("Found keyword \"new \" in line %d\n", yylineno); }
<PHP>[aA][sS]									{ printf("Found keyword \"as\" in line %d\n", yylineno); }
<PHP>[aA][nN][dD]								{ printf("Found keyword \"and\" in line %d\n", yylineno); }
<PHP>[oO][rR]									{ printf("Found keyword \"or\" in line %d\n", yylineno); }
<PHP>[xX][oO][rR]								{ printf("Found keyword \"xor\" in line %d\n", yylineno); }
<PHP>[bB][rR][eE][aA][kK]						{ printf("Found keyword \"break\" in line %d\n", yylineno); }
<PHP>[cC][oO][nN][tT][iI][nN][uU][eE]			{ printf("Found keyword \"continue\" in line %d\n", yylineno); }
<PHP>[dD][oO]									{ printf("Found keyword \"do\" in line %d\n", yylineno); }
<PHP>[wW][hH][iI][lL][eE]						{ printf("Found keyword \"while\" in line %d\n", yylineno); }
<PHP>[eE][nN][dD][wW][hH][iI][lL][eE]			{ printf("Found keyword \"endwhile\" in line %d\n", yylineno); }
<PHP>[fF][oO][rR]								{ printf("Found keyword \"for\" in line %d\n", yylineno); }
<PHP>[eE][nN][dD][fF][oO][rR]					{ printf("Found keyword \"endfor\" in line %d\n", yylineno); }
<PHP>[fF][oO][rR][eE][aA][cC][hH]				{ printf("Found keyword \"foreach\" in line %d\n", yylineno); }
<PHP>[eE][nN][dD][fF][oO][rR][eE][aA][cC][hH]	{ printf("Found keyword \"endforeach\" in line %d\n", yylineno); }
<PHP>[eE][cC][hH][oO]							{ printf("Found keyword \"echo\" in line %d\n", yylineno); }
<PHP>[iI][fF]									{ printf("Found keyword \"if\" in line %d\n", yylineno); }
<PHP>[eE][lL][sS][eE][iI][fF]					{ printf("Found keyword \"elseif\" in line %d\n", yylineno); }
<PHP>[eE][lL][sS][eE]							{ printf("Found keyword \"else\" in line %d\n", yylineno); }
<PHP>[eE][nN][dD][iI][fF]						{ printf("Found keyword \"endif\" in line %d\n", yylineno); }
<PHP>[sS][wW][iI][tT][cC][hH]					{ printf("Found keyword \"switch\" in line %d\n", yylineno); }
<PHP>[cC][aA][sS][eE]							{ printf("Found keyword \"case\" in line %d\n", yylineno); }
<PHP>[dD][eE][fF][aA][uU][lL][tT]				{ printf("Found keyword \"default\" in line %d\n", yylineno); }
<PHP>[eE][nN][dD][sS][wW][iI][tT][cC][hH]		{ printf("Found keyword \"endswitch\" in line %d\n", yylineno); }
<PHP>[fF][uU][nN][cC][tT][iI][oO][nN]			{ printf("Found keyword \"function\" in line %d\n", yylineno); }
<PHP>[iI][nN][sS][tT][aA][nN][cC][eE][oO][fF]	{ printf("Found keyword \"instanceof\" in line %d\n", yylineno); }
<PHP>[rR][eE][tT][uU][rR][nN]					{ printf("Found keyword \"return\" in line %d\n", yylineno); }
<PHP>[sS][tT][aA][tT][iI][cC]					{ printf("Found keyword \"static\" in line %d\n", yylineno); }
<PHP>[vV][aA][rR]								{ printf("Found keyword \"var\" in line %d\n", yylineno); }


<HARD_COMPLEX_INSERT,PHP>"++"						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"--"						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"!" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"*" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"*=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"**" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"**=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"/" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"/=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"%" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"+" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"+=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"." 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>".=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<PHP>"," 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"-" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"-=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"<<" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>">>" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"<<=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>">>=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"<" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"<=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>">" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<PHP>">=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"==" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"!=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"<>" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"===" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"!==" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"&&" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"||" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"$" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }

<PHP>";" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<PHP>":" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<PHP>"?" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"(" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>")" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"{" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); cntBracket++;}
<HARD_COMPLEX_INSERT,PHP>"}" 						{ 
									if(stackState[nState] == HARD_COMPLEX_INSERT && cntBracket == 0)
									{
										printf("Found operator \".\" in line %d\n",  yylineno);

										--nState;
										BEGIN(stackState[--nState]);
									}
									else 
									{
										printf("Found symbol \"%s\" in line %d\n", yytext, yylineno);
										cntBracket--;
										if(cntBracket < 0) {
											printf("Error Brackets subs");
										}
									}
								}
<HARD_COMPLEX_INSERT,PHP>"["						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>"]" 						{ 
									printf("Found symbol \"%s\" in line %d\n", yytext, yylineno);
														
									if(stackState[nState - 1] == SIMPLE_COMPLEX_INSERT){
										printf("Found operator \".\" in line %d\n", yylineno);

										--nState;
										BEGIN(stackState[--nState]);
									}
								}
<HARD_COMPLEX_INSERT,PHP>"::" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"->"						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<HARD_COMPLEX_INSERT,PHP>"=>" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }

<HARD_COMPLEX_INSERT,PHP>{TRUE}						{ printf("Found boolean value TRUE in line %d\n", yylineno); }
<HARD_COMPLEX_INSERT,PHP>{FALSE}					{ printf("Found boolean value FALSE in line %d\n", yylineno); }
<HARD_COMPLEX_INSERT,PHP>{NULL}						{ printf("Found value NULL in line %d\n", yylineno); }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>{INT_10}				{ printf("Found int value %d in line %d\n", (int)strtol(yytext,NULL,10), yylineno); }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>{INT_16}				{ printf("Found int value %d in line %d\n", (int)strtol(yytext,NULL,16), yylineno); }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>{INT_8}					{ printf("Found int value %d in line %d\n", (int)strtol(yytext,NULL,8), yylineno); }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>{INT_2}					{ printf("Found int value %d in line %d\n", (int)strtol(yytext+2,NULL,2), yylineno); }
<HARD_COMPLEX_INSERT,PHP>{FLOAT}					{ printf("Found float value %f in line %d\n", atof(yytext), yylineno); }
<HARD_COMPLEX_INSERT,PHP>{EXPONENT}					{ printf("Found float value %f in line %d\n", atof(yytext), yylineno); }
<HARD_COMPLEX_INSERT,PHP,SIMPLE_SQUARE_BRACKETS>{ID}						{ printf("Found identifier \"%s\" in line %d\n", yytext, yylineno); }

<HARD_COMPLEX_INSERT,SIMPLE_SQUARE_BRACKETS>.		{ printf("Error simple insert %d\n", yylineno); }
<INITIAL,PHP>[ \t\n]+			;
<INITIAL>.*										;

%%