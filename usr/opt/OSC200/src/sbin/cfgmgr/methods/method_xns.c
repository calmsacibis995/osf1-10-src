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
static char	*sccsid = "@(#)$RCSfile: method_xns.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:04:12 $";
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

#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysconfig.h>
#include <netns/ns_config.h>

#include "cm.h"

/*
 *      Local BSS
 */
int			XNS_configured;
int			XNS_loaded;
kmod_id_t		XNS_id;


/*
 *
 *	Name:		XNS_method()
 *	Description:	Inet Configuration Method
 *	Returns:	Zero 		On success.
 *			Non-zero	On failure.
 *
 */
int
XNS_method( cm_log_t * logp, ENT_t entry, cm_op_t op, cm_op_t * rop,
	char *opts )
{
	int	rc;

	rc = 0;
	if (op & CM_OP_LOAD) {
		rc = XNS_method_load(logp, entry);
		if (rc == 0) {
			*rop = CM_OP_LOAD;
			METHOD_LOG(LOG_INFO, MSG_LOADED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}

	if (op & CM_OP_CONFIGURE) {
		rc = XNS_method_configure(logp, entry);
		if (rc == 0) {
			*rop = CM_OP_CONFIGURE;
			METHOD_LOG(LOG_INFO, MSG_CONFIGURED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}

	if (op & CM_OP_UNCONFIGURE) {
		rc = XNS_method_unconfigure(logp, entry);
		if (rc == 0) {
			*rop = CM_OP_UNCONFIGURE;
			METHOD_LOG(LOG_INFO, MSG_UNCONFIGURED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}

	if (op & CM_OP_UNLOAD) {
		rc = XNS_method_unload(logp, entry);
		if (rc == 0) {
			*rop = CM_OP_UNLOAD;
			METHOD_LOG(LOG_INFO, MSG_UNLOADED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}
	return(rc == 0 ? 0 : -1);
}


/*
 *
 */
int
XNS_method_load( cm_log_t * logp, ENT_t entry )
{
	int	rc;

	if (XNS_loaded)
		return(KMOD_LOAD_L_EBUSY);
	if ((rc=cm_kls_load(entry, &XNS_id)) != 0)
		return(rc);
	XNS_loaded = 1;
	return(0);
}


/*
 *
 */
int
XNS_method_unload( cm_log_t * logp, ENT_t entry )
{
	int	rc;

	if (!XNS_loaded)
		return(KMOD_UNLOAD_L_EEXIST);
	if (XNS_configured)
		return(KMOD_UNLOAD_C_EBUSY);
	if ((rc=cm_kls_unload(XNS_id)) != 0)
		return(rc);
	XNS_id = LDR_NULL_MODULE;
	XNS_loaded = 0;
	return(0);
}


/*
 *
 */
int
XNS_method_configure( cm_log_t * logp, ENT_t entry )
{
	int	rc;

	if (!XNS_loaded)
		return(KMOD_CONFIG_L_EEXIST);
	if (XNS_configured)
		return(KMOD_CONFIG_C_EBUSY);
        if ((rc=cm_kls_call(XNS_id, SYSCONFIG_CONFIGURE, NULL, 0,
		NULL, 0)) != 0)
		return(rc);
	XNS_configured = 1;
	return(0);
}


/*
 *
 */
int
XNS_method_unconfigure( cm_log_t * logp, ENT_t entry )
{
	int	rc;

	if (!XNS_loaded)
		return(KMOD_UNCONFIG_L_EEXIST);
	if (!XNS_configured)
		return(KMOD_UNCONFIG_C_EEXIST);
        if ((rc=cm_kls_call(XNS_id, SYSCONFIG_UNCONFIGURE, NULL, 0,
		NULL, 0)) != 0) {
		return(rc);
	}
	XNS_configured = 0;
	return(0);
}

