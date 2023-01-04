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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxdiff/invokediff.c,v 1.1.2.2 92/08/03 09:48:43 Dave_Hill Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *
 *	dxdiff
 *
 *	invokediff.c - invokes diff pipeline to process info
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 22nd April 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	April 25th 1988		Laurence P. G. Cable
 *
 *	Changed return status code to ensure that error message is only
 *	displayed when diff returns 2 to indicate an error.
 */

static char sccsid[] = "@(#)invokediff.c	1.10	17:45:25 2/21/89";


#include <stdio.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include <sys/limits.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "y.tab.h"
#include "filestuff.h"
#include "parsediff.h"
#include "alloc.h"
#include "dxdiff.h"

#define	WRITEFD 1
#define	READFD  0

extern	char **environ;

/********************** Private Routines ************************/


/********************************
 *
 *     FreeEnvironmentList
 *
 ********************************/

static 
FreeEnvironmentList(env)
	char **env;
{
	register char **p;

	if (env == (char **)NULL)
		return;

	for (p = env; *p; p++)
		XtFree(*p);

	XtFree((char *)env);
}

/********************************
 *
 *     CreateEnvironmentList
 *
 ********************************/

static char **
CreateEnvironmentList(shellvars)
	EnvVariables	**shellvars;
{
	int		nenv = 0,
			noff = 0;
	char		**myenv = (char **)NULL;
	register char	**p,**q;
	EnvVariables	**sp;
	

	if (environ != (char **)NULL) {
		for (p = environ; *p; p++, nenv++)
		;
		noff = nenv;
	}

	if (shellvars != (EnvVariables **)NULL) 
		for (sp = shellvars; *sp; sp++, nenv++)
		;

	if ((myenv = (char **)XtMalloc(nenv * sizeof (char *) + 1)) == 
	    (char **)NULL) {	/* error */
		return myenv;
	}
	myenv[nenv] = (char *)NULL;

	for (p = environ, q = myenv; q < myenv + noff; p++, q++) {
		int l;

		if ((*q = (char *)XtMalloc((l = strlen(*p) + 1))) 
		    == (char *)NULL) {	/* error */
			FreeEnvironmentList(myenv);
			return (char **)NULL;
		}
		bcopy(*p, *q, l);
	}

	for (sp = shellvars; q < myenv + nenv; sp++, q++) {
		int	sl,vl;
		Boolean notnull; 

		sl = strlen((*sp)->shellvariable); 
		vl = ((notnull = ((*sp)->value != (char *)NULL)) ? strlen((*sp)->value) : 0) + 1;

		if ((*q = (char *)XtMalloc(sl + vl)) == (char *)NULL) {
			FreeEnvironmentList(myenv);
			return (char **)NULL;
		}

		bcopy((*sp)->shellvariable + 1, *q, sl--);
		(*q)[sl] = '=';
		if (notnull) {
			bcopy((*sp)->value, (*q) + ++sl, vl);
		} else {
			(*q)[sl + 1] = '\0';
		}
	}

	return myenv;
}

/********************************
 *
 *     _InvokeDiff
 *
 ********************************/


static Boolean
_InvokeDiff(name, argv, environ, dlb)
	char 			 *name,
				 **argv,
				 **environ;
	register DiffListBlkPtr  dlb;
{
	int		pid;
	int		pipefds[2];
	int		ret;
#ifdef __osf__
	unsigned long	status;
#else
	union	wait	status;
#endif

	if (pipe(pipefds) == -1) {	/* error */
		perror("failed pipe(2) in _InvokeDiff\n");
		return False;
	}

	if ((pid = vfork())) {	/* parent */
		close(pipefds[WRITEFD]);

		if (pid == -1) {	/* error */
			perror("failed vfork(2) in _InvokeDiff\n");
			close(pipefds[READFD]);
			return False;
		}

		SetyylexInputStream(pipefds[READFD]);

		parsediff(dlb);

		while ((ret = wait(&status)) != -1 && ret != pid)
		;

		if (ret == -1) {	/* error */
			perror("bad wait(2) in _InvokeDiff\n");
		}
		close(pipefds[READFD]);

#ifdef __osf__
		return ( ret == pid && WEXITSTATUS(status) < 2);
#else
		return ( ret == pid && status.w_retcode < 2);
#endif
	} else {		/* child */
		close(pipefds[READFD]);
		close(1);	/* stdout */
		close(2);	/* stderr */

		if ((ret = fcntl(pipefds[WRITEFD], F_DUPFD, 1)) == -1 ||
		     ret != 1) { /* error */
			_exit(1);
		}	/* get the pipe on stdout */

		if ((ret = fcntl(pipefds[WRITEFD], F_DUPFD, 2)) == -1 ||
		     ret != 2) { /* error */
			_exit(1);
		}	/* get the pipe on stderr */

		close(pipefds[WRITEFD]); /* throw it away */

		execve(name, argv, environ);
		perror("failed execve(2) in _InvokeDiff\n");
		close(ret);
		_exit(1);	/* now we are in trouble! */
	}
}

/********************** Public Routines ************************/

/********************************
 *
 *     InvokeDiff
 *
 ********************************/

static EnvVariables	lf = { DXDIFFSHELLVARL, (char *)NULL },
			rf = { DXDIFFSHELLVARR, (char *)NULL },
			*shv[] = { &lf, &rf, (EnvVariables *)NULL };

Boolean
InvokeDiff(argv, leftfile, rightfile, dlb)
	char			 **argv,
				 *leftfile,
				 *rightfile;
	register DiffListBlkPtr  dlb;
{
	Boolean		ret;
	char		**myenv;

	lf.value = leftfile;
	rf.value = rightfile;

	myenv = CreateEnvironmentList(shv);

	ret  = _InvokeDiff(argv[0], ((argv[1]) ? argv + 1 : (char **)NULL), 
			   myenv, dlb);

	FreeEnvironmentList(myenv);
	
	return ret;
}
