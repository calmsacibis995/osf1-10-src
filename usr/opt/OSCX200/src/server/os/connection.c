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
/***********************************************************
Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: connection.c,v 1.149 92/10/20 09:29:05 rws Exp $ */
/*****************************************************************
 *  Stuff to create connections --- OS dependent
 *
 *      EstablishNewConnections, CreateWellKnownSockets, ResetWellKnownSockets,
 *      CloseDownConnection, CheckConnections, AddEnabledDevice,
 *	RemoveEnabledDevice, OnlyListToOneClient,
 *      ListenToAllClients,
 *
 *      (WaitForSomething is in its own file)
 *
 *      In this implementation, a client socket table is not kept.
 *      Instead, what would be the index into the table is just the
 *      file descriptor of the socket.  This won't work for if the
 *      socket ids aren't small nums (0 - 2^8)
 *
 *****************************************************************/

#include "X.h"
#include "Xproto.h"
#include <sys/param.h>
#include <errno.h>
#include "Xos.h"			/* for strings, file, time */
#include <sys/socket.h>

#include <signal.h>
#include <setjmp.h>

#ifdef hpux
#include <sys/utsname.h>
#include <sys/ioctl.h>
#endif

#ifdef AIXV3
#include <sys/ioctl.h>
#endif

#ifdef TCPCONN
# include <netinet/in.h>
# ifndef hpux
#  ifdef apollo
#   ifndef NO_TCP_H
#    include <netinet/tcp.h>
#   endif
#  else
#   include <netinet/tcp.h>
#  endif
# endif
#endif

#ifdef UNIXCONN
/*
 * sites should be careful to have separate /tmp directories for diskless nodes
 */
#include <sys/un.h>
#include <sys/stat.h>
static int unixDomainConnection = -1;
#endif

#include <stdio.h>
#include <sys/uio.h>
#include "osstruct.h"
#include "osdep.h"
#include "opaque.h"
#include "dixstruct.h"

#ifdef DNETCONN
#include <netdnet/dn.h>
#endif /* DNETCONN */

#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif

typedef long CCID;      /* mask of indices into client socket table */

#ifndef X_UNIX_PATH
#ifdef hpux
#define X_UNIX_DIR	"/usr/spool/sockets/X11"
#define X_UNIX_PATH	"/usr/spool/sockets/X11/"
#define OLD_UNIX_DIR	"/tmp/.X11-unix"
#else
#define X_UNIX_DIR	"/tmp/.X11-unix"
#define X_UNIX_PATH	"/tmp/.X11-unix/X"
#endif
#endif

extern char *display;		/* The display number */
int lastfdesc;			/* maximum file descriptor */

int WellKnownConnections;	/* Listener mask */
int EnabledDevices[mskcnt];	/* mask for input devices that are on */
int AllSockets[mskcnt];	/* select on this */
int AllClients[mskcnt];	/* available clients */
int LastSelectMask[mskcnt];	/* mask returned from last select call */
int ClientsWithInput[mskcnt];	/* clients with FULL requests in buffer */
int ClientsWriteBlocked[mskcnt];/* clients who cannot receive output */
int OutputPending[mskcnt];	/* clients with reply/event data ready to go */
int MaxClients = MAXSOCKS ;
int NConnBitArrays = mskcnt;
Bool NewOutputPending;		/* not yet attempted to write some new output */
Bool AnyClientsWriteBlocked;	/* true if some client blocked on write */

Bool RunFromSmartParent;	/* send SIGUSR1 to parent process */
Bool PartialNetwork;		/* continue even if unable to bind all addrs */
static int ParentProcess;

static Bool debug_conns = FALSE;

static int SavedAllClients[mskcnt];
static int SavedAllSockets[mskcnt];
static int SavedClientsWithInput[mskcnt];
int GrabInProgress = 0;

int ConnectionTranslation[MAXSOCKS];
extern int auditTrailLevel;
extern ClientPtr NextAvailableClient();

extern SIGVAL AutoResetServer();
extern SIGVAL GiveUp();
extern XID CheckAuthorization();
static void CloseDownFileDescriptor(), ErrorConnMax();
extern void FreeOsBuffers(), ResetOsBuffers();

