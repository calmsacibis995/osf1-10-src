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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: proc.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/06/10 17:07:35 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: pchild pnote pwait pjwait dowait pflushall pflush pclrcurr 
 *            palloc padd pads psavejob prestjob pendjob pprint ptprint 
 *            dojobs dofg dofg1 dobg dobg1 dostop dokill pkill pstart 
 *            panystop pfind pgetcurr donotify pfork okpcntl setnice
 *	      donice
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 *
 *	1.23  com/cmd/csh/proc.c, cmdcsh, bos320, 9139320 8/15/91 18:09:07	
 */ 

#include <sys/wait.h>
#include <sys/ioctl.h>
#include "sh.h"
#include "dir.h"
#include "proc.h"

/*
 * Functions that manage processes, handling hanging, termination
 */

#define BIGINDEX	9	/* largest desirable job index */

/*
 * Local static and global variables.
 */

/* +4 here is 1 for '\0', 1 ea for << >& >> */
static uchar_t		command[PMAXLEN+4];
int			cmdlen;
uchar_t			*cmdp;


/*
 * pchild - called at interrupt level by the SIGCHLD signal
 *	indicating that at least one child has terminated or stopped
 *	thus at least one wait system call will definitely return a
 *	childs status.  Top level routines (like pwait) must be sure
 *	to mask interrupts when playing with the proclist data structures!
 */
void 
pchild(int signo)
{
	register struct process *pp;
	register struct process	*fp;
	register pid_t pid;
	union wait sl;
	int jobflags;
	struct rusage ru;
	int woptions;

#ifdef DEBUG
fprintf(stderr,"entering pchild\n"), fflush(stderr);
#endif
	if (!timesdone)                                 /* ??? */
		timesdone++, times(&shtimes);           /* ??? */
loop:
	pid = wait3(&sl, (setintr ? WUNTRACED : 0) | WNOHANG, &ru);
#ifdef DEBUG
fprintf(stderr,"pchild: back from wait3\n"), fflush(stderr);
#endif
	if (pid <= 0) {
		if (errno == EINTR) {
			errno = 0;
			goto loop;
		}
		pnoprocesses = pid == -1;
#ifdef DEBUG
fprintf(stderr,"leaving pchild\n"), fflush(stderr);
#endif
		return;
	}
	for (pp = proclist.p_next; pp != PNULL; pp = pp->p_next)
		if (pid == pp->p_pid)
			goto found;
	goto loop;
found:
#ifdef DEBUG
fprintf(stderr,"child found\n");
#endif
	if (pid == atoi((char *) value((uchar_t *)"child")))
		unsetv((uchar_t *)"child");
	pp->p_flags &= ~(PRUNNING|PSTOPPED|PREPORTED);
	if (WIFSTOPPED(sl)) {
		pp->p_flags |= PSTOPPED;
		pp->p_reason = sl.w_stopsig;
	} else {
		if (pp->p_flags & (PTIME|PPTIME) || adrof((uchar_t *)"time"))
			(void)gettimeofday(&pp->p_etime, (struct timezone *)0);
		pp->p_rusage = ru;
		if (WIFSIGNALED(sl)) {
			if (sl.w_termsig == SIGINT)
				pp->p_flags |= PINTERRUPTED;
			else
				pp->p_flags |= PSIGNALED;
			if (sl.w_coredump)
				pp->p_flags |= PDUMPED;
			pp->p_reason = sl.w_termsig;
		} else {
			pp->p_reason = sl.w_retcode;
			if (pp->p_reason != 0)
				pp->p_flags |= PAEXITED;
			else
				pp->p_flags |= PNEXITED;
		}
	}
	jobflags = 0;
	fp = pp;
	do {
		if ((fp->p_flags & (PPTIME|PRUNNING|PSTOPPED)) == 0 &&
		    !child && adrof((uchar_t *)"time") &&
		    fp->p_rusage.ru_utime.tv_sec+fp->p_rusage.ru_stime.tv_sec >=
		     (unsigned long) atoi((char *)value((uchar_t *)"time")))
			fp->p_flags |= PTIME;
		jobflags |= fp->p_flags;
	} while ((fp = fp->p_friends) != pp);
	pp->p_flags &= ~PFOREGND;
	if (pp == pp->p_friends && (pp->p_flags & PPTIME)) {
		pp->p_flags &= ~PPTIME;
		pp->p_flags |= PTIME;
	}
	if ((jobflags & (PRUNNING|PREPORTED)) == 0) {
		fp = pp;
		do {
			if (fp->p_flags&PSTOPPED)
				fp->p_flags |= PREPORTED;
		} while((fp = fp->p_friends) != pp);
		while(fp->p_pid != fp->p_jobid)
			fp = fp->p_friends;
		if (jobflags&PSTOPPED) {
			if (pcurrent && pcurrent != fp)
				pprevious = pcurrent;
			pcurrent = fp;
		} else
			pclrcurr(fp);
		if (jobflags&PFOREGND) {
			if (jobflags & (PSIGNALED|PSTOPPED|PPTIME) ||
			    !EQ(dcwd->di_name, fp->p_cwd->di_name)) {
				;	/* print in pjwait */
			}
		} else {
			if (jobflags&PNOTIFY || adrof((uchar_t *)"notify")) {
				csh_printf("\n");
				pprint(pp, NUMBER|NAME|REASON);
				if ((jobflags&PSTOPPED) == 0)
					pflush(pp);
			} else {
				fp->p_flags |= PNEEDNOTE;
				neednote++;
			}
		}
	}
	goto loop;
}

