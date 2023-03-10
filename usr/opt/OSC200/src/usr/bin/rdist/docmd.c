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
static char	*sccsid = "@(#)$RCSfile: docmd.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/05/11 09:08:00 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * docmd.c	1.6  com/cmd/arch/rdist,3.1,9013 11/1/89 13:48:51
 */

/*
#ifndef lint
static char sccsid[] = "(#)docmd.c	5.6 (Berkeley) 6/1/90";
#endif not lint
*/

#include <setjmp.h>
#include <netdb.h>
#include "defs.h"

FILE	*lfp;			/* log file for recording files updated */
struct	subcmd *subcmds;	/* list of sub-commands for current cmd */
jmp_buf	env;

void	cleanup();
void	lostconn();

/*
 * Do the commands in cmds (initialized by yyparse).
 */
docmds(dhosts, argc, argv)
	char **dhosts;
	int argc;
	char **argv;
{
	register struct cmd *c;
	register struct namelist *f;
	register char **cpp;
	extern struct cmd *cmds;

	signal(SIGHUP, cleanup);
	signal(SIGINT, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGTERM, cleanup);

	for (c = cmds; c != NULL; c = c->c_next) {
		if (dhosts != NULL && *dhosts != NULL) {
			for (cpp = dhosts; *cpp; cpp++)
				if (strcmp(c->c_name, *cpp) == 0)
					goto fndhost;
			continue;
		}
	fndhost:
		if (argc) {
			for (cpp = argv; *cpp; cpp++) {
				if (c->c_label != NULL &&
				    strcmp(c->c_label, *cpp) == 0) {
					cpp = NULL;
					goto found;
				}
				for (f = c->c_files; f != NULL; f = f->n_next)
					if (strcmp(f->n_name, *cpp) == 0)
						goto found;
			}
			continue;
		} else
			cpp = NULL;
	found:
		switch (c->c_type) {
		case ARROW:
			doarrow(cpp, c->c_files, c->c_name, c->c_cmds);
			break;
		case DCOLON:
			dodcolon(cpp, c->c_files, c->c_name, c->c_cmds);
			break;
		default:
			fatal(MSGSTR(ILLCOM, "illegal command type %d\n"), c->c_type);
		}
	}
	closeconn();
}

/*
 * Process commands for sending files to other machines.
 */
doarrow(filev, files, rhost, cmds)
	char **filev;
	struct namelist *files;
	char *rhost;
	struct subcmd *cmds;
{
	register struct namelist *f;
	register struct subcmd *sc;
	register char **cpp;
	int n, ddir, opts = options;

	if (debug)
		printf("doarrow(%x, %s, %x)\n", files, rhost, cmds);

	if (files == NULL) {
		error(MSGSTR(NOUPDATE, "no files to be updated\n"));
		return;
	}

	subcmds = cmds;
	ddir = files->n_next != NULL;	/* destination is a directory */
	if (nflag)
		printf(MSGSTR(UPDATEH, "updating host %s\n"), rhost);
	else {
		if (setjmp(env))
			goto done;
		signal(SIGPIPE, lostconn);
		if (!makeconn(rhost))
			return;
		if ((lfp = fopen(tmpf, "w")) == NULL) {
			fatal(MSGSTR(CANTOPEN, "cannot open %s\n"), tmpf);
			exit(1);
		}
	}
	for (f = files; f != NULL; f = f->n_next) {
		if (filev != NULL) {
			for (cpp = filev; *cpp; cpp++)
				if (strcmp(f->n_name, *cpp) == 0)
					goto found;
			if (!nflag)
				(void) fclose(lfp);
			continue;
		}
	found:
		n = 0;
		if (cmds != NULL) {
			for (sc = cmds; sc != NULL; sc = sc->sc_next) {
				if (sc->sc_type != INSTALL)
					continue;
				n++;
				install(f->n_name, sc->sc_name,
					sc->sc_name == NULL ? 0 : ddir, sc->sc_options);
				opts = sc->sc_options;
			}
		}
		if (n == 0)
			install(f->n_name, NULL, 0, options);
	}
done:
	if (!nflag) {
		(void) signal(SIGPIPE, cleanup);
		(void) fclose(lfp);
		lfp = NULL;
	}
	if (cmds != NULL) {
		for (sc = cmds; sc != NULL; sc = sc->sc_next)
			if (sc->sc_type == NOTIFY)
				notify(tmpf, rhost, sc->sc_args, 0);
	}
	if (!nflag) {
		(void) unlink(tmpf);
		for (; ihead != NULL; ihead = ihead->nextp) {
			free((void *)ihead);
			if ((opts & IGNLNKS) || ihead->count == 0)
				continue;
			log(lfp, MSGSTR(MISSLN, "%s: Warning: missing links\n"),
				ihead->pathname);
		}
	}
}

