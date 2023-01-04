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
static char *rcsid = "@(#)$RCSfile: pick.c,v $ $Revision: 4.1.3.2 $ (DEC) $Date: 1992/01/29 17:51:33 $";
#endif
/* pick.c - select messages by content */
/*

**Revision History

Oct-11, 1991 Aju John DEC ULTRIX/OSF Engineering   AJ01

Modifications were made so that pick would delete a sequence
if there were no hits. This prevents scan from picking up
the hits from a former pick with a same sequence name.

** Revision history ends

*/


#include "../h/mh.h"
#include "../zotnet/tws.h"
#include <stdio.h>

/*  */

static struct swit switches[] = {
#define	ANDSW	0
    "and", 0,
#define	ORSW	1
    "or", 0,
#define	NOTSW	2
    "not", 0,
#define	LBRSW	3
    "lbrace", 0,
#define	RBRSW	4
    "rbrace", 0,

#define	CCSW	5
    "cc  pattern", 0,
#define	DATESW	6
    "date  pattern", 0,
#define	FROMSW	7
    "from  pattern", 0,
#define	SRCHSW	8
    "search  pattern", 0,
#define	SUBJSW	9
    "subject  pattern", 0,
#define	TOSW	10
    "to  pattern", 0,
#define	OTHRSW	11
    "-othercomponent  pattern", 0,
#define	AFTRSW	12
    "after date", 0,
#define	BEFRSW	13
    "before date", 0,
#define	DATFDSW	14
    "datefield field", 5,

#define	SEQSW	15
    "sequence name", 0,
#define	PUBLSW	16
    "public", 0,
#define	NPUBLSW	17
    "nopublic", 0,
#define	ZEROSW	18
    "zero", 0,
#define	NZEROSW	19
    "nozero", 0,

#define	LISTSW	20
    "list", 0,
#define	NLISTSW	21
    "nolist", 0,

#define	HELPSW	22
    "help", 4,

    NULL, NULL
};

/*  */

static int  listsw = 0;

/*  */

/* ARGSUSED */

main (argc, argv)
char   *argv[];
{
    int     publicsw = -1,
            zerosw = 1,
            msgp = 0,
            seqp = 0,
            vecp = 0,
	    lo,
	    hi,
            msgnum;
    char   *maildir,
           *folder = NULL,
            buf[100],
           *cp,
          **ap,
          **argp,
           *arguments[MAXARGS],
           *msgs[MAXARGS],
           *seqs[NATTRS + 1],
           *vec[MAXARGS];

    struct msgs *mp;
    register FILE *fp;
                       /* AJ 01 */
    int    i,          /* a for loop counter */
           str_len,    /* length of lines in mh_seq file */
           change_flg, /* indicates lines have changed */
           compare,    /* stores status of comparisons */
           ret_val;    /* general function return status */
    int     seq_length = 800; /* max length of each line in seq file AJ01 */
    FILE   *fdtmp,     /* temporary file descriptor  */
           *fdseq;     /* mh_seq file descriptor */
    char   *lines;     /* buffer to read lines from mh_seq file */
    char   *mh_seq_tmp = ".mh_seq_tmp"; /* a temp seq file  AJ01 */

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
	if (*cp == '-') {
	    if (*++cp == '-') {
		vec[vecp++] = --cp;
		goto pattern;
	    }
	    switch (smatch (cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    adios (NULLCP, "-%s unknown", cp);
		case HELPSW: 
		    (void) sprintf (buf, "%s [+folder] [msgs] [switches]",
			    invo_name);
		    help (buf, switches);
		    listsw = 0;	/* HACK */
		    done (1);

		case CCSW: 
		case DATESW: 
		case FROMSW: 
		case SUBJSW: 
		case TOSW: 
		case DATFDSW: 
		case AFTRSW: 
		case BEFRSW: 
		case SRCHSW: 
		    vec[vecp++] = --cp;
	    pattern: ;
		    if (!(cp = *argp++))/* allow -xyz arguments */
			adios (NULLCP, "missing argument to %s", argp[-2]);
		    vec[vecp++] = cp;
		    continue;
		case OTHRSW: 
		    adios (NULLCP, "internal error!");

		case ANDSW:
		case ORSW:
		case NOTSW:
		case LBRSW:
		case RBRSW:
		    vec[vecp++] = --cp;
		    continue;

		case SEQSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, "missing argument to %s", argp[-2]);
		    if (seqp < NATTRS)
			seqs[seqp++] = cp;
		    else
			adios (NULLCP, "only %d sequences allowed!", NATTRS);
		    listsw = 0;
		    continue;
		case PUBLSW: 
		    publicsw = 1;
		    continue;
		case NPUBLSW: 
		    publicsw = 0;
		    continue;
		case ZEROSW: 
		    zerosw++;
		    continue;
		case NZEROSW: 
		    zerosw = 0;
		    continue;

		case LISTSW: 
		    listsw++;
		    continue;
		case NLISTSW: 
		    listsw = 0;
		    continue;
	    }
	}
	if (*cp == '+' || *cp == '@')
	    if (folder)
		adios (NULLCP, "only one folder at a time!");
	    else
		folder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	else
	    msgs[msgp++] = cp;
    }
    vec[vecp] = NULL;

