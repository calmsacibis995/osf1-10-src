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
static char rcsid[] = "@(#)$RCSfile: basename.c,v $ $Revision: 4.2.5.5 $ (DEC) $Date: 1993/10/11 15:36:39 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * basename.c	1.9  cmdsh, bos320, 9125320 6/10/91 09:01:15	
 */

#include	<stdio.h>
#include	<locale.h>
#include	"basename_msg.h"

#define MSGSTR(n,s) catgets(catd,MS_BASENAME,n,s)

/*
 * NAME: basename
 * FUNCTION: Returns the base name of a string parameter
 */
main(argc, argv)
int argc;
char **argv;
{
	register wchar_t *p1, *p2, *p3;
	union {
		wchar_t wbuf[BUFSIZ];
		char cbuf[BUFSIZ];
	} workbuf;
	wchar_t wbuf2[BUFSIZ];

	if (argc < 2 || argc > 3)  {		/* No args? or >2 args? */
		nl_catd catd;

		catd = catopen(MF_BASENAME, NL_CAT_LOCALE);
		fprintf(stderr, MSGSTR(USAGE, "Usage: basename string [suffix]\n"));
		exit(1);
	}

	(void) setlocale(LC_ALL, "");

       	mbstowcs(workbuf.wbuf, argv[1], sizeof (workbuf.wbuf));
	p1 = workbuf.wbuf;
	p2 = p1;
	while (*p1) {				/* remove trailing slashs */
		if (*p1 != L'/')  {
			p2 = p1;
			while(*p1 && *p1 != L'/')
				p1++;
			if (*p1)
				*p1++ = L'\0';
		}
		else
			p1++;
	}

	if ((p2 == workbuf.wbuf) && (*p2 == L'/')) { /* if p2 is equal to argv[1] */
		p2[1] = L'\0';			/* the string could be all   */
		goto output;			/* slashes so output only 1. */
	}


	if (argc>2) {				/* if there was a suffix,    */
		mbstowcs(wbuf2, argv[2], sizeof (wbuf2));
		for(p3=wbuf2; *p3; p3++)	/* remove it unless it is the*/
			;			/* entire remaing string,    */
		while(p1>p2 && p3>wbuf2){	/* remove it.                */
			if(*--p1 == L'/')
				while( *p1 == L'/')
					--p1;
			else
				*++p1;
			if(*--p3 != *--p1)
				goto output;
		}

	if(p1 != p2)
		*p1 = L'\0';
	}
output:
	wcstombs(workbuf.cbuf, p2, sizeof(workbuf.cbuf));
	puts(workbuf.cbuf);
	exit(0);
}
