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
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/************************************************************************
 * Modification History
 *
 * 22-Nov-91 -- Lloyd Wheeler
 *		OSF-ize and ANSI C-ify.
 *
 * 29-Aug-90 -- Sam Hsu
 *		Cleanup and merge into 4.titan/law.
 *
 * 29-May-90 -- Win Treese
 *		Created for 2DA from fb_data.c
 *
 ************************************************************************/
#define _PA_DATA_C_

#ifndef _WS_DATA_C_
#include <sys/devio.h>
#include <sys/param.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/buf.h>
#include <io/dec/uba/ubavar.h>		/* auto-config headers 		*/
#include <sys/proc.h>
#include <io/dec/ws/bt459.h>		/* specific to BT459 VDAC 	*/
#endif

#include <io/dec/ws/stamp.h>
#include <io/dec/ws/px.h>
#include <sys/pxinfo.h>

#include "fb.h"
#include "px.h"

#ifdef BINARY

extern pa_info pa_softc[];

#else /*BINARY*/
#ifndef lint
static char *sccsid = "@(#)pa_data.c	5.1      (ULTRIX)  6/19/91";
#endif

#if NPX > 0

pa_info pa_softc[NPX];

#endif /*NPX*/

#endif BINARY
