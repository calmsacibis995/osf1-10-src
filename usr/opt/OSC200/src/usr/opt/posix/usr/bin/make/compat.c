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
static char *rcsid = "@(#)$RCSfile: compat.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:01:35 $";
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
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*-
 * compat.c --
 *	The routines in this file implement the full-compatibility
 *	mode of make. Most of the special functionality of make
 *	is available in this mode. Things not supported:
 *	    - different shells.
 *	    - friendly variable substitution.
 *
 * Interface:
 *	Compat_Run	    Initialize things for this module and recreate
 *	    	  	    thems as need creatin'
 */

#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <sys/signal.h>
#include    <sys/wait.h>
#include    <sys/errno.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    <unistd.h>
#include    "make.h"
#include    "pmake_msg.h"

extern nl_catd	catd;

/*
 * The following array is used to make a fast determination of which
 * characters are interpreted specially by the shell.  If a command
 * contains any of these characters, it is executed by the shell, not
 * directly by us.
 */

static char 	    meta[256];

static GNode	    *curTarg;
static int  	    CompatRunCommand(char *, GNode *);


/*
 * COMPATTRACE macro used only during development for tracing suffix module.
 * Use the compiler option -DCOMPATTRACE or -DALLTRACE to turn on.
 */

#if defined(COMPATTRACE) || defined(ALLTRACE)
#undef COMPATTRACE
#define COMPATTRACE(message) { \
   printf("COMPATTRACE: %s.\n",message);\
   }
#else
#define COMPATTRACE(message)
#endif

/*-
 *-----------------------------------------------------------------------
 * CompatInterrupt --
 *	Interrupt the creation of the current target and remove it if
 *	it ain't precious.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The target is removed and the process exits. If .INTERRUPT exists,
 *	its commands are run first WITH INTERRUPTS IGNORED..
 *
 *-----------------------------------------------------------------------
 */
static void
CompatInterrupt (int signo)
{
    /* int	    signo; */

    GNode   *gn;
   
    COMPATTRACE("CompatInterrupt: Interrupt the creation of the current target");

    if ((curTarg != NILGNODE) && !Targ_Precious (curTarg)) {
	struct stat st;
	char 	  *file = Var_Value (TARGET, curTarg);

	if (lstat (file, &st) == SUCCESS && (!S_ISDIR(st.st_mode)) &&
	    unlink (file) == SUCCESS) {
	    fprintf (stderr, catgets(catd, MS_MAKE, REMOVED,
	    			     "*** %s removed\n"), file);
	}

	/*
	 * Run .INTERRUPT only if hit with interrupt signal
	 */
	if (signo == SIGINT) {
	    gn = Targ_FindNode(".INTERRUPT", TARG_NOCREATE);
	    if (gn != NILGNODE) {
		Lst_ForEach(gn->commands, (int(*)(void*,void*))CompatRunCommand, (ClientData)gn);
	    }
	}
    }
    if (signo != SIGQUIT) {
	(void) signal(signo, SIG_DFL);
	kill(getpid(), signo);
    }
    exit (2);
}

/*-
 *-----------------------------------------------------------------------
 * CompatRunCommand --
 *	Execute the next command for a target. If the command returns an
 *	error, the node's made field is set to ERROR and creation stops.
 *
 * Results:
 *	0 if the command succeeded, 1 if an error occurred.
 *
 * Side Effects:
 *	The node's 'made' field may be set to ERROR.
 *
 *-----------------------------------------------------------------------
 */
