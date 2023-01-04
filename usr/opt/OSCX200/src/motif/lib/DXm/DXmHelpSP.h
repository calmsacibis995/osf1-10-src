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
***********************************************************
**                                                        *
**  Copyright (c) Digital Equipment Corporation, 1990  	  *
**  All Rights Reserved.  Unpublished rights reserved	  *
**  under the copyright laws of the United States.	  *
**                                                        *
**  The software contained on this media is proprietary	  *
**  to and embodies the confidential technology of 	  *
**  Digital Equipment Corporation.  Possession, use,	  *
**  duplication or dissemination of the software and	  *
**  media is authorized only pursuant to a valid written  *
**  license from Digital Equipment Corporation.	    	  *
**  							  *
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	  *
**  disclosure by the U.S. Government is subject to	  *
**  restrictions as set forth in Subparagraph (c)(1)(ii)  *
**  of DFARS 252.227-7013, or in FAR 52.227-19, as	  *
**  applicable.	    					  *
**  		                                          *
***********************************************************
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

#ifndef _DXmHelpShellP_h
#define _DXmHelpShellP_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
#include <DECW$INCLUDE:XmP.h>
#else
#include <Xm/XmP.h>
#endif

/*
 * define the additional data a shell class widget has that a
 * shell widget doesn't
 */

typedef struct
{
    XtGrabKind		    grabkind;
} 
    DXmHelpShellPart;

/*
 * now define the actual help shell widget data struct
 */

typedef struct 
{
    CorePart		core;	    /* basic widget			    */
    CompositePart	composite;  /* composite specific data		    */
    ShellPart		shell;	    /* shell specific data		    */
    WMShellPart		wm;	    /* common window manager specific data  */
    VendorShellPart	vendor;	    /* vendor window manager specific data  */
    TopLevelShellPart	topLevel;   /* toplevel window specific data	    */
    DXmHelpShellPart	helpshell;  /* help shell specific data		    */
} 
    DXmHelpShellWidgetRec, *DXmHelpShellWidget;

/*
 * define the attributes (or things it does) that a help shell class has
 * that a shell class doesn't
 */

typedef struct 
{
    XtPointer		extension;	/* Pointer to extension record */
}
    DXmHelpShellClassPart;


typedef struct _DXmHelpShellClassRec 
{
    CoreClassPart	    core_class;
    CompositeClassPart	    composite_class;
    ShellClassPart	    shell_class;
    WMShellClassPart	    wm_shell_class;
    VendorShellClassPart    vendor_shell_class;
    TopLevelShellClassPart  top_level_shell_class;
    DXmHelpShellClassPart   help_shell_class;
} 
    DXmHelpShellClassRec, *DXmHelpShellWidgetClass;

externalref DXmHelpShellClassRec	dxmHelpShellClassRec;
externalref DXmHelpShellWidgetClass     dxmHelpShellWidgetClass;

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif 
