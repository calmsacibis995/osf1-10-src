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
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
/*****************************************************************************
Copyright 1987, 1988, 1989, 1990, 1991 by Digital Equipment Corp., Maynard, MA

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*****************************************************************************/
#include "Xos.h"
#ifdef TCPCONN /* to rest of file */
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "xtraplib.h"
#include "xtraplibp.h"

#define RetryCount 10

#ifndef lint
static char SCCSID[] = "@(#)XEStreamInternet.c	1.9 - 90/09/18  ";
static char RCSID[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/lib/xtrap/XESTInet.c,v 1.1.2.2 92/02/06 11:57:26 Jim_Ludwig Exp $";
#endif

#ifdef MASSCOMP
#define IPPORT_USERRESERVED IPPORT_RESERVED
#endif MASSCOMP

#ifdef FUNCTION_PROTOS
int OpenInternetStream(XESTREAM *st, char *addr, CARD32 mode, int mask)
#else
int OpenInternetStream(st, addr, mode, mask)
    XESTREAM *st;
    char     *addr;
    CARD32   mode;
    int      mask;
#endif
{
    int status = True;
    register struct sockaddr_in *insock = &(st->u.D.insock);
    register INT32 *sd = &(st->u.D.sd);
    char host[32];
    char port_str[12];
    int i;

    /* Currently the "mode" qualifier is not used for Internet since
     * it really has no relevance.  If it ever does, it appropriate
     * conversion from "descriptor" types would be done here.
     */

    if (addr[0] == '\0')
    {   /* Attempt to find a port not in use */
        int sockport = IPPORT_USERRESERVED + getpid() % (1000L - RetryCount);

        /* Get the name of this host */
        (void)gethostname(host,sizeof(host));
        memset((char *)insock,0L,sizeof(struct sockaddr_in));
        insock->sin_family      = AF_INET;
        insock->sin_addr.s_addr = INADDR_ANY;

        if ((*sd = socket(AF_INET,SOCK_STREAM,0L)) >= 0L)
        {   /* Try RetryCount times to bind the Socket */
            for (i=0L; i<RetryCount;i++)
            {
                insock->sin_port = sockport+i;
                if (bind(*sd,(struct sockaddr *)insock,
                    sizeof(struct sockaddr_in)) == 0L)
                {
                    break;
                }
            }
            if (listen(*sd,8L) == 0L)
            {   /* encode return address in addr2 */
                sprintf(port_str, "%d", insock->sin_port);
                st->addr2_lgth = strlen(host) + strlen(port_str) + 2L;  
                if ((st->addr2 = (char *)malloc(st->addr2_lgth)) != NULL)
                {
                    strcpy(st->addr2, host);
                    strcpy(&(st->addr2[strlen(host)+1L]), port_str);
                }
                else
                {   /* no memory avail */
                    SIOErr(st, strerror(ENOMEM));
		    (void)close(*sd);
		    st->addr2 = NULL;
                    status = False;
                }
            }
            else
            {   /* listen failed */
                SIOErr(st, strerror(errno));
		(void)close(*sd);
		st->addr2 = NULL;
                status = False;
            }
        }
        else
        {   /* we failed */
            SIOErr(st, strerror(errno));
	    (void)close(*sd);
	    st->addr2 = NULL;
            status = False;
        }
    }
    else
    {   /* Request for TCP/IP client mode */
	st->addr2 = NULL;
        status = False; /* Not Yet Implemented */
    }

    return(status);
}

#ifdef FUNCTION_PROTOS
int CloseInternetStream(XESTREAM *st)
#else
int CloseInternetStream(st)
    XESTREAM *st;
#endif
{
    int status = True;

    if ((shutdown(st->u.D.fd)) || (close(st->u.D.fd)))
    {
        SIOErr(st, strerror(errno));
        status = False;
    }
    return(status);
}

#ifdef FUNCTION_PROTOS
int AcceptInternetStream(XESTREAM *st)
#else
int AcceptInternetStream(st)
    XESTREAM *st;
#endif
{
    int status = True;
    register struct _dsc *d = &(st->u.D);
    struct sockaddr addr;
    int fromlen=sizeof(addr);
    int i;

    /* set up the socket for non-blocking */
    if (fcntl(d->sd, F_SETFL, O_NDELAY) < 0L)
    {
        SIOErr(st, strerror(errno));
        status = False;
    }
    else
    {
        /* Set up a timeout loop so that we don't wait forever */
        for (i=0L; i<AcceptLoops; i++)
        {
            /* Finish the connection to the socket */
            if ((d->fd = accept(d->sd,&addr,&fromlen)) >= 0L)
            {   /* got one! */
                break;
            }
            else if (errno != EWOULDBLOCK)
            {   /* Error acepting socket! */
                SIOErr(st, strerror(errno));
                status = False;
                break;
            }
            sleep(AcceptInterval);
        }
        if (i >= AcceptLoops)
        {
             SIOErr(st, "Retries exceeded in AcceptInternetStream() ");
            status = False;
        }
            /* set up the socket for blocking */
#if defined(osf1) || defined(_XOPEN_SOURCE) || defined(sun) || defined(MASSCOMP)
        else if (fcntl(d->fd, F_SETFL, O_SYNC) < 0L)
#else
        else if (fcntl(d->fd, F_SETFL, O_FSYNC) < 0L)
#endif
        {
            SIOErr(st, strerror(errno));
            status = False;
        }
    }
    (void)shutdown(d->sd,2);
    (void)close(d->sd);

    return(status);
}

#endif
