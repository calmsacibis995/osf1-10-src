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
static char *rcsid = "@(#)$RCSfile: spl_rt.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:43:27 $";
#endif

#include <rt_preempt.h>
#include <rt_preempt_debug.h>

#if     RT_PREEMPT
#include <mach/mips/boolean.h>
#include <sys/preempt.h>

extern int rt_preempt_enabled;
#if     RT_PREEMPT_DEBUG
extern int rt_preempt_splx;
extern int rt_preempt_spl0;
#endif

spl0()
{
    if (!rt_preempt_enabled)
        return(spl0x());
    else {
        register unsigned int ret;

        ret = spl0x();
        preemption_point_safe(rt_preempt_spl0);
        return(ret);
    }
}


splnone()
{
    if (!rt_preempt_enabled)
        return(spl0x());
    else {
        register unsigned int ret;

        ret = spl0x();
        preemption_point_safe(rt_preempt_spl0);
        return(ret);
    }
}
   
splx(ipl)
    int ipl;
{
    if (!rt_preempt_enabled)
        return(splx1(ipl));
    else {
        register unsigned int ret;

        ret = splx1(ipl);
        preemption_point_safe(rt_preempt_splx);
        return(ret);
    }
}

splx_nopreempt(ipl)
    int ipl;
{
    return(splx1(ipl));
}
#endif  /* RT_PREEMPT */

