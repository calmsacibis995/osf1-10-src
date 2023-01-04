/* dwc_ui_mba.c */
#ifndef lint
static char rcsid[] = "$Id$";
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
**  Subsystem:
**	Mouse Button Actions (MBA).
**
**  Version: 2.0
**
**  Abstract:
**	This is the MBA tool, which helps overcome the deficiencies of the
**	DECwindows toolkit intrinsics in the area of mouse action support. A
**	set of routines is available for application developers; these routines
**	should be called upon reception of X events; they will return an
**	ACTION, which is the interpretation of the X event in the context of
**	previous X events received.
**
**  Keywords:
**	None.
**
**  Environment:
**	User mode, executable image
**
**  Author:
**	Denis G. Lacroix
**
**  Creation Date: 5-Dec-88
**
**  Modification History:
**  V3-006  Paul Ferwerda					17-Jul-1990
**		Port to Motif, tweaked include files. Changed VoidProc to
**		XmVoidProc. Converted to XtApp.
**	V2-005	Denis G. Lacroix				12-Jun-1989
**		Adding a new routine for use in the day view: MBAResetContext
**	V2-004	Denis G. Lacroix				23-May-1989
**		Fixed a bug which meant that the state engine would be stuck
**		in mode 2 (UP STATE) whereas it should have gone back to mode
**		0 (INITIAL STATE).
**	V2-003	Denis G. Lacroix				10-May-1989
**		Fix the way double click is implemented: a timer was
**		used; time stamps in the events are now used instead.
**	V2-002	Denis G. Lacroix				10-May-1989
**		Increased the 'shaky hand' mouse threshold to 5 pixels.
**	V2-001	Per Hamnqvist					05-Apr-1989
**		Conditionally define NULL
**	V1-004  Denis G. Lacroix			       15-Dec-1988
**		Fixed the way the double click timer is fetched from the
**		resource database.
**	V1-003  Marios Cleovoulou			       14-Dec-1988
**		Pass widget and saved event back to motion_start_proc
**	V1-002  Marios Cleovoulou			       14-Dec-1988
**		Combine MBAPROTO.H into DWC_UI_MBA.H.  Use "LIB$" for includes.
**	V1-001  Denis G. Lacroix	  			5-Dec-1988
**		Initial version.
**--
*/

#include "dwc_compat.h"
#include "dwc_ui_calendar.h"	    /* for CALGetAppContext */
#include "dwc_ui_mba.h"		    /* typedefs to be shared with services' users   */


/*
**  Double click and implied motion default timer value (milliseconds)
*/
#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#define	MBADEFAULTTIMERVALUE   3000
#else
#define	MBADEFAULTTIMERVALUE   250
#endif

/*
**  Mouse threshold: if mouse moves less than MBAPIXELS, then it's not
**  motion, just the user's shaky hand.
*/
#define MBAPIXELS	5

/*
**  Type Definitions
*/
#define	MbaInitialState 0
#define	MbaDownState 1
#define	MbaUpState 2
#define MbaMotionState 3
#define MbaNoTransitionState 4

typedef struct
    {
    XtIntervalId    timer_2_id;
    int		    current_state;
    Boolean	    use_threshold;
    Boolean	    report_motion;
    void	    (*motion_start_proc)();
    Widget	    widget ;
    XEvent	    event ;
    int		    x_root;
    int		    y_root;
    } MbaContextBlock;

typedef enum
    {
    MbaNothing,
    MbaStart,
    MbaCancel
    } MbaTimerAction;


/*
**  To access rows in the state table
*/

#define MB1DOWN		0
#define MB1UP		1
#define MB1DOWNMOTION	2
#define OTHERMOTION	3
#define OTHERXEVENT	4
#define TIMEREVENT	5

typedef struct
    {
    MbaTimerAction  timer_2_action; /* implied motion timer		    */
    MbaAction	    action;
    int	    next_state;	    /* -1 means no state change		    */
    } MbaStateTableCell;

/*
**  The state table we will be using: there is one row for each of the
**  events which can cause a state transition; each column corresponds to a
**  given state (MbaInitialState, MbaDownState, MbaUpState, MbaMotionState).
**  The entries in the table give what should be done with the timers, as well
**  as what should be returned to the caller and the transition which should
**  happen upon an event,
*/

