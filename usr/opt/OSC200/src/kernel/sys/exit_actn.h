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
 * @(#)$RCSfile: exit_actn.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 13:22:35 $
 */
/*		@(#)exit_actn.h	4.1  (ULTRIX)        7/2/90	*/

#ifndef	_SYS_EXIT_ACTN_H_
#define _SYS_EXIT_ACTN_H_

/*
 *	The exit_actn struct.
 *	These structures are dynamically allocated and chained onto
 *	the u area to indicate kernel functions to be called before
 *	process exit.	See ../sys/kern_exit.c
 */

struct exit_actn{
	struct exit_actn *xa_next;
	int (*xa_func)();
};


int lmf_exit_actn();
#endif	/* _SYS_EXIT_ACTN_H_ */

