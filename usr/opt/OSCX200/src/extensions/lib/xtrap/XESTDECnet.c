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
#ifdef DNETCONN /* to rest of file */
#include "Xos.h"
#include <sys/stat.h>
#include <socket.h>
#include <errno.h>
#include "xtraplib.h"
#include "xtraplibp.h"

#ifndef lint
static char SCCSID[] = "@(#)XEStreamDECnet.c	1.6 - 90/11/07  ";
static char RCSID[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/lib/xtrap/XESTDECnet.c,v 1.1.2.2 92/02/06 11:57:04 Jim_Ludwig Exp $";
#endif

#ifdef FUNCTION_PROTOS
int OpenDECnetStream(XESTREAM *st, char *addr, CARD32 mode, int mask)
#else
int OpenDECnetStream(st, addr, mode, mask)
    XESTREAM *st;
    char     *addr;
    CARD32   mode;
    int      mask;
#endif
{
    int status = True;
    register struct sockaddr_dn *dnsock = &(st->u.D.dnsock);
    register INT32 *sd = &(st->u.D.sd);
    char *host;
    char tmpstr1[9L], tmpstr2[9L];
  
    /* Currently the "mode" qualifier is not used for DECnet since
     * it really has no relevance.  If it ever does, it appropriate
     * conversion from "descriptor" types would be done here.
     */

    /* Get the name of this host. This will need to be changed for phase V */
    if ((host = getnodename()) == NULL)
    {   /* we got serious DECnet problems! */
        return(False);
    }
    memset((char *)dnsock,0,sizeof(struct sockaddr_dn));
    dnsock->sdn_family      = AF_DECnet;
    (void)sprintf(tmpstr1, "%04x", getpid());
    (void)sprintf(tmpstr2, "%03x", st);
    (void)sprintf(dnsock->sdn_objname,"S%04.4s%03.3s", 
        &(tmpstr1[strlen(tmpstr1)-4L]), &(tmpstr2[strlen(tmpstr2)-3L]));
    dnsock->sdn_objnamel = strlen(dnsock->sdn_objname);

    if ((*sd = socket(AF_DECnet,SOCK_STREAM,DNPROTO_NSP)) >= 0L &&
        bind(*sd,(struct sockaddr *)dnsock,sizeof(struct sockaddr_dn)) == 0 &&
        listen(*sd,8L) == 0L)
    {   /* we're all set! */
        st->addr2_lgth = strlen(host) + strlen(dnsock->sdn_objname) + 2L;
        if ((st->addr2 = (char *)malloc(st->addr2_lgth)) != NULL)
        {
            strcpy(st->addr2, host);
            strcpy(&(st->addr2[strlen(host)+1L]), dnsock->sdn_objname);
        }
        else
        {   /* no memory avail */
             SIOErr(st, strerror(ENOMEM));
            status = False;
        }
    }
    else
    {   /* we failed */
        char text[BUFSIZ];
        sprintf(text,"socket, bind, or listen call failed (%s) ",
            strerror(errno));
        SIOErr(st, text);
        status = False;
    }

    return(status);
}

#ifdef FUNCTION_PROTOS
int CloseDECnetStream(XESTREAM *st)
#else
int CloseDECnetStream(st)
    XESTREAM *st;
#endif
{
    int status = True;

    if ((shutdown(st->u.D.fd)) || (close(st->u.D.fd)))
    {
        char text[BUFSIZ];
        sprintf(text,"Can't shutdown/close socket (%s) ",
            strerror(errno));
        SIOErr(st, text);
        status = False;
    }
    return(status);
}

#ifdef FUNCTION_PROTOS
int AcceptDECnetStream(XESTREAM *st)
#else
int AcceptDECnetStream(st)
    XESTREAM *st;
#endif
{
    int status = True;
    register struct _dsc *d = &(st->u.D);
    INT32 fromlen=sizeof(struct sockaddr_dn);
    int i;

    /* set up the socket for non-blocking */
    if (fcntl(d->sd, F_SETFL, O_NDELAY) < 0L)
    {
        SIOErr(st, strerror(errno));
        status = False;
    }
    else
    { /* Set up a timeout loop so that we don't wait forever */
        for (i=0L; i<AcceptLoops; i++)
        {
            /* Finish the connection to the socket */
            if ((d->fd = accept(d->sd,&(d->dnsock),&fromlen)) >= 0L)
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
            SIOErr(st, "Retries exceeded in AcceptDECnetStream()! ");
            status = False;
        }
            /* set up the socket for blocking */
        else if (fcntl(d->fd, F_SETFL, O_FSYNC) < 0L)
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
