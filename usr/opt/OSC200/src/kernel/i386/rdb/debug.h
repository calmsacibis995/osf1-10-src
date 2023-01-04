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
 *	@(#)$RCSfile: debug.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:12:47 $
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
 * 5799-CGZ (C) COPYRIGHT IBM CORPORATION 1986,1987,1988,1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
#endif

typedef int (*FN)();

#include "i386/rdb/debug_mach.h"		/* name shortned due to AIX V2.2.1 */

int err_flag;			       /* debugger error flag */
#define MAX_LINE	128	       /* somewhat short, but limited stack size */
#define MAX_ARG		(MAX_LINE/4)
#define SHORT_LINE 64		       /* for short responses */
#define DEBUG_DEBUG	0x01
#define SHOW_REGS	0x02
#define SCAN_SYM	0x04		/* use scan_sym to find statics */
#define CS_PRINT	0x08		/* show CS symbolicly */
#define LPS_2		0x10		/* have go issue LPS 2 for ex packets */
#define OPTION_TRACE_TABLE 0x20		/* use trace table */
#define OPTION_MOD	0x40		/* / = use previous size */
#define OPTION_EXPAND	0x80		/* show macro expansions */
#define OPTION_UNASM	0x100		/* debug unasm code */
#define OPTION_CR3	0x200		/* load CR3 on debugger entry */
#define OPTION_NOSYM	0x400		/* don't lookup in main symtab */

#include "i386/rdb/debug_fn.h"
