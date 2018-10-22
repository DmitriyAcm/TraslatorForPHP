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
	int cur_state = PHP;
	int cur_complex_state;
	int last_state;
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
<SINGLE_QUOTED_STRING>\'			{ BEGIN(PHP); printf("Found string literal\n\"%s\"\nfrom line %d to line %d\n", buf, s, yylineno); }

<PHP>"<<<"              			{ BEGIN(BEGDOC); }
<BEGDOC>[ \t]*            			;
<BEGDOC>(\'{ID}\'|{ID})[^\n]        { BEGIN(PHP); printf("ERROR! Wrong doc literal!\n"); }
<BEGDOC>\'{ID}\'\n          		{ BEGIN(NOWDOC); cur_state = NOWDOC; buf[0]='\0'; stringID[0]='\0'; strcpy(stringID,yytext+1); stringID[strlen(stringID)-2]='\0'; s = yylineno; }
<BEGDOC>{ID}\n            			{ BEGIN(HEREDOC); cur_state = HEREDOC; buf[0]='\0'; stringID[0]='\0'; strcpy(stringID,yytext); stringID[strlen(stringID)-1]='\0'; s = yylineno; }

<NOWDOC,HEREDOC>^{ID};\n			{
										char check_end[1000] = "";
										strncat(check_end, yytext, strcspn(yytext,";"));
										if (strcmp(check_end,stringID)==0)
										{
											BEGIN(PHP);
											buf[strlen(buf)-1] = '\0';
											printf("Found string literal\n\"%s\"\nfrom line %d to line %d\n", buf, s, yylineno);					
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

<HEREDOC>[^\\\$\{]									{ strcat(buf,yytext); }

<DOUBLE_QUOTED_STRING,HEREDOC>\\\$					{ strcat(buf,"$"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\\\{					{ strcat(buf,"{"); }
<DOUBLE_QUOTED_STRING,HEREDOC>\${ID}				{
														int curline = yylineno;
														printf("Found string literal\n\"%s\"\nfrom line %d to line %d\nFound operator \".\" in line %d\n", buf, s, curline, curline); 
														buf[0]='\0';

														printf("Found operator \"$\" in line %d\n",curline);
														printf("Found identifier \"%s\" in line %d\n",yytext+1,curline);
														buf[0]='\0';
														last_state = cur_state;
														BEGIN(SIMPLE_COMPLEX_INSERT);
													}
<DOUBLE_QUOTED_STRING,HEREDOC>\$					{ strcat(buf,"$"); }
<SIMPLE_COMPLEX_INSERT>[^\[-]						{ BEGIN(last_state); printf("Found operator \".\" in line %d\n", yylineno); strcat(buf,yytext); }
<SIMPLE_COMPLEX_INSERT>\[							{ 
														printf("Found symbol \"[\" in line %d\n",yylineno);
														cur_complex_state = cur_state;
														cur_state = SIMPLE_SQUARE_BRACKETS;
														BEGIN(cur_state);
													}
<SIMPLE_COMPLEX_INSERT>->{ID}						{
														int curline = yylineno;
														printf("Found symbol \"->\" in line %d\n",curline);
														printf("Found identifier \"%s\" in line %d\n",yytext+2,curline);
														BEGIN(cur_state);
													}
<SIMPLE_COMPLEX_INSERT>[-]						    { BEGIN(last_state); printf("Found operator \".\" in line %d\n", yylineno); strcat(buf,yytext); }
<DOUBLE_QUOTED_STRING,HEREDOC>(\{\$)|(\$\{) 		{ 
														BEGIN(HARD_COMPLEX_INSERT); 
														cur_complex_state = cur_state; 
														cur_state = HARD_COMPLEX_INSERT; 
														int curline = yylineno; 
														printf("Found string literal\n\"%s\"\nfrom line %d to line %d\nFound operator \".\" in line %d\n", buf, s, curline, curline);

														printf("Found operator \"$\" in line %d\n", yylineno);

														buf[0]='\0';
													}
<HARD_COMPLEX_INSERT>\}								{ cur_state=cur_complex_state; BEGIN(cur_state); printf("Found operator \".\" in line %d\n",  yylineno); }
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
<DOUBLE_QUOTED_STRING>\"			 				{ BEGIN(PHP); printf("Found string literal\n\"%s\"\nfrom line %d to line %d\n", buf, s, yylineno); }

<DOUBLE_QUOTED_STRING,HEREDOC>\\											{ strcat(buf,"\\"); }
<HEREDOC>\s.*										{ strcat(buf,yytext); }




\<\?(php)?										{ BEGIN(PHP); printf("Found start PHP-script in line %d\n", yylineno); }
<PHP>\?\>										{ BEGIN(INITIAL); printf("Found end PHP-script in line %d\n", yylineno); }
<PHP><<EOF>>									{ BEGIN(INITIAL); printf("Found end PHP-script in line %d\n", yylineno); }

<HTML_COMMENT,MULTILINE_COMMENT,BEGDOC,NOWDOC,HEREDOC,SINGLE_QUOTED_STRING,DOUBLE_QUOTED_STRING><<EOF>>			{ BEGIN(INITIAL); printf("ERROR! Unexpected end of file!"); }

<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>const						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>class						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>extends						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>public						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>protected					{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>private						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>new 						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>as							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>and							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>or							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>xor							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>break						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>continue					{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>do							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>while						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>endwhile					{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>for							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>endfor						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>foreach						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>endforeach					{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>echo						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>if							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>elseif						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>else						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>endif						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>switch						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>case						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>default						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>endswitch						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>function					{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>instanceof					{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>return						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>static						{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>var							{ printf("Found keyword \"%s\" in line %d\n", yytext, yylineno); }

<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"++"						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"--"						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"!" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"*" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"*=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"**" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"**=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"/" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"/=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"%" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"+" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"+=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"." 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>".=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"," 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"-" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"-=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"<<" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>">>" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"<<=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>">>=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"<" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"<=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>">" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>">=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"==" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"!=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"<>" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"===" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"!==" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"&&" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"||" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"=" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"$" 						{ printf("Found operator \"%s\" in line %d\n", yytext, yylineno); }

<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>";" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>":" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"?" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"(" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>")" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"{" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"}" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"["							{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"]" 						{ 
																				printf("Found symbol \"%s\" in line %d\n", yytext, yylineno);
														
																				if(cur_state == SIMPLE_SQUARE_BRACKETS){
																					cur_state=cur_complex_state;
																					printf("Found operator \".\" in line %d\n", yylineno);
																					BEGIN(cur_state);
																				}
																			}
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"::" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"->"						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>"=>" 						{ printf("Found symbol \"%s\" in line %d\n", yytext, yylineno); }

<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>{TRUE}						{ printf("Found boolean value TRUE in line %d\n", yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>{FALSE}					{ printf("Found boolean value FALSE in line %d\n", yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>{NULL}						{ printf("Found value NULL in line %d\n", yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>{INT_10}					{ printf("Found int value %d in line %d\n", (int)strtol(yytext,NULL,10), yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>{INT_16}					{ printf("Found int value %d in line %d\n", (int)strtol(yytext,NULL,16), yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>{INT_8}					{ printf("Found int value %d in line %d\n", (int)strtol(yytext,NULL,8), yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>{INT_2}					{ printf("Found int value %d in line %d\n", (int)strtol(yytext+2,NULL,2), yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>{FLOAT}					{ printf("Found float value %f in line %d\n", atof(yytext), yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>{EXPONENT}					{ printf("Found float value %f in line %d\n", atof(yytext), yylineno); }
<SIMPLE_SQUARE_BRACKETS,HARD_COMPLEX_INSERT,PHP>{ID}						{ printf("Found identifier \"%s\" in line %d\n", yytext, yylineno); }

<SIMPLE_SQUARE_BRACKETS,INITIAL,HARD_COMPLEX_INSERT,PHP>[ \t\n]+			;
<INITIAL>.*										;

%%