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
	@(#)$RCSfile: radio.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/03 00:00:34 $
*/

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

#ifndef _radio_h
#define _radio_h
extern Radio RadioCreate();	/*  */

extern void RadioAddWidget();	/* radio, widget */
    /* Radio radio; */
    /* Widget widget; */

extern void RadioSetCurrent();	/* radio, name */
    /* Radio radio; */
    /* char *name; */

extern void RadioSetOpened();	/* radio, name */
    /* Radio radio; */
    /* char *name; */

extern void RadioFixFolders();	/* radio */
    /* Radio radio; */

extern void RadioAddButtons();	/* radio, names, num_names, position */
    /* Radio radio; */
    /* char *names; */
    /* Cardinal num_names; */
    /* int position; */

extern void RadioAddFolders();	/* radio, names, num_names, position */
    /* Radio radio; */
    /* char *names; */
    /* Cardinal num_names; */
    /* int position; */

extern void RadioDeleteButton(); /* radio, name */
    /* Radio radio; */
    /* char *name; */

extern char *RadioGetCurrent(); /* radio */
    /* Radio radio; */

extern Cardinal RadioGetNumChildren(); /* radio */
    /* Radio radio; */

extern char *RadioGetName(); /* radio, i */
    /* Radio radio; */
    /* int i; */

extern void RadioSetHighlight(); /* radio, name, value */
    /* Radio radio; */
    /* char *name; */
    /* Boolean value; */


#endif _radio_h
