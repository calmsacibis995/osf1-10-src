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
static char	*sccsid = "@(#)$RCSfile: header.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/08/17 13:36:50 $";
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
 *  Abstract:
 *	routines to write pieces of the Header module.
 *	exports WriteHeader which directs the writing of
 *	the Header module
 */

#include "write.h"
#include "utils.h"
#include "global.h"

static void
WriteIncludes(file)
    FILE *file;
{
    fprintf(file, "#include <mach/kern_return.h>\n");
    fprintf(file, "#if\t%s || %s\n", NewCDecl, LintLib);
    fprintf(file, "#include <mach/port.h>\n");
    fprintf(file, "#include <mach/message.h>\n");
    fprintf(file, "#endif\n");
    fprintf(file, "\n");
}

static void
WriteExternalDecls(file)
    FILE *file;
{
    fprintf(file, "mig_external void %s\n", InitRoutineName);
    fprintf(file, "#if\t%s\n", NewCDecl);
    fprintf(file, "    (port_t rep_port);\n");
    fprintf(file, "#else\n");
    fprintf(file, "    ();\n");
    fprintf(file, "#endif\n");
}

static void
WriteProlog(file)
    FILE *file;
{
    fprintf(file, "#ifndef\t_%s\n", SubsystemName);
    fprintf(file, "#define\t_%s\n", SubsystemName);
    fprintf(file, "\n");
    fprintf(file, "/* Module %s */\n", SubsystemName);
    fprintf(file, "\n");

    WriteIncludes(file);

    fprintf(file, "#ifndef\tmig_external\n");
    fprintf(file, "#define mig_external extern\n");
    fprintf(file, "#endif\n");
    fprintf(file, "\n");

    WriteExternalDecls(file);
}

static void
WriteEpilog(file)
    FILE *file;
{
    fprintf(file, "\n");
    fprintf(file, "#endif\t/* _%s */\n", SubsystemName);
}

static void
WriteRoutine(file, rt)
    FILE *file;
    routine_t *rt;
{
    fprintf(file, "\n");
    fprintf(file, "/* %s %s */\n", rtRoutineKindToStr(rt->rtKind), rt->rtName);
    fprintf(file, "mig_external %s %s\n", ReturnTypeStr(rt), rt->rtUserName);
    fprintf(file, "#if\t%s\n", LintLib);
    fprintf(file, "    (");
    WriteList(file, rt->rtArgs, WriteNameDecl, akbUserArg, ", " , "");
    fprintf(file, ")\n");
    WriteList(file, rt->rtArgs, WriteVarDecl, akbUserArg, ";\n", ";\n");
    fprintf(file, "{ ");
    if (!rt->rtProcedure)
	fprintf(file, "return ");
    fprintf(file, "%s(", rt->rtUserName);
    WriteList(file, rt->rtArgs, WriteNameDecl, akbUserArg, ", ", "");
    fprintf(file, "); }\n");
    fprintf(file, "#else\n");
    fprintf(file, "#if\t%s\n", NewCDecl);
    fprintf(file, "(\n");
    WriteList(file, rt->rtArgs, WriteVarDecl, akbUserArg, ",\n", "\n");
    fprintf(file, ");\n");
    fprintf(file, "#else\n");
    fprintf(file, "    ();\n");
    fprintf(file, "#endif\n");
    fprintf(file, "#endif\n");
}

void
WriteHeader(file, stats)
    FILE *file;
    statement_t *stats;
{
    register statement_t *stat;

    WriteProlog(file);
    for (stat = stats; stat != stNULL; stat = stat->stNext)
	switch (stat->stKind)
	{
	  case skRoutine:
	    WriteRoutine(file, stat->stRoutine);
	    break;
	  case skImport:
	  case skUImport:
	    WriteImport(file, stat->stFileName);
	    break;
	  case skSImport:
	    break;
	  default:
	    fatal("WriteUser(): bad statement_kind_t (%d)",
		  (int) stat->stKind);
	}
    WriteEpilog(file);
}
