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
static char *rcsid = "@(#)$RCSfile: kdebugd.c,v $ $Revision: 1.1.3.6 $ (DEC) $Date: 1992/11/06 15:35:02 $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <ctype.h>
#include <sys/param.h>

#define BUFSIZE 1024
#define LINESIZE 80
#define KDB 1
#define DBX 2

#define DPRINTF if (debug) fprintf

int kdebug_fd;
int dbx_fd;
int debug_fd;
FILE *debug_fp;
int writer;
int debug = 0;

typedef struct {
    char        line[100];
    char        debugtty[100];
} gateway;

char err_string[LINESIZE];

extern char *getenv();
static	char *tbuf;
static	int hopcount;	/* detect infinite loops in termcap, init 0 */
static	char *remotefile;

/*
 * Rdecode does the grunge work to decode the
 * string capability escapes.
 */
char *remdecode(str, area)
    register char *str;
    char **area;
{
	register char *cp;
	register int c;
	register char *dp;
	int i;

	cp = *area;
	while ((c = *str++) && c != ':') {
		switch (c) {

		case '^':
			c = *str++ & 037;
			break;

		case '\\':
			dp = "E\033^^\\\\::n\nr\rt\tb\bf\f";
			c = *str++;
nextc:
			if (*dp++ == c) {
				c = *dp++;
				break;
			}
			dp++;
			if (*dp)
				goto nextc;
			if (isdigit(c)) {
				c -= '0', i = 2;
				do
					c <<= 3, c |= *str++ - '0';
				while (--i && isdigit(*str));
			}
			break;
		}
		*cp++ = c;
	}
	*cp++ = 0;
	str = *area;
	*area = cp;
	return (str);
} /* remdecode */

/*
 * Skip to the next field.  Notice that this is very dumb, not
 * knowing about \: escapes or any such.  If necessary, :'s can be put
 * into the termcap file in octal.
 */
char *tskip(bp)
    register char *bp;
{

	while (*bp && *bp != ':')
		bp++;
	if (*bp == ':')
		bp++;
	return (bp);
} /* tskip */

/*
 * Get a string valued option.
 * These are given as
 *	cl=^Z
 * Much decoding is done on the strings, and the strings are
 * placed in area, which is a ref parameter which is updated.
 * No checking on area overflow.
 */
char *
rgetstr(id, area)
    char *id;
    char **area;
{
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return (0);
		if (*bp != '=')
			continue;
		bp++;
		return (remdecode(bp, area));
	}
} /* rgetstr */

init(argc, argv, init_rset)
    int argc;
    char *argv[];
    fd_set *init_rset;
{
    unsigned bits = LDECCTQ;
    struct sgttyb arg;
    static char buf[BUFSIZE];
    static char buf2[BUFSIZE];
    char *DV=buf2;
    int BR;
    gateway gate;

    /*
     * get the run control information from dbx
     */
    dbx_fd = 0;
    if (read(dbx_fd, &gate, sizeof(gate)) != sizeof(gate)) {
	fatal("bad read");
    }

    /*
     * initialize the defaults parameters
     */

    if (strlen(gate.debugtty)) {
        if ((debug_fd = open(gate.debugtty, O_RDWR | O_NDELAY, NULL)) > 0) {
	    if ((debug_fp = fdopen(debug_fd, "r+")) != NULL) {
		debug = 1;
	    }
	}
    }

    if (rgetent(buf, gate.line) <= 0) {
	sprintf(err_string,
		"cannot get KDEBUG_LINE entry for '%s', check /etc/remote\n",
                gate.line);
	fatal(err_string);
    }

    if ((BR = rgetnum("br")) <= 0) {
	sprintf(err_string,
		"%s has no br(baud rate), edit /etc/remote\n", gate.line);
	fatal(err_string);
    }

    if ((DV = rgetstr("dv", &DV)) == (char *)0) {
	sprintf(err_string,
        	"%s has no dv(device), edit /etc/remote\n", gate.line);
	fatal(err_string);
    }

    kdebug_fd = open(DV, O_RDWR, O_EXCL);
    if (kdebug_fd < 0) {
	sprintf(err_string,
        	"can't open %s(%s), check device file\n", gate.line, DV);
	fatal(err_string);
    }

    switch (BR) {
    case 50:
        arg.sg_ispeed = arg.sg_ospeed = B50;
	break;
    case 75:
        arg.sg_ispeed = arg.sg_ospeed = B75;
	break;
    case 110:
        arg.sg_ispeed = arg.sg_ospeed = B110;
	break;
    case 134:
        arg.sg_ispeed = arg.sg_ospeed = B134;
	break;
    case 150:
        arg.sg_ispeed = arg.sg_ospeed = B150;
	break;
    case 200:
        arg.sg_ispeed = arg.sg_ospeed = B200;
	break;
    case 300:
        arg.sg_ispeed = arg.sg_ospeed = B300;
	break;
    case 600:
        arg.sg_ispeed = arg.sg_ospeed = B600;
	break;
    case 1200:
        arg.sg_ispeed = arg.sg_ospeed = B1200;
	break;
    case 1800:
        arg.sg_ispeed = arg.sg_ospeed = B1800;
	break;
    case 2400:
        arg.sg_ispeed = arg.sg_ospeed = B2400;
	break;
    case 4800:
        arg.sg_ispeed = arg.sg_ospeed = B4800;
	break;
    case 9600:
        arg.sg_ispeed = arg.sg_ospeed = B9600;
	break;
    case 19200:
        arg.sg_ispeed = arg.sg_ospeed = EXTA;
	break;
    default:
	sprintf(err_string,
        	"unknown speed(%d) for %s(%s)\n", BR, gate.line, DV);
	fatal(err_string);
    };

    arg.sg_flags = RAW;
    ioctl(kdebug_fd, TIOCSETP, (char *)&arg);
    ioctl(kdebug_fd, TIOCLBIS, (char *)&bits);

    ioctl(dbx_fd, TIOCSETP, (char *)&arg);
    ioctl(dbx_fd, TIOCLBIS, (char *)&bits);

    writer = 0;

    FD_ZERO(init_rset);
    FD_SET(kdebug_fd, init_rset);
    FD_SET(dbx_fd, init_rset);

    DPRINTF(debug_fp, "kdebugd: starting on line %s\n", gate.line);
    fflush(0);
}

