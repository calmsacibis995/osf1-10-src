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
static char	*sccsid = "@(#)$RCSfile: error.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:22:14 $";
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
 * ABSTRACT:
 *	Routines to print out error messages.
 *  Exports variables;
 *	errors - the total number of errors encountered
 *  Exports routines:
 *	fatal, warn, error, unix_error_string, set_program_name
 */

#include <stdio.h>
#include <varargs.h>
#include "global.h"
#include "error.h"

extern int yylineno;
extern char *yyinname;

static char *program;
int errors = 0;

#if	defined(_BLD) && defined(NO_VFPRINTF)
#ifndef	_DOPRNT
#define	_DOPRNT	_doprnt
#endif
int
vfprintf(iop, fmt, ap)
	FILE *iop;
	char *fmt;
	va_list ap;
{
	int len;
	char localbuf[BUFSIZ];

	if (iop->_flag & _IONBF) {
		iop->_flag &= ~_IONBF;
		iop->_ptr = iop->_base = localbuf;
		len = _DOPRNT(fmt, ap, iop);
		(void) fflush(iop);
		iop->_flag |= _IONBF;
		iop->_base = NULL;
		iop->_bufsiz = 0;
		iop->_cnt = 0;
	} else
		len = _DOPRNT(fmt, ap, iop);

	return (ferror(iop) ? EOF : len);
}
#endif	/* defined(_BLD) && defined(NO_VFPRINTF) */

/*ARGSUSED*/
/*VARARGS1*/
void
fatal(format, va_alist)
    char *format;
    va_dcl
{
    va_list pvar;
    va_start(pvar);
    fprintf(stderr, "%s: fatal: ", program);
    vfprintf(stderr, format, pvar);
    fprintf(stderr, "\n");
    va_end(pvar);
    exit(1);
}

/*ARGSUSED*/
/*VARARGS1*/
void
warn(format, va_alist)
    char *format;
    va_dcl
{
    va_list pvar;
    va_start(pvar);
    if (!BeQuiet && (errors == 0))
    {
	fprintf(stderr, "\"%s\", line %d: warning: ", yyinname, yylineno-1);
	vfprintf(stderr, format, pvar);
	fprintf(stderr, "\n");
    }
    va_end(pvar);
}

/*ARGSUSED*/
/*VARARGS1*/
void
error(format, va_alist)
    char *format;
    va_dcl
{
    va_list pvar;
    va_start(pvar);
    fprintf(stderr, "\"%s\", line %d: ", yyinname, yylineno-1);
    vfprintf(stderr, format, pvar);
    fprintf(stderr, "\n");
    va_end(pvar);
    errors++;
}

char *
unix_error_string(errno)
    int errno;
{
    extern int sys_nerr;
    extern char *sys_errlist[];
    static char buffer[256];
    char *error_mess;

    if ((0 <= errno) && (errno < sys_nerr))
	error_mess = sys_errlist[errno];
    else
	error_mess = "strange errno";

    sprintf(buffer, "%s (%d)", error_mess, errno);
    return buffer;
}

void
set_program_name(name)
    char *name;
{
    program = name;
}
