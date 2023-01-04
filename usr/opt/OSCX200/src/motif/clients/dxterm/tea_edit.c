/* #module TEA_Edit.c "X3.0-3M" */
/*
 *  Title:	TEA_Edit.c
 *
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
 *
 *  Module Abstract:
 *
 *	This module handles the DECterm Edit Menu (Copy, Paste)
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	Tom Porcher  22-Mar-1988
 *
 *  Modification history:
 *
 * Eric Osman		 4-Oct-1993	BL-E
 *	- Fix "clear display / select all / copy" crash by not XtFree'ing
 *	  if XmCvtXmStringToCT returned NULL.  NOTE:  On vxt, I fixed
 *	  XmCvtXmStringToCT to return NULL when appropriate, but I had
 *	  no power over the vms side !  Hence the crash still occurs on
 *	  vms until "they" fix XmCvtXmStringToCT, which hopefully they will
 *	  because I put in a high priority qar for motif library.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     03-May-1993     V1.2/BL2
 *      - Always call XmClipboardEndCopy() after calling XmClipboardCopy().
 *
 * Alfred von Campe     20-Jan-1993     Ag/BL12 (SSB)
 *      - Fix error checking in edit_paste_cb to workaround a bug in Motif
 *        where XmClipboardInquireLength() incorrectly return ClipboardSuccess
 *        instead of ClipboardNoData when the clipboard is empty.
 *
 * Aston Chan		21-Oct-1992	Post V3.1
 *	- Add break's to get_callback_time()'s case statement.
 *	- Use XtFree() to free ct (in copy_proc ) instead of XmStringFree()..
 *
 * Alfred von Campe     14-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 * Eric Osman		13-Oct-1992	VXT V1.2
 *	- Use Motif 1.1 version of tea_edit on vxt V1.2 to solve problem
 *	  of copies and pastes hanging
 *	- Add missing "break" statements in get_callback_time
 *
 * Eric Osman           11-June-1992    Sun
 *      - Some casting for I18n compilation problems
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Aston Chan		Nov-15-1991	V3.1
 *	- Add XmStringFree() to free the XmString created by XmStringCreate()
 *
 * Alfred von Campe     06-Oct-1991     Hercules/1 T0.7
 *      - Changed #include "cupaste.h" to "CutPaste.h"
 *      - Moved DECtermXtMalloc() and DECtermXtCalloc routines from UTIL.C.
 *
 * Alfred von Campe     20-May-1991     V3.0
 *	- Reduced the volume in XBell calls on clipboard errors.
 *
 * Alfred von Campe     04-Apr-1991     V3.0
 *	- Fixed paste bug which would truncate large paste operations and
 *        plugged a memory leak in the same paste routine.
 *
 * Mark Woodbury	24-Apr-1990	x3.0-3M
 *	- Motif conversion.  Added time parameter to DECwTermCopySelection call
 *
 * Bob Messenger	30-May-1989	X2.0-13
 *	- Convert fprintfs to calls to log_message, so messages can be flushed.
 *
 * Bob Messenger	14-May-1989	X2.0-10
 *	- Convert all printf calls to fprintf on stderr.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Convert to new widget bindings (DECwTerm instead of DwtDECterm).
 *
 * Tom Porcher		 7-Sep-1988	X0.5-0
 *	- Change DwtDECtermSelectAll() and DwtDECtermCopySelection() to use
 *	  selection XA_PRIMARY and time xbutton.time.
 *
 * Tom Porcher		20-Jul-1988	X0.4-7
 *	- Change clip_label to a compound string.
 *
 * Peter Sichel         11-Apr-1988     X0.4-7
 *      - Change to convert_widget_to_stream instead of relying
 *        on callback tag which is not available from UIL.
 *
 * Tom Porcher		 5-Apr-1988	X0.4-7
 *	- Change selection callback proc to new Toolkit selection routine
 *	  call format.
 *
 */


#include "mx.h"
#if defined (VMS_DECTERM) || defined (VXT_DECTERM)
#include "CutPaste.h"
#else
#include <Xm/CutPaste.h>
#endif

