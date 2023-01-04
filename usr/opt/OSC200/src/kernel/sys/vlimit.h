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
 *	@(#)$RCSfile: vlimit.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:03:04 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _SYS_VLIMIT_H_
#define _SYS_VLIMIT_H_

/* LIM_NORAISE is not emulated */
#define	LIM_NORAISE	0	/* if <> 0, can't raise limits */
#define	LIM_CPU		1	/* max secs cpu time */
#define	LIM_FSIZE	2	/* max size of file created */
#define	LIM_DATA	3	/* max growth of data space */
#define	LIM_STACK	4	/* max growth of stack */
#define	LIM_CORE	5	/* max size of ``core'' file */
#define	LIM_MAXRSS	6	/* max desired data+stack core usage */

#define	NLIMITS		6

#define INFINITY        0x7fffffff

#endif /* _SYS_VLIMIT_H_ */