void
pnote(void)
{
	register struct process *pp;
	int omask, flags;

	neednote = 0;
	for (pp = proclist.p_next; pp != PNULL; pp = pp->p_next) {
		if (pp->p_flags & PNEEDNOTE) {
			omask = sigblock(sigmask(SIGCHLD));
			pp->p_flags &= ~PNEEDNOTE;
			flags = pprint(pp, NUMBER|NAME|REASON);
			if ((flags&(PRUNNING|PSTOPPED)) == 0)
				pflush(pp);
			(void) sigsetmask(omask);
		}
	}
}

/*
 * pwait - wait for current job to terminate, maintaining integrity
 *	of current and previous job indicators.
 */
void
pwait(void)
{
	register struct process *fp, *pp;
	int omask;

#ifdef DEBUG
fprintf(stderr,"entering pwait\n");
#endif
	/*
	 * Here's where dead procs get flushed.
	 */
	omask = sigblock(sigmask(SIGCHLD));
	for (pp = (fp = &proclist)->p_next; pp != PNULL; pp = (fp = pp)->p_next)
		if (pp->p_pid == 0) {
			fp->p_next = pp->p_next;
			xfree(pp->p_command);
			if (pp->p_cwd && --pp->p_cwd->di_count == 0)
				if (pp->p_cwd->di_next == 0)
					dfree(pp->p_cwd);
			xfree((uchar_t *)pp);
			pp = fp;
		}
	(void)sigsetmask(omask);
	pjwait(pcurrjob);
}

/*
 * pjwait - wait for a job to finish or become stopped
 *	It is assumed to be in the foreground state (PFOREGND)
 */
void
pjwait(register struct process *pp)
{
	register struct process *fp;
	int jobflags, reason;
	int omask;

#ifdef DEBUG
fprintf(stderr,"entering pjwait\n");
#endif
	while (pp->p_pid != pp->p_jobid)
		pp = pp->p_friends;
	fp = pp;
	do {
		if ((fp->p_flags&(PFOREGND|PRUNNING)) == PRUNNING)
			csh_printf(MSGSTR(M_BUG1,
				"BUG: waiting for background job!\n"));
	} while ((fp = fp->p_friends) != pp);
	/*
	 * Now keep pausing as long as we are not interrupted (SIGINT),
	 * and the target process, or any of its friends, are running
	 */
	fp = pp;

	omask = sigblock(sigmask(SIGCHLD));

#ifdef DEBUG
fprintf(stderr,"pjwait: entering wait loop\n");
#endif
	for (;;) {
		jobflags = 0;
		do
			jobflags |= fp->p_flags;
		while((fp = (fp->p_friends)) != pp);
		if ((jobflags & PRUNNING) == 0)
			break;
	 /* Test for child existence and call sigpause to reset
	    signal queue to avoid problems caused by race condition  */

	sigpause(sigblock(0) &~ sigmask(SIGCHLD));
	}
#ifdef DEBUG
fprintf(stderr,"pjwait: after wait loop\n"), fflush(stderr);
#endif
	(void)sigsetmask(omask);
	if (tpgrp > 0)  {
		struct sigvec nsv, osv;
		
		nsv.sv_handler = SIG_IGN;
		nsv.sv_mask = SA_RESTART;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGTTOU, &nsv, &osv);
		IOCTL(FSHTTY, TIOCSPGRP, &tpgrp, "20");
		(void)sigvec(SIGTTOU, &osv, (struct sigvec *)NULL);
	}
        if ((jobflags&(PSIGNALED|PSTOPPED|PTIME)) ||
             !EQ(dcwd->di_name, fp->p_cwd->di_name)) {
                if (jobflags&PSTOPPED)
                        csh_printf("\n");
                (void)pprint(pp, AREASON|SHELLDIR);
        }
	if ((jobflags&(PINTERRUPTED|PSTOPPED)) && setintr &&
	    (!gointr || !EQ(gointr, "-"))) {
		if ((jobflags & PSTOPPED) == 0)
			pflush(pp);
		pintr1(0);
		/*NOTREACHED*/
	}
	reason = 0;
	fp = pp;
	do {
		if (fp->p_reason)
			reason = fp->p_flags & (PSIGNALED|PINTERRUPTED) ?
				fp->p_reason | QUOTE : fp->p_reason;
	} while ((fp = fp->p_friends) != pp);
	set((uchar_t *)"status", putn(reason));
	if (reason && exiterr)
		exitstat();
	pflush(pp);
