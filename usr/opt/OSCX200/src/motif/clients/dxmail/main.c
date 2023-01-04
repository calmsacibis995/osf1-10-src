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
#ifndef lint
static char rcs_id[] = "@(#)$RCSfile: main.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:58:23 $";
#endif

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 */


/* main.c */

#define MAIN 1			/* Makes global.h actually declare vars */
#include "decxmail.h"
#include "msg.h"
#include "toc.h"
#include <signal.h>

static XtIntervalId timerid;

/* This gets called every five minutes. */

/* ARGSUSED */
static Boolean WorkCheckScans(data)
Opaque data;
{
    register int i;

    DEBUG(("[magic toc check ..."));
    for (i = 0; i < numScrns; i++) {
	if (scrnList[i]->toc)
	    TocRecheckValidity(scrnList[i]->toc);
	if (scrnList[i]->msg)
	    TocRecheckValidity(MsgGetToc(scrnList[i]->msg));
    }
    DEBUG(("done]\n"));
    return TRUE;
}


/* ARGSUSED */
static Boolean WorkCheckMail(data)
Opaque data;
{
    DEBUG(("(Checking for new mail..."));
    TocCheckForNewMail();
    DEBUG(("done)\n"));
    return TRUE;
}


static Boolean WorkCheckPoint(data)
Opaque data;
{
    MsgHandle handle = (MsgHandle) data;
    Msg msg = MsgFromHandle(handle);
    Toc toc;
    MsgFreeHandle(handle);
    if (msg) {
	MsgCheckPoint(msg);
	toc = MsgGetToc(msg);
	if (toc) {
	    TocSetCacheValid(toc);
	}
    }
    return TRUE;
}

/*ARGSUSED*/
static void CheckMail(closure, id)
Opaque closure;
XtIntervalId id;
{
    static int count = 0;
    register int i;
    timerid = XtAddTimeOut((unsigned long)60000, 
			   (XtTimerCallbackProc) CheckMail, (Opaque) NULL);
    if (defNewMailCheck)
	XtAddWorkProc(WorkCheckMail, (Opaque) NULL);
    if (count++ % 5 == 0) {
	if (defMakeCheckpoints) {
	    for (i=0 ; i<numScrns ; i++)
		if (scrnList[i]->msg)
		    XtAddWorkProc(WorkCheckPoint,
				  (Opaque) MsgGetHandle(scrnList[i]->msg));
	}
	XtAddWorkProc(WorkCheckScans, (Opaque) NULL);
    }
}


/* ARGSUSED */
static void EmptyOldWastebasket(closure, id)
Opaque closure;
XtIntervalId id;
{
    if (defUseWastebasket && defWastebasketDaysToKeep > 0)
	TocExpungeOldMessages(WastebasketFolder, defWastebasketDaysToKeep);
    (void) XtAddTimeOut((unsigned long) 24 * 60 * 60000,
			(XtTimerCallbackProc) EmptyOldWastebasket,
			(Opaque) NULL);
}

/* Main loop. */

main(argc, argv)
unsigned int argc;
char **argv;
{

  XEvent event;
  InitializeWorld(argc, argv);
  if (defNewMailCheck)
    TocCheckForNewMail();
  timerid = XtAddTimeOut((unsigned long)60000,
			 (XtTimerCallbackProc)CheckMail, (Opaque) NULL);
  (void) XtAddTimeOut((unsigned long) 90000, 
		      (XtTimerCallbackProc)EmptyOldWastebasket,
		      (Opaque) NULL);

#ifdef IGNOREDDIF
  XtMainLoop();
#else /* IGNOREDDIF */
  /*     XtMainLoop();  is replaced by the following lines of code, to fix the DPS problem */
  
  for (;;) {
    XtAppNextEvent((XtAppContext)_XtDefaultAppContext(), &event);
    if(!(dps_exists && XDPSDispatchEvent(&event)))
      XtDispatchEvent(&event);
  }
#endif /* IGNOREDDIF */
  return 0;
}