/*
 * Create a connection to the rdist server on the machine rhost.
 */
makeconn(rhost)
	char *rhost;
{
	register char *ruser, *cp;
	static char *cur_host = NULL;
	/* static int port = -1; */
	static u_short port;
	static int newport = TRUE;
	char tuser[20];
	int n;
	extern char user[];
	extern int userid;

	if (debug)
		printf("makeconn(%s)\n", rhost);

	if (cur_host != NULL && rem >= 0) {
		if (strcmp(cur_host, rhost) == 0)
			return(1);
		closeconn();
	}
	cur_host = rhost;
	cp = index(rhost, '@');
	if (cp != NULL) {
		char c = *cp;

		*cp = '\0';
		strncpy(tuser, rhost, sizeof(tuser)-1);
		*cp = c;
		rhost = cp + 1;
		ruser = tuser;
		if (*ruser == '\0')
			ruser = user;
		else if (!okname(ruser))
			return(0);
	} else
		ruser = user;
	if (!qflag)
		printf(MSGSTR(UPDATEH, "updating host %s\n"), rhost);
	(void) sprintf(buf, "%s -Server%s", _PATH_RDIST, qflag ? " -q" : "");
	if (newport) {
		struct servent *sp;

		if ((sp = getservbyname("shell", "tcp")) == NULL)
			fatal(MSGSTR(UNKNOWNS, "shell/tcp: unknown service"));
		port = sp->s_port;
		newport = FALSE;
	}

	if (debug) {
		printf("port = %d, luser = %s, ruser = %s\n", ntohs(port), user, ruser);
		printf("buf = %s\n", buf);
	}

	fflush(stdout);
	setreuid(userid, 0);
	rem = rcmd(&rhost, port, user, ruser, buf, (char *)0);
	setreuid(0, userid);
	if (rem < 0)
		return(0);
	cp = buf;
	if (read(rem, cp, 1) != 1)
		lostconn();
	if (*cp == 'V') {
		do {
			if (read(rem, cp, 1) != 1)
				lostconn();
		} while (*cp++ != '\n' && cp < &buf[BUFSIZ]);
		*--cp = '\0';
		cp = buf;
		n = 0;
		while (*cp >= '0' && *cp <= '9')
			n = (n * 10) + (*cp++ - '0');
		if (*cp == '\0' && n == VERSION)
			return(1);
		error(MSGSTR(CONNFAIL, "connection failed: version numbers don't match (local %d, remote %d)\n"), VERSION, n);
	} else
		error(MSGSTR(CONNFAIL2, "connection failed: version numbers don't match\n"));
	closeconn();
	return(0);
}

/*
 * Signal end of previous connection.
 */
closeconn()
{
	if (debug)
		printf("closeconn()\n");

	if (rem >= 0) {
		(void) write(rem, "\2\n", 2);
		(void) close(rem);
		rem = -1;
	}
}

void
lostconn()
{
	if (iamremote)
		cleanup();
	log(lfp, MSGSTR(LOSTCON, "rdist: lost connection\n"));
	longjmp(env, 1);
}

