/* #module DT_version "X0.5-9" */
/*
 *  Title:	TEA_info_boxes.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights       |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	Routines for manipulating info boxes, typically ones that the
 *	user has to respond to.  Such boxes are put up to show warnings
 *	that unexpected events have occurred, such as a file failure,
 *	or bad status from an internal procedure.
 *
 *  Author:	Eric Osman
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Aston Chan		23-Apr-1993	V1.2/BL2
 *	Startup/Fullname support.
 *	- Change TEA_display to be globalref.
 *
 *  Alfred von Campe    14-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 *  Eric Osman          11-June-1992    Sun
 *      - Add some casting for I18n compilation errors
 *
 *  Alfred von Campe    02-Apr-1992     Ag/BL6.2.1
 *	- Use noshare only if VMS.
 *
 *  Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Eric Osman		11-Nov-1991	V3.1
 *	- If stm is NON_STREAM, use a global shell for warning window.
 *
 *  Bob Messenger	02-Jul-1990	X3.0-5
 *	- Fix bug in Motif update: get XmNmessageString instead of
 *	  XmNlabelString.
 *
 *  Mark Woodbury	25-May-1990	X3.0-3M
 *	- Motif Update
 *
 *  Bob Messenger	16-Aug-1989	X2.0-16
 *	- If the shell hasn't been realized yet, just write an error
 *	  message and return.
 *
 *  Bob Messenger	30-May-1989	X2.0-13
 *	- Convert fprintf's to calls to log_message, so the messages can be
 *	  flushed.
 *
 *  Bob Messenger	14-May-1989	X2.0-10
 *	- Convert printf to fprintf on stderr.
 *
 *  Bob Messenger	19-Apr-1989	X2.0-6
 *	- Exit with status on VMS.
 *	- Call process_exit instead of exit.
 *
 *  Bob Messenger	15-Feb-1989	X1.1-1
 *	- Fix Ultrix compilation warning.
 *
 *  Eric Osman		14-Oct-1987	X0.2
 *	First release.
 *
 *  <modifier's name>	<date>		<ident of revised code>
 *	<description of change and purpose of change>
 */


#include "mx.h"

#ifdef VMS_DECTERM
#include "DECspecific.h"

globalvalue DECW$_CANT_FETCH_WIDGET;

#else

#include <DXm/DECspecific.h>
#define DECW$_CANT_FETCH_WIDGET 0

#endif

extern char *get_text();

globalref MrmHierarchy s_MRMHierarchy;    /* DRM database id */
globalref MrmType *dummy_class;           /* and class var */
globalref XtAppContext TEA_app_context;


/*
! Table of contents
*/

extern void	warning_window ();


#ifdef VMS_DECTERM
noshare Display *warn_display;
noshare Widget warn_shell;
#else
Display *warn_display;
Widget warn_shell;
#endif

globalref Display *TEA_display;

make_warn_shell ()
{
globalref def_x, def_y;
Arg arglist[10];

        XtSetArg( arglist[0], "x", def_x);
        XtSetArg( arglist[1], "y", def_y);
	warn_shell = XtAppCreateShell (DECTERM_APPL_NAME, DECTERM_APPL_CLASS,
	    applicationShellWidgetClass, TEA_display, arglist, 2);
	if (! warn_shell)
	    {
	    log_message ("Can't XtAppCreateShell for warning boxes.\n");
	    XtCloseDisplay (warn_display);
	    return;
	    }
}
/*
 * Procedure to present a warning window.
 *
 *	Usage:
 *
 *		warn_window (stream, id, param, param...);
 *
 *		The params stuff into string just like printf.
 *
 *		If stm is NON_STREAM, global context is used, and created first
 *		if necessary.
 */
void	warn_window (stm, id, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
STREAM *stm;
char *id;
void *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10 ;
{
    Arg arglist[1];
    Widget slot;
    XmString warn_string;
    char 	message[1000];
    char	*format;
    Widget shell;
    int status;
    XmString cs;
    long count, cvt_status;
/*
 * Make sure the shell widget has been realized.
 */
    if (stm != (STREAM *)NON_STREAM && ! XtIsRealized( stm->parent ) )
	{
	log_message( "Warning message %s before shell was realized\n", id );
	return;
	}
/*
 * If no stream has been supplied, and no shell has been established for
 * warning windows yet, do it.  If can't, just log the message and give up.
 */
    if (stm != (STREAM *)NON_STREAM) shell = stm->parent;
    else
	{
	if (! warn_shell) make_warn_shell ();
	if (! warn_shell)
	    {
	    log_message ("Undisplayed DECterm warning tag is: \"%s\"\n", id);
	    return;
	    }
	else shell = warn_shell;
	}
/*
 * Fetch file selection box from DRM.
 * We don't exit entire controller if warning box fails.  This allows for
 * possibility of user continuing to use their other DECterms even after quota
 * errors.
 */
    if ((status=MrmFetchWidget (s_MRMHierarchy, id, shell, &slot,
	& dummy_class)) != MrmSUCCESS)
            {   
	    log_message(
	    "Unable to fetch \"%s\" warning box, MRM error code = %d\n",
		id, status);
	    return;
            }
/*
 * Get text from widget which we'll use as format string, have sprintf
 * fill in the parameters.  Use resultant string as new string for box.
 */
    XtSetArg( arglist[0], XmNmessageString, &warn_string);
    XtGetValues( slot, arglist, 1);
    format = get_text (warn_string);
    sprintf (message, format, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
    XtFree (format);
    cs = DXmCvtFCtoCS( message, &count, &cvt_status );
    XtSetArg( arglist[0], XmNmessageString, cs );
    XtSetValues( slot, arglist, 1);
    XmStringFree( cs );
    XtManageChild(slot);
#if defined (VMS_DECTERM) || defined(VXT_DECTERM)
	log_message( "%s\n", message );
#endif
}

/*
 * Come here when user has acknowledged the box.
 */
void warn_window_cb (widget)
Widget widget;
{XtDestroyWidget(widget);}
