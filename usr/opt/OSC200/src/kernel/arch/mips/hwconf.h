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
 *	@(#)$RCSfile: hwconf.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:14:57 $
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
 * derived from hwconf.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */
/*
 * hdwconf.h -- hardware specific configuration information
 *
 * Modification History: 
 *
 * 07-Apr-89 -- afd
 *	Move macros for getting items from "systype" word to cpuconf.h
 *	Move defines for R2000a cpu type and PMAX systype to cpuconf.h
 *
 * 09-Nov-88 -- afd
 *	Add macros for getting items from "systype" word.
 *	Add defines for R2000a cpu type and PMAX systype.
 */

/*
 * revision id for chips
 */
union rev_id {
	unsigned int	ri_uint;
	struct {
#if	BYTE_MSF
		unsigned int	Ri_fill:16,
				Ri_imp:8,		/* implementation id */
				Ri_majrev:4,		/* major revision */
				Ri_minrev:4;		/* minor revision */
#else	/* BYTE_MSF */
		unsigned int	Ri_minrev:4,		/* minor revision */
				Ri_majrev:4,		/* major revision */
				Ri_imp:8,		/* implementation id */
				Ri_fill:16;
#endif	/* BYTE_MSF */
	} Ri;
};
#define	ri_imp		Ri.Ri_imp
#define	ri_majrev	Ri.Ri_majrev
#define	ri_minrev	Ri.Ri_minrev

struct imp_tbl {
	char *it_name;
	unsigned it_imp;
};

/*
 * NVRAM information
 */
#define ENV_MAXLEN	32
#define ENV_ENTRIES	6
struct promenv {
	char	name[ENV_MAXLEN];
	char	value[ENV_MAXLEN];

};

/*
 * contains configuration information for all hardware in system
 */
struct hw_config {
	unsigned	icache_size;
	unsigned	dcache_size;
	union rev_id	cpu_processor;
	union rev_id	fpu_processor;
	unsigned char	cpubd_type;
	unsigned char	cpubd_rev;
	char		cpubd_snum[5];
	int		cpubd_config;
	struct promenv	promenv[ENV_ENTRIES];
#ifdef TODO
	add memory board id prom information
#endif /* TODO */
};

/*
 * options to hdwconf() syscall
 */
#define HWCONF_GET	0
#define HWCONF_SET	1


#ifdef KERNEL
extern struct hw_config hwconf;
#endif /* KERNEL */