okname(name)
	register char *name;
{
	register char *cp = name;
	register int c;

	do {
		c = *cp;
#if defined(NLS) || defined(KJI)
		if (!NCisalpha(c) && !NCisdigit(c) && c != '_' && c != '-')
			goto bad;
#else
		if (c & 0200)
			goto bad;
		if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-')
			goto bad;
#endif
		cp++;
	} while (*cp);
	return(1);
bad:
	error(MSGSTR(USERN, "invalid user name %s\n"), name);
	return(0);
}

time_t	lastmod;
FILE	*tfp;
extern	char target[], *tp;

/*
 * Process commands for comparing files to time stamp files.
 */
dodcolon(filev, files, stamp, cmds)
	char **filev;
	struct namelist *files;
	char *stamp;
	struct subcmd *cmds;
{
	register struct subcmd *sc;
	register struct namelist *f;
	register char **cpp;
	struct timeval tv[2];
	struct timezone tz;
	struct stat stb;

	if (debug)
		printf("dodcolon()\n");

	if (files == NULL) {
		error(MSGSTR(NOUPDATE, "no files to be updated\n"));
		return;
	}
	if (stat(stamp, &stb) < 0) {
		error("%s: %s\n", stamp, sys_errlist[errno]);
		return;
	}
	if (debug)
		printf("%s: %d\n", stamp, stb.st_mtime);

	subcmds = cmds;
	lastmod = stb.st_mtime;
	if (nflag || (options & VERIFY))
		tfp = NULL;
	else {
		if ((tfp = fopen(tmpf, "w")) == NULL) {
			error("%s: %s\n", stamp, sys_errlist[errno]);
			return;
		}
		(void) gettimeofday(&tv[0], &tz);
		tv[1] = tv[0];
		(void) utimes(stamp, tv);
	}

	for (f = files; f != NULL; f = f->n_next) {
		if (filev != NULL) {
			for (cpp = filev; *cpp; cpp++)
				if (strcmp(f->n_name, *cpp) == 0)
					goto found;
			continue;
		}
	found:
		tp = NULL;
		cmptime(f->n_name);
	}

	if (tfp != NULL)
		(void) fclose(tfp);
	for (sc = cmds; sc != NULL; sc = sc->sc_next)
		if (sc->sc_type == NOTIFY)
			notify(tmpf, NULL, sc->sc_args, lastmod);
	if (!nflag && !(options & VERIFY))
		(void) unlink(tmpf);
}

/*
 * Compare the mtime of file to the list of time stamps.
 */
cmptime(name)
	char *name;
{
	struct stat stb;

	if (debug)
		printf("cmptime(%s)\n", name);

	if (except(name))
		return;

	if (nflag) {
		printf(MSGSTR(COMPDATES, "comparing dates: %s\n"), name);
		return;
	}

	/*
	 * first time cmptime() is called?
	 */
	if (tp == NULL) {
		if (exptilde(target, name) == NULL)
			return;
		tp = name = target;
		while (*tp)
			tp++;
	}
	if (access(name, 4) < 0 || stat(name, &stb) < 0) {
		error("%s: %s\n", name, sys_errlist[errno]);
		return;
	}

	switch (stb.st_mode & S_IFMT) {
	case S_IFREG:
		break;

	case S_IFDIR:
		rcmptime(&stb);
		return;

	default:
		error(MSGSTR(NOTPLAIN, "%s: not a plain file\n"), name);
		return;
	}

	if (stb.st_mtime > lastmod)
		log(tfp, MSGSTR(NEWFILE, "new: %s\n"), name);
}

rcmptime(st)
	struct stat *st;
{
	register DIR *d;
	register struct dirent *dp;
	register char *cp;
	char *otp;
	int len;

	if (debug)
		printf("rcmptime(%x)\n", st);

	if ((d = opendir(target)) == NULL) {
		error("%s: %s\n", target, sys_errlist[errno]);
		return;
	}
	otp = tp;
	len = tp - target;
	while (dp = readdir(d)) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (len + 1 + strlen(dp->d_name) >= BUFSIZ - 1) {
			error(MSGSTR(TOOLONG, "%s/%s: Name too long\n"), target, dp->d_name);
			continue;
		}
		tp = otp;
		*tp++ = '/';
		cp = dp->d_name;
		while (*tp++ = *cp++)
			;
		tp--;
		cmptime(target);
	}
	closedir(d);
	tp = otp;
	*tp = '\0';
}

