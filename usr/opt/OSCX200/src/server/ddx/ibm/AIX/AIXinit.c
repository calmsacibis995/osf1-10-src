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
 * $XConsortium: AIXinit.c,v 1.3 91/07/16 12:56:15 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#include <fcntl.h>
#include <sys/hft.h>

#include "Xmd.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursor.h"
#include "ibmScreen.h"
#include "OSio.h"
#include "hftQueue.h"

#include "ibmTrace.h"

void
ibmMachineDependentInit()
{
    TRACE(("ibmMachineDependentInit()\n"));

    hftInitQueue(AIXDefaultDisplay,FALSE);
    RemoveEnabledDevice(hftQFD);
    return;
}

/***==================================================================***/

#if !defined(FORCE_DISPLAY_NUMBER)

#if !defined(MAX_SERVER_NUM)
#define MAX_SERVER_NUM 255
#endif

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#include <errno.h>
#include <sys/wait.h>

#include "Xproto.h"

int AIXTCPSocket= -1;
static char dummy[20];

extern char *display ;

void
AIXFindServerIndex()
{
struct sockaddr_in insock ;
int sockNum, sockFD ;

    if ((sockFD = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
	Error("creating TCP socket (in FindATCPSocket)\n",NULL);
	display= "0";
	return;
    }

    bzero( (char *)&insock, sizeof insock );
    insock.sin_family = AF_INET;
    insock.sin_addr.s_addr = htonl(INADDR_ANY);

    for ( sockNum = 0 ; sockNum < MAX_SERVER_NUM ; sockNum++ ) {
	insock.sin_port = htons( X_TCP_PORT + sockNum ) ;
	if ( bind( sockFD, (struct sockaddr *) &insock, sizeof insock ) ) {
	    if (errno!=EADDRINUSE) {
		display= "0";
		return;
	    }
	}
	else break;
    }
    AIXTCPSocket= sockFD;
    (void) sprintf( display = dummy, "%d", sockNum ) ;
    return;
}
#endif /* FORCE_DISPLAY_NUMBER */
