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
static char rcsid[] = "@(#)$RCSfile: genxlt.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/10/18 20:52:51 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDICONV)
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.4  com/cmd/iconv/genxlt.c, cmdiconv, bos320, 9130320 7/16/91 03:05:49
 */


#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <iconv.h>
#include "iconvTable.h"

#include "genxlt_msg.h" 

nl_catd catd;   /* Cat descriptor for scmc conversion */

#define MSGSTR(Num, Str) catgets(catd, MS_genxlt, Num, Str)

#ifndef	LINE_MAX
#define	LINE_MAX	1024
#endif

#define	True	1
#define	False	0
#define	INVAL_MARK	2
	
static IconvTable	iconvtable = {ICONV_REL2_MAGIC, {0}, {0}, {0}, {0}};
static unsigned char 	flag[256] = {False};
static unsigned char 	valid_data[256] = {False};
static int	line = 0;		/* current line no. */

/*
 * NAME: getval()
 *                                                                    
 * FUNCTION: returns the integer value of a number beginning at ptr.
 *                                                                    
 * RETURN VALUE DESCRIPTION:  getval returns an integer corresponding to 
 *	the string representation of 'num' held in *ptr or -1 if it's
 *	invalid.
 *			    
 */  

static int	getval(wchar_t *ptr)
{	
	int	val, n;
	char	data[LINE_MAX];
	
	if ((n = wcstombs(data, ptr, LINE_MAX)) != -1)
		if (sscanf(&data[0], "%x", &val) > 0)
				return val;
	return -1;
}

/*
 * NAME: parse_line
 *                                                                    
 * FUNCTION: Parses a line and fills one entry in the table.
 *                                                                    
 * RETURN VALUE DESCRIPTION:  NONE
 *			    
 */  

static void	parse_line(wchar_t *ptr)
{
	int	idx;
	int	val;
	char	s[MB_LEN_MAX];
	int	n;


	/*
	 * skip white spaces.
	 */
	while (*ptr && iswspace(*ptr))
		ptr++;

	if ((n = wctomb(s, *ptr)) == -1) {
		fprintf(stderr, MSGSTR(M_MSG_5,
			"genxlt: Invalid format at line %d\n"), line);
		exit(2);
	}

	if (n == 1 && s[0] == '#')
		return;			/* this is comment line */

	if (n == 1 && !s[0])
	        return;			/* blank line */

	if ((idx = getval(ptr)) == -1) {
		fprintf(stderr, MSGSTR(M_MSG_5,
			"genxlt: Invalid format at line %d\n"), line);
		exit(2);
	}

	/*
	 *	skip until white space.
	 */
	while (*ptr && !iswspace(*ptr))
		ptr++;

	/*
	 *	then skip white spaces.
	 */
	while (*ptr && iswspace(*ptr))
		ptr++;

	/*
	 *	if hits EOL
	 */
	if (!*ptr) {
		fprintf(stderr, MSGSTR(M_MSG_5,
			"genxlt: Invalid format at line %d\n"), line);
		exit(2);
	}

	if ((n = wctomb(s, *ptr)) == -1) {
		fprintf(stderr, MSGSTR(M_MSG_5,
			"genxlt: Invalid format at line %d\n"), line);
		exit(2);
	}
	if (n == 1 && s[0] == INVAL_STRINGHEAD) {
		flag[idx] = INVAL_MARK;
	}
	else {
		if ((val = getval(ptr)) == -1) {
			fprintf(stderr, MSGSTR(M_MSG_5,
				"genxlt: Invalid format at line %d\n"), line);
			exit(2);
		}
		iconvtable.data[idx] = val;
		valid_data[val] = flag[idx] = True;
	}
}

/*
 * NAME: main
 *                                                                    
 * FUNCTION: 	Creates a compiled version of the translation tables
 * 	suitable for use by ICONV table converter, etc.
 *	The source is a file or standard input.
 *	The target is standard output or a file which can be optionally
 *	specified with the -f option.
 *	The input is expected to be in a two column hexadecimal format.
 *                                                                    
 * RETURN VALUE DESCRIPTION:  If a error occurs 1 is returned to the
 *	execution environment, otherwise 0 is returned.
 *			    
 */  

main(int argc, char **argv, char **envp)
{
	int	i;
	unsigned char	inval_char;
	FILE 	*sf, *tf;
	wchar_t	lbuf[LINE_MAX];
	char *outfile = NULL;		/* output filename */
	int c;

	setlocale(LC_ALL, "");
	catd = catopen(MF_GENXLT, NL_CAT_LOCALE);

	
	while ((c = getopt(argc, argv, "f:")) != -1) {
		switch(c) {
			case 'f':
				outfile = optarg;
				break;
			default:
				fprintf(stderr, MSGSTR(M_MSG_1, 
				"Usage: genxlt [-f outputfile] [inputfile]\n"));
				exit(1);
		}
	}

	if (argc == optind)
		sf = stdin;
	else if ((sf = fopen(argv[optind], "r")) == NULL) {
		perror(argv[optind]);
		exit(1);
	}
	
	while (!feof(sf)) {
		fgetws(lbuf, LINE_MAX, sf);
		line++;
		parse_line(lbuf);
	}
	fclose(sf);

	iconvtable.inval_handle = False;
	for (i = 0 ; i < 256 ; i++) if (!valid_data[i]) {
		inval_char = i & 0xff; 
		break;
	}

	for (i = 0 ; i < 256 ; i++) {
		if (flag[i] == INVAL_MARK) {
			if (!iconvtable.inval_handle) {
				iconvtable.inval_char = inval_char; 
				iconvtable.inval_handle = True;
			}
			iconvtable.data[i] = inval_char;
		}
		else if (!flag[i]) {
			fprintf(stderr, MSGSTR(M_MSG_3,
			"genxlt: There was no assignment for index %d\n"), i);
			exit(1);
		}
	}

	if (outfile != NULL) {
		tf = fopen(outfile, "w");
		if (!tf) {
			fprintf(stderr, MSGSTR(M_MSG_2,
				"genxlt: Unable to open output file.\n"));
			exit(1);
		}
	}
	else
		tf = stdout;

	if (fwrite(&iconvtable, sizeof (IconvTable), 1, tf) != 1) {
		fprintf(stderr,
			MSGSTR(M_MSG_4,
			"genxlt: Unable to write to output file.\n"));
		exit(1);
	}
	fclose(tf);
	exit(0);
}

