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
static char *rcsid = "@(#)$RCSfile: kn220_hrdclck.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:00:40 $";
#endif

#include  <sys/types.h>
#include  <machine/reg.h>
#include  <machine/cpu.h>

extern int cpecount;
extern hardclock();


/***********************************************************************
/* FUNCTION:	kn220_hardclock
/*
/* PURPOSE:	This function is a processor specific hardclock wrapper
/*		for the DS5500 (MIPSFAIR 2). It performs hardware 
/*		dependent, hardclock related processing prior to calling
/*		hardclock. 
/***********************************************************************
*/

kn220_hardclock(intr_frame)
	u_int *intr_frame;
{

/*   ackrtclock(); */

   /* Count cache parity errors */
   if (intr_frame[EF_SR] & SR_PE) 
      {
      cpecount++;
      clearcpe();
      }

   hardclock(intr_frame[EF_EPC], intr_frame[EF_SR]);
   
   return;
   
}


