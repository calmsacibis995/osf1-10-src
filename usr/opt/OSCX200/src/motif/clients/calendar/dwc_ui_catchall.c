/* DWC$UI_CATCHALL "V3.0-008" */
#ifndef lint
static char rcsid[] = "$Header$";
#endif /* lint */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; user interface routines
**
**  AUTHOR:
**
**	Denis G. Lacroix, February 1989
**
**  ABSTRACT:
**
**	Error handling for Calendar's User Interface
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3.0-008 Paul Ferwerda					23-Oct-1990
**		Change name of DestoryErrorBox to ERRORDestroyErrorBox.
** V3.0-007 Paul Ferwerda					27-Sep-1990
**		Changed NDEBUG to DEBUG to be consistent with the rest of the
**		ui.
** V3-006 PGLF	05-Feb-1990 Tweaked help files, got rid of LIB$: and port to
**			    Motif 
**	V2-005	Denis G. Lacroix				21-Jun-1989
**		We should only signal SS$_DEBUG when we are in debugging
**		mode, not in the code that ships! Test NDEBUG.
**	V2-004	Per Hamnqvist					24-Mar-1989
**		Signal SS$_DEBUG on VMS, for now, so that one can trace
**		the fault.
**	V2-003	Denis G. Lacroix				28-Feb-89
**		Added dwc_compat.h for ULTRIX build
**	V2-002  Denis G. Lacroix				28-Feb-89
**		Added DWC$UI_Establish and modified DWC$UI_Catchall
**	V2-001  Denis G. Lacroix				28-Feb-1989
**		Initial version.
**--
*/

#include    "dwc_compat.h"

#include <stdio.h>
#ifdef vaxc
#pragma nostandard
#endif
#include <Xm/Xm.h>
#ifdef vaxc
#pragma standard
#endif

#ifdef VMS
#include    <ssdef.h>
#endif

#include    "dwc_ui_calendar.h"
#include    "dwc_ui_catchall.h"	
#include    "dwc_ui_errorboxes.h"
#include    "dwc_ui_datestructures.h"
#include    "dwc_ui_icons.h"

extern AllDisplaysRecord ads;	    /* defined in dwc_ui_calendar.c */

/*
**  Static Data Definitions
*/
static DWC$Context established_context;



static void
LAST_CHANCE
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    XtRemoveGrab( w );
    XtUnmanageChild( w );
    XtDestroyWidget( w );
    XFlush( XtDisplay( w ));

#ifdef DEBUG
#ifdef VMS
    LIB$SIGNAL(SS$_DEBUG);
#endif
#endif
    
    exit( 0 );
}

void
DWC$UI_Catchall
#ifdef _DWC_PROTO_
	(
	DWC$FailureType	failure_type,
	int		error_code_1,
	int		error_code_2)
#else	/* no prototypes */
	(failure_type, error_code_1, error_code_2)
	DWC$FailureType	failure_type;
	int		error_code_1;
	int		error_code_2;
#endif	/* prototype */
    {
    static MrmRegisterArg regvec[] =
	{
	    {"errorbox_lastchance",   (caddr_t)LAST_CHANCE},
	    {"errorbox_destroy",      (caddr_t)ERRORDestroyErrorBox},

    };
    static MrmCount regnum = ( sizeof(regvec) / sizeof(regvec[0]) );    

    MrmType	    class ;
    Widget	    msgbox ;
    int		    status ;

    printf("Calendar_Catchall: failure_type: %d, error code #1: %d, #2 %d.\n",
	failure_type, error_code_1, error_code_2);

    if (ads.hierarchy == NULL )
	exit( 0 );

    MrmRegisterNames(regvec, regnum);
    
    status = MrmFetchWidget(ads.hierarchy,
			    "ErrorLastChance",
			    ads.root,
			    &msgbox,
			    &class);

    if (status != MrmSUCCESS)
	{
	printf("Can't fetch last chance box from DRM database");
	exit( 0 );
	};

    XtManageChild (msgbox) ;
    XtRealizeWidget (msgbox);

    ICONSInactiveCursorDisplay( msgbox, ads.inactive_cursor );
    XtAddGrab( msgbox, TRUE, FALSE );

    XtAppMainLoop(CALGetAppContext());
}

void
DWC$UI_Establish
#ifdef _DWC_PROTO_
	(
	DWC$Context	context)
#else	/* no prototypes */
	(context)
	DWC$Context	context;
#endif	/* prototype */
    {
    established_context = context;    
    }
