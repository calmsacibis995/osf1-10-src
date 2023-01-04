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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: message.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:22 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *
 *	UNIX debugger
 *
 */



#include <i386/kdb/defs.h>

msg		BADMOD	=  "bad modifier";
msg		BADCOM	=  "bad command";
msg		BADSYM	=  "symbol not found";
msg		BADLOC	=  "automatic variable not found";
msg		NOCFN	=  "c routine not found";
msg		NOMATCH	=  "cannot locate value";
msg		NOBKPT	=  "no breakpoint set";
msg		BADKET	=  "unexpected ')'";
msg		NOADR	=  "address expected";
msg		NOPCS	=  "no process";
msg		BADVAR	=  "bad variable";
msg		EXBKPT	=  "too many breakpoints";
msg		A68BAD	=  "bad a68 frame";
msg		A68LNK	=  "bad a68 link";
msg		ADWRAP	=  "address wrap around";
msg		BADEQ	=  "unexpected `='";
msg		BADWAIT	=  "wait error: process disappeared!";
msg		ENDPCS	=  "process terminated";
msg		NOFORK	=  "try again";
msg		BADSYN	=  "syntax error";
msg		NOEOR	=  "newline expected";
msg		SZBKPT	=  "bkpt: command too long";
msg		BADFIL	=  "bad file format";
msg		BADNAM	=  "not enough space for symbols";
msg		LONGFIL	=  "filename too long";
msg		NOTOPEN	=  "cannot open";
msg		BADMAG	=  "bad core magic number";
msg		TOODEEP =  "$<< nesting too deep";

char _ctype_[1 + 256] = {
	0,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C|_S,	_C|_S,	_C|_S,	_C|_S,	_C|_S,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
	_N,	_N,	_N,	_N,	_N,	_N,	_N,	_N,
	_N,	_N,	_P,	_P,	_P,	_P,	_P,	_P,
	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U,
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P,
	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L,
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C
};
