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
static char	*sccsid = "@(#)$RCSfile: method_s5fs.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:03:45 $";
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
extern int		SVFS_method();

/*
 *      Local BSS
 */
const static char 	s5fs_name[] = "Method s5fs";
int			s5fs_configured;	
int			s5fs_loaded;
kmod_id_t		s5fs_id;


#define	reterr(x)	do { \
			cm_log(logp, LOG_ERR, "%s: %s", s5fs_name, cm_msg(x)); \
			return(-1);	\
			} while(0)
/*
 *
 *	Name:		s5fs_method()
 *	Description:	System V file system Configuration Method 
 *	Returns:	Zero 		On success.
 *			Non-zero	On failure.
 *
 */

int
SVFS_method( cm_log_t * logp, ENT_t entry, cm_op_t op, cm_op_t *rop,
	char *opts )
{
	int	rc;

	switch (op) {

	case CM_OP_LOAD:
		if (s5fs_loaded)
			reterr(KMOD_EALREADY);
		if ((rc=cm_kls_load(entry, &s5fs_id)) != 0)
			reterr(rc);
		s5fs_loaded = 1;
		break;

	case CM_OP_UNLOAD:
		if (!s5fs_loaded)
			reterr(KMOD_EEXIST);
		if ( s5fs_configured )
			reterr(KMOD_EBUSY);
		if ((rc=cm_kls_unload(s5fs_id)) != 0)
			reterr(rc);
		s5fs_id = LDR_NULL_MODULE;
		s5fs_loaded = 0;
		break;

	case CM_OP_CONFIGURE:
		if (!s5fs_loaded)
			reterr(KMOD_EEXIST);
		if (s5fs_configured)
			reterr(KMOD_EBUSY);
		if ((rc=s5fs_method_configure(logp, entry, s5fs_id)) != 0)
			reterr(rc);
		s5fs_configured = 1;
		break;

	case CM_OP_UNCONFIGURE:
		if (!s5fs_loaded)
			reterr(KMOD_EEXIST);
		if (!s5fs_configured)
			reterr(KMOD_ECEXIST);
		if ((rc=s5fs_method_unconfigure(logp, s5fs_id)) != 0)
			reterr(rc);
		s5fs_configured = 0;
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
s5fs_method_configure( cm_log_t * logp, ENT_t entry, kmod_id_t s5fs_id )
{
	int	rc;

	cm_log(logp, LOG_DEBUG, "%s: Configuring s5fs", s5fs_name);

        if ((rc=cm_kls_call(s5fs_id, SYSCONFIG_CONFIGURE, NULL, 0, 
		NULL, 0)) != 0) {
		return(rc);
	}
	cm_log(logp, LOG_DEBUG, "%s: Configured s5fs", s5fs_name);
	return(0);
}


/*
 *
 */
int
s5fs_method_unconfigure( cm_log_t * logp, kmod_id_t s5fs_id )
{
	int	rc;

        if ((rc=cm_kls_call(s5fs_id, SYSCONFIG_UNCONFIGURE, NULL, 0, 
		NULL, 0)) != 0) {
		return(rc);
	}
	cm_log(logp, LOG_DEBUG, "%s: Deconfigured s5fs", s5fs_name);
	return(0);
}
