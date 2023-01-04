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
/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
no- tice appear in all copies and that both that copyright
no- tice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

********************************************************/

/* RANDOM CRUFT! THIS HAS NO OFFICIAL X CONSORTIUM BLESSING */

/* $XConsortium: mitmisc.c,v 1.4 91/06/17 11:36:15 rws Exp $ */

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#define _MITMISC_SERVER_
#include "mitmiscstr.h"

extern Bool permitOldBugs;

static unsigned char MITReqCode;
static int ProcMITDispatch(), SProcMITDispatch();
static void MITResetProc();

void
MITMiscExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();

    if (extEntry = AddExtension(MITMISCNAME, 0, 0,
				 ProcMITDispatch, SProcMITDispatch,
				 MITResetProc, StandardMinorOpcode))
	MITReqCode = (unsigned char)extEntry->base;
}

/*ARGSUSED*/
static void
MITResetProc (extEntry)
ExtensionEntry	*extEntry;
{
}

static int
ProcMITSetBugMode(client)
    register ClientPtr client;
{
    REQUEST(xMITSetBugModeReq);

    REQUEST_SIZE_MATCH(xMITSetBugModeReq);
    if ((stuff->onOff != xTrue) && (stuff->onOff != xFalse))
    {
	client->errorValue = stuff->onOff;
	return BadValue;
    }
    permitOldBugs = stuff->onOff;
    return(client->noClientException);
}

static int
ProcMITGetBugMode(client)
    register ClientPtr client;
{
    REQUEST(xMITGetBugModeReq);
    xMITGetBugModeReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xMITGetBugModeReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.onOff = permitOldBugs;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof(xMITGetBugModeReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcMITDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_MITSetBugMode:
	return ProcMITSetBugMode(client);
    case X_MITGetBugMode:
	return ProcMITGetBugMode(client);
    default:
	return BadRequest;
    }
}

static int
SProcMITSetBugMode(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xMITSetBugModeReq);

    swaps(&stuff->length, n);
    return ProcMITSetBugMode(client);
}

static int
SProcMITGetBugMode(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xMITGetBugModeReq);

    swaps(&stuff->length, n);
    return ProcMITGetBugMode(client);
}

static int
SProcMITDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_MITSetBugMode:
	return SProcMITSetBugMode(client);
    case X_MITGetBugMode:
	return SProcMITGetBugMode(client);
    default:
	return BadRequest;
    }
}
