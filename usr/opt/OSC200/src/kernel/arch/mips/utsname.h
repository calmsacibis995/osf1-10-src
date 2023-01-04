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
 *	@(#)$RCSfile: utsname.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:19:38 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from utsname.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/* for MIPS_UNAME system call */

#ifdef	MSERIES
#define	SYS_NMLN	65	/* # of chars in uname-returned strings */
#endif
#ifdef	PMAX
#define	SYS_NMLN	32	/* # of chars in uname-returned strings */
#endif

struct sm_utsname {
	char	sysname[SYS_NMLN];	/* Same as nodename */
	char	nodename[SYS_NMLN];	/* hostname */
	char	release[SYS_NMLN];	/* MIPS OS release name */
	char	version[SYS_NMLN];	/* MIPS OS release number */
	char	machine[SYS_NMLN];	/* MIPS system type */
	char	m_type[SYS_NMLN];	/* MIPS Specific Machine Type */
	char	base_rel[SYS_NMLN];	/* Base Release of Initial Port */
	char	reserve5[SYS_NMLN];	/* reserved for future use */
	char	reserve4[SYS_NMLN];	/* reserved for future use */
	char	reserve3[SYS_NMLN];	/* reserved for future use */
	char	reserve2[SYS_NMLN];	/* reserved for future use */
	char	reserve1[SYS_NMLN];	/* reserved for future use */
	char	reserve0[SYS_NMLN];	/* reserved for future use */
};
#if defined KERNEL || defined INKERNEL
extern struct sm_utsname  sm_utsname;
#endif

/* valid release values */
#define V_UMIPSBSD	"UMIPS-BSD"
#define V_UMIPSV	"UMIPS-V"
#define V_ULTRIX	"ULTRIX"
#define V_MACH		"MACH"

/* valid version values */
#define R_2_0		"2_0"
#define R_3_0		"3_0"

/* valid machine values, must be what cpp defines */
#define M_MIPS		"mips"

/* valid base_rel values */
#define BR_V30_ATT	"ATT_V3_0"
#define BR_V31_ATT	"ATT_V3_1"
#define BR_43_BSD	"4_3_BSD"

/* valid m_type values */
#define MT_M500		"m500"
#define MT_M800		"m800"
#define MT_M1200	"m1200"
#define MT_DT1200	"dt1200"
#define MT_DEC3100	"pmax"
