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
static char	*sccsid = "@(#)$RCSfile: rsh.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/11 19:06:20 $";
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

*/
/* 
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 The Regents of the University of California.
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
 */
/*
#ifndef lint
char copyright[] =
" Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "rsh.c	5.21 (Berkeley) 5/15/90";
#endif  not lint */

#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include <netinet/in.h>
#include <netdb.h>

#include <pwd.h>
#include <stdio.h>
#include <errno.h>

#include <nl_types.h>
#include <locale.h>
#include "rsh_msg.h" 
#define MSGSTR(Num,Str) catgets(catd,MS_RSH,Num,Str)
nl_catd catd;

/*
 * rsh - remote shell
 */
/* VARARGS */
int	error();
int	rfd2;
char	*rindex(), *strcpy();


#define _PATH_RLOGIN	"/usr/bin/rlogin"
main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
        extern int optind;
        struct passwd *pw;
        struct servent *sp;
#ifdef 0
        long omask;
#else /* alpha */
        int omask;
#endif /* alpha */
	int argoff, asrsh, ch, dflag, nflag, one, pid, rem, uid;
        register char *p;
        char *args, *host, *user, *copyargs();
        void sendsig();

	setlocale(LC_ALL,"");
        catd = NLcatopen("rsh.cat",NL_CAT_LOCALE);
	
        argoff = asrsh = dflag = nflag = 0;
        one = 1;
        host = user = NULL;	

	/* if called as something other than "rsh", use it as the host name */
        if (p = rindex(argv[0], '/'))
                ++p;
        else
                p = argv[0];
        if (strcmp(p, "rsh"))
                host = p;
        else
                asrsh = 1;

        /* handle "rsh host flags" */
    	if (!host && argc > 2 && argv[1][0] != '-') {
                host = argv[1];
                argoff = 1;
        }
#define OPTIONS "8KLdel:nw"

	 while ((ch = getopt(argc - argoff, argv + argoff, OPTIONS)) != EOF)
                switch(ch) {
                case 'K':
			break;
		case 'L':       /* -8Lew are ignored to allow rlogin aliases */
                case 'e':
                case 'w':
                case '8':
                        break;
                case 'd':
                        dflag = 1;
                        break;
            	case 'l':
                        user = optarg;
                        break;
		case 'n':
                        nflag = 1;
                        break;
		case '?':
                default:
                        usage();
			exit(1);
                }
        optind += argoff;

        /* if haven't gotten a host yet, do so */
        if (!host && !(host = argv[optind++])) {
                usage();
                exit(1);
        }

        /* if no further arguments, must have been called as rlogin. */
        if (!argv[optind]) {
                if (asrsh)
			 *argv = "rlogin";
                execv(_PATH_RLOGIN, argv);
                (void)fprintf(stderr, MSGSTR( RSH_CANT_EXEC, "rsh: can't exec %s.\n"), _PATH_RLOGIN);
                exit(1);
	}

	argc -= optind;
        argv += optind;

        if (!(pw = getpwuid(uid = getuid()))) {
                (void)fprintf(stderr, MSGSTR( RSH_UNKNOWN_UID, 
			"rsh: unknown user id.\n"));
                exit(1);
        }
        if (!user)
                user = pw->pw_name;
	args = copyargs(argv);

        sp = NULL;

	if (sp == NULL)
                sp = getservbyname("shell", "tcp");
        if (sp == NULL) {
		(void)fprintf(stderr, MSGSTR(RSH_UNKSERVER, "rsh: shell/tcp: unknown service\n")); /*MSG*/
                exit(1);
        }

	rem = rcmd(&host, sp->s_port, pw->pw_name, user, args, &rfd2);
	if (rem < 0)
                exit(1);

        if (rfd2 < 0) {
                (void)fprintf(stderr, MSGSTR( RSH_NOSTDERR,
			"rsh: can't establish stderr.\n"));
                exit(1);
        }
	if (dflag) {
                if (setsockopt(rem, SOL_SOCKET, SO_DEBUG, &one,
                    sizeof(one)) < 0)
                        (void)fprintf(stderr, MSGSTR( RSH_SETSOCK, 
				"rsh: setsockopt: %s.\n"), strerror(errno));
                if (setsockopt(rfd2, SOL_SOCKET, SO_DEBUG, &one,
                    sizeof(one)) < 0)
                        (void)fprintf(stderr, MSGSTR( RSH_SETSOCK,
				"rsh: setsockopt: %s.\n"), strerror(errno));
        }

        (void)setuid(uid);
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGTERM));
        if (signal(SIGINT, SIG_IGN) != SIG_IGN)
                (void)signal(SIGINT, sendsig);
        if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
                (void)signal(SIGQUIT, sendsig);
        if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
                (void)signal(SIGTERM, sendsig);

        if (!nflag) {
                pid = fork();
                if (pid < 0) {
			(void)fprintf(stderr, MSGSTR(RSH_FORK,
                            "rsh: fork: %s.\n"), strerror(errno));
                        exit(1);
                }
        }

	{
                (void)ioctl(rfd2, FIONBIO, &one);
                (void)ioctl(rem, FIONBIO, &one);
        }

        talk(nflag, omask, pid, rem);

        if (!nflag)
                (void)kill(pid, SIGKILL);
        exit(0);
}
talk(nflag, omask, pid, rem)
        int nflag, pid;
