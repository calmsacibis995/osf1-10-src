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
static char *rcsid = "@(#)$RCSfile: intr.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/03 10:45:01 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from trap.c	4.10	(ULTRIX)	12/19/90";
 */

#include <machine/reg.h>
#include <machine/cpu.h>
#include <hal/cpuconf.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <sys/table.h>

extern int cpu;
extern struct cpusw *cpup;
extern int (*c0vec_tbl[])();
extern int ffintr();
extern int cold;
extern int cvec;

#ifdef STATCLOCK
extern gatherstats();
#else
#define gatherstats     stray_intr
#endif

int iplmask[IPLSIZE];           /* afdfix: this can vary with cpu type */
int     atintr_level = 0;

#if NCPUS > 1
volatile unsigned long  system_intr_cnts_level[NCPUS][INTR_MAX_LEVEL];
volatile unsigned long  system_intr_cnts_type[CPUS]INTR_TYPE_SIZE];
#else	/* NCPUS */
volatile unsigned long  system_intr_cnts_level[INTR_MAX_LEVEL];
volatile unsigned long  system_intr_cnts_type[INTR_TYPE_SIZE];
#endif	/* NCPUS */
volatile int *system_intr_cnts_type_transl;

/*
 * TODO: pending interrupts now come for CAUSE reg which is passed as arg4
 */
kn01_intr(ep, code, sr, cause)
u_int *ep;
u_int code, sr, cause;
{
        register int req;
        register int prevmask;
        extern struct reg_desc cause_desc[], sr_desc[];

/*
        XPRINTF(XPR_INTR, "intr cause=%r  sr=%r\n", cause, cause_desc,
            sr, sr_desc);
*/
        atintr_level++;
        prevmask = (sr & SR_IMASK);
        cause &= prevmask;

        /*
         * Should probably direct vector these.
	 *
	 * Note:  It is important to decrement the interrupt level (req)
	 *        by one because ffintr will start the level at one (zero
	 *        indicates none found).  All the tables, including the
	 *        interrupt statistics tables, are indexed starting at
	 *        zero.
         */
        if (req = ffintr(cause)) {
                req--;
		incr_interrupt_counter_level(req);
                splx(iplmask[req]);
/*              XPRINTF(XPR_INTR, "calling intr %d\n", req, 0, 0, 0);
*/
                (*c0vec_tbl[req])(ep);
            }
        else {
/* Don't define this, because the SII generates strays for TZ30 & RRD42 */

#ifdef  TRAP_LOG_STRAYS
                log(LOG_WARNING,"intr: cause bit vanished\n");
                stray_intr(ep);
#else
                incr_interrupt_counter_type(INTR_TYPE_STRAY);
#endif
	
        }
        atintr_level--;
}



/*
 * Interrupt for hard error comes here.
 * then we call system specific error handling routine.
 */
memintr(ep)
        u_int *ep;              /* exception frame ptr */
{
        if ((*(cpup->harderr_intr))(ep) < 0)
                panic("no hard error interrupt routine configured");
}



/*
 * Note: stray interrupts cannot happen on the PMAX system
 */
stray_intr(ep)
u_int *ep;
{
        unsigned cause, get_cause();

        cause = get_cause();
        log(LOG_WARNING,"intr: stray interrupt\n");
        log(LOG_WARNING,"\tPC: 0x%x\n\tCause register: %R\n\tCause register: %R\n\tStatus register: %R\n",
                ep[EF_EPC], ep[EF_CAUSE], cause_desc, cause, cause_desc,
               ep[EF_SR], sr_desc);
#ifdef notdef
/*  OSF version of stray_intr */
        extern struct reg_desc cause_desc[];
        unsigned cause, get_cause();

	incr_interrupt_counter_type(INTR_TYPE_STRAY);
        cause = get_cause();

        dprintf("stray interrupt, cause=%r\n", cause, cause_desc);

        log(LOG_WARNING,"intr: stray interrupt\n");
        log(LOG_WARNING,"intr: PC: 0x%x\n", ep[EF_EPC]);
        log(LOG_WARNING,"intr: ExcCause: %R\n", ep[EF_CAUSE], cause_desc);
        log(LOG_WARNING,"intr: Cause now: %R\n", cause, cause_desc);
        log(LOG_WARNING,"intr: Status register: %R\n", ep[EF_SR], sr_desc) ;
#endif /* notdef */
}


/*
 * Routine to handle stray interrupts.
 * Parameter:
 *      ep       Pointer to exception frame.
 *      vec      The vector obtained from reading the vector register.
 *
 * Used by systems that have an scb (default entry).
 */
stray(ep,vec)
int *ep;
int vec;
{
  if (cold) {
        if ((cpu == DS_5400) || (cpu == DS_5500))
                cvec = (vec - 0x200) & 0xfffe;
        else
                cvec = vec;
  }
  else {
        /* dgd -- changed cprintf to dprintf */
	incr_interrupt_counter_type(INTR_TYPE_STRAY);
        dprintf("Stray intr, vec=0x%x\n",vec);
  }
}
    


/*
 * Passive release
 */
passive_release(ep,vec)
int *ep;
int vec;
{
	/* DO NOTHING!.... maybe bump a global counter? */
}


#include <net/net_globals.h>

softintr2()
{
#if     NETISR_THREAD
        /* We just dont do this */
        panic("softintr2 ???\n");
#else
	incr_interrupt_counter_type(INTR_TYPE_OTHER);
        acksoftnet();
        Netintr();
#endif
}
