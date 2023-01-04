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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: versys.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:12:33 $";
#endif
/* 
 * COMPONENT_NAME: UUCP versys.c
 * 
 * FUNCTIONS: versys 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/
/*
1.3  versys.c, bos320 6/16/90 00:04:12";
*/
#include "uucp.h"
/* VERSION( versys.c	5.2 -  -  ); */

/*
 * verify system name
 * input:
 *	name	-> system name
 * returns:  
 *	0	-> success
 *	FAIL	-> failure
 */
versys(name)
char *name;
{
	register FILE *fp;
	register char *iptr;
	char line[300];

	/* check for illegal chars in site name */
	char *namep;
	namep = name;
	while (*namep)
	    if (! (isascii(*namep++))) {
	      fprintf (stderr, MSGSTR(MSG_VER_1,
	      "A non-ascii character was detected in system name %s\n"), name);
	      exit( 2 );
        }

	if (EQUALS(name, Myname))
		return(0);

	fp = fopen(SYSFILE, "r");
	if (fp == NULL)
		return(FAIL);
	
	while (fgets(line, sizeof(line) ,fp) != NULL) {
		if((line[0] == '#') || (line[0] == ' ') || (line[0] == '\t') || 
			(line[0] == '\n'))
			continue;

		if ((iptr=strpbrk(line, " \t")) == NULL)
		    continue;	/* why? */
		*iptr = '\0';
		if (EQUALS(name, line)) {
			(void) fclose(fp);
			return(0);
		}
	}
	fclose(fp);
	return(FAIL);
}
