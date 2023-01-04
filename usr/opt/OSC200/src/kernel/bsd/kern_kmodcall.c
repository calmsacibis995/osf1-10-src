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
static char	*sccsid = "@(#)$RCSfile: kern_kmodcall.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 23:46:41 $";
#endif 
/*
 */
/* @(#)kern_kmodcall.c  3.1 08:27:21 2/26/91 SecureWare */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#include <sys/secdefines.h>
#include <sys/security.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/sysconfig.h>

/*
 * NAME:	kmodcall()
 *
 * FUNCTION:	Calls a kernel loaded subsystem's {}configure() entry point
 *		Passing an input and output buffers, akin to ioctl().
 */
kmodcall(p, args, retval)
        register struct proc *p;
	void *args;
	long *retval;
{
        struct args {
	    sysconfig_entrypt_t	entrypt; /* Configuration entry pt address */
	    long	op;		/* Configuration command request */
	    caddr_t	in_buf;		/* Input data buffer */
	    u_long   	in_len;		/* Input data buffer length */
	    caddr_t	out_buf;	/* Output data buffer */
	    u_long 	out_len;	/* Output data buffer length */
        } *uap = (struct args *)args;
        caddr_t in_memp = NULL;		/* Auxillary input data kernel buffer */
        caddr_t out_memp = NULL;	/* Auxillary output data kernel buffer */
#define STK_PARMS       128		
        char in_stkbuf[STK_PARMS];	/* Static input data buffer */
        char out_stkbuf[STK_PARMS];	/* Static output data buffer */
        caddr_t in_data = in_stkbuf;	/* Input data buffer */
        caddr_t out_data = out_stkbuf;	/* Output data buffer */
	size_t	in_len = (size_t)uap->in_len;	/* Input data buffer length */
	size_t	out_len = (size_t)uap->out_len;	/* Output data buffer length */
	int	error;

#if SEC_BASE
						/* Must have privilege */
	if (!privileged(SEC_DEBUG, EPERM))	/* XXX need new priv */
		return (EPERM);
#else
						/* Must be super user */
        if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif
						/* Sanity check entry pt */
	if (uap->entrypt == NULL) 
		return (EINVAL);

	/*
	 * Create and copyin input buffer
	 */
        if (in_len > SYSCONFIG_PARAM_MAX)
		return (EINVAL);
        if (in_len > sizeof(in_stkbuf)) {
		if ((in_memp = (caddr_t) kalloc(in_len)) == NULL)
			return (ENOMEM);
		in_data = in_memp;
	}
	if (in_len)
		if (error=copyin(uap->in_buf, (caddr_t)in_data, in_len))
			goto out;

	/*
	 * Create and initialize output buffer
	 */
        if (out_len > SYSCONFIG_PARAM_MAX) {
		error = EINVAL;
		goto out;
        }
        if (out_len > sizeof(out_stkbuf)) {
		if ((out_memp = (caddr_t) kalloc(out_len)) == NULL) {
			error = ENOMEM;
			goto out;
		}
		out_data = out_memp;
	}
	if (out_len)
		bzero((caddr_t)out_data, out_len);

	/*
	 * Call configure{} entry point, passing input and output buffers
	 */
	error = (*uap->entrypt)((sysconfig_op_t)uap->op,
				in_data, in_len, 
				out_data, out_len);

	/*
	 * If {}configure has no error, copy data
	 */
	if (error == 0) 
		copyout((caddr_t)out_data, (caddr_t)uap->out_buf, out_len);

out:
        if (in_memp)	
		kfree(in_memp, in_len);
        if (out_memp)	
		kfree(out_memp, out_len);
	return (error);
}

