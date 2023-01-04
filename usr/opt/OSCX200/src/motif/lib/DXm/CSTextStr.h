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
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1990 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	< to be supplied >
**
**  ABSTRACT:
**
**	< to be supplied >
**
**  ENVIRONMENT:
**
**	< to be supplied >
**
**  MODIFICATION HISTORY:
**
**	< to be supplied >
**
**--
**/


#ifndef _DXmCSTextStr_h
#define _DXmCSTextStr_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

/* HACK
 */

typedef int * DXmCSTextSource;
typedef int   DXmCSTextSourceRec;

extern DXmCSTextSource DXmCSTextSourceCreate(); /* value */
    /* XmString 	value; */

extern void DXmCSTextSourceDestroy(); /* source */
    /* XmTextSource 	source; */

extern XmString DXmCSTextSourceGetValue(); /* source */
    /* DXmCSTextSource 	source; */

extern DXmCSTextStatus DXmCSTextSourceSetValue(); /* source, value */
    /* DXmCSTextSource 	source; */
    /* XmString 	value; */

extern Boolean DXmCSTextSourceGetEditable(); /* source */
    /* DXmCSTextSource 	source; */

extern void DXmCSTextSourceSetEditable(); /* source, editable */
    /* DXmCSTextSource 	source; */
    /* Boolean 		editable; */

extern int DXmCSTextSourceGetMaxLength(); /* source */
    /* DXmCSTextSource 	source; */

extern void DXmCSTextSourceSetMaxLength(); /* source, max */
    /* DXmCSTextSource 	source; */
    /* int max; */

extern XmString DXmCSTextSourceRead(); /* source, from, to */
    /* DXmCSTextSource 	source; */
    /* DXmCSTextPosition from, 
			to 	*/

extern DXmCSTextStatus DXmCSTextSourceReplace();/* source,from,to,i_str,len */
    /* DXmCSTextSource 	source; */
    /* DXmCSTextPosition from, 
			to 	*/
    /* XmString 	i_str 	*/
    /* long 		*len 	*/

extern int DXmCSTextSourceGetTextPath(); /* source */
    /* DXmCSTextSource 	source; */


#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /*  _DXmCSTextStr_h */