#ifdef DEBUG
fprintf(stderr,"leaving pjwait\n");
#endif
}

/*
 * dowait - wait for all processes to finish
 */
void
dowait(void)
{
	register struct process *pp;
	int omask;

	pjobs++;
	/* if (setintr)
		sigrelse(SIGINT); */
	omask = sigblock(sigmask(SIGCHLD));
loop:
	for (pp = proclist.p_next; pp; pp = pp->p_next)
		if (pp->p_pid && /* pp->p_pid == pp->p_jobid && */
		    pp->p_flags&PRUNNING) {

			sigpause(0);
			goto loop;
		}
	(void)sigsetmask(omask);
	pjobs = 0;
}

/*
 * pflushall - flush all jobs from list (e.g. at fork())
 */
void
pflushall(void)
{
	register struct process	*pp;

	for (pp = proclist.p_next; pp != PNULL; pp = pp->p_next)
		if (pp->p_pid)
			pflush(pp);
}

/*
 * pflush - flag all process structures in the same job as the
 *	the argument process for deletion.  The actual free of the
 *	space is not done here since pflush is called at interrupt level.
 */
void
pflush(register struct process	*pp)
{
	register struct process *np;
	register int index;

	if (pp->p_pid == 0) {
		csh_printf(MSGSTR(M_BUG2, "BUG: process flushed twice"));
		return;
	}
	while (pp->p_pid != pp->p_jobid)
		pp = pp->p_friends;
	pclrcurr(pp);
	if (pp == pcurrjob)
		pcurrjob = 0;
	index = pp->p_index;
	np = pp;
	do {
		np->p_index = np->p_pid = 0;
		np->p_flags &= ~PNEEDNOTE;
	} while ((np = np->p_friends) != pp);
	if (index == pmaxindex) {
		for (np = proclist.p_next, index = 0; np; np = np->p_next)
			if (np->p_index > index)
				index = np->p_index;
		pmaxindex = index;
	}
}

/*
 * pclrcurr - make sure the given job is not the current or previous job;
 *	pp MUST be the job leader
 */
void
pclrcurr(register struct process *pp)
{

	if (pp == pcurrent)
		if (pprevious != PNULL) {
			pcurrent = pprevious;
			pprevious = pgetcurr(pp);
		} else {
			pcurrent = pgetcurr(pp);
			pprevious = pgetcurr(pp);
		}
	else if (pp == pprevious)
		pprevious = pgetcurr(pp);
}

/*
 * palloc - allocate a process structure and fill it up.
 *	an important assumption is made that the process is running.
 */
void
palloc(pid_t pid, register struct command *t)
{
	register struct process	*pp;
	int i;

	pp = (struct process *)calloc(1, sizeof(struct process));
	pp->p_pid = pid;
	pp->p_flags = t->t_dflg & FAND ? PRUNNING : PRUNNING|PFOREGND;
	if (t->t_dflg & FTIME)
		pp->p_flags |= PPTIME;
	cmdp = command;
	cmdlen = 0;
	padd(t);
	*cmdp++ = 0;
	if (t->t_dflg & FPOU) {
		pp->p_flags |= PPOU;
		if (t->t_dflg & FDIAG)
			pp->p_flags |= PDIAG;
	}
	pp->p_command = savestr(command);
	if (pcurrjob) {
		struct process *fp;
		/* careful here with interrupt level */
		pp->p_cwd = 0;
		pp->p_index = pcurrjob->p_index;
		pp->p_friends = pcurrjob;
		pp->p_jobid = pcurrjob->p_pid;
		for (fp = pcurrjob; fp->p_friends != pcurrjob; fp = fp->p_friends)
			;
		fp->p_friends = pp;
	} else {
		pcurrjob = pp;
		pp->p_jobid = pid;
		pp->p_friends = pp;
		pp->p_cwd = dcwd;
		dcwd->di_count++;
		if (pmaxindex < BIGINDEX)
			pp->p_index = ++pmaxindex;
		else {
			struct process *np;

			for (i = 1; ; i++) {
				for (np = proclist.p_next; np; np = np->p_next)
					if (np->p_index == i)
						goto tryagain;
				pp->p_index = i;
				if (i > pmaxindex)
					pmaxindex = i;
				break;			
			tryagain:;
			}
		}
		if (pcurrent == PNULL)
			pcurrent = pp;
		else if (pprevious == PNULL)
			pprevious = pp;
	}
	pp->p_next = proclist.p_next;
	proclist.p_next = pp;
	(void)gettimeofday(&pp->p_btime,(struct timezone *)0);
}

/*
 * padd() build output line when job is
 * running.
 */
