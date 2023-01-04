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
static char *rcsid = "@(#)$RCSfile: dixfonts_load.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 92/10/23 15:56:13 $";
#endif
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1992 BY                  		    *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
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
****************************************************************************/

#include <X11/Xserver/loadable_server.h>
#include <stdio.h>

void 
LoadFontRenderers()
{
    register 		int i;
    void 		(*InitProc)();

    for ( i = 0 ; i < LS_NumFontRenderersLibraries; i++ ) {
        if ( LS_LoadLibraryReqs(LS_FontRenderersLibraries, 
		i, 1, True) == 0 ) {
	    fprintf(stderr, "Cannot load all font renderer libraries.\n");
	    fprintf(stderr, 
		"Library %s will not be an available font renderer.\n",
	        LS_GetLibName(LS_FontRenderersLibraries, i));
	    continue;
	}
	InitProc = LS_GetInitProc(LS_FontRenderersLibraries, i);
	if ( InitProc != NULL )  {
	    InitProc();
	    LS_MarkLibraryInited(LS_FontRenderersLibraries, i);
	}
	else {
	    fprintf(stderr, "Cannot initialize all font renderer libraries.\n");
	    fprintf(stderr, 
		"Library %s will not be an available font renderer.\n",
	        LS_GetLibName(LS_FontRenderersLibraries, i));
	    fprintf(stderr, "Cannot find initialization routine %s\n",
		LS_GetInitProcName(LS_FontRenderersLibraries, i));
	}
    }
}
void 
UnloadFontRenderers()
{
    LS_MarkForUnloadLibraryReqs(LS_FontRenderersLibraries, 
	0, LS_NumFontRenderersLibraries);
    return;
}