extern STREAM  *convert_widget_to_stream();
extern char *DECtermXtMalloc();
extern void warn_window();


static Time get_callback_time( call_data )
    XmAnyCallbackStruct *call_data;
{
    Time	time;

    if (call_data->event != NULL)
	switch (call_data->event->type) {
	    case ButtonPress:
	    case ButtonRelease:
		time = call_data->event->xbutton.time;	break;
	    case KeyPress:
	    case KeyRelease:
		time = call_data->event->xkey.time;	break;
	    default:
		time = CurrentTime;
	}
    else
	time = CurrentTime;

    return time;
}

static void copy_proc( w, stm, selectionp, typep, value, lengthp, formatp,
			timestamp )
    Widget w;
    STREAM *stm;
    Atom *selectionp;
    Atom *typep;
    char *value;
    unsigned long *lengthp;
    int *formatp;
    Time timestamp;
/*
    The newer Motif clipboard routines require the time of the event that
    triggered the copy.  This is passed to DECwTermCopySelection so I pass it
    on to here.
*/
{
    int status;
    long cvt_status ;
    XmString clip_label;
    unsigned long item_id;
    int	private_id = 0;
    int data_id;
    XmString cs;
    char *ct, *str;
    long count;

    if ( value == NULL ) {
	log_message( "DECterm:  no current selection\n");
	return;
    }

#ifdef ORIGINAL_CODE
 * clip_label = DwtLatin1String( DECTERM_APPL_TITLE )
#endif

    clip_label = XmStringCreate(DECTERM_APPL_TITLE, XmSTRING_DEFAULT_CHARSET);
    status = XmClipboardStartCopy (XtDisplay(stm->parent),
				  XtWindow(stm->parent),
				  clip_label, timestamp, NULL, NULL, &item_id );

    XmStringFree (clip_label);

    if (status != ClipboardSuccess)
	log_message( "Can't begin copy to clipboard\n");
    else {
	status = XmClipboardCopy( XtDisplay( stm->parent ),
				  XtWindow( stm->parent ),
				  item_id, "DDIF", value, *lengthp,
				  private_id, &data_id );
	if ( status != ClipboardSuccess )
	    log_message( "Can't copy to clipboard\n" );
	cs = (XmString) DXmCvtDDIFtoCS( value, &count, &cvt_status );
	if (cvt_status != DXmCvtStatusOK)
	    {
	    ct = NULL;
	    *lengthp = 0;
	    log_message ("In copy_proc,  DXmCvtDDIFtoCS failed, status = %d\n",
		cvt_status);
	    }
	else
	    {
	    ct = XmCvtXmStringToCT( cs );
	    if (ct)
		*lengthp = strlen( ct );
	    else
		*lengthp = 0;
	    }
	status = XmClipboardCopy( XtDisplay( stm->parent ),
				  XtWindow( stm->parent ),
				  item_id, "COMPOUND_TEXT", ct, *lengthp,
				  private_id, &data_id );
	if (ct) XtFree( ct );
	if ( status != ClipboardSuccess )
	    log_message( "Can't copy to clipboard\n" );
	str = (char *) DXmCvtCStoOS( cs, (long *)lengthp, &cvt_status );
	if (cvt_status != DXmCvtStatusOK)
	    {
	    str = NULL;
	    *lengthp = 0;
	    log_message ("In copy_proc, DXmCvtCStoOS failed, status = %d\n",
		cvt_status);
	    }
	status = XmClipboardCopy ( XtDisplay(stm->parent),
				   XtWindow(stm->parent),
				   item_id, "STRING", str, *lengthp,
				   private_id, &data_id );
	if (str) XtFree( str );
	XmStringFree( cs );
	XmStringFree( value );
       if (status != ClipboardSuccess)
	   log_message( "Can't copy to clipboard\n");

       status = XmClipboardEndCopy (XtDisplay(stm->parent),
				       XtWindow(stm->parent),
				       item_id );
       if (status != ClipboardSuccess)
	    log_message( "Can't end copy to clipboard\n");
    }
}


/*
 * edit_copy_cb( )
 *	The "Copy" button has been pressed.
 */
