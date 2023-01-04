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
#ifndef lint
static char     *sccsid = "@(#)$RCSfile: err.c,v $ $Revision: 1.1.5.6 $ (DEC) $Date: 1993/12/20 21:30:52 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1.1
 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 * FUNCTIONS: error, diag_error, safe_malloc, yyerror
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.11  com/cmd/nls/err.c, cmdnls, bos320, 9132320m 8/11/91 14:08:39
 */

#include <stdio.h>
#include <stdarg.h>
#include <nl_types.h>
#include <stdlib.h>
#include "err.h"


/* This is the default message text array for localedef error messages */
static char *err_fmt[]={
"",
"The symbol '%s' is not the correct type.\n",
"Could not open '%s' for read.\n",
"Internal error. [file %s - line %d].\n",
"Syntax Error: expected %d arguments and received %d arguments.\n",
"Illegal limit in range specification.\n",
"Memory allocation failure: [line %d -- module %s].\n",
"Could not open temporary file '%s' for write.\n",
"The '%s' character is longer than <mb_cur_max>.\n",
"The '%s' character is undefined.\n",
"The start of the range must be numerically less than the\n\
 end of the range.\n",
"The symbol range containing %s and %s is incorrectly formatted.\n",
"Illegal character reference or escape sequence in '%s'.\n",
"You have specified different names for the same character '%d'.\n",
"The character symbol '%s', has already been specified.\n",
"There are characters in the codeset which are unspecified\n\
 in the collation order.\n",
"Illegal decimal constant '%s'.\n",
"Illegal octal constant '%s'.\n",
"Illegal hexadecimal constant '%s'.\n",
"Missing closing quote in string '%s'.\n",
"Illegal character, '%c', in input file.  Character will be ignored.\n",
"Character for escape_char statement missing.  Statement ignored.\n",
"Character for <comment_char> statement missing.  Statement ignored.\n",
"'%c' is not a POSIX Portable Character. Statement ignored.\n",
"The character symbol'%s' is missing the closing '>'. The '>'\
 will be added.\n",
"Unrecognized keyword, '%s', statement ignored.\n",
"The character symbol '%s' is undefined in the range '%s...%s'.\n\
 The symbol will be ignored.\n",
"The encoding specified for the '%s' character is unsupported.\n",
"The character, '%s', has already been assigned a weight.\n\
 Specification ignored.\n",
"The character %s in range %s...%s already has a collation weight.\
 Range ignored.\n",
"No toupper section defined for this locale sourcefile.\n",
"The use of the \"...\" keyword assumes that the codeset is contiguous\n\
 between the two range endpoints specified.\n",
"The collation substitution,  %s%s, contains a symbol which is not\n\
 a character.\n",
"The symbol, %s, referenced has not yet been specified in the\n\
 collation order.\n",
"usage: localedef [-cvw] [-m methfile] [-f charmap] [-i locsrc] \\\n\t\
 [-C cc opts] [-L ld opts] [-P tool path] locname\n",
"localedef [ERROR]: FILE: %s, LINE: %d, CHAR: %d\n",
"localedef [WARNING]: FILE: %s, LINE: %d, CHAR: %d\n",
"localedef [ERROR]: FILE: %s, LINE: %d, CHAR: %d\n\
 Syntax Error.\n",
"Specific collation weight assignment is not valid when no sort\n\
 keywords have been specified.\n",
"Required header files sys/localedef.h and sys/lc_core.h\
 are the wrong version or are missing.\\\n",
"The <mb_cur_min> keyword must be defined as 1, you have\
 defined it \nas %d.\n",
"The <code_set_name> must contain only characters from the\
 portable \ncharacter set, %s is not valid.\n",
"The collation directives forward and backward are mutually\n\
exclusive.\n",
"Received too many arguments, expected %d.\n",
"The %s category has already been defined.\n",
"The %s category is empty.\n",
"Unrecognized category %s is not processed by localedef.\n",
"The POSIX defined categories must appear before any unrecognized\
 categories.\n",
"The file code for the digit %s is not one greater than the \n\
file code for %s.\n",
"The process code for the digit %s is not one greater than the \n\
process code for %s.\n",
"The symbol %s has already been defined. Ignoring definition as a \n\
collating-symbol.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE upper \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE lower \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE alpha \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE space \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE cntrl \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE punct \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE graph \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE print \nkeyword.\n",
"Locale does not specify the minimum required for the LC_CTYPE\
 upper keyword.\n",
"Locale does not specify the minimum required for the LC_CTYPE\
 lower keyword.\n",
"Locale does not specify the minimum required for the LC_CTYPE\
 space keyword.\n",
"Locale does not specify only '0', '1', '2', '3', '4', '5',\
 '6', '7', '8',\nand '9' for LC_CTYPE digit keyword.\n",
"Locale does not specify only '0', '1', '2', '3', '4', '5',\
 '6', '7', '8',\n'9', 'a' through 'f', and 'A' through 'F' for LC_CTYPE\
 xdigit keyword.\n",
"The number of operands to LC_COLLATE order exceeds COLL_WEIGHTS_MAX.\n",
"Unable to exec /usr/bin/sh to process localedef intermediate files.\n",
"Unable to load extension method for %s from file %s.\n",
"%s method must be defined explicitly.\n",
"Unable to load locale \"%s\" for copy directive.\n",
"Locale name longer than PATH_MAX (%d).\n",
"The character, '%s', has been assigned too\
 many weights.\nExtra weights ignored.\n",
"\"%s\" was not declared in a charclass statement.\n",
};

