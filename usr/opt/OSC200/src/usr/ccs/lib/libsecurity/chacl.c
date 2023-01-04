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
static char *rcsid = "@(#)$RCSfile: chacl.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:22:15 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	chacl.c,v $
 * Revision 1.1.1.2  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.6.3.2  1992/04/08  20:30:15  marquard
 * 	Added POSIX ACL support.
 * 	[1992/04/05  14:05:01  marquard]
 *
 * Revision 1.6  1991/03/04  17:43:13  devrcs
 * 	Comment out ident directives
 * 	[91/01/31  08:54:20  lehotsky]
 * 
 * Revision 1.5  91/01/07  15:57:01  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:26:39  dwm]
 * 
 * Revision 1.4  90/10/07  20:05:44  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:12:35  gm]
 * 
 * Revision 1.3  90/07/17  12:18:31  devrcs
 * 	Internationalized
 * 	[90/07/05  07:22:19  staffan]
 * 
 * Revision 1.2  90/06/22  21:46:13  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:01:38  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.
 * All rights reserved.
 */

/* #ident "@(#)chacl.c	2.1 16:15:46 4/20/90 SecureWare" */
/* #ident "@(#)chacl.c	2.1 11:13:27 1/25/89 SecureWare" */

#include <sys/secdefines.h>

#if SEC_BASE && SEC_ACL

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#include <sys/secpolicy.h>
#include <acl.h>

extern int errno;
extern int sec_errno;

/*
 * Change the ACL associated with a file
 */

chacl (path, acl, size)
	char	*path;
	acle_t	*acl;
	int	size;
{
	obj_t	obj;
	attr_t	attr;
	u_int   tagnum;

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

	obj.o_file = path;

	return setlabel(acl_config.policy, tagnum,
		&attr, OT_REGULAR, &obj);
}
#endif
