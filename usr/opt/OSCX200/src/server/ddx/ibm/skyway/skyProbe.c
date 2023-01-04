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
 * $XConsortium: skyProbe.c,v 1.2 91/07/16 13:15:46 jap Exp $
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

/*
 *  skyProbe.c - probe for display
 *               copied from ibm/GAI/gaiOsd.c
 */

#include "OSio.h"

#include <sys/hft.h>
#include "hftUtils.h"
#include "ibmTrace.h"

extern unsigned hftQueryCurrentDevice();
extern void ibmInfoMsg() ;
extern int hftQFD;

/* adapter number */

/* put this in          */
/* #include "X_msg.h"        */
/*      aixErrMsg(M_MSG_9); */

int
skyProbe(atype, anumber)
	/* adapter type (like HFT_SKYWAY_ID) */
    unsigned  atype;
	/* adapter number */
    int  anumber;
{
    unsigned int  devid;
    int  fd;

    TRACE(("skyProbe(atype=0x%x, anumber=%d)\n", atype, anumber)) ;

/**** added so that console can be used in debugging ****/
    AIXDefaultDisplay= HFT_SKYWAY_ID;
    TRACE(("AIXDefaultDisplay is %x\n",AIXDefaultDisplay)) ;
/********************************************************/

    if ( (devid=hftHasAnAttachedNumber(atype,anumber)) ) {
	ibmInfoMsg("Found an IBM Graphics Adapter \n");

	TRACE(("found Adp 0x%x number %d\n",devid, (devid & HFT_DEVNUM_MASK)));

	if( (fd=hftOpenOnDevice(devid)) < 0 ){
	    ibmInfoMsg("Couldn't open a HFTVT on an IBM Graphics Adapter\n");
	    return -1;
	}
	hftSetQFDOrNoHotkey(fd);
	if( hftQFD == fd ){
	    AIXDefaultDisplay= devid;
	    TRACE(("AIXDefaultDisplay is %x\n",AIXDefaultDisplay)) ;
	}
	return fd;
    }
    else  {
	ibmInfoMsg("Didn't find an IBM Graphics Adapter\n");
	return -1;
    }
}