MbaStateTableCell state_table[][4] =
{
    {
    /*
    **  Mouse button 1 down event
    */
{MbaStart, MbaButtonDown, MbaDownState},
    {MbaNothing, MbaIgnoreEvent, MbaNoTransitionState},
	{MbaNothing, MbaDoubleClick, MbaInitialState},
	    {MbaNothing, MbaIgnoreEvent, MbaNoTransitionState}
    },
    
    {
    /*
    **  Mouse button 1 up event
    */
{MbaNothing, MbaIgnoreEvent, MbaNoTransitionState},
    {MbaCancel, MbaButtonUp, MbaUpState},
	{MbaNothing, MbaIgnoreEvent, MbaNoTransitionState},
	    {MbaNothing, MbaButtonUpEndingMotion, MbaInitialState}
    },
    {
    /*
    **  Motion event while Mouse Button 1 is down
    */
{MbaNothing, MbaIgnoreEvent, MbaNoTransitionState},
    {MbaCancel, MbaMotionStart, MbaMotionState},
	{MbaNothing, MbaIgnoreEvent, MbaNoTransitionState},
	    {MbaNothing, MbaMotion, MbaMotionState}
    },
    {
    /*
    **  Other motion event
    */
{MbaNothing, MbaIgnoreEvent, MbaNoTransitionState},
    {MbaNothing, MbaIgnoreEvent, MbaNoTransitionState},
	{MbaNothing, MbaIgnoreEvent, MbaNoTransitionState},
	    {MbaNothing, MbaMotion, MbaMotionState}
    },
    {
    /*
    **  Other mouse events
    */    
{MbaNothing, MbaIgnoreEvent, MbaNoTransitionState},
    {MbaCancel, MbaCancelAction, MbaInitialState},     
	{MbaNothing, MbaIgnoreEvent, MbaInitialState},
	    {MbaNothing, MbaCancelMotion, MbaInitialState}
    },
    /*
    **  Timer events
    */    
    {
{MbaNothing, MbaIgnoreEvent, MbaNoTransitionState},
    {MbaCancel, MbaCallback, MbaMotionState},
	{MbaNothing, MbaNoAction, MbaInitialState},
	    {MbaCancel, MbaNoAction, MbaNoTransitionState}
    }
};

/*
**  One variable to store the timer values for both double click and implied
**  motion, and another to store whether are not we already fetched the timer
**  value. 
*/

Boolean		TimerAlreadyFetched = FALSE;
unsigned long	MbaTimerValue;

/*
**  Prototypes for local convenience routines
*/

static int
timer_callback_proc PROTOTYPE ((
	caddr_t		client_data,
	XtIntervalId	*id));

static MbaAction
go_through_state_table PROTOTYPE ((
	int		event,
	XEvent		*xevent,
	MbaContextBlock	*ctxt_block_ptr));


/*
**++
**  Functional Description:
**	MBAInitContext
**	Initializes the MBA.
**
**  Keywords:
**	None.
**
**  Arguments:
**	widget:		    Saved in context block
**	use_threshold:	    whether the threshold feature should be used when
**			    dealing with motion events.
**	report_motion:	    motion events should be ignored.
**	motion_start_proc:  the routine to callback when implied motion starts.
**
**  Result:
**	returns a pointer to an opaque context block which should be used in
**	subsequent calls to the MBA services.
**
**  Exceptions:
**	None.
**--
*/
caddr_t MBAInitContext
#ifdef _DWC_PROTO_
	(
	Widget		widget,
	Boolean		use_threshold,
	Boolean		report_motion,
	void		(*motion_start_proc)())
#else	/* no prototypes */
	(widget, use_threshold, report_motion, motion_start_proc)
	Widget		widget;
	Boolean		use_threshold;
	Boolean		report_motion;
	void		(*motion_start_proc)();
#endif	/* prototype */
{
    MbaContextBlock	*ctxt_block_ptr;
    char		*representation;
    XrmValue		value;
    
    /*
    **	Let's try to initialize the double click delay value using Xrm, if
    **	it hasn't been done already. It looks like value and representation
    **	don't have to be freed.
    */
    if ( !TimerAlreadyFetched )
    {
	XrmGetResource
	(
	    XtDatabase(XtDisplay(widget)),
	    "doubleClickDelay",
	    NULL,
	    &representation,
	    &value
	);
	if (value.size > 0)
	{
	    MbaTimerValue = atoi(value.addr);
	}
	else
	{
	    MbaTimerValue = MBADEFAULTTIMERVALUE;
	}
#if DEBUG
	MbaTimerValue = MBADEFAULTTIMERVALUE;
#endif	    
	TimerAlreadyFetched = TRUE;
    }
    
    /*
    **  Allocates and initializes the context block
    */
    ctxt_block_ptr = (MbaContextBlock *)XtMalloc( sizeof(MbaContextBlock) );

    ctxt_block_ptr->timer_2_id		= 0;
    ctxt_block_ptr->current_state	= MbaInitialState;
    ctxt_block_ptr->use_threshold	= use_threshold;
    ctxt_block_ptr->report_motion	= report_motion;
    ctxt_block_ptr->motion_start_proc	= motion_start_proc;
    ctxt_block_ptr->widget		= widget;

    /*
    **  That's it
    */
    return( (caddr_t)ctxt_block_ptr );
}

