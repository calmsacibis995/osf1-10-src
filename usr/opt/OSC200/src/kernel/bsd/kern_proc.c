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
static char *rcsid = "@(#)$RCSfile: kern_proc.c,v $ $Revision: 4.3.11.3 $ (DEC) $Date: 1993/08/13 14:26:57 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <mach_kdb.h>
#include <machine/reg.h>
#if	!defined(ibmrt) && !defined(mips)
#include <machine/psl.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <ufs/quota.h>
#include <sys/acct.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/uio.h>

pid_t	pidhash[PIDHSZ];
struct	proc *proc, *procNPROC;	/* the proc table itself */
struct	proc *freeproc, *zombproc, *allproc;
			/* lists of procs in various states */
int	nproc;
struct	prochd qs[NQS];
int	mpid;			/* generic for unique process id's */

pid_t                   PID_RSRVD = PID_MAX+1;

/*
 * unhash pid
 */

pidunhash(register struct proc *p)
{
	register int i, x;

	i = PIDHASH(p->p_pid);
	x = p - proc;
	if (pidhash[i] == x) {
		pidhash[i] = p->p_idhash;
		return;
	}
	else {
		for (i = pidhash[i]; i != 0; i = proc[i].p_idhash)
			if (proc[i].p_idhash == x) {
				proc[i].p_idhash = p->p_idhash;
				return;
			}
		panic("pidunhash");
	}
}

/*
 * Clear any pending stops for top and all descendents.
 */
spgrp(top)
	struct proc *top;
{
	register struct proc *p;
	int f = 0;

	p = top;
	for (;;) {
		p->p_sig &=
			  ~(sigmask(SIGTSTP)|sigmask(SIGTTIN)|sigmask(SIGTTOU));
		f++;
		/*
		 * If this process has children, descend to them next,
		 * otherwise do any siblings, and if done with this level,
		 * follow back up the tree (but not past top).
		 */
		if (p->p_cptr)
			p = p->p_cptr;
		else if (p == top)
			return (f);
		else if (p->p_osptr)
			p = p->p_osptr;
		else for (;;) {
			p = p->p_pptr;
			if (p == top)
				return (f);
			if (p->p_osptr) {
				p = p->p_osptr;
				break;
			}
		}
	}
}

/*
 * Is p an inferior of the current process?
 */
inferior(p)
	register struct proc *p;
{

	for (; p != u.u_procp; p = p->p_pptr)
		if (p->p_ppid == 0)
			return (0);
	return (1);
}

struct proc *
pfind(pid)
	long pid;
{
	register struct proc *p;

	for (p = &proc[pidhash[PIDHASH(pid)]]; p != &proc[0]; p = &proc[p->p_idhash])
		if (p->p_pid == pid)
			return (p);
	return ((struct proc *)0);
}

/*
 * Locate a process group by number
 */
struct pgrp *
pgfind(pgid)
	register pid_t pgid;
{
	register struct pgrp *pgrp = pgrphash[PIDHASH(pgid)];

	for (; pgrp; pgrp = pgrp->pg_hforw)
		if (pgrp->pg_id == pgid)
			return(pgrp);
	return ((struct pgrp *)0);
}

/*
 * Move p to a new or existing process group (and session)
 */
