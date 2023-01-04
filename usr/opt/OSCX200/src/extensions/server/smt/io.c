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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/server/smt/io.c,v 1.1.2.4 92/08/25 13:47:42 Jim_Ludwig Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/***********************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#include <stdio.h>
#include "Xos.h"
#include "Xmd.h"
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/uio.h>
#include "X.h"

#include "os.h"
#include "osdep.h"
#include "opaque.h"
#include "dixstruct.h"
#include "misc.h"
#include "smt.h"

extern int ClientsWithInput[];
extern int ClientsWriteBlocked[];
extern int OutputPending[];
extern long OutputBufferSize;
extern ClientPtr ConnectionTranslation[];
extern Bool NewOutputPending;
extern Bool AnyClientsWriteBlocked;
static Bool CriticalOutputPending;
static int timesThisConnection = 0;

extern int errno;

#define request_length(req, cli) ((cli->swapped ? \
	lswaps((req)->length) : (req)->length) << 2)
#define MAX_TIMES_PER         10

/*****************************************************************
 * ReadRequestFromClient
 *    Returns one request from client.  If the client misbehaves,
 *    returns NULL.  The dispatcher closes down all misbehaving clients.  
 *
 *        client:  index into bit array returned from WaitForSomething() 
 *
 *        status: status is set to
 *            > 0 the number of bytes in the request if the read is sucessful 
 *            = 0 if action would block (entire request not ready)
 *            < 0 indicates an error (probably client died)
 *
 *****************************************************************/

/*ARGSUSED*/
char *
smtReadRequestFromClient(who, status, oldbuf)
    ClientPtr who;
    int *status;          /* read at least n from client */
    char *oldbuf;
{
#define YieldControl()				\
        { isItTimeToYield = TRUE;		\
	  timesThisConnection = 0; }
#define YieldControlNoInput()			\
        { YieldControl();			\
	  BITCLEAR(ClientsWithInput, client); }
#define YieldControlAndReturnNull()		\
        { YieldControlNoInput();		\
	  return((char *) NULL ); }

    register int client = ((OsCommPtr)who->osPrivate)->fd;
    register int result;
    register int gotnow;
    register int needed;
    register ConnectionInput *pBuff;
    register xReq *request;
    register smtControlBlock *pSmt = ((OsCommPtr)who->osPrivate)->pSmt;

#if 0
    if (!SM_ISEMPTY(pSmt)) {
	xReq *req;
	req = (xReq *)SM_GETBEGIN(pSmt);

	BITSET( ClientsWithInput, client );
	*status = req->length<<2;
	pSmt->curr_conn_type = SM_SHAREDMEM;
	if ((++timesThisConnection == MAX_TIMES_PER) || (isItTimeToYield)) {
	    isItTimeToYield = TRUE;
	    timesThisConnection = 0;
	}
    	return((char *)req);
    }
#else
	BITSET( ClientsWithInput, client );
	if ((++timesThisConnection == MAX_TIMES_PER) || (isItTimeToYield)) {
	    isItTimeToYield = TRUE;
	    timesThisConnection = 0;
	}
    	return((char *)req);
#endif
	
