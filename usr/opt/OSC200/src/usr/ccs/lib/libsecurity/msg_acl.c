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
static char *rcsid = "@(#)$RCSfile: msg_acl.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:24:18 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	msg_acl.c,v $
 * Revision 1.1.1.2  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.6.3.2  1992/04/08  20:30:24  marquard
 * 	Added POSIX ACL support.
 * 	[1992/04/05  14:05:12  marquard]
 *
 * Revision 1.6  1991/03/04  17:44:45  devrcs
 * 	Comment out ident directives
 * 	[91/01/31  08:57:15  lehotsky]
 * 
 * Revision 1.5  91/01/07  15:59:07  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:29:44  dwm]
 * 
 * Revision 1.4  90/10/07  20:08:02  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:16:10  gm]
 * 
 * Revision 1.3  90/07/17  12:20:40  devrcs
 * 	Internationalized
 * 	[90/07/05  07:27:56  staffan]
 * 
 * Revision 1.2  90/06/22  21:47:36  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:34:39  staffan]
 * 
 * $OSF_EndLog$
 */
/* Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved.
 *
 * Subroutine that implement access control list changes on
 * message queues.
 */

/* #ident "@(#)msg_acl.c	2.1 16:15:48 4/20/90 SecureWare" */
/* #ident "@(#)msg_acl.c	2.1 11:13:31 1/25/89 SecureWare, Inc." */

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
 * Get the ACL associated with a message queue.
 */

msg_statacl (msqid, acl, size)
	int	msqid;
	acle_t	*acl;
	int	size;
{
	int		rval;
	obj_t		obj;
	attr_t		attr;
	u_int           tagnum;

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


	attr.ir_length = size * sizeof (acle_t);
	attr.ir = (char *) acl;

	obj.o_msgid = msqid;

	rval = getlabel(acl_config.policy, tagnum, &attr,
			OT_MESSAGE_QUEUE, &obj);
	if (rval == 0)
		switch (attr.code) {
		case SEC_WILDCARD_TAG:
			errno = EINVAL;
			sec_errno = ESEC_WILDCARD_TAG;
			rval = -1;
			break;
		case SEC_ACTUAL_TAG:
			rval = attr.ir_length / sizeof(acle_t);
			break;
		}
	return rval;
}

/*
 * Change the ACL associated with a message queue
 */

msg_chacl (msqid, acl, size)
	int	msqid;
	acle_t	*acl;
	int	size;
{
	obj_t		obj;
	attr_t		attr;
	u_int           tagnum;

	/* Make sure acl_config structure is initialized */
	if (acl_init() == -1) {
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

	obj.o_msgid = msqid;

	return setlabel(acl_config.policy, tagnum, &attr,
			    OT_MESSAGE_QUEUE, &obj);
}
#endif
