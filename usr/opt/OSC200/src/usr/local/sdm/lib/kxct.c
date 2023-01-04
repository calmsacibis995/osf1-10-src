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
static char	*sccsid = "@(#)$RCSfile: kxct.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:03:10 $";
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
/*
 * kxct - kerberos command execution
 */
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <varargs.h>

extern char *rindex();
extern char *salloc();
extern char *malloc();
extern char *getenv();
extern int errno;

#define SEND_OVER	1
#define WANT_BACK	2

static int sigsock;
static int datasock;
static u_char debug;

struct sockaddr_in peername;
struct sockaddr_in sockname;
char *hostname;
int sock;
u_short sigport;
u_short dataport;
u_long cksum;

void sendsig();

kxct(host, basedir, owner, tempmode, tempslot, dbg, argc, argv)
char *host;
char *basedir;
char *owner;
int tempmode;
int tempslot;
int dbg;
int argc;
char **argv;
{
    char *p;
    char *tempfile;
    char buf[BUFSIZ];
    int cc, rc;
    int child;
    int i;
    int len;
    int pid;
    int s;
    int status;
    struct hostent *hp;
    struct servent *sp;
    struct sockaddr_in sin;
    u_char byte;
    u_short port;
    union wait w;

    debug = (dbg != 0);
if (debug)
diag("kxct entered");

    /*
     * create main connection
     */
    (void) bzero((char *)&peername, sizeof(peername));
    peername.sin_port = htons(549);
if (debug)
diag("host lookup");
    hp = gethostbyname(host);
    if (hp == NULL) {
	fprintf(stderr, "Unknown host: %s\n", host);
	return(-1);
    }
if (debug)
diag("hostname %s", hp->h_name);
    hostname = salloc(hp->h_name);
    if (hostname == NULL) {
	fprintf(stderr, "salloc: %s", errmsg(-1));
	return(-1);
    }
    peername.sin_family = hp->h_addrtype;
    (void) bcopy((char *)hp->h_addr, (char *)&peername.sin_addr,
		 sizeof(hp->h_addr));
if (debug) diag("socket");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
	fprintf(stderr, "socket: %s\n", errmsg(-1));
	return(-1);
    }
if (debug) diag("connect");
    if (connect(sock, &peername, sizeof(peername)) < 0) {
	fprintf(stderr, "connect: %s\n", errmsg(-1));
	(void) close(sock);
	return(-1);
    }
    i = sizeof(sockname);
if (debug) diag("getsockname");
    if (getsockname(sock, (struct sockaddr *) &sockname, &i) < 0) {
	fprintf(stderr, "getsockname: %s\n", errmsg(-1));
	(void) close(sock);
	return(-1);
    }

    /*
     * indicate debugging option
     */
if (debug) diag("write debug");
    if (write(sock, (char *)&debug, sizeof(debug)) != sizeof(debug)) {
	fprintf(stderr, "write: %s\n", errmsg(-1));
	(void) close(sock);
	return(-1);
    }

    /*
     * create control connection
     */
if (debug) diag("socket control");
    sigsock = socket(AF_INET, SOCK_STREAM, 0);
    if (sigsock < 0) {
	fprintf(stderr, "socket: %s\n", errmsg(-1));
	(void) close(sock);
	return(-1);
    }
    bcopy(&sockname, &sin, sizeof(sin));
    sin.sin_port = 0;
