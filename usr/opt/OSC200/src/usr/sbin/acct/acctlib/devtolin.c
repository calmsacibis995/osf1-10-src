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
static char	*sccsid = "@(#)$RCSfile: devtolin.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:58:37 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 *	convert device to linename (as in /dev/linename)
 *	return ptr to LSZ-byte string, "?" if not found
 *	device must be character device
 *	maintains small list in tlist for speed
 */

#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <sys/dir.h>

#define TSIZE1	50	/* # distinct names, for speed only */
static	tsize1;
static struct tlist {
	char	tname[LSZ];	/* linename */
	dev_t	tdev;		/* device */
} tl[TSIZE1];

static struct dirent *d;

extern dev_t	lintodev();

char *
devtolin(device)
dev_t device;
{
	register struct tlist *tp;
	DIR *fdev;

	for (tp = tl; tp < &tl[tsize1]; tp++)
		if (device == tp->tdev)
			return(tp->tname);

	if ((fdev = opendir("/dev")) == NULL)
		return("?");
	while ( (d = readdir(fdev)) != NULL)
		if (lintodev(d->d_name) == device) {
			if (tsize1 < TSIZE1) {
				tp->tdev = device;
				(void)CPYN(tp->tname, d->d_name);
				tsize1++;
			}
			closedir(fdev);
			return(d->d_name);
		}
	closedir(fdev);
	return("?");
}
