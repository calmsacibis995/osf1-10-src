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
 *	@(#)$RCSfile: ldr_exec.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/07/15 18:50:56 $
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

#ifndef _SYS_LDR_EXEC_H_
#define _SYS_LDR_EXEC_H_

/*
 * Default Loader File Name
 */
#define	LDR_EXEC_DEFAULT_LOADER	"/sbin/loader"

/*
 *  AT_EXEC_LOADER_FLAGS Auxiliary Vector Entry Value
 */
#define	LDR_EXEC_SYSTEM_MASK	0xffff0000
#define	LDR_EXEC_USER_MASK	0x0000ffff

#define	LDR_EXEC_SETUID_F	0x80000000
#define	LDR_EXEC_SETGID_F	0x40000000
#define	LDR_EXEC_PTRACE_F	0x20000000

#endif
