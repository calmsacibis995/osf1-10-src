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
/*   $RCSfile: DialogSP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:32:31 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmDialogShellP_h
#define _XmDialogShellP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/XmP.h>
#include <Xm/DialogS.h>
#include <X11/ShellP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* macros: THESE BELONG IN XmP.h */
#define XtX(w)		 ((w)->core.x)
#define XtY(w)		 ((w)->core.y)
#define XtWidth(w)	 ((w)->core.width)
#define XtHeight(w)	 ((w)->core.height)
#define XtBorderWidth(w) ((w)->core.border_width)
#define XtBackground(w)	 ((w)->core.background_pixel)
#define XtSensitive(w)	 ((w)->core.sensitive && (w)->core.ancestor_sensitive) 

#ifndef XtParent
#define XtParent(w)	 ((w)->core.parent)
#endif


/* The DialogShell instance record */

typedef struct 
{
    /* internal fields */
    XtGrabKind 		grab_kind;
    Position		old_x, old_y;

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
    unsigned char       fit_to_screen_policy;
#endif

#ifdef DEC_MOTIF_RTOL
    unsigned char       dxm_layout_direction;
#endif

} XmDialogShellPart;

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
#define FitToScreenPolicy(w) ((w)->dialog.fit_to_screen_policy)
#endif

#ifdef DEC_MOTIF_RTOL
#define LayoutDS(w) ((w)->dialog.dxm_layout_direction)
#endif

/* Full instance record declaration */

typedef  struct _XmDialogShellRec 
{	
    CorePart		    core;
    CompositePart	    composite;
    ShellPart		    shell;
    WMShellPart		    wm;
    VendorShellPart	    vendor;
    TransientShellPart	    transient;
    XmDialogShellPart	    dialog;
} XmDialogShellRec;

typedef  struct _XmDialogShellWidgetRec /* OBSOLETE (for compatibility only).*/
{	
    CorePart		    core;
    CompositePart	    composite;
    ShellPart		    shell;
    WMShellPart		    wm;
    VendorShellPart	    vendor;
    TransientShellPart	    transient;
    XmDialogShellPart	    dialog;
} XmDialogShellWidgetRec;



/* DialogShell class structure */

typedef struct 
{
    XtPointer			extension;	 /* Pointer to extension record */
} XmDialogShellClassPart;


/* Full class record declaration */

typedef struct _XmDialogShellClassRec 
{
    CoreClassPart 		core_class;
    CompositeClassPart 		composite_class;
    ShellClassPart 		shell_class;
    WMShellClassPart	        wm_shell_class;
    VendorShellClassPart 	vendor_shell_class;
    TransientShellClassPart  	transient_shell_class;
    XmDialogShellClassPart 	dialog_shell_part;
} XmDialogShellClassRec;


externalref XmDialogShellClassRec  xmDialogShellClassRec;


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
#endif /* _XmDialogShellP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
