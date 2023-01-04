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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: printf.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/10/11 20:04:35 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
 *
 * FUNCTIONS: printf
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/cmd/posix/printf.c, cmdposix, bos320, 9125320 6/4/91 08:54:09
 */

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <nl_types.h>
#include <mbstr.h>
#include <stdlib.h>
#include "printf_msg.h"

#define MAXSTR LINE_MAX			/* max string to be output */
nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_PRINTF,num,str)  

int err = 0;			/* have we seen an error */

static char *advance(char *source, char *nxtchr);
static char *escwork(char *source, int bflag);
static int doargs(char **dest, char **fmtptr, char *args);
static int finishline(char *dest, char *fmtptr);
static int find_format(char *string);

main(int argc, char **argv)
{
	char *fmtptr, *restart;			/* pointer to format */
	char  *start;
	char outstr[MAXSTR + 1];		/* output string */

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_PRINTF, NL_CAT_LOCALE);
	errno = 0;

	if (argv[1]) {
		fmtptr = argv[1];
		restart = argv[1];
	}
	else
	   	exit(0);

	/*
 	 * This section transforms octal numbers and backslash sequences to
 	 * the correct character representations and stores the resulting 
 	 * string back into fmtptr.
         */
	start = escwork(fmtptr, 0);
	*start = '\0';

        /*
         * If no format specification, simply printf format string 
         */

	if (!find_format(fmtptr)) {
		printf(fmtptr);
		exit(0);
	}

	/*
	 * Escape sequences have been translated.  Now process 
         * format arguments.
         */

	argv += 2;
	start = outstr;
	while (*argv){
		if (doargs(&start, &fmtptr, *argv)) 
			fmtptr = restart;
		else
			argv++;
	}

	/*
         * Check to see if 'format' is done. if not transfer the
         * rest of the 'format' to output string.
         */

	if (strlen(fmtptr))
		finishline(start, fmtptr);

	printf("%s", outstr);
	exit(err? 1: 0);
}



/*
 * 	escwork
 *
 * This routine transforms octal numbers and backslash sequences to the
 * correct character representations and stores the resulting string 
 * in the character pointer destin.
 *
 * The returned value is a character pointer to the last available position in
 * destination string.
 */

char *
escwork(char *source, int bflag)
/* char *source;			 pointer to source      */
/* int 	bflag;				 are we doing %b	*/
{
	char *destin;			/* pointer to destination */
	char *nxtchr;			/* pointer to next complete char */
	int backslash;			/* flag for backslash */
	int octnum;			/* octal number */

	destin = nxtchr = source;

       /*
        *  Parse through line to transform octal numbers and
        *  backslash sequences to the correct character
        *  representation.
        */ 
	for (backslash = 0; *source != '\0'; source++) {

	       /*
		*  Found octal number  
                */
		if (backslash && *source >= '0' && *source <= '7') {
			int i = 0;
			if (bflag)	/* must have a zero */
				if (*source != '0') {
					fprintf(stderr, MSGSTR(INVAL, 
					  "printf: invalid escape sequence - must start with a zero\n"));
					exit(1);
				} else
					source++;
			octnum = 0;
			while (i++ < 3 && *source >= '0' && *source <= '7') {
				octnum = (8*octnum) + (*source - '0');
				source++;
			}
			*destin++ = (char)octnum;
			backslash = 0;
			nxtchr = source;	/* first char after number */
			source--;		/* soon to be incremented */
			continue;
		}

	       /*
	 	*  Found backslash sequence.  If valid character, replace
                *  it.  If not a valid character, don't transform.
         	*/

		if (backslash) {
				switch (*source) {
				case '\\':
					*destin = '\\';
					break;
				case 'a':
					*destin = 07;
					break;
				case 't':
					*destin = '\t';
					break;
				case 'n':
					*destin = '\n';
					break;
				case 'v':
					*destin = '\v';
					break;
				case 'b':
					*destin = '\b';
					break;
				case 'r':
					*destin = '\r';
					break;
				case 'f':
					*destin = '\f';
					break;
				default:
					*destin = '\\';
					*++destin = *source;
				}
	
			backslash = 0;
			++destin;
			nxtchr = advance(source, nxtchr);
			continue;
		}

		/* found a backslash */
		if ((*source == '\\') && (nxtchr == source)) {
			backslash++;
			nxtchr = advance(source, nxtchr);
			continue;
		}

		*destin++ = *source;		/* normal character */

		nxtchr = advance(source, nxtchr);
		
	}

	if (backslash)  
		++destin;

	return(destin);				/* return pointer */
}




