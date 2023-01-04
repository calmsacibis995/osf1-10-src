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
 * Definitions for pmerge program
 */

struct pheader {
	long    tsize;          /* text size in bytes, padded to DW bdry*/
	long    dsize;          /* initialized data "  "                */
	long    bsize;          /* uninitialized data "   "             */
	long    entry;          /* entry 				*/
	long    text_start;     /* base of text used for this file      */
	long    data_start;     /* base of data used for this file      */
	long    bss_start;      /* base of bss used for this file       */
	long	image		/* pointer to the image in memory	*/
};

#define MAX_MEMORY	16*1024*1024	/* largest physical memory	*/
#define TRUE 1
#define FALSE 0
#define PAGSIZ	8192			/* page size for today		*/