/*
 * Notify the list of people the changes that were made.
 * rhost == NULL if we are mailing a list of changes compared to at time
 * stamp file.
 */
notify(file, rhost, to, lmod)
	char *file, *rhost;
	register struct namelist *to;
	time_t lmod;
{
	register int fd, len;
	FILE *pf;
	struct stat stb;

	if ((options & VERIFY) || to == NULL)
		return;
	if (!qflag) {
		printf(MSGSTR(NOTIFY1, "notify "));
		if (rhost)
			printf("@%s ", rhost);
		prnames(to);
	}
	if (nflag)
		return;

	if ((fd = open(file, 0)) < 0) {
		error("%s: %s\n", file, sys_errlist[errno]);
		return;
	}
	if (fstat(fd, &stb) < 0) {
		error("%s: %s\n", file, sys_errlist[errno]);
		(void) close(fd);
		return;
	}
	if (stb.st_size == 0) {
		(void) close(fd);
		return;
	}
	/*
	 * Create a pipe to mailling program.
	 */
	(void)sprintf(buf, "%s -oi -t", _PATH_SENDMAIL);
	pf = popen(buf, "w");
	if (pf == NULL) {
		error(MSGSTR(MAILFAIL, "notify: \"%s\" failed\n"), _PATH_SENDMAIL);
		(void) close(fd);
		return;
	}
	/*
	 * Output the proper header information.
	 */
	fprintf(pf, "From: rdist (Remote distribution program)\n");
	fprintf(pf, "To:");
	if (!any('@', to->n_name) && rhost != NULL)
		fprintf(pf, " %s@%s", to->n_name, rhost);
	else
		fprintf(pf, " %s", to->n_name);
	to = to->n_next;
	while (to != NULL) {
		if (!any('@', to->n_name) && rhost != NULL)
			fprintf(pf, ", %s@%s", to->n_name, rhost);
		else
			fprintf(pf, ", %s", to->n_name);
		to = to->n_next;
	}
	putc('\n', pf);
	if (rhost != NULL)
		fprintf(pf, "Subject: files updated by rdist from %s to %s\n",
			host, rhost);
	else
#if defined(NLS) || defined(KJI)
		fprintf(pf, "Subject: files updated after %s\n", NLctime(&lmod));
#else
		fprintf(pf, "Subject: files updated after %s\n", ctime(&lmod));
#endif
	putc('\n', pf);

	while ((len = read(fd, buf, BUFSIZ)) > 0)
		(void) fwrite(buf, (size_t)1, (size_t)len, pf);
	(void) close(fd);
	(void) pclose(pf);
}

/*
 * Return true if name is in the list.
 */
inlist(list, file)
	struct namelist *list;
	char *file;
{
	register struct namelist *nl;

	for (nl = list; nl != NULL; nl = nl->n_next)
		if (!strcmp(file, nl->n_name))
			return(1);
	return(0);
}

/*
 * Return TRUE if file is in the exception list.
 */
except(file)
	char *file;
{
	register struct	subcmd *sc;
	register struct	namelist *nl;

	if (debug)
		printf("except(%s)\n", file);

	for (sc = subcmds; sc != NULL; sc = sc->sc_next) {
		if (sc->sc_type != EXCEPT && sc->sc_type != PATTERN)
			continue;
		for (nl = sc->sc_args; nl != NULL; nl = nl->n_next) {
			if (sc->sc_type == EXCEPT) {
				if (!strcmp(file, nl->n_name))
					return(1);
				continue;
			}
			re_comp(nl->n_name);
			if (re_exec(file) > 0)
				return(1);
		}
	}
	return(0);
}

char *
colon(cp)
	register char *cp;
{

	while (*cp) {
		if (*cp == ':')
			return(cp);
		if (*cp == '/')
			return(0);
		cp++;
	}
	return(0);
}