/*
 *    doargs
 *
 * This routine does the actual formatting of the input arguments.
 *
 * This routine handles the format of the following form:
 *	%pw.df
 *		p:	prefix, one or more of {- + # or blank}.
 *		w:	width specifier, can be *.
 *		.:	decimal.
 *		d:	precision specifier can be *.
 *		f:	format xXioudfeEgGcbs.
 *
 * The minimum set required is "%f".  Note that "%%" prints one "%" in output.
 */

int
doargs(char **dest, char **fmtptr, char *args)
/* char **dest;			 destination string  */
/* char **fmtptr;  		 format string       */
/* char *args; 		 	 argument to process */
{
	char tmpchar, *last;
	char *ptr;			
	char *start, *end;
	int octnum;
	long lnum;
	double fnum;
	int percent;			/* flag for "%" */
	int width;			/* flag for width */

	percent = 0;

	/*
         *   "%" indicates a conversion is about to happen.  This section
         *   parses for a "%"
         */

	for (last = *fmtptr; *last != '\0'; last++) {
		if (!percent) {
			if (*last == '%')
				percent++;
			continue;
		}

		/*
          	 * '%' has been found check the next character for conversion.
         	 */

		switch (*last) {
		case '%':
			percent = 0;
			continue;
		case 'x':
		case 'X':
		case 'd':
		case 'o':
		case 'i':
		case 'u':
			if (*args == '\'' || *args == '"') {
				wchar_t wc;
				if (mbtowc(&wc, args+1, MB_CUR_MAX) < 0)
					lnum = *(args+1);
				else
					lnum = wc;
			}
			else {
				errno = 0;
				if (*last == 'u')
					lnum = strtoul(args, &ptr, 0);
				else
					lnum = strtol(args, &ptr, 0);
				if (ptr == args) {
					fprintf(stderr, MSGSTR(NOTNUM, "printf: \"%s\" expected numeric value\n"), args);
					err = 1;
				}
				else if (*ptr != '\0') {
					fprintf(stderr, MSGSTR(NOTCONV, "printf: \"%s\" not completely converted\n"), args);
					err = 1;
				}
				/*
				 * Treat as arithmetic overflow if input
				 * value larger than an integer or a positive
				 * number become negative or vice versa.
				 */
				else if (errno ||
				((*args == '-') && (lnum != (int)lnum)) ||
				((*args != '-') &&
				((lnum != (unsigned int)lnum) ||
				(((int)lnum < 0) &&
				((*last == 'i') || (*last == 'd')))))) {
					fprintf(stderr, MSGSTR(OFLOW, "printf: \"%s\" arithmetic overflow\n"), args);
					err = 1;
				}
			}
			tmpchar = *(++last);
			*last = '\0';
			sprintf(*dest, *fmtptr, lnum);
			*last = tmpchar;
			break;
		case 'f':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
			if (*args == '\'' || *args == '"') {
				wchar_t wc;
				if (mbtowc(&wc, args+1, MB_CUR_MAX) < 0)
					lnum = *(args+1);
				else
					lnum = wc;
			}
			else {
				errno = 0;
				fnum = strtod(args, &ptr);
				if (ptr == args) {
					fprintf(stderr, MSGSTR(NOTNUM, "printf: \"%s\" expected numeric value\n"), args);
					err = 1;
				}
				else if (*ptr != '\0') {
					fprintf(stderr, MSGSTR(NOTCONV, "printf: \"%s\" not completely converted\n"), args);
					err = 1;
				}
				else if (errno) {
					fprintf(stderr, MSGSTR(OFLOW, "printf: \"%s\" arithmetic overflow\n"), args);
					err = 1;
				}
			}
			tmpchar = *(++last);
			*last = '\0';
			sprintf(*dest, *fmtptr, fnum);
			*last = tmpchar;
			break;
		case 'c':
			tmpchar = *(++last);
			*last = '\0';
			sprintf(*dest, *fmtptr, *args);
			*last = tmpchar;
			break;
		case 's':
			tmpchar = *(++last);
			*last = '\0';
			sprintf(*dest, *fmtptr, args);
			*last = tmpchar;
			break;
		case 'b':
			start = escwork(args, 1);
			*start = '\0';
			if ((end = strstr(args,"\\c")) != '\0')
				*end = '\0';
			*last = 's';
			tmpchar = *(++last);
			*last = '\0';
			sprintf(*dest, *fmtptr, args);
			*last = tmpchar;
			*(last-1) = 'b';	/* put %b back for later */
			break;
		default:
			if (isdigit(*last)) {
				errno = 0;
				width = strtol(last, &ptr, 0);
					if (errno) {
						fprintf(stderr, MSGSTR(TOOBIG, "printf: value too large\n"));
						exit(1);
					}
				continue;
			}
			switch (*last) {	
			case '-':
			case '+':
			case ' ':
			case '#':
			case '.':
				continue;
			default:
				tmpchar = *(++last);
				*last = '\0';
				sprintf(*dest, *fmtptr);
				*last = tmpchar;
				break;
			} 
		} 
		*fmtptr = last;
		*dest += strlen(*dest);
		percent = 0;
		return(0);
	} 
	sprintf(*dest, *fmtptr);
	*dest += strlen(*dest);
	return(1);
} 


