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
 * @(#)$RCSfile: DialogSEP.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:32:25 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: DialogSEP.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:32:25 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmDialogShellExtP_h
#define _XmDialogShellExtP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/VendorSEP.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XmIsDialogShellExt
#define XmIsDialogShellExt(w)	XtIsSubclass(w, xmDialogShellExtObjectClass)
#endif /* XmIsDialogShellExt */

externalref WidgetClass xmDialogShellExtObjectClass;

typedef struct _XmDialogShellExtClassRec	*XmDialogShellExtObjectClass ;
typedef struct _XmDialogShellExtRec		*XmDialogShellExtObject ;


typedef struct _XmDialogShellExtClassPart{
    int			empty;
}XmDialogShellExtClassPart, *XmDialogShellExtClassPartPtr;

typedef struct _XmDialogShellExtClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart		desktop_class;
    XmShellExtClassPart		shell_class;
    XmVendorShellExtClassPart 	vendor_class;
    XmDialogShellExtClassPart 	dialog_class;
}XmDialogShellExtClassRec;

typedef struct _XmDialogShellExtPart{
    int		      	empty;
} XmDialogShellExtPart;

externalref XmDialogShellExtClassRec 	xmDialogShellExtClassRec;

typedef struct _XmDialogShellExtRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmShellExtPart		shell;
    XmVendorShellExtPart 	vendor;
    XmDialogShellExtPart 	dialog;
}XmDialogShellExtRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmDialogShellExtP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