pgmv(p, pgid, mksess)
	register struct proc *p;
	pid_t pgid;
	int mksess;
{
	register struct pgrp *pgrp = pgfind(pgid);
	register struct proc **pp = &p->p_pgrp->pg_mem;
	register struct proc *cp;
	struct pgrp *opgrp;
	register n;

	if (pgrp && mksess)	/* firewalls */
		panic("pgmv: setsid into non-empty pgrp");
	if (SESS_LEADER(p))
		panic("pgmv: session leader attempted setpgrp");
#ifdef	COMPAT_43
	/*
	 * If moving to or from pgrp 0 and process is currently
	 * in a pgrp of which it is the sole member, reuse
	 * that pgrp structure in order to preserve session
	 * and tty pgrp.  If moving to a non-zero pgrp, the
	 * new pgrp must not yet exist.
	 */
	opgrp = p->p_pgrp;
	if (!mksess && (pgid == 0 || (pgrp == NULL && opgrp->pg_id == 0)) &&
	    opgrp->pg_mem == p && p->p_pgrpnxt == NULL) {
		register struct pgrp **pgp;

		/* Put pgrp onto proper hash chain */
		pgp = &pgrphash[PIDHASH(opgrp->pg_id)];
		for (; *pgp; pgp = &(*pgp)->pg_hforw)
			if (*pgp == opgrp) {
				*pgp = opgrp->pg_hforw;
				goto found;
			}
		panic("pgmv: can't find pgrp on hash chain");
	found:
		opgrp->pg_id = pgid;
		opgrp->pg_hforw = pgrphash[n=PIDHASH(pgid)];
		pgrphash[n] = opgrp;
		return;
	}
#endif
	if (pgrp == NULL) {
		/*
		 * new process group
		 */
		if (p->p_pid != pgid)
			panic("pgmv: new pgrp and pid != pgid");
		pgrp = (struct pgrp *)kalloc(sizeof(struct pgrp));
		if (mksess) {
			register struct session *sess;
			/*
			 * new session
			 */
			sess = (struct session *)kalloc(sizeof(struct session));
			ASSERT(sess != NULL);
			sess->s_leader = p;
		 	sess->s_id = p->p_pid;
		 	sess->s_count = 1;
		 	sess->s_ttyvp = (struct vnode *)NULL;
	 		sess->s_pgrps = pgrp;
	 		sess->s_fpgrpp = (struct pgrp **)NULL;
			SESS_LOCK_INIT(sess);
			SESS_FPGRP_LOCK_INIT(sess);
                        PROC_LOCK(p);
			p->p_flag &= ~SCTTY;
			PROC_UNLOCK(p);
			pgrp->pg_session = sess;
			pgrp->pg_sessnxt = NULL;
			if (p != u.u_procp)
				panic("pgmv: mksession and p != u.u_procp");
		} else {
			struct session *psess = p->p_session;

			pgrp->pg_session = psess;
			SESS_LOCK(psess);

			/* Add pgrp to session and increment the pgrp count */
			pgrp->pg_sessnxt = psess->s_pgrps;
			psess->s_pgrps = pgrp;
			psess->s_count++;
			SESS_UNLOCK(psess);
		}
		pgrp->pg_id = pgid;
		pgrp->pg_hforw = pgrphash[n=PIDHASH(pgid)];
		pgrphash[n] = pgrp;
		pgrp->pg_jobc = 0;
		pgrp->pg_mem = NULL;
	}
	/*
	 * adjust eligibility of affected pgrps to participate in job control
	 */
	if (PGRP_JOBC(p))
		p->p_pgrp->pg_jobc--;
	for (cp = p->p_cptr; cp; cp = cp->p_osptr)
		if (PGRP_JOBC(cp))
			cp->p_pgrp->pg_jobc--;
	/*
	 * unlink p from old process group
	 */
	for (; *pp; pp = &(*pp)->p_pgrpnxt)
		if (*pp == p) {
			*pp = p->p_pgrpnxt;
			goto done;
		}
	panic("pgmv: can't find p on old pgrp");
done:
	/*
	 * link into new one
	 */
	p->p_pgrpnxt = pgrp->pg_mem;
	pgrp->pg_mem = p;
	opgrp = p->p_pgrp;
	p->p_pgrp = pgrp;
	/*
	 * adjust eligibility of affected pgrps to participate in job control
	 */
	if (PGRP_JOBC(p))
		p->p_pgrp->pg_jobc++;
	for (cp = p->p_cptr; cp; cp = cp->p_osptr)
		if (PGRP_JOBC(cp))
			cp->p_pgrp->pg_jobc++;
	/*
	 * old pgrp empty?
	 */
	if (!opgrp->pg_mem)
		pgdelete(opgrp);
}

