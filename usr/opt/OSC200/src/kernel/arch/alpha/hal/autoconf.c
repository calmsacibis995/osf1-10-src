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
static char *rcsid = "@(#)$RCSfile: autoconf.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/09 21:45:55 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)autoconf.c	9.1	(ULTRIX/OSF)	10/21/91";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: alpha/autoconf.c
 *
 * 12-sep-91 jac update references to the structure 'bus'
 *
 * 26-Apr-91 -- afd
 *	Created this file for Alpha support.
 */

#include <sys/systm.h>
#include <io/common/devdriver.h>

/*
 * The following variables are related to the configuration process,
 * and are used in initializing the system.
 */
int	cold = 1 ;		/* if 1, still working on cold-start */
int	autoconf_cvec;	/* global for interrupt vector from probe routines */
int	autoconf_br;	/* global for IPL from probe routines */
int	(*autoconf_intr)();	/* interrupt handler */
u_int	autoconf_csr;	/* csr base address in k1seg */

config_fillin(ctlr)
	register struct controller *ctlr;
{
	printf("%s%d at %s%d", ctlr->ctlr_name, ctlr->ctlr_num,
	       ctlr->bus_name, ctlr->bus_num);

	if ((strcmp(ctlr->bus_name, "vaxbi") == 0) ||
	    (strcmp(ctlr->bus_name, "xmi") == 0)) 
		printf(" node %d", ctlr->slot);
	else if ((strcmp(ctlr->bus_name, "tc") == 0) ||
		 (strcmp(ctlr->bus_name, "tcds") == 0))
		printf(" slot %d", ctlr->slot);
}