#ifdef XDMCP
void XdmcpOpenDisplay(), XdmcpInit(), XdmcpReset(), XdmcpCloseDisplay();
#endif

#ifdef  XDPS
extern void XDPSPrivSetOverflow();
#endif

#ifdef TCPCONN
static int
open_tcp_socket ()
{
    struct sockaddr_in insock;
    int request;
    int retry;
#ifndef SO_DONTLINGER
#ifdef SO_LINGER
    static int linger[2] = { 0, 0 };
#endif /* SO_LINGER */
#endif /* SO_DONTLINGER */

#ifdef AIXV3
#ifndef FORCE_DISPLAY_NUM
    extern int AIXTCPSocket;
    if (AIXTCPSocket>=0) {
        request= AIXTCPSocket;
    } else
#endif /* FORCE_DISPLAY_NUM */
#endif /* AIX && etc. */

    if ((request = socket (AF_INET, SOCK_STREAM, 0)) < 0) 
    {
	Error ("Creating TCP socket");
	return -1;
    } 
#ifdef SO_REUSEADDR
    /* Necesary to restart the server without a reboot */
    {
	int one = 1;
	setsockopt(request, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
    }
#endif /* SO_REUSEADDR */
#ifdef AIXV3
#ifndef FORCE_DISPLAY_NUMBER
    if (AIXTCPSocket<0)
#endif
#endif
    {
    bzero ((char *)&insock, sizeof (insock));
    insock.sin_family = AF_INET;
    insock.sin_port = htons ((unsigned short)(X_TCP_PORT + atoi (display)));
    insock.sin_addr.s_addr = htonl(INADDR_ANY);
    retry = 20;
    while (bind(request, (struct sockaddr *) &insock, sizeof (insock)))
    {
	if (--retry == 0) {
	    Error ("Binding TCP socket");
	    close (request);
	    return -1;
	}
#ifdef SO_REUSEADDR
	sleep (1);
#else
	sleep (10);
#endif /* SO_REUSEDADDR */
    }
    }
#ifdef SO_DONTLINGER
    if(setsockopt (request, SOL_SOCKET, SO_DONTLINGER, (char *)NULL, 0))
	Error ("Setting TCP SO_DONTLINGER");
#else
#ifdef SO_LINGER
    if(setsockopt (request, SOL_SOCKET, SO_LINGER,
		   (char *)linger, sizeof(linger)))
	Error ("Setting TCP SO_LINGER");
#endif /* SO_LINGER */
#endif /* SO_DONTLINGER */
    if (listen (request, 5)) {
	Error ("TCP Listening");
	close (request);
	return -1;
    }
    return request;
}
#endif /* TCPCONN */

#ifdef UNIXCONN

static struct sockaddr_un unsock;

static int
open_unix_socket ()
{
    int oldUmask;
    int request;

    bzero ((char *) &unsock, sizeof (unsock));
    unsock.sun_family = AF_UNIX;
    oldUmask = umask (0);
#ifdef X_UNIX_DIR
    if (!mkdir (X_UNIX_DIR, 0777))
	chmod (X_UNIX_DIR, 0777);
#endif
    strcpy (unsock.sun_path, X_UNIX_PATH);
    strcat (unsock.sun_path, display);
#ifdef hpux
    {  
        /*    The following is for backwards compatibility
         *    with old HP clients. This old scheme predates the use
 	 *    of the /usr/spool/sockets directory, and uses hostname:display
 	 *    in the /tmp/.X11-unix directory
         */
        struct utsname systemName;
	static char oldLinkName[256];

        uname(&systemName);
        strcpy(oldLinkName, OLD_UNIX_DIR);
        if (!mkdir(oldLinkName, 0777))
	    chown(oldLinkName, 2, 3);
        strcat(oldLinkName, "/");
        strcat(oldLinkName, systemName.nodename);
        strcat(oldLinkName, display);
        unlink(oldLinkName);
        symlink(unsock.sun_path, oldLinkName);
    }
#endif	/* hpux */
    unlink (unsock.sun_path);
    if ((request = socket (AF_UNIX, SOCK_STREAM, 0)) < 0) 
    {
	Error ("Creating Unix socket");
	return -1;
    } 
    if (bind(request, (struct sockaddr *)&unsock, strlen(unsock.sun_path)+2))
    {
	Error ("Binding Unix socket");
	close (request);
	return -1;
    }
    if (listen (request, 5))
    {
	Error ("Unix Listening");
	close (request);
	return -1;
    }
    (void)umask(oldUmask);
    return request;
}
#endif /*UNIXCONN */

#ifdef hpux
/*
 * hpux returns EOPNOTSUPP when using getpeername on a unix-domain
 * socket.  In this case, smash the socket address with the address
 * used to bind the connection socket and return success.
 */
hpux_getpeername(fd, from, fromlen)
    int	fd;
    struct sockaddr *from;
    int		    *fromlen;
{
    int	    ret;
    int	    len;

    ret = getpeername(fd, from, fromlen);
    if (ret == -1 && errno == EOPNOTSUPP)
    {
	ret = 0;
	len = strlen(unsock.sun_path)+2;
	if (len > *fromlen)
	    len = *fromlen;
	bcopy ((char *) &unsock, (char *) from, len);
	*fromlen = len;
    }
    return ret;
}

#define getpeername(fd, from, fromlen)	hpux_getpeername(fd, from, fromlen)

#endif

#ifdef DNETCONN
static int
open_dnet_socket ()
{
    int request;
    struct sockaddr_dn dnsock;

    if ((request = socket (AF_DECnet, SOCK_STREAM, 0)) < 0) 
    {
	Error ("Creating DECnet socket");
	return -1;
    } 
    bzero ((char *)&dnsock, sizeof (dnsock));
    dnsock.sdn_family = AF_DECnet;
    sprintf(dnsock.sdn_objname, "X$X%d", atoi (display));
    dnsock.sdn_objnamel = strlen(dnsock.sdn_objname);
    if (bind (request, (struct sockaddr *) &dnsock, sizeof (dnsock)))
    {
	Error ("Binding DECnet socket");
	close (request);
	return -1;
    }
    if (listen (request, 5))
    {
	Error ("DECnet Listening");
	close (request);
	return -1;
    }
    return request;
}
#endif /* DNETCONN */

/*****************
 * CreateWellKnownSockets
 *    At initialization, create the sockets to listen on for new clients.
 *****************/

void
CreateWellKnownSockets()
{
    int		request, i;

    CLEARBITS(AllSockets);
    CLEARBITS(AllClients);
    CLEARBITS(LastSelectMask);
    CLEARBITS(ClientsWithInput);

    for (i=0; i<MAXSOCKS; i++) ConnectionTranslation[i] = 0;
    
#if defined(hpux) || defined(SVR4)
	lastfdesc = _NFILE - 1;
#else
	lastfdesc = getdtablesize() - 1;
#endif	/* hpux */

    if (lastfdesc > MAXSOCKS)
    {
	lastfdesc = MAXSOCKS;
	if (debug_conns)
	    ErrorF( "GOT TO END OF SOCKETS %d\n", MAXSOCKS);
    }

    WellKnownConnections = 0;
#ifdef TCPCONN
    if ((request = open_tcp_socket ()) != -1) {
	WellKnownConnections |= (1 << request);
	DefineSelf (request);
    }
    else if (!PartialNetwork) 
    {
	FatalError ("Cannot establish tcp listening socket");
    }
#endif /* TCPCONN */
#ifdef DNETCONN
    if ((request = open_dnet_socket ()) != -1) {
	WellKnownConnections |= (1 << request);
	DefineSelf (request);
    }
    else if (!PartialNetwork) 
    {
	FatalError ("Cannot establish dnet listening socket");
    }
#endif /* DNETCONN */
#ifdef UNIXCONN
    if ((request = open_unix_socket ()) != -1) {
	WellKnownConnections |= (1 << request);
	unixDomainConnection = request;
    }
    else if (!PartialNetwork) 
    {
	FatalError ("Cannot establish unix listening socket");
    }
#endif /* UNIXCONN */
    if (WellKnownConnections == 0)
        FatalError ("Cannot establish any listening sockets");
    signal (SIGPIPE, SIG_IGN);
    signal (SIGHUP, AutoResetServer);
    signal (SIGINT, GiveUp);
    signal (SIGTERM, GiveUp);
    AllSockets[0] = WellKnownConnections;
    ResetHosts(display);
    /*
     * Magic:  If SIGUSR1 was set to SIG_IGN when
     * the server started, assume that either
     *
     *  a- The parent process is ignoring SIGUSR1
     *
     * or
     *
     *  b- The parent process is expecting a SIGUSR1
     *     when the server is ready to accept connections
     *
     * In the first case, the signal will be harmless,
     * in the second case, the signal will be quite
     * useful
     */
    if (signal (SIGUSR1, SIG_IGN) == SIG_IGN)
	RunFromSmartParent = TRUE;
    ParentProcess = getppid ();
    if (RunFromSmartParent) {
	if (ParentProcess > 0) {
	    kill (ParentProcess, SIGUSR1);
	}
    }
#ifdef XDMCP
    XdmcpInit ();
#endif
}

void
ResetWellKnownSockets ()
{
    ResetOsBuffers();
#if defined(UNIXCONN) && !defined(SVR4)
    if (unixDomainConnection != -1)
    {
	/*
	 * see if the unix domain socket has disappeared
	 */
	struct stat	statb;

	if (stat (unsock.sun_path, &statb) == -1 ||
	    (statb.st_mode & S_IFMT) != S_IFSOCK)
	{
	    ErrorF ("Unix domain socket %s trashed, recreating\n",
	    	unsock.sun_path);
	    (void) unlink (unsock.sun_path);
	    (void) close (unixDomainConnection);
	    WellKnownConnections &= ~(1 << unixDomainConnection);
	    unixDomainConnection = open_unix_socket ();
	    if (unixDomainConnection != -1)
		WellKnownConnections |= (1 << unixDomainConnection);
	}
    }
#endif /* UNIXCONN */
    ResetAuthorization ();
    ResetHosts(display);
    /*
     * See above in CreateWellKnownSockets about SIGUSR1
     */
    if (RunFromSmartParent) {
	if (ParentProcess > 0) {
	    kill (ParentProcess, SIGUSR1);
	}
    }
    /*
     * restart XDMCP
     */
#ifdef XDMCP
    XdmcpReset ();
#endif
}

static void
AuthAudit (client, letin, saddr, len, proto_n, auth_proto)
    int client;
    Bool letin;
    struct sockaddr *saddr;
    int len;
    unsigned short proto_n;
    char *auth_proto;
{
    char addr[128];

    if (!len)
        strcpy(addr, "local host");
    else
	switch (saddr->sa_family)
	{
	case AF_UNSPEC:
#ifdef UNIXCONN
	case AF_UNIX:
#endif
	    strcpy(addr, "local host");
	    break;
#ifdef TCPCONN
	case AF_INET:
	    sprintf(addr, "IP %s port %d",
		    inet_ntoa(((struct sockaddr_in *) saddr)->sin_addr),
		    ((struct sockaddr_in *) saddr)->sin_port);
	    break;
#endif
#ifdef DNETCONN
	case AF_DECnet:
	    sprintf(addr, "DN %s",
		    dnet_ntoa(&((struct sockaddr_dn *) saddr)->sdn_add));
	    break;
#endif
	default:
	    strcpy(addr, "unknown address");
	}
    if (letin)
	AuditF("client %d connected from %s\n", client, addr);
    else
	AuditF("client %d rejected from %s\n", client, addr);
    if (proto_n)
	AuditF("  Auth name: %.*s\n", proto_n, auth_proto);
}

/*****************************************************************
 * ClientAuthorized
 *
 *    Sent by the client at connection setup:
 *                typedef struct _xConnClientPrefix {
 *                   CARD8	byteOrder;
 *                   BYTE	pad;
 *                   CARD16	majorVersion, minorVersion;
 *                   CARD16	nbytesAuthProto;    
 *                   CARD16	nbytesAuthString;   
 *                 } xConnClientPrefix;
 *
 *     	It is hoped that eventually one protocol will be agreed upon.  In the
 *        mean time, a server that implements a different protocol than the
 *        client expects, or a server that only implements the host-based
 *        mechanism, will simply ignore this information.
 *
 *****************************************************************/

char * 
ClientAuthorized(client, proto_n, auth_proto, string_n, auth_string)
    ClientPtr client;
    char *auth_proto, *auth_string;
    unsigned short proto_n, string_n;
{
    register OsCommPtr priv;
    union {
	struct sockaddr sa;
#ifdef UNIXCONN
	struct sockaddr_un un;
#endif /* UNIXCONN */
#ifdef TCPCONN
	struct sockaddr_in in;
#endif /* TCPCONN */
#ifdef DNETCONN
	struct sockaddr_dn dn;
#endif /* DNETCONN */
    } from;
    int	fromlen = sizeof (from);
    XID	 auth_id;

    auth_id = CheckAuthorization (proto_n, auth_proto,
				  string_n, auth_string);

    priv = (OsCommPtr)client->osPrivate;
    if (auth_id == (XID) ~0L)
    {
	if (getpeername (priv->fd, &from.sa, &fromlen) != -1)
	{
	    if (InvalidHost (&from.sa, fromlen))
		AuthAudit(client->index, FALSE, &from.sa, fromlen,
			  proto_n, auth_proto);
	    else
	    {
		auth_id = (XID) 0;
		if (auditTrailLevel > 1)
		    AuthAudit(client->index, TRUE, &from.sa, fromlen,
			      proto_n, auth_proto);
	    }
	}
	if (auth_id == (XID) ~0L)
	    return "Client is not authorized to connect to Server";
    }
    else if (auditTrailLevel > 1)
    {
	if (getpeername (priv->fd, &from.sa, &fromlen) != -1)
	    AuthAudit(client->index, TRUE, &from.sa, fromlen,
		      proto_n, auth_proto);
    }

    priv->auth_id = auth_id;
    priv->conn_time = 0;

#ifdef XDMCP
    /* indicate to Xdmcp protocol that we've opened new client */
    XdmcpOpenDisplay(priv->fd);
#endif /* XDMCP */
    /* At this point, if the client is authorized to change the access control
     * list, we should getpeername() information, and add the client to
     * the selfhosts list.  It's not really the host machine, but the
     * true purpose of the selfhosts list is to see who may change the
     * access control list.
     */
    return((char *)NULL);
}

/*****************
 * EstablishNewConnections
 *    If anyone is waiting on listened sockets, accept them.
 *    Returns a mask with indices of new clients.  Updates AllClients
 *    and AllSockets.
 *,
	clientsReadable[i]****************/

/*ARGSUSED*/
Bool
EstablishNewConnections(clientUnused, closure)
    ClientPtr clientUnused;
    pointer closure;
{
    int readyconnections;     /* mask of listeners that are ready */
    int curconn;                  /* fd of listener that's ready */
    register int newconn;         /* fd of new client */
#if LONG_BIT == 64
    int connect_time;
#else /* LONG_BIT == 32 */
    long connect_time;
#endif /* LONG_BIT */
    register int i;
    register ClientPtr client;
    register OsCommPtr oc;

#ifdef TCP_NODELAY
    union {
	struct sockaddr sa;
#ifdef UNIXCONN
	struct sockaddr_un un;
#endif /* UNIXCONN */
#ifdef TCPCONN
	struct sockaddr_in in;
#endif /* TCPCONN */
#ifdef DNETCONN
	struct sockaddr_dn dn;
#endif /* DNETCONN */
    } from;
    int	fromlen;
#endif /* TCP_NODELAY */

    readyconnections = (((int)closure) & WellKnownConnections);
    if (!readyconnections)
	return TRUE;
    connect_time = GetTimeInMillis();
    /* kill off stragglers */
    for (i=1; i<currentMaxClients; i++)
    {
	if (client = clients[i])
	{
	    oc = (OsCommPtr)(client->osPrivate);
	    if (oc && (oc->conn_time != 0) &&
		(connect_time - oc->conn_time) >= TimeOutValue)
		CloseDownClient(client);     
	}
    }
    while (readyconnections) 
    {
	curconn = ffs (readyconnections) - 1;
	readyconnections &= ~(1 << curconn);
	if ((newconn = accept (curconn,
			      (struct sockaddr *) NULL, 
			      (int *)NULL)) < 0) 
	    continue;
#ifdef TCP_NODELAY
	fromlen = sizeof (from);
	if (!getpeername (newconn, &from.sa, &fromlen))
	{
	    if (fromlen && (from.sa.sa_family == AF_INET)) 
	    {
		int mi = 1;
		setsockopt (newconn, IPPROTO_TCP, TCP_NODELAY,
			   (char *)&mi, sizeof (int));
	    }
	}
#endif /* TCP_NODELAY */
    /* ultrix reads hang on Unix sockets, hpux reads fail, AIX fails too */
#if defined(O_NONBLOCK) && (!defined(ultrix) && !defined(hpux) && !defined(AIXV3))
	(void) fcntl (newconn, F_SETFL, O_NONBLOCK);
#else
#ifdef FIOSNBIO
	{
	    int	arg;
	    arg = 1;
	    ioctl(newconn, FIOSNBIO, &arg);
	}
#else
#if defined(AIXV3) && defined(FIONBIO)
	{
	    int arg;
	    arg = 1;
	    ioctl(newconn, FIONBIO, &arg);
	}
#else
	fcntl (newconn, F_SETFL, FNDELAY);
#endif
#endif
#endif
	oc = (OsCommPtr)xalloc(sizeof(OsCommRec));
	if (!oc)
	{
	    ErrorConnMax(newconn);
	    close(newconn);
	    continue;
	}
	if (GrabInProgress)
	{
	    BITSET(SavedAllClients, newconn);
	    BITSET(SavedAllSockets, newconn);
	}
	else
	{
	    BITSET(AllClients, newconn);
	    BITSET(AllSockets, newconn);
	}
	oc->fd = newconn;
	oc->input = (ConnectionInputPtr)NULL;
	oc->output = (ConnectionOutputPtr)NULL;
	oc->conn_time = connect_time;
#ifdef SMT
        oc->pSmt = NULL;
#endif SMT
	if ((newconn < lastfdesc) &&
	    (client = NextAvailableClient((pointer)oc)))
	{
	    ConnectionTranslation[newconn] = client->index;
	}
	else
	{
	    ErrorConnMax(newconn);
	    CloseDownFileDescriptor(oc);
	}
    }
    return TRUE;
}

#define NOROOM "Maximum number of clients reached"

/************
 *   ErrorConnMax
 *     Fail a connection due to lack of client or file descriptor space
 ************/

static void
ErrorConnMax(fd)
    register int fd;
{
    xConnSetupPrefix csp;
    char pad[3];
    struct iovec iov[3];
    char byteOrder = 0;
    int whichbyte = 1;
    struct timeval waittime;
    int mask[mskcnt];

    /* if these seems like a lot of trouble to go to, it probably is */
    waittime.tv_sec = BOTIMEOUT / MILLI_PER_SECOND;
    waittime.tv_usec = (BOTIMEOUT % MILLI_PER_SECOND) *
		       (1000000 / MILLI_PER_SECOND);
    CLEARBITS(mask);
    BITSET(mask, fd);
    (void)select(fd + 1, (int *) mask, (int *) NULL, (int *) NULL, &waittime);
    /* try to read the byte-order of the connection */
    (void)read(fd, &byteOrder, 1);
    if ((byteOrder == 'l') || (byteOrder == 'B'))
    {
	csp.success = xFalse;
	csp.lengthReason = sizeof(NOROOM) - 1;
	csp.length = (sizeof(NOROOM) + 2) >> 2;
	csp.majorVersion = X_PROTOCOL;
	csp.minorVersion = X_PROTOCOL_REVISION;
	if (((*(char *) &whichbyte) && (byteOrder == 'B')) ||
	    (!(*(char *) &whichbyte) && (byteOrder == 'l')))
	{
	    swaps(&csp.majorVersion, whichbyte);
	    swaps(&csp.minorVersion, whichbyte);
	    swaps(&csp.length, whichbyte);
	}
	iov[0].iov_len = sz_xConnSetupPrefix;
	iov[0].iov_base = (char *) &csp;
	iov[1].iov_len = csp.lengthReason;
	iov[1].iov_base = NOROOM;
	iov[2].iov_len = (4 - (csp.lengthReason & 3)) & 3;
	iov[2].iov_base = pad;
	(void)writev(fd, iov, 3);
    }
}

/************
 *   CloseDownFileDescriptor:
 *     Remove this file descriptor and it's I/O buffers, etc.
 ************/

static void
CloseDownFileDescriptor(oc)
    register OsCommPtr oc;
{
    int connection = oc->fd;

    close(connection);
    FreeOsBuffers(oc);
    BITCLEAR(AllSockets, connection);
    BITCLEAR(AllClients, connection);
    BITCLEAR(ClientsWithInput, connection);
    if (GrabInProgress)
    {
	BITCLEAR(SavedAllSockets, connection);
	BITCLEAR(SavedAllClients, connection);
	BITCLEAR(SavedClientsWithInput, connection);
    }
    BITCLEAR(ClientsWriteBlocked, connection);
    if (!ANYSET(ClientsWriteBlocked))
    	AnyClientsWriteBlocked = FALSE;
    BITCLEAR(OutputPending, connection);
    xfree(oc);
}

/*****************
 * CheckConections
 *    Some connection has died, go find which one and shut it down 
 *    The file descriptor has been closed, but is still in AllClients.
 *    If would truly be wonderful if select() would put the bogus
 *    file descriptors in the exception mask, but nooooo.  So we have
 *    to check each and every socket individually.
 *****************/

void
CheckConnections()
{
    int			mask;
    int			tmask[mskcnt]; 
    register int	curclient, curoff;
    int			i;
    struct timeval	notime;
    int r;

    notime.tv_sec = 0;
    notime.tv_usec = 0;

    for (i=0; i<mskcnt; i++)
    {
	mask = AllClients[i];
        while (mask)
    	{
	    curoff = ffs (mask) - 1;
 	    curclient = curoff + (i << 5);
            CLEARBITS(tmask);
            BITSET(tmask, curclient);
            r = select (curclient + 1, (int *)tmask, (int *)NULL, (int *)NULL, 
			&notime);
            if (r < 0)
		CloseDownClient(clients[ConnectionTranslation[curclient]]);
	    mask &= ~(1 << curoff);
	}
    }	
}


/*****************
 * CloseDownConnection
 *    Delete client from AllClients and free resources 
 *****************/

void
CloseDownConnection(client)
    ClientPtr client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;

    if (oc->output && oc->output->count)
	FlushClient(client, oc, (char *)NULL, 0);
    ConnectionTranslation[oc->fd] = 0;
#ifdef XDMCP
    XdmcpCloseDisplay(oc->fd);
#endif
#ifdef  XDPS
    XDPSPrivSetOverflow(client, 0);
#endif
#ifdef SMT
    if (oc->pSmt != (smtControlBlockPtr)NULL) 
	SmtCloseDownConnection(client, oc);
#endif SMT
    CloseDownFileDescriptor(oc);
    client->osPrivate = (pointer)NULL;
    if (auditTrailLevel > 1)
	AuditF("client %d disconnected\n", client->index);
}


