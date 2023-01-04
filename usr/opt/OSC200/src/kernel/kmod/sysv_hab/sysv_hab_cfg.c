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
static char *rcsid = "@(#)$RCSfile: sysv_hab_cfg.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/10/01 22:30:16 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)sysv_hab_cfg.c	3.2	(ULTRIX/OSF)	3/29/91";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/*
 * System V habitat - OSF/1 Binary compatability
 *	Configuration functions
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/conf.h>
#include <sys/exec_incl.h>
#include <sys/vm.h>
#include <sys/uio.h>
#include <sys/stream.h>	
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/sysconfig.h>
#include <sys/habitat.h>
#include <sys/sysv_syscall.h>
#include "sysv_kmod.h"

#define STATIC	static



/*
 * External references
 */
extern  int	uprintf();

extern	struct sysent sysv_sysent[];	/* new sysent vector	*/
extern	char 	*sysv_syscallnames[];	/* syscall names	*/


/*
 * Module data
 */
STATIC struct compat_mod * sysv_recognizer();
STATIC struct sysent     * sysv_syscall();
int 	                   sysv_hab_configure();

STATIC struct compat_mod sysv_compat_mod;

/* System call counters. 
 *	These are incremented on each system call that is serviced. The
 *	counters are cleared on a "clear" command */
STATIC int sysv_stats[sysv_LAST - sysv_FIRST + 1];

/* trace flags. When a flag is set and the CM_TRACE flag is in cm_flags
 * the system call is traced */
STATIC char	sysv_trace[sysv_LAST - sysv_FIRST + 1];


/*
 * sysv_cfg_static
 * 	Configure statically linked module. (called by cm_init())
 */
void
sysv_cfg_static( void )
{
	sysv_hab_configure(SYSCONFIG_CONFIGURE,"sysv_hab",8,0,0);
	sysv_compat_mod.cm_flags |= CM_STATIC;	/* can't unload */
}



/* 
 * sysv_hab_configure
 *	This is the configuration function that is invoked by the 
 *	configuration manager daemon using an svc.
 */
int
sysv_hab_configure(op,indata,indatalen,outdata,outdatalen)
     sysconfig_op_t op;		/* command */
     int *indata; 
     size_t indatalen;
     int *outdata; 
     size_t outdatalen;
{
	int ret_val = 0, i;
	struct compat_mod * mod_ptr = &sysv_compat_mod;


	switch (op) {
	      case SYSCONFIG_CONFIGURE:
		/* The cfgmgr provides the stanza name when it
		 * configures the module */
		if(indatalen > MAXCOMPATNAMSZ || indatalen <= 0)
			return(EINVAL);

		/* Initialize the compat_mod struct */
		if(ret_val = cm_setup(mod_ptr, (char *)indata, indatalen, 
			SYSV_HAB, SYSVV11A, SYSVV11, 
			sysv_hab_configure, 0, sysv_syscall, 
			sysv_syscallnames, sysv_stats, sysv_trace, 
			SYSV_HAB_NO, sysv_FIRST, sysv_LAST)) {
			return(ret_val);
		}

		ret_val = cm_add(mod_ptr);
		if(SYSV_DEBUG)
			printf("sysv_hab configure done %d\n", ret_val);
		break;

	      case SYSCONFIG_UNCONFIGURE:
		if(SYSV_DEBUG)
			printf("sysv_hab being un-configured\n");

		ret_val = cm_del(mod_ptr);

		break;

	      case SYSCONFIG_QUERY:
		if(SYSV_DEBUG)
			printf("sysv_hab being queried\n");

		ret_val = cm_query(mod_ptr, indata, indatalen, outdata,
			outdatalen);

		break;

	      case SYSCONFIG_OPERATE:
		if(SYSV_DEBUG)
			printf("sysv_hab being operated\n");

		ret_val = cm_operate(mod_ptr, indata, indatalen, outdata,
			outdatalen);

		break;

	      default:
		ret_val = EINVAL;
	}
	return(ret_val);
}




/*
 * sysv_syscall
 *	This function returns a pointer to the needed sysent struct. 
 *	If the index is out of range it returns 0.
 */
STATIC
struct sysent *
sysv_syscall(indx)
int	indx;
{
struct sysent * ret = 0;

	if (indx >= sysv_FIRST && indx <=  sysv_LAST) {
		/* decide if we are tracing this call.  */
		if(SYSV_DEBUG){
			if(cm_trace_this(&sysv_compat_mod)) {
				printf("(%s %d) %s {%d}\n", 
			    	u.u_comm, u.u_procp->p_pid, 
			    	sysv_compat_mod.call_name[indx - sysv_FIRST],
			    	indx);
			}
		}

		/* count the call */
		sysv_stats[indx - sysv_FIRST] ++ ;
		sysv_compat_mod.cm_nsyscalls ++ ;

		/* get the sysent pointer */
		ret = &sysv_sysent[indx - sysv_FIRST];
	}
	return(ret);
}


