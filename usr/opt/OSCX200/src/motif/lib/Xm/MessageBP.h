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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: MessageBP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:42:35 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmessageP_h
#define _XmessageP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/BulletinBP.h>
#include <Xm/MessageB.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  New fields for the MessageBox widget class record  */

typedef struct
{
   int mumble;   /* No new procedures */
} XmMessageBoxClassPart;


/* Full class record declaration */

typedef struct _XmMessageBoxClassRec
{
   CoreClassPart             core_class;
   CompositeClassPart        composite_class;
   ConstraintClassPart       constraint_class;
   XmManagerClassPart        manager_class;
   XmBulletinBoardClassPart  bulletin_board_class;
   XmMessageBoxClassPart     message_box_class;
} XmMessageBoxClassRec;

externalref XmMessageBoxClassRec xmMessageBoxClassRec;


/* New fields for the MessageBox widget record */

typedef struct
{
    unsigned char           dialog_type;
    unsigned char           default_type;
    Boolean		    internal_pixmap;
    Boolean                 minimize_buttons;

    unsigned char           message_alignment;
    XmString                message_string;
    Widget                  message_wid;

    Pixmap                  symbol_pixmap;
    Widget                  symbol_wid;

    XmString                ok_label_string;
    XtCallbackList          ok_callback;
    Widget                  ok_button;

    XmString                cancel_label_string;
    XtCallbackList          cancel_callback;

    XmString                help_label_string;
    Widget                  help_button;

    Widget                  separator;

} XmMessageBoxPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmMessageBoxRec
{
    CorePart	         core;
    CompositePart        composite;
    ConstraintPart       constraint;
    XmManagerPart        manager;
    XmBulletinBoardPart  bulletin_board; 
    XmMessageBoxPart     message_box;
} XmMessageBoxRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern XmGeoMatrix _XmMessageBoxGeoMatrixCreate() ;
extern Boolean _XmMessageBoxNoGeoRequest() ;

#else

extern XmGeoMatrix _XmMessageBoxGeoMatrixCreate( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *desired) ;
extern Boolean _XmMessageBoxNoGeoRequest( 
                        XmGeoMatrix geoSpec) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmMessage_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