AddEnabledDevice(fd)
    int fd;
{
    BITSET(EnabledDevices, fd);
    BITSET(AllSockets, fd);
}


RemoveEnabledDevice(fd)
    int fd;
{
    BITCLEAR(EnabledDevices, fd);
    BITCLEAR(AllSockets, fd);
}

/*****************
 * OnlyListenToOneClient:
 *    Only accept requests from  one client.  Continue to handle new
 *    connections, but don't take any protocol requests from the new
 *    ones.  Note that if GrabInProgress is set, EstablishNewConnections
 *    needs to put new clients into SavedAllSockets and SavedAllClients.
 *    Note also that there is no timeout for this in the protocol.
 *    This routine is "undone" by ListenToAllClients()
 *****************/

OnlyListenToOneClient(client)
    ClientPtr client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    if (! GrabInProgress)
    {
	COPYBITS (ClientsWithInput, SavedClientsWithInput);
        BITCLEAR (SavedClientsWithInput, connection);
	if (GETBIT(ClientsWithInput, connection))
	{
	    CLEARBITS(ClientsWithInput);	    
	    BITSET(ClientsWithInput, connection);
	}
	else
        {
	    CLEARBITS(ClientsWithInput);	    
	}
	COPYBITS(AllSockets, SavedAllSockets);
	COPYBITS(AllClients, SavedAllClients);

	UNSETBITS(AllSockets, AllClients);
	BITSET(AllSockets, connection);
	CLEARBITS(AllClients);
	BITSET(AllClients, connection);
	GrabInProgress = client->index;
    }
}

