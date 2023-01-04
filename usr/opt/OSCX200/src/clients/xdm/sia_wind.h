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
 * @(#)$RCSfile: sia_wind.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/08/02 23:53:48 $
 */
/*
	18oct91 jc - SIA wind header file
*/

#ifndef _sia_wind_h_
#define _sia_wind_h_

#include <sia.h>

extern void sia_wind_init( int argc, char *argv[], struct display *d);

extern void sia_wind_term();

extern int sia_wind_collector(
    unsigned long timeout,
    int rendition,
    unsigned char *client_title, 
    XFontStruct* client_title_font,
    Pixel client_title_color,
    unsigned char *title, 
    XFontStruct* title_font,
    Pixel title_color,
    int num_prompts, 
    prompt_t pmpts[],
    XFontStruct* prompt_font,
    Pixel prompt_color,
    XFontStruct* answer_font,
    Pixel answer_color
    );

extern void sia_wind_getdpy(
    Display **pdpy,
    XtAppContext *pcontext,
    Widget *ptoplevel
    );

extern void sia_wind_error(
    char *errmsg,
    unsigned long timeout
    );

#endif /* _sia_wind_h_ */
