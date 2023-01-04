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
/* $XConsortium: FSErrDis.c,v 1.2 91/05/13 15:11:29 gildea Exp $ */

/* @(#)FSErrDis.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <X11/Xos.h>
#include "FSlibint.h"

char       *FSErrorList[] = {
     /* FSBadRequest	 */ "BadRequest, invalid request code or no such operation",
     /* FSBadFormat	 */ "BadFormat, bad font format mask",
     /* FSBadFont	 */ "BadFont, invalid Font parameter",
     /* FSBadRange	 */ "BadRange, invalid character range attributes",
     /* FSBadEventMask	 */ "BadEventMask, illegal event mask",
     /* FSBadAccessContext */ "BadAccessContext, insufficient permissions for operation",
     /* FSBadIDChoice  */ "BadIDChoice, invalid resource ID chosen for this connection",
     /* FSBadName	 */ "BadName, named font does not exist",
     /* FSBadResolution	 */ "BadResolution, improperly formatted resolution",
     /* FSBadAlloc	 */ "BadAlloc, insufficient resources for operation",
     /* FSBadLength	 */ "BadLength, request too large or internal FSlib length error",
     /* FSBadImplementation */ "BadImplementation, request unsupported",
};
int         FSErrorListSize = sizeof(FSErrorList);


FSGetErrorText(svr, code, buffer, nbytes)
    register int code;
    register FSServer *svr;
    char       *buffer;
    int         nbytes;
{

    char       *defaultp = NULL;
    char        buf[32];
    register _FSExtension *ext;

    if (nbytes == 0)
	return;
    sprintf(buf, "%d", code);
    if (code <= (FSErrorListSize / sizeof(char *)) && code > 0) {
	defaultp = FSErrorList[code];
	FSGetErrorDatabaseText(svr, "FSProtoError", buf, defaultp, buffer, nbytes);
    }
    ext = svr->ext_procs;
    while (ext) {		/* call out to any extensions interested */
	if (ext->error_string != NULL)
	    (*ext->error_string) (svr, code, &ext->codes, buffer, nbytes);
	ext = ext->next;
    }
    return;
}

/* ARGSUSED */
FSGetErrorDatabaseText(svr, name, type, defaultp, buffer, nbytes)
    register char *name,
               *type;
    char       *defaultp;
    FSServer     *svr;
    char       *buffer;
    int         nbytes;
{
    if (nbytes == 0)
	return;
    (void) strncpy(buffer, (char *) defaultp, nbytes);
    if ((strlen(defaultp) + 1) > nbytes)
	buffer[nbytes - 1] = '\0';
}
