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
static char *rcsid = "@(#)$RCSfile: chk_invoker.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 09:32:52 $";
#endif
/*****************************************************************************
*
*	int	siad_chk_invoker()
*
* Description: The purpose of this routine is to check the calling routine
* for appropriate privaleges, ussually uid==0.
*
* Returns: 
*		SAIDSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
#include <stdio.h>
#include <sia.h>
#include <siad.h>
siad_chk_invoker()
{
	extern int errno;

/* This is only for a full login. -DAL
	errno = 0;
	(void) getluid();
	if(errno == 0)
		return SIADFAIL;
	else
		return SIADSUCCESS;
*/
	if(geteuid() == 0)
		return SIADSUCCESS;
	else
		return SIADFAIL;
}
