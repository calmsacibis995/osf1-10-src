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
/* BuildSystemHeader added automatically */
/* $Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/DynList.h,v 1.1.2.2 92/12/11 08:33:39 devrcs Exp $ */
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
/* COPYRIGHT (c) 1988 BY DIGITAL EQUIPMENT CORPORATION.  ALL RIGHTS RESERVED.
 *
 * PROGRAM:	General utility (originally for EPIC/Writer equation editor)
 *
 * DESCRIPTION:	Generic stack handler
 *	This utility allocates a stack or dynamic list as a chunk of a
 *	user-specified size.  Additional chunks are allocated as needed.
 *	If the free_unused_flag is set then unused chunks will be freed by
 *	all routines that decrement the stack pointer; otherwise no chunks will
 *	be freed until DynListDestroy() is called.
 *	This utility allows for adjustment and tuning of the tradeoff between
 *	memory usage and the number of malloc() and free() calls.
 *
 * REVISION HISTORY:
 *  jnh 29-Aug-88 Created.
 *
 ******	Include files ******/

/****** Data declarations ******/
typedef POINTER DYNAMIC_LIST;

#ifndef NULL
#define NULL (0L)
#endif

/****** Function and subroutine definitions ******/

/* Call this routine first for each dynamic list. */
PROTO4 (DYNAMIC_LIST DynListInit ,
  int item_size,		/* number of bytes in each data item */
  int chunk_size,		/* number of items to allocate in one chunk */
  int free_unused_flag,		/* non-zero iff unused chunks of memory 
				   should be freed as they become unused. */
  char *name);			/* name to print when actually allocating and
				   freeing space.  If NULL or "", then these
				   debugging messages will not be printed.
				   You must allocate space for 'name' and
				   you must not free it until I'm done with
				   it:  I don't make a copy, I use yours. */

/* Call this routine last for each dynamic list. */
PROTO1 (void DynListDestroy ,DYNAMIC_LIST dynamic_list);

/* Allocates (or finds) space for an item at the end of the list. */
PROTO1 (POINTER DynListNext ,DYNAMIC_LIST dynamic_list); /* jj-port */

/* Decrements the stack pointer and then */
/* returns the address of the item at the top of the list. */
/* if stack underflow, returns NULL. */
/* if free_unused_flag is set, then DynListPrev may free memory starting at */
/*    the item ABOVE the item returned:   thus getting item 43 allows */
/*    DynListPrev to trash item 44. */
PROTO1 (POINTER DynListPrev ,DYNAMIC_LIST dynamic_list); /* jj-port */

/* Calls DynListNext() and writes data into the allocated space. */
/* I recommend bypassing this routine and calling DynListNext() directly. */
PROTO2 (void DynListPush ,DYNAMIC_LIST dynamic_list, POINTER data);

/* Returns the address of the item at the top of the list */
/* and then decrements the stack pointer. */
/* if stack underflow, returns NULL. */
/* if free_unused_flag is set, then DynListPop may free memory starting at */
/*    the item ABOVE the item returned:   thus popping item 43 allows */
/*    DynListPop to trash item 44. */
PROTO1 (POINTER DynListPop ,DYNAMIC_LIST dynamic_list); /* jj-port */
