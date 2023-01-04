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
static char *rcsid = "@(#)$RCSfile: sem_acl.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:25:52 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	sem_acl.c,v $
 * Revision 1.1.1.2  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.6.3.2  1992/04/08  20:30:38  marquard
 * 	Added POSIX ACL support.
 * 	[1992/04/05  14:05:31  marquard]
 *
 * Revision 1.6  1991/03/04  17:45:25  devrcs
 * 	Comment out ident directives
 * 	[91/01/31  08:58:26  lehotsky]
 * 
 * Revision 1.5  91/01/07  15:59:56  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:30:57  dwm]
 * 
 * Revision 1.4  90/10/07  20:08:53  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:17:36  gm]
 * 
 * Revision 1.3  90/07/17  12:21:28  devrcs
 * 	Internationalized
 * 	[90/07/05  07:44:52  staffan]
 * 
 * Revision 1.2  90/06/22  21:48:04  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:43:27  staffan]
 * 
 * $OSF_EndLog$
 */
/* Copyright (c) 1988-90 SecureWare, Inc.
 * All rights reserved.
 */

/* #ident "@(#)sem_acl.c	2.1 16:15:51 4/20/90 SecureWare" */
/* #ident "@(#)sem_acl.c	2.1 11:13:35 1/25/89 SecureWare, Inc." */

#include <sys/secdefines.h>

#if SEC_BASE && SEC_ACL

#include <sys/types.h>
#include <sys/errno.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/secpolicy.h>
#include <sys/sec_objects.h>
#include <acl.h>

extern int errno;
extern int sec_errno;

/*
 * Get the ACL associated with a semaphore
 */

sem_statacl (semid, acl, size)
	int	semid;
	acle_t	*acl;
	int	size;
{
	int	rval;
	obj_t	obj;
	attr_t	attr;
	u_int   tagnum;

	/* Make sure acl_config structure is initialized */
	if (acl_init () == -1) {
		errno = EINVAL;
		sec_errno = ESEC_ACL_CONFIG_FAILURE;
		return (-1);
	}
#if SEC_ACL_SWARE
        tagnum = ACL_OBJ_TAG;
#endif

#if SEC_ACL_POSIX
        tagnum = PACL_OBJ_ACCESS_TAG;
#endif


	attr.ir = (char *) acl;
	attr.ir_length = size * sizeof (acle_t);
	obj.o_semid = semid;
	rval = getlabel(acl_config.policy, tagnum, &attr,
		OT_SEMAPHORE, &obj);
	if (rval == 0)
		switch (attr.code) {
		case SEC_WILDCARD_TAG:
			rval = -1;
			errno = EINVAL;
			sec_errno = ESEC_WILDCARD_TAG;
			break;
		case SEC_ACTUAL_TAG:
			rval = attr.ir_length / sizeof(acle_t);
			break;
		}
	return rval;
}

/*
 * Change the ACL associated with a semaphore
 */

sem_chacl (semid, acl, size)
	int	semid;
	acle_t	*acl;
	int	size;
{
	obj_t	obj;
	attr_t	attr;
	u_int   tagnum;

	/* Make sure the acl_config structure is initialized */
	if (acl_init () == -1) {
		errno = EINVAL;
		sec_errno = ESEC_ACL_CONFIG_FAILURE;
		return (-1);
	}
#if SEC_ACL_SWARE
        tagnum = ACL_OBJ_TAG;
#endif

#if SEC_ACL_POSIX
        tagnum = PACL_OBJ_ACCESS_TAG;
#endif

	if (acl == ACL_DELETE) {
		attr.code = SEC_WILDCARD_TAG;
		attr.ir = (char *) 0;
		attr.ir_length = 0;
	} else {
		attr.code = SEC_ACTUAL_TAG;
		attr.ir = (char *) acl;
		attr.ir_length = size * sizeof (acle_t);
	}

	obj.o_semid = semid;

	return setlabel(acl_config.policy, tagnum,
		&attr, OT_SEMAPHORE, &obj);
}
#endif

