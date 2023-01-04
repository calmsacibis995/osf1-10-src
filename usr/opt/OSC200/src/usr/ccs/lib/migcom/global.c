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
static char	*sccsid = "@(#)$RCSfile: global.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:22:20 $";
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
 *  ABSTRACT:
 *	exports most of the global variables.
 * 	init_global sets input file name before yyparse
 *		is called.
 *	more_global is called after yyparse and before
 *		any code generation to initialize names
 *		that are derived from the subsystem name.
 *
 */

#include "string.h"
#include "global.h"

boolean_t BeQuiet = FALSE;
boolean_t BeVerbose = FALSE;
boolean_t UseMsgRPC = TRUE;
boolean_t GenSymTab = FALSE;

boolean_t IsKernel = FALSE;
boolean_t IsCamelot = FALSE;

string_t RCSId = strNULL;

string_t SubsystemName = strNULL;
u_int SubsystemBase = 0;

string_t MsgType = strNULL;
string_t WaitTime = strNULL;
string_t ErrorProc = "MsgError";
string_t ServerPrefix = strNULL;
string_t UserPrefix = strNULL;
char CamelotPrefix[4] = "op_";

string_t yyinname;

char NewCDecl[] = "(defined(__STDC__) || defined(c_plusplus))";
char LintLib[] = "defined(LINTLIBRARY)";

void
init_global()
{
    yyinname = strmake("<no name yet>");
}

string_t HeaderFileName = strNULL;
string_t UserFileName   = strNULL;
string_t ServerFileName = strNULL;

identifier_t InitRoutineName;
identifier_t SetMsgTypeName;
identifier_t ReplyPortName;
identifier_t ReplyPortIsOursName;
identifier_t MsgTypeVarName;
identifier_t DeallocPortRoutineName;
identifier_t AllocPortRoutineName;
identifier_t ServerProcName;

void
more_global()
{
    if (SubsystemName == strNULL)
	fatal("no SubSystem declaration");

    if (HeaderFileName == strNULL)
	HeaderFileName = strconcat(SubsystemName, ".h");
    if (UserFileName == strNULL)
	UserFileName = strconcat(SubsystemName, "User.c");
    if (ServerFileName == strNULL)
	ServerFileName = strconcat(SubsystemName, "Server.c");

    InitRoutineName = strconcat("init_", SubsystemName);
    SetMsgTypeName = strconcat("set_msg_type_", SubsystemName);
    MsgTypeVarName = strconcat("msg_type_var_", SubsystemName);
    ReplyPortName = strconcat(SubsystemName, "_reply_port");
    ReplyPortIsOursName = strconcat(ReplyPortName, "_is_ours");
    DeallocPortRoutineName = strconcat(ReplyPortName, "_dealloc");
    AllocPortRoutineName = strconcat(ReplyPortName, "_alloc");
    ServerProcName = strconcat(SubsystemName, "_server");
}