/*
 * remove process from process group
 */
pgrm(p)
	register struct proc *p;
{
	register struct proc **pp = &p->p_pgrp->pg_mem;

	for (; *pp; pp = &(*pp)->p_pgrpnxt)
		if (*pp == p) {
			*pp = p->p_pgrpnxt;
			goto done;
		}
	panic("pgrm: can't find p in pgrp");
done:
	if (!p->p_pgrp->pg_mem)
		pgdelete(p->p_pgrp);
	p->p_pgrp = 0;
}

/*
 * delete a process group
 */
pgdelete(pgrp)
	register struct pgrp *pgrp;
{
        register struct pgrp            **pgp, *spgrp, *opgrp;
                 struct session         *session;
                 struct vnode           *vp;
                 int                    retval, error;
                 int                    *retvalp = &retval;
                 int                    s;

        pgp = &pgrphash[PIDHASH(pgrp->pg_id)];
	for (; *pgp; pgp = &(*pgp)->pg_hforw)
		if (*pgp == pgrp) {
			*pgp = pgrp->pg_hforw;
			goto done;
		}
	panic("pgdelete: can't find pgrp on hash chain");
done:
	session = pgrp->pg_session;
	SESS_LOCK(session);
	s = splhigh();
	SESS_FPGRP_LOCK(session);
	/*
	 * If removing foreground pgrp, remove it from terminal also.
	 * tty code should check for non-NULL pgrp.
	 */
	if (session->s_fpgrpp && (*session->s_fpgrpp == pgrp)) {
		*session->s_fpgrpp = (struct pgrp *)NULL;
	}
	SESS_FPGRP_UNLOCK(session);
	splx(s);

        /*
         * Remove the pgrp from the session's pgrp list.
         */
        spgrp = session->s_pgrps;

        /* Special case: pgrp is first in list. */
        if (spgrp == pgrp)
            session->s_pgrps = pgrp->pg_sessnxt;
        else {
            while (spgrp != NULL) {
                if (spgrp->pg_sessnxt == pgrp) {
                    spgrp->pg_sessnxt = pgrp->pg_sessnxt;
                    goto done2;
                }
                opgrp = spgrp->pg_sessnxt;
                spgrp = opgrp;
            }
            panic("pgdelete: can't find pgrp in session pgrp list");
        }

done2:
        SESS_UNLOCK(session);
	pgrp->pg_sessnxt = NULL;
	pgrp->pg_session = NULL;
	kfree(pgrp, sizeof(struct pgrp));

        SESS_LOCK(session);
        if(--session->s_count == 0) {
                session->s_fpgrpp = (struct pgrp **)NULL;
                vp = session->s_ttyvp;
                session->s_ttyvp = (struct vnode *)NULL;
                SESS_UNLOCK(session);
                ASSERT(vp == (struct vnode *)NULL);

                kfree(session, sizeof(struct session));
        } else {
                SESS_UNLOCK(session);
        }
}

/*
 * init the process queues
 */
pqinit()
{
	register struct proc *p;

	/*
	 * most procs are initially on freequeue
	 *	nb: we place them there in their "natural" order.
	 */

	freeproc = NULL;
	for (p = procNPROC; --p > proc; freeproc = p) {
		p->p_nxt = freeproc;
		PROC_LOCK_INIT(p);
	}

	/*
	 * but proc[0] is special ...
	 *
	 * N.B.  proc[0]'s proc lock was initialized in setup_main.
	 */

	allproc = p;
	p->p_nxt = NULL;
	p->p_prev = &allproc;

	zombproc = NULL;
}


#if	MACH_KDB

#define	printf	kdbprintf

extern int indent;

