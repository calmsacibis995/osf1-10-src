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
**++
**
**  COPYRIGHT (c) 1991 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
**  ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**
**  ABSTRACT:
**	This file contains function prototype definitions for the
**	windowing CDA Viewer Routines.
**--
**/

/*
 * Translate __ predefined symbols to pre ANSI C style when not
 * compiling with an ANSI C conformant compiler.
 */
#include <cdatrans.h>

/*
 * Include CDA defined types.
 */
#ifdef __vms__
#define CDA_DECWINDOWS

#ifndef _cdatyp_
#include <cda$typ.h>
#endif

#endif

#ifdef __unix__
#define CDA_DECWINDOWS

#ifndef _cdatyp_
#include <cda_typ.h>
#endif

#endif

#ifndef CDA_DECWINDOWS
#ifndef _cdatyp_
#include <cda$typ.h>
#endif
#endif

#ifdef CDA_DECWINDOWS
#define DVR_BOOLEAN	Boolean 	/* Boolean is defined by X */
#else
#ifndef HAB
#define HAB HANDLE
#endif
#define DVR_BOOLEAN 	BOOL            /* BOOL is defined by PM */
#endif

#ifndef DVR_VIEW_TYPES

#define DVR_VIEW_TYPES
typedef struct dvrviewerwidget  CDA_FAR * DvrViewerWidget;
#endif

/* external documented entry points */

#ifdef CDA_DECWINDOWS  /*** DECWINDOWS specific definitions ***/

PROTO( DvrViewerWidget DvrViewer,
		(Widget,
		 CDAenvirontext *,
		 Position,
		 Position,
		 Dimension,
		 Dimension,
		 Boolean,
		 Boolean,
		 CDAflags,
		 XtCallbackList,
		 XtCallbackList) );

PROTO( DvrViewerWidget DvrViewerCreate,
		(Widget,
		 CDAenvirontext *,
		 ArgList,
		 CDAcardinal) );

/* converter selection widget create routines */
PROTO( CDAstatus DvrConverterFileSelection,
		(Widget	*,
		 Widget	*,
		 CDAflags,
		 Position,
		 Position,
		 CDAenvirontext	*,
		 Boolean,
		 XtCallbackList) );

PROTO( CDAstatus DvrConvFileSelectionCreate,
		 (Widget *,
		  Widget *,
	   	  CDAitemlist *,
		  XtCallbackList) );

#else  /* OS/2 and/or MS-Windows */

PROTO( DvrViewerWidget CDA_APIENTRY DvrViewer,
		(HAB,
		 HWND,
		 HWND CDA_FAR *,
                 CDAcardinal,
		 CDAcardinal,
		 CDAcardinal,
		 CDAcardinal,
		 DVR_BOOLEAN,
		 DVR_BOOLEAN,
		 CDAflags,
		 void (CDA_CALLBACK *) (HWND, DvrCallbackStruct CDA_FAR *),
                 void (CDA_CALLBACK *) (HWND, DvrCallbackStruct CDA_FAR *) ));

PROTO( DvrViewerWidget CDA_APIENTRY DvrViewerCreate,
		(HAB,
		 HWND,
		 HWND CDA_FAR *,
		 CDAitemlist CDA_FAR *) );

PROTO(CDAstatus CDA_APIENTRY DvrSetValues,
		(DvrViewerWidget,
		 CDAitemlist CDA_FAR *) );

PROTO(CDAstatus CDA_APIENTRY DvrGetValues,
		(DvrViewerWidget,
		 CDAitemlist CDA_FAR *) );

PROTO(CDAstatus CDA_APIENTRY DvrLoadFile,
		(DvrViewerWidget,
		 CDAenvirontext CDA_FAR *,
		 CDAenvirontext CDA_FAR *,
		 CDAenvirontext CDA_FAR *,
		 CDAstatus (CDA_CALLBACK *) (CDAuserparam, CDAsize CDA_FAR *, 
					     CDAbufaddr CDA_FAR *),
		 CDAuserparam) );

PROTO(CDAstatus CDA_APIENTRY DvrDisplayFirstPage,
                (DvrViewerWidget) );

PROTO (HWND CDA_APIENTRY DvrConverterFileSelection, 
            (HWND,
	     CDAconstant,
	     int,
	     int,
   	     CDAenvirontext CDA_FAR *,
	     CDAboolean, 
             BOOL (CDA_CALLBACK *)(HWND, WORD, WORD, LONG)));

PROTO (HWND CDA_APIENTRY DvrConvFileSelectionCreate,
            (HWND, 
             DVR_BOOLEAN (CDA_CALLBACK *)(HWND, WORD, WORD, LONG), 
             CDAitemlist CDA_FAR *));

#endif

PROTO(CDAstatus CDA_APIENTRY DvrViewerFile,
		(DvrViewerWidget,
		 CDAenvirontext CDA_FAR *,
		 CDAenvirontext CDA_FAR *,
		 CDAenvirontext CDA_FAR *,
		 CDAstatus (CDA_CALLBACK *) (CDAuserparam, CDAsize CDA_FAR *, 
					     CDAbufaddr CDA_FAR *),
		 CDAuserparam) );

PROTO(CDAstatus CDA_APIENTRY DvrRegisterClass,
		() );

PROTO(CDAstatus CDA_APIENTRY DvrCloseFile,
		(DvrViewerWidget) );

PROTO(CDAstatus CDA_APIENTRY DvrDocumentInfo,
		(DvrViewerWidget,
		 CDAenvirontext CDA_FAR * CDA_FAR *) );

PROTO(CDAstatus CDA_APIENTRY DvrTopDocument,
		(DvrViewerWidget) );

PROTO(CDAstatus CDA_APIENTRY DvrBottomDocument,
		(DvrViewerWidget) );

PROTO(CDAstatus CDA_APIENTRY DvrNextPage,
		(DvrViewerWidget) );

PROTO(CDAstatus CDA_APIENTRY DvrPreviousPage,
		(DvrViewerWidget) );

PROTO(CDAstatus CDA_APIENTRY DvrGotoPage,
		(DvrViewerWidget,
		 CDAconstant) );