/****************
 * ListenToAllClients:
 *    Undoes OnlyListentToOneClient()
 ****************/

ListenToAllClients()
{
    if (GrabInProgress)
    {
	ORBITS(AllSockets, AllSockets, SavedAllSockets);
	ORBITS(AllClients, AllClients, SavedAllClients);
	ORBITS(ClientsWithInput, ClientsWithInput, SavedClientsWithInput);
	GrabInProgress = 0;
    }	
}

/****************
 * IgnoreClient
 *    Removes one client from input masks.
 *    Must have cooresponding call to AttendClient.
 ****************/

static int IgnoredClientsWithInput[mskcnt];

IgnoreClient (client)
    ClientPtr	client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    if (!GrabInProgress || GrabInProgress == client->index)
    {
    	if (GETBIT (ClientsWithInput, connection))
	    BITSET(IgnoredClientsWithInput, connection);
    	else
	    BITCLEAR(IgnoredClientsWithInput, connection);
    	BITCLEAR(ClientsWithInput, connection);
    	BITCLEAR(AllSockets, connection);
    	BITCLEAR(AllClients, connection);
	BITCLEAR(LastSelectMask, connection);
    }
    else
    {
    	if (GETBIT (SavedClientsWithInput, connection))
	    BITSET(IgnoredClientsWithInput, connection);
    	else
	    BITCLEAR(IgnoredClientsWithInput, connection);
	BITCLEAR(SavedClientsWithInput, connection);
	BITCLEAR(SavedAllSockets, connection);
	BITCLEAR(SavedAllClients, connection);
    }
    isItTimeToYield = TRUE;
}

