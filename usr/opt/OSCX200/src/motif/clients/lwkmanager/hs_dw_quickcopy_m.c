/*
** COPYRIGHT (c) 1989, 1990, 1991, 1992 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/


/*
**++
**  Subsystem:
**	LinkWorks Manager User Interface
**
**  Version: V1.0
**
**  Abstract:
**	Global selection support routines..
**
**  Keywords:
**	{keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Patricia Avigdor
**
**  Creation Date: 7-Mar-1990
**
**  Modification History:
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"
#include "hs_decwindows.h"
#include <X11/Intrinsic.h>

#ifdef VMS
#include <decw$include/Xatom.h>
#else
#include <X11/Xatom.h>
#endif /* VMS */



/*
**  Forward Routine Declarations	
*/

_DeclareFunction(static _Void QuickCopyGotSelection,
    (_Widget widget, caddr_t *closure, Atom *selection, Atom *type, caddr_t *value,
	int *length, int *format));
_DeclareFunction(static _Void QuickCopySendKill,
    (_Widget widget, Time time));
_DeclareFunction(static _Void QuickCopyReceiveClientMessage,
    (_Widget widget, _WindowPrivate private, XClientMessageEvent *event,
	Boolean *continue_to_dispatch));


/*
**  Static Data Definitions
*/
static	Atom	HsAtomTargets;
static	Atom	HsAtomDDIF;
static	Atom	HsAtomKill;
static	Atom	HsAtomStuff;
static	Atom	HsAtomEntry;
static	Atom	HsAtomAtom;


static char svn_translations[] =
    "~Ctrl <Btn2Up>:	QuickCopyCopy()\n\
      Ctrl <Btn2Up>:	QuickCopyMove()" ;
      
/*
** Type definitions
*/
typedef struct {
    _Boolean	    waiting_for_answer;
    Atom	    type;
    caddr_t	    *value;
    _Integer	    length;
    _Integer	    format;
    }_HsClosure;
     

_Void  EnvDWQuickCopyInitialize(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**	Initializes the atoms for quick copy.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    Display		    *display;
    EventMask		    event_mask;
    
    display = XtDisplay(private->main_widget);
        
    HsAtomTargets = XmInternAtom(display, "TARGETS", _False);
    HsAtomDDIF    = XmInternAtom(display, "DDIF", _False);
    HsAtomAtom	  = XmInternAtom(display, "ATOM", _False);

    HsAtomKill	  = XmInternAtom(display, "_DEC_LWK_MGR_KILL_SELECTION",
		    _False);
    HsAtomStuff   = XmInternAtom(display, "_DEC_LWK_MGR_STUFF_SELECTION",
		    _False);
    HsAtomEntry   = XmInternAtom(display, "_DEC_LWK_MGR_ENTRY", _False);

    return;

    }
    

_Void  EnvDWQuickCopyAddHandler(private, action_table, table_size)
_WindowPrivate private;

    XtActionsRec *action_table;
 int table_size;

/*
**++
**  Functional Description:
**	Initializes the atoms for quick copy.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    XtTranslations svntranslations;
    Arg		    arglist[2];
    XtAppContext    appcontext;
    
    /*
    ** Put private as user data to the svn widget.
    */

    XtSetArg(arglist[0], XmNuserData, private);
    XtSetValues(private->svn, arglist, 1);        

    appcontext = XtWidgetToApplicationContext(private->main_widget);
    
    /*
    ** Adds the actions table.
    */
    
    XtAppAddActions(appcontext, action_table, table_size);

    /*
    ** Parses the svn widget translation table.
    */
    
    svntranslations = XtParseTranslationTable(svn_translations);
    
    /*
    ** Adds the translation tables to the existing one.
    */
    
    XtAugmentTranslations(private->svn, svntranslations); 
    
    /*
    ** Add event handler on svn widget for client messages.
    */
    
    XtAddEventHandler(private->svn, NoEventMask, TRUE,
		    (XtEventHandler) QuickCopyReceiveClientMessage,
		    (caddr_t *) private);
    return;
    }


_Boolean  EnvDWQuickCopyConvertSelection
    (widget, selection, target, type, value, length, format)