void
padd(register struct command *t)
{
	uchar_t **argp;

	if (t == 0)
		return;
	switch (t->t_dtyp) {

	case TPAR:
		pads("( ");
		padd(t->t_dspr);
		pads(" )");
		break;

	case TCOM:
		for (argp = t->t_dcom; *argp; argp++) {
			if (**argp == ALIASCHR)
				pads((char *)*argp + 1);
			else
				pads((char *)*argp);
			if (argp[1])
				pads(" ");
		}
		break;

	case TFIL:
		padd(t->t_dcar);
		pads(" | ");
		padd(t->t_dcdr);
		return;

	case TLST:
		padd(t->t_dcar);
		pads("; ");
		padd(t->t_dcdr);
		return;
	}
	if ((t->t_dflg & FPIN) == 0 && t->t_dlef) {
		pads((t->t_dflg & FHERE) ? " << " : " < ");
		pads((char *)t->t_dlef);
	}
	if ((t->t_dflg & FPOU) == 0 && t->t_drit) {
		pads((t->t_dflg & FCAT) ? " >>" : " >");
		if (t->t_dflg & FDIAG)
			pads("&");
		pads(" ");
		pads((char *)t->t_drit);
	}
}

void
pads(char *cp)
{
	register int i = strlen(cp);

	if (cmdlen >= PMAXLEN)
		return;
	if (cmdlen + i >= PMAXLEN) {
		strcpy( (char *)cmdp, " ...");
		cmdlen = PMAXLEN;
		cmdp += 4;
		return;
	}
	strcpy( (char *)cmdp, cp);
	cmdp += i;
	cmdlen += i;
}

/*
 * psavejob - temporarily save the current job on a one level stack
 *	so another job can be created.  Used for { } in exp6
 *	and `` in globbing.
 */
void
psavejob()
{

	pholdjob = pcurrjob;
	pcurrjob = PNULL;
}

/*
 * prestjob - opposite of psavejob.  This may be missed if we are interrupted
 *	somewhere, but pendjob cleans up anyway.
 */
void
prestjob()
{

	pcurrjob = pholdjob;
	pholdjob = PNULL;
}

/*
 * pendjob - indicate that a job (set of commands) has been completed
 *	or is about to begin.
 */
void
pendjob()
{
	register struct process *pp, *tp;

	if (pcurrjob && (pcurrjob->p_flags&(PFOREGND|PSTOPPED)) == 0) {
		pp = pcurrjob;
		pholdjob = pcurrjob = 0;
		while (pp->p_pid != pp->p_jobid)
			pp = pp->p_friends;
		csh_printf("[%d]", pp->p_index);
		tp = pp;
		do {
			csh_printf(" %d", pp->p_pid);
			pp = pp->p_friends;
		} while (pp != tp);
		csh_printf("\n");
	} else
		pholdjob = pcurrjob = 0;
}

/*
 * pprint - print a job
 */
