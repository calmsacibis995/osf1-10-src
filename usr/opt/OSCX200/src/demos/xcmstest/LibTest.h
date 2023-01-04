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
 * (c) Copyright 1990 Tektronix Inc.
 * 	All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Tektronix not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * Tektronix disclaims all warranties with regard to this software, including
 * all implied warranties of merchantability and fitness, in no event shall
 * Tektronix be liable for any special, indirect or consequential damages or
 * any damages whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of this
 * software.
 *
 *
 *	NAME
 *		LibTest.h
 *
 *	DESCRIPTION
 *		Public include file for the LibTest test interface tool
 *
 *	REVISION
 *		$Header: /usr/sde/x11/rcs/x11/src/./demos/xcmstest/LibTest.h,v 1.2 91/12/15 12:42:16 devrcs Exp $
 */
#ifndef LIBTEST_H
#define LIBTEST_H

/*
 *	DEFINES
 */
#ifndef	GLOBAL
#  define	GLOBAL
#endif

/*
 *	EXTERNS
 */
extern	int CommandArgc;	/* GLOBAL */
extern	char **CommandArgv;	/* GLOBAL */

/*
 *	TYPEDEFS
 */
#ifndef Status
typedef int Status;
#endif

typedef Status (*PFStatus)();

typedef struct {
    char *pstring;
    PFStatus pfunc;
} FuncTableEntry;

typedef struct _LtDefineEntry{
    char		*pstring;
    unsigned long	define;
} LtDefineEntry;

extern PFStatus LtStrToFuncPtr();
extern char *LtDefineToStr();
extern int  LtStrToDefine();

#endif /* LIBTEST_H */
