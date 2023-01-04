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
/*
 * @(#)$RCSfile: schyield.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/07/19 14:35:17 $
 */
/*
 * sched_yield.s
 *
 * POSIX 1003.4 yield primitive.
 *
 * To improve yield performance, implement the POSIX yield primitive as a
 * routine which invokes the Mach swtch() trap.  This could be done by an
 * explicit call to the swtch() routine in libmach, but we save a couple of
 * instructions this way.
 *
 * This will break if the Mach system calls and traps are removed.
 *
 *	Revision History:
 *
 * 03-May-91	Peter H. Smith
 *	New file.
 */

#include <mach/machine/syscall_sw.h>

kernel_trap(sched_yield,-60,0)