#ifdef 0
        long omask;
#else /* alpha */
        int omask;
#endif /* alpha */
        register int rem;
{
        register int cc, wc;
        register char *bp;
        int readfrom, ready, rembits, nfds= 0;
        char buf[BUFSIZ];

	if (!nflag && pid == 0) {
                (void)close(rfd2);

reread:         errno = 0;
                if ((cc = read(0, buf, sizeof buf)) <= 0)
                        goto done;
                bp = buf;

rewrite:        rembits = 1 << rem;
                nfds = rem + 1 ;
                if (select(nfds, (fd_set *)0, (fd_set *)&rembits, (fd_set *)0, (struct timeval *)0) < 0) {
                        if (errno != EINTR) {
                                (void)fprintf(stderr, MSGSTR( RSH_SELECT,
					"rsh: select: %s.\n"), strerror(errno));
                      		exit(1);
                        }
                        goto rewrite;
                }
                if ((rembits & (1 << rem)) == 0)
                        goto rewrite;

		        wc = write(rem, bp, cc);
                if (wc < 0) {
                        if (errno == EWOULDBLOCK)
                                goto rewrite;
                        goto done;
                }
                bp += wc;
                cc -= wc;
                if (cc == 0)
                        goto reread;
                goto rewrite;
done:
                (void)shutdown(rem, 1);
                exit(0);
        }
	
	(void)sigsetmask(omask);
        readfrom = (1 << rfd2) | (1 << rem);	
	if (rfd2 > rem)
	 nfds = rfd2 + 1;
	else
	 nfds = rem + 1;
        do {
                ready = readfrom;
                if (select(nfds, (fd_set *)&ready, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0) {
                        if (errno != EINTR) {
                                (void)fprintf(stderr, MSGSTR( RSH_SELECT,
					"rsh: select: %s.\n"), strerror(errno));
                                exit(1);
                        }
			continue;
                }
                if (ready & (1 << rfd2)) {
                        errno = 0;
		 cc = read(rfd2, buf, sizeof buf);
                        if (cc <= 0) {
                                if (errno != EWOULDBLOCK)
                                        readfrom &= ~(1 << rfd2);
                        } else
                                (void)write(2, buf, cc);
   		}
		if (ready & (1 << rem)) {
                        errno = 0;
		cc = read(rem, buf, sizeof buf);
                        if (cc <= 0) {
                                if (errno != EWOULDBLOCK)
                                        readfrom &= ~(1 << rem);
                        } else
                                (void)write(1, buf, cc);
    		}
	} while(readfrom);
}
     
void
sendsig(signo)
	char signo;
{

	(void) write(rfd2, &signo, 1);
}

char *
copyargs(argv)
        char **argv;
{
        register int cc;
        register char **ap, *p;
        char *args, *malloc();

	cc = 0;
        for (ap = argv; *ap; ++ap)
                cc += strlen(*ap) + 1;
        if (!(args = malloc((u_int)cc))) {
                (void)fprintf(stderr, MSGSTR( RSH_ERROR,
			"rsh: %s.\n"), strerror(ENOMEM));
                exit(1);
	}
        for (p = args, ap = argv; *ap; ++ap) {
                (void)strcpy(p, *ap);
                for (p = strcpy(p, *ap); *p; ++p);
                if (ap[1])
                        *p++ = ' ';
        }
        return(args);
}

usage()
{
        (void)fprintf(stderr, MSGSTR( RSH_USAGE,
            "usage: rsh [-ndx]%s[-l login] host [command]\n")," ");
}
