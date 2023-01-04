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
/*
 *
 *  CONTRIBUTORS:
 *
 *      Dick Annicchiarico
 *      Robert Chesler
 *      Dan Coutu
 *      Gene Durso
 *      Marc Evans
 *      Alan Jamison
 *      Mark Henry
 *      Ken Miller
 *
 */
/*
 * This file contains stream initialization routines for the currently
 * supported I/O streams (see also xtrapdi.h).  Once an I/O stream is
 * selected by a given XTrap client, it must be registered (allocated)
 * by one of the initialization streams below.
 */

#include "Xos.h"
#include "xtraplib.h"
#include "xtraplibp.h"
#ifdef VMS
#include <iodef.h>
#include <libdef.h>
#include <dvidef.h>
#include <psldef.h>
#include <jpidef.h>
#include <ssdef.h>
#include <stsdef.h>
#include <nfbdef.h>
#include <errno.h>  /* used in error handling within AST's */
#endif

#ifndef lint
static char SCCSID[] = "@(#)XEStream.c	1.20 - 90/11/07  ";
static char RCSID[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/lib/xtrap/XEStream.c,v 1.1.2.2 92/02/06 11:57:45 Jim_Ludwig Exp $";
#endif

    /* forward declarations */
#ifdef FUNCTION_PROTOS
static void free_stream(XESTREAM *st);
#else
static void free_stream();
#endif

#ifdef VMS
extern int queue_read_qio();
extern int accept_qio();
static int ReadQIOStream();
static int WriteQIOStream();
static void CompleteQIORead();
static void CompleteQIOWrite();
#endif
static XESTREAM XETrapSTREAM;                  /* in XETrapGlobals.c */

#ifdef FUNCTION_PROTOS
XESTREAM *s_open(CARD8 type, char *addr, CARD32 mode, int mask)
#else
XESTREAM *s_open(type, addr, mode, mask)
    CARD8  type;
    char   *addr;
    CARD32 mode;
    int    mask;
#endif
{
    XESTREAM *xp  = &XETrapSTREAM;
    XESTREAM *xpn = NULL;
    int_function open_stream = NULL;
    int status = True;
    static Bool firsttime = True;

    if (firsttime == True)
    {   /* this is the first stream to be defined */
        firsttime = False;
        xp->next = NULL;
    }
    /* position to the end of the list */
    while (xp->next != NULL)
    {
        xp = xp->next;
    }
    if ((xp->next = (XESTREAM *)malloc(sizeof(XESTREAM))) == NULL)
    {   /* Can't use SIOErr() cause it's not allocated */
        fprintf(stderr, "SIO:  Can't allocate memory in s_open()!\n");
        status = False;
    }
    else
    {
        xpn             = xp->next;               /* point to alloc'd struct */
        xpn->next       = NULL;                   /* fill in common fields */
        xpn->err_func   = (void_function )printf; /* default error reporting */
#ifdef VMS
        xpn->u.Q.byte_cnt = 0L;                   /* default stream values */
        xpn->u.Q.bufsiz   = BUFSIZstream;
        xpn->u.Q.bufinc   = BUFstream;
        xpn->u.Q.errno    = 0L;
        xpn->u.Q.write_chan = -1L;
        xpn->u.Q.read_chan = -1L;
#else
        xpn->u.D.sd = -1L;
#endif
        if ((xpn->addr1 = (char *)malloc(strlen(addr)+1)) == NULL)
        {
            SIOErr(xpn, "Can't allocate memory! ");
            status = False; /* malloc failed! */
        }
        else
        {
            strcpy(xpn->addr1, addr);
            xpn->addr1_lgth = strlen(xpn->addr1)+1L; /* incl null terminator */
            xpn->type = type;
            switch(type)
            {
#ifdef TCPCONN
                case StreamInternet:
#ifdef VMS
                    xpn->queue   = True;
#else
                    xpn->queue   = False;
#endif
                    open_stream  = OpenInternetStream;
                    xpn->accept  = AcceptInternetStream;
                    xpn->close   = CloseInternetStream;
#ifdef VMS
                    xpn->read    = ReadQIOStream;
                    xpn->write   = WriteQIOStream;
#else
                    xpn->read    = (int_function)ReadFileStream;
                    xpn->write   = (int_function)WriteFileStream;
#endif
                    break;
#endif
#ifdef DNETCONN
                case StreamDECnet:
#ifdef VMS
                    xpn->queue   = True;
#else
                    xpn->queue   = False;
#endif
                    open_stream  = OpenDECnetStream;
                    xpn->accept  = AcceptDECnetStream;
                    xpn->close   = CloseDECnetStream;
#ifdef VMS
                    xpn->read    = ReadQIOStream;
                    xpn->write   = WriteQIOStream;
#else
                    xpn->read    = (int_function)ReadFileStream;
                    xpn->write   = (int_function)WriteFileStream;
#endif
                    break;
#endif
#if defined MBXCONN && defined VMS
                case StreamMbx:     /* Currently VMS only */
                    xpn->queue   = True;
                    open_stream  = OpenMbxStream;
                    xpn->accept  = accept_qio;
                    xpn->close   = CloseQIOStream;
                    xpn->read    = ReadQIOStream;
                    xpn->write   = WriteQIOStream;
                    break;
#endif /* MBXCONN && VMS */
                case StreamFile:
                    xpn->queue   = False;
                    open_stream  = OpenFileStream;
                    xpn->accept  = NULL;
                    xpn->close   = CloseFileStream;
                    xpn->read    = (int_function)ReadFileStream;
                    xpn->write   = (int_function)WriteFileStream;
                    break;
#ifdef LATCONN
                case StreamLAT:
#ifdef VMS
                    xpn->queue   = True;
#else
                    xpn->queue   = False;
#endif
                    open_stream  = OpenLATStream;
                    xpn->accept  = AcceptLATStream;
                    xpn->close   = CloseLATStream;
#ifdef VMS
                    xpn->read    = ReadQIOStream;
                    xpn->write   = WriteQIOStream;
#else
                    xpn->read    = ReadLATStream;
                    xpn->write   = WriteLATStream;
#endif
                    break;
#endif
                default:
                    SIOErr(xpn, "Invalid stream type! ");
                    status = False;
                    break;
            }
        }
    }
    if ((status == True) && (xpn->queue == True))
    {
#ifdef vms  /* or some other queue-based platform */
        status = XECreateEFN(xpn);  /* Get event flag for queue notif. */
        /* allocate queue headers plus quadword-alignment buffer */
        if ((xpn->u.Q.read_queue = (QUEUE_HDR *)calloc(1L,sizeof(QUEUE_HDR)))
             == NULL)
        {
            SIOErr(xpn, "Can't allocate memory! ");
            status = False;
        }
        if ((xpn->u.Q.free_queue = (QUEUE_HDR *)calloc(1L,sizeof(QUEUE_HDR)))
             == NULL)
        {
            SIOErr(xpn, "Can't allocate memory! ");
            status = False;
        }
#endif
    }
    if (status == True)
    {   /* Now call the stream-specific open routine */
        status = (*open_stream)(xpn, addr, mode, mask);
    }
    if (status == False)
    {   /* if we failed after all that, free the mem and report the bad news */
        free_stream(xpn);
        xpn = NULL;
    }
#ifdef vms
    sys$setast(True);   /* Make sure AST's are enabled! */
#endif
    return(xpn);
}

#ifdef FUNCTION_PROTOS
int s_close(register XESTREAM *st)
#else
int s_close(st)
    register XESTREAM *st;
#endif
{
    int status = True;

    if (st)
    {
        if (st->close)
        {
            status = (*st->close)(st);
        }
        free_stream(st);
    }
    return(status);
}

#ifdef FUNCTION_PROTOS
int s_accept(register XESTREAM *st)
#else
int s_accept(st)
    register XESTREAM *st;
#endif
{
    int status = True;
    if (st)
    {
        if (st->accept)
        {
            status = (*st->accept)(st);
        }
    }
    return(status);
}


#ifdef FUNCTION_PROTOS
int s_read(XESTREAM *st, void *buf, CARD32 size)
#else
int s_read(st,buf,size)
    XESTREAM *st;
    void *buf;
    CARD32 size;
#endif
{
    return((*st->read)(st,buf,size));
}

#ifdef FUNCTION_PROTOS
int s_write(XESTREAM *st, void *buf, CARD32 size)
#else
int s_write(st,buf,size)
    XESTREAM *st;
    void *buf;
    CARD32 size;
#endif
{
    return((*st->write)(st,buf,size));
}

#ifdef FUNCTION_PROTOS
int s_setbufsiz(XESTREAM *st, CARD32 size)
#else
int s_setbufsiz(st,size)
    XESTREAM *st;
    CARD32 size;
#endif
{
    int status = True;  /* Currently *always* returns successfully */

#ifdef VMS
    if (st)
    {
        if (!size)
        {
            size = BUFSIZstream;
        }
        st->u.Q.bufsiz = size;
    }
#endif /* VMS */
    return(status);
}

#ifdef FUNCTION_PROTOS
static void free_stream(XESTREAM *st)
#else
static void free_stream(st)
    XESTREAM *st;
#endif
{
    register XESTREAM *list = &XETrapSTREAM;
#ifdef vms  /* or some other queue-based platform */
    if (st->queue == True)
    {
        lib$free_ef(&st->u.Q.efn);
#ifdef DNETCONN
        lib$free_ef(&st->u.Q.accept_efn);
#endif /* DNETCONN */
    }
#endif /* VMS */
    if (st)
    {
        while(list->next != NULL)
        {
            if (list->next == st)
            {   /* Got it, now remove it from the list */
                list->next = list->next->next;
            }
        }    
        if (st->addr1)
        {
            (void)free(st->addr1);
        }
        if (st->queue)
        {
#ifdef VMS
            QRECPtr ptr;
            while(lib$remqhi(st->u.Q.read_queue, &ptr) != LIB$_QUEWASEMP)
            {
                (void)free(ptr->data);
                (void)free(ptr);
            }
            (void)free(st->u.Q.read_queue);
            while(lib$remqhi(st->u.Q.free_queue, &ptr) != LIB$_QUEWASEMP)
            {
                (void)free(ptr->data);
                (void)free(ptr);
            }
            (void)free(st->u.Q.free_queue);
#endif
        }
        if (st->addr2)
        {
            (void)free(st->addr2);
        }
        (void)free(st);
    }
}

#ifdef VMS
/*
 *  DESCRIPTION:
 *
 *      This function performs the transport specific functions for closing
 *      down a VMS I/O channel.
 *
 */
#ifdef FUNCTION_PROTOS
int CloseQIOStream(XESTREAM *st)
#else
int CloseQIOStream(st)
    XESTREAM *st;
#endif
{
    int status = True;
    int vms_status;

    if (st->u.Q.write_chan != -1L)
    {
        if ((vms_status = sys$cancel(st->u.Q.write_chan)) != SS$_NORMAL)
        {
            char text[BUFSIZ];
            msg_sprint(text,vms_status);
            SIOErr(st, text);
            status = False;
        }
        if ((vms_status = sys$dassgn(st->u.Q.write_chan)) != SS$_NORMAL)
        {
            char text[BUFSIZ];
            msg_sprint(text,vms_status);
            SIOErr(st, text);
            status = False;
        }
    }
    if ((st->u.Q.read_chan != -1L) && 
        (st->u.Q.read_chan != st->u.Q.write_chan))
    {
        if ((vms_status = sys$cancel(st->u.Q.write_chan)) != SS$_NORMAL)
        {
            char text[BUFSIZ];
            msg_sprint(text,vms_status);
            SIOErr(st, text);
            status = False;
        }
        if ((vms_status = sys$dassgn(st->u.Q.read_chan)) != SS$_NORMAL)
        {
            char text[BUFSIZ];
            msg_sprint(text,vms_status);
            SIOErr(st, text);
            status = False;
        }
    }

    return(status);
}
/*
 *  DESCRIPTION:
 *
 *      This function allocates the read/free queue entries and returns
 *      an available entry.  Used exclusively for read queueing.  Also
 *      is self-initializing.
 *
 */
#ifdef FUNCTION_PROTOS
QRECPtr s_get_free_buffer(XESTREAM *st)
#else
QRECPtr s_get_free_buffer(st)
    XESTREAM *st;
#endif
{
    int i;
    QRECPtr ret;

    if (lib$remqhi(st->u.Q.free_queue, &ret) == SS$_NORMAL)
    {
        return(ret);
    }
    /* Need to allocate more free buffers for this stream */
    st->u.Q.bufinc <<= 1L;
    for (i=0; i<st->u.Q.bufinc; i++)
    {
        if ((ret = (QREC *) calloc (1L,sizeof(QREC))) == NULL)
        {
            return(NULL);
        }
        if ((ret->data = (char *) calloc (1L, st->u.Q.bufsiz)) == NULL)
        {
            (void)free(ret);
            return(NULL);
        }
        if (i != st->u.Q.bufinc - 1L) /* don't bother with the last one */
        {
            lib$insqti(ret, st->u.Q.free_queue);
        }
    }
    return(ret);
}

/*
 *  DESCRIPTION:
 *
 *      This function performs the transport specific functions for queueing
 *      a read of data from an XTrap server via a generic VMS I/O channel.
 *
 */
#ifdef FUNCTION_PROTOS
int queue_read_qio(XESTREAM *st)
#else
int queue_read_qio(st)
    XESTREAM *st;
#endif
{
    QRECPtr buf;
    int     status = True;
    int     vms_status;

    if (!(buf = s_get_free_buffer(st)))
    {
        /* Can't allocate read buffer! */
        st->u.Q.errno = ENOMEM;
        SIOErr(st, strerror(st->u.Q.errno));
        status = False;
    }
    else
    {
        buf->st = st;
        if ((vms_status = sys$qio(0L,st->u.Q.read_chan,st->u.Q.read_mode,
            buf->iosb,CompleteQIORead,buf,buf->data,st->u.Q.bufsiz,
            0L,0L,0L,0L)) != SS$_NORMAL)
        {
            char text[BUFSIZ];
            msg_sprint(text,vms_status);
            SIOErr(st, text);
            st->u.Q.errno = EIO;
            status = False;
        }
    }
    return(status);
}
#ifdef FUNCTION_PROTOS
static int accept_qio(XESTREAM *st)
#else
static int accept_qio(st)
    XESTREAM *st;
#endif
{
    int status = True;

    if (st->u.Q.read_mode == IO$_READVBLK)
    {   /* We're reading! */
        status = queue_read_qio(st);
    }
    return(status);
}

#ifdef FUNCTION_PROTOS
static int ReadQIOStream(XESTREAM *st, void *client_buf, CARD16 size)
#else
static int ReadQIOStream(st, client_buf, size)
    XESTREAM *st;
    void     *client_buf;
    CARD16   size;
#endif
{
    struct _qio *q   = &st->u.Q;
    QRECPtr read_entry = NULL;
    INT16 tot_chars  = 0L;
    INT16 this_lgth  = 0L;
    int   vms_status;

    if (q->errno)
    {   /* Error encountered in queueing! AST or queuing of AST */
        char text[BUFSIZ];
        sprintf(text, "%s (ReadQIOStream()) ", strerror(q->errno));
        SIOErr(st, text);
        return(0L);
    }
    sys$setast(False);   /* Disable AST's */
    while(q->byte_cnt < size)
    {
        vms_status = sys$clref(q->efn);   /* Re-cock event flag */
        sys$setast(True);   /* Enable AST's */
        vms_status = sys$waitfr(q->efn);
        sys$setast(False);   /* Disable AST's */
    }
    sys$setast(False);   /* Disable AST's */
    while(tot_chars < size)
    {
        if ((vms_status = lib$remqhi(st->u.Q.read_queue, &read_entry))
            != LIB$_QUEWASEMP)
        {
            this_lgth = MIN((size-tot_chars),(read_entry->iosb[1L]));
            memcpy((char *)client_buf + tot_chars, read_entry->data, 
                this_lgth);
            tot_chars += this_lgth;
#ifdef DEBUG
            fprintf(stderr, "QIO tot: (%x) %d-%d\n",st,q->byte_cnt, this_lgth);
#endif /* DEBUG */
            q->byte_cnt -= this_lgth;
            if (this_lgth < read_entry->iosb[1L])
            {   /* we got more data, we need to stuff part of this back */
                memcpy(read_entry->data, &read_entry->data[this_lgth],
                    read_entry->iosb[1L] - this_lgth);
                /* modify iosb to reflect the new length */
                read_entry->iosb[1L] = read_entry->iosb[1L] - this_lgth;
                vms_status = lib$insqhi(read_entry, st->u.Q.read_queue);
            }
            else
            {   /* All done with this entry */
                vms_status = lib$insqti(read_entry, st->u.Q.free_queue);
            }
        }
    }
    if (!q->byte_cnt)
    {   /* No more bytes to read */
        vms_status = sys$clref(q->efn);   /* Re-cock event flag */
    }
    sys$setast(True);   /* Enable AST's */

    return(tot_chars);
}

/*
 *  DESCRIPTION:
 *
 *      This function performs the transport specific functions for writing
 *      data back to an XTrap server via a generic VMS I/O channel.
 *
 */
#ifdef FUNCTION_PROTOS
static int WriteQIOStream(XESTREAM *st, void *buf, CARD16 size)
#else
static int WriteQIOStream(st, buf, size)
    XESTREAM *st;
    void     *buf;
    CARD16   size;
#endif
{
    int vms_status;
    char text[BUFSIZ];

    if (st->u.Q.write_chan)
    {
        if ((vms_status = sys$qio(0L,st->u.Q.write_chan,
            st->u.Q.write_mode,st->u.Q.iosb,CompleteQIOWrite,st,buf,size,
            0L,0L,0L,0L)) == SS$_NORMAL)
        {
            return(size);
        }
    }
    msg_sprint(text,vms_status);
    SIOErr(st, text);
    st->u.Q.errno = EIO;
    return(0L);
}

/*
 *  DESCRIPTION:
 *
 *      This function performs the transport specific functions for completing
 *      a queued read of data.  Status is checked, the buffer is queued to
 *      the stream's read entry queue, and another read is queued. 
 *
 */
#ifdef FUNCTION_PROTOS
static void CompleteQIORead(QRECPtr qrec)
#else
static void CompleteQIORead(qrec)
    QRECPtr qrec;
#endif
{
    int vms_status;
    Bool queue_another = True;
    char text[BUFSIZ];
#ifdef DEBUG
    int  i;
    char *ptr;
    unsigned char ch;
#endif /* DEBUG */

    switch(qrec->iosb[0])
    {   /* check out status of I/O */
        case SS$_NORMAL:
            break;
        case SS$_BUFFEROVF:
        case SS$_DATAOVERUN:
#ifdef VERBOSE
            sprintf(text, 
                "Data read exceeded buffer size '%d', possible perf. concern ",
                qrec->st->u.Q.bufsiz);
             SIOErr(qrec->st, text);
#endif /* VERBOSE */
            break;
        case SS$_LINKDISCON:
        case SS$_ABORT:
        case SS$_LINKABORT:
            queue_another = False;
            break;
        default:
            msg_sprint(text,qrec->iosb[0]);
            SIOErr(qrec->st, text);
            qrec->st->u.Q.errno = EIO;
            break;
    }
#ifdef DEBUG
    fprintf(stderr,"QIO status: %%x%x\n",qrec->iosb[0L]);
    fprintf(stderr,"QIO: (%d) ", qrec->iosb[1L]);
    ptr = qrec->data;
    for (i=0; i<qrec->iosb[1L]; i++)
    {
        ch = *ptr++;
        fprintf(stderr, "%02x ", ch);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "QIO tot: (%x) %d+%d\n",qrec->st,qrec->st->u.Q.byte_cnt, 
        qrec->iosb[1L]);
    fprintf(stderr, "QIO tot (st): %d (%x)\n",qrec->st->u.Q.byte_cnt,
        qrec->st);
#endif  /* DEBUG */
    vms_status = lib$insqti(qrec, qrec->st->u.Q.read_queue);
    qrec->st->u.Q.byte_cnt += qrec->iosb[1L];
    vms_status = sys$setef(qrec->st->u.Q.efn);

    if (queue_another == True)
    {
        queue_read_qio(qrec->st);    /* Now queue another read */
    }
    return;
}

/*
 *  DESCRIPTION:
 *
 *      This function performs the VMS write completion presumably as
 *      a result of an AST being fired. Status checking is done. 
 *
 */
#ifdef FUNCTION_PROTOS
static void CompleteQIOWrite(XESTREAM *st)
#else
static void CompleteQIOWrite(st)
    XESTREAM *st;
#endif
{
    if (st->u.Q.iosb[0] != STS$M_SUCCESS)
    {
        char text[BUFSIZ];
        msg_sprint(text,st->u.Q.iosb[0]);
        SIOErr(st, text);
        st->u.Q.errno = EIO;
    }
    return;
}
  /* Reserve an arbitrary event flag in cluster 0 */
#ifdef FUNCTION_PROTOS
static int XECreateEFN(XESTREAM *st)
#else
static int XECreateEFN(st)
    XESTREAM *st;
#endif
{
    int vms_status;
    st->u.Q.efn = XEStreamEFN;
    vms_status = sys$clref(st->u.Q.efn);
#ifdef DNETCONN
    vms_status = lib$get_ef(&st->u.Q.accept_efn); /* Get one for DECnet! */
#endif
   return(True);
}
#endif