_Widget widget;
 Atom *selection;
 Atom *target;
 Atom *type;
 caddr_t *value;

    int *length;
 int *format;

/*
**++
**  Functional Description:
**                           
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    Atom	    *targets_array;
    _SvnData	    entry;
    _DDIFString	    ddif_string;
    _Boolean	    conversion;
    lwk_encoding    encoding = (lwk_encoding) 0;
    lwk_integer	    encoding_size = (lwk_integer) 0;
		    
    if (*selection != XA_PRIMARY)
	conversion = _False;

    if (*target == HsAtomTargets) {
	/*
	** Build the targets array.
	*/
	targets_array = (Atom *) _AllocateMem(3 * sizeof(Atom));
	targets_array[0] = HsAtomDDIF;
	targets_array[1] = HsAtomTargets;
	targets_array[2] = HsAtomEntry;
	*value = (caddr_t) targets_array;
	*length = (3 * sizeof(Atom));
	*format = 32;
	*type = HsAtomAtom;
	
	conversion = _True;
    }
    if (*target == HsAtomDDIF) {
	/*
	** Get the current selection.
	*/
	EnvSvnWindowGetSelectedEntry(widget, &entry);
	
	if (entry != (_SvnData) 0) {
	    /*
	    ** Get the selected object name.
	    */
	    EnvDWGetObjectName(entry, &ddif_string);
	    
	    /*
	    ** Fill in the return values.
	    */
	    *length = _LengthDDIFString(ddif_string);
	    *format = 8;
	    *type = HsAtomDDIF;
	    *value = (caddr_t) _CopyDDIFString(ddif_string);
	    _FreeMem(ddif_string);
		     	    
	    conversion = _True;
	}
	else
	    conversion = _False;
    }
    if (*target == HsAtomEntry) {
	/*
	** Get the current selection.
	*/
	EnvSvnWindowGetSelectedEntry(widget, &entry);
	
	if ((entry != (_SvnData) 0) && (!(entry->segment))) {
	    /*
	    ** Get the selected object encoding
	    */
	    EnvDWGetObjectEncoding(entry, &encoding, &encoding_size);
	    
	    /*
	    ** Fill in the return values.
	    */
	    *length = encoding_size;
	    *format = 8;
	    *type = HsAtomEntry;
	    *value = (caddr_t) _AllocateMem(encoding_size);
	    _CopyMem(encoding, *value, encoding_size);
	    if (encoding != (lwk_encoding) 0)	    
		lwk_delete_encoding(&encoding);
	    
	    conversion = _True;
	}
	else
	    conversion = _False;
    }

    return(conversion);
    }
    

_Void  EnvDWQuickCopyLoseSelection
    (widget, selection)
_Widget widget;
 Atom *selection;

/*
**++
**  Functional Description:
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    Window owner;
    
    if (*selection != XA_PRIMARY)
	return;

    /*
    ** Get the selection owner.
    */
    owner = XGetSelectionOwner(XtDisplay(widget), *selection);
    
    /*
    ** Clear the selection.
    */
    if (owner != XtWindow(widget))
        DXmSvnClearSelections(widget);
	
    return;
    }

_Void  EnvDWQuickCopyConversionDone
    (widget, selection, target)
_Widget widget;
 Atom *selection;
 Atom *target;

