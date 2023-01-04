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
static char	*sccsid = "@(#)$RCSfile: del_ab.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:48:41 $";
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
 * FUNCTIONS: del_ab
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
/*   del_ab.c 1.5 com/cmd/sccs/lib/comobj,3.1,9013 9/15/89 13:51:03"; */


# include	"defines.h"

del_ab(p,dt,pkt)
register char *p;
register struct deltab *dt;
struct packet *pkt;
{
	extern  char    *satoi(), *strncpy();
	int n;
	extern char *Datep;
	char *sid_ab();

	if (*p++ != CTLCHAR)
		fmterr(pkt);
	if (*p++ != BDELTAB)
		return(*--p);
	NONBLANK(p);
	dt->d_type = *p++;
	NONBLANK(p);
	p = sid_ab(p,&dt->d_sid);
	NONBLANK(p);
	date_ab(p,&dt->d_datetime);
	p = Datep;
	NONBLANK(p);
	n = strcspn(p, " ");
	strncpy(dt->d_pgmr,p,n);
	dt->d_pgmr[n] = 0;
	p += n + 1;
	NONBLANK(p);
	p = satoi(p,&dt->d_serial);
	NONBLANK(p);
	p = satoi(p,&dt->d_pred);
	if (*p != '\n')
		fmterr(pkt);
	return(BDELTAB);
}
