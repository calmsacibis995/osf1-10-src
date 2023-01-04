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
/********************************************************************/
/*                    Compiler Revisions									  */
/*																						  */
/*  init   Date        Revision/Changes                             */
/*	 ----   ----        ----------------									  */
/*  bsb   9/7/90       1.0	/ Initial general customer release       */
/********************************************************************/
/*                    error.c Revisions									  */
/*																						  */
/*  init   Date        Revision/Changes									  */
/*  ----   ----        -----------------									  */
/*  bsb   9/7/90       1.0 / Initial general customer release       */
/********************************************************************/


/*
    error.c - Error handling routines for the scripts compiler

    When YACC detects an error, it calls the routine yyerror()
    with one argument, the string "syntax error".  
*/

#include "scsi.h"

int    ErrorCount   = 0;
int    WarningCount = 0;

void error_msg(s,ErrorNumber,aux)
char *s;
int ErrorNumber;
char *aux;
{
    switch(ErrorNumber) {
	CASE(NO_INPUT):
	    sprintf(s,"No input file specified");
            break;
	CASE(CANT_READ):
	    sprintf(s,"Unable to open input file %s",aux);
	    break;
	CASE(MALLOC_ERR):
	    sprintf(s,"Unable to allocate memory");
	    break;
	CASE(CANT_WRITE):
	    sprintf(s,"Unable to write to output file %s",aux);
            break;
	CASE(MASK_WO_DATA):
	    sprintf(s,"MASK field specified without compare data");
	    break;
	CASE(MULTIPLE_ACK):
	    sprintf(s,"ACK specified multiple times");
	    break;
	CASE(MULTIPLE_ATN):
	    sprintf(s,"ATN specified multiple times");
            break;
	CASE(MULTIPLE_TARGET):
	    sprintf(s,"TARGET specified multiple times");
	    break;
	CASE(MULTIPLE_PHASE):
	    sprintf(s,"PHASE specified multiple times");
	    break;
	CASE(MULTIPLE_DATA):
	    sprintf(s,"DATA specified multiple times");
	    break;
	CASE(MULTIPLE_MASK):
	    sprintf(s,"MASK specified multiple times");
	    break;
	CASE(MULTIPLE_DECLARATION):
	    sprintf(s,"Variable %s declared multiple times",aux);
            break;
	CASE(ORIGIN_UNDEFINED):
	    sprintf(s,"Unable to resolve expression for origin");
	    break;
	CASE(COUNT24_RANGE):
	    sprintf(s,"Count value (24-bit) is out of range");
            break;
	CASE(ADDR24_RANGE):
	    sprintf(s,"Address value (24-bit) is out of range");
            break;
	CASE(ID_RANGE):
	    sprintf(s,"ID value has more than one bit set");
	    break;
	CASE(DATA_RANGE):
	    sprintf(s,"Data value (8-bit) is out of range");
	    break;
	CASE(REGISTER_RANGE):
	    sprintf(s,"Register number is out of range (0-63)");
	    break;
	CASE(IDENT_TOO_LONG):
	    sprintf(s,"Identifier is too long, truncated to %d characters",MAX_IDENT_LEN);
	    break;
	CASE(ATN_AND_PHASE):
	    sprintf(s,"ATN and PHASE match specified");
            break;
	CASE(SYNTAX_ERROR):
	    sprintf(s,"Improper syntax");
	    break;
	CASE(REG_IO_ILLEGAL):
	    sprintf(s,"Illegal combination of registers");
	    break;
	CASE(UNRESOLVED):
	    sprintf(s,"%s not resolved to known value by compile pass",aux);
            break;
	CASE(UNEXPECTED_CHAR):
	    sprintf(s,"Unexpected character in input stream");
	    break;
	CASE(BAD_CONSTANT):
	    sprintf(s,"Constant syntax incorrect");
            break;
	CASE(CONSTANT_OVERFLOW):
	    sprintf(s,"Constant more than 32 bits");
	    break;
	CASE(ENTRY_NOT_LABEL):
	    sprintf(s,"Declared entry point %s is not a label",aux);
	    break;
	CASE(RELATIVE_EXTERN):
	    sprintf(s,"Attempt to use external variable in relative mode");
	    break;
	CASE(MISMATCH_BRACKETS):
	    sprintf(s,"Mismatched brackets in pass parameter");
	    break;
	default:
	    sprintf(s,"Unknown error type.  See NCR");
	    break;
    }
}


void error(ErrorNumber,aux)
UINT32 ErrorNumber;
char *aux;
{
    char *msg;
    int i;
    int compile_pass;
    char s[128];

    error_msg(s,(int)(ErrorNumber & 0xffff),aux);

    if (ErrorNumber & CRITICAL) {
	if (err_flag)
	      fprintf(errfile,"ERROR - %s.\nAborting...\n",s);
	fprintf(stderr,"ERROR - %s.\nAborting ...\n",s);
        exit( (int) (ErrorNumber & 0xff) );
    }

    if (compile_flag)
        compile_pass = 1;
    else if (iteration_num == 1 && (ErrorNumber & FIRST_PASS))
        compile_pass = 0;
    else
        return;

    if (ErrorNumber & ERROR) {
        ErrorCount++;
        msg = "ERROR  ";
    }
    else {
        WarningCount++;
        msg = "WARNING";
    }

    if (ErrorNumber & SHELL) {
	if (err_flag)
	      fprintf(errfile,"%s - %s.\n",msg,s);
	fprintf(stderr,"%s - %s.\n",msg,s);     /* Print to stderr only */
    }
    else if (ErrorNumber & LEX) {
      
	/* Always warn on stderr */
	if (err_flag)
	      fprintf(errfile,"%s - %s in line %d, character %d.\n",msg,s,(int)line_no,ErrorChar+1);
	fprintf(stderr,"%s - %s in line %d, character %d.\n",msg,s,(int)line_no,ErrorChar+1);

        /* Also warn in .lis file */
        if (lis_flag) {
            if (compile_pass) {
                fprintf(lisfile,"%s - %s in following line.\n",msg,s);
                for (i=0; i<ErrorChar; i++)
                    fputc('-',lisfile);
                fprintf(lisfile,"v\n");
            }
            else {
                fprintf(lisfile,"%s - %s in line %d, character %d.\n",msg,s,(int)line_no,ErrorChar+1);
            }
        }
    }
    else {
	/* Always warn on stderr */
	if (err_flag)
	      fprintf(errfile,"%s - %s in line %d.\n",msg,s,(int)line_no);
	fprintf(stderr,"%s - %s in line %d.\n",msg,s,(int)line_no);

        /* Also warn in .lis file */
        if (lis_flag) {
            if (compile_pass) {
                fprintf(lisfile,"%s - %s in following line.\n",msg,s);
            }
            else {
                fprintf(lisfile,"%s - %s in line %d.\n",msg,s,(int)line_no);
            }
        }
    }
}


yyerror(s)
char *s;
{
    error(SYNTAX_ERROR,s);
    return 0;
}
