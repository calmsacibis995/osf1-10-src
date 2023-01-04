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
static char	*sccsid = "@(#)$RCSfile: db_reload.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:14:22 $";
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

*/
/* 
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1986, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
#ifndef lint

#endif  not lint */

#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <arpa/nameser.h>
#include "ns.h"
#include "db.h"

extern time_t	resettime;

/*
 * Flush and reload data base.
 */

db_reload()
{
	extern char *bootfile;

#ifdef DEBUG
	if (debug >= 3)
		fprintf(ddt,"reload()\n");
#endif
	syslog(LOG_NOTICE, MSGSTR(RELOAD, "reloading nameserver\n")); /*MSG*/

	qflush();
	sqflush();
	fwdtab_free();
	free_sort_list();
        getnetconf();
	ns_init(bootfile);
	time(&resettime);
}

db_free(htp)
	struct hashbuf *htp;
{
	register struct databuf *dp, *nextdp;
	register struct namebuf *np, *nextnp;
	struct namebuf **npp, **nppend;

	npp = htp->h_tab;
	nppend = npp + htp->h_size;
	while (npp < nppend) {
	    for (np = *npp++; np != NULL; np = nextnp) {
		if (np->n_hash != NULL)
			db_free(np->n_hash);
		(void) free((char *)np->n_dname);
		for (dp = np->n_data; dp != NULL; ) {
			nextdp = dp->d_next;
			(void) free((char *)dp);
			dp = nextdp;
		}
		nextnp = np->n_next;
		free((char *)np);
	    }
	}
	(void) free((char *)htp);
}

db_inv_free()
{
	register struct invbuf *ip;
	register int i, j;

	for (i = 0; i < INVHASHSZ; i++)
		for (ip = invtab[i]; ip != NULL; ip = ip->i_next)
			for (j = 0; j < INVBLKSZ; j++)
				ip->i_dname[j] = NULL;
}

fwdtab_free()
{
	extern	struct fwdinfo *fwdtab;
	struct fwdinfo *fp, *nextfp;

	for (fp = fwdtab; fp != NULL; fp = nextfp) {
		nextfp = fp->next;
		free((char *)fp);
	}
	fwdtab = NULL;
}
