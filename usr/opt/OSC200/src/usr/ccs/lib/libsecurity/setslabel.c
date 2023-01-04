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
static char *rcsid = "@(#)$RCSfile: setslabel.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:26:08 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	setslabel.c,v $
 * Revision 1.1.1.2  92/03/31  00:45:00  devrcs
 *  *** OSF1_1B25 version ***
 * 
 * Revision 1.6.2.2  1992/02/11  17:43:03  hosking
 * 	bug 4520: return error if passed a NULL ir pointer
 * 	[1992/02/11  17:42:36  hosking]
 *
 * Revision 1.6  1991/03/04  17:45:43  devrcs
 * 	Comment out ident directives
 * 	[91/01/31  08:58:56  lehotsky]
 * 
 * Revision 1.5  91/01/07  16:00:17  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:31:25  dwm]
 * 
 * Revision 1.4  90/10/07  20:09:14  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:18:12  gm]
 * 
 * Revision 1.3  90/07/17  12:21:46  devrcs
 * 	Internationalized
 * 	[90/07/05  07:45:36  staffan]
 * 
 * Revision 1.2  90/06/22  21:48:18  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:47:51  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988 SecureWare, Inc.
 * All Rights Reserved.
 */

/* #ident "@(#)setslabel.c	2.1 16:16:57 4/20/90 SecureWare" */
/* #ident "@(#)setslabel.c	2.1 11:13:08 1/25/89 SecureWare, Inc." */

#include <sys/secdefines.h>

#if SEC_BASE
#if SEC_MAC

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <sys/secpolicy.h>
#include <mandatory.h>

extern int errno;
extern int sec_errno;

/* Return values:
 *   -1 and errno = EPERM  means process sensitivity label already set
 *			   or no privilege or clearance not set yet or
 *			   changing to specified label would violate
 *			   a constraint
 *    0 and errno = 0      means success
 */

setslabel (ir)
mand_ir_t *ir;
{
	attr_t	attr;
	obj_t	obj;

	/* Make sure mand_config structure is initialized */
	if (mand_init() != 0) {
		errno = EINVAL;
		sec_errno = ESEC_MAC_CONFIG_FAILURE;
		return (-1);
	}
	
	/* WILDCARD subject not supported */

	if (ir == (mand_ir_t *) NULL) {
		errno = EINVAL;
		sec_errno = ESEC_WILD_SUBJ_SL;
		return (-1);
	}

	attr.code = SEC_ACTUAL_TAG;
	attr.ir = (char *) ir;
	attr.ir_length = mand_bytes();

	obj.o_pid = 0;

	return setlabel(mand_config.policy, MAND_SUBJ_SL_TAG,
		&attr, OT_PROCESS, &obj);
}
#endif
#endif
