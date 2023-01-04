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
 *	@(#)$RCSfile: bootblk.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:53 $
 */ 
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

 *
 * Define a bootblock.  The boot block always resides on block zero of the
 * partition if a disk, or the first block of a tape.
 */

struct bootblock{
  int reserved[2];
  int magic;               /* Must contain magic number for boot block */
  int type;                /* Boot type. */
  union{
    struct{
      unsigned ladr;         /* Load address. */
      unsigned sadr;         /* Start address. */
      int      bcnt;         /* Block count. */
      int      sblk;         /* Starting block.  Usually this block. */
    } bb0;
    struct{
      unsigned ladr;         /* Load address. */
      unsigned sadr;         /* Start address. */
      struct{
	int cnt;             /* Blocks in a span. */
	int blk;             /* Starting block of span */
      } spans[1];            /* Any number of spans, terminated by zero cnt.*/
    } bb1;
  } bb;
};

#define BB_MAGIC    161146
#define BB_CONTIGUOUS      0
#define BB_SCATTERED       1