int
pprint(register struct process *pp, int flag)
{
	register status, reason;
	struct process *tp;
	extern uchar_t *linp, linbuf[];
	int jobflags, pstatus, k;
	char *format;

	while (pp->p_pid != pp->p_jobid)
		pp = pp->p_friends;
	if (pp == pp->p_friends && (pp->p_flags & PPTIME)) {
		pp->p_flags &= ~PPTIME;
		pp->p_flags |= PTIME;
	}
	tp = pp;
	status = reason = -1; 
	jobflags = 0;
	do {
		jobflags |= pp->p_flags;
		pstatus = pp->p_flags & PALLSTATES;
		if (tp != pp && linp != linbuf && !(flag&FANCY) &&
		    (pstatus == status && pp->p_reason == reason ||
		     !(flag&REASON)))
			display_char(' ');
		else {
			if (tp != pp && linp != linbuf)
				csh_printf("\n");
			if(flag&NUMBER)
				if (pp == tp) {
					csh_printf("[%d]%s %c ", pp->p_index,
					    pp->p_index < 10 ? " " : "",
					    pp==pcurrent ? '+' :
					       (pp == pprevious ? '-' : ' '));
				}
				else {
					for(k=1;k<7;k++)
						display_char(' ');
				}
			if (flag&FANCY)
				csh_printf("%5d ", pp->p_pid);
			if (flag&(REASON|AREASON)) {
				if (flag&NAME)
					format = (char *)"%-21s";
				else
					format = (char *)"%s";
				if (pstatus == status)
					if (pp->p_reason == reason) {
						csh_printf((char *)format, "");
						goto prcomd;
					} else
						reason = pp->p_reason;
				else {
					status = pstatus;
					reason = pp->p_reason;
				}
				switch (status) {

				case PRUNNING:
					csh_printf((char *)format, 
						MSGSTR(M_RUN,"Running "));
					break;

				case PINTERRUPTED:
				case PSTOPPED:
				case PSIGNALED:
					if ((flag&(REASON|AREASON)) 
						&& reason != SIGINT 
						&& reason != SIGPIPE)
					csh_printf((char *) format,
					MSGSTR(mesg[pp->p_reason].mesgno, 
					(char *)mesg[pp->p_reason].pname));
					break;

				case PNEXITED:
				case PAEXITED:
					if (flag & REASON)
						if (pp->p_reason)
							csh_printf(MSGSTR(M_EXITNUM,
							"Exit %-16d"), pp->p_reason);
						else
							csh_printf((char *)format, 
							MSGSTR(M_DONE,"Done"));
					break;

				default:
					csh_printf(MSGSTR(M_BUG3,
					       "BUG: status=%-9o"), status);
				}
			}
		}
prcomd:
		if (flag&NAME) {
			csh_printf("%s", pp->p_command);
			if (pp->p_flags & PPOU) {
				display_char(' ');
				display_char('|');
			}
			if (pp->p_flags & PDIAG)
				display_char('&');
		}
		if (flag&(REASON|AREASON) && pp->p_flags&PDUMPED)
			csh_printf(MSGSTR(M_CORE," (core dumped)"));
		if (tp == pp->p_friends) {
			if (flag&AMPERSAND) {
				display_char(' ');
				display_char('&');
			}
			if (flag&JOBDIR &&
			    !EQ(tp->p_cwd->di_name, dcwd->di_name)) {
				csh_printf(" (wd: ");
				dtildepr(value((uchar_t *)"home"),
						tp->p_cwd->di_name);
				display_char(')');
			}
		}
		if (pp->p_flags&PPTIME && !(status&(PSTOPPED|PRUNNING))) {
			if (linp != linbuf)
				csh_printf("\n\t");
			{   static struct rusage zru;
			    prusage(&zru, &pp->p_rusage, &pp->p_etime,
				&pp->p_btime);
			}
		}
		if (tp == pp->p_friends) {
			if (linp != linbuf)
				csh_printf("\n");
			if (flag&SHELLDIR && !EQ(tp->p_cwd->di_name, dcwd->di_name)) {
				csh_printf(MSGSTR(M_WD, "(wd now: "));
				dtildepr(value((uchar_t *)"home"),dcwd->di_name);
				csh_printf(")\n");
			}
		}
	} while ((pp = pp->p_friends) != tp);
	if (jobflags&PTIME && (jobflags&(PSTOPPED|PRUNNING)) == 0) {
		if (jobflags & NUMBER) {
			for(k=1;k<7;k++)
				display_char(' ');
		}
		ptprint(tp);
	}
	return (jobflags);
}

void
ptprint(register struct process *tp)
{
	struct timeval tetime, diff;
	static struct timeval ztime;
	static struct rusage zru;
	struct rusage ru;
	register struct process *pp = tp;

	ru = zru;
	tetime = ztime;
	do {
		ruadd(&ru, &pp->p_rusage);
		tvsub(&diff, &pp->p_etime, &pp->p_btime);
		if (timercmp(&diff, &tetime, >))
			tetime = diff;
	} while ((pp = pp->p_friends) != tp);
	prusage(&zru, &ru, &tetime, &ztime);
}

/*
 * dojobs - print all jobs
 */
void
dojobs(uchar_t **v)
{
	register struct process *pp;
	register int flag = NUMBER|NAME|REASON;
	int i;

	if (chkstop)
		chkstop = 2;
	if (*++v) {
		if (v[1] || !EQ(*v, "-l"))
			error(MSGSTR(M_JOBS, "Usage: jobs [ -l ]"));
		flag |= FANCY|JOBDIR;
	}
	for (i = 1; i <= pmaxindex; i++)
		for (pp = proclist.p_next; pp; pp = pp->p_next)
			if (pp->p_index == i && pp->p_pid == pp->p_jobid) {
				pp->p_flags &= ~PNEEDNOTE;
				if (!(pprint(pp, flag) & (PRUNNING|PSTOPPED)))
					pflush(pp);
				break;
			}
}

/*
 * dofg - builtin - put the job into the foreground
 */
void
dofg(uchar_t **v)
{
	register struct process *pp;
	int omask;

	okpcntl();
	++v;
	do {
		pp = pfind(*v);
		pstart(pp, 1);
		if (setintr)
			omask = sigblock(sigmask(SIGINT));
		pjwait(pp);
	} while (*v && *++v);
	if (setintr)
		(void)sigsetmask(omask);
}

/*
 * %... - builtin - put the job into the foreground
 */
void
dofg1(uchar_t **v)
{
	register struct process *pp;
	int omask;

	okpcntl();
	pp = pfind(v[0]);
	pstart(pp, 1);
	if (setintr)
		omask = sigblock(sigmask(SIGINT));
	pjwait(pp);
	if (setintr)
		(void)sigsetmask(omask);
}

/*
 * dobg - builtin - put the job into the background
 */
void
dobg(uchar_t **v)
{
	register struct process *pp;

	okpcntl();
	++v;
	do {
		pp = pfind(*v);
		pstart(pp, 0);
	} while (*v && *++v);
}

/*
 * %... & - builtin - put the job into the background
 */
void
dobg1(uchar_t **v)
{
	register struct process *pp;

	pp = pfind(v[0]);
	pstart(pp, 0);
}