quit()
{
    DPRINTF(debug_fp, "kdebugd: exiting\n");
    fflush(0);
    exit(0);
}

fatal(msg)
    char *msg;
{
    extern int errno;

    DPRINTF(debug_fp, "kdebugd: %s (errno %d)\n", msg, errno);
    fflush(0);
    exit(1);
}

main(argc, argv)
    int	argc;
    char *argv[];
{
    int nbytes;
    char buf[BUFSIZE];
    fd_set rset;
    fd_set init_rset;

    init(argc, argv, &init_rset);

    while (1) {
	bcopy(&init_rset, &rset, sizeof(fd_set));
	if (select(FD_SETSIZE, &rset, NULL, NULL, NULL) < 0) {
	    fatal("select");
	}

	if (FD_ISSET(kdebug_fd, &rset)) {
	    if ((nbytes = read(kdebug_fd, buf, BUFSIZE)) <= 0) {
	        fatal("read from kdebug");
	    }
	    write(dbx_fd, buf, nbytes);

	    if (debug) {
	        if (writer != KDB) {
	            write(debug_fd, "\nKDB> ", 6);
	        }
	        writer = KDB;
	        write(debug_fd, buf, nbytes);
	    }
	}

	if (FD_ISSET(dbx_fd, &rset)) {
	    if ((nbytes = read(dbx_fd, buf, BUFSIZE)) <= 0) {
	        fatal("read from dbx");
	    }
	    write(kdebug_fd, buf, nbytes);

	    if (debug) {
	        if (writer != DBX)
	            write(debug_fd, "\nDBX> ", 6);
	        writer = DBX;
	        write(debug_fd, buf, nbytes);
	    }
	}
    }
}

/*
 * remcap - routines for dealing with the remote host data base
 *
 * derived from termcap
 */

#define MAXHOP		32		/* max number of tc= indirections */
#define SYSREMOTE	"/etc/remote"	/* system remote file */

#define	E_TERMCAP	RM = SYSREMOTE
#define V_TERMCAP	"REMOTE"
#define V_TERM		"HOST"

static char	*RM;

/*
 * termcap - routines for dealing with the terminal capability data base
 *
 * BUG:		Should use a "last" pointer in tbuf, so that searching
 *		for capabilities alphabetically would not be a n**2/2
 *		process when large numbers of capabilities are given.
 * Note:	If we add a last pointer now we will screw up the
 *		tc capability. We really should compile termcap.
 *
 * Essentially all the work here is scanning and decoding escapes
 * in string capabilities.  We don't use stdio because the editor
 * doesn't, and because living w/o it is not hard.
 */

/*
 * Get an entry for terminal name in buffer bp,
 * from the termcap file.  Parse is very rudimentary;
 * we just notice escaped newlines.
 */
rgetent(bp, name)
	char *bp;
	char *name;
{
	char lbuf[BUFSIZE], *cp, *p;
	int rc1, rc2;

	remotefile = cp = getenv(V_TERMCAP);
	if (cp == (char *)0 || strcmp(cp, SYSREMOTE) == 0) {
		remotefile = cp = SYSREMOTE;
		return (getent(bp, name, cp));
	} else {
		if ((rc1 = getent(bp, name, cp)) != 1)
			*bp = '\0';
		remotefile = cp = SYSREMOTE;
		rc2 = getent(lbuf, name, cp);
		if (rc1 != 1 && rc2 != 1)
			return (rc2);
		if (rc2 == 1) {
			p = lbuf;
			if (rc1 == 1)
				while (*p++ != ':')
					;
			if (strlen(bp) + strlen(p) > BUFSIZE) {
				write(2, "Remcap entry too long\n", 23);
				return (-1);
			}
			strcat(bp, p);
		}
		tbuf = bp;
		return (1);
	}
} /* rgetent */