static int
CompatRunCommand (char *cmd, GNode *gn)
{
    /* char    	  *cmd;	    	Command to execute */
    /* GNode   	  *gn;    	Node from which the command came */

    char    	  *cmdStart;	/* Start of expanded command */
    char          *cp;
    Boolean 	  silent,   	/* Don't print command */
		  execute,	/* Execute the command */
		  errCheck, 	/* Check errors */
		  plusPrefix;	/* Command was prefixed with a plus */
    int 	  reason;   	/* Reason for child's death */
    int	    	  status;   	/* Description of child's death */
    int	    	  cpid;	    	/* Child actually found */
    ReturnStatus  stat;	    	/* Status of fork */
    LstNode 	  cmdNode;  	/* Node where current command is located */
    char    	  **av;	    	/* Argument vector for thing to exec */
    Boolean 	  local;    	/* TRUE if command should be executed
				 * locally */

    plusPrefix = FALSE;
    silent = gn->type & OP_SILENT;
    errCheck = !(gn->type & OP_IGNORE);
    execute = !(noExecute || touchFlag);

    COMPATTRACE("CompatRunCommand: Execute the next command for a target");

#ifdef DEBUG_FLAG	
    if (DEBUG(MAKE)) {
      if (gn->name != NULL) {
	printf("CompatRunCommand: Running \"%s\"\n# command for target \"%s\".\n", cmd, gn->name);
      }
    }
#endif

    cmdNode = Lst_Member (gn->commands, (ClientData)cmd);
    cmdStart = Var_Subst (cmd, gn, FALSE);

    /*
     * Str_Break will return an argv with a NULL in av[1], thus causing
     * execvp to choke and die horribly. Besides, how can we execute a null
     * command? In any case, we warn the user that the command expanded to
     * nothing (is this the right thing to do?).
     */
     
    if (*cmdStart == '\0') {
	Error(catgets(catd, MS_MAKE, EMPTYSTR,
		"%s expands to empty string"), cmd);
	return(0);
    } else {
	cmd = cmdStart;
    }

    Lst_Replace (cmdNode, (ClientData)cmdStart);

    while ((*cmd == '@') || (*cmd == '-') || (*cmd == '+')) {
	if (*cmd == '@') {
	    silent = TRUE;
	} else if (*cmd == '+') {
	    plusPrefix = TRUE;
	    execute = TRUE;
	} else {
	    errCheck = FALSE;
	}
	cmd++;
    }
    
    /*
     * Search for meta characters in the command. If there are no meta
     * characters, there's no need to execute a shell to execute the
     * command.
     */
    for (cp = cmd; !meta[*cp]; cp++) {
	continue;
    }

    /*
     * Print the command before echoing if we're not supposed to be quiet for
     * this one.
     * 
     * We want to print the command if:
     *	- the command has a plus prefix and is not suppressed by the user
     *	- the command is not suppressed by the user, and this is not
     *	  simply a query or touch request
     *	- we are responding to the -n option (noExecute) and this is
     * 	  not a query or touch request as well
     *
     */
    if ( (plusPrefix && !silent) || 
	 (!silent && !queryFlag && !touchFlag) ||
	 (noExecute && !queryFlag && !touchFlag)) {
	printf ("%s\n", cmd);
	fflush(stdout);
    }

    /*
     * If we're not supposed to execute any commands, this is as far as
     * we go...
     */
    if (!execute) {
	return (0);
    }
    
    if (*cp != '\0') {
	/*
	 * If *cp isn't the null character, we hit a "meta" character and
	 * need to pass the command off to the shell. We give the shell the
	 * -e flag as well as -c if it's supposed to exit when it hits an
	 * error.
	 */
	static char	*shargv[4] = { "/bin/sh" };

	shargv[1] = (errCheck ? "-ec" : "-c");
	shargv[2] = cmd;
	shargv[3] = (char *)NULL;
	av = shargv;
    } else {
	/*
	 * No meta-characters, so no need to exec a shell. Break the command
	 * into words to form an argument vector we can execute.
	 */
	av = Str_Break((char *)NULL, cmd, (int *)NULL);
    }
    
    local = TRUE;

    /*
     * Fork and execute the single command. If the fork fails, we abort.
     */
    cpid = fork();
    if (cpid < 0) {
	Fatal(catgets(catd, MS_MAKE, FORKFAIL, "Could not fork"));
    }
    if (cpid == 0) {
	if (local) {
	    char *msg;

	    execvp(av[0], av);
	    msg = catgets(catd, MS_MAKE, NOTFOUND, ": not found\n") ;
	    write (2, av[0], strlen (av[0]));
	    write (2, msg  , strlen (msg  ));
	} else {
	    (void)execv(av[0], av);
	}
	exit(1);
    }
    
    /*
     * The child is off and running. Now all we can do is wait...
     */
    while (1) {

	while ((stat = wait(&reason)) != cpid) {
	    if (stat == -1 && errno != EINTR) {
		break;
	    }
	}
	
	if (stat > -1) {
	    if (WIFSTOPPED(reason)) {
		status = WSTOPSIG(reason);		/* stopped */
	    } else if (WIFEXITED(reason)) {
		status = WEXITSTATUS(reason);		/* exited */
		if (status != 0) {
		    printf (catgets(catd, MS_MAKE, ERRCODE,
			    "*** Error code %d"), status);
		}
	    } else {
		status = WTERMSIG(reason);		/* signaled */
		printf (catgets(catd, MS_MAKE, SIGNAL, "*** Signal %d"), status);
	    } 
	    
	    if (!WIFEXITED(reason) || (status != 0)) {
		if (errCheck) {
		    gn->made = ERROR;
		    if (keepgoing) {
			/*
			 * Abort the current target, but let others
			 * continue.
			 */
			printf (catgets(catd, MS_MAKE, CONTINUE,
				" (continuing)\n"));
		    }
		} else {
		    /*
		     * Continue executing commands for this target.
		     * If we return 0, this will happen...
		     */
		    printf (catgets(catd, MS_MAKE, IGNORE, " (ignored)\n"));
		    status = 0;
		}
	    }
	    break;
	} else {
	    Fatal (catgets(catd, MS_MAKE, WAITERR, "error in wait: %d"), stat);
	    /*NOTREACHED*/
	}
    }

    return (status);
}