/*
**++
**  Functional Description:
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    return;
    }

_Boolean  EnvDWQuickCopyGetSelection
    (widget, time, object, move)
_Widget widget;
 Time time;
 lwk_object *object;
 _Boolean move;

/*
**++
**  Functional Description:
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _HsClosure volatile *closure;
    _Boolean volatile value = _True;
    lwk_status   status;
    XtAppContext appcontext;
        
    _StartExceptionBlock
    
    closure = (_HsClosure *) _AllocateMem(sizeof(_HsClosure));
    _ClearMem((_AnyPtr) closure, sizeof(_HsClosure));
    
    closure->waiting_for_answer	    = _True;
    closure->type		    = None;
    closure->value		    = (caddr_t *) 0;
    closure->length		    = (_Integer) 0;
    closure->format		    = (_Integer) 0;
    

    /*
    ** Get the current selected object if any.
    */
    XtGetSelectionValue(widget, XA_PRIMARY, HsAtomEntry,
	QuickCopyGotSelection, (Opaque) closure, time);

    appcontext = XtWidgetToApplicationContext(widget);

    /*
    **	We loop, and process X events, timer events and other events from
    **	alternate input sources until our selection callback gets called.
    */

    while (closure->waiting_for_answer) {
	/*
	**  XtProcessEvent returns when it has processed an X event, a
	**  timer, or an event coming from an alternate input source.
	*/
	XtAppProcessEvent (appcontext, XtIMXEvent | XtIMTimer | XtIMAlternateInput) ;
    };

    if ((closure->type == None) || (closure->type == XT_CONVERT_FAIL)) 
	_Raise(get_selection_failed);

    if (closure->type != HsAtomEntry) 
	_Raise(get_selection_failed);

    /*
    ** Create an object from the encoding returned.
    */
    status = lwk_import(lwk_c_domain_object_desc,
	(lwk_encoding) closure->value, object);	

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(objdsc_decode_error); /*object descriptor decoding failed */
    }

    /*    
    ** Free the storage.
    */
    _FreeMem(closure);

    /*
    ** Send a Kill selection message to the owner of the selection if it was a
    ** move operation(Ctrl/mb3)
    */
    if (move)
	QuickCopySendKill(widget, time);
    
    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */
    
    _Exceptions
	_When(get_selection_failed)
	    _FreeMem(closure);
	    value = _False;
       _WhenOthers
            _Reraise;
	    
    _EndExceptionBlock

    return (value);
    }

static _Void  QuickCopyGotSelection
    (widget, closure, selection, type, value, length, format)
_Widget widget;
 caddr_t *closure;
 Atom *selection;
 Atom *type;

    caddr_t *value;
 int *length;
 int *format;

/*
**++
**  Functional Description:
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	DO NOT raise an exception from this module.  It is
**	a routine called by the DECwindows Toolkit.
**--
*/
    {
    _HsClosure	*closure_ptr = (_HsClosure *) closure;

    closure_ptr->waiting_for_answer = _False;
    closure_ptr->type		    = *type;
    closure_ptr->value		    = (caddr_t *) value;
    closure_ptr->length		    = (_Integer) *length ;
    closure_ptr->format		    = (_Integer) *format ;
	
    return;
    }
    

static _Void  QuickCopySendKill(widget, time)
_Widget widget;
 Time time;

/*
**++
**  Functional Description:
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    XClientMessageEvent cm;

    cm.window = XGetSelectionOwner(XtDisplay(widget), XA_PRIMARY);

    if (cm.window != None) {
	cm.type		= ClientMessage;
	cm.display	= XtDisplay(widget);
	cm.message_type	= HsAtomKill;
	cm.format	= 32;
     	cm.data.l[0]	= (long) XA_PRIMARY;
	cm.data.l[1]	= (long) time;

	XSendEvent(XtDisplay(widget), cm.window, TRUE, NoEventMask,
	    (XEvent *) &cm);
    }
    
    return;
    }
    

static _Void  QuickCopyReceiveClientMessage(widget, private, event, continue_to_dispatch)
_Widget widget;

    _WindowPrivate private;
 XClientMessageEvent *event;

    Boolean *continue_to_dispatch;

/*
**++
**  Functional Description:
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	DO NOT raise an exception from this module.  It is
**	a routine called by the DECwindows Toolkit.
**--
*/
    {
    Window owner;

    if (event->type != ClientMessage)
	return;

    if ((event->message_type == HsAtomKill) &&    
        (event->data.l[0]   == (long) XA_PRIMARY)) {

	/*
	** Get the selection owner.
	*/
	owner = XGetSelectionOwner(XtDisplay(widget), event->data.l[0]);
	
	/*
	** Clear the selection.
	*/
	if (owner != XtWindow(widget))
	    DXmSvnClearSelections(widget);
    } 
    	
    return;
    }
    

_Void  EnvDWQuickCopyDisown(private, timestamp)
_WindowPrivate private;
 Time timestamp;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    Window owner;
    
    owner = XGetSelectionOwner(XtDisplay(private->svn), XA_PRIMARY);

    if (owner == XtWindow(private->svn))
	XtDisownSelection(private->svn, XA_PRIMARY, timestamp);
    
    return;
    }
    