/*  */

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (!msgp)
	msgs[msgp++] = "all";
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
    m_setseq (mp);

    if (seqp == 0)
	listsw++;
    if (publicsw == -1)
	publicsw = mp -> msgflags & READONLY ? 0 : 1;
    if (publicsw && (mp -> msgflags & READONLY))
	adios (NULLCP, "folder %s is read-only, so -public not allowed",
		folder);

/*  */

    if (!pcompile (vec, NULLCP))
	done (1);

    lo = mp -> lowsel;
    hi = mp -> hghsel;

    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED) {
	    if ((fp = fopen (cp = m_name (msgnum), "r")) == NULL)
		admonish (cp, "unable to read message");
	    if (fp && pmatches (fp, msgnum, 0L, 0L)) {
		if (msgnum < lo)
		    lo = msgnum;
		if (msgnum > hi)
		    hi = msgnum;
	    }
	    else {
		mp -> msgstats[msgnum] &= ~SELECTED;
		mp -> numsel--;
	    }
	    if (fp)
		(void) fclose (fp);
	}

    mp -> lowsel = lo;
    mp -> hghsel = hi;

    /* AJ 01 starts */
    if ( (mp -> numsel <= 0) && zerosw && ( *mh_seq != NULL) )
      {
	lines = (char *) calloc (seq_length, sizeof(char));
	if (lines == NULL)
	  adios ("calloc()", "unables to allocate memory");

	if (chdir (maildir) == NOTOK)
	  adios (maildir, "unable to change directory to");


	fdseq = fopen (mh_seq, "r");
	if (fdseq != NULL) /* if there was an earlier sequences file..*/
	  {                   
	    fclose( fdseq);

	    for (i=0; seqs[i]; i++)
	      {
		change_flg = 0; /* assume no changes */
		str_len = strlen(seqs[i]); 
		fdtmp = fopen ( mh_seq_tmp , "w");
		if (fdtmp == NULL)
		  adios (maildir, "unable to create files in");
		fdseq = fopen (mh_seq, "r");/* shudn't happen*/
		if (fdseq == NULL)
		  adios (maildir, "unable to open sequence file in");

		do
		  {
		    lines[0] = 0; /* initializing */
		    ret_val = fgets(lines, seq_length -1, fdseq);
		    if (ret_val == 0) break;
		    compare = strncmp(seqs[i], lines, str_len);
		    if (compare) /*no match if compare is true*/
		      fputs( lines, fdtmp);
		    else /* matched - a candidate to delete */
		      if (lines[str_len] != ':') /* confirming */
			fputs( lines, fdtmp);
		      else /* the entry is dropped, thus deleted */
			change_flg = 1;
		  }
		while (ret_val) ; /* for do ^ */

		fflush(fdtmp);
		fclose(fdtmp);
		fclose(fdseq);
		if (change_flg)
		  rename(mh_seq_tmp, mh_seq);
		else
		  unlink(mh_seq_tmp);
		i++;
	      } /*  for i, seqs[i] */
	  }/*  if (fdtmp != NULL) */

      } /* if ( (mp -> numsel <= 0) && zerosw)  */
    /* AJ 01 ends */

    if (mp -> numsel <= 0) 
      adios (NULLCP, "no messages match specification");
/*  */

    seqs[seqp] = NULL;
    for (seqp = 0; seqs[seqp]; seqp++) {
	if (zerosw && !m_seqnew (mp, seqs[seqp], publicsw))
	    done (1);
	for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	    if (mp -> msgstats[msgnum] & SELECTED)
		if (!m_seqadd (mp, seqs[seqp], msgnum, publicsw))
		    done (1);
    }

    if (listsw) {
	for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	    if (mp -> msgstats[msgnum] & SELECTED)
		printf ("%s\n", m_name (msgnum));
    }
    else
	printf ("%d hit%s\n", mp -> numsel,
		mp -> numsel == 1 ? "" : "s");

    m_replace (pfolder, folder);
    m_sync (mp);
    m_update ();

    done (0);
}

/*  */

void done (status)
int	status;
{
    if (listsw && status && !isatty (fileno (stdout)))
	printf ("0\n");
    exit (status);
}