/*
**++
**  Functional Description:
**	MBAFreeContext
**	When done with the MBA services, should be called by the user so that
**	the data structures can be deallocated.
**
**  Keywords:
**	None.
**
**  Arguments:
**	context:    a pointer to an opaque context block previously returned by
**		    a call to MBAInitContext.
**
**  Result:
**	None.
**
**  Exceptions:
**	None.
**--
*/void
MBAFreeContext
#ifdef _DWC_PROTO_
	(
	caddr_t	context)
#else	/* no prototypes */
	(context)
	caddr_t	context;
#endif	/* prototype */
    {
    XtFree(context);    
    
    /*
    **  That's it
    */
    return;
    }

/*
**++
**  Functional Description:
**	MBAResetContext
**	This routines resets the state engine to its initial state; pending
**	timers are removed.
**
**  Keywords:
**	None.
**
**  Arguments:
**	context:    a pointer to an opaque context block previously returned by
**		    a call to MBAInitContext.
**
**  Result:
**	The interpretation of the X event.
**
**  Exceptions:
**	None.
**--
*/
void MBAResetContext
#ifdef _DWC_PROTO_
	(
	caddr_t	context)
#else	/* no prototypes */
	(context)
	caddr_t	context;
#endif	/* prototype */
{
    MbaContextBlock *ctxt_block_ptr;    

    ctxt_block_ptr = (MbaContextBlock *)context;

    ctxt_block_ptr->current_state = MbaInitialState;
    if (ctxt_block_ptr->timer_2_id != 0)
    {
	XtRemoveTimeOut( ctxt_block_ptr->timer_2_id );
	ctxt_block_ptr->timer_2_id = 0;
    }

    return;
}

/*
**++
**  Functional Description:
**	MBAMouseButton1Down
**	Should be called to obtain the interpretation of a Mouse Button 1 Down
**	event. This routine will read the state table, and transition to the
**	appropriate state.
**
**  Keywords:
**	None.
**
**  Arguments:
**	context:    a pointer to an opaque context block previously returned by
**		    a call to MBAInitContext.
**	event:	    self explanatory.
**
**  Result:
**	The interpretation of the X event.
**
**  Exceptions:
**	None.
**--
*/
MbaAction
MBAMouseButton1Down
#ifdef _DWC_PROTO_
	(
	caddr_t	context,
	XEvent	*event)
#else	/* no prototypes */
	(context, event)
	caddr_t	context;
	XEvent	*event;
#endif	/* prototype */
    {
    MbaContextBlock *ctxt_block_ptr;
    XButtonEvent    *button_event;
    

    ctxt_block_ptr = (MbaContextBlock *)context;
    button_event = (XButtonEvent *)event;

    ctxt_block_ptr->x_root = button_event->x_root;
    ctxt_block_ptr->y_root = button_event->y_root;
    
    return( go_through_state_table(MB1DOWN, event, ctxt_block_ptr) );
    }


/*
**++
**  Functional Description:
**	MBAMouseButton1Up
**	Should be called to obtain the interpretation of a Mouse Button 1 Up
**	event. This routine will read the state table, and transition to the
**	appropriate state.
**
**  Keywords:
**	None.
**
**  Arguments:
**	context:    a pointer to an opaque context block previously returned by
**		    a call to MBAInitContext.
**	event:	    self explanatory.
**
**  Result:
**	The interpretation of the X event.
**
**  Exceptions:
**	None.
**--
*/
MbaAction
MBAMouseButton1Up
#ifdef _DWC_PROTO_
	(
	caddr_t	context,
	XEvent	*event)
#else	/* no prototypes */
	(context, event)
	caddr_t	context;
	XEvent	*event;
#endif	/* prototype */
    {
    MbaContextBlock *ctxt_block_ptr;

    ctxt_block_ptr = (MbaContextBlock *)context;

    return( go_through_state_table(MB1UP, event, ctxt_block_ptr) );
    }