/*
 * dostop - builtin - stop the job
 */
void
dostop(uchar_t **v)
{
	pkill(++v, SIGSTOP);
}

/*
 * dokill - builtin - superset of kill (1)
 */
void
dokill(uchar_t **v)
{
	register int signum;
	register uchar_t *name;

	v++;
	if (v[0] && v[0][0] == '-') {
		if (v[0][1] == 'l') {
			for (signum = 1; signum < NSIG; signum++) {
				if (name = mesg[signum].iname)
					csh_printf("%s ", name);
				if ((signum % 16) == 0)
					csh_printf("\n");
			}
			csh_printf("\n");
			return;
		}
		if (digit(v[0][1])) {
			signum = atoi((char *)v[0]+1);
			if (signum < 1 || signum >= NSIG)
				bferr(MSGSTR(M_BADSIG, "Bad signal number"));
		} else {
			name = &v[0][1];
			for (signum = 1; signum < NSIG; signum++) {
			if (mesg[signum].iname && EQ(name, mesg[signum].iname))
				goto gotsig;
			}
			setname(name);
			bferr(MSGSTR(M_NOSIG,
				"Unknown signal; kill -l lists signals"));
		}
gotsig:
		v++;
	} else
		signum = SIGTERM;
	pkill(v, signum);
}

void
pkill(uchar_t **v, int signum)
{
	register struct process *pp, *np;
	register int jobflags = 0;
	pid_t pid;
	int err = 0;
	int omask;
	extern char *sys_errlist[];

	omask = sigmask(SIGCHLD);
	if (setintr)
		/* sighold(SIGINT); */
		omask |= sigmask(SIGINT);
	omask = sigblock(omask) & ~omask;
	while (*v) {
		if ( **v == '`' ) {
			gflag = 0;
			rscan(v, tglob);
			if (gflag)
				v = glob(v);
		}
		if (**v == '%') {
			np = pp = pfind(*v);
			do
				jobflags |= np->p_flags;
			while ((np = np->p_friends) != pp);
			switch (signum) {

			case SIGSTOP:
			case SIGTSTP:
			case SIGTTIN:
			case SIGTTOU:
				if ((jobflags & PRUNNING) == 0) {
					csh_printf(MSGSTR(M_STOPD,
						"%s: Already stopped\n"), *v);
					err++;
					goto cont;
				}
			}
			if (killpg(pp->p_jobid, signum) < 0)
			{
				csh_printf("%s: %s\n", *v, strerror(errno));
				err++;
			}
			if (signum == SIGTERM || signum == SIGHUP)
				(void) killpg(pp->p_jobid, SIGCONT);
		} else if (!(digit(**v) || **v == '-'))
			bferr(MSGSTR(M_ID,
				"Arguments should be jobs or process id's"));
		else {
			pid = atoi((char *)*v);
			if (kill(pid, signum) < 0) {
				csh_printf("%d: %s\n", pid, strerror(errno));
				err++;
				goto cont;
			}
			if (signum == SIGTERM || signum == SIGHUP)
				kill(pid, SIGCONT);
		}
cont:
		v++;
	}
	(void)sigsetmask(omask);
	if (err)
		error((char *)NOSTR);
}

/*
 * pstart - start the job in foreground/background
 */
void
pstart(register struct process *pp, int foregnd)
{
	register struct process *np;
	int jobflags = 0;
	int omask;

	 omask = sigblock(sigmask(SIGCHLD));
	np = pp;
	do {
		jobflags |= np->p_flags;
		if (np->p_flags&(PRUNNING|PSTOPPED)) {
			np->p_flags |= PRUNNING;
			np->p_flags &= ~PSTOPPED;
			if (foregnd)
				np->p_flags |= PFOREGND;
			else
				np->p_flags &= ~PFOREGND;
		}
	} while((np = np->p_friends) != pp);
	if (!foregnd)
		pclrcurr(pp);
	pprint(pp, foregnd ? NAME|JOBDIR : NUMBER|NAME|AMPERSAND);
	if (foregnd)  {
		struct sigvec nsv, osv;
		
		nsv.sv_handler = SIG_IGN;
		nsv.sv_mask = SA_RESTART;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGTTOU, &nsv, &osv);
		IOCTL(FSHTTY, TIOCSPGRP, &pp->p_jobid, "18");
		(void)sigvec(SIGTTOU, &osv, (struct sigvec *)NULL);
	}
        /*
         * 1. child process of csh (shell script) receives SIGTTIN/SIGTTOU
         * 2. parent process (csh) receives SIGCHLD
         * 3. The "csh" signal handling function pchild() is invoked
         *    with a SIGCHLD signal.
         * 4. pchild() calls wait3(WNOHANG) which returns 0.
         *    The child process is NOT ready to be waited for at this time.
         *    pchild() returns without picking-up the correct status
         *    for the child process which generated the SIGCHILD.
         * 5. CONSEQUENCE : csh is UNaware that the process is stopped
         * 6. THIS LINE HAS BEEN COMMENTED OUT : if (jobflags&PSTOPPED)
         *
         *         beto  - aug/03/91 - defect 31266 - apar 21418
         */
	killpg(pp->p_jobid, SIGCONT);
	(void)sigsetmask(omask);
}