/*
 *   finishline
 *
 *	This routine finishes processing the extra format specifications
 */

int
finishline(char *dest, char *fmtptr)
/* char *dest;			 destination string */
/* char *fmtptr;	         format string      */
{ 
	char tmpchar, *last;
	int percent;				/* flag for "%" */
	int width;
	char *ptr;

	/*
         * Check remaining format for "%".  If no "%", transfer 
         * line to output.  If found "%" replace with null for %s or
         * %c, replace with 0 for all others.
         */

	if (!strchr(fmtptr, '%')) {
		sprintf(dest, fmtptr);
		return(0);
	}

	for (percent = 0, last = fmtptr; *last != '\0'; last++) {
		if (!percent) {
			if (*last == '%') {
				percent++;
				fmtptr = last;
			} else
				*dest++ = *last;
			continue;
		}

		/*
                 * OK. '%' has been found check the next character
                 * for conversion.
                 */
		switch (*last) {
		case '%':
			*dest++ = '%';
			percent = 0;
			continue;
		case 'x':
		case 'X':
		case 'd':
		case 'o':
		case 'i':
		case 'u':
		case 'f':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
		case 'b':
			*last = 'd';
			tmpchar = *(++last);
			*last = '\0';
			sprintf(dest, fmtptr, 0);
			*last = tmpchar;
			fmtptr = last;
			dest += strlen(dest);
			percent = 0;
			last--;
			break;
		case 'c':
		case 's':
			*last = 's';
			tmpchar = *(++last);
			*last = '\0';
			sprintf(dest, fmtptr, "");
			*last = tmpchar;
			fmtptr = last;
			dest += strlen(dest);
			percent = 0;
			last--;
			break;
		default:
			if (isdigit(*last)) {
				errno = 0;
				width = strtol(last, &ptr,0);
					if (errno) {
						fprintf(stderr, MSGSTR(TOOBIG, "printf: value too large\n"));
						exit(1);
					}
				continue;
			}
			switch (*last) {	
			case '-':
			case '+':
			case ' ':
			case '#':
			case '.':
				continue;
			default:
				break;
			} 
		} 
	} 
	*dest = '\0';	/* Terminate the string */
	return(0);
} 

char *
advance(char *source, char *nxtchr)
{
    	int len;

	if (source >= nxtchr)  {
    		len = mblen(nxtchr, MB_CUR_MAX);

    		if (len > 0)
			nxtchr += len;
    		else
			nxtchr++;
	}

	return(nxtchr);
}

/*
 * Determine if a string has printf format strings in it.
 * Watching out for '%%', which doesn't qualify.
 * 
 * Returns	1 - if format strings
 *		0 - otherwise
 */
static int
find_format(char *string)
{
	char *p = string;

	while (*p) {
		if ((*p++ == '%') && (*p++ != '%'))
			return (1);
	}
	return (0);
}
