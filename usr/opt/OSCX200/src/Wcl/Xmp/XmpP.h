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
#ifndef _XmpP_h_
#define _XmpP_h_
#include <X11/Xmp/COPY>

/*
* SCCS_data: @(#) XmpP.h	1.4 92/03/18 11:12:16
*
*	This module contains declarations private to the implementation
*	of the Xmp library.
*
*******************************************************************************
*/

#include <X11/Wc/WcCreateP.h>
#include <X11/Xmp/Xmp.h>

void XmpCvtStringToXmString	( CONVERTER(NULL) );
void XmpCvtStringToMenuWidget	( CONVERTER(wcWidgetCvtArgs) );

void XmpPopupACT ( ACTION( menu ) );
void XmpFixTranslationsACT ( ACTION( text ) );
void XmpFixTranslationsCB ( CALLBACK( text ) );
void XmpAddMwmCloseCallbackACT ( ACTION( shell_cbList ) );
void XmpAddMwmCloseCallbackCB ( CALLBACK( shell_cbList ) );
void XmpAddTabGroupCB ( CALLBACK( tabGroupWidgetNameOpt ) );
void XmpAddTabGroupACT ( ACTION( tabGroupWidgetNameOpt ) );
void XmpTableChildConfigCB ( CALLBACK( child_col_row_hSpan_vSpan ) );
void XmpTableChildConfigACT ( ACTION( child_col_row_hSpan_vSpan ) );
void XmpTableChildPositionCB( CALLBACK( child_col_row ) );
void XmpTableChildPositionACT ( ACTION( child_col_row ) );
void XmpTableChildResizeCB( CALLBACK( child_hSpan_vSpan ) );
void XmpTableChildResizeACT ( ACTION( child_hSpan_vSpan ) );
void XmpTableChildOptionsCB( CALLBACK( child_opts ) );
void XmpTableChildOptionsACT ( ACTION( child_opts ) );

#endif /* _XmpP_h_ */
