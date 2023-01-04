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
static char	*sccsid = "@(#)$RCSfile: pass4.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/06/03 14:40:41 $";
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
 * Copyright (c) 1980, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint

#endif /* not lint */

#include <sys/param.h>
#include <ufs/dinode.h>
#include <ufs/fs.h>
#include <stdlib.h>
#include <string.h>
#include "fsck.h"

int	pass4check();

pass4()
{
	register ino_t inumber;
	register struct zlncnt *zlnp;
	struct dinode *dp;
	struct inodesc idesc;
	int n;

#ifdef DEBUG
	printf ("Entering Pass4\n");
#endif
	bzero((char *)&idesc, sizeof(struct inodesc));
	idesc.id_type = ADDR;
	idesc.id_func = pass4check;
	for (inumber = ROOTINO; inumber <= lastino; inumber++) {
		idesc.id_number = inumber;
		switch (statemap[inumber]) {

		case FSTATE:
		case DFOUND:
			n = lncntp[inumber];
			if (n)
				adjust(&idesc, (short)n);
			else {
				for (zlnp = zlnhead; zlnp; zlnp = zlnp->next)
					if (zlnp->zlncnt == inumber) {
						zlnp->zlncnt = zlnhead->zlncnt;
						zlnp = zlnhead;
						zlnhead = zlnhead->next;
						free((char *)zlnp);
						clri(&idesc, "UNREF", 1);
						break;
					}
			}
			break;

		case DSTATE:
			clri(&idesc, "UNREF", 1);
			break;

		case DCLEAR:
			dp = ginode(inumber);
			if (dp->di_size == 0) {
				clri(&idesc, "ZERO LENGTH", 1);
				break;
			}
			/* fall through */
		case FCLEAR:
			clri(&idesc, "BAD/DUP", 1);
			break;

		case USTATE:
			break;

		default:
			errexit("BAD STATE %d FOR INODE I=%d",
			    statemap[inumber], inumber);
		}
	}
}

pass4check(idesc)
	register struct inodesc *idesc;
{
	register struct dups *dlp;
	int nfrags, res = KEEPON;
	daddr_t blkno = idesc->id_blkno;

	for (nfrags = idesc->id_numfrags; nfrags > 0; blkno++, nfrags--) {
		if (chkrange(blkno, 1)) {
			res = SKIP;
		} else if (testbmap(blkno)) {
			for (dlp = duplist; dlp; dlp = dlp->next) {
				if (dlp->dup != blkno)
					continue;
				dlp->dup = duplist->dup;
				dlp = duplist;
				duplist = duplist->next;
				free((char *)dlp);
				break;
			}
			/*
			 * We only want to free this block is all
			 * duplicate blocks are known. If all_dups
			 * is not set, don't free the block.
			 */
			if (dlp == 0 && all_dups) {
				clrbmap(blkno);
				n_blks--;
			}
		}
	}
	return (res);
}
