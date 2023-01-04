%{
/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
 * @(#)$RCSfile: kr.l,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 11:19:24 $
 */
%}
/*
"..." - one token containing expansion of `...` and \c
'...' - one token containing ...
\c - literal c
`...` - results of evaluating ...
a ; b - evaluate a, then b
a | b - a piped into b
#... - comment
*/

%{
#undef input
#undef unput

#include "krash_p.h"

static char *next_input = NULL;
static char *input_ptr = NULL;
static Boolean added_nl = False;

void *set_input(str)
char *str;
{
  if(input_ptr != NULL){
    if(*input_ptr != NULL)
      fprintf(stderr, "Didn't consume last input\n");
    free(next_input);
  }
  next_input = copy(str, -1);
  input_ptr = next_input;
  added_nl = False;
}

unsigned char input(){
  char c;

  c = *input_ptr;
  if(c != '\0') input_ptr++;
  else if(((input_ptr == next_input) || (*(input_ptr - 1) != '\n')) &&
	  (added_nl == False)){
    c = '\n';
    added_nl = True;
  }
  return(c);
}

void unput(c)
char c;
{
  if(c == '\0') return;
  if(input_ptr == next_input){
    fprintf(stderr, "Tried to back past beginning\n");
    return;
  }
  if((c != '\n') || (added_nl == False)) input_ptr--;
}

%}
%%

;               { yylval = NULL;
                  return(SEMICOLON); }

\|              { yylval = NULL;
                  return(PIPE); }

^\#.*		{ yylval = copy((char *) yytext, yyleng);
                  return(COMMENT); }

\"[^"]* 	{ int i;
                  if(yytext[yyleng-1] == '\\') yymore();
                  else {
                    input();
                    for(i=0;i<yyleng;i++){
                      if((yytext[i] == '\n') &&
                         ((i == 0) || (yytext[i-1] != '\\'))){
                        fprintf(stderr, "Unmatched \".\n");
                        return(ERROR);
                      }
                    }
                    yylval = copy((char *) &yytext[1], yyleng - 1);
                    return(DOUBLE_STRING);
	          }
                }

\'[^']* 	{ int i;
                  if(yytext[yyleng-1] == '\\') yymore();
                  else {
                    input();
                    for(i=0;i<yyleng;i++){
                      if((yytext[i] == '\n') &&
                         ((i == 0) || (yytext[i-1] != '\\'))){
                        fprintf(stderr, "Unmatched \'.\n");
                        return(ERROR);
                      }
                    }
                    yylval = copy((char *) &yytext[1], yyleng - 1);
                    return(SINGLE_STRING);
                  }
	        }

\`[^`]*	        { int i;
                  if(yytext[yyleng-1] == '\\') yymore();
                  else {
                    input();
                    for(i=0;i<yyleng;i++){
                      if((yytext[i] == '\n') &&
                         ((i == 0) || (yytext[i-1] != '\\'))){
                        fprintf(stderr, "Unmatched \`.\n");
                        return(ERROR);
                      }
                    }
                    yylval = copy((char *) &yytext[1], yyleng - 1);
                    return(BACK_QUOTE);
                  }
	        }

[ \t\n]*     	{ if((yytext[yyleng-1] == '\n') && (added_nl == True))
                    yylval = copy((char *) yytext, yyleng - 1);
                  else yylval = copy((char *) yytext, yyleng);
		  return(WHITESPACE);
		}

([^ \t;\|\"\'\`\n]|\\[ \t;\|\"\'\`])* 
                { yylval = copy((char *) yytext, yyleng);
		  return(ID);
		}

\n              { yylval = copy((char *) yytext, yyleng);
                  return(ERROR);
                }
