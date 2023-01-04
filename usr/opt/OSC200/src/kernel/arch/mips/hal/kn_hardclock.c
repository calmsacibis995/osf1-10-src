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
static char *rcsid = "@(#)$RCSfile: kn_hardclock.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/14 17:19:55 $";
#endif

#include  <sys/types.h>
#include  <machine/reg.h>
#include  <machine/cpu.h>
#include  <hal/mc146818clock.h>

extern int cpecount;
extern hardclock(), mc146818ackrtclock();

/***********************************************************************
/* FUNCTION:	kn_hardclock
/*
/* PURPOSE:	This function is a processor specific hardclock wrapper.
/*		Currently it is used by the DS3100, DS5000, DS5100, 
/*		DS5000_100, DS5000_300 and MAXINE. It performs hardware 
/*		dependent, hardclock related processing prior to calling
/*		hardclock. Specificly it acks the real time clock and 
/*		counts cache parity errors. To improve performance the
/*		ack is implemented via a macro which allows the code to 
/*		be inlined.
/***********************************************************************
*/


kn_hardclock(intr_frame)
	u_int *intr_frame;
{

   MC146818ACKRTCLOCK();
/*   mc146818ackrtclock();
*/
/*   {
/*   extern char *rt_clock_addr;	/* addr of the mc146818clock chip */
/*   register volatile struct rt_clock *rt =(struct rt_clock *)rt_clock_addr;
   register int c;
   c = rt->rt_regc;
   }
*/

   /* Count cache parity errors */
   if (intr_frame[EF_SR] & SR_PE) 
      {
      cpecount++;
      clearcpe();
      }

   hardclock(intr_frame[EF_EPC], intr_frame[EF_SR]);
   
   return;
   
}


