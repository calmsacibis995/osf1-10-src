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
static char *rcsid = "@(#)$RCSfile: ult_bin_cfg.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:04:09 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)ult_bin_cfg.c	3.2	(ULTRIX/OSF)	3/29/91";
#endif 

/*
 * Ultrix 4.2 - OSF/1 Binary compatability
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
#include <sys/ult_syscall.h>
#include "ult_kmod.h"

#define STATIC	static


/*
 * External references
 */
extern  int	uprintf();

extern	struct sysent ult_sysent[];	/* new sysent vector	*/
extern	char 	*ult_syscallnames[];	/* syscall names	*/


/*
 * Module data
 */
STATIC struct compat_mod * ult_recognizer();
STATIC struct sysent     * ult_syscall();
int 	                   ult_bin_configure();

struct compat_mod ult_compat_mod;

/* System call counters. 
 *	These are incremented on each system call that is serviced. The
 *	counters are cleared on a "clear" command */
STATIC int ult_stats[ult_LAST - ult_FIRST + 1];

/* trace flags. When a flag is set and the CM_TRACE flag is in cm_flags
 * the system call is traced */
STATIC char	ult_trace[ult_LAST - ult_FIRST + 1];


/*
 * ult_cfg_static
 * 	Configure statically linked module. (called by cm_init())
 */
void
ult_cfg_static( void )
{
	ult_bin_configure(SYSCONFIG_CONFIGURE,"ult_bin",7,0,0);
	ult_compat_mod.cm_flags |= CM_STATIC;	/* can't unload */
}



/* 
 * ult_bin_configure
 *	This is the configuration function that is invoked by the 
 *	configuration manager daemon using an svc.
 */
int
ult_bin_configure(op,indata,indatalen,outdata,outdatalen)
     sysconfig_op_t op;		/* command */
     int *indata; 
     size_t indatalen;
     int *outdata; 
     size_t outdatalen;
{
	int ret_val = 0, i;
	struct compat_mod * mod_ptr = &ult_compat_mod;


	switch (op) {
	      case SYSCONFIG_CONFIGURE:
		/* The cfgmgr provides the stanza name when it
		 * configures the module */
		if(indatalen > MAXCOMPATNAMSZ || indatalen <= 0)
			return(EINVAL);

		/* Initialize the compat_mod struct */
		if(ret_val = cm_setup(mod_ptr, (char *)indata, indatalen, 
			ULTBIN, ULT42V11A, ULT42V11, 
			ult_bin_configure, ult_recognizer, ult_syscall, 
			ult_syscallnames, ult_stats, ult_trace, 
			0, ult_FIRST, ult_LAST)) {
			return(ret_val);
		}

		ret_val = cm_add(mod_ptr);
		if(ULT_DEBUG)
			printf("ult_bin configure done %d\n", ret_val);
		break;

	      case SYSCONFIG_UNCONFIGURE:
		if(ULT_DEBUG)
			printf("ult_bin 4.2 being un-configured\n");

		ret_val = cm_del(mod_ptr);

		break;

	      case SYSCONFIG_QUERY:
		if(ULT_DEBUG)
			printf("ult_bin 4.2 being queried\n");

		ret_val = cm_query(mod_ptr, indata, indatalen, outdata,
			outdatalen);

		break;

	      case SYSCONFIG_OPERATE:
		if(ULT_DEBUG)
			printf("ult_bin 4.2 being operated\n");

		ret_val = cm_operate(mod_ptr, indata, indatalen, outdata,
			outdatalen);

		break;

	      default:
		ret_val = EINVAL;
	}
	return(ret_val);
}


/*
 * ult_recognizer
 *	This function detects an Ultrix execuitable by looking for 
 *	a vstamp value of 0x20a.
 */
STATIC
struct compat_mod *
ult_recognizer(hdr, aux, vp)
struct filehdr *hdr;
struct aouthdr *aux;
struct vnode   *vp;
{
	if(ULT_DEBUG && aux->vstamp <= 0x20a)
		printf("(%s %d) ult_recognizer: vstamp 0x%x\n",
			u.u_comm, u.u_procp->p_pid, aux->vstamp);

	return (((aux->vstamp <= 0x20a) ) ? &ult_compat_mod : 0);
}



/*
 * ult_syscall
 *	This function returns a pointer to the needed sysent struct. 
 *	If the index is out of range it returns 0.
 */
STATIC
struct sysent *
ult_syscall(indx)
int	indx;
{
struct sysent * ret = 0;

	if (indx >= ult_FIRST && indx <=  ult_LAST) {
		/* decide if we are tracing this call.
		 *  CM_TRACE must be set. 
		 *  skipcount must be 0.
		 */
		if(cm_trace_this(&ult_compat_mod)) {
			/* These have special trace code in the 
			 * function */
			switch(indx - ult_FIRST){
			case SYS_ult_open:		/*open*/
			case SYS_ult_close:		/*close*/
			case SYS_ult_obreak:		/*obreak*/
			case SYS_ult_ioctl:		/*ioctl*/
			case SYS_ult_fcntl:		/*fcntl*/
			case SYS_ult_waitpid:		/*waitpid*/
			case SYS_ult_stat:		/*stat*/
			case SYS_ult_lstat:		/*lstat*/
			case SYS_ult_getrusage:		/*getrusage*/
			case SYS_ult_execv:		/*execv*/
			case SYS_ult_execve:		/*execve*/
			case SYS_ult_fstat:		/*ofstat*/
			case SYS_ult_semctl:		/*semctl*/
			case SYS_ult_semop:		/*semop*/
			case SYS_ult_shmsys:		/*shmsys*/
			    if(ult_trace[indx-ult_FIRST])
				ult_compat_mod.cm_flags |= CM_TRACE_THIS;
				break;
			default:
			    if(ult_trace[indx-ult_FIRST])
				printf("(%s %d) %s {%d}\n", 
				    u.u_comm, u.u_procp -> p_pid, 
				    ult_compat_mod.call_name[indx - ult_FIRST],
				    indx);
			}
		}

		/* count the call */
		ult_stats[indx - ult_FIRST] ++ ;
		ult_compat_mod.cm_nsyscalls ++ ;

		/* get the sysent pointer */
		ret = &ult_sysent[indx - ult_FIRST];
	}
	return(ret);
}