if (debug) diag("bind control");
    if (bind(sigsock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
	fprintf(stderr, "bind: %s\n", errmsg(-1));
	(void) close(sigsock);
	(void) close(sock);
	return(-1);
    }
    i = sizeof(sin);
if (debug) diag("getsockname control");
    if (getsockname(sigsock, &sin, &i) < 0) {
	fprintf(stderr, "getsockname: %s\n", errmsg(-1));
	(void) close(sigsock);
	(void) close(sock);
	return(-1);
    }
    port = sin.sin_port;
if (debug) diag("listen control");
    if (listen(sigsock, 1) < 0) {
	fprintf(stderr, "listen: %s\n", errmsg(-1));
	(void) close(sigsock);
	(void) close(sock);
	return(-1);
    }
if (debug) diag("write control port");
    if (write(sock, (char *)&port, sizeof(port)) != sizeof(port)) {
	fprintf(stderr, "write: %s\n", errmsg(-1));
	(void) close(sigsock);
	(void) close(sock);
	return(-1);
    }
if (debug) diag("read control port");
    if (read(sock, (char *)&sigport, sizeof(sigport)) != sizeof(sigport)) {
	fprintf(stderr, "client read1: %s\n", errmsg(-1));
	(void) close(sigsock);
	(void) close(sock);
	return(-1);
    }
if (debug) diag("accept control");
    if ((s = accept(sigsock, (struct sockaddr *)&sin, &i)) < 0) {
	fprintf(stderr, "accept: %s\n", errmsg(-1));
	(void) close(sigsock);
	(void) close(sock);
	return(-1);
    }
    (void) close(sigsock);
    sigsock = s;
    if (i != sizeof(sin)) {
	fprintf(stderr, "accept: len %d vs %d\n", i, sizeof(sin));
	(void) close(sigsock);
	(void) close(sock);
	return(-1);
    }
    if (bcmp(&sin.sin_addr, &peername.sin_addr, sizeof(peername.sin_addr))) {
	fprintf(stderr, "accept: connection from wrong host: %s\n",
		inet_ntoa(sin.sin_addr));
	(void) close(sigsock);
	(void) close(sock);
	return(-1);
    }
    if (sin.sin_port != sigport) {
	fprintf(stderr, "accept: connection from wrong port: %d vs %d\n",
		ntohs(sin.sin_port), ntohs(sigport));
	(void) close(sigsock);
	(void) close(sock);
	return(-1);
    }

    /*
     * setup child for work
     */
if (debug) diag("fork");
    pid = fork();
    if (pid == -1) {
	fprintf(stderr, "fork: %s\n", errmsg(-1));
	(void) close(sigsock);
	(void) close(sock);
	return(-1);
    }
    if (pid != 0) {			/* child does work, parent waits */
	(void) close(sock);
	(void) free(hostname);
	status = wait_for_child(pid);
if (debug)
diag("kxct exited");
	return(status);
    }
    (void) close(sigsock);

    /*
     * create data connection
     */
    datasock = socket(AF_INET, SOCK_STREAM, 0);
    if (datasock < 0) {
	fprintf(stderr, "socket: %s\n", errmsg(-1));
	(void) close(sock);
	exit(-1);
    }
    bcopy(&sockname, &sin, sizeof(sin));
    sin.sin_port = 0;
    if (bind(datasock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
	fprintf(stderr, "bind: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    i = sizeof(sin);
    if (getsockname(datasock, &sin, &i) < 0) {
	fprintf(stderr, "getsockname: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    port = sin.sin_port;
    if (listen(datasock, 1) < 0) {
	fprintf(stderr, "listen: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    if (write(sock, (char *)&port, sizeof(port)) != sizeof(port)) {
	fprintf(stderr, "write: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    errno = 0;
    if (read(sock, (char *)&dataport, sizeof(dataport)) != sizeof(dataport)) {
	fprintf(stderr, "client read2: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    if ((s = accept(datasock, (struct sockaddr *)&sin, &i)) < 0) {
	fprintf(stderr, "accept: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    (void) close(datasock);
    datasock = s;
    if (i != sizeof(sin)) {
	fprintf(stderr, "accept: len %d vs %d\n", i, sizeof(sin));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    if (bcmp(&sin.sin_addr, &peername.sin_addr, sizeof(peername.sin_addr))) {
	fprintf(stderr, "accept: connection from wrong host: %s\n",
		inet_ntoa(sin.sin_addr));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    if (sin.sin_port != dataport) {
	fprintf(stderr, "accept: connection from wrong port: %d vs %d\n",
		ntohs(sin.sin_port) != ntohs(dataport));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }

    /*
     * adjust argument list for tempslot
     */
    if (tempslot != 0) {
	tempfile = argv[tempslot];
	if ((p = rindex(tempfile, '/')) != NULL)
	    p++;
	else
	    p = tempfile;
	argv[tempslot] = salloc(p);
	if (argv[tempslot] == NULL) {
	    fprintf(stderr, "salloc: %s", errmsg(-1));
	    (void) close(datasock);
	    (void) close(sock);
	    exit(-1);
	}
    }

    /*
     * authenticate with server
     */
    cksum = 0;
    p = basedir;
    while (*p != '\0')
	cksum += ((unsigned)(*p++));
    p = owner;
    while (*p != '\0')
	cksum += ((unsigned)(*p++));
    for (i = 0; i < argc; i++) {
	p = argv[i];
	while (*p != '\0')
	    cksum += ((unsigned)(*p++));
    }

    /*
     * indicate that we have no secrets
     */
    byte = 0;
    if (write(sock, &byte, sizeof(byte)) != sizeof(byte)) {
	fprintf(stderr, "write: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }

    /*
     * send argument list description
     */
    send_arglist(argc, argv, basedir, owner, tempslot, tempmode);

    /*
     * send authentication
     */
    send_authentication();

    /*
     * send temporary file
     */
    if (tempslot != 0 && transmit_data(tempfile, tempmode) < 0) {
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }

    /*
     * wait for command to finish
     */
    wait_for_exit();

    /*
     * receive temporary file
     */
    if (tempslot != 0 && receive_data(tempfile, tempmode) < 0) {
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }

    /*
     * receive exit code
     */
    if (read(datasock, (char *)&status, sizeof(status)) != sizeof(status))
	fprintf(stderr, "client read failed1: %s\n", errmsg(-1));
    status = ntohl((u_long)status);

    close(datasock);
    close(sock);

    exit(status);
    /*NOTREACHED*/
}

wait_for_child(pid)
int pid;
{
    char inbuf[BUFSIZ];
    int i;
    int readfrom, writeto, rfds, wfds;
    char *bp;
    int cc, rc;
    int child;
    char buf[BUFSIZ];
    union wait w;

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	signal(SIGINT, sendsig);
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	signal(SIGQUIT, sendsig);
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
	signal(SIGTERM, sendsig);
    i = 1;
    ioctl(sigsock, FIONBIO, &i);
    readfrom = (1<<sigsock);
    for (;;) {
	rfds = readfrom;
	if ((rfds & (1<<sigsock)) == 0)
	    break;
	if (select(16, &rfds, 0, 0, 0) < 0) {
	    if (errno != EINTR) {
		(void) fprintf(stderr, "select: %s\n", errmsg(-1));
		(void) close(sigsock);
		(void) kill(pid, SIGKILL);
		return(-1);
	    }
	    continue;
	}
	if (rfds & (1<<sigsock)) {
	    errno = 0;
	    cc = read(sigsock, buf, sizeof(buf));
	    if (cc <= 0) {
		if (errno != EWOULDBLOCK)
		    readfrom &= ~(1<<sigsock);
	    } else
		(void) write(2, buf, cc);
	}
    }
    (void) close(sigsock);
    do {
	child = wait(&w);
    } while (child != pid && child != -1);
    if (child == -1) {
	(void) fprintf(stderr, "wait: %s\n", errmsg(-1));
	return(-1);
    }
    if (WIFSIGNALED(w) || w.w_retcode == 0377)
	return(-1);
    return(w.w_retcode);
}

send_arglist(argc, argv, basedir, owner, tempslot, tempmode)
int argc;
char **argv;
char *basedir, *owner;
int tempslot, tempmode;
{
    int length;
    u_short count;
    int i;
    u_char byte;

    count = htons((u_short)(argc + 2));
    if (write(sock, (char *)&count, sizeof(count)) != sizeof(count)) {
	fprintf(stderr, "write argc error: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    length = strlen(basedir) + 1 + strlen(owner) + 1;
    for (i = 0; i < argc; i++)
	length += strlen(argv[i])+1;
    count = htons((u_short)length);
    if (write(sock, (char *)&count, sizeof(count)) != sizeof(count)) {
	fprintf(stderr, "write length error: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    count = htons((u_short)tempslot);
    if (write(sock, (char *)&count, sizeof(count)) != sizeof(count)) {
	fprintf(stderr, "write tempslot error: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    if (tempslot != 0) {
	if (tempmode == 0) {
	    fprintf(stderr, "tempfile requested without file transfer\n");
	    (void) close(datasock);
	    (void) close(sock);
	    exit(-1);
	}
	byte = tempmode;
	if (write(sock, &byte, sizeof(byte)) != sizeof(byte)) {
	    fprintf(stderr, "write tempmode error: %s\n", errmsg(-1));
	    (void) close(datasock);
	    (void) close(sock);
	    exit(-1);
	}
    }
    cksum = htonl(cksum);
    if (write(sock, (char *)&cksum, sizeof(cksum)) != sizeof(cksum)) {
	fprintf(stderr, "write cksum error: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    length = strlen(basedir) + 1;
    if (write(sock, basedir, length) != length) {
	fprintf(stderr, "write base directory error: %s\n",
	     i, errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    length = strlen(owner) + 1;
    if (write(sock, owner, length) != length) {
	fprintf(stderr, "write owner error: %s\n",
	     i, errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    for (i = 0; i < argc; i++) {
	length = strlen(argv[i])+1;
	if (write(sock, argv[i], length) != length) {
	    fprintf(stderr, "write argv[%d] error: %s\n",
		 i, errmsg(-1));
	    (void) close(datasock);
	    (void) close(sock);
	    exit(-1);
	}
    }
}

send_authentication()
{
    u_short count;
    int len;
    u_char byte;
    char *user;

    count = htons((u_short)getuid());
    if (write(sock, (char *)&count, sizeof(count)) != sizeof(count)) {
	fprintf(stderr, "write userid error: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    count = ntohs(count);
    byte = 0;
    if (write(sock, &byte, sizeof(byte)) != sizeof(byte)) {
	fprintf(stderr, "write: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    if ((user = getenv("USER")) == NULL) {
	fprintf(stderr, "getenv: USER not defined\n");
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    len = strlen(user);
    byte = len;
    if (write(sock, &byte, sizeof(byte)) != sizeof(byte)) {
	fprintf(stderr, "write: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
    if (write(sock, user, len) != len) {
	fprintf(stderr, "write: %s\n", errmsg(-1));
	(void) close(datasock);
	(void) close(sock);
	exit(-1);
    }
}

wait_for_exit()
{
    char inbuf[BUFSIZ];
    int i;
    int readfrom, writeto, rfds, wfds;
    char *bp;
    int cc, rc;
    int child;
    char buf[BUFSIZ];

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	signal(SIGINT, sendsig);
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	signal(SIGQUIT, sendsig);
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
	signal(SIGTERM, sendsig);
    i = 1;
    ioctl(sock, FIONBIO, &i);
    readfrom = (1<<sock) | (1<<0);
    writeto = 0;
    bp = NULL;
    for (;;) {
	rfds = readfrom;
	wfds = writeto;
	if ((rfds & (1<<sock)) == 0 && wfds == 0)
	    break;
	if (select(16, &rfds, &wfds, 0, 0) < 0) {
	    if (errno != EINTR) {
		perror("select");
		exit(-1);
	    }
	    continue;
	}
	if (rfds & (1<<sock)) {
	    errno = 0;
	    cc = read(sock, buf, sizeof(buf));
	    if (cc <= 0) {
		if (errno != EWOULDBLOCK)
		    readfrom &= ~(1<<sock);
	    } else
		(void) write(1, buf, cc);
	}
	if (wfds & (1<<sock)) {
	    cc = write(sock, bp, rc);
	    if (cc < 0) {
		if (errno == EWOULDBLOCK)
		    continue;
		readfrom &= ~(1<<0);
		writeto &= ~(1<<sock);
		(void) shutdown(sock, 1);
		continue;
	    }
	    rc -= cc; bp += cc;
	    if (rc != 0)
		continue;
	    writeto &= ~(1<<sock);
	    readfrom |= (1<<0);
	    bp = NULL;
	}
	if (bp != NULL)
	    continue;
	if (rfds & (1<<0)) {
	    errno = 0;
	    rc = read(0, inbuf, sizeof(inbuf));
	    if (rc <= 0) {
		if (errno != EWOULDBLOCK) {
		    readfrom &= ~(1<<0);
		    writeto &= ~(1<<sock);
		    (void) shutdown(sock, 1);
		    continue;
		}
		continue;
	    }
	    bp = inbuf;
	    readfrom &= ~(1<<0);
	    writeto |= (1<<sock);
	}
    }
}

void
sendsig(signo)
int signo;
{
    char sig = (char) signo;

    (void) write(sigsock, &sig, 1);
}

transmit_data(file, mode)
char *file;
int mode;
{
    u_long count;
    u_short perm;
    char *buffer;
    int bufsize;
    int fd;
    struct stat st;
    int len, len2;
    char exists;

    if (lstat(file, &st) < 0) {
	if (errno != ENOENT) {
	    fprintf(stderr, "lstat %s: %s\n", file, errmsg(-1));
	    return(-1);
	}
	if ((mode&SEND_OVER) == 0)
	    return(0);
	exists = 0;
    } else {
	if ((mode&SEND_OVER) == 0) {
	    fprintf(stderr, "file %s exists and would be overwritten\n", file);
	    return(-1);
	}
	if ((st.st_mode&S_IFMT) != S_IFREG) {
	    fprintf(stderr, "%s not a regular file\n");
	    return(-1);
	}
	exists = 1;
    }
    if (write(datasock, (char *)&exists, sizeof(exists)) != sizeof(exists)) {
	fprintf(stderr, "write failed: %s", errmsg(-1));
	return(-1);
    }
    if (!exists)
	return(0);
    perm = st.st_mode&0777;
    perm = htons(perm);
    if (write(datasock, (char *)&perm, sizeof(perm)) != sizeof(perm)) {
	fprintf(stderr, "write failed: %s", errmsg(-1));
	return(-1);
    }
    count = st.st_size;
    count = htonl(count);
    if (write(datasock, (char *)&count, sizeof(count)) != sizeof(count)) {
	fprintf(stderr, "write failed: %s", errmsg(-1));
	return(-1);
    }
    count = st.st_size;
    if (count == 0)
	return(0);
    if ((fd = open(file, O_RDONLY, 0)) < 0) {
	fprintf(stderr, "open failed: %s\n", errmsg(-1));
	return(-1);
    }
    if ((bufsize = count) > 10*1024)
	buffer = malloc(bufsize = 10*1024);
    else
	buffer = malloc(bufsize);
    if (buffer == NULL) {
	fprintf(stderr, "malloc failed\n");
	(void) close(fd);
	return(-1);
    }
    while (count != 0) {
	if ((len = read(fd, buffer, bufsize)) <= 0) {
	    if (len == 0)
		fprintf(stderr, "premature EOF\n");
	    else
		fprintf(stderr, "client read3: %s\n", errmsg(-1));
	    (void) close(fd);
	    return(-1);
	}
	if ((len2 = write(datasock, buffer, len)) != len) {
	    fprintf(stderr, "write: %s\n", errmsg(-1));
	    (void) close(fd);
	    return(-1);
	}
	count -= len;
	if (count < bufsize)
	    bufsize = count;
    }
    if (close(fd) < 0) {
	fprintf(stderr, "close: %s\n", errmsg(-1));
	return(-1);
    }
    (void) free(buffer);
    return(0);
}

receive_data(file, mode)
char *file;
int mode;
{
    u_long count;
    u_short perm;
    char *buffer;
    int bufsize;
    int fd;
    int len, len2;
    int error;
    char exists;

    if (debug) {
	fprintf(stderr, "client: receiving data\n"); fflush(stderr);
    }
    if ((mode&WANT_BACK) == 0)
	return(0);
    if (read(datasock, (char *)&exists, sizeof(exists)) != sizeof(exists)) {
	fprintf(stderr, "client read failed2: %s\n", errmsg(-1));
	return(-1);
    }
    if (debug) {
	fprintf(stderr, "client: exists %d\n", exists); fflush(stderr);
    }
    if (!exists) {
	(void) unlink(file);
	return(0);
    }
    if (read(datasock, (char *)&perm, sizeof(perm)) != sizeof(perm)) {
	fprintf(stderr, "client read failed3: %s\n", errmsg(-1));
	return(-1);
    }
    perm = ntohs(perm);
    if (debug) {
	fprintf(stderr, "client: perm %#o\n", perm); fflush(stderr);
    }
    if (read(datasock, (char *)&count, sizeof(count)) != sizeof(count)) {
	fprintf(stderr, "client read failed4: %s\n", errmsg(-1));
	return(-1);
    }
    count = ntohl(count);
    if (debug) {
	fprintf(stderr, "client: count %d\n", count); fflush(stderr);
    }
    (void) chmod(file, (int)((perm&0777)|0200));
    if ((fd = open(file, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
	fprintf(stderr, "open failed: %s\n", errmsg(-1));
	return(-1);
    }
    if (fchmod(fd, (int)(perm&0777)) < 0) {
	fprintf(stderr, "fchmod failed: %s\n", errmsg(-1));
	return(-1);
    }
    if (count == 0) {
	(void) close(fd);
	return(0);
    }
    /*
     * should check for space on disk, but could be expensive and unreliable
     */
    if ((bufsize = count) > 10*1024)
	buffer = malloc(bufsize = 10*1024);
    else
	buffer = malloc(bufsize);
    if (buffer == NULL) {
	fprintf(stderr, "malloc failed\n");
	(void) close(fd);
	return(-1);
    }
    if (debug) {
	fprintf(stderr, "client: bufsize %d\n", bufsize); fflush(stderr);
    }
    while (count != 0) {
	if (debug) {
	    fprintf(stderr, "client: loop count %d\n", count); fflush(stderr);
	}
	if ((len = read(datasock, buffer, bufsize)) <= 0) {
	    if (len == 0)
		fprintf(stderr, "premature EOF\n");
	    else
		fprintf(stderr, "client read4: %s\n", errmsg(-1));
	    (void) close(fd);
	    return(-1);
	}
	if (debug) {
	    fprintf(stderr, "client: read %d bytes\n", len);
	}
	if ((len2 = write(fd, buffer, len)) != len) {
	    fprintf(stderr, "write: %s\n", errmsg(-1));
	    (void) close(fd);
	    return(-1);
	}
	if (debug) {
	    fprintf(stderr, "client: wrote %d bytes\n", len2); fflush(stderr);
	}
	count -= len;
	if (count < bufsize)
	    bufsize = count;
    }
    if (close(fd) < 0) {
	fprintf(stderr, "close: %s\n", errmsg(-1));
	return(-1);
    }
    (void) free(buffer);
    return(0);
}

/*VARARGS1*/
diag(fmt, va_alist)
char *fmt;
va_dcl
{
    char buf[BUFSIZ];
    char *ptr;
    va_list vargs;
    struct timeval tv;

    (void) gettimeofday(&tv, (struct timezone *)0);
    (void) sprintf(buf, "%d.%d= ", tv.tv_sec, tv.tv_usec);
    ptr = buf + strlen(buf);
    va_start(vargs);
    ptr += vsprintf(ptr, fmt, vargs);
    va_end(vargs);
    *ptr++ = '\n';
    *ptr = '\0';
    (void) fputs(buf, stderr);
    (void) fflush(stderr);
}
