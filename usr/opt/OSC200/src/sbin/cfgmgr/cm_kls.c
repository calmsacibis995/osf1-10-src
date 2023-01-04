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
static char	*sccsid = "@(#)$RCSfile: cm_kls.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:32:44 $";
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

#include <stdio.h>
#include <sys/syscall.h>
#include <sys/sysconfig.h>
#include <stdlib.h>
#include <errno.h>

#include "cfgmgr.h"
#include "cm.h"

/*
 *      MOD:	Connect to kernel loader server
 *	Return:		0		Success
 *			KLDR_EATTACH	Failure
 */
int
cm_kls_attach( void )
{
	ldr_process_t	kern_proc;
	int		rc;

        kern_proc = ldr_kernel_process();
        if (rc=ldr_xattach(kern_proc)) {
		cfgmgr_log(LOG_DEBUG, "ldr_xattach: %s\n", strerror(abs(rc)));
		return(KLDR_EATTACH);
	}
	return(0);
}


/*
 *      MOD:	Disconnect from kernel loader server
 *	Return:		0		Success
 */
void
cm_kls_detach( void )
{
	ldr_process_t	kern_proc;

        kern_proc = ldr_kernel_process();
        ldr_xdetach(kern_proc);
        return;
}


/*
 *	MOD:
 *	Return:		0		Success
 *			KLDR_ENOTKMOD	Failure
 *			KLDR_ELOAD	Failure
 */
int
cm_kls_load( ENT_t entry, kmod_id_t * kmod_idp )
{
	ldr_process_t		kern_proc;
        ldr_load_flags_t        kmod_flgs;
        char *                  kmod_path;
        int                     type;
        int                     rc;

        kern_proc = ldr_kernel_process();

	type = dbattr_match_type(entry, MODULE_TYPE, TYPE_LIST);
	kmod_flgs = dbattr_match_type(entry, MODULE_FLAGS,LFLAGS_LIST );
        kmod_path = AFgetval(AFgetatr(entry, MODULE_PATH));

	if (type != TYPE_DYNAMIC)
		return(KLDR_ENOTKMOD);

	if (rc=ldr_xload(kern_proc, kmod_path, kmod_flgs, kmod_idp)) {
		cfgmgr_log(LOG_DEBUG, "ldr_xload: %s: %s\n", kmod_path,
			strerror(abs(rc)));
		return(KLDR_ELOAD);
	}

	return(0);

}


/*
 *	MOD:
 *	Return:		0		Success
 *			KLDR_EUNLOAD	Failure
 */
int
cm_kls_unload( ldr_module_t kmod_id )
{
	ldr_process_t	kern_proc;
	int		rc;

        kern_proc = ldr_kernel_process();
        if (rc=ldr_xunload(kern_proc, kmod_id)) {
		cfgmgr_log(LOG_DEBUG, "ldr_xunload: %s\n", strerror(abs(rc)));
		return(KLDR_EUNLOAD);
	}
	return(0);
}


/*
 *	MOD:
 *	Return:		0		Success
 *			KLDR_EFAIL	Failure
 *	Note:		The global errno variable should be set to an
 *			appropriate value on return.
 *
 */
int
cm_kls_call( 	kmod_id_t 	kmod_id,
		sysconfig_op_t	op,
		caddr_t 	inbuf,
		int 		insiz,
		caddr_t 	outbuf,
		int * 		outsiz )
{
	ldr_process_t	kern_proc;
        ldr_entry_pt_t	entrypt;
	int		rc;

        kern_proc = ldr_kernel_process();
	if (rc=ldr_xentry( kern_proc, kmod_id, &entrypt)) {
		cfgmgr_log(LOG_DEBUG, "ldr_xentry: %s\n", strerror(abs(rc)));
		return(KLDR_NOENT);
	}

	if (syscall(SYS_kmodcall, entrypt, op, inbuf, insiz, outbuf, outsiz))
		return(KLDR_EFAIL);

	return(0);
}

