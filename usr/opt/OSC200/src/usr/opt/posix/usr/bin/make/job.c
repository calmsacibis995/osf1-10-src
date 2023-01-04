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
static char *rcsid = "@(#)$RCSfile: job.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:02:13 $";
#endif

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*-
 * job.c --
 *	handle the creation etc. of our child processes.
 *
 * Interface:
 *	Job_CheckCommands   	Verify that the commands for a target are
 *	    	  	    	ok. Provide them if necessary and possible.
 *
 *	Job_Touch 	    	Update a target without really updating it.
 */

#include "make.h"
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "job.h"
#include "pmake_msg.h"

extern int     errno;
extern nl_catd catd ;

/*
 * JOB macro used only during development for tracing JOB module.
 * Use the compiler option -DJOBTRACE or -DALLTRACE to turn on.
 */

#if defined(JOBTRACE) || defined(ALLTRACE)
#undef JOBTRACE
#define JOBTRACE(message) { \
   printf("JOBTRACE: %s.\n",message);\
   }
#else
#define JOBTRACE(message)
#endif

/*-
 *-----------------------------------------------------------------------
 * Job_Touch --
 *	Touch the given target. Called by JobStart when the -t flag was
 *	given
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The data modification of the file is changed. In addition, if the
 *	file did not exist, it is created.
 *-----------------------------------------------------------------------
 */
void
Job_Touch (GNode *gn, Boolean silent)
{
    /* GNode         *gn;	      	The node of the file to touch */
    /* Boolean 	  silent;   	TRUE if should not print messages */

    int		  streamID;   	/* ID of stream opened to do the touch */
    struct timeval times[2];	/* Times for utimes() call */

    JOBTRACE("Job_Touch: Touch the given target");

    if (!silent) {
	printf (catgets(catd, MS_MAKE, TOUCH, "touch %s\n"), gn->name);
    }

    if (noExecute) {
	return;
    }

    if (gn->type & OP_ARCHV) {
	Arch_Touch (gn);
    } else {
	char	*file = gn->path ? gn->path : gn->name;

	times[0].tv_sec = times[1].tv_sec = now;
	times[0].tv_usec = times[1].tv_usec = 0;
	if (utimes(file, times) < 0){
	    streamID = open (file, O_RDWR | O_CREAT, 0666);

	    if (streamID >= 0) {
		char	c;

		/*
		 * Read and write a byte to the file to change the
		 * modification time, then close the file.
		 */
		if (read(streamID, &c, 1) == 1) {
		    lseek(streamID, 0L, SEEK_SET);
		    write(streamID, &c, 1);
		}
		
		(void)close (streamID);
	    } else
		printf(catgets(catd, MS_MAKE, TOUCHERR,
		       "*** couldn't touch %s: %s"), file, strerror(errno));
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * Job_CheckCommands --
 *	Make sure the given node has all the commands it needs. 
 *
 * Results:
 *	TRUE if the commands list is/was ok.
 *
 * Side Effects:
 *	The node will have commands from the .DEFAULT rule added to it
 *	if it needs them.
 *-----------------------------------------------------------------------
 */
Boolean
Job_CheckCommands (GNode *gn, void (*abortProc)(const char *, ...))
{
    /* GNode          *gn;   The target whose commands need verifying */
    /* void    	      (*abortProc)(const char *, ...); */
    /* Function to abort with message */

  JOBTRACE("Job_CheckCommands: Make sure the given node has all the commands it needs")

    if (OP_NOP(gn->type) && Lst_IsEmpty (gn->commands)) {
	/*
	 * No commands. Look for .SCCS_GET rule from which we might infer
	 * commands 
	 */

	if ((SCCS_GET != NILGNODE) && (TrySccsGet == TRUE) && (!Lst_IsEmpty(SCCS_GET->commands)) && 
	    (gn->SccsGetFileExists == TRUE)) {
             

	    Make_HandleTransform(SCCS_GET, gn);
	    Var_Set (IMPSRC, Var_Value (TARGET, gn), gn);
	/*
	 * No commands. Look for .DEFAULT rule from which we might infer
	 * commands 
	 */
	   } else if ((DEFAULT != NILGNODE) && !Lst_IsEmpty(DEFAULT->commands)) {
	    /*
	     * Make only looks for a .DEFAULT if the node was never the
	     * target of an operator, so that's what we do too. If
	     * a .DEFAULT was given, we substitute its commands for gn's
	     * commands and set the IMPSRC variable to be the target's name
	     * The DEFAULT node acts like a transformation rule, in that
	     * gn also inherits any attributes or sources attached to
	     * .DEFAULT itself.
	     */
	    Make_HandleTransform(DEFAULT, gn);
	    Var_Set (IMPSRC, Var_Value (TARGET, gn), gn);

	   } else if (Dir_MTime (gn) == 0) {
	    /*
	     * The node wasn't the target of an operator we have no .DEFAULT
	     * rule to go on and the target doesn't already exist. There's
	     * nothing more we can do for this branch. If the -k flag wasn't
	     * given, we stop in our tracks, otherwise we just don't update
	     * this node's parents so they never get examined. 
	     */
	    if (keepgoing) {
		printf (catgets(catd, MS_MAKE, MCONT,
		  "Can't figure out how to make %s (continuing)\n"), gn->name);
		return (FALSE);
	    } else {
		(*abortProc) (catgets(catd, MS_MAKE, MSTOP,
		  "Can't figure out how to make %s. Stop"), gn->name);
		return(FALSE);
	    }
	}
    }
    return (TRUE);
}
