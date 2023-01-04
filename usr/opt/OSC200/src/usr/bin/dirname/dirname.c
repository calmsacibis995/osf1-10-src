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
static char rcsid[] = "@(#)$RCSfile: dirname.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/10/11 16:46:17 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: dirname 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/cmd/sh/dirname.c, cmdsh, bos320, 9142320h 10/15/91 08:56:37 
 */

#include	<stdio.h>
#include	<locale.h>
#include	"dirname_msg.h"

#define MSGSTR(n,s) catgets(catd,MS_DIRNAME,n,s)

/*
 * NAME: dirname
 * FUNCTION: Returns the directory name of a string parameter
 */
main(argc, argv)
char **argv;
{
	register wchar_t *p1, *p2;
	register int  i = 1;
	union {
		wchar_t wbuf[BUFSIZ];
		char cbuf[BUFSIZ];
	} workbuf;

	(void) setlocale(LC_ALL, "");

	/* XPG4: ignore leading -- */
	if (argc > 1){
		mbstowcs(workbuf.wbuf, argv[1], sizeof (workbuf.wbuf));
		if(wcscmp(workbuf.wbuf, L"--") == 0){
			i++;
			argc--;
		}
	}

	if (argc < 2) {
		nl_catd catd;

		catd = catopen(MF_DIRNAME, NL_CAT_LOCALE);
		fprintf(stderr, MSGSTR(USAGE, "Usage: dirname string\n"));
		exit(1);
	}

	if(i != 1)
		mbstowcs(workbuf.wbuf, argv[i], sizeof (workbuf.wbuf));
	p1 = workbuf.wbuf;
	p2 = p1;

	/**********
	  Get to the end of the string
	**********/

	while(*p1)
		p1++;
	p1--;

	/**********
	  backup over any trailing '/'
	**********/
	for (; p1>p2; p1--)
		if (*p1 != L'/')
			break;
	/*********
	  skip anything that isn't a '/'
	**********/
	for (; p1>=p2; p1--)
		if (*p1 == L'/')
			break;
	/**********
	  skip any more '/', 
	**********/
	for (; p1>p2; p1--)
		if (*p1 != L'/')
			break;

	*++p1 = L'\0';

	if (*p2 == L'\0') {
		*p2 = L'.';
		*(p2+1) = L'\0';
	}
	wcstombs(workbuf.cbuf, p2, sizeof(workbuf.cbuf));
	puts(workbuf.cbuf);
	exit(0);
}
