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
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * derived from pdma_entry.c	4.2      (ULTRIX)  11/15/90";
 */


/************************************************************************
 *
 * pdma_entry.c	11/10/89
 *
 * Pseudo DMA entry array for the PDMA control code.
 *
 * Modification history:
 *
 * 06/05/91	Tom Tierney
 * Merge of ULTRIX 4.2 SCSI subsystem and OSF/1 reference port work.
 * This module is a result of work done by Fred Canter and Bill Burns
 * to merge pdma_entry.c version 4.2 from ULTRIX and the OSF/1
 * reference port of the SCSI subsystem.
 *
 * Removed OSF conditional ifdef.
 *
 * 05/29/91	Paul Grist
 * Added 3max+/bigmax support - share 3min pdma entry.
 *
 * 09/07/90     Maria Vella
 * Submitted this file to release.
 *
 * 07/25/90     Janet L. Schank
 * Removed DS3100 support.
 *
 * 10/12/89	John A. Gallant
 * Created this file to support the PDMA work in the scsi drivers.
 *
 * 11/10/89	John A. Gallant
 * Added the DS5000 entry.
 *
 ************************************************************************/

/* 
This file contains the Pseudo DMA entry points for all the PDMA systems.
Each entry has to be defined in their particular file. All new entries
have to be added to this array.
NOTE: 
    This array MUST be NULL terminated.  The routines the scan this array
     will stop scanning when a NULL pointer is encountered.
*/

#include <data/scsi_data.c>

/* External functions and variables. */

extern PDMA_ENTRY pdma_ds5000;		/* the entry for DS5000 */
extern PDMA_ENTRY pdma_ds5500;		/* the entry for DS5000 */
extern PDMA_ENTRY pdma_ds5000_100;	/* entry for 3min */
extern PDMA_ENTRY pdma_flamingo;

/************************************************************************/

PDMA_ENTRY *pdma_entry[] =
{

#ifdef DS5000
    &pdma_ds5000,
#endif
#ifdef DS5000
    &pdma_ds5500,
#endif
#ifdef DS5000_100
    &pdma_ds5000_100,
#endif
#ifdef DS5000_300
    &pdma_ds5000_100,
#endif
#ifdef ALPHAFLAMINGO
    &pdma_flamingo,
#endif

    ( 0 )			/* MUST: null termination */
};

/************************************************************************/
