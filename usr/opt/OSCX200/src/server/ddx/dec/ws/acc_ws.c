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
/* $XConsortium: acc_none.c,v 1.2 91/07/08 11:16:21 keith Exp $ */
/***********************************************************
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
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
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <errno.h>
#include <sys/devio.h>

#include "X.h"
#include "Xproto.h"

#include "scrnintstr.h"
#include "servermd.h"

#include "input.h"
#include <sys/workstation.h>
#include "ws.h"

extern Bool (*fbScreenInitProc)();
extern Bool cfbScreenInit();
extern Bool sfbScreenInit();
/*DMC #ifdef DMC_temp /* Dave Coleman temp */
Bool ropInitProc();
/*DMC #endif	/* DMC_temp (Dave Coleman temp) */
Bool pxInitProc();

Bool sfbInitProc(index, pScreen, argc, argv)
    int		index;
    ScreenPtr   pScreen;
    int		argc;
    char	**argv;
{
    fbScreenInitProc = sfbScreenInit;
    fbInitProc(index, pScreen, argc, argv);
    fbScreenInitProc = cfbScreenInit;
}

wsAcceleratorTypes types[] = {
/* ||| Are these exactly correct idents for TURBOchannel ROM? */
	{"PMAGB-BA", sfbInitProc},
	{"PMAGB-BB", sfbInitProc},
	{"PMAG-RO ", ropInitProc},
	{"PMAG-JA ", ropInitProc},
	{"PMAG-CA ", pxInitProc },
	{"PMAG-DA ", pxInitProc },
	{"PMAG-FA ", pxInitProc },
};

int num_accelerator_types = sizeof (types) / sizeof (wsAcceleratorTypes);
