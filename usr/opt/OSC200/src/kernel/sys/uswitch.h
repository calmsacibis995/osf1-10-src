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
 * @(#)$RCSfile: uswitch.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/05/22 17:59:03 $
 */

#ifndef _SYS_USWITCH_H_
#define _SYS_USWITCH_H_

#ifndef _HABITAT_H_
#include <sys/habitat.h>
#endif

/*
 * Values for "cmd" used by the habitat system call 
*/
#define USC_GET		1
#define USC_SET		2

/*
 * Feature macros:
 * u_uswitch is laid out:
 * bits 0-7:  habitat value
 * bit  8:    set   if *(NULL) == 0 for calling process. (System V behavior)
 *            clear if *(NULL) causes SIGSEGV. (default)
 * bits 9-31: reserved for future use.
*/

#define USW_NULLP	(1<<8)

/* mask out all but habitat */
#define USW_GETHAB(x)	((x)&(0xff))

/* unused bits mask */
#define USW_UNUSED(x)	((x)&(0xfffffe00))


/*
 * Value macros
 * USW_MIN and USW_MAX are used for range checking. Currently they are
 * defined by the min and max habitat values, but that can change when
 * non-habitat related switches are used.
*/
#define USW_MIN		hbval(DEFAULT_HAB_NO)
#define USW_MAX		hbval(MAX_HAB_NO)

/*
 * WFS values for switching worlds (aka file name space mapping)
 */
#define USW_OSF1	hbval(OSF1_HAB_NO)
#define USW_SVID2	hbval(SVID2_HAB_NO)
#define USW_SVID3	hbval(SVID3_HAB_NO)
#define USW_SVR4	hbval(SVR4_HAB_NO)
#define USW_SOE2	hbval(SOE2_HAB_NO)
#define USW_MPSG	hbval(MPSG_HAB_NO)

#define ISHAB_OSF1(h)	(USW_GETHAB(h) == USW_OSF1)
#define ISHAB_SVID2(h)	(USW_GETHAB(h) == USW_SVID2)
#define ISHAB_SVID3(h)	(USW_GETHAB(h) == USW_SVID3)
#define ISHAB_SVR4(h)	(USW_GETHAB(h) == USW_SVR4)
#define ISHAB_SOE2(h)	(USW_GETHAB(h) == USW_SOE2)

/* 
 * Toggle values
*/
#define USW_OFF		0
#define USW_ON		1

#endif /* _SYS_USWITCH_H_ */
