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
static char	*sccsid = "@(#)$RCSfile: cmds.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 93/01/29 14:34:51 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * cmds.c	4.1 15:58:50 7/19/90 SecureWare 
 */

/*
#ifndef lint
static char sccsid[] = "cmds.c	5.4 (Berkeley) 6/30/88";
#endif 
*/
/*
 * lpc -- line printer control program -- commands:
 */

#include "lp.h"
#include <sys/time.h>
#include <locale.h>
#include "printer_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PRINTER,n,s)
#if SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_PRINTER_SEC,n,s)
#endif SEC_BASE

#include <NLchar.h>
#include <NLctype.h>

#if SEC_BASE
#include <sys/security.h>
#if SEC_MAC
#include <mandatory.h>
#endif

extern priv_t	*privvec();

extern uid_t	lp_uid;
extern gid_t	lp_gid;
#endif

/*
 * kill an existing daemon and disable printing.
 */
abort(argc, argv)
	char *argv[];
{
	register int c, status;
	register char *cp1, *cp2;
	char prbuf[100];

	if (argc == 1) {
		fprintf(stderr, MSGSTR(CMDS_1, "Usage: abort {all | printer ...}\n"));
		return(1);
	}
	if (argc == 2 && !strcmp(argv[1], "all")) {
		printer = prbuf;
		while (getprent(line) > 0) {
			cp1 = prbuf;
			cp2 = line;
			while ((c = *cp2++) && c != '|' && c != ':')
				*cp1++ = c;
			*cp1 = '\0';
			abortpr(1);
		}
		return(0);
	}
	while (--argc) {
		printer = *++argv;
		if ((status = pgetent(line, printer)) < 0) {
			fprintf(stderr, MSGSTR(CMDS_2, "cannot open printer description file\n"));
			continue;
		} else if (status == 0) {
			fprintf(stderr, MSGSTR(CMDS_3, "unknown printer %s\n"), printer);
			continue;
		}
		abortpr(1);
	}
	return(0);
}