getent(bp, name, cp)
    char *bp, *name, *cp;
{
	register int c;
	register int i = 0, cnt = 0;
	char ibuf[BUFSIZE], *cp2;
	int tf;

	tbuf = bp;
	tf = 0;
	/*
	 * TERMCAP can have one of two things in it. It can be the
	 * name of a file to use instead of /etc/termcap. In this
	 * case it better start with a "/". Or it can be an entry to
	 * use so we don't have to read the file. In this case it
	 * has to already have the newlines crunched out.
	 */
	if (cp && *cp) {
		if (*cp!='/') {
			cp2 = getenv(V_TERM);
			if (cp2 == (char *)0 || strcmp(name,cp2) == 0) {
				strcpy(bp,cp);
				return (rnchktc());
			} else
				tf = open(E_TERMCAP, O_RDONLY);
		} else
			tf = open(RM = cp, O_RDONLY);
	}
	if (tf == 0)
		tf = open(E_TERMCAP, O_RDONLY);
	if (tf < 0)
		return (-1);
	for (;;) {
		cp = bp;
		for (;;) {
			if (i == cnt) {
				cnt = read(tf, ibuf, BUFSIZE);
				if (cnt <= 0) {
					close(tf);
					return (0);
				}
				i = 0;
			}
			c = ibuf[i++];
			if (c == '\n') {
				if (cp > bp && cp[-1] == '\\') {
					cp--;
					continue;
				}
				break;
			}
			if (cp >= bp+BUFSIZE) {
				write(2,"Remcap entry too long\n", 23);
				break;
			} else
				*cp++ = c;
		}
		*cp = 0;

		/*
		 * The real work for the match.
		 */
		if (rnamatch(name)) {
			close(tf);
			return (rnchktc());
		}
	}
} /* getent */

/*
 * rnchktc: check the last entry, see if it's tc=xxx. If so,
 * recursively find xxx and append that entry (minus the names)
 * to take the place of the tc=xxx entry. This allows termcap
 * entries to say "like an HP2621 but doesn't turn on the labels".
 * Note that this works because of the left to right scan.
 */
rnchktc()
{
	register char *p, *q;
	char tcname[BUFSIZE];	/* name of similar terminal */
	char tcbuf[BUFSIZE];
	char *holdtbuf = tbuf;
	int l;
	char *cp;

	p = tbuf + strlen(tbuf) - 2;	/* before the last colon */
	while (*--p != ':')
		if (p<tbuf) {
			write(2, "Bad remcap entry\n", 18);
			return (0);
		}
	p++;
	/* p now points to beginning of last field */
	if (p[0] != 't' || p[1] != 'c')
		return (1);
	strcpy(tcname, p+3);
	q = tcname;
	while (*q && *q != ':')
		q++;
	*q = 0;
	if (++hopcount > MAXHOP) {
		write(2, "Infinite tc= loop\n", 18);
		return (0);
	}
	if (getent(tcbuf, tcname, remotefile) != 1) {
		if (strcmp(remotefile, SYSREMOTE) == 0)
			return (0);
		else if (getent(tcbuf, tcname, SYSREMOTE) != 1)
			return (0);
	}
	for (q = tcbuf; *q++ != ':'; )
		;
	l = p - holdtbuf + strlen(q);
	if (l > BUFSIZE) {
		write(2, "Remcap entry too long\n", 23);
		q[BUFSIZE - (p-holdtbuf)] = 0;
	}
	strcpy(p, q);
	tbuf = holdtbuf;
	return (1);
} /* rnchktc */

/*
 * Tnamatch deals with name matching.  The first field of the termcap
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal : terminator after the last
 * name (before the first field) stops us.
 */
rnamatch(np)
    char *np;
{
	register char *Np, *Bp;

	Bp = tbuf;
	if (*Bp == '#')
		return (0);
	for (;;) {
		for (Np = np; *Np && *Bp == *Np; Bp++, Np++)
			continue;
		if (*Np == 0 && (*Bp == '|' || *Bp == ':' || *Bp == 0))
			return (1);
		while (*Bp && *Bp != ':' && *Bp != '|')
			Bp++;
		if (*Bp == 0 || *Bp == ':')
			return (0);
		Bp++;
	}
} /* rnamatch */

/*
 * Return the (numeric) option id.
 * Numeric options look like
 *	li#80
 * i.e. the option string is separated from the numeric value by
 * a # character.  If the option is not found we return -1.
 * Note that we handle octal numbers beginning with 0.
 */
rgetnum(id)
    char *id;
{
	register int i, base;
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (*bp == 0)
			return (-1);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return (-1);
		if (*bp != '#')
			continue;
		bp++;
		base = 10;
		if (*bp == '0')
			base = 8;
		i = 0;
		while (isdigit(*bp))
			i *= base, i += *bp++ - '0';
		return (i);
	}
} /* rgetnum */

/*
 * Handle a flag option.
 * Flag options are given "naked", i.e. followed by a : or the end
 * of the buffer.  Return 1 if we find the option, or 0 if it is
 * not given.
 */
rgetflag(id)
    char *id;
{
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
			if (!*bp || *bp == ':')
				return (1);
			else if (*bp == '@')
				return (0);
		}
	}
} /* rgetflag */