void
panystop(int neednl)
{
	register struct process *pp;

	chkstop = 2;
	for (pp = proclist.p_next; pp; pp = pp->p_next)
		if (pp->p_flags & PSTOPPED)
			error(MSGSTR(M_SJOBS, "\nThere are stopped jobs") + 1 - neednl);
}

struct process *
pfind(uchar_t *cp)
{
	register struct process *pp, *np;

	if (cp == 0 || cp[1] == 0 || EQ(cp, "%%") || EQ(cp, "%+")) {
		if (pcurrent == PNULL)
			bferr(MSGSTR(M_CURJOB, "No current job"));
		return (pcurrent);
	}
	if (EQ(cp, "%-") || EQ(cp, "%#")) {
		if (pprevious == PNULL)
			bferr(MSGSTR(M_PREVJOB, "No previous job"));
		return (pprevious);
	}
	if (digit(cp[1])) {
		int index = atoi((char *)cp+1);
		for (pp = proclist.p_next; pp; pp = pp->p_next)
			if (pp->p_index == index && pp->p_pid == pp->p_jobid)
				return (pp);
		bferr(MSGSTR(M_SUCHJOB, "No such job"));
	}
	np = PNULL;
	for (pp = proclist.p_next; pp; pp = pp->p_next)
		if (pp->p_pid == pp->p_jobid) {
			if (cp[1] == '?') {
				register uchar_t *dp;
				for (dp = pp->p_command; *dp; dp++) {
					if (*dp != cp[2])
						continue;
					if (prefix(cp+2, dp))
						goto match;
				}
			} else if (prefix(cp+1, pp->p_command)) {
match:
				if (np)
					bferr(MSGSTR(M_AMBIG, "Ambiguous"));
				np = pp;
			}
		}
	if (np)
		return (np);
	if (cp[1] == '?')
		bferr(MSGSTR(M_NOJOB, "No job matches pattern"));
	else
		bferr(MSGSTR(M_SUCHJOB, "No such job"));
}

/*
 * pgetcurr - find most recent job that is not pp, preferably stopped
 */
struct process *
pgetcurr(register struct process *pp)
{
	register struct process *np;
	register struct process *xp = PNULL;

	for (np = proclist.p_next; np; np = np->p_next)
		if (np != pcurrent && np != pp && np->p_pid &&
		    np->p_pid == np->p_jobid) {
			if (np->p_flags & PSTOPPED)
				return (np);
			if (xp == PNULL)
				xp = np;
		}
	return (xp);
}

/*
 * donotify - flag the job so as to report termination asynchronously
 */
void
donotify(uchar_t **v)
{
	register struct process *pp;

	pp = pfind(*++v);
	pp->p_flags |= PNOTIFY;
}

/*
 *
 * Do the fork and whatever should be done in the child side that
 * should not be done if we are not forking at all (like for simple builtin's)
 * Also do everything that needs any signals fiddled with in the parent side
 *
 *
 * Wanttty tells whether process and/or tty pgrps are to be manipulated:
 *      -1:     leave tty alone; inherit pgrp from parent
 *       0:     already have tty; manipulate process pgrps only
 *       1:     want to claim tty; manipulate process and tty pgrps
 *
 * It is usually just the value of tpgrp.
 */
