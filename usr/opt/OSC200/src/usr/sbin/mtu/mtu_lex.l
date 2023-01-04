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
 * @(#)$RCSfile: mtu_lex.l,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:18:36 $
 */
%}
%{
/*  Title: mtu_lex.l MTU lexical rules and actions
 **
 **  Copyright (c) Digital Equipment Corporation, 1991, 1992
 **  All Rights Reserved.  Unpublished rights reserved
 **  under the copyright laws of the United States.
 **  
 **  The software contained on this media is proprietary
 **  to and embodies the confidential technology of 
 **  Digital Equipment Corporation.  Possession, use,
 **  duplication or dissemination of the software and
 **  media is authorized only pursuant to a valid written
 **  license from Digital Equipment Corporation.
 **
 **  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 **  disclosure by the U.S. Government is subject to
 **  restrictions as set forth in Subparagraph (c)(1)(ii)
 **  of DFARS 252.227-7013, or in FAR 52.227-19, as
 **  applicable.
 **
 */

/*
 **++
 **  FACILITY:  Common Agent/DECmcc Framework
 **
 **  MODULE DESCRIPTION:
 **
 ** Lex Lexical rules and actions for (CA_)MTU program.
 **
 **      LEX Reference information:
 **      ULTRIX-32 Supplementary Documents - Programmer 
 **      "LEX - A lexical analyzer Generator" by Lesk and Schmidt.
 **
 **      Lex comprises (1) lexical analyzer code generator and 
 **      (2) runtime library (-ll).  The generated code is designed
 **      to complement YYACC.
 **
 **      The generated module (lex.yy.c) consists of a well-known routine --
 **             int yylex()
 **             if returns  0, then EOF; else <token-number> and yylval contains the <token-value>
 **      
 **      yylex() interfaces:
 **             yylex() consumes the input stream (yyin), and produces two (2) output streams
 **             (1) filtered stream of lexical units (yyout), and (2) a stream on tokens (numbers and values)
 **             which are returned to the yylex caller.
 **
 **       The lexical rules are expressed as "regular expressions".
 **       The default rule is to merely copy the input string to the ouput stream.
 **       The explicit actions are specified to perform special functions.
 **         (eg. generating token numbers and values for yyacc, transformation of input string,...)
 **
 **       Special yylex variables :
 **       char yytext[YYLMAX] - current string
 **       int yyleng          - length of current string
 **       int yylineno        - current line number of input file
 **       
 **   DEBUGGING HINTS:
 **   With cc -DLEXDEBUG lookup() is expanded to generate debug code
 **   dependent on the run-time value of the variable "debug".
 **   Unfortunately on UTLRIX the variable is a stack variable, one must 
 **   edit the generated code, making 'debug' a static variable. 
 **   (But even then I still experienced problems).
 **
 **   MTU_LEX recognizes two special debugging directives which may be appropiately inserted(editted)
 **   into the input file:  
 **        %BREAK% <string> - Issue a break-point request to dbx/debug-32/...
 **        %TRACE% <string> - Write a "trace-msg" to stdout
 **
 **  AUTHORS:
 **
 **      Rahul Bose
 **
 **  CREATION DATE:  1-March-1991
 **
 **  MODIFICATION HISTORY:
 **
 ** 01	  01-March-1991	Rahul Bose      Created (for DECmcc)
 ** 02    06-Aug-1992   Pete Burgess    Modified for CA to make convention integration with YACC
 ** 03    13-Sep-1992   Pete Burgess    Modifications and documentation for CAU t1.0.1 
 **                                      o c89 -std compliance, simplification of lex rules and actions
 **                                      o debugging directives: %BREAK and %TRACE%
 *************************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mtu.h"
#include "mtu_yacc.h"

extern int log_pageno;
extern char log_header_str1[PAGE_WIDTH], log_header_str2[PAGE_WIDTH];
extern T_CMDLINE_OPTION cmdline_table[N_CMDLINE_OPTIONS]; 

#define yywrap() 1

/* Extend size of yytext token value buffer */
#ifdef YYLMAX
# undef YYLMAX
#endif
#define YYLMAX 2048

/*
 ** Redefine ECHO macro
 */
#ifdef ECHO
# undef ECHO
# define ECHO if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf(yyout, "%s",yytext)
#endif

/* 
** Redefine output macro to display line and page information
*/
#ifdef output
# undef output
# define output(c) {\
  if (cmdline_table[OPT_LISTFILE].is_enabled) { \
    putc(c,yyout);\
      if(c=='\n'){\
        if(yylineno%PAGE_SIZE==0) {\
          fprintf(yyout,"\f\n%s Page %2d\n\n", log_header_str1, ++log_pageno);\
          fprintf(yyout, log_header_str2);\
          } \
        fprintf(yyout,"%4d\t",yylineno); \
      } \
    } \
  }
#endif

/*
 * External function declarations
 */
extern int yylook(), yyerror(), yyback();
extern long yylval;
char token_value [SIZE_OF_IDENTIFIER];
char quoted_string_value [SIZE_OF_DESCRIPTION+1];
%}

/**************** Lexical rules *********************************************/
%%     
"--"(.)*$ {
/*
 ** Filter out <comment> from the token stream
 **	Match string (from "-- " to end of line)
 */
  ECHO; 
  }

