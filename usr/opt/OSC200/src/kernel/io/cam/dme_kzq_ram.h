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
 * @(#)$RCSfile: dme_kzq_ram.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 15:34:26 $
 */
#ifndef __DME_KZQ_RAM_H__

#define __DME_KZQ_RAM_H__

/* ---------------------------------------------------------------------- */
/* dme_kzq_ram.h	Version 1.00		July 20, 1991 */

/*  This file contains the definitions and data structures needed by
    the KZQ DME related files.

Modification History

	Version	  Date		Who	Reason

	1.00    07/20/90        rps	Created this file.
*/

typedef struct dme_kzq_struct 
  {
    /*
     * These pointers contain the free DMA segments available to the
     * the SIM DME.
     *
     * Allocate pointers to each of the Active Segment structure.
     * The dme_XXkb_flink points at the first free segment and
     * dme_XXkb_blink points at the last free segment. The list
     * is empty if the blink points at the flink.
     */
    SEGMENT_ELEMENT *head_1kb;
    SEGMENT_ELEMENT *tail_1kb;
    SEGMENT_ELEMENT *head_4kb;
    SEGMENT_ELEMENT *tail_4kb;
    SEGMENT_ELEMENT *head_8kb;
    SEGMENT_ELEMENT *tail_8kb;
    SEGMENT_ELEMENT *head_16kb;
    SEGMENT_ELEMENT *tail_16kb;

    void*	SVAPTE;		/* System Virtual address of first PTE */
    				/* used to double map P0 Virt Addresses*/
    void*	SVA;		/* S0 address used to dbl mapped buffer*/

  } DME_KZQ_STRUCT;

#endif