/*
**++
**  Functional Description:
**      MBAMouseButtonOther
**	Should be called to obtain the interpretation of any Mouse Button event,
**	but Mouse Button 1 event. This routine will read the state table, and
**	transition to the appropriate state.
**
**  Keywords:
**	None.
**
**  Arguments:
**	context:    a pointer to an opaque context block previously returned by
**		    a call to MBAInitContext.
**	event:	    self explanatory.
**
**  Result:
**	The interpretation of the X event.
**
**  Exceptions:
**	None.
**--
*/
MbaAction
MBAMouseButtonOther
#ifdef _DWC_PROTO_
	(
	caddr_t	context,
	XEvent	*event)
#else	/* no prototypes */
	(context, event)
	caddr_t	context;
	XEvent	*event;
#endif	/* prototype */
    {
    return( go_through_state_table(OTHERXEVENT, event, (MbaContextBlock *)context) );
    }

/*
**++
**  Functional Description:
**      MBAMouseButton1Motion
**	Should be called to obtain the interpretation of a motion event while
**	MMouse Button 1 is down. This routine will read the state table, and
**	transition to the appropriate state; it will also take the appropriate
**	steps in order to handle the motion threshold problem, according to what
**	the user specified in MBAInitContext.
**
**  Keywords:
**	None.
**
**  Arguments:
**	context:    a pointer to an opaque context block previously returned by
**		    a call to MBAInitContext.
**	event:	    self explanatory.
**
**  Result:
**	The interpretation of the X event.
**
**  Exceptions:
**	None.
**--
*/
MbaAction
MBAMouseButton1Motion
#ifdef _DWC_PROTO_
	(
	caddr_t	context,
	XEvent	*event)
#else	/* no prototypes */
	(context, event)
	caddr_t	context;
	XEvent	*event;
#endif	/* prototype */
    {
    MbaContextBlock *ctxt_block_ptr;
    XButtonEvent    *button_event;
    
    ctxt_block_ptr  = (MbaContextBlock *)context;
    button_event    = (XButtonEvent *)event;
    
    if ( !ctxt_block_ptr->report_motion ||
	 (ctxt_block_ptr->current_state != MbaMotionState &&
	    ctxt_block_ptr->use_threshold &&
	    abs(button_event->x_root - ctxt_block_ptr->x_root) <= MBAPIXELS &&
	    abs(button_event->y_root - ctxt_block_ptr->y_root) <= MBAPIXELS)
       )
	{
	return ( MbaIgnoreEvent );	
	}
    else
	{
	return( go_through_state_table(MB1DOWNMOTION, event, ctxt_block_ptr) );
	}
    }

/*
**++
**  Functional Description:
**	go_through_state_table
**	Given an event which corresponds to a row in the state table, this
**	routine will transition to the appropriate state, setting up or
**	cancelling the appropriate timers.
**	!!!: the implied motion timer should probably not be started if the
**	motion_start_proc wasn't specified in MBAInitContext.
**
**  Keywords:
**	None.
**
**  Arguments:
**	event:	    MB1DOWN, MB1UP, MB1DOWNMOTION, OTHERMOTION, OTHERXEVENT,
**		    or TIMEREVENT.
**	context:    a pointer to an opaque context block previously returned by
**		    a call to MBAInitContext.
**
**  Result:
**	The interpretation of the X event.
**
**  Exceptions:
**	None.
**--
*/
static MbaAction
go_through_state_table
#ifdef _DWC_PROTO_
	(
	int		event,
	XEvent		*xevent,
	MbaContextBlock	*ctxt_block_ptr)
#else	/* no prototypes */
	(event, xevent, ctxt_block_ptr)
	int		event;
	XEvent		*xevent;
	MbaContextBlock	*ctxt_block_ptr;
