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
static char	*sccsid = "@(#)$RCSfile: killall.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:12:40 $";
#endif 
/*
 */

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Kill all processes except kprocs, 0, 1, this one, and its ancestors.
 *		killall 	sends SIGKILL
 *		killall -l	list the signals
 *		killall [-]n	sends signal n
 *		killall -	sends SIGTERM, then SIGKILL to all killable
 *				processes that are still alive after a delay.
 *		killall - [-]n	as above, but signal n instead of SIGTERM
 */                                                                   

#include <sys/version.h>	/* this is what we really want */
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

#include <stdio.h>
#include <sys/table.h>
#include <sys/signal.h>

#if SEC_ARCH
#include <sys/proc.h>  /* for SSYS value to avoid terminating daemons */
#endif

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "killall_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_KILLALL,n,s)
#ifdef SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_KILLALL_SEC,n,s)
#endif
#else
#define MSGSTR(n,s) s
#endif

extern	char	*calloc();
extern	char	*realloc();

typedef	struct proc_info	*proc_info_t;
struct	proc_info {
	pid_t			pid;
	pid_t			pgrp;
};

proc_info_t			proc_table = (proc_info_t)0;
int				max_proc_table, uid, ppid, pgrp, nprocs;


/*
 *	Translate thread state to a number in an ordered scale.
 *	When collapsing all the threads' states to one for the
 *	entire task, the lower-numbered state dominates.
 */


char *signm[] = { 0,
"HUP", "INT", "QUIT", "ILL", "TRAP", "IOT", "EMT", "FPE",	/* 1-8 */
"KILL", "BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", "URG",	/* 9-16 */
"STOP", "TSTP", "CONT", "CHLD", "TTIN", "TTOU", "IO", "XCPU",	/* 17-24 */
"XFSZ", "VTALRM", "PROF", "WINCH", 0, "USR1", "USR2", 0,	/* 25-31 */
};


#define KPROC               5      /* Kernel processes */

/* Intervals between testing for all processes killed */
char sleepsched[] = {1, 2, 2, 5, 10, 10};

