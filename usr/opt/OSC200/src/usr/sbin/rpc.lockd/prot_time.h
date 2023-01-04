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
/*
 *	@(#)$RCSfile: prot_time.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/12/11 16:54:38 $
 */
/*
 * @(#)prot_time.h	1.2 90/07/23 NFSSRC4.0 1.6 88/02/07
 *
 * This file consists of all timeout definition used by rpc.lockd
 */

#define MAX_LM_TIMEOUT_COUNT	1
#define OLDMSG			30		/* counter to throw away old msg */
/*
#define LM_TIMEOUT_DEFAULT 	15
*/
#define LM_TIMEOUT_DEFAULT	5
#define LM_GRACE_DEFAULT 	3
int 	LM_TIMEOUT;
int 	LM_GRACE;
int	grace_period;