void
edit_copy_cb( w, isn, call_data )
    Widget	w;
    ISN		isn;
    XmAnyCallbackStruct *call_data;
{
    STREAM	*stm;
    stm = convert_widget_to_stream(w);

    DECwTermCopySelection( stm->terminal, XA_PRIMARY,
		XInternAtom( XtDisplay( stm->terminal ), "DDIF", FALSE ),
			     copy_proc, stm, get_callback_time(call_data) );
}

/*
 * edit_paste_cb()
 *	The "Paste" button has been pressed.
 */
void
edit_paste_cb( w, isn, call_data )
    Widget	w;
    ISN		isn;
    caddr_t	call_data;
{
    STREAM	    *stm = convert_widget_to_stream(w);
    Display         *display = XtDisplay(stm->parent);
    Window          window = XtWindow(stm->parent);
    Time            timestamp = get_callback_time(call_data);
    char	    *buffer;
    unsigned long   length = 0;
    unsigned long   num_bytes = 0;
    int		    private_id;
    XmString	cs;
    int		status, cvt_status;
    Boolean	buffer_flag = False;

    if(XmClipboardStartRetrieve(display, window, timestamp) == ClipboardLocked)
    {
        warn_window(stm, "cant_lock_clipboard_warning", "");
        XBell(display, 0);
        return;
    }

    buffer = NULL;
    if ( XmClipboardInquireLength( display, window,
				  "DDIF", &length ) != ClipboardNoData ) {
	if ( buffer = DECtermXtMalloc( length + 1 )) {
	    status = XmClipboardRetrieve( display, window, "DDIF", buffer,
					  length, &num_bytes, &private_id );
	    if (( status == ClipboardSuccess ) && ( num_bytes > 0 )) {
		buffer[num_bytes] = '\0';
	        DECwTermStuff( stm->terminal,
                    XInternAtom( XtDisplay( stm->terminal ), "DDIF", FALSE ),
                    buffer, num_bytes );
		buffer_flag = True;
	    }
	    XtFree( buffer );
	}
    }
    if ( !buffer_flag ) {
    if ( XmClipboardInquireLength( display, window,
			"COMPOUND_TEXT", &length ) != ClipboardNoData ) {
	if ( buffer = DECtermXtMalloc( length + 1 )) {
	    status = XmClipboardRetrieve( display, window, "COMPOUND_TEXT",
				  buffer, length, &num_bytes, &private_id );
	    if (( status == ClipboardSuccess ) && ( num_bytes > 0 )) {
		buffer[num_bytes] = '\0';
	        DECwTermStuff( stm->terminal,
                    XInternAtom( XtDisplay( stm->terminal ), 
			"COMPOUND_TEXT", FALSE ), buffer, num_bytes );
		buffer_flag = True;
	    }
	    XtFree( buffer );
	}
    }
    }
    if ( !buffer_flag ) {
    if ( XmClipboardInquireLength( display, window,
				   "STRING", &length ) == ClipboardNoData ) {
        log_message( "Attempted to paste while clipboard was empty!\n" );
        XBell( display, 0 );
    } else {
        if (( buffer = DECtermXtMalloc( length + 1 )) == NULL ) {
            warn_window( stm, "cant_paste_warning", "" );
	} else {
            status = XmClipboardRetrieve( display, window, "STRING", buffer,
					  length, &num_bytes, &private_id );
	    if (( status == ClipboardSuccess ) && ( num_bytes > 0 )) {
		buffer[num_bytes] = '\0';
		DECwTermStuff( stm->terminal, XA_STRING, buffer, num_bytes );
		buffer_flag = True;
	    } else {
	      if (status != ClipboardNoData) /* workaround for Motif bug */
	       log_message("Unknown status code %d from XmClipboardRetrieve.\n",
		             status );
	   }
	    XtFree( buffer );
        }
    }
    }
    if(XmClipboardEndRetrieve(display, window) != ClipboardSuccess)
    {
        log_message("Can't unlock clipboard!\n");    /* Can this ever happen? */
        XBell(display, 0);
    }

}
#if 0
/*
 * The following doesn't work because after the third call to DECwTermStuff()
 * we are overflowing the input queues.  It's not the size of the buffer that
 * matters, but rather how often we call DECwTermStuff() before allowing the
 * callback that empties the queues to do its magic.  I'm leaving it in here
 * for future reference.
 */

    if(XmClipboardStartRetrieve(display, window, timestamp) == ClipboardLocked)
    {
        log_message("Clipboard is currently locked by another application.\n");
        XBell(display, 0);
        return;
    }

    buffer = XtMalloc(BUFFER_SIZE);

    do
    {

        status = XmClipboardRetrieve(display, window, "STRING", buffer,
                                     BUFFER_SIZE, &length, &private_id);

        if(status == ClipboardLocked)
        {
            log_message("Can't retrieve clipboard.\n");
            XBell(display, 0);
            break;
        }
        else if(status == ClipboardNoData)
        {
            log_message("Clipboard is empty.\n");
            XBell(display, 0);
            break;
        }
        else if(status == ClipboardSuccess || status == ClipboardTruncate)
        {
            DECwTermStuff(stm->terminal, XA_STRING, buffer, length);
        }
        else
        {
            log_message("Unknown status code from XmClipboardRetrieve.\n");
            XBell(display, 0);
            break;
        }


    } while(status == ClipboardTruncate);

    if(XmClipboardEndRetrieve(display, window) == ClipboardLocked)
    {
        log_message("Can't unlock clipboard!\n");
        XBell(display, 0);
    }

    XtFree(buffer);
