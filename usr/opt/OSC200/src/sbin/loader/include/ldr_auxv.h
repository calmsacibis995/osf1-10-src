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
 *	@(#)$RCSfile: ldr_auxv.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/04/23 10:53:53 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_auxv.h
 * declarations for loader auxiliary vector abstraction
 * Depends on "standards.h"
 *
 * OSF/1 Release 1.0
 */

#ifndef	_H_LDR_AUXV
#define	_H_LDR_AUXV


extern int
ldr_auxv_init __((void));

extern int
ldr_auxv_get_exec_filename __((char **));

extern int
ldr_auxv_get_exec_loader_filename __((char **));

extern int
ldr_auxv_get_exec_loader_flags __((int *));

extern int
ldr_auxv_get_pagesz __((int *));

extern int
ldr_auxv_get_execfd __((int *));

#endif	/* _H_LDR_AUXV */
