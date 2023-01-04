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
*************************************************************************
**									*
**  Copyright (c) Digital Equipment Corporation, 1990, 1991		*
**  All Rights Reserved.  Unpublished rights reserved			*
**  under the copyright laws of the United States.			*
**									*
**  The software contained on this media is proprietary			*
**  to and embodies the confidential technology of			*
**  Digital Equipment Corporation.  Possession, use,			*
**  duplication or dissemination of the software and			*
**  media is authorized only pursuant to a valid written		*
**  license from Digital Equipment Corporation.				*
**									*
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or			*
**  disclosure by the U.S. Government is subject to			*
**  restrictions as set forth in Subparagraph (c)(1)(ii)		*
**  of DFARS 252.227-7013, or in FAR 52.227-19, as			*
**  applicable.								*
**									*
*************************************************************************
*/

#ifndef _decspecific_h
#define _decspecific_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

/*
** This include file contains the DECspecific definitions for the DECmotif
** toolkit.
*/

#ifdef VMS
#include <DECW$INCLUDE:DECwI18n.h>
#else
#include <X11/DECwI18n.h>
#endif


/* These replace DwtChildren and DwtNumChildren  */


#define XmChildren(w)             ((w)->composite.children)
#define XmNumChildren(w)          ((w)->composite.num_children) 

/*
 * additional compound string value for nested support
 */
#define XmSTRING_DIRECTION_REVERT 2

/*
 * compound string converter status returns
 */
#define DXmCvtStatusOK        1
#define DXmCvtStatusDataLoss  2
#define DXmCvtStatusFail      3


/* Constants and function declarations for DXmCreateCursor 
 * These constant values have been deprecated.  Use the
 * values in DECw$Cursor.h instead
 */
#define DXm_WAIT_CURSOR		4
#define DXm_HELP_CURSOR		54
#define DXm_INACTIVE_CURSOR	6


/*
 * compound string converter externs
 */
#ifdef _NO_PROTO

/* dxmmisc.c */
extern void DXmInitialize ( );
extern void DXmChangeWindowGeometry ( );
extern XtGeometryResult DXmMakeGeometryRequest ( );
extern Boolean DXmStringCheck ( );
extern Cardinal DXmNumChildren ( );
extern WidgetList DXmChildren ( );
extern void DXmActivateWidget ( );
extern char *DXmFindFontFallback ( );
extern XFontStruct *DXmLoadQueryFont ( );
extern XmString DXmGetLocaleString ( );
extern Opaque DXmCvtCStoFC ( );
extern XmString DXmCvtFCtoCS ( );
extern Opaque DXmCvtCStoOS ( );
extern XmString DXmCvtOStoCS ( );
extern Opaque DXmCvtCStoDDIF ( );
extern XmString DXmCvtDDIFtoCS ( );
extern Cursor _DXmCreateWaitCursor ( );
extern Cursor DXmCreateCursor ( );
extern XmStringCharSet DXmGetLocaleCharset ( );
extern Boolean DXmCSContainsStringCharSet ( );
extern KeySym DXmGetLocaleMnemonic ( );
extern XmStringCharSet *DXmGetLocaleCharsets ( );
extern XmFontList DXmFontListCreateDefault ( );
extern void DXmFormSpaceButtonsEqually ( );
extern void DXmHelpOnContext ();

/* ErrorMsg.c */

extern char *DXmDescToNull ();
extern Widget DXmDisplayVmsMessage ( );
extern Widget DXmDisplayCSMessage ();

#ifdef VMS
/* hyperhelp.c */
extern void DXmHelpSystemOpen ();
extern void DXmHelpSystemDisplay ();
extern void DXmHelpSystemClose ();
#endif

/* position_widget.c */

extern void DXmPositionWidget ();

#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* c++ */

/* DXmMisc.c */
extern void DXmInitialize ( void );
extern void DXmChangeWindowGeometry ( Widget w , XtWidgetGeometry *size );
extern XtGeometryResult DXmMakeGeometryRequest ( Widget w , XtWidgetGeometry *geom );
extern Boolean DXmStringCheck ( XmString *old_external , XmString *new_external );
extern Cardinal DXmNumChildren ( CompositeWidget w );
extern WidgetList DXmChildren ( CompositeWidget w );
extern void DXmActivateWidget ( Widget w );
extern char *DXmFindFontFallback ( char *fontname );
extern XFontStruct *DXmLoadQueryFont ( Display *d , char *fontname );
extern XmString DXmGetLocaleString ( I18nContext context , char *ascii , I18nWordType word_type );
extern Opaque DXmCvtCStoFC ( XmString cs , long *byte_count , long *status );
extern XmString DXmCvtFCtoCS ( Opaque fc , long *byte_count , long *status );
extern Opaque DXmCvtCStoOS ( XmString cs , long *byte_count , long *status );
extern XmString DXmCvtOStoCS ( Opaque os_string , long *byte_count , long *status );
extern Opaque DXmCvtCStoDDIF ( XmString cs , long *byte_count , long *status );
extern XmString DXmCvtDDIFtoCS ( Opaque ddif , long *size , long *return_status );
extern Cursor DXmCreateCursor ( Widget w , int cursorkind );
extern XmStringCharSet DXmGetLocaleCharset ( void );
extern Boolean DXmCSContainsStringCharSet ( XmString str );
extern KeySym DXmGetLocaleMnemonic ( I18nContext context , Widget w , char *mnemonic , XmStringCharSet charset );
extern XmStringCharSet *DXmGetLocaleCharsets ( void );
extern XmFontList DXmFontListCreateDefault ( Widget widget , String resource_name );
extern void DXmFormSpaceButtonsEqually ( Widget parent , Widget *widget_list , Cardinal num_widgets );
extern void DXmHelpOnContext (Widget w, Boolean confine);

/* ErrorMsg.c */

#ifdef  VAXC
extern char *DXmDescToNull ( struct dsc$descriptor_s *desc );
#endif
extern Widget DXmDisplayVmsMessage ( Widget Parent , char *Name , int Pos , int X , int Y , int Style , int *Msgvec , Widget *Widget_id , int (*User_routine )(), XtCallbackList Ok_callback , XtCallbackList Help_callback );
extern Widget DXmDisplayCSMessage ( Widget Parent , char *Name , int Pos , int X , int Y , int Style , long *Msgvec , Widget *Widget_id , int (*User_routine )(), XtCallbackList Ok_callback , XtCallbackList Help_callback );

#ifdef VMS
/* hyperhelp.c */
extern void DXmHelpSystemOpen (		Opaque	*help_context,
					Widget	main_window,
					char	*help_file,
					void	((*routine )()),
					Opaque	tag );

extern void DXmHelpSystemDisplay (	Opaque	help_context,
					char	*help_file,
					char	*keyword,
					char	*name,
					void	((*routine )()),
					Opaque	tag );

extern void DXmHelpSystemClose (	Opaque	help_context,
					void	((*routine )()),
					Opaque	tag );
#endif

/* position_widget.c */

extern void DXmPositionWidget (Widget new_widget, Widget *avoid_widgets, int widget_cnt);



#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* c++ */

#endif



#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _decspecific_h - do not add anything after this endif */
