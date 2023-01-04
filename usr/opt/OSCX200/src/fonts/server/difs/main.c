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
/* $XConsortium: main.c,v 1.10 92/11/18 21:29:37 gildea Exp $ */
/*
 * Font server main routine
 */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include	"FS.h"
#include	"FSproto.h"
#include	"clientstr.h"
#include	"resource.h"
#include	"misc.h"
#include	"globals.h"
#include	"servermd.h"
#include	"cache.h"
#include	"site.h"

char       *ConnectionInfo;
int         ConnInfoLen;

Cache       serverCache;

#ifndef DEFAULT_CONFIG_FILE
#define DEFAULT_CONFIG_FILE "/usr/lib/X11/fs/config"
#endif

#define	SERVER_CACHE_SIZE	10000	/* for random server cacheables */

extern void InitProcVectors();
extern void InitFonts();
extern void InitAtoms();
extern void InitExtensions();
extern void ProcessCmdLine();
static Bool create_connection_block();

extern int  ListenSock;
extern ClientPtr currentClient;
char       *configfilename;
extern Bool drone_server;

main(argc, argv)
    int         argc;
    char      **argv;
{
    int         i;

    argcGlobal = argc;
    argvGlobal = argv;

    configfilename = DEFAULT_CONFIG_FILE;

    /* init stuff */
    ProcessCmdLine(argc, argv);
    InitErrors();
    /*
     * do this first thing, to get any options that only take effect at
     * startup time.  it is erad again each time the server resets
     */
    if (ReadConfigFile(configfilename) != FSSuccess)
	FatalError("couldn't parse config file");

    while (1) {
	serverGeneration++;
	OsInit();
	if (serverGeneration == 1) {
	    /* do first time init */
	    serverCache = CacheInit(SERVER_CACHE_SIZE);
	    CreateSockets(ListenSock);
	    InitProcVectors();
	    clients = (ClientPtr *) fsalloc(MAXCLIENTS * sizeof(ClientPtr));
	    if (!clients)
		FatalError("couldn't create client array");
	    for (i = MINCLIENT; i < MAXCLIENTS; i++)
		clients[i] = NullClient;
	    /* make serverClient */
	    serverClient = (ClientPtr) fsalloc(sizeof(ClientRec));
	    if (!serverClient)
		FatalError("couldn't create server client");
	}
	ResetSockets();

	/* init per-cycle stuff */
	InitClient(serverClient, SERVER_CLIENT, (pointer) 0);

	clients[SERVER_CLIENT] = serverClient;
	currentMaxClients = MINCLIENT;
	currentClient = serverClient;

	if (!InitClientResources(serverClient))
	    FatalError("couldn't init server resources");

	InitExtensions();
	InitAtoms();
	InitFonts();
	SetConfigValues();
	if (!create_connection_block())
	    FatalError("couldn't create connection block");

#ifdef DEBUG
	fprintf(stderr, "Entering Dispatch loop\n");
#endif

	Dispatch();

#ifdef DEBUG
	fprintf(stderr, "Leaving Dispatch loop\n");
#endif

	/* clean up per-cycle stuff */
	CacheReset();
	CloseDownExtensions();
	if ((dispatchException & DE_TERMINATE) || drone_server)
	    break;
	fsfree(ConnectionInfo);
	/* note that we're parsing it again, for each time the server resets */
	if (ReadConfigFile(configfilename) != FSSuccess)
	    FatalError("couldn't parse config file");
    }

    CloseErrors();
    exit(0);
}

void
NotImplemented()
{
    NoopDDA();			/* dummy to get difsutils.o to link */
    FatalError("Not implemented");
}

static Bool
create_connection_block()
{
    fsConnSetupAccept setup;
    char       *pBuf;

    setup.release_number = VENDOR_RELEASE;
    setup.vendor_len = strlen(VENDOR_STRING);
    setup.max_request_len = MAX_REQUEST_SIZE;
    setup.length = (SIZEOF(fsConnSetupAccept) + setup.vendor_len + 3) >> 2;

    ConnInfoLen = SIZEOF(fsConnSetupAccept) + ((setup.vendor_len + 3) & ~3);
    ConnectionInfo = (char *) fsalloc(ConnInfoLen);
    if (!ConnectionInfo)
	return FALSE;

    bcopy((char *) &setup, ConnectionInfo, SIZEOF(fsConnSetupAccept));
    pBuf = ConnectionInfo + SIZEOF(fsConnSetupAccept);
    bcopy(VENDOR_STRING, pBuf, (int) setup.vendor_len);

    return TRUE;
}
