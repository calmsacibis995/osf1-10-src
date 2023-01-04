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
static char *rcsid = "@(#)$RCSfile: asa.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/11 20:04:54 $";
#endif
/*
 * COMPONENT_NAME : asa command - interpret carriage-control characters
 *
 * SYNOPSIS : asa [file...]
 *
 * DESCRIPTION : The asa utility will write its input files to standard output, 
 *		 mapping carriage-control characters from the text files to 
 *		 line-printer control sequences in an implementation-dependent
 *		 manner.
 *		 The first character of every line will be removed from the
 *		 input, and the following actions will be performed:
 *
 *		 If the character removed is:
 *		 space	The rest of the line will be output without change
 *		 0	A newline character will be output, then the rest of 
 *			the input line.
 *		 1	A form-feed character will be output, then the rest of
 *			the input line.
 *		 +	The newline character of the previous line will be 
 *			replaced by a carriage-return character, followed by
 *			the rest of the input line. If the + is the first 
 *			character in the input, it will have the same effect
 *			as the space character.
 *		 other  It will be output, then the rest of the input line.
 *
 * EXIT STATUS : 0 	All input files were output successfully.
 *		>0 	An error occurred.
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include "asa_msg.h"

#define MSGSTR(Num, Str) catgets(catd, MS_ASA, Num, Str)

nl_catd catd;

int first_char;

#define NL	'\n'	/* newline */
#define CR	'\r'	/* carriage return */
#define FF	'\f'    /* form feed */

static void asa_process(FILE *);

void
main(argc, argv)
int argc;
char *argv[];
{
	FILE *fp;
		 
	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_ASA, NL_CAT_LOCALE);

	first_char = 2;

	if (argc == 1)
	  asa_process ((FILE *) stdin);
	else
	  while (--argc > 0)
	    if ((fp = fopen(*++argv, "r")) == NULL) {
	      fprintf(stderr,MSGSTR(OPERR, "asa: can't open %s\n"), *argv);  
	      exit(1);
            }
	    else {
	      asa_process(fp);
	      fclose(fp);
            }
	/* output a newline character if it has not been output*/
	if (first_char == 1)
	  putc(NL, stdout);
	exit(0);
}
	      
	
/*
 * NAME : asa_process
 *
 * FUNCTION : interpret carriage-control characters for a single file
 *
 * RETURN VALUE : void
 *
 */

void
asa_process(fp)
FILE *fp;
{
	register char c;
	while ((c = getc(fp)) != EOF) {
	  if (first_char != 0) {
	  /* first character of a line */
	    if ((first_char != 2) && (c != '+')) {
	    /* it is not the first character of input and not equal to '+'*/
	      putc(NL, stdout);
            } 
	    switch (c) {
	    case ' ':
		break;
            case '0':
		putc(NL, stdout);
		break;
            case '1':
		putc(FF, stdout);
	        break;
	    case '+':
		if (first_char != 2) {
		/* it is not the first character of input */
		  putc(CR, stdout);
	        }
		break;
	    case NL:
		break;
	    default:
		putc(c, stdout);
		break;
	    }
	  }
	  else {
	    if (c != NL)
	      putc(c, stdout);
	  }
	  if (c == NL)
	    first_char = 1;
	  else
	    first_char = 0;
	}
}
