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
static char	*sccsid = "@(#)$RCSfile: syscall_sw.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/09/03 19:26:15 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#include <mach_ipc_xxxhack.h>
#include <mach_emulation.h>
#include <mach_sctimes.h>

/*
 *	To add a new entry:
 *		Add an "FN(routine, arg count)" to the table below.
 *
 *		Add trap definition to "kern/syscall_sw.h" and
 *		recompile user library.
 *
 * WARNING:	If you add a trap which requires more than 6
 *		parameters, mach/ca/syscall_sw.h and ca/trap.c both need
 *		to be modified for it to work successfully on an
 *		RT.
 */

#include <mach/port.h>
#include <mach/kern_return.h>
#include <kern/syscall_sw.h>

port_t		null_port()
{
	return(PORT_NULL);
}

kern_return_t	kern_invalid()
{
	return(KERN_INVALID_ARGUMENT);
}


/* Include declarations of the trap functions. */

#include <mach/mach_traps.h>
#include <kern/ipc_basics.h>
#include <kern/syscall_subr.h>

mach_trap_t	mach_trap_table[] = {
	FN(kern_invalid, 0),			/* 0 */
	FN(kern_invalid, 0),			/* 1 */
	FN(kern_invalid, 0),			/* 2 */
	FN(kern_invalid, 0),			/* 3 */
	FN(kern_invalid, 0),			/* 4 */
	FN(kern_invalid, 0),			/* 5 */
	FN(kern_invalid, 0),			/* 6 */
	FN(kern_invalid, 0),			/* 7 */
	FN(kern_invalid, 0),			/* 8 */
	FN(kern_invalid, 0),			/* 9 */
	FN(task_self, 0),			/* 10 */
	FN(thread_reply, 0),			/* 11 */
	FN(task_notify, 0),			/* 12 */
	FN(thread_self, 0),			/* 13 */
#if	MACH_IPC_XXXHACK
	FF1(msg_send_old, 3, APSIG),		/* 14 */	/* obsolete */
	FF1(msg_receive_old, 3, APSIG),		/* 15 */	/* obsolete */
	FF1(msg_rpc_old, 5, APSIG),		/* 16 */	/* obsolete */
#else	
	FN(kern_invalid, 0),			/* 14 */
	FN(kern_invalid, 0),			/* 15 */
	FN(kern_invalid, 0),			/* 16 */
#endif
	FN(kern_invalid, 0),			/* 17 */
	FN(kern_invalid, 0),			/* 18 */
	FN(kern_invalid, 0),			/* 19 */
	FF1(msg_send_trap, 4, APSIG),		/* 20 */
	FF1(msg_receive_trap, 5, APSIG),	/* 21 */
	FF1(msg_rpc_trap, 6, APSIG),		/* 22 */
	FN(kern_invalid, 0),			/* 23 */
	FN(kern_invalid, 0),			/* 24 */
	FN(kern_invalid, 0),			/* 25 */
	FN(kern_invalid, 0),			/* 26 */
	FN(kern_invalid, 0),			/* 27 */
	FN(kern_invalid, 0),			/* 28 */
	FN(kern_invalid, 0),			/* 29 */
	FF1(lw_wire, 3, APSIG),			/* 30 */
	FF1(lw_unwire, 1, APSIG),		/* 31 */
	FN(kern_invalid, 0),			/* 32 */
	FN(kern_invalid, 0),			/* 33 */
	FN(kern_invalid, 0),			/* 34 */
	FN(kern_invalid, 0),			/* 35 */
	FN(kern_invalid, 0),			/* 36 */
	FN(kern_invalid, 0),			/* 37 */
	FN(kern_invalid, 0),			/* 38 */
	FN(kern_invalid, 0),			/* 39 */
	FN(kern_invalid, 0),			/* 40 */
	FN(init_process, 0),			/* 41 */
	FN(kern_invalid, 0),			/* 42 */
	FN(map_fd, 5),				/* 43 */
	FN(kern_invalid, 0),			/* 44 */
	FN(kern_invalid, 0),			/* 45 */
	FN(kern_invalid, 0),			/* 46 */
	FN(kern_invalid, 0),			/* 47 */
	FN(kern_invalid, 0),			/* 48 */
	FN(kern_invalid, 0),			/* 49 */
	FN(kern_invalid, 0),			/* 50 */
	FN(kern_invalid, 0),			/* 51 */
#if	MACH_EMULATION
	FN(htg_unix_syscall, 3),	       	/* 52 */
#else	
	FN(kern_invalid, 0),			/* 52 */
#endif
	FN(kern_invalid, 0),			/* 53 */
	FN(kern_invalid, 0),			/* 54 */
	FN(host_self, 0),			/* 55 */
	FN(host_priv_self, 0),			/* 56 */
	FN(kern_invalid, 0),			/* 57 */
	FN(kern_invalid, 0),			/* 58 */
 	FN(swtch_pri, 1),			/* 59 */
	FN(swtch, 0),				/* 60 */
	FN(thread_switch, 3),			/* 61 */
	FN(kern_invalid, 0),			/* 62 */
	FN(kern_invalid, 0),			/* 63 */
	FN(kern_invalid, 0),			/* 64 */
	FN(kern_invalid, 0),			/* 65 */
	FN(kern_invalid, 0),			/* 66 */
	FN(kern_invalid, 0),			/* 67 */
	FN(kern_invalid, 0),			/* 68 */
	FN(kern_invalid, 0),			/* 69 */
#if	MACH_SCTIMES
	FN(mach_sctimes_0, 0),			/* 70 */
	FN(mach_sctimes_1, 1),			/* 71 */
	FN(mach_sctimes_2, 2),			/* 72 */
	FN(mach_sctimes_3, 3),			/* 73 */
	FN(mach_sctimes_4, 4),			/* 74 */
	FN(mach_sctimes_5, 5),			/* 75 */
	FN(mach_sctimes_6, 6),			/* 76 */
	FF1(mach_sctimes_7, 0, APSIG),		/* 77 */
	FF1(mach_sctimes_8, 6, APSIG),		/* 78 */
	FN(mach_sctimes_9, 1),			/* 79 */
	FN(mach_sctimes_10, 2),			/* 80 */
	FN(mach_sctimes_11, 2),			/* 81 */
	FN(mach_sctimes_port_alloc_dealloc, 1),	/* 82 */
#else	
	FN(kern_invalid, 0),			/* 70 */
	FN(kern_invalid, 0),			/* 71 */
	FN(kern_invalid, 0),			/* 72 */
	FN(kern_invalid, 0),			/* 73 */
	FN(kern_invalid, 0),			/* 74 */
	FN(kern_invalid, 0),			/* 75 */
	FN(kern_invalid, 0),			/* 76 */
	FN(kern_invalid, 0),			/* 77 */
	FN(kern_invalid, 0),			/* 78 */
	FN(kern_invalid, 0),			/* 79 */
	FN(kern_invalid, 0),			/* 80 */
	FN(kern_invalid, 0),			/* 81 */
	FN(kern_invalid, 0),			/* 82 */
#endif
	FN(kern_invalid, 0),			/* 83 */
	FN(kern_invalid, 0),			/* 84 */
	FN(kern_invalid, 0),			/* 85 */
	FN(kern_invalid, 0),			/* 86 */
	FN(kern_invalid, 0),			/* 87 */
	FN(kern_invalid, 0),			/* 88 */
	FN(kern_invalid, 0),			/* 89 */
	FN(kern_invalid, 0),			/* 90 */
	FN(kern_invalid, 0),			/* 91 */
	FN(kern_invalid, 0),			/* 92 */
	FN(kern_invalid, 0),			/* 93 */
	FN(kern_invalid, 0),			/* 94 */
	FN(kern_invalid, 0),			/* 95 */
	FN(kern_invalid, 0),			/* 96 */
	FN(kern_invalid, 0),			/* 97 */
	FN(kern_invalid, 0),			/* 98 */
	FN(kern_invalid, 0),			/* 99 */
	FN(kern_invalid, 0),			/* 100 */
	FN(kern_invalid, 0),			/* 101 */
	FN(kern_invalid, 0),			/* 102 */
	FN(kern_invalid, 0),			/* 103 */
	FN(kern_invalid, 0),			/* 104 */
	FN(kern_invalid, 0),			/* 105 */
	FN(kern_invalid, 0),			/* 106 */
	FN(kern_invalid, 0),			/* 107 */
	FN(kern_invalid, 0),			/* 108 */
	FN(kern_invalid, 0),			/* 109 */
	FN(kern_invalid, 0),			/* 110 */
	FN(kern_invalid, 0),			/* 111 */
	FN(kern_invalid, 0),			/* 112 */
	FN(kern_invalid, 0),			/* 113 */
	FN(kern_invalid, 0),			/* 114 */
	FN(kern_invalid, 0),			/* 115 */
	FN(kern_invalid, 0),			/* 116 */
	FN(kern_invalid, 0),			/* 117 */
	FN(kern_invalid, 0),			/* 118 */
	FN(kern_invalid, 0),			/* 119 */
	FN(kern_invalid, 0),			/* 120 */
	FN(kern_invalid, 0),			/* 121 */
	FN(kern_invalid, 0),			/* 122 */
	FN(kern_invalid, 0),			/* 123 */
	FN(kern_invalid, 0),			/* 124 */
	FN(kern_invalid, 0),			/* 125 */
	FN(kern_invalid, 0),			/* 126 */
	FN(kern_invalid, 0),			/* 127 */
	FN(kern_invalid, 0),			/* 128 */
	FN(kern_invalid, 0),			/* 129 */
	FN(kern_invalid, 0),			/* 130 */
};

int	mach_trap_count = (sizeof(mach_trap_table) / sizeof(mach_trap_table[0]));
