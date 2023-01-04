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
static char	*sccsid = "@(#)$RCSfile: storage.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/12/03 22:41:25 $";
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

/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: MBMalloc, StoreMName, StoreSName
 *
 * ORIGINS: 00 03 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "mfile1.h"
#include "lint2.h"

/*
** Symbol name storage definitions.
*/
static char	*snameTbl;		/* symbol name table */
static int	snameEmpty;		/* remaining table space */
static char	*mnameTbl;		/* member name table */
static int	mnameEmpty;		/* remaining table space */

/*
** Copy symbol string into permanent string storage.
** Return pointer to saved string.
*/
char *
StoreSName(cp)
	register char *cp;
{
	register int len;

	/* Has storage area been exceeded? */
	len = strlen(cp) + 1;
	if (len > snameEmpty) {
		/* Bigger chuck than NAMEBLK needed? */
		snameEmpty = (len > NAMEBLK) ? len : NAMEBLK;
		snameTbl = (char *) getmem((unsigned) snameEmpty);
	}

	/* Copy string into storage. */
	(void) strncpy(snameTbl, cp, len);
	cp = snameTbl;
	snameTbl += len;
	snameEmpty -= len;
	return (cp);
}

/*
** Copy symbol member string into permanent string storage.
** Return pointer to saved string.
*/
char *
StoreMName(cp)
	register char *cp;
{
	register int len;

	/* Has storage area been exceeded? */
	len = strlen(cp) + 1;
	if (len > mnameEmpty) {
		/* Bigger chuck than NAMEBLK needed? */
		mnameEmpty = (len > NAMEBLK) ? len : NAMEBLK;
		mnameTbl = (char *) getmem((unsigned) mnameEmpty);
	}

	/* Copy string into storage. */
	(void) strncpy(mnameTbl, cp, len);
	cp = mnameTbl;
	mnameTbl += len;
	mnameEmpty -= len;
	return (cp);
}

/*
** Allocate memory for member symbol table.
*/
MBTAB *
MBMalloc()
{
	static int bunchsize = 0;
	static MBTAB *mbbunch;

	if (bunchsize == 0) {
		mbbunch = (MBTAB *) getmem(MBRBLK * sizeof(MBTAB));
		bunchsize = MBRBLK;
	}
	return (&mbbunch[--bunchsize]);
}