/*-
 *-----------------------------------------------------------------------
 * CompatMake --
 *	Make a target.
 *
 * Results:
 *	0
 *
 * Side Effects:
 *	If an error is detected and not being ignored, the process exits.
 *
 *-----------------------------------------------------------------------
 */
static int
CompatMake (GNode *gn, GNode *pgn)
{
    /* GNode   	  *gn;	    The node to make */
    /* GNode   	  *pgn;	    Parent to abort if necessary */

    COMPATTRACE("CompatMake: Make a target");

    if (gn->made == UNMADE) {
	/*
	 * First mark ourselves to be made, then apply whatever transformations
	 * the suffix module thinks are necessary. Once that's done, we can
	 * descend and make all our children. If any of them has an error
	 * but the -k flag was given, our 'make' field will be set FALSE again.
	 * This is our signal to not attempt to do anything but abort our
	 * parent as well.
	 */
	gn->make = TRUE;
	gn->made = BEINGMADE;

	if (DEBUG(MAKE)) {
            printf(catgets(catd, MS_DEBUG, MAKE001, "CompatMake: Finding implied suffix dependencies and relationships for \"%s\".....\n") ,gn->name);
	  }
	Suff_FindDeps (gn);

	if (DEBUG(MAKE)) {
            printf(catgets(catd, MS_DEBUG, MAKE002, "CompatMake: All implied suffix dependencies and  relationships for \"%s\" are now determined.\n"),gn->name);
	  }


	if (DEBUG(MAKE)) {
            printf(catgets(catd, MS_DEBUG, MAKE003, "CompatMake: Running creation commands on all children of \"%s\".....\n"), gn->name);
	  }
	Lst_ForEach (gn->children, (int(*)(void*,void*))CompatMake, (ClientData)gn);
	if (!gn->make) {
	    gn->made = ABORTED;
	    pgn->make = FALSE;
	    return (0);
	}


	
	if (Lst_Member ((Lst)gn->iParents, (ClientData)pgn) != NILLNODE) {
	  if (DEBUG(MAKE)) {
	    printf(catgets(catd, MS_DEBUG, MAKE004, "CompatMake: Implied Parents for \"%s\" were found.\n"));
	  }
	  Var_Set (IMPSRC, Var_Value(TARGET, gn), pgn);
	} else {
	  if (DEBUG(MAKE)) {
	    printf(catgets(catd, MS_DEBUG, MAKE005,
			   "CompatMake: No Implied Parents for \"%s\" were found.\n"), gn->name);
	  }
	}



#ifdef DEBUG_FLAG	
	if (DEBUG(MAKE)) {
            printf("CompatMake: All children of \"%s\" are considered \"made\".\n",gn->name);
	  }
#endif
	/*
	 * All the children were made ok. Now cmtime contains the modification
	 * time of the newest child, we need to find out if we exist and when
	 * we were modified last. The criteria for datedness are defined by the
	 * Make_OODate function.
	 */
	if (DEBUG(MAKE)) {
	    printf(catgets(catd, MS_DEBUG, MAKE006,
		"CompatMake: Examining modification time of this target \"%s\" as compared to its newest child.\n"), gn->name);
	}
	if (!Make_OODate(gn)) {
	    gn->made = UPTODATE;
	    if (DEBUG(MAKE)) {
	    printf(catgets(catd, MS_DEBUG, MAKE007, "CompatMake: With respect of its newest child, modification time of \"%s\"\n was considered up-to-date.\n"), gn->name);
	    }
	    return (0);
	} else if (DEBUG(MAKE)) {
	    printf(catgets(catd, MS_DEBUG, MAKE007, "CompatMake: With respect of its newest child, modification time of \"%s\"\n was considered out-to-date.\n"), gn->name);
	}

	/*
	 * If the user is just seeing if something is out-of-date, 
	 * set exit value now to tell him/her "yes".
	 */
	queryExit = 1;

	/*
	 * We need to be re-made. We also have to make sure we've got a $?
	 * variable. To be nice, we also define the $> variable using
	 * Make_DoAllVar().
	 */

	if (DEBUG(MAKE)) {
	  printf(catgets(catd, MS_DEBUG, MAKE008,
		 "CompatMake: Target \"%s\" must be remade!\n"), gn->name);
	}

	Make_DoAllVar(gn);
		    
	/*
	 * Alter our type to tell if errors should be ignored or things
	 * should not be printed so CompatRunCommand knows what to do.
	 */
	if (Targ_Ignore (gn)) {
	    gn->type |= OP_IGNORE;
	}
	if (Targ_Silent (gn)) {
	    gn->type |= OP_SILENT;
	}

	if (Job_CheckCommands (gn, Fatal)) {
	    /*
	     * Our commands are ok, but we still have to worry about the -t
	     * flag...
	     */
	    curTarg = gn;
	    if (DEBUG(MAKE)) {
	      printf("CompatMake: Running all commands for \"%s\".\n", gn->name);
	    }
	    Lst_ForEach (gn->commands, (int(*)(void*,void*))CompatRunCommand, (ClientData)gn);
	    if (touchFlag)
		Job_Touch(gn, gn->type & OP_SILENT);
	    curTarg = NILGNODE;
	} else {
	    gn->made = ERROR;
	}

	if (gn->made != ERROR) {
	    /*
	     * If the node was made successfully, mark it so, update
	     * its modification time and timestamp all its parents. Note
	     * that for .ZEROTIME targets, the timestamping isn't done.
	     * This is to keep its state from affecting that of its parent.
	     */
	    gn->made = MADE;
	    /*
	     * This is what Make does and it's actually a good thing, as it
	     * allows rules like
	     *
	     *	cmp -s y.tab.h parse.h || cp y.tab.h parse.h
	     *
	     * to function as intended. Unfortunately, thanks to the stateless
	     * nature of NFS (and the speed of this program), there are times
	     * when the modification time of a file created on a remote
	     * machine will not be modified before the stat() implied by
	     * the Dir_MTime occurs, thus leading us to believe that the file
	     * is unchanged, wreaking havoc with files that depend on this one.
	     */
	    if (noExecute || Dir_MTime(gn) == 0) {
		gn->mtime = now;
	    }
	    if (DEBUG(MAKE)) {
		printf(catgets(catd, MS_DEBUG, MAKE009, "CompatMake: Target \"%s\" has a new modification time: %s\n"),
		       gn->name, Targ_FmtTime(gn->mtime));
	    }
	    pgn->childMade = TRUE;
	    Make_TimeStamp(pgn, gn);
	} else if (keepgoing) {
	    pgn->make = FALSE;
	} else {
	    printf (catgets(catd, MS_MAKE, STOP, "\n\nStop.\n"));
	    exit (2);
	}
    } else if (gn->made == ERROR) {
	/*
	 * Already had an error when making this beastie. Tell the parent
	 * to abort.
	 */
	pgn->make = FALSE;
    } else {
	if (Lst_Member ((Lst)gn->iParents, (ClientData)pgn) != NILLNODE) {
	    Var_Set (IMPSRC, Var_Value(TARGET, gn), pgn);
	}
	switch(gn->made) {
	    case BEINGMADE:
		Error(catgets(catd, MS_MAKE, ERRGRAPH, "Graph cycles through %s\n"), gn->name);
		gn->made = ERROR;
		pgn->make = FALSE;
		break;
	    case MADE:
		pgn->childMade = TRUE;
		Make_TimeStamp(pgn, gn);
		break;
	    case UPTODATE:
		Make_TimeStamp(pgn, gn);
		break;
	    default:
		break;
	}
    }

    return (0);
}
	
