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
/* m_sync.c - synchronize message sequences */
#ifndef	lint
static char ident[] = "@(#)$RCSfile: m_sync.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:36:24 $ devrcs Exp Locker: devbld $";
#endif	lint

#include "../h/mh.h"
#include <stdio.h>
#include <signal.h>
#ifndef	sigmask
#define	sigmask(s)	(1 << ((s) - 1))
#endif	not sigmask


/* decision logic
    1.  public and folder readonly: make it private
    2a. public: add it to the sequences file
    2b. private: add it to the profile
 */


void m_sync (mp)
register struct msgs *mp;
{
    int     bits;
    register int    i;
    register char  *cp;
    char    flags,
	    attr[BUFSIZ],
	    seq[BUFSIZ * 2];
    register FILE  *fp;
#ifndef	BSD42
    TYPESIG (*hstat) (), (*istat) (), (*qstat) (), (*tstat) ();
#else	BSD42
    int	    smask;
#endif	BSD42

    if (!(mp -> msgflags & SEQMOD))
	return;
    mp -> msgflags &= ~SEQMOD;

    m_getdefs ();
    (void) sprintf (seq, "%s/%s", mp -> foldpath, mh_seq);
    bits = FFATTRSLOT;
    fp = NULL;

    flags = mp -> msgflags;
    if (mh_seq == NULL || *mh_seq == NULL)
	mp -> msgflags |= READONLY;

    for (i = 0; mp -> msgattrs[i]; i++) {
	(void) sprintf (attr, "atr-%s-%s", mp -> msgattrs[i], mp -> foldpath);
	if (mp -> msgflags & READONLY
		|| mp -> attrstats & (1 << (bits + i))) {
    priv: ;
	    if (cp = m_seq (mp, mp -> msgattrs[i]))
		m_replace (attr, cp);
	    else
		(void) m_delete (attr);
	}
	else {
	    (void) m_delete (attr);
	    if ((cp = m_seq (mp, mp -> msgattrs[i])) == NULL)
		continue;
	    if (fp == NULL) {
		if ((fp = fopen (seq, "w")) == NULL
			&& (unlink (seq) == NOTOK ||
			    (fp = fopen (seq, "w")) == NULL)) {
		    admonish (attr, "unable to write");
		    goto priv;
		}
#ifndef	BSD42
		hstat = signal (SIGHUP, SIG_IGN);
		istat = signal (SIGINT, SIG_IGN);
		qstat = signal (SIGQUIT, SIG_IGN);
		tstat = signal (SIGTERM, SIG_IGN);
#else	BSD42
		smask = sigblock (sigmask (SIGHUP) | sigmask (SIGINT)
				    | sigmask (SIGQUIT) | sigmask (SIGTERM));
#endif	BSD42
	    }
	    fprintf (fp, "%s: %s\n", mp -> msgattrs[i], cp);
	}
    }

    if (fp) {
	(void) fclose (fp);
#ifndef	BSD42
	(void) signal (SIGHUP, hstat);
	(void) signal (SIGINT, istat);
	(void) signal (SIGQUIT, qstat);
	(void) signal (SIGTERM, tstat);
#else	BSD42
	(void) sigsetmask (smask);
#endif	BSD42
    }
    else
	if (!(mp -> msgflags & READONLY))
	    (void) unlink (seq);

    mp -> msgflags = flags;
}