/****************
 * AttendClient
 *    Adds one client back into the input masks.
 ****************/

AttendClient (client)
    ClientPtr	client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    if (!GrabInProgress || GrabInProgress == client->index)
    {
    	BITSET(AllClients, connection);
    	BITSET(AllSockets, connection);
	BITSET(LastSelectMask, connection);
    	if (GETBIT (IgnoredClientsWithInput, connection))
	    BITSET(ClientsWithInput, connection);
    }
    else
    {
	BITSET(SavedAllClients, connection);
	BITSET(SavedAllSockets, connection);
	if (GETBIT(IgnoredClientsWithInput, connection))
	    BITSET(SavedClientsWithInput, connection);
    }
}

#ifdef AIXV3

static int grabbingClient;
static int reallyGrabbed;

/****************
* DontListenToAnybody:
*   Don't listen to requests from any clients. Continue to handle new
*   connections, but don't take any protocol requests from anybody.
*   We have to take care if there is already a grab in progress, though.
*   Undone by PayAttentionToClientsAgain. We also have to be careful
*   not to accept any more input from the currently dispatched client.
*   we do this be telling dispatch it is time to yield.

*   We call this when the server loses access to the glass
*   (user hot-keys away).  This looks like a grab by the 
*   server itself, but gets a little tricky if there is already
*   a grab in progress.
******************/

void
DontListenToAnybody()
{
    if (!GrabInProgress) {
	COPYBITS(ClientsWithInput, SavedClientsWithInput);
	COPYBITS(AllSockets, SavedAllSockets);
	COPYBITS(AllClients, SavedAllClients);
	GrabInProgress = TRUE;
	reallyGrabbed = FALSE;
    } else {
	grabbingClient = ((OsCommPtr)clients[GrabInProgress]->osPrivate)->fd;
	reallyGrabbed = TRUE;
    }
    CLEARBITS(ClientsWithInput);
    UNSETBITS(AllSockets, AllClients);
    CLEARBITS(AllClients);
    isItTimeToYield = TRUE;
}

void
PayAttentionToClientsAgain()
{
    if (reallyGrabbed) {
	BITSET(AllSockets, grabbingClient);
	BITSET(AllClients, grabbingClient);
    } else {
	ListenToAllClients();
    }
    reallyGrabbed = FALSE;
}

#endif
