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
/* ttym.c - miscellaneous routines */
#ifndef	lint
static char ident[] = "@(#)$RCSfile: ttym.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:39:26 $ devrcs Exp Locker: devbld $";
#endif	lint

#include <pwd.h>
#ifndef	NSIG
#include <signal.h>
#endif	NSIG
#include <sys/time.h>

static int  ttyf ();

/*  */

static  ttym (fd, command, line, user, vec)
int     fd;
char   *command,
       *line,
       *user,
      **vec;
{
    TYPESIG     (*pstat) ();
    char   *ap,
           *term,
           *myself,
           *getlogin (),
           *rindex (),
           *ttyname ();
    struct passwd  *pw,
                   *getpwuid ();

    if ((term = ap = ttyname (2)) && (term = rindex (term, '/')))
	term++;
    if (term == NULL || *term == NULL)
	term = ap;
    if ((myself = getlogin ()) == NULL || *myself == NULL)
	myself = (pw = getpwuid (getuid ())) ? pw -> pw_name : NULL;

    pstat = signal (SIGPIPE, SIG_IGN);
    (void) write (fd, command, strlen (command));
    (void) write (fd, "", 1);

    if (term)
	(void) write (fd, term, strlen (term));
    (void) write (fd, "", 1);

    if (myself)
	(void) write (fd, myself, strlen (myself));
    (void) write (fd, "", 1);

    if (line && *line)
	(void) write (fd, line, strlen (line));
    (void) write (fd, "", 1);

    if (user && *user)
	(void) write (fd, user, strlen (user));
    (void) write (fd, "", 1);

    if (vec)
	while (ap = *vec++) {
	    (void) write (fd, ap, strlen (ap));
	    (void) write (fd, "", 1);
	}

    (void) write (fd, "", 1);
    (void) signal (SIGPIPE, pstat);
}

/*  */

static int  ttyv (fd)
int     fd;
{
    int     ifds,
	    nbits;
    char    c;
    struct timeval tv;

    ifds = 1 << fd;
    nbits = getdtablesize();
    tv.tv_sec = SMLWAIT;
    tv.tv_usec = 0;
    if (select (nbits, &ifds, (int *) 0, (int *) 0, &tv) <= 0
	    || read (fd, &c, 1) != 1)
	return NOTOK;
    if (c == NULL)
	return fd;
    putc (c, stderr);

    (void) ttyf (fd, stderr);
    return NOTOK;
}


static int  ttyf (fd, f)
int     fd;
FILE * f;
{
    int     i;
    char    buffer[BUFSIZ];

    while ((i = read (fd, buffer, sizeof buffer)) > 0)
	(void) fwrite (buffer, sizeof (char), i, f);
    return i;
}