/* 
  Global indicating if an error has been encountered in the source or not.
  This flag is used in conjunction with the -c option to decide if a locale
  should be created or not.
*/
int err_flag = 0;
static nl_catd catd = 0;


/*
*  FUNCTION: usage
*
*  DESCRIPTION:
*  Prints a usage statement to stderr and exits with a return code 'status'
*/
void usage(int status)
{
    if (catd == NULL)
	catd = catopen("localedef.cat", NL_CAT_LOCALE);

    fprintf(stdout, 
	    catgets(catd, LOCALEDEF, ERR_USAGE, err_fmt[ERR_USAGE]));

    exit(status);
}


/*
*  FUNCTION: error
*
*  DESCRIPTION:
*  Generic error routine.  This function takes a variable number of arguments
*  and passes them on to vprintf with the error message text as a format.
*
*  Errors from this routine are fatal.
*/
void error(int err, ...)
{
    extern int yylineno, yycharno;
    extern char *yyfilenm;
    va_list ap;
    va_start(ap, err);
    
    if (catd==NULL)
	catd = catopen("localedef.cat", NL_CAT_LOCALE);

    fprintf(stderr, 
	    catgets(catd, LOCALEDEF, ERR_ERROR, err_fmt[ERR_ERROR]),
	    yyfilenm, yylineno, yycharno);

    vfprintf(stderr, catgets(catd, LOCALEDEF, err, err_fmt[err]), ap);
    
    if (err == ERR_INTERNAL)
      exit(2);			/* Locale might be too large, etc... */

    exit(4);
}


/*
*  FUNCTION: diag_error
*
*  DESCRIPTION:
*  Generic error routine.  This function takes a variable number of arguments
*  and passes them on to vprintf with the error message text as a format.
*
*  Errors from this routine are considered non-fatal, and if the -c flag
*  is set will still allow generation of a locale.
*/
void diag_error(int err, ...)
{
    extern int yylineno, yycharno;
    extern char *yyfilenm;
    va_list ap;
    va_start(ap, err);
    
    if (err != ERR_CODESET_DEP) /* Set err_flag only if not "..." range err */
	err_flag++;
    
    if (catd==NULL)
	catd = catopen("localedef.cat", NL_CAT_LOCALE);

    fprintf(stderr, 
	    catgets(catd, LOCALEDEF, ERR_WARNING, err_fmt[ERR_WARNING]),
	    yyfilenm, yylineno, yycharno);

    vfprintf(stderr, catgets(catd, LOCALEDEF, err, err_fmt[err]), ap);
}


/*
*  FUNCTION: yyerror
*
*  DESCRIPTION:
*  Replacement for the yyerror() routine.  This is called by the yacc 
*  generated parser when a syntax error is encountered.
*
*  Syntax errors are considered fatal.
*/
void yyerror(const char *s)
{
    extern int yylineno, yycharno;
    extern char *yyfilenm;

    if (catd==NULL)
	catd = catopen("localedef.cat", NL_CAT_LOCALE);

    fprintf(stderr,
	    catgets(catd, LOCALEDEF, ERR_SYNTAX, err_fmt[ERR_SYNTAX]),
	    yyfilenm, yylineno, yycharno);

    exit(4);
}


/*
*  FUNCTION: safe_malloc
*
*  DESCRIPTION:
*  Backend for the MALLOC macro which verifies that memory is available.
*
*  Out-of-memory results in a fatal error.
*/
void * safe_malloc(size_t bytes, const char *file, int lineno)
{
  void * p;

  p = calloc(bytes, 1);
  if (p == NULL)
    error(ERR_MEM_ALLOC_FAIL, file, lineno);

  return p;
}



/*
 * msgstr - returns a string to the message
 */

char * msgstr(int n)
{
    return catgets(catd, LOCALEDEF, n, err_fmt[n]);
}

    