pid_t
pfork(struct command *t, int wanttty)
{
	pid_t pid;
	bool ignint = 0;
	pid_t pgrp;
	int omask;
	int pipeflag = 0;  /* command is part of a pipe */
	static int pipe_pgrp;
	struct sigvec osv, nsv;

	/*
	 * A child will be uninterruptible only under very special
	 * conditions. Remember that the semantics of '&' is
	 * implemented by disconnecting the process from the tty so
	 * signals do not need to ignored just for '&'.
	 * Thus signals are set to default action for children unless:
	 *	we have had an "onintr -" (then specifically ignored)
	 *	we are not playing with signals (inherit action)
	 */
	if (t->t_dflg&FPIN || t->t_dflg&FPOU)	/* in||out is pipe */
		pipeflag = 1;
	else
		pipe_pgrp = 0;
	if (setintr)
		ignint = (tpgrp == -1 && (t->t_dflg&FINT))
		    || (gointr && EQ(gointr, "-"));
	/*
	 * Hold SIGCHLD until we have the process installed in our table.
	 */
	omask = sigblock(sigmask(SIGCHLD));
	while ((pid = fork()) < 0)
		if (setintr == 0) {
			sleep(FORKSLEEP);
		}
		else {
			(void)sigsetmask(omask);
			error(MSGSTR(M_NOPROCS,"No more processes"));
		}
	if (pid == 0) {
		settimes();
		pgrp = pcurrjob ? pcurrjob->p_jobid : getpid();
		pflushall();
		pcurrjob = PNULL;
		timesdone = 0;
		child++;
		(void)sigsetmask(0);
		if (setintr) {
			setintr = 0;		/* until I think otherwise */
			/*
			 * Children just get blown away on SIGINT, SIGQUIT
			 * unless "onintr -" seen.
			 */
			nsv.sv_handler = ignint ? SIG_IGN : SIG_DFL;
			nsv.sv_mask = SA_RESTART;
			nsv.sv_onstack = 0;
			(void)sigvec(SIGINT, &nsv, &osv);
			(void)sigvec(SIGQUIT, &nsv, &osv);
			if (wanttty >= 0) {
				nsv.sv_handler = SIG_DFL;
				/* make stoppable */
				(void)sigvec(SIGTSTP, &nsv, &osv);
				(void)sigvec(SIGTTIN, &nsv, &osv);
				(void)sigvec(SIGTTOU, &nsv, &osv);
			}
			nsv.sv_handler = parterm;
			(void)sigvec(SIGTERM, &nsv, &osv);
		} else if (tpgrp == -1 && (t->t_dflg&FINT)) {
			nsv.sv_handler = SIG_IGN;
			nsv.sv_mask = 0;
			nsv.sv_onstack = 0;
			(void)sigvec(SIGINT, &nsv, &osv);
			(void)sigvec(SIGQUIT, &nsv, &osv);
		}
		if (wanttty >= 0 && tpgrp >= 0)
		{
			if (setpgrp(0,pgrp) < 0 && pgrp != pid)
				pgrp = pid;
			setpgrp(0, pgrp);
		}
		if (tpgrp > 0)
			tpgrp = 0;		/* gave tty away */

/******************************************************************/
/*  Implementation note:
	The signal SIGTTOU has to be ignored here because the
	POSIX implementation of the tcsetpgrp ioctl will send
	SIGTTOU to any background process that calls it
	before implementing the Terminal Process Group change
*/
/******************************************************************/
		if (wanttty > 0)  {
#ifdef DEBUG
fprintf(stderr,"pfork: about to icotl(TIOCSPGRP)\n"), fflush(stderr);
#endif
			nsv.sv_handler = SIG_IGN;
			nsv.sv_mask = SA_RESTART;
			nsv.sv_onstack = 0;
			(void)sigvec(SIGTTOU, &nsv, &osv);
			IOCTL(FSHTTY, TIOCSPGRP, &pgrp, "19");
			(void)sigvec(SIGTTOU, &osv,(struct sigvec *)NULL);
		}
		/*
		 * Nohup and nice apply only to TCOM's but it would be
		 * nice (?!?) if you could say "nohup (foo;bar)"
		 * Then the parser would have to know about nice/nohup/time
		 */
		if (t->t_dflg & FNOHUP)  {
#ifdef DEBUG
fprintf(stderr,"pfork: about to nohup and nice\n"), fflush(stderr);
#endif
			nsv.sv_handler = SIG_IGN;
			nsv.sv_mask = 0;
			nsv.sv_onstack = 0;
			(void)sigvec(SIGHUP, &nsv, (struct sigvec*)NULL);
		}
		if (t->t_dflg & FNICE) {
			nice(t->t_nice);
		}

	} else {
	/*********************************************************/
	/* Implementation note:
	   There is a race condition in pipes where the leftmost
	   child's pgrp isn't set before the other siblings do their
	   setpgrp. 
	*/
	/*********************************************************/
		if (pipeflag && !pipe_pgrp)
			pipe_pgrp = pid;
		if (wanttty >= 0 && tpgrp >= 0) {
			if(setpgrp(pid,pipe_pgrp) < 0 && pid != pipe_pgrp) {
				pipe_pgrp = pid;
				setpgrp(pid,pid);
			}
		}
		palloc(pid, t);
		(void)sigsetmask(omask);
	}

	return (pid);
}

void
okpcntl(void)
{

	if (tpgrp == -1)
		error(MSGSTR(M_NOJCNTL, "No job control in this shell"));
	if (tpgrp == 0)
		error(MSGSTR(M_NOSJCNTL, "No job control in subshells"));
}

int
siggetmask(void)
{
	sigset_t set;

	sigprocmask(0,(sigset_t *)NULL,&set);
	return((int)set);	/* XXX UGLY int/sigset_t dependency */
}

int
siggetflags(int sig)
{
	static struct sigaction sa;

	sigaction(sig,(struct sigaction *)0,&sa);
	return(sa.sa_flags);
}

/*
 * donice is only called when it on the line by itself or with a +- value
 */
void
donice(register uchar_t **v)
{
	register uchar_t *cp;

	v++;
	cp = *v++;
	if (cp == 0) {
		nice(NICE);
	} else if (*v == 0 && strchr("+-",cp[0])) {
		nice(getn(cp));
	}
}
