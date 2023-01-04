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
static char *rcsid = "@(#)$RCSfile: siad_c_user.c,v $ $Revision: 1.1.11.3 $ (DEC) $Date: 1993/06/08 01:20:30 $";
#endif
/*****************************************************************************
*
*	int	siad_chk_user(char *loginname, int  chkflg)
*
* Description: The purpose of this routine is to check the calling routine
* for appropriate privaleges, ussually uid==0.
*
* Returns: 
*		SIADSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak siad_chk_user = __siad_chk_user
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"
extern struct passwd siad_bsd_passwd;
extern char siad_bsd_getpwbuf[];

siad_chk_user(loginname,chkflg)
char *loginname;
int  chkflg;
{
	struct passwd *pwd;
	if(bsd_siad_getpwnam(loginname, &siad_bsd_passwd, &siad_bsd_getpwbuf,SIABUFSIZ) == SIADFAIL)
		return SIADFAIL;
	else if(chkflg & (CHGPASSWD | CHGSHELL | CHGFINGER))
			return SIADSUCCESS;
		/* BSD and C2 supports changes of password|shell|finger*/
	return SIADFAIL;
}
