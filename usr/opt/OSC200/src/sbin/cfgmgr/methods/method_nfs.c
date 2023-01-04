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
static char	*sccsid = "@(#)$RCSfile: method_nfs.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:03:30 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysconfig.h>

#include "cm.h"


/*
 *      Export Static Method Entry Point
 */
extern int              NFS_method();

/*
 *      Local BSS
 */
const static char 	nfs_name[] = "Method nfs";
int			nfs_configured;	
int			nfs_loaded;
kmod_id_t		nfs_id;

#define	reterr(x)	do { \
			cm_log(logp, LOG_ERR, "%s: %s", nfs_name, cm_msg(x)); \
			return(-1);	\
			} while (0)
/*
 *
 *	Name:		nfs_method()
 *	Description:	NFS Configuration Method 
 *	Returns:	Zero 		On success.
 *			Non-zero	On failure.
 *
 */
int
NFS_method( cm_log_t * logp, ENT_t entry, cm_op_t op, cm_op_t *rop,
	char *opts ) 
{
	int	rc;

	switch (op) {

	case CM_OP_LOAD:
		if (nfs_loaded)
			reterr(KMOD_EALREADY);
		if ((rc=cm_kls_load(entry, &nfs_id)) != 0)
			reterr(rc);
		nfs_loaded = 1;
		break;

	case CM_OP_UNLOAD:
		if (!nfs_loaded)
			reterr(KMOD_EEXIST);
		if ( nfs_configured )
			reterr(KMOD_EBUSY);
		if ((rc=cm_kls_unload(nfs_id)) != 0)
			reterr(rc);
		nfs_id = LDR_NULL_MODULE;
		nfs_loaded = 0;
		break;

	case CM_OP_CONFIGURE:
		if (!nfs_loaded)
			reterr(KMOD_EEXIST);
		if (nfs_configured)
			reterr(KMOD_EBUSY);
		if ((rc=nfs_method_configure(logp, entry, nfs_id)) != 0)
			reterr(rc);
		nfs_configured = 1;
		break;

	case CM_OP_UNCONFIGURE:
		if (!nfs_loaded)
			reterr(KMOD_EEXIST);
		if (!nfs_configured)
			reterr(KMOD_ECEXIST);
		if ((rc=nfs_method_unconfigure(logp, nfs_id)) != 0)
			reterr(rc);
		nfs_configured = 0;
		break;

	case CM_OP_QUERY:
	default:
		reterr(KMOD_EINVAL);
	}
	return(0);
}


/*
 *
 */
int
nfs_method_configure( cm_log_t * logp, ENT_t entry, kmod_id_t nfs_id )
{
	int	rc;

	cm_log(logp, LOG_DEBUG, "%s: Configuring nfs", nfs_name);

        if ((rc=cm_kls_call(nfs_id, SYSCONFIG_CONFIGURE, NULL, 0, 
		NULL, 0)) != 0) {
		return(rc);
	}
	cm_log(logp, LOG_DEBUG, "%s: Configured nfs", nfs_name);
	return(0);
}


/*
 *
 */
int
nfs_method_unconfigure( cm_log_t * logp, kmod_id_t nfs_id )
{
	int	rc;

        if ((rc=cm_kls_call(nfs_id, SYSCONFIG_UNCONFIGURE, NULL, 0, 
		NULL, 0)) != 0) {
		return(rc);
	}
	cm_log(logp, LOG_DEBUG, "%s: Deconfigured nfs", nfs_name);
	return(0);
}
