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
static char     *sccsid = "@(#)$RCSfile: openchild.c,v $ $Revision: 4.3.7.2 $ (DEC) $Date: 1993/06/07 23:31:44 $";
#endif
/*
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef basename
#undef basename
#endif
#endif


/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.7 88/02/08 
 */

/*
 * Open two pipes to a child process, one for reading, one for writing.
 * The pipes are accessed by FILE pointers. This is NOT a public
 * interface, but for internal use only!
 */
#include <stdio.h>
#include <stddef.h>

extern char *strrchr();
#ifdef _NO_PROTO
/* extern char *sprintf(); */
#endif /* _NO_PROTO */
static char *basename();
static char *SHELL = "/bin/sh";


/*
 * returns pid, or -1 for failure
 */
_openchild(command, fto, ffrom)
	char *command;
	FILE **fto;
	FILE **ffrom;
{
	int i;
	int pid;
	int pdto[2];
	int pdfrom[2];
	char *com;

		
	if (pipe(pdto) < 0) {
		goto error1;
	}
	if (pipe(pdfrom) < 0) {
		goto error2;
	}
	switch (pid = vfork()) {
	case -1:
		goto error3;

	case 0:
		/* 
		 * child: read from pdto[0], write into pdfrom[1]
		 */
		(void) close(0); 
		(void) dup(pdto[0]);
		(void) close(1); 
		(void) dup(pdfrom[1]);
		for (i = _rpc_dtablesize() - 1; i >= 3; i--) {
			(void) close(i);
		}
		com = (char *) malloc((size_t) strlen(command) + 6);
		if (com == NULL) {
			_exit(~0);
		}	
		(void) sprintf(com, "exec %s", command);
		execl(SHELL, basename(SHELL), "-c", com, NULL);
		_exit(~0);
 
	default:
		/*
		 * parent: write into pdto[1], read from pdfrom[0]
		 */
		*fto = fdopen(pdto[1], "w");
		(void) close(pdto[0]);
		*ffrom = fdopen(pdfrom[0], "r");
		(void) close(pdfrom[1]);
		break;
	}
	return (pid);

	/*
	 * error cleanup and return
	 */
error3:
	(void) close(pdfrom[0]);
	(void) close(pdfrom[1]);
error2:
	(void) close(pdto[0]);
	(void) close(pdto[1]);
error1:
	return (-1);
}

static char *
basename(path)
	char *path;
{
	char *p;

	p = strrchr(path, '/');
	if (p == NULL) {
		return (path);
	} else {
		return (p + 1);
	}
}
