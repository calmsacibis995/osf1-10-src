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
/*   $RCSfile: SelectioB.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:49:05 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmSelectionBox_h
#define _XmSelectionBox_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass xmSelectionBoxWidgetClass;

typedef struct _XmSelectionBoxClassRec * XmSelectionBoxWidgetClass;
typedef struct _XmSelectionBoxRec      * XmSelectionBoxWidget;


#ifndef XmIsSelectionBox
#define XmIsSelectionBox(w)  (XtIsSubclass (w, xmSelectionBoxWidgetClass))
#endif



/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmSelectionBoxGetChild() ;
extern Widget XmCreateSelectionBox() ;
extern Widget XmCreateSelectionDialog() ;
extern Widget XmCreatePromptDialog() ;

#else

extern Widget XmSelectionBoxGetChild( 
                        Widget sb,
#if NeedWidePrototypes
                        unsigned int which) ;
#else
                        unsigned char which) ;
#endif /* NeedWidePrototypes */
extern Widget XmCreateSelectionBox( 
                        Widget p,
                        String name,
                        ArgList args,
                        Cardinal n) ;
extern Widget XmCreateSelectionDialog( 
                        Widget ds_p,
                        String name,
                        ArgList sb_args,
                        Cardinal sb_n) ;
extern Widget XmCreatePromptDialog( 
                        Widget ds_p,
                        String name,
                        ArgList sb_args,
                        Cardinal sb_n) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmSelectionBox_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
