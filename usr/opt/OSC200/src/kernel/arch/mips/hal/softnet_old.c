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
static char *rcsid = "@(#)$RCSfile: softnet_old.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:23:37 $";
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
 * derived from trap.c  4.10    (ULTRIX)        12/19/90";
 */
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

extern struct cpusw *cpup;   /* Pointer to cpusw entry for this machine*/

extern int mmap_debug;

#ifdef notdef
softnet()
{
        extern int netisr;
        register struct isrent {
                int value;
                int (*function)();
        } *pisr;
        extern struct isrent netisr_tab[];
        int s, signo;


        /* see if any "extreme" priority signals posted by the
           fpu code */
        if (CURRENT_CPUDATA->cpu_fpe_event) {
                s = splfpu();   /* sync with fpu */
                /* send the signals...lowest numbered (highest
                   priority) first */
                while(CURRENT_CPUDATA->cpu_fpe_sendsig) {
                        signo=ffs(CURRENT_CPUDATA->cpu_fpe_sendsig);
                        /* note signo is 1-32 */
                        CURRENT_CPUDATA->cpu_fpe_sendsig &= ~(1<<(signo-1));
                        splx(s);
                        unix_master();
                        psignal(CURRENT_CPUDATA->cpu_fpe_event,signo);
                        unix_release(); s = splfpu();
                }
                CURRENT_CPUDATA->cpu_fpe_event = 0; /* no more */
                splx(s);
        }

        if (!BOOT_CPU) {
                acksoftnet();
        } else {
                pisr = netisr_tab;

                while(pisr->value != -1) {
                        s = splimp();
                        if(netisr & (1<<pisr->value)) {
                                netisr &= ~(1<<pisr->value);
                                splx(s);
                                (*pisr->function)();
                        } else
                                splx(s);
                        pisr++;
                }
                s = splimp();
                if (!netisr) {
                        acksoftnet();
                }
                splx(s);
        }

        if (CURRENT_CPUDATA->cpu_wto_event) {
                if (mmap_debug ||
                   (sm_killp(CURRENT_CPUDATA->cpu_wto_pfn) == -1)){
                   /*
                    *  log the information into error log buffer,
                    *  print the error info on the console and then crash.
                    */

	  	     (*(cpup->log_errinfo))(CURRENT_CPUDATA->cpu_log_errinfo);

		     (*(cpup->print_consinfo))(CURRENT_CPUDATA->cpu_consinfo);
                        CURRENT_CPUDATA->cpu_wto_event = 0;
                        panic("CPU write timeout");
                }
                else
                        CURRENT_CPUDATA->cpu_wto_event = 0;
        }
}
#endif /* notdef */