\" {
/*
 ** Recognize Quoted-string token 
 ** write lexical string to output stream (yyout), and write token name to token stream (YACC).
 */
  char ch, prev_ch = 0;
  int count=0;

  ECHO;
 
  do {
    ch = input();
    output(ch);
    if (count < sizeof(quoted_string_value)) {
      if (ch == '\f') ch = '\n'; /* Replace \f with \n */
      if (ch == '\t') ch = ' '; /* Replace \t with space char */
      if (!((ch == ' ') && (prev_ch == ' '))) 
	prev_ch = quoted_string_value[count++] = ch;
    }
    else {
      if (ch == 0) return (0);
    }
  }
  while (ch != '\"');
  quoted_string_value[--count] = NUL;
  yylval = (long)quoted_string_value;
  return (QUOTEDSTRING);
}

DEFINITIONS  {ECHO; return(KW_DEFINITIONS);}
BEGIN  {ECHO; return(KW_BEGIN);}
END  {ECHO; return(KW_END);}
IMPORTS   {ECHO; return(KW_IMPORTS);}
FROM   {ECHO; return(KW_FROM);}
OBJECT {ECHO; return(KW_OBJECT);}
IDENTIFIER  {ECHO; return(KW_IDENTIFIER);}
OBJECT"-"TYPE   {ECHO; return(KW_OBJECTTYPE);}
SYNTAX   {ECHO; return(KW_SYNTAX);}
ACCESS   {ECHO; return(KW_ACCESS);}
STATUS   {ECHO; return(KW_STATUS);}
INTEGER   {ECHO; return(KW_INTEGER);}
OCTET    {ECHO; return(KW_OCTET);}
STRING   {ECHO; return(KW_STRING);}
DisplayString {ECHO; return(KW_DisplayString);}
SIZE     {ECHO; return(KW_SIZE);}
SEQUENCE {ECHO; return(KW_SEQUENCE);}
OF       {ECHO; return(KW_OF);}
NetworkAddress {ECHO; return (KW_NETWORKADDRESS);}
Counter {ECHO; return(KW_COUNTER);}
Gauge {ECHO; return(KW_GAUGE);}
TimeTicks {ECHO; return(KW_TIMETICKS);}
Opaque {ECHO; return(KW_OPAQUE);}
IpAddress {ECHO; return(KW_IPADDRESS);}
read"-"only   {ECHO; return(KW_readonly);}
read"-"write  {ECHO; return(KW_readwrite);}
write"-"only  {ECHO; return(KW_writeonly);}
not"-"accessible   {ECHO; return(KW_notaccessible);}
non"-"zero {ECHO; return(KW_nonzero);}
mandatory   {ECHO; return(KW_mandatory);}
optional  {ECHO; return(KW_optional);}
obsolete  {ECHO; return(KW_obsolete);}
deprecated {ECHO; return(KW_deprecated);}
DESCRIPTION {ECHO; return(KW_DESCRIPTION);}
REFERENCE {ECHO; return(KW_REFERENCE);}
INDEX {ECHO; return(KW_INDEX);}
OPTIONAL {ECHO; return(KW_OPTIONAL_CAPS);}
DEFVAL {ECHO; return(KW_DEFVAL);} 
NULL {ECHO; return (KW_NULL);}
TRAP"-"TYPE   {ECHO; return(KW_TRAPTYPE);}
ENTERPRISE {ECHO; return(KW_ENTERPRISE);}
VARIABLES {ECHO; return(KW_VARIABLES);}
PhysAddress {ECHO; return(KW_PhysAddress);}
::= {ECHO; return(EQ);}
\{ {ECHO; return(LEFT_CURL);}
\} {ECHO; return(RIGHT_CURL);}
\( {ECHO; return(LEFT_BRT);}
\) {ECHO; return(RIGHT_BRT);}
\, {ECHO; return(COMMA);}
\; {ECHO; return(S_COLON);}
".." {ECHO; return(DOUBLE_DOT);}
"." {ECHO; return(SINGLE_DOT);}

[0-9]+   { 
/*
 ** Recognize <number> token,
 ** Write string to output stream (yyout), and
 ** write token name and value to token stream (YACC).
 */
    ECHO; 
#if defined(sun) || defined(sparc)
    yylval = (long) strtol((char *)yytext,0,10);
#else
    yylval = strtoul((char *)yytext,0,10);
#endif
    return(NUMB);
    }

[A-Za-z\'][A-Za-z0-9_\-\']*   { 
/*
 ** Recognize <identifier> token,
 ** Write string to output stream (yyout), and
 ** write token name and value to token stream (YACC).
 **	insure token size is within limits,
 **	convert '-' to '_'
 */
    char *ptr;

    ECHO; 
    ptr = token_value;
    if (yyleng <= sizeof(token_value))
      strcpy(ptr, (char *)yytext);
    else {
      strncpy(ptr, (char *)yytext, sizeof(token_value));
      *(ptr+sizeof(token_value)) = NUL;
    }
    while ((ptr = strchr(ptr, '-'))) *ptr = '_';
    yylval = (long)token_value;
    return(IDENT);
    }

"%BREAK%"(.)*$ {
/*
 ** Recognize %BREAK% token
 ** o Report directive
 ** o Break to debugger
 */
  fprintf (stdout, "\nLEX Break (line %d), (%s)\n", yylineno, yytext);
  DEBUG_BREAK;
}

"%TRACE%"(.)*$ {
/*
 ** Recognize %TRACE% token, report trace message
 */
  fprintf (stdout, "\nLEX Trace (line %d), (%s)\n", yylineno, yytext);
}
