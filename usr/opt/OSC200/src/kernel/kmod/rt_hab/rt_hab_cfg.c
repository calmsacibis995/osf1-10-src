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
static char *rcsid = "@(#)$RCSfile: rt_hab_cfg.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 16:34:54 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)rt_hab_cfg.c	3.2	(ULTRIX/OSF)	3/29/91";
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
 * Real-time  habitat - OSF/1 Binary compatability
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
#include <sys/rt_syscall.h>
#include "rt_kmod.h"

#define STATIC	static



/*
 * External references
 */
extern  int	uprintf();

extern	struct sysent rt_sysent[];	/* new sysent vector	*/
extern	char 	*rt_syscallnames[];	/* syscall names	*/


/*
 * Module data
 */
STATIC struct compat_mod * rt_recognizer();
STATIC struct sysent     * rt_syscall();
int 	                   rt_hab_configure();

STATIC struct compat_mod rt_compat_mod;

/* System call counters. 
 *	These are incremented on each system call that is serviced. The
 *	counters are cleared on a "clear" command */
STATIC int rt_stats[rt_LAST - rt_FIRST + 1];

/* trace flags. When a flag is set and the CM_TRACE flag is in cm_flags
 * the system call is traced */
STATIC char	rt_trace[rt_LAST - rt_FIRST + 1];


/*
 * rt_cfg_static
 * 	Configure statically linked module. (called by cm_init())
 */
void
rt_cfg_static( void )
{
	rt_hab_configure(SYSCONFIG_CONFIGURE,"rt_hab",6,0,0);
	rt_compat_mod.cm_flags |= CM_STATIC;	/* can't unload */
}



/* 
 * rt_hab_configure
 *	This is the configuration function that is invoked by the 
 *	configuration manager daemon using an svc.
 */
int
rt_hab_configure(op,indata,indatalen,outdata,outdatalen)
     sysconfig_op_t op;		/* command */
     int *indata; 
     size_t indatalen;
     int *outdata; 
     size_t outdatalen;
{
	int ret_val = 0, i;
	struct compat_mod * mod_ptr = &rt_compat_mod;


	switch (op) {
	      case SYSCONFIG_CONFIGURE:
		/* The cfgmgr provides the stanza name when it
		 * configures the module */
		if(indatalen > MAXCOMPATNAMSZ || indatalen <= 0)
			return(EINVAL);

		/* Initialize the compat_mod struct */
		if(ret_val = cm_setup(mod_ptr, (char *)indata, indatalen, 
			RT_HAB, RTV11A, RTV11, 
			rt_hab_configure, 0, rt_syscall, 
			rt_syscallnames, rt_stats, rt_trace, 
			RT_HAB_NO, rt_FIRST, rt_LAST)) {
			return(ret_val);
		}

		ret_val = cm_add(mod_ptr);
		if(RT_DEBUG)
			printf("rt_hab configure done %d\n", ret_val);
		break;

	      case SYSCONFIG_UNCONFIGURE:
		if(RT_DEBUG)
			printf("rt_hab being un-configured\n");

		ret_val = cm_del(mod_ptr);

		break;

	      case SYSCONFIG_QUERY:
		if(RT_DEBUG)
			printf("rt_hab being queried\n");

		ret_val = cm_query(mod_ptr, indata, indatalen, outdata,
			outdatalen);

		break;

	      case SYSCONFIG_OPERATE:
		if(RT_DEBUG)
			printf("rt_hab being operated\n");

		ret_val = cm_operate(mod_ptr, indata, indatalen, outdata,
			outdatalen);

		break;

	      default:
		ret_val = EINVAL;
	}
	return(ret_val);
}




/*
 * rt_syscall
 *	This function returns a pointer to the needed sysent struct. 
 *	If the index is out of range it returns 0.
 */
STATIC
struct sysent *
rt_syscall(indx)
int	indx;
{
struct sysent * ret = 0;

	if (indx >= rt_FIRST && indx <=  rt_LAST) {
		/* decide if we are tracing this call.  */
		if(cm_trace_this(&rt_compat_mod)) {
			printf("(%s %d) %3d %s {%d}\n", 
			    u.u_comm, u.u_procp -> p_pid, 
			    rt_compat_mod.call_name[indx - rt_FIRST],
			    indx);
		}

		/* count the call */
		rt_stats[indx - rt_FIRST] ++ ;
		rt_compat_mod.cm_nsyscalls ++ ;

		/* get the sysent pointer */
		ret = &rt_sysent[indx - rt_FIRST];
	}
	return(ret);
}


