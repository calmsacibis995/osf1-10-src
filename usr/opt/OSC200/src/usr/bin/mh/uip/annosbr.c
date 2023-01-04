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
/* annosbr.c - prepend annotation to messages */
#ifndef	lint
static char ident[] = "@(#)$RCSfile: annosbr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:36:55 $ devrcs Exp Locker: devbld $";
#endif	lint

#include "../h/mh.h"
#include "../zotnet/tws.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


extern int  errno;
long lseek ();
static annosbr();

/*  */

annotate (file, comp, text, inplace, datesw)
register char   *file,
		*comp,
		*text;
int     inplace,
        datesw;
{
    int     i,
            fd;

    if ((fd = lkopen (file, 2)) == NOTOK) {
	switch (errno) {
	    case ENOENT: 
		break;

	    default: 
		admonish (file, "unable to lock and open");
		break;
	}
	return 1;
    }

    i = annosbr (fd, file, comp, text, inplace, datesw);

    (void) lkclose (fd, file);

    return i;
}

/*  */

static	annosbr (src, file, comp, text, inplace, datesw)
register char  *file,
	       *comp,
	       *text;
int     src,
	inplace,
	datesw;
{
    int     mode,
            fd;
    register char  *cp,
                   *sp;
    char    buffer[BUFSIZ],
            tmpfil[BUFSIZ];
    struct stat st;
    register    FILE *tmp;

    mode = fstat (src, &st) != NOTOK ? (st.st_mode & 0777) : m_gmprot ();

    (void) strcpy (tmpfil, m_scratch (file, "annotate"));

    if ((tmp = fopen (tmpfil, "w")) == NULL) {
	admonish (tmpfil, "unable to create");
	return 1;
    }
    (void) chmod (tmpfil, mode);

    if (datesw)
	fprintf (tmp, "%s: %s\n", comp, dtimenow ());
    if (cp = text) {
	do {
	    while (*cp == ' ' || *cp == '\t')
		cp++;
	    sp = cp;
	    while (*cp && *cp++ != '\n')
		continue;
	    if (cp - sp)
		fprintf (tmp, "%s: %*.*s", comp, cp - sp, cp - sp, sp);
	} while (*cp);
	if (cp[-1] != '\n' && cp != text)
	    (void) putc ('\n', tmp);
    }
    (void) fflush (tmp);
    cpydata (src, fileno (tmp), file, tmpfil);
    (void) fclose (tmp);

    if (inplace) {
	if ((fd = open (tmpfil, 0)) == NOTOK)
	    adios (tmpfil, "unable to open for re-reading");
	(void) lseek (src, 0L, 0);
	cpydata (fd, src, tmpfil, file);
	(void) close (fd);
	(void) unlink (tmpfil);
    }
    else {
	(void) strcpy (buffer, m_backup (file));
	if (mhrename (file, buffer) == NOTOK) {
	    switch (errno) {
		case ENOENT:	/* unlinked early - no annotations */
		    (void) unlink (tmpfil);
		    break;

		default:
		    admonish (buffer, "unable to rename %s to", file);
		    break;
	    }
	    return 1;
	}
	if (mhrename (tmpfil, file) == NOTOK) {
	    admonish (file, "unable to rename %s to", tmpfil);
	    return 1;
	}
    }

    return 0;
}