main(argc, argv)
char *argv[];
{
	register long i, j;
	register pid_t pid;
	long nleft;
	register int signo = SIGKILL;
	int clobber = 0;
	int found = 0;
		/* clobber != 0 to send SIGKILL to signallable processes
		 * that survive the sleep schedule.
		 */

			/* Minus and then a number says what signal */
			/* to send; whereas a minus alone says to   */
			/* follow with SIGTERM if the first signal  */
			/* fails.                                   */


#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif /* SEC_BASE */

	if (argc > 1) { 

	if (*argv[1] == '-') {

		if (argv[1][1] == 'l') {
			for (signo = 0; signo <= NSIG; signo++) {
				if (signm[signo])
					printf("%s ", signm[signo]);
				if (signo == 16)
					printf("\n");
			}
			printf("\n");
			exit(0);
			}
			if(argv[1][1] =='\n' || argv[1][1] == '\0') {
			++clobber;
			signo = SIGTERM;
			}
				
			else if (isdigit(argv[1][1])){
			signo = atoi(argv[1]+1);
				if (signo < 0 || signo > NSIG) {
				printf(MSGSTR(OUT_OF_RANGE, "killall: %s: number out of range\n"),
				    argv[1]);
				exit(1);
				}
			} 
			else {
			char *name = argv[1]+1;
			for (signo = 0; signo <= NSIG; signo++)
			if (signm[signo] && !strcmp(signm[signo], name))
				{ 
				  found = 1; 
				  break; 
				}
			if (found == 0) {
			printf(MSGSTR(LIST, "killall: %s: unknown signal; killall -l lists signals\n"), name);
			exit(1);
			}
			}

		}

	}

#ifndef SEC_BASE
    uid = getuid();
#endif

/*
* Our own process group (that includes 
* the current process) and our parent.
*/

    pgrp = getpgrp();
    ppid = getppid();

    max_proc_table = 200;
    proc_table = (proc_info_t)calloc((unsigned)max_proc_table,
    				     sizeof(struct proc_info));

#if SEC_BASE
    if (!authorized_user("sysadmin")) {
	fprintf(stderr, MSGSTR(NEED_AUTH, "%s: need sysadmin authorization\n"), command_name);
	exit(1);
    }
    if (forceprivs(privvec(SEC_KILL, SEC_DEBUG,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
				-1), (priv_t *) 0)) {
	fprintf(stderr, MSGSTR(INSUFF_PRIVS, "%s: insufficient privileges\n"), command_name);
	exit(1);
    }
    disablepriv(SEC_SUSPEND_AUDIT);
#endif

    get_proc_table();


/*
 * Try to signal the unmarked processes. First save parent and group
 * process id
 */

    for (i = 0; i < nprocs; i++)
		if (pid = proc_table[i].pid)
		{

				if( ((proc_table[i].pid - KPROC) <= 0) ||
			/*
			 * Ignore members of our own process group (that
			 * includes the current process) and our parent.
			 */
					  proc_table[i].pid == ppid ||
					  proc_table[i].pgrp == pgrp ||
				          kill(pid,signo) != 0) {
				          	proc_table[i].pid = 0;  /* Mark it not 2B killed */
						}
		}

	if (clobber == 0)			     /* Didn't specify '-' */
		exit(0);

	for (j = 0; j < sizeof sleepsched; ++j)
	{       /* See if anyone killable is left */
		for (nleft = i = 0; i < nprocs; i++)
		{	
			pid = proc_table[i].pid;
			if (pid != 0 && kill(pid, 0) == 0)
				++nleft;
			else
				proc_table[i].pid = 0;
		}
		if (nleft == 0)
			exit(0);

/* Remove this after sleep definition is out of included include file */

#undef sleep
		sleep( (unsigned) sleepsched[j] );
	}

	for (i=0; i < nprocs; i++) {

	       if (pid = proc_table[i].pid) {
			kill(pid,SIGKILL);
			}
		}

	exit(0);
}

/* All of this should come out of the process manager... */

get_proc_table()
{
    register int i,j;
    long	nproc;
#define    NPROC    16
    struct tbl_procinfo proc[NPROC];
    struct tbl_procinfo *mproc;

    nproc = table(TBL_PROCINFO, 0, (char *)0, 32767, 0);

    for (i=0; i < nproc; i += NPROC) {
	j = table(TBL_PROCINFO, i, (char *)proc, NPROC, sizeof(proc[0]));
        for (j = j - 1; j >= 0; j--) {

            mproc = &proc[j];
	    if ((mproc->pi_status == PI_EMPTY) || (mproc->pi_pid == 0))
	        continue;
#if SEC_ARCH
	    /* Ignore security policy daemons */

	    if (mproc->pi_flag & SSYS)
		continue;
#endif
            save(mproc);
	}
    }
#undef	NPROC
}


save(mproc)
    struct tbl_procinfo *mproc;
{
    proc_info_t pi;

    nprocs++;

    if (nprocs > max_proc_table) {
	max_proc_table *= 2;
	proc_table = (proc_info_t)realloc((char *)proc_table,
				(unsigned)max_proc_table*sizeof(*proc_table));
    }

    pi = &proc_table[nprocs-1];

#ifdef SEC_BASE

    pi->pid	= mproc->pi_pid;
    pi->pgrp	= mproc->pi_pgrp;

#else

    if(!uid) {
    		pi->pid	= mproc->pi_pid;
    		pi->pgrp	= mproc->pi_pgrp;
    	}
    else
	if(uid == mproc->pi_uid && ppid == mproc->pi_ppid ) {
    		pi->pid	 = mproc->pi_pid;
    		pi->pgrp = mproc->pi_pgrp;
	}
#endif SEC_BASE
}
