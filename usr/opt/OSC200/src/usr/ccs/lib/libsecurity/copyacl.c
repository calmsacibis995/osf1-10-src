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
static char *rcsid = "@(#)$RCSfile: copyacl.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:22:21 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	copyacl.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.6.2  1992/07/21  15:33:07  hosking
 * 	bug 6530: move PACL version of copyacl() from cp.c here
 * 	[1992/07/21  15:30:18  hosking]
 *
 * Revision 1.1.4.2  1991/11/21  21:29:50  coren
 * 	copied from 1.0.4 tree.
 * 	[1991/11/21  21:08:47  coren]
 * 
 * Revision 1.1.2.2  1991/11/20  21:53:00  coren
 * 	Newly created, to copy ACLs for cp(1).
 * 	[1991/09/12  15:17:50  coren]
 * 
 * $OSF_EndLog$
 */

/*
 * Copyright (c) 1991, SecureWare, Inc.		All rights reserved.
 * 
 * copyacl.c - ACL support routines for applications
 *
 * copyacl(3)-routine designed to transfer an ACL from the specified source
 * file or directory to a target file or directory. Setting the ACL on an
 * object requires ownership of the object as well as write permission to the
 * object.
 */

#include <sys/secdefines.h>

#if SEC_ACL_POSIX
#include <sys/types.h>
#include <sys/security.h>
#include <sys/stat.h>
#include <acl.h>

/*
 * Copy an ACL of type 'acl_type' (ACL_TYPE_ACCESS or ACL_TYPE_DEFAULT)
 * from file 'source' to file 'target.'
 * Returns 0 for success, -1 for failure.  Called only by copyacl() (below)
 */

static
copyacl2(source, target, acl_type)
char *source, *target;
{
	acl_t	acl_buf;

	/* get a buffer to hold the acl */

	if (acl_alloc(&acl_buf)) {
		return(-1);
	}

	/* Get ACL from source file */

	acl_buf->acl_num = 0;
	if (acl_read (source, acl_type, acl_buf) == -1) {
		goto error;
	}

	/*
	 * If setting or getting a default acl and
	 * the directory currently does not have a
	 * default acl, initialize the base entries.
	 */

	if (acl_buf->acl_num == 0  && acl_type == ACL_TYPE_DEFAULT)
		if (get_wildcard_acl(acl_buf, source))
			goto error;

	/* Propagate acl from source file to target file */

	if ((acl_write (target, acl_type, acl_buf)) == 0) {
		acl_free(acl_buf);
		return(0);
	}

error:	acl_free(acl_buf);
	return(-1);
}

/*
 * Top level routine to propagate an ACL from one file to another
 */

int
copyacl(source, target, privs)
char *source, *target;
{
	struct stat statbuf;
	privvec_t savep;
	int ret;

	/* Raise privileges if requested to do so */

	if (privs)
		(void) forceprivs(privvec(SEC_OWNER,
			SEC_ALLOWDACACCESS, -1), savep);

	/* Try to propagate the access ACL from the source to the target. */

	ret = copyacl2(source, target, ACL_TYPE_ACCESS);
	if (ret)
		goto error;

	/* If source is a directory, also try to propagate default ACL. */

	ret = stat(source, &statbuf);

	if (ret)
		goto error;

	if (S_ISDIR(statbuf.st_mode)) {
		ret = copyacl2(source, target, ACL_TYPE_DEFAULT);
	}
error:
	/* Restore the caller's privileges (if required) and return status */

	if (privs)
		seteffprivs(savep, (priv_t *) 0);

	return(ret);
}
#endif /* SEC_ACL_POSIX */