#endif

/*
 * edit_selectall_cb()
 *	The "Select all" button has been pressed.
 */
void
edit_selectall_cb( w, isn, call_data )
    Widget	w;
    ISN		isn;
    XmAnyCallbackStruct *call_data;
{
    STREAM	*stm;
    stm = convert_widget_to_stream(w);

    DECwTermSelectAll( stm->terminal, XA_PRIMARY,
			 get_callback_time(call_data) );
}


/*
 * DECwindows applications are supposed to use XtMalloc (not malloc) to ask
 * for memory.  Unfortunately, if XtMalloc fails it doesn't return NULL like
 * malloc does; instead, it invokes a (standard) error handler.  To remedy
 * this problem, DECterm should call DECtermXtMalloc instead, which will
 * return NULL if the requested memory can not be allocated.  Use XtFree to
 * free memory allocated with DECtermXtMalloc.  The same applies to
 * DECtermXtCalloc.
 *
 * Note: These two routines are put here for lack of a better place.  Only
 *       DECtermXtMalloc is used in this module, but both routines were moved
 *       here from UTIL.C, since that module isn't used for dxterm.
 */

globalref XtAppContext TEA_app_context;
static Boolean DECtermXtMallocFailed;
static Boolean DECtermXtCallocFailed;


static void
DECtermXtMallocErrorHandler()
{
    DECtermXtMallocFailed = TRUE;
}


char *
DECtermXtMalloc(size)
int size;
{
char *buffer;
XtErrorMsgHandler oldHandler = XtAppSetErrorMsgHandler(TEA_app_context,
                                (XtErrorMsgHandler)DECtermXtMallocErrorHandler);

    DECtermXtMallocFailed = FALSE;

    buffer = XtMalloc(size);

    XtAppSetErrorMsgHandler(TEA_app_context, oldHandler);

    if(DECtermXtMallocFailed)
        return NULL;
    else
        return buffer;
}


static void
DECtermXtCallocErrorHandler()
{
    DECtermXtCallocFailed = TRUE;
}


char *
DECtermXtCalloc(num, size)
int num, size;
{
char *buffer;
XtErrorMsgHandler oldHandler = XtAppSetErrorMsgHandler(TEA_app_context,
                                (XtErrorMsgHandler)DECtermXtCallocErrorHandler);

    DECtermXtCallocFailed = FALSE;

    buffer = XtCalloc(num, size);

    XtAppSetErrorMsgHandler(TEA_app_context, oldHandler);

    if(DECtermXtCallocFailed)
        return NULL;
    else
        return buffer;
}

