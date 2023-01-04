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
/* whom.c - report who a message would go to */
#ifndef	lint
static char ident[] = "@(#)$RCSfile: whom.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:39:37 $ devrcs Exp Locker: devbld $";
#endif	lint

#include "../h/mh.h"
#include <stdio.h>
#include <signal.h>

/*  */

static struct swit switches[] = {
#define	ALIASW	0
    "alias aliasfile", 0,

#define	CHKSW	1
    "check", 0,
#define	NOCHKSW	2
    "nocheck", 0,

#define	DRAFTSW	3
    "draft", 0,

#define	DFOLDSW	4
    "draftfolder +folder", 6,
#define	DMSGSW	5
    "draftmessage msg", 6,
#define	NDFLDSW	6
    "nodraftfolder", 0,

#define	HELPSW	7
    "help", 4,

#define	CLIESW	8
    "client host", -6,
#define	SERVSW	9
    "server host", -6,
#define	SNOOPSW	10
    "snoop", -5,

#define	FILLSW	11
    "fill-in file", -7,

#ifdef X400
#define MTSSW   12
    "mts smtp/x400", 0,
#endif X400

    NULL, NULL
};

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     child_id,
	    i,
	    status,
	    isdf = 0,
	    distsw = 0,
            vecp = 0;
    char   *cp,
	   *dfolder = NULL,
	   *dmsg = NULL,
           *msg = NULL,
          **ap,
          **argp,
	    backup[BUFSIZ],
            buf[100],
           *arguments[MAXARGS],
           *vec[MAXARGS];

    invo_name = r1bindex (argv[0], '/');
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

    vec[vecp++] = invo_name;
    vec[vecp++] = "-whom";
    vec[vecp++] = "-library";
    vec[vecp++] = getcpy (m_maildir (""));

/*  */

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    adios (NULLCP, "-%s unknown", cp);
		case HELPSW: 
		    (void) sprintf (buf, "%s [switches] [file]", invo_name);
		    help (buf, switches);
		    done (1);

		case CHKSW: 
		case NOCHKSW: 
		case SNOOPSW:
		    vec[vecp++] = --cp;
		    continue;

		case DRAFTSW:
		    msg = draft;
		    continue;

		case DFOLDSW: 
		    if (dfolder)
			adios (NULLCP, "only one draft folder at a time!");
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, "missing argument to %s", argp[-2]);
		    dfolder = path (*cp == '+' || *cp == '@' ? cp + 1 : cp,
			    *cp != '@' ? TFOLDER : TSUBCWF);
		    continue;
		case DMSGSW: 
		    if (dmsg)
			adios (NULLCP, "only one draft message at a time!");
		    if (!(dmsg = *argp++) || *dmsg == '-')
			adios (NULLCP, "missing argument to %s", argp[-2]);
		    continue;
		case NDFLDSW: 
		    dfolder = NULL;
		    isdf = NOTOK;
		    continue;

		case ALIASW: 
		case CLIESW: 
		case SERVSW: 
		case FILLSW:
#ifdef X400
                case MTSSW:
#endif X400
		    vec[vecp++] = --cp;
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, "missing argument to %s", argp[-2]);
		    vec[vecp++] = cp;
		    continue;
	    }
	if (msg)
	    adios (NULLCP, "only one draft at a time!");
	else
	    vec[vecp++] = msg = cp;
    }
    if (cp = m_find ("Aliasfile")) {	/* allow Aliasfile: profile entry */
	vec[vecp++] = "-alias";
	vec[vecp++] = getcpy(cp);
    }

/*  */

    if (msg == NULL) {
#ifdef	WHATNOW
	if ((cp = getenv ("mhdraft")) == NULL || *cp == NULL)
#endif	WHATNOW
	    cp  = getcpy (m_draft (dfolder, dmsg, 1, &isdf));
	msg = vec[vecp++] = cp;
    }
    if ((cp = getenv ("mhdist"))
	    && *cp
	    && (distsw = atoi (cp))
	    && (cp = getenv ("mhaltmsg"))
	    && *cp) {
	if (distout (msg, cp, backup) == NOTOK)
	    done (1);
	vec[vecp++] = "-dist";
	distsw++;
    }
    vec[vecp] = NULL;

    closefds (3);

    if (distsw)
	for (i = 0; (child_id = fork ()) == NOTOK && i < 5; i++)
	    sleep (5);
    switch (distsw ? child_id : OK) {
	case NOTOK:
    	    advise (NULLCP, "unable to fork, so checking directly...");
	case OK:
	    execvp (postproc, vec);
	    fprintf (stderr, "unable to exec ");
	    perror (postproc);
	    _exit (-1);

	default:
	    (void) signal (SIGHUP, SIG_IGN);
	    (void) signal (SIGINT, SIG_IGN);
	    (void) signal (SIGQUIT, SIG_IGN);
	    (void) signal (SIGTERM, SIG_IGN);

	    status = pidwait (child_id, OK);

	    (void) unlink (msg);
	    if (mhrename (backup, msg) == NOTOK)
		adios (msg, "unable to rename %s to", backup);
	    done (status);
    }
}