/*-
 *-----------------------------------------------------------------------
 * Compat_Run --
 *	Initialize this mode and start making.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Guess what?
 *
 *-----------------------------------------------------------------------
 */
void
Compat_Run(Lst targs)
{
    /* Lst	    	  targs;    List of target nodes to re-create */

    char    	  *cp;	    /* Pointer to string of shell meta-characters */
    GNode   	  *gn;	    /* Current root target */

    COMPATTRACE("Compat_Run: Initialize this mode and start making");

    if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
	signal(SIGINT, CompatInterrupt);
    }
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN) {
	signal(SIGTERM, CompatInterrupt);
    }
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN) {
	signal(SIGHUP, CompatInterrupt);
    }
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN) {
	signal(SIGQUIT, CompatInterrupt);
    }

    for (cp = "#=|^(){};&<>*?[]:$`\\\n"; *cp != '\0'; cp++) {
	meta[*cp] = 1;
    }
    /*
     * The null character serves as a sentinel in the string.
     */
    meta[0] = 1;

    /*
     * For each entry in the list of targets to create, call CompatMake on
     * it to create the thing. CompatMake will leave the 'made' field of gn
     * in one of several states:
     *	    UPTODATE	    gn was already up-to-date
     *	    MADE  	    gn was recreated successfully
     *	    ERROR 	    An error occurred while gn was being created
     *	    ABORTED	    gn was not remade because one of its inferiors
     *	    	  	    could not be made due to errors.
     */
    while (!Lst_IsEmpty (targs)) {
	gn = (GNode *) Lst_DeQueue (targs);
	CompatMake (gn, gn);

	if (gn->made == UPTODATE) {
	    printf (catgets(catd, MS_MAKE, UPTODATE, "`%s' is up to date.\n"), gn->name);
	} else if (gn->made == ABORTED) {
	    printf (catgets(catd, MS_MAKE, NOTREMAKE, "`%s' not remade because of errors.\n"), gn->name);
	}
    }
}
