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
 *	@(#)$RCSfile: addrconf.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:56:28 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* addrconf.h -- Definitions for getaddressconf() system call
 *
 * OSF/1 Release 1.0
 */

#ifndef _SYS_ADDRCONF_H
#define _SYS_ADDRCONF_H

/* Address-space configuration record, as returned by getaddressconf() */

struct addressconf {
	caddr_t			ac_base; /* base of region */
	unsigned		ac_flags;
};


/* Values for ac_flags */

#define		AC_UPWARD	0x0	/* area grows upward */
#define		AC_DOWNWARD	0x1	/* area grows downward */
#define		AC_FIXED	0x0	/* fixed base address */
#define		AC_FLOAT	0x2	/* base address floats above prev. region */

/* Known areas */

#define	AC_TEXT		0		/* absolute program text */
#define AC_DATA		1		/* absolute program data */
#define AC_BSS		2		/* absolute program bss */
#define AC_STACK	3		/* stack area */
#define AC_LDR_TEXT	4		/* loader text area */
#define AC_LDR_DATA	5		/* loader data area */
#define AC_LDR_BSS	6		/* loader bss area */
#define AC_LDR_PRIV	7		/* loader private data file -- inherited */
#define AC_LDR_GLB	8		/* loader global data file (KPT & preloads ) */
#define AC_LDR_PRELOAD	9		/* loader preloaded library data */
#define AC_MMAP_TEXT	10		/* mmap'ed file text */
#define AC_MMAP_DATA	11		/* mmap'ed file data */
#define AC_MMAP_BSS	12		/* mmap'ed file bss */

#define AC_N_AREAS	13		/* number of known areas */

#ifndef	KERNEL
#ifndef _NO_PROTO

extern int getaddressconf(struct addressconf *buf, size_t size);

#else /* _NO_PROTO */

extern int getaddressconf();

#endif /* _NO_PROTO */

#else /* KERNEL */

extern struct addressconf addressconf[AC_N_AREAS];

#endif /* KERNEL */

#endif /* _SYS_ADDRCONF_H */