#endif	/* prototype */
    {
    MbaAction	return_value;
    int		current_state;
    Time	old_timestamp,
		new_timestamp;

    /*
    **  The current state is saved
    */
    current_state = ctxt_block_ptr->current_state;

    /*	  
    **  If we have a state change
    */	  
    if ( state_table[event][current_state].next_state != MbaNoTransitionState)
	{
	if ( (current_state == MbaUpState) &&
	     (event == MB1DOWN) )
	    {
	    /*
	    **	The last event we have received was MB1UP; since we are now
	    **	receiving MB1DOWN, let's check whether what we are seeing is
	    **	a double click
	    */
	    old_timestamp = ctxt_block_ptr->event.xbutton.time;
	    new_timestamp = ((XButtonEvent *)xevent)->time;
	    if (((new_timestamp - old_timestamp) <= MbaTimerValue) &&
		((XButtonEvent *)xevent)->button != Button2)
		{
		/*
		**  Double Click
		*/
		return_value = MbaDoubleClick;
		ctxt_block_ptr->current_state = MbaInitialState;
		}
	    else
		{
		/*  
		**  Not a double click. Let's fix the current state so that
		**  we behave as if we had received MB1DOWN in a virgin
		**  environment.
		*/
		current_state = MbaInitialState;
		/*
		return_value = MbaButtonDown;
		ctxt_block_ptr->current_state = MbaDownState;
		*/
		}
	    }

        /*	  
	**  If the last even we received was not an MB1UP or the last event was
	**  an MB1UP and we're currently receiving something other than an
	**  MB1DOWN
	*/	  
        if ( (current_state != MbaUpState) ||
	     ( (current_state == MbaUpState) && (event != MB1DOWN) ) )
	    {
	    ctxt_block_ptr->current_state =
		state_table[event][current_state].next_state;
	    return_value = state_table[event][current_state].action;
	    if ( (event  == MB1UP) &&
	         (xevent != NULL ) )
		{
		memcpy (&(ctxt_block_ptr->event), xevent, sizeof (XEvent));
		}
	    }
	}
    else
	{
	/*
	**  We don't transition since there is no state change
	*/
	return_value = state_table[event][current_state].action;	
	}

    /*
    **  Let's see what we have to do with the implied motion timer
    */    	
    if ( event != TIMEREVENT )
    {
	switch (state_table[event][current_state].timer_2_action)
	{
	/*
	**  Implied motion timer
	*/
	case MbaStart:
#if DEBUG
	    if ( ctxt_block_ptr->timer_2_id != 0)
		printf("Bug: timer_2_id not NULL\n");
#endif
	    ctxt_block_ptr->timer_2_id = XtAppAddTimeOut
	    (
		CALGetAppContext(),
		MbaTimerValue,
		(XtTimerCallbackProc)timer_callback_proc,
		ctxt_block_ptr
	    );
	    memcpy (&(ctxt_block_ptr->event), xevent, sizeof (XEvent));
	    break;
	case MbaCancel:
#if DEBUG
	    if ( ctxt_block_ptr->timer_2_id == 0)
		printf("timer_2_id is NULL\n");
#endif
	    if (ctxt_block_ptr->timer_2_id != 0)
	    {
		XtRemoveTimeOut( ctxt_block_ptr->timer_2_id );
		ctxt_block_ptr->timer_2_id = 0;
	    }
	    break;
	case MbaNothing:
	    break;
	default:
	    break;
	}
    }
	
    /*
    **  That's it
    */
    return( return_value );
}

/*
**++
**  Functional Description:
**	timer_callback_proc
**	This is the callback routine which will be called by the toolkit when
**	the double click timer or the implied motion timer trigger.
**
**  Keywords:
**	None.
**
**  Arguments:
**	client_data:	a pointer to an MBA context block.
**	id:		the id of the timer which triggered.
**
**  Result:
**	None.
**
**  Exceptions:
**	None.
**--
*/
static int
timer_callback_proc
#ifdef _DWC_PROTO_
	(
	caddr_t		client_data,
	XtIntervalId	*id)
#else	/* no prototypes */
	(client_data, id)
	caddr_t		client_data;
	XtIntervalId	*id;
#endif	/* prototype */
    {
    MbaAction	    action;
    MbaContextBlock *ctxt_block_ptr;
	
    /*
    **	The client data we passed is really a context block pointer
    */
    ctxt_block_ptr = (MbaContextBlock *)client_data;

#if DEBUG
	printf("Timer callback procedure (implied motion)\n");
#endif

    if ( (ctxt_block_ptr->timer_2_id == *id) &&
         (ctxt_block_ptr->report_motion)     &&
	 (ctxt_block_ptr->motion_start_proc != NULL) )
	{
	/*
	**  We want to go thru the state table only if it is the
	**  implied motion timer, only if the user asked for motion
	**  events to be reported and if a motion callback procedure was
	**  specified.
	*/
	action = go_through_state_table(TIMEREVENT, NULL, ctxt_block_ptr);
	if ( action == MbaCallback )
	    {
	    (*ctxt_block_ptr->motion_start_proc)
	      (ctxt_block_ptr->widget, &(ctxt_block_ptr->event));
	    }
	}


    /*
    **  We rely on timer_1_id and timer_2_id being NULL when there are no
    **	pending timers
    */
    if (ctxt_block_ptr->timer_2_id == *id)
	{
	/*
	**  It was the implied motion timer
	*/
	ctxt_block_ptr->timer_2_id = 0;
	}

    /*
    **  That's it
    */    
    return;
}

