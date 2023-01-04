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
 * $XConsortium: mipsKbdNET.c,v 1.5 91/07/18 22:58:36 keith Exp $
 *
 * Copyright 1991 MIPS Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of MIPS not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  MIPS makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * MIPS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL MIPS
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsKbdNET.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

/*
 *	keysym mapping for the network keyboard.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <fcntl.h>
#ifdef SYSV
#include <sys/termio.h>
#include <bsd/sys/socket.h>
#include <bsd/netinet/in.h>
#include <bsd/netinet/tcp.h>
#include <bsd/sys/un.h>
#else /* SYSV */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#endif /* SYSV */
#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "scrnintstr.h"
#include "mips.h"
#include "mipsIo.h"
#include "mipsKbd.h"

#if NETWORK_KEYBD
char	*netKeybdAddr = NULL;

int
openNetKeybd()
{
    struct sockaddr_in  tcpsock;
    struct sockaddr	*addr;
    int			addrlen;
    int                 connFd;
    int			optval;

    if (netKeybdAddr) {
	ErrorF("connecting to network keyboard at %s\n", netKeybdAddr);
	bzero((char *)&tcpsock, sizeof(tcpsock));
	tcpsock.sin_family = AF_INET;
	tcpsock.sin_port = htons(6007);
	tcpsock.sin_addr.s_addr = inet_addr(netKeybdAddr);
	addr = (struct sockaddr *) &tcpsock;
	addrlen = sizeof(tcpsock);
	if ((connFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    Error("creating TCP socket");
	else if (connect(connFd, addr, addrlen) == -1)
	    ErrorF("unable to connect to network keyboard\n");
	else {
#ifdef SYSV
	    optval = 1;
	    ioctl(connFd, FIONBIO, &optval);
#else /* SYSV */
	    fcntl(connFd, F_SETFL, FNDELAY);
#endif /* SYSV */
	    ErrorF("connected to network keyboard\n");
	    keybdPriv.cap = DEV_READ;
	    netkeyboard();
	    return(connFd);
	}
    }
    return(-1);
}

netKeybd()
{
    void netKeybdEvent();

#ifdef XT_KEYBOARD
    keybdType[XT_KEYBOARD].keybdEvent = netKeybdEvent;
#endif /* XT_KEYBOARD */
#ifdef AT_KEYBOARD
    keybdType[AT_KEYBOARD].keybdEvent = netKeybdEvent;
#endif /* AT_KEYBOARD */
#ifdef UNIX1_KEYBOARD
    keybdType[UNIX1_KEYBOARD].keybdEvent = netKeybdEvent;
#endif /* UNIX1_KEYBOARD */
}

void
netKeybdEvent(pKeybd, code)
DevicePtr	pKeybd;
u_char		code;
{
    static int		release = 0;
    u_char		kindex;

    if (code == 0xf0)
	release = 1;
    else {
	kindex = code;
	genKeybdEvent(pKeybd, release, kindex);
	release = 0;
    }
}
#endif /* NETWORK_KEYBD */