abortpr(dis)
{
	register FILE *fp;
	struct stat stbuf;
	int pid, fd;
#if SEC_BASE
	privvec_t	saveprivs;
#endif

#if SEC_BASE
	if (forceprivs(privvec(SEC_OWNER, SEC_CHOWN, SEC_KILL,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpc");
		exit(1);
	}
#endif
	bp = pbuf;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	(void) sprintf(line, "%s/%s", SD, LO);
	printf("%s:\n", printer);

	/*
	 * Turn on the owner execute bit of the lock file to disable printing.
	 */
	if (dis) {
		if (stat(line, &stbuf) >= 0) {
			if (chmod(line, (stbuf.st_mode & 0777) | 0100) < 0)
				fprintf(stderr, MSGSTR(CMDS_4, "\tcannot disable printing\n"));
			else
				printf(MSGSTR(CMDS_5, "\tprinting disabled\n"));
		} else if (errno == ENOENT) {
			if ((fd = open(line, O_WRONLY|O_CREAT, 0760)) < 0)
				fprintf(stderr, MSGSTR(CMDS_6, "\tcannot create lock file\n"));
			else {
				(void) close(fd);
#if SEC_MAC
				chslabel(line, mand_syslo);
#endif
#if SEC_BASE
				chown(line, lp_uid, lp_gid);
#endif
				printf(MSGSTR(CMDS_5, "\tprinting disabled\n"));
				printf(MSGSTR(CMDS_7, "\tno daemon to abort\n"));
			}
#if SEC_BASE
			seteffprivs(saveprivs, (priv_t *) 0);
#endif
			return(0);
		} else {
			fprintf(stderr, MSGSTR(CMDS_8, "\tcannot stat lock file\n"));
#if SEC_BASE
			seteffprivs(saveprivs, (priv_t *) 0);
#endif
			return(1);
		}
	}
	/*
	 * Kill the current daemon to stop printing now.
	 */
	if ((fp = fopen(line, "r")) == NULL) {
		fprintf(stderr, MSGSTR(CMDS_9, "\tcannot open lock file\n"));
#if SEC_BASE
		seteffprivs(saveprivs, (priv_t *) 0);
#endif
		return(1);
	}
	if (!getline(fp) || flock(fileno(fp), LOCK_SH|LOCK_NB) == 0) {
		(void) fclose(fp);	/* unlocks as well */
		printf(MSGSTR(CMDS_7, "\tno daemon to abort\n"));
#if SEC_BASE
		seteffprivs(saveprivs, (priv_t *) 0);
#endif
		return(0);
	}
	/* need fp a little longer */
	if (kill(pid = atoi(line), SIGTERM) < 0)
		printf(MSGSTR(CMDS_10, "\tWarning: daemon (pid %d) not killed\n"), pid);
	else
		printf(MSGSTR(CMDS_11, "\tdaemon (pid %d) killed\n"), pid);
	if (flock(fileno(fp), LOCK_SH|LOCK_NB) != 0) {
		int i;
		printf(MSGSTR(CMDS_12,
			"\tWaiting for daemon (pid %d) to die...\n"),pid);
		for(i = 0;i < 10;i++)
		    if (flock(fileno(fp), LOCK_SH|LOCK_NB) == 0)
			break;
		    else
			sleep(1);
		/* see if it is still alive */
		if (flock(fileno(fp), LOCK_SH|LOCK_NB) != 0) {
		    /* kill it hard */
		    if (kill(pid = atoi(line), SIGKILL) < 0)
			printf(MSGSTR(CMDS_10,
				"\tWarning: daemon (pid %d) not killed\n"),
				pid);
		    else
			printf(MSGSTR(CMDS_11,
				"\tdaemon (pid %d) killed\n"), pid);
		}
	}
	(void) fclose(fp);
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	return(0);
}

/*
 * Remove all spool files and temporaries from the spooling area.
 */
clean(argc, argv)
	char *argv[];
{
	register int c, status;
	register char *cp1, *cp2;
	char prbuf[100];
#if SEC_MAC
	privvec_t	saveprivs;
#endif

	if (argc == 1) {
		fprintf(stderr, MSGSTR(CMDS_13,
			"Usage: clean {all | printer ...}\n"));
		return(1);
	}
#if SEC_MAC
	if (forceprivs(privvec(SEC_ALLOWMACACCESS, -1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpc");
		exit(1);
	}
#endif
	if (argc == 2 && !strcmp(argv[1], "all")) {
		printer = prbuf;
		while (getprent(line) > 0) {
			cp1 = prbuf;
			cp2 = line;
			while ((c = *cp2++) && c != '|' && c != ':')
				*cp1++ = c;
			*cp1 = '\0';
			cleanpr();
		}
#if SEC_MAC
		seteffprivs(saveprivs, (priv_t *) 0);
#endif
		return(0);
	}
	while (--argc) {
		printer = *++argv;
		if ((status = pgetent(line, printer)) < 0) {
			fprintf(stderr, MSGSTR(CMDS_2,
				"cannot open printer description file\n"));
			continue;
		} else if (status == 0) {
			fprintf(stderr, MSGSTR(CMDS_3,
				"unknown printer %s\n"), printer);
			continue;
		}
		cleanpr();
	}
#if SEC_MAC
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	return(0);
}

/* do not let this select() conflict with the system select call */
static
select(d)
struct dirent *d;
{
	int c = d->d_name[0];

	if ((c == 't' || c == 'c' || c == 'd') && d->d_name[1] == 'f')
		return(1);
	return(0);
}

/*
 * Comparison routine for scandir. Sort by job number and machine, then
 * by `cf', `tf', or `df', then by the sequence letter A-Z, a-z.
 */
sortq(d1, d2)
struct dirent **d1, **d2;
{
	int c1, c2;

	if (c1 = strcmp((*d1)->d_name + 3, (*d2)->d_name + 3))
		return(c1);
	c1 = (*d1)->d_name[0];
	c2 = (*d2)->d_name[0];
	if (c1 == c2)
		return((*d1)->d_name[2] - (*d2)->d_name[2]);
	if (c1 == 'c')
		return(-1);
	if (c1 == 'd' || c2 == 'c')
		return(1);
	return(-1);
}

/*
 * Remove incomplete jobs from spooling area.
 */
cleanpr()
{
	register int i, n;
	register char *cp, *cp1, *lp;
	struct dirent **queue;
	int nitems;

	bp = pbuf;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	printf("%s:\n", printer);

	for (lp = line, cp = SD; *lp++ = *cp++; )
		;
	lp[-1] = '/';

	/*
	 * line now contains spooling directory, followed by a slash, 
	 * while lp points to the next character position
	 */

	nitems = scandir(SD, &queue, select, sortq);
	if (nitems < 0) {
		fprintf(stderr, MSGSTR(CMDS_14, "\tcannot examine spool directory\n"));
		return(1);
	}
	if (nitems == 0)
		return(0);
	i = 0;
	do {
		cp = queue[i]->d_name;
		if ((*cp == 'c' || *cp == 'd' || *cp == 't') 
			&& *(cp+1) == 'f') {			/* 001 gray */
			strcpy(lp, cp);
			unlinkf(line);
		}

        } while (++i < nitems);
	return(0);
}
 
unlinkf(name)
	char	*name;
{
	if (unlink(name) < 0)
		fprintf(stderr, MSGSTR(CMDS_15, "\tcannot remove %s\n"), name);
	else
		printf(MSGSTR(CMDS_16, "\tremoved %s\n"), name);
}

/*
 * Enable queuing to the printer (allow lpr's).
 */
enable(argc, argv)
	char *argv[];
{
	register int c, status;
	register char *cp1, *cp2;
	char prbuf[100];

	if (argc == 1) {
		fprintf(stderr, MSGSTR(CMDS_17, "Usage: enable {all | printer ...}\n"));
		return(1);
	}
	if (argc == 2 && !strcmp(argv[1], "all")) {
		printer = prbuf;
		while (getprent(line) > 0) {
			cp1 = prbuf;
			cp2 = line;
			while ((c = *cp2++) && c != '|' && c != ':')
				*cp1++ = c;
			*cp1 = '\0';
			enablepr();
		}
		return(0);
	}
	while (--argc) {
		printer = *++argv;
		if ((status = pgetent(line, printer)) < 0) {
			fprintf(stderr, MSGSTR(CMDS_2, "cannot open printer description file\n"));
			continue;
		} else if (status == 0) {
			fprintf(stderr, MSGSTR(CMDS_3, "unknown printer %s\n"), printer);
			continue;
		}
		enablepr();
	}
	return(0);
}

enablepr()
{
	struct stat stbuf;
#if SEC_BASE
	privvec_t	saveprivs;
#endif

#if SEC_BASE
	if (forceprivs(privvec(SEC_OWNER, -1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpc");
		exit(1);
	}
#endif
	bp = pbuf;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	(void) sprintf(line, "%s/%s", SD, LO);
	printf("%s:\n", printer);

	/*
	 * Turn off the group execute bit of the lock file to enable queuing.
	 */
	if (stat(line, &stbuf) >= 0) {
		if (chmod(line, stbuf.st_mode & 0767) < 0)
			fprintf(stderr, MSGSTR(CMDS_18,
				"\tcannot enable queuing\n"));
		else
			printf(MSGSTR(CMDS_19, "\tqueuing enabled\n"));
	}
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	return(0);
}

/*
 * Disable queuing.
 */
disable(argc, argv)
	char *argv[];
{
	register int c, status;
	register char *cp1, *cp2;
	char prbuf[100];

	if (argc == 1) {
		fprintf(stderr, MSGSTR(CMDS_20, "Usage: disable {all | printer ...}\n"));
		return(1);
	}
	if (argc == 2 && !strcmp(argv[1], "all")) {
		printer = prbuf;
		while (getprent(line) > 0) {
			cp1 = prbuf;
			cp2 = line;
			while ((c = *cp2++) && c != '|' && c != ':')
				*cp1++ = c;
			*cp1 = '\0';
			disablepr();
		}
		return(0);
	}
	while (--argc) {
		printer = *++argv;
		if ((status = pgetent(line, printer)) < 0) {
			fprintf(stderr, MSGSTR(CMDS_2, "cannot open printer description file\n"));
			continue;
		} else if (status == 0) {
			fprintf(stderr, MSGSTR(CMDS_3, "unknown printer %s\n"), printer);
			continue;
		}
		disablepr();
	}
	return(0);
}

disablepr()
{
	register int fd;
	struct stat stbuf;
#if SEC_BASE
	privvec_t	saveprivs;
#endif

#if SEC_BASE
	if (forceprivs(privvec(SEC_OWNER, SEC_CHOWN,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpc");
		exit(1);
	}
#endif
	bp = pbuf;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	(void) sprintf(line, "%s/%s", SD, LO);
	printf("%s:\n", printer);
	/*
	 * Turn on the group execute bit of the lock file to disable queuing.
	 */
	if (stat(line, &stbuf) >= 0) {
		if (chmod(line, (stbuf.st_mode & 0777) | 010) < 0)
			fprintf(stderr, MSGSTR(CMDS_21,
				"\tcannot disable queuing\n"));
		else
			printf(MSGSTR(CMDS_22, "\tqueuing disabled\n"));
	} else if (errno == ENOENT) {
		if ((fd = open(line, O_WRONLY|O_CREAT, 0670)) < 0)
			fprintf(stderr, MSGSTR(CMDS_6,
				"\tcannot create lock file\n"));
		else {
			(void) close(fd);
#if SEC_MAC
			chslabel(line, mand_syslo);
#endif
#if SEC_BASE
			chown(line, lp_uid, lp_gid);
#endif
			printf(MSGSTR(CMDS_22, "\tqueuing disabled\n"));
		}
#if !SEC_BASE
		return(0);
#endif
	} else
		fprintf(stderr, MSGSTR(CMDS_8, "\tcannot stat lock file\n"));
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	return(0);
}

/*
 * Disable queuing and printing and put a message into the status file
 * (reason for being down).
 */
down(argc, argv)
	char *argv[];
{
	register int c, status;
	register char *cp1, *cp2;
	char prbuf[100];

	if (argc == 1) {
		fprintf(stderr, MSGSTR(CMDS_23, "Usage: down {all | printer} [message ...]\n"));
		return(1);
	}
	if (!strcmp(argv[1], "all")) {
		printer = prbuf;
		while (getprent(line) > 0) {
			cp1 = prbuf;
			cp2 = line;
			while ((c = *cp2++) && c != '|' && c != ':')
				*cp1++ = c;
			*cp1 = '\0';
			putmsg(argc - 2, argv + 2);
		}
		return(0);
	}
	printer = argv[1];
	if ((status = pgetent(line, printer)) < 0) {
		fprintf(stderr, MSGSTR(CMDS_2, "cannot open printer description file\n"));
		return(1);
	} else if (status == 0) {
		fprintf(stderr, MSGSTR(CMDS_3, "unknown printer %s\n"), printer);
		return(1);
	}
	putmsg(argc - 2, argv + 2);
	return(0);
}

putmsg(argc, argv)
	char **argv;
{
	register int fd;
	register char *cp1, *cp2;
	char buf[1024];
	struct stat stbuf;
#if SEC_BASE
	privvec_t	saveprivs;
#endif

#if SEC_BASE
	if (forceprivs(privvec(SEC_OWNER, SEC_CHOWN,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpc");
		exit(1);
	}
#endif
	bp = pbuf;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	if ((ST = pgetstr("st", &bp)) == NULL)
		ST = DEFSTAT;
	printf("%s:\n", printer);
	/*
	 * Turn on the group execute bit of the lock file to disable queuing and
	 * turn on the owner execute bit of the lock file to disable printing.
	 */
	(void) sprintf(line, "%s/%s", SD, LO);
	if (stat(line, &stbuf) >= 0) {
		if (chmod(line, (stbuf.st_mode & 0777) | 0110) < 0)
			fprintf(stderr, MSGSTR(CMDS_21,
				"\tcannot disable queuing\n"));
		else
			printf(MSGSTR(CMDS_24,
				"\tprinter and queuing disabled\n"));
	} else if (errno == ENOENT) {
		if ((fd = open(line, O_WRONLY|O_CREAT, 0770)) < 0)
			fprintf(stderr, MSGSTR(CMDS_6,
				"\tcannot create lock file\n"));
		else {
			(void) close(fd);
#if SEC_MAC
			chslabel(line, mand_syslo);
#endif
#if SEC_BASE
			chown(line, lp_uid, lp_gid);
#endif
			printf(MSGSTR(CMDS_24,
				"\tprinter and queuing disabled\n"));
		}
#if SEC_BASE
		seteffprivs(saveprivs, (priv_t *) 0);
#endif
		return(0);
	} else
		fprintf(stderr, MSGSTR(CMDS_8, "\tcannot stat lock file\n"));
	/*
	 * Write the message into the status file.
	 */
	(void) sprintf(line, "%s/%s", SD, ST);
#if SEC_BASE
	fd = open(line, O_WRONLY|O_CREAT|O_EXCL, 0664);
	if (fd >= 0) {
		close(fd);
#if SEC_MAC
		chslabel(line, mand_syslo);
#endif
		chown(line, lp_uid, lp_gid);
	}
	fd = open(line, O_WRONLY);
#else
	fd = open(line, O_WRONLY|O_CREAT, 0664);
#endif
	if (fd < 0 || flock(fd, LOCK_EX) < 0) {
		fprintf(stderr, MSGSTR(CMDS_25,
			"\tcannot create status file\n"));
#if SEC_BASE
		(void) close(fd);	/* Bug fix */
		seteffprivs(saveprivs, (priv_t *) 0);
#endif
		return(1);
	}
	(void) ftruncate(fd, 0);
	if (argc <= 0) {
		(void) write(fd, "\n", 1);
		(void) close(fd);
#if SEC_BASE
		seteffprivs(saveprivs, (priv_t *) 0);
#endif
		return(0);
	}
	cp1 = buf;
	while (--argc >= 0) {
		cp2 = *argv++;
		while (*cp1++ = *cp2++)
			;
		cp1[-1] = ' ';
	}
	cp1[-1] = '\n';
	*cp1 = '\0';
	(void) write(fd, buf, strlen(buf));
	(void) close(fd);
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
}

/*
 * Exit lpc
 */
quit(argc, argv)
	char *argv[];
{
	exit(0);
}

/*
 * Kill and restart the daemon.
 */
restart(argc, argv)
	char *argv[];
{
	register int c, status;
	register char *cp1, *cp2;
	char prbuf[100];

	if (argc == 1) {
		fprintf(stderr, MSGSTR(CMDS_26, "Usage: restart {all | printer ...}\n"));
		return(1);
	}
	if (argc == 2 && !strcmp(argv[1], "all")) {
		printer = prbuf;
		while (getprent(line) > 0) {
			cp1 = prbuf;
			cp2 = line;
			while ((c = *cp2++) && c != '|' && c != ':')
				*cp1++ = c;
			*cp1 = '\0';
			abortpr(0);
			startpr(0);
		}
		return(0);
	}
	while (--argc) {
		printer = *++argv;
		if ((status = pgetent(line, printer)) < 0) {
			fprintf(stderr, MSGSTR(CMDS_2, "cannot open printer description file\n"));
			continue;
		} else if (status == 0) {
			fprintf(stderr, MSGSTR(CMDS_3, "unknown printer %s\n"), printer);
			continue;
		}
		abortpr(0);
		startpr(0);
	}
	return(0);
}

/*
 * Enable printing on the specified printer and startup the daemon.
 */
start(argc, argv)
	char *argv[];
{
	register int c, status;
	register char *cp1, *cp2;
	char prbuf[100];

	if (argc == 1) {
		fprintf(stderr, MSGSTR(CMDS_27, "Usage: start {all | printer ...}\n"));
		return(1);
	}
	if (argc == 2 && !strcmp(argv[1], "all")) {
		printer = prbuf;
		while (getprent(line) > 0) {
			cp1 = prbuf;
			cp2 = line;
			while ((c = *cp2++) && c != '|' && c != ':')
				*cp1++ = c;
			*cp1 = '\0';
			startpr(1);
		}
		return(0);
	}
	while (--argc) {
		printer = *++argv;
		if ((status = pgetent(line, printer)) < 0) {
			fprintf(stderr, MSGSTR(CMDS_2, "cannot open printer description file\n"));
			continue;
		} else if (status == 0) {
			fprintf(stderr, MSGSTR(CMDS_3, "unknown printer %s\n"), printer);
			continue;
		}
		startpr(1);
	}
	return(0);
}

startpr(enable)
{
	struct stat stbuf;
#if SEC_BASE
	privvec_t	saveprivs;
#endif

#if SEC_BASE
	if (forceprivs(privvec(SEC_OWNER, SEC_REMOTE, -1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpc");
		exit(1);
	}
#endif
	bp = pbuf;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	(void) sprintf(line, "%s/%s", SD, LO);
	printf("%s:\n", printer);

	/*
	 * Turn off the owner execute bit of the lock file to enable printing.
	 */
	if (enable && stat(line, &stbuf) >= 0) 
	{
	    if (chmod(line, stbuf.st_mode & (enable==2 ? 0666 : 0677)) < 0)
		fprintf(stderr, MSGSTR(CMDS_28,	"\tcannot enable printing\n"));
	    else
	    {
		if (enable == 2)
		    printf(MSGSTR(CMDS_19, "\tqueuing enabled\n"));
		printf(MSGSTR(CMDS_29, "\tprinting enabled\n"));
	    }
	}
	if (!startdaemon(printer))
		fprintf(stderr, MSGSTR(CMDS_30, "\tcouldn't start daemon\n"));
	else
		printf(MSGSTR(CMDS_31, "\tdaemon started\n"));
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	return(0);
}

/*
 * Print the status of each queue listed or all the queues.
 */
status(argc, argv)
	char *argv[];
{
	register int c, status;
	register char *cp1, *cp2;
	char prbuf[100];

	if (argc == 1) {
		printer = prbuf;
		while (getprent(line) > 0) {
			cp1 = prbuf;
			cp2 = line;
			while ((c = *cp2++) && c != '|' && c != ':')
				*cp1++ = c;
			*cp1 = '\0';
			prstat();
		}
		return(0);
	}
	while (--argc) {
		printer = *++argv;
		if ((status = pgetent(line, printer)) < 0) {
			fprintf(stderr, MSGSTR(CMDS_2, "cannot open printer description file\n"));
			continue;
		} else if (status == 0) {
			fprintf(stderr, MSGSTR(CMDS_3, "unknown printer %s\n"), printer);
			continue;
		}
		prstat();
	}
	return(0);
}

/*
 * Print the status of the printer queue.
 */
prstat()
{
	struct stat stbuf;
	register int fd, i;
	register struct dirent *dp;
	DIR *dirp;

	bp = pbuf;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	if ((ST = pgetstr("st", &bp)) == NULL)
		ST = DEFSTAT;
	/* get baud rate */
	BR = pgetnum("br");
	if ((LP = pgetstr("lp", &bp)) == NULL)
		LP = DEFDEVLP;
	if ((RP = pgetstr("rp", &bp)) == NULL)
		RP = DEFLP;
	RM = pgetstr("rm", &bp);
	/*
	 * Figure out whether the local machine is the same as the remote 
	 * machine entry (if it exists).  If not, then ignore the local
	 * queue information.
	 */
	 if (RM != (char *) NULL) {
		char name[256];
		struct hostent *hp;

		/* get the standard network name of the local host */
 		gethostname(name, sizeof(name));
		name[sizeof(name)-1] = '\0';
		hp = gethostbyname(name);
		if (hp == (struct hostent *) NULL) {
		    syslog(LOG_ERR,
			MSGSTR(CMDS_32, "%s: unable to get network name for local machine %s"),
			printer, name);
		    goto localcheck_done;
		} else strcpy(name, hp->h_name);

		/* get the standard network name of RM */
		hp = gethostbyname(RM);
		if (hp == (struct hostent *) NULL) {
		    syslog(LOG_ERR,
			MSGSTR(CMDS_33, "%s: unable to get hostname for remote machine %s"), printer, RM);
		    goto localcheck_done;
		}

		/* if printer is not on local machine, ignore LP */
		if (strcmp(name, hp->h_name) != 0) *LP = '\0';
	}
localcheck_done:
	printf("%s:\n", printer);
	if (*LP)
		printf(MSGSTR(CMDS_34, "\tprinter is on device '%s' speed %d\n"), LP, BR);
	else
		printf(MSGSTR(CMDS_35, "\tprinter is on remote host %s with name %s\n"),
		       RM, RP);
	(void) sprintf(line, "%s/%s", SD, LO);
	if (stat(line, &stbuf) >= 0) {
		printf(MSGSTR(CMDS_36, "\tqueuing is %s\n"),
			(stbuf.st_mode & 010) ? MSGSTR(CMDS_37, "disabled") : MSGSTR(CMDS_38, "enabled"));
		printf(MSGSTR(CMDS_39, "\tprinting is %s\n"),
			(stbuf.st_mode & 0100) ? MSGSTR(CMDS_37, "disabled") : MSGSTR(CMDS_38, "enabled"));
	} else {
		printf(MSGSTR(CMDS_40, "\tqueuing is enabled\n"));
		printf(MSGSTR(CMDS_41, "\tprinting is enabled\n"));
	}
	if ((dirp = opendir(SD)) == NULL) {
		fprintf(stderr, MSGSTR(CMDS_14, "\tcannot examine spool directory\n"));
		return(1);
	}
	i = 0;
	while ((dp = readdir(dirp)) != NULL) {
		if (*dp->d_name == 'c' && dp->d_name[1] == 'f')
			i++;
	}
	closedir(dirp);
	if (i == 0)
		printf(MSGSTR(CMDS_42, "\tno entries\n"));
	else if (i == 1)
		printf(MSGSTR(CMDS_43, "\t1 entry in spool area\n"));
	else
		printf(MSGSTR(CMDS_44, "\t%d entries in spool area\n"), i);
	fd = open(line, O_RDONLY);
	if (fd < 0 || flock(fd, LOCK_SH|LOCK_NB) == 0) {
		(void) close(fd);	/* unlocks as well */
		printf(MSGSTR(CMDS_45, "\tno daemon present\n"));
		return(0);
	}
	(void) close(fd);
	putchar('\t');
	(void) sprintf(line, "%s/%s", SD, ST);
	fd = open(line, O_RDONLY);
	if (fd >= 0) {
		(void) flock(fd, LOCK_SH);
		while ((i = read(fd, line, sizeof(line))) > 0)
			(void) fwrite(line, 1, i, stdout);
		(void) close(fd);	/* unlocks as well */
	}
	return(0);
}

/*
 * Stop the specified daemon after completing the current job and disable
 * printing.
 */
stop(argc, argv)
	char *argv[];
{
	register int c, status;
	register char *cp1, *cp2;
	char prbuf[100];

	if (argc == 1) {
		fprintf(stderr, MSGSTR(CMDS_46, "Usage: stop {all | printer ...}\n"));
		return(1);
	}
	if (argc == 2 && !strcmp(argv[1], "all")) {
		printer = prbuf;
		while (getprent(line) > 0) {
			cp1 = prbuf;
			cp2 = line;
			while ((c = *cp2++) && c != '|' && c != ':')
				*cp1++ = c;
			*cp1 = '\0';
			stoppr();
		}
		return(0);
	}
	while (--argc) {
		printer = *++argv;
		if ((status = pgetent(line, printer)) < 0) {
			fprintf(stderr, MSGSTR(CMDS_2, "cannot open printer description file\n"));
			continue;
		} else if (status == 0) {
			fprintf(stderr, MSGSTR(CMDS_3, "unknown printer %s\n"), printer);
			continue;
		}
		stoppr();
	}
	return(0);
}

stoppr()
{
	register int fd;
	struct stat stbuf;
#if SEC_BASE
	privvec_t	saveprivs;
#endif

#if SEC_BASE
	if (forceprivs(privvec(SEC_OWNER, SEC_CHOWN,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpc");
		exit(1);
	}
#endif
	bp = pbuf;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	(void) sprintf(line, "%s/%s", SD, LO);
	printf("%s:\n", printer);

	/*
	 * Turn on the owner execute bit of the lock file to disable printing.
	 */
	if (stat(line, &stbuf) >= 0) {
		if (chmod(line, (stbuf.st_mode & 0777) | 0100) < 0)
			fprintf(stderr, MSGSTR(CMDS_4,
				"\tcannot disable printing\n"));
		else
			printf(MSGSTR(CMDS_5, "\tprinting disabled\n"));
	} else if (errno == ENOENT) {
		if ((fd = open(line, O_WRONLY|O_CREAT, 0760)) < 0)
			fprintf(stderr, MSGSTR(CMDS_6, "\tcannot create lock file\n"));
		else {
			(void) close(fd);
#if SEC_MAC
			chslabel(line, mand_syslo);
#endif
#if SEC_BASE
			chown(line, lp_uid, lp_gid);
#endif
			printf(MSGSTR(CMDS_5, "\tprinting disabled\n"));
		}
	} else
		fprintf(stderr, MSGSTR(CMDS_8, "\tcannot stat lock file\n"));
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	return(0);
}

struct	queue **queue;
int	nitems;
time_t	mtime;

/*
 * Put the specified jobs at the top of printer queue.
 */
topq(argc, argv)
	char *argv[];
{
	register int n, i;
	struct stat stbuf;
	register char *cfname;
	int status, changed;

	if (argc < 3) {
		fprintf(stderr, MSGSTR(CMDS_47, "Usage: topq printer [jobnum ...] [user ...]\n"));
		return(1);
	}

	--argc;
	printer = *++argv;
	status = pgetent(line, printer);
	if (status < 0) {
		fprintf(stderr, MSGSTR(CMDS_2, "cannot open printer description file\n"));
		return(1);
	} else if (status == 0) {
		fprintf(stderr, MSGSTR(CMDS_48, "%s: unknown printer\n"), printer);
		return(1);
	}
	bp = pbuf;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	printf("%s:\n", printer);

	if (chdir(SD) < 0) {
		fprintf(stderr, MSGSTR(CMDS_49, "\tcannot chdir to %s\n"), SD);
		return(1);
	}
	nitems = getq(&queue);
	if (nitems == 0)
		return(0);
	changed = 0;
	mtime = queue[0]->q_time;
	for (i = argc; --i; ) {
		if (doarg(argv[i]) == 0) {
			printf(MSGSTR(CMDS_50, "\tjob %s is not in the queue\n"), argv[i]);
			continue;
		} else
			changed++;
	}
	for (i = 0; i < nitems; i++)
		free(queue[i]);
	free(queue);
	if (!changed) {
		printf(MSGSTR(CMDS_51, "\tqueue order unchanged\n"));
		return(0);
	}
	/*
	 * Turn on the public execute bit of the lock file to
	 * get lpd to rebuild the queue after the current job.
	 */
#if SEC_BASE
	if (changed && stat(LO, &stbuf) >= 0) {
		privvec_t	saveprivs;

		forceprivs(privvec(SEC_OWNER, -1), saveprivs);
		(void) chmod(LO, (stbuf.st_mode & 0777) | 01);
		seteffprivs(saveprivs, (priv_t *) 0);
	}
#else
	if (changed && stat(LO, &stbuf) >= 0)
		(void) chmod(LO, (stbuf.st_mode & 0777) | 01);
#endif
	return(0);
} 

/*
 * Reposition the job by changing the modification time of
 * the control file.
 */
touch(q)
	struct queue *q;
{
	struct timeval tvp[2];

	tvp[0].tv_sec = tvp[1].tv_sec = --mtime;
	tvp[0].tv_usec = tvp[1].tv_usec = 0;
	return(utimes(q->q_name, tvp));
}

/*
 * Checks if specified job name is in the printer's queue.
 * Returns:  negative (-1) if argument name is not in the queue.
 */
doarg(job)
	char *job;
{
	register struct queue **qq;
	register int jobnum, n;
	register char *cp, *machine;
	int cnt = 0;
	FILE *fp;
#if SEC_BASE
	privvec_t	saveprivs;
#endif

	/*
	 * Look for a job item consisting of system name, colon, number 
	 * (example: ucbarpa:114)  
	 */
	if ((cp = index(job, ':')) != NULL) {
		machine = job;
		*cp++ = '\0';
		job = cp;
	} else
		machine = NULL;

#if SEC_BASE
	if (forceprivs(privvec(SEC_OWNER,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpc");
		exit(1);
	}
#endif
	/*
	 * Check for job specified by number (example: 112 or 235ucbarpa).
	 */
	if (isdigit(*job)) {
		jobnum = 0;
		do
			jobnum = jobnum * 10 + (*job++ - '0');
		while (isdigit(*job));
		for (qq = queue + nitems; --qq >= queue; ) {
			n = 0;
			cp = (*qq)->q_name+3;
			if (!isdigit(*cp))
			    cp++;
			for (; isdigit(*cp); )
				n = n * 10 + (*cp++ - '0');
			if (jobnum != n)
				continue;
			if (*job && strcmp(job, cp) != 0)
				continue;
			if (machine != NULL && strcmp(machine, cp) != 0)
				continue;
			if (touch(*qq) == 0) {
				printf(MSGSTR(CMDS_52, "\tmoved %s\n"), (*qq)->q_name);
				cnt++;
			}
		}
#if SEC_BASE
		seteffprivs(saveprivs, (priv_t *) 0);
#endif
		return(cnt);
	}
	/*
	 * Process item consisting of owner's name (example: henry).
	 */
	for (qq = queue + nitems; --qq >= queue; ) {
		if ((fp = fopen((*qq)->q_name, "r")) == NULL)
			continue;
		while (getline(fp) > 0)
			if (line[0] == 'P')
				break;
		(void) fclose(fp);
		if (line[0] != 'P' || strcmp(job, line+1) != 0)
			continue;
		if (touch(*qq) == 0) {
			printf(MSGSTR(CMDS_52, "\tqueue order changed %s\n"), (*qq)->q_name);
			cnt++;
		}
	}
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	return(cnt);
}

/*
 * Enable everything and start printer (undo `down').
 */
up(argc, argv)
	char *argv[];
{
	register int c, status;
	register char *cp1, *cp2;
	char prbuf[100];

	if (argc == 1) {
		fprintf(stderr,
			MSGSTR(CMDS_53, "Usage: up {all | printer ...}\n"));
		return(1);
	}
	if (argc == 2 && !strcmp(argv[1], "all")) {
		printer = prbuf;
		while (getprent(line) > 0) {
			cp1 = prbuf;
			cp2 = line;
			while ((c = *cp2++) && c != '|' && c != ':')
				*cp1++ = c;
			*cp1 = '\0';
			startpr(2);
		}
		return(0);
	}
	while (--argc) {
		printer = *++argv;
		if ((status = pgetent(line, printer)) < 0) {
			fprintf(stderr, MSGSTR(CMDS_2,
				"cannot open printer description file\n"));
			continue;
		} else if (status == 0) {
			fprintf(stderr, MSGSTR(CMDS_3,
				"unknown printer %s\n"), printer);
			continue;
		}
		startpr(2);
	}
	return(0);
}
