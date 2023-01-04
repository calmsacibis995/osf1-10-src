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
static char	*sccsid = "@(#)$RCSfile: sec_b1.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:52:29 $";
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
/*
 * Copyright (c) 1988 SecureWare, Inc.  All rights reserved.
 *
 * This Module contains Proprietary Information of SecureWare, Inc. and
 * should be treated as Confidential.
 */
/*

*/
/*

 */

#include <sec/include_sec>

#if SEC_ARCH
#ifndef _OSF_SOURCE

/*
 * 		COVERT CHANNEL SUPPRESSION
 *
 * The system parameters used for the seed are those that change
 * often enough to thwart a good guess by a user program as to
 * their values, and ones that change often enough to hope that
 * the value is different between successive calls to this
 * routine, even one very soon after another.  These system
 * variables, then, act as noise;  their high rates of change
 * relative to the rate of system calls significantly reduces
 * their roles as leaskage paths and enhances their noise value.
 * Note also that they are monotonically increasing, so that
 * values are not always bunched together.
 * NOTE: these values are extremely system-dependent and must be
 *       matched with system parameters available on each port.
 */
static long
noise_value(p)
	struct proc *p;
{
	return lbolt + (p - proc) + sysinfo.syscall +
	    sysinfo.outch + sysinfo.rawch + sysinfo.readch + sysinfo.writech +
	    sysinfo.bread + sysinfo.bwrite + sysinfo.phread + sysinfo.phwrite;
}


/*
 * 		STOPPING COVERT CHANNEL
 *
 * Return a new PID.  This PID is in the range:
 *	SEC_MINPID  <=  PID  <  max_pid
 * but has no relation to the previous PID chosen so that the PID
 * number cannot be used as a system variable for a covert storage
 * channel.
 *
 * The algorithm is to use semi-random values of system variables to
 * come up with a seed, and then fit it into the PID name space.  Although
 * some of the seed is under control of the user, others are not so that
 * neither the system or user totally controls the PID.  The prev_pid
 * argument is used to handle when succesive calls to sec_newpid() result
 * in the same value of all the other system variables because system
 * values have not changed enough.
 *
 * If the prev_pid is less than SEC_MINPID, we are setting up system processes
 * (like init) generated from the kernel.  Preserve those PIDs in sequential
 * fashion.
 */

sec_newpid(max_pid, prev_pid)
	int max_pid;
	int prev_pid;
{
	long seed;
	int pid_space;

	if (prev_pid < SEC_MINPID - 1)
		/*
		 * Initial system crafted processes are staticly made so
		 * they can keep the same PIDs.  This is especially true
		 * for PID 1, the init process, which is known as PID 1
		 * all over the place.
		 */
		return prev_pid + 1;

	/*
	 * All other PIDs should be random.
	 */
	pid_space = max_pid - SEC_MINPID + 1;
	seed = prev_pid + noise_value(u.u_procp);

	return SEC_MINPID + (int) (seed % pid_space);
}


/*
 * 		STOPPING COVERT CHANNEL
 *
 * Return a semi-random slot number for the IPC ID so that successive calls 
 * to obtain an IPC entity are not sequential.  This greatly reduces a
 * leakage path on IPC, but does not handle leakage through modulation of
 * key values.
 *
 * The algorithm randomizes the particular free slot rather than the ID
 * itself, because the former will not fail if there are free slots, but
 * the latter quite possible will, meaning expensive retries.  Both ways
 * yield the same results, but the latter way does it faster.
 * PORT NOTE: the system V IPC scheme is extremely system dependent.
 *            Please plant this hook carefully, and change ifdefs if necessary.
 */
void
#ifdef AUX
sec_ipc_slot(ipcfcn, new_entry)
	register int (*ipcfcn)();
	struct ipc_perm **new_entry;
#else
sec_ipc_slot(slots, new_entry, slot_size, slot_max)
	struct ipc_perm *slots;
	struct ipc_perm **new_entry;
	int slot_size;
	int slot_max;
#endif
{
	register int free_slots;
	register int slot_scan;
#ifdef AUX
	struct ipc_perm *pslot;
#else
	register struct ipc_perm *pslot;
#endif
	int which_empty_slot;

	/*
	 * First, find the number of empty slots.
	 */
#ifdef AUX
	free_slots = 0;
	for (slot_scan = 0; (*ipcfcn)(slot_scan, &pslot); slot_scan++)
		if ((pslot->mode & IPC_ALLOC) == 0)
			free_slots++;
#else
	pslot = slots;
	free_slots = 0;
	for (slot_scan = 0; slot_scan < slot_max; slot_scan++)  {
		if ((pslot->mode & IPC_ALLOC) == 0)
			free_slots++;
		pslot = (struct ipc_perm *) (((char *) pslot) + slot_size);
	}
#endif

	if (free_slots > 0)  {
		/*
		 * Then, get a random number in the range
		 *	0  <=  #  <  free_slots
		 */
		which_empty_slot = (int) (noise_value(u.u_procp) % free_slots);

		/*
		 * Finally, scan through the empty slots again and pick
		 * out the one that was chosen.
		 */
#ifdef AUX
		free_slots = 0;
		for (slot_scan = 0; (*ipcfcn)(slot_scan, &pslot); slot_scan++) {
			if ((pslot->mode & IPC_ALLOC) == 0 &&
			    free_slots++ == which_empty_slot)
				break;
		}
#else
		pslot = slots;
		free_slots = 0;
		for (slot_scan = 0; slot_scan < slot_max; slot_scan++)  {
			if ((pslot->mode & IPC_ALLOC) == 0 &&
			    free_slots++ == which_empty_slot)
				break;
			pslot = (struct ipc_perm *)((char *)pslot + slot_size);
		}
		if (slot_scan >= slot_max)
			pslot = (struct ipc_perm *) 0;
#endif
	}
	else
		pslot = (struct ipc_perm *) 0;

	*new_entry = pslot;
}
#endif /* !_OSF_SOURCE */
#endif /* SEC_ARCH */
