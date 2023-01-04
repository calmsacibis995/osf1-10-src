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
static char	*sccsid = "@(#)$RCSfile: fmalloc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:49:07 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: fmalloc, ffree, ffreeall
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* fmalloc.c 1.4 com/cmd/sccs/lib/comobj,3.1,9021 9/15/89 13:59:07"; */

#include "defines.h"

char    *ptrlist;
void	free();

/* allocate asize bytes;
 * chain to ptrlist for later ffreeall
 */
char *
fmalloc(asize)
unsigned asize;
{
	char *ptr, *malloc();

	asize += sizeof(char *);

	if (!(ptr = malloc(asize)))
		fatal(MSGCO(OTOFSPC, 
                  "\nThere is not enough memory available now.(ut9)\n"));

	*(char **)ptr = ptrlist;
	ptrlist = ptr;
	return ptr + sizeof(char *);
}

/* free space allocated by fmalloc; unlink from ptrlist */
ffree(aptr)
char *aptr;
{
	register char **pp, *ptr;

	aptr -= sizeof(char *);
	for (pp = &ptrlist;; pp = (char **)ptr) {
		if (!(ptr = *pp))
			fatal(MSGCO(FFREE, 
                           "\nAn attempt to free memory failed.\n"));  /* MSG */
		if (aptr == ptr)
			break;
	}
	*pp = *(char **)ptr;
	free(aptr);
}

/* free all memory on ptrlist */
ffreeall()
{
	register char *ptr;

	while (ptr = ptrlist) {
		ptrlist = *(char **)ptr;
		free(ptr);
	}
}
