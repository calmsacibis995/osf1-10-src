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
/* anno.c - annotate messages */
#ifndef	lint
static char ident[] = "@(#)$RCSfile: anno.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:36:52 $ devrcs Exp Locker: devbld $";
#endif	lint

#include "../h/mh.h"
#include <ctype.h>
#include <stdio.h>

static make_comp();
/*  */

static struct swit switches[] = {
#define	COMPSW	0
    "component field", 0,

#define	INPLSW	1
    "inplace", 0,
#define	NINPLSW	2
    "noinplace", 0,

#define	DATESW	3
    "date", 0,
#define	NDATESW	4
    "nodate", 0,

#define	TEXTSW	5
    "text body", 0,

#define	HELPSW	6
    "help", 4,

    NULL, NULL
};

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int     inplace = 0,
            datesw = 1,
            msgp = 0,
            msgnum;
    char   *cp,
           *maildir,
           *comp = NULL,
           *text = NULL,
           *folder = NULL,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *msgs[MAXARGS];
    struct msgs *mp;

    invo_name = r1bindex (argv[0], '/');
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

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
		    (void) sprintf (buf, "%s [+folder] [msgs] [switches]",
			    invo_name);
		    help (buf, switches);
		    done (1);

		case COMPSW: 
		    if (comp)
			adios (NULLCP, "only one component at a time!");
		    if (!(comp = *argp++) || *comp == '-')
			adios (NULLCP, "missing argument to %s", argp[-2]);
		    continue;

		case DATESW: 
		    datesw++;
		    continue;
		case NDATESW: 
		    datesw = 0;
		    continue;

		case INPLSW: 
		    inplace++;
		    continue;
		case NINPLSW: 
		    inplace = 0;
		    continue;

		case TEXTSW: 
		    if (text)
			adios (NULLCP, "only one body at a time!");
		    if (!(text = *argp++) || *text == '-')
			adios (NULLCP, "missing argument to %s", argp[-2]);
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    if (folder)
		adios (NULLCP, "only one folder at a time!");
	    else
		folder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	}
	else
	    msgs[msgp++] = cp;
    }
#ifdef UCI
    if (strcmp(invo_name, "fanno") == 0)	/* ugh! */
	datesw = 0;
#endif UCI

/*  */

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (!msgp)
	msgs[msgp++] = "cur";
    if (!folder)
	folder = m_getfolder ();
    maildir = m_maildir (folder);

    if (chdir (maildir) == NOTOK)
	adios (maildir, "unable to change directory to");
    if (!(mp = m_gmsg (folder)))
	adios (NULLCP, "unable to read folder %s", folder);
    if (mp -> hghmsg == 0)
	adios (NULLCP, "no messages in %s", folder);

    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    done (1);

    make_comp (&comp);

    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED)
	    (void) annotate (m_name (msgnum), comp, text, inplace, datesw);

    m_replace (pfolder, folder);
    if (mp -> lowsel != mp -> curmsg)
	m_setcur (mp, mp -> lowsel);
    m_sync (mp);
    m_update ();

    done (0);
}

/*  */

static make_comp (ap)
register char **ap;
{
    register char  *cp;
    char    buffer[BUFSIZ];

    if (*ap == NULL) {
	printf ("Enter component name: ");
	(void) fflush (stdout);

	if (fgets (buffer, sizeof buffer, stdin) == NULL)
	    done (1);
	*ap = trimcpy (buffer);
    }

    if ((cp = *ap + strlen (*ap) - 1) > *ap && *cp == ':')
	*cp = NULL;
    if (strlen (*ap) == 0)
	adios (NULLCP, "null component name");
    if (**ap == '-')
	adios (NULLCP, "invalid component name %s", *ap);
    if (strlen (*ap) >= NAMESZ)
	adios (NULLCP, "too large component name %s", *ap);

    for (cp = *ap; *cp; cp++)
	if (!isalnum (*cp) && *cp != '-')
	    adios (NULLCP, "invalid component name %s", *ap);
}
