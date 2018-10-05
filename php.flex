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
%x COMPLEX_INSERT
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
	int cur_state;
%}


"<!--"								{ BEGIN(HTML_COMMENT); }
<HTML_COMMENT>[^\-]+				;
<HTML_COMMENT>"-"					;
<HTML_COMMENT>\-{2}\>				{ BEGIN(INITIAL); }

<PHP>("//"|"#").*       			;

<PHP>"/*"							{ BEGIN(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>[^*]*			;
<MULTILINE_COMMENT>"*"+[^*/]* 		;
<MULTILINE_COMMENT>"*"+"/" 			{ BEGIN(PHP); }

<PHP>\'								{ BEGIN(SINGLE_QUOTED_STRING); buf[0] = '\0'; s = yylineno; }
<SINGLE_QUOTED_STRING>\\\'			{ strcat(buf, "\'"); }
<SINGLE_QUOTED_STRING>\\\\     		{ strcat(buf, "\\"); }
<SINGLE_QUOTED_STRING>\\[^\\\']     { strcat(buf, yytext); }
<SINGLE_QUOTED_STRING>[^\\\']+		{ strcat(buf, yytext); }
<SINGLE_QUOTED_STRING>\'			{ BEGIN(PHP); printf("Found single quoted literal \"%s\" from line %d to line %d\n", buf, s, yylineno); }

<PHP>"<<<"              			{ BEGIN(BEGDOC); }
<BEGDOC>[ \t]*            			;
<BEGDOC>(\'{ID}\'|{ID})[^\n]        { BEGIN(PHP); printf("ERROR! Wrong doc literal!\n"); }
<BEGDOC>\'{ID}\'\n          		{ BEGIN(NOWDOC); cur_state = NOWDOC; buf[0]='\0'; stringID[0]='\0'; strcpy(stringID,yytext+1); stringID[strlen(stringID)-2]='\0'; s = yylineno; }
<BEGDOC>{ID}\n            			{ BEGIN(HEREDOC); cur_state = HEREDOC; buf[0]='\0'; stringID[0]='\0'; strcpy(stringID,yytext); stringID[strlen(stringID)-1]='\0'; s = yylineno; }

<NOWDOC,HEREDOC>^{ID};				{
										yytext[strlen(yytext)-1] = '\0';
										if (strcmp(yytext,stringID)==0)
										{
											BEGIN(PHP);
											buf[strlen(buf)-1] = '\0';
											if (cur_state == HEREDOC) 	{ printf("Found heredoc literal \"%s\" from line %d to line %d\n", buf, s, yylineno); }
											else 						{ printf("Found nowdoc literal \"%s\" from line %d to line %d\n", buf, s, yylineno); }
											printf("Found symbol \";\" in line %d\n", yylineno);
										}
										else
										{
											strcat(buf,yytext);
										}
									}
									
<NOWDOC>\n+							{ strcat(buf,yytext); }
<NOWDOC>[^\s\n]+					{ strcat(buf,yytext); }


<PHP>"\""											{ BEGIN(DOUBLE_QUOTED_STRING); cur_state = DOUBLE_QUOTED_STRING;  buf[0]='\0'; s = yylineno; }
<DOUBLE_QUOTED_STRING>[^\\\"\$\{]+					{ strcat(buf,yytext); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\\$					{ strcat(buf,"$"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\\{					{ strcat(buf,"{"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\{\$\}				{ strcat(buf,"{$}"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\$\{\}				{ strcat(buf,"${}"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\{\}					{ strcat(buf,"{}"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\${ID}				{ 
													  _itoa(yylineno,num,10);
													  strcat(buf,"\nFound operator $ in line ");
													  strcat(buf,num);
													  strcat(buf,"\n"); 
													  
													  strcat(buf,"\nFound begin identifier \"");
													  strcat(buf,yytext+1);
													  strcat(buf,"\" in line ");
													  strcat(buf,num);
													  strcat(buf,"\n"); 
													  num[0]='\0';  
													}
<DOUBLE_QUOTED_STRING,HEREDOC>(\{\$)|(\$\{) 		{ BEGIN(COMPLEX_INSERT); printf("Found start complex insert in literal from line %d\n{\n",  yylineno); }
<COMPLEX_INSERT>\}									{ BEGIN(cur_state); printf("}\n",  yylineno); }
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
<DOUBLE_QUOTED_STRING>\"			 				{ BEGIN(PHP); printf("Found double quoted literal \"%s\" from line %d to line %d\n", buf, s, yylineno); }

			
<HEREDOC>\s.*										{ strcat(buf,yytext); }
<HEREDOC,DOUBLE_QUOTED_STRING>\n+					{ strcat(buf,yytext); }
<HEREDOC>;											{ strcat(buf,yytext); }

\<\?(php)?										{ BEGIN(PHP); printf("Found start PHP-script in line %d\n", yylineno); }
<PHP>\?\>										{ BEGIN(INITIAL); printf("Found end PHP-script in line %d\n", yylineno); }
<PHP><<EOF>>									{ BEGIN(INITIAL); printf("Found end PHP-script in line %d\n", yylineno); }

<HTML_COMMENT,MULTILINE_COMMENT,BEGDOC,NOWDOC,HEREDOC,SINGLE_QUOTED_STRING,DOUBLE_QUOTED_STRING><<EOF>>			{ BEGIN(INITIAL); printf("ERROR! Unexpected end of file!"); }

<COMPLEX_INSERT,PHP>const						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>class						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>extends						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>public						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>protected					{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>private						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>new 						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>as							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>and							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>or							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>xor							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>break						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>continue					{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>do							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>while						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>for							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>foreach						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>echo						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>if							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>elseif						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>else						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>function					{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>return						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>static						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }

<COMPLEX_INSERT,PHP>"++"						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"--"						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"!" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"*" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"**" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"/" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"%" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"+" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"." 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"," 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"-" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"<" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"<=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>">" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>">=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"==" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"!=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"<>" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"===" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"!==" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"&&" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"||" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"$" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }

<COMPLEX_INSERT,PHP>";" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"(" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>")" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"{" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"}" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"[" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"]" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"::" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"->"						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>"=>" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }

<COMPLEX_INSERT,PHP>{TRUE}						{ printf("Found boolean value \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>{FALSE}						{ printf("Found boolean value \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>{NULL}						{ printf("Found value \"%s\" in line %d\n", yytext, yylineno); }
<COMPLEX_INSERT,PHP>{INT_10}					{ printf("Found int value %d in line %d\n", (int)strtol(yytext,NULL,10), yylineno); }
<COMPLEX_INSERT,PHP>{INT_16}					{ printf("Found int value %d in line %d\n", (int)strtol(yytext,NULL,16), yylineno); }
<COMPLEX_INSERT,PHP>{INT_8}						{ printf("Found int value %d in line %d\n", (int)strtol(yytext,NULL,8), yylineno); }
<COMPLEX_INSERT,PHP>{INT_2}						{ printf("Found int value %d in line %d\n", (int)strtol(yytext,NULL,2), yylineno); }
<COMPLEX_INSERT,PHP>{FLOAT}						{ printf("Found float value %f in line %d\n", atof(yytext), yylineno); }
<COMPLEX_INSERT,PHP>{EXPONENT}					{ printf("Found float value %f in line %d\n", atof(yytext), yylineno); }
<COMPLEX_INSERT,PHP>{ID}						{ printf("Found identifier \"%s\" in line %d\n", yytext, yylineno); }

<INITIAL,COMPLEX_INSERT,PHP>[ \t\n]+			;
<INITIAL>.*										;

%%