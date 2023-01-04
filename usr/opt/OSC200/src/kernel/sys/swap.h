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
 * @(#)$RCSfile: swap.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/27 15:51:11 $
 */
/*
 *  Module Name:
 *	swap.h
 *  Description:
 *	header file for SVR4 swapctl() system call
 */

#ifndef _SYS_SWAP_H
#define _SYS_SWAP_H

/* The following are for the swapctl system call */

#define	SC_ADD		1	/* add a specified resource for swapping */
#define	SC_LIST		2	/* list all the swapping resources */
#define	SC_REMOVE	3	/* remove the specified swapping resource */
#define SC_GETNSWP	4	/* get number of swapping resources configued */

typedef struct swapres {
	char	*sr_name;	/* pathname of the resource specified */
	off_t	sr_start;	/* starting offset of the swapping resource */
	off_t 	sr_length;	/* length of the swap area */
} swapres_t;

typedef struct swapent {
	char 	*ste_path;	/* get the name of the swap file */
	off_t	ste_start;	/* starting block for swapping */
	off_t	ste_length;	/* length of swap area */
	long	ste_pages;	/* numbers of pages for swapping */
	long	ste_free;	/* numbers of ste_pages free */
	long	ste_flags;	/* see below */
} swapent_t;

/* ste_flags values */

#define	ST_INDEL	0x01		/* this file is in the process */
					/* of being deleted. Don't     */
					/* allocate from it. This can  */
					/* be turned of by swapadd'ing */
					/* this device again.          */
#define	ST_DOINGDEL	0x02		/* this file is in the process */
					/* of being deleted. Fail      */
					/* another deletion attempt    */
					/* if this flag is on.         */
#define	ST_DELETED	0x04		/* this file has been deleted  */
					/* but the data structures have*/
					/* not been freed up yet.      */

typedef struct	swaptable {
	int	swt_n;			/*number of swapents following */
	struct	swapent	swt_ent[1];	/* array of swt_n swapents */
} swaptbl_t;

#endif	/* _SYS_SWAP_H */
