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
static char *rcsid = "@(#)$RCSfile: setclrnce.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:26:02 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	setclrnce.c,v $
 * Revision 1.1.1.2  92/03/31  00:45:00  devrcs
 *  *** OSF1_1B25 version ***
 * 
 * Revision 1.6.2.2  1992/02/11  17:40:45  hosking
 * 	bug 4520: return error if passed a NULL ir pointer
 * 	[1992/02/11  17:40:21  hosking]
 *
 * Revision 1.6  1991/03/04  17:45:37  devrcs
 * 	Comment out ident directives
 * 	[91/01/31  08:58:44  lehotsky]
 * 
 * Revision 1.5  91/01/07  16:00:09  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:31:14  dwm]
 * 
 * Revision 1.4  90/10/07  20:09:06  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:17:59  gm]
 * 
 * Revision 1.3  90/07/17  12:21:38  devrcs
 * 	Internationalized
 * 	[90/07/05  07:45:18  staffan]
 * 
 * Revision 1.2  90/06/22  21:48:13  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:45:42  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988 SecureWare, Inc.
 * All Rights Reserved.
 */

/* #ident "@(#)setclrnce.c	2.1 16:16:53 4/20/90 SecureWare" */
/* #ident "@(#)setclrnce.c	2.1 11:13:05 1/25/89 SecureWare, Inc." */

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
 *   -1 and errno = EPERM  means process clearance already set
 *    0 and errno = 0      means success
 */

setclrnce (ir)
	mand_ir_t *ir;
{
	attr_t	attr;
	obj_t	obj;
	int	ret;

	/* Make sure the mand_config structure is initialized */
	if (mand_init() != 0) {
		errno = EINVAL;
		sec_errno = ESEC_MAC_CONFIG_FAILURE;
		return -1;
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

	return setlabel(mand_config.policy, MAND_SUBJ_CL_TAG, &attr,
			OT_PROCESS, &obj);
}
#endif
#endif
