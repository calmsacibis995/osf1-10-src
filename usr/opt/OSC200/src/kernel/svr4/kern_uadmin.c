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
/*  Module Name:
 *	kern_uadmin.c
 *  Description:
 *	implements the SVR4 uadmin() system call
*/

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/reboot.h>
#include <sys/uswitch.h>
#include <sys/uadmin.h>

#include <sys/habitat.h>

/*
 * Function Name:
 *	uadmin()
 *
 * Description:
 *	Implements SVR4 uadmin() system call.
 *
 * Inputs:
 *	current proc pointer
 *	system call args
 *	pointer to system call return value array
 *
 * Outputs:
 *	None
 *
 * Return value:
 * 	Normally doesn't return, because system is rebooted.
 *	Returns a Failure with the ret_val value set to not-implemented
 * 	if the REMOUNT option is requested, as this option is not
 *	supported.  (syscall() sets errno based on the ret_val value).
 *
 * Called Functions:
 *	Calls the reboot() system call to do the work of rebooting the system.
 *
 * Called by:
 *	syscall()
 *
 * Side effects:
 *	reboots system
 *	OR None
 *	
 * Notes:
 *	There appears to be not support for remounting the root file
 *	system in OSF/1.  uadmin(2) is not supported in VAX SVR3, and
 *	there is no reference to remounting in the VAX SVR3 permuted
 *	index.  uadmin(2), including the REMOUNT option, does exist in
 *	SCO SVR3.  This information leads me to believe that it is not
 *	necessary to REMOUNT option of uadmin() in the DEC OSF/1
 *	release.
 *
 * Dependencies:
 *	Depends on the kernel reboot() system call.
 *
 */
uadmin(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
#ifdef	__alpha
		long cmd;
		long fcn;  /* options for the A_SHUTDOWN or A_REBOOT command */
		long mdep; /* machine dependent arg -- ignored in this
			   * implementation
			   */
#else	/* __alpha */
		int cmd;
		int fcn;  /* options for the A_SHUTDOWN or A_REBOOT command */
		int mdep; /* machine dependent arg -- ignored in this
			   * implementation
			   */
#endif	/* __alpha */
	} *uap = (struct args *) args;

	struct reboot_args {
		int	opt;
	} reboot_args;

	if( uap->cmd == A_REMOUNT ) {
		/* not supported */
		return(ENOSYS);
	}	


	/* check fcn values for reboot commands */
	if( uap->fcn != AD_HALT && uap->fcn != AD_BOOT && uap->fcn != AD_IBOOT )
		return(EINVAL);

	reboot_args.opt = 0;	/* clear out this variable */

	switch(uap->cmd) {

	case A_REBOOT:
		/* no further processing before reboot */
		reboot_args.opt |= RB_NOSYNC;
		/* fall through */

	case A_SHUTDOWN:
		switch(uap->fcn) {
		case AD_HALT:
			/*
			 * hardware doesn't support turning off the power,
			 * so just reboot to monitor
			 */
			/* fall through */
		case AD_IBOOT:
			/* interactive reboot - leave at monitor prompt */
			reboot_args.opt |= RB_HALT;
			break;
		case AD_BOOT:
			/* boot default unix */
			break;
		default:
		 	return(EINVAL);
		}
		break;

	default:
		return(EINVAL);

	}

	return( reboot(p,&reboot_args,retval) );
}
