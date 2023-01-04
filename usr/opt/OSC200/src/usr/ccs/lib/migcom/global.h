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
 *	@(#)$RCSfile: global.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:22:23 $
 */ 
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

#ifndef	_GLOBAL_H_
#define	_GLOBAL_H_

#define	EXPORT_BOOLEAN
#include <mach/boolean.h>
#include <sys/types.h>
#include "string.h"

extern boolean_t BeQuiet;	/* no warning messages */
extern boolean_t BeVerbose;	/* summarize types, routines */
extern boolean_t UseMsgRPC;
extern boolean_t GenSymTab;

extern boolean_t IsKernel;
extern boolean_t IsCamelot;

extern string_t RCSId;

extern string_t SubsystemName;
extern u_int SubsystemBase;

extern string_t MsgType;
extern string_t WaitTime;
extern string_t ErrorProc;
extern string_t ServerPrefix;
extern string_t UserPrefix;
extern char CamelotPrefix[4];

extern int yylineno;
extern string_t yyinname;

extern void init_global();

extern string_t HeaderFileName;
extern string_t UserFileName;
extern string_t ServerFileName;

extern identifier_t InitRoutineName;
extern identifier_t SetMsgTypeName;
extern identifier_t ReplyPortName;
extern identifier_t ReplyPortIsOursName;
extern identifier_t MsgTypeVarName;
extern identifier_t DeallocPortRoutineName;
extern identifier_t AllocPortRoutineName;
extern identifier_t ServerProcName;

extern void more_global();

extern char NewCDecl[];
extern char LintLib[];

#endif	/* _GLOBAL_H_ */
