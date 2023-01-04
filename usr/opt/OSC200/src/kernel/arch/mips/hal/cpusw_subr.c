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
static char *rcsid = "@(#)$RCSfile: cpusw_subr.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/06/03 10:35:00 $";
#endif


/*
 * Some old history from cpusw.c:
 * 
 * Revision 1.1.2.4  92/03/13  17:22:40  halproject_Donald_Dutile
 * 
 * 	Added argument to whatspl. Not sure how this worked before.
 * 	[92/03/12  13:02:33  Gary_Dupuis]
 * 
 * 	Moved cpuswitch-based getspl() and whatspl() from spl_subr.c into
 * 	here. 
 * 
 * Revision 1.1.2.2  92/02/12  16:43:56  Donald_Dutile
 * 	Moved getspl() and whatspl() to hal/spl_subr.c.
 * 	[92/02/10  15:49:19  Donald_Dutile]
 * 
 * 	Moved platform specific code that go through the cpu switch
 * 	table from machdep.c to here.  Most of these functions are
 * 	defined HAL functions.
 * 	[92/02/09  17:53:17  Donald_Dutile]
 * 
 */


/*
 * The following routines are the ones used by DEC's HAL
 * that are implemented via a cpu switch table.
 *
 * Most of these have been pulled from machdep.c for hw indep.
 *
 * These routines were pulled from cpusw.c because they are
 * implemented in a per-system fashion. These functions must be
 * contained in a separate module for the linker to link
 * these functions in for generic kernels, and not link them
 * in for target kernels.
 *
 * Note, that no (new) system must require these wrapper routines
 * or else they will generate multiple defines if some overlap w/target
 * kernel versions of these functions.
 *
 */

#include <hal/cpuconf.h>

extern struct	cpusw	*cpup;      /* pointer to cpusw entry */

/*
 * Delay for n microseconds
 * Call through the system switch to specific delay routine.
 */
microdelay(usecs)
        int usecs;
{
        (*(cpup->microdelay))(usecs);
}


/*
 * Wait until the write buffer of the processor we are on is empty.
 * In "critical path code", don't check return status.
 */
wbflush()
{
        (*(cpup->wbflush))();
}
                                         
getspl()
{
        return((*(cpup->getspl))());
}


whatspl(sr)
unsigned int	sr;

{
        return((*(cpup->whatspl))(sr));
}