proc_print(p)
register struct proc *p;
{

	extern void cred_print();
	extern void print_simple_lock();

	iprintf("pid= %d p_link= %X p_rlink= %X p_nxt= %X\n",
		p->p_pid, p->p_link, p->p_rlink, p->p_nxt);
	iprintf("p_prev= %X p_usrpri= %x, p_pri= %x p_cpu= %x\n",
		p->p_prev, p->p_usrpri, p->p_pri, p->p_cpu);
	iprintf("p_stat= %x p_time= %x p_nice= %x p_slptime= %x\n",
		p->p_stat, p->p_time, p->p_nice, p->p_slptime);
	iprintf("p_cursig= %x p_sig= %X p_sigmask= %X p_sigignore= %X\n",
		p->p_cursig, p->p_sig, p->p_sigmask, p->p_sigignore);
	iprintf("p_sigcatch= 0x%X, p_flag= %X p_ruid= %d p_svuid= %d\n",
		p->p_sigcatch, p->p_flag, p->p_ruid, p->p_svuid);
	iprintf("p_rgid= %d p_svgid= %d p_ppid= %d p_xstat= %x\n",
		p->p_rgid, p->p_svgid, p->p_ppid, p->p_xstat);
        iprintf("p_pgid= %d, p_rcred= 0x%X, p_session= 0x%X\n",
		p->p_pgid, p->p_rcred, p->p_session);
	iprintf("Credentials (0x%X):\n", p->p_rcred);
	indent += 4;
	if ((p->p_rcred != NOCRED) && (p->p_rcred != 0))
		cred_print(p->p_rcred);
	indent -= 4;
	iprintf("p_ru= %X p_rssize= %X p_maxrss= %X p_swrss= %X\n",
		p->p_ru, p->p_rssize, p->p_maxrss, p->p_swrss);
	iprintf("p_swaddr= %X p_stopsig= %X p_cpticks= %X p_pctcpu= %X p_ndx= %x\n",
		p->p_swaddr, p->p_stopsig, p->p_cpticks, p->p_pctcpu, p->p_ndx);
	iprintf("p_idhash= %x p_pptr= %X p_cptr= %X p_osptr= %X\n",
		p->p_idhash, p->p_pptr, p->p_cptr, p->p_osptr);
	iprintf("p_ysptr= %X p_pgrp= %X p_pgrpnxt= %X\n",
		p->p_ysptr, p->p_pgrp, p->p_pgrpnxt);
	iprintf("p_traceflag= %X p_tracep= %X p_logdev= %x task= %X\n",
		p->p_traceflag, p->p_tracep, p->p_logdev, p->task);
	iprintf("utask= %X thread= %X exit_thread= %X\n",
		p->utask, p->thread, p->exit_thread);
	indent += 4;
	if (p->p_pgrp)
		pgrp_print(p->p_pgrp);
	indent -= 4;
#if	MACH_SLOCKS
	indent += 4;
	print_simple_lock(&p->p_lock);
	indent -= 4;
#endif
}


pgrp_print(pgrp)
register struct pgrp *pgrp;
{
	register struct proc *p;
	register struct session *s;

	iprintf("pg_id= %d pg_hforw= %X pg_mem= %X pg_session= %X pg_jobc= %X\n",
		pgrp->pg_id, pgrp->pg_hforw, pgrp->pg_mem, pgrp->pg_session,
		pgrp->pg_jobc);
	for (p = pgrp->pg_mem; p; p = p->p_pgrpnxt)
		printf("\t\tpid %d addr %X pgrp %X\n",
			p->p_pid, p, p->p_pgrp);
	if (s = pgrp->pg_session) {
		indent += 4;
		iprintf("s_count= %D s_leader= %X s_ttyvp= %X s_ttyp= %X\n",
			s->s_count, s->s_leader, s->s_ttyvp, s->s_ttyp);
		indent -= 4;
	}
}
#endif	/* MACH_KDB */
