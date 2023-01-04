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
 *  Copyright (c) Digital Equipment Corporation, 1990  
 *  All Rights Reserved.  Unpublished rights reserved
 *  under the copyright laws of the United States.
 *  
 *  The software contained on this media is proprietary
 *  to and embodies the confidential technology of 
 *  Digital Equipment Corporation.  Possession, use,
 *  duplication or dissemination of the software and
 *  media is authorized only pursuant to a valid written
 *  license from Digital Equipment Corporation.
 *
 *  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 *  disclosure by the U.S. Government is subject to
 *  restrictions as set forth in Subparagraph (c)(1)(ii)
 *  of DFARS 252.227-7013, or in FAR 52.227-19, as
 *  applicable.
 */

/******************************************************************************/
/*                                                                            */
/*   FACILITY:                                                                */
/*                                                                            */
/*       Error message                                                        */
/*                                                                            */
/*   ABSTRACT:                                                                */
/*                                                                            */
/*       This module contains routines to output a series of error            */
/*       messages to a message box.                                           */
/*                                                                            */
/*   AUTHOR: Tom Scarpelli                                                    */
/*                                                                            */
/*   CREATION DATE:     4-Feb-1988                                            */
/*                                                                            */
/*   MODIFICATION HISTORY:                                                    */
/*                                                                            */
/*   003    Q&D port to Motif    					      */
/*          9/14/89  Leo                                                      */
/*   002    10-Mar-1988 TS                                                    */
/*          Convert output to CS for message box.                             */
/*   001    4-Feb-1988 TS                                                     */
/*          Created module.                                                   */
/*                                                                            */
/******************************************************************************/


/*********************************************/
/*                                           */
/* INCLUDE FILES                             */
/*                                           */
/*********************************************/

#include <Xm/Xm.h>
#include <X11/StringDefs.h>
#include <Xm/MessageB.h>
#include "DXmPrivate.h"


#ifdef VMS
#include <sys$library:descrip.h>
#include <sys$library:stsdef.h>
#else
#endif

/*********************************************/
/*                                           */
/* TABLE OF CONTENTS                         */
/*                                           */
/*********************************************/

#ifdef DWTVMS
Widget 		DXM$DISPLAY_CS_MESSAGE();
#endif

Widget 		DXmDisplayCSMessage();
static Widget 	DisplayCSMessage();
static Widget	CreateMessageBox();
static int 	MsgRoutine();
static int	CSMsgRoutine();

#ifdef VAXC	    /* ??? Why is this #ifdef'd VAXC instead of VMS ??? */
Widget 		DXM$DISPLAY_VMS_MESSAGE();
Widget 		DXmDisplayVmsMessage();
static Widget 	DisplayVmsMessage();
#endif

/*********************************************/
/*                                           */
/* MACRO DEFINITIONS                         */
/*                                           */
/*********************************************/

#define K_FORMAT_VMS  0		/* Use descriptor for user arg		    */
#define K_FORMAT_CC   1		/* Use null terminated string for user arg  */
#define K_FORMAT_CS   2		/* Use CS format for user arg		    */
#define K_ARG_COUNT   8		/* Message Box arguments		    */
#define K_BUF_SIZE    256	/* Fao buffer sizes			    */

/*********************************************/
/*                                           */
/* STRUCTURE DEFINITIONS                     */
/*                                           */
/*********************************************/

struct args
    {
    int  len;			/* Length of built up message string  */
    char *address;		/* Address of built up message string */
    int	 flags;			/* Type of user arg (K_FORMATxxx)     */
    int	 *msgvec;		/* Users message vector as passed     */
    int  (*user)();		/* Address of users routine           */
    };

/*********************************************/
/*                                           */
/* EXTERNAL AND GLOBAL VARIABLES DEFINITIONS */
/*                                           */
/*********************************************/         

extern Widget 	  XmCreateMessageDialog();/* Message Box widget               */
extern XmString   XmStringLtoRCreate();	/* Convert string to CS             */
extern char	 *DXmDescToNull();	/* Convert desc to null term string */
extern int	  FaoDL();		/* Format a string                  */

#ifdef	VAXC
globalvalue	SS$_NORMAL;		/* Normal successful completion     */
extern int 	SYS$PUTMSG();		/* Retrieve and format an error msg */
#ifndef STS$M_SEVERITY
#define STS$M_SEVERITY 7
#define STS$M_SUCCESS 1
#define STS$V_SEVERITY   0x00
#define STS$V_SUCCESS    0x00
#define $VMS_STATUS_SUCCESS(code) 	( ( (code) & STS$M_SUCCESS ) 	>> STS$V_SUCCESS )
#define $VMS_STATUS_SEVERITY(code) 	( ( (code) & STS$M_SEVERITY ) 	>> STS$V_SEVERITY )
#endif
#endif


#ifdef	VAXC

#define ReadDesc(source_desc, addr, len) \
    LIB$ANALYZE_SDESC(source_desc,&len,&addr)

/*---------------------------------------------------*/
/* This is a VMS specific routine for converting a   */
/* string descriptor to a null terminated string.    */
/*---------------------------------------------------*/
char *DXmDescToNull(desc)
     struct dsc$descriptor_s *desc;
{
    char          *nullterm_string;
    char          *temp_string;
    unsigned short temp_length;

        if (desc->dsc$b_class <= DSC$K_CLASS_D) {
            temp_length = desc->dsc$w_length;
            temp_string = desc->dsc$a_pointer;
        }
        else
            ReadDesc(desc,temp_string,temp_length);

        nullterm_string = (char *) XtMalloc (temp_length+1);

        if (temp_length != 0)
            bcopy(temp_string,nullterm_string,temp_length);

        *(nullterm_string+temp_length) = '\0';  /* Make it null-terminated */

        return (nullterm_string);
}
#endif /* VAXC */


#ifdef	VAXC

Widget
DXM$DISPLAY_VMS_MESSAGE( Parent, Name_desc, Pos, X, Y, Style, Msgvec, 
			Widget_id, User_routine, Ok_callback, Help_callback )

/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*       This routine accepts a VMS standard message vector,                  */
/*       creates a message box if necessary, and outputs the                  */
/*       error message(s) to the specified message box.  It                   */
/*       is then up to the caller to MANAGE the message widget.               */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */

    Widget *Parent;		/*Widget id of parent widget                  */
    struct dsc$descriptor_s *Name_desc;	/* Name used to define message box    */
    int    *Pos; 		/* Flag indicating positioning of message box */
    int    *X;			/* Placement of left side of message box      */
    int    *Y;			/* Placement of upper side of message box     */
    int    *Style;		/* Flag indicating modal or modeless          */
    int    *Msgvec;		/* VMS message vector to be passed to $PUTMSG */
    Widget *Widget_id;		/* Widget id of existing msg box or 0 if none */
    int    (*User_routine)();	/* Routine called after each message is       */
				/* formatted but before it is displayed       */
    XtCallbackList Ok_callback;	/* Callback executed when "Acknowledged"      */
				/* button is selected                         */
    XtCallbackList Help_callback;/* Callback executed when help is requested  */

/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   FUNCTION VALUE                                                           */
/*                                                                            */
/*       Widget id of message box widget is returned.                         */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       A message box widget may be created.                                 */
/*                                                                            */
/******************************************************************************/

	{
	Widget	W;
	char	*Name;

	Name = DXmDescToNull( Name_desc );

	W = DisplayVmsMessage( *Parent, 
				Name, 
				Pos   == NULL ? 1 : *Pos, 
				X     == NULL ? 0 : *X, 
				Y     == NULL ? 0 : *Y, 
				Style == NULL ? 0 : *Style, 
				Msgvec,
				Widget_id, 
				User_routine, 
				Ok_callback, 
				Help_callback, 
				K_FORMAT_VMS );

	XtFree( Name );

	return W;

	}



Widget
DXmDisplayVmsMessage( Parent, Name, Pos, X, Y, Style, Msgvec, Widget_id, 
			User_routine, Ok_callback, Help_callback )

/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*       This routine accepts a VMS standard message vector,                  */
/*       creates a message box if necessary, and outputs the                  */
/*       error message(s) to the specified message box.  It                   */
/*       is then up to the caller to MANAGE the message widget.               */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */

    Widget Parent;		/*Widget id of parent widget                  */
    char   *Name;		/* Name used to define message box            */
    int    Pos; 		/* Flag indicating positioning of message box */
    int    X;			/* Placement of left side of message box      */
    int    Y;			/* Placement of upper side of message box     */
    int    Style;		/* Flag indicating modal or modeless          */
    int    *Msgvec;		/* VMS message vector to be passed to $PUTMSG */
    Widget *Widget_id;		/* Widget id of existing msg box or 0 if none */
    int    (*User_routine)();	/* Routine called after each message is       */
				/* formatted but before it is displayed       */
    XtCallbackList Ok_callback;	/* Callback executed when "Acknowledged"      */
				/* button is selected                         */
    XtCallbackList Help_callback;/* Callback executed when help is requested  */

/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   FUNCTION VALUE                                                           */
/*                                                                            */
/*       Widget id of message box widget is returned.                         */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       A message box widget may be created.                                 */
/*                                                                            */
/******************************************************************************/

	{
	return DisplayVmsMessage( Parent, Name, Pos, X, Y, Style, Msgvec,
				  Widget_id, User_routine, Ok_callback,
				  Help_callback, K_FORMAT_CC );
	}

#endif	/* VAXC */


#ifdef DWTVMS
Widget
DXM$DISPLAY_CS_MESSAGE( Parent, Name_desc, Pos, X, Y, Style, Msgvec, 
			Widget_id, User_routine, Ok_callback, Help_callback )

/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*       This routine accepts a message vector where the message codes        */
/*       are pointers to CS strings, creates a message box if necessary,      */
/*	 and outputs the error message(s) to the specified message box.  It   */
/*       is then up to the caller to MANAGE the message widget.               */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */

    Widget *Parent;		/*Widget id of parent widget                  */
    struct dsc$descriptor_s *Name_desc;	/* Name used to define message box    */
    int    *Pos; 		/* Flag indicating positioning of message box */
    int    *X;			/* Placement of left side of message box      */
    int    *Y;			/* Placement of upper side of message box     */
    int    *Style;		/* Flag indicating modal or modeless          */
    int    *Msgvec;		/* Message vector containing CS pointers      */
    Widget *Widget_id;		/* Widget id of existing msg box or 0 if none */
    int    (*User_routine)();	/* Routine called after each message is       */
				/* formatted but before it is displayed       */
    XtCallbackList Ok_callback;	/* Callback executed when "Acknowledged"      */
				/* button is selected                         */
    XtCallbackList Help_callback;/* Callback executed when help is requested  */

/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   FUNCTION VALUE                                                           */
/*                                                                            */
/*       Widget id of message box widget is returned.                         */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       A message box widget may be created.                                 */
/*                                                                            */
/******************************************************************************/

	{
	Widget	W;
	char	*Name;

	Name = DXmDescToNull( Name_desc );

	W = DisplayCSMessage( *Parent, 
				Name, 
				Pos   == NULL ? 1 : *Pos, 
				X     == NULL ? 0 : *X, 
				Y     == NULL ? 0 : *Y, 
				Style == NULL ? 0 : *Style, 
				Msgvec, 
				Widget_id, 
				User_routine, 
				Ok_callback, 
				Help_callback, 
				K_FORMAT_VMS );

	XtFree( Name );

	return W;

	}
#endif /* DWTVMS */


Widget
DXmDisplayCSMessage( Parent, Name, Pos, X, Y, Style, Msgvec, Widget_id, 
			User_routine, Ok_callback, Help_callback )

/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*       This routine accepts a message vector where the message codes        */
/*       are pointers to CS strings, creates a message box if necessary,      */
/*	 and outputs the error message(s) to the specified message box.  It   */
/*       is then up to the caller to MANAGE the message widget.               */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */

    Widget Parent;		/*Widget id of parent widget                  */
    char   *Name;		/* Name used to define message box            */
    int    Pos; 		/* Flag indicating positioning of message box */
    int    X;			/* Placement of left side of message box      */
    int    Y;			/* Placement of upper side of message box     */
    int    Style;		/* Flag indicating modal or modeless          */
    long   *Msgvec;		/* Message vector containing CS pointers      */
    Widget *Widget_id;		/* Widget id of existing msg box or 0 if none */
    int    (*User_routine)();	/* Routine called after each message is       */
				/* formatted but before it is displayed       */
    XtCallbackList Ok_callback;	/* Callback executed when "Acknowledged"      */
				/* button is selected                         */
    XtCallbackList Help_callback;/* Callback executed when help is requested  */

/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   FUNCTION VALUE                                                           */
/*                                                                            */
/*       Widget id of message box widget is returned.                         */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       A message box widget may be created.                                 */
/*                                                                            */
/******************************************************************************/

	{
	return DisplayCSMessage( Parent, Name, Pos, X, Y, Style, Msgvec,
				  Widget_id, User_routine, Ok_callback,
				  Help_callback, K_FORMAT_CC );
	}


#ifdef	VAXC

static Widget
DisplayVmsMessage( Parent, Name, Pos, X, Y, Style, Msgvec, Widget_id, 
		   User_routine, Ok_callback, Help_callback, Flags )

/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*       This routine accepts a VMS standard message vector,                  */
/*       creates a message box if necessary, and outputs the                  */
/*       error message(s) to the specified message box.  It                   */
/*       is then up to the caller to MANAGE the message widget.               */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */

    Widget Parent;		/*Widget id of parent widget                  */
    char   *Name;		/* Name used to define message box            */
    int    Pos; 		/* Flag indicating positioning of message box */
    int    X;			/* Placement of left side of message box      */
    int    Y;			/* Placement of upper side of message box     */
    int    Style;		/* Flag indicating modal or modeless          */
    int    *Msgvec;		/* VMS message vector to be passed to $PUTMSG */
    Widget *Widget_id;		/* Widget id of existing msg box or 0 if none */
    int    (*User_routine)();	/* Routine called after each message is       */
				/* formatted but before it is displayed       */
    XtCallbackList Ok_callback;	/* Callback executed when "Acknowledged"      */
				/* button is selected                         */
    XtCallbackList Help_callback;/* Callback executed when help is requested  */
    int    Flags;		/*  Flag indicating C format or VMS format    */

/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   FUNCTION VALUE                                                           */
/*                                                                            */
/*       Widget id of message box widget is returned.                         */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       A message box widget may be created.                                 */
/*                                                                            */
/******************************************************************************/

	{
	int		S, *p, *q, ac = 0;
	short int	L, i;
	struct args 	Call_Block;
	char		*New_addr, *msg;
	XmString	label;
	Arg 		al[K_ARG_COUNT];
	Widget		w_id = 0;
        unsigned char	type;		    /* message box dialog type */

	/*************************************************/
	/* Initialize descriptor for total error message.*/
	/* Setup the user's callback routine, if any.    */
	/*************************************************/

	Call_Block.len     = 0;
	Call_Block.address = 0;
	Call_Block.flags   = Flags;
	Call_Block.user    = User_routine;

	/*****************************************************/
	/* Make our own copy of the message vector to make   */
	/* sure $PUTMSG returns only the text of the message */
	/*****************************************************/

	if ( Msgvec == NULL ) return 0;

	L = *Msgvec;
	q = Msgvec;
	S = L * 4 + 4;		/* size of Msgvec in bytes */

	p = msg = XtMalloc( S );

	for (i=0; i<=L; i++) 
	    *p++ = *q++;	/* make a local copy of Msgvec */

	msg[2] = 1;		/* force text only output from $PUTMSG */
	Call_Block.msgvec = msg;

	/*****************************************************/
	/* Call $PUTMSG to do most of the work for us.  This */
	/* will break up the message vector and call our     */
	/* callback routine for each one.  When we return,   */
	/* the message string will be ready to be output.    */
	/*****************************************************/

	S = SYS$PUTMSG( Call_Block.msgvec, MsgRoutine, 0, &Call_Block );

	/*************************************************/
	/* If the message is empty, nothing to do!	 */
	/*************************************************/

	if ( Call_Block.len == 0 || S != SS$_NORMAL) return 0;

	/*************************************************/
	/* Create the message box widget if necessary.   */
	/*************************************************/

	if ( Widget_id == NULL ) Widget_id = &w_id;

	if ( *Widget_id == 0 ) {
	    switch ($VMS_STATUS_SEVERITY(msg[1])) {
		case STS$K_WARNING:
		    type = XmDIALOG_WARNING;
		    break;
		case STS$K_SUCCESS:
		case STS$K_INFO:
		    type = XmDIALOG_INFORMATION;
		    break;
		case STS$K_ERROR:
		case STS$K_SEVERE:
		    type = XmDIALOG_ERROR;
		    break;
		default:
		    type = XmDIALOG_MESSAGE;
		    break;
		}
	    *Widget_id = CreateMessageBox( Parent, Name, Pos, X, Y, Style,
					   Ok_callback, Help_callback,
					   type);
	    }

	/*************************************************/
	/* Set up the label for the message box.	 */
	/* Convert the label to CS format.		 */
	/*************************************************/

	New_addr = DXmDescToNull( &Call_Block.len );
	label    = XmStringLtoRCreate( New_addr, "ISO8859-1" );

	XtSetArg( al[0], XmNmessageString, label );
	XtSetValues( *Widget_id, al, 1 );

	/****************************************************/
	/* Free the memory used to hold the message strings */
	/* and return the id of the created message widget. */
	/****************************************************/

	XmStringFree( label );
	XtFree( New_addr );
	XtFree( Call_Block.msgvec );
	XtFree( Call_Block.address );

	return *Widget_id;
	}


static int
MsgRoutine( Msg_desc, Call_Block )

/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*       This routine is the callback routine from the $PUTMSG                */
/*       system service.  It builds up the entire message by                  */
/*       appending the new message to the currently built up one,             */
/*       separating them with a <cr><lf> pair.                                */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */

    struct dsc$descriptor_s *Msg_desc;	/* Text of error msg already formatted*/
    struct args             *Call_Block;/* struct containing additional info  */

/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   FUNCTION VALUE                                                           */
/*                                                                            */
/*       A 0 is returned to $PUTMSG to prevent it from outputting             */
/*       the error message to SYS$OUTPUT and/or SYS$ERROR.                    */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       Virtual memory is allocated                                          */
/*                                                                            */
/******************************************************************************/

	{
	int	Len = 0, S;
	char	*Addr, *New_addr, crlf[] = "\r\n";

	/******************************************/
	/* Call back to the user if so requested. */
	/******************************************/

	if ( Call_Block->user != NULL ) 
	    {

	    switch ( Call_Block->flags )
		{

		case K_FORMAT_CC:
		    New_addr = DXmDescToNull( Msg_desc ); 
		    break;

		case K_FORMAT_VMS:
		    New_addr = Msg_desc;
		    break;

		case K_FORMAT_CS:
		    New_addr = Msg_desc->dsc$a_pointer; 
		    break;

		}

	    S = (*Call_Block->user)(New_addr);

	    switch ( Call_Block->flags )
		{

		case K_FORMAT_CC:
		    XtFree( New_addr );
		    break;

		case K_FORMAT_VMS:
		    break;

		case K_FORMAT_CS:
		    break;

		}

	    if ( S == 0 ) return 0;

	    }

	/******************************************/
	/* Get length and address of new message. */
	/******************************************/

	S = ReadDesc( Msg_desc, Addr, Len );
	if ( S != SS$_NORMAL ) return 0;

	if ( Len != 0 )
	    {

	    if ( Call_Block->len != 0 )
		{

		/******************************************/
		/* Allocate enough memory to hold old and */
		/* new messages plus <cr><lf> separator.  */
		/******************************************/

		New_addr = XtMalloc( Len + Call_Block->len + 2 );

		/*******************************************/
		/* Copy previous message to new string.    */
		/*******************************************/
		 
		DXmBCopy (Call_Block->address, New_addr, Call_Block->len);

		/*******************************************/
		/* Add <cr><lf> pair to separate messages. */
		/*******************************************/

		New_addr[ Call_Block->len ]     = '\r';
		New_addr[ Call_Block->len + 1 ] = '\n';

		/*****************************************/
		/* Free the previously allocated memory. */
		/*****************************************/

		XtFree( Call_Block->address );

		/***************************************************/
		/* Append the new message to the end of the buffer */
		/***************************************************/

		DXmBCopy(Addr, &New_addr[Call_Block->len + 2], Len);

		/***************************************/
		/* Update our info for the new buffer. */
		/***************************************/

		Call_Block->len = Call_Block->len + Len + 2;
		Call_Block->address = New_addr;

		}
	    else
		{

		/***************************************************/
		/* Allocate enough memory to hold the new message. */
		/***************************************************/

		New_addr = XtMalloc( Len );

		/*****************************************/
	        /* Copy the new message into the buffer. */
		/*****************************************/

		DXmBCopy(Addr, New_addr, Len);

		/***************************************/
	        /* Update our info for the new buffer. */
		/***************************************/

	        Call_Block->len = Len;
	        Call_Block->address = New_addr;

		}
	    }

	return 0;

	}

#endif


static Widget
CreateMessageBox( Parent, Name, Pos, X, Y, Style, Ok_callback, Help_callback, Type )

/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*       This routine creates a message box.                                  */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */

    Widget Parent;		/*Widget id of parent widget                  */
    char   *Name;		/* Name used to define message box            */
    int    Pos; 		/* Flag indicating positioning of message box */
    int    X;			/* Placement of left side of message box      */
    int    Y;			/* Placement of upper side of message box     */
    int    Style;		/* Flag indicating modal or modeless          */
    XtCallbackList Ok_callback;	/* Callback executed when "Acknowledged"      */
				/* button is selected                         */
    XtCallbackList Help_callback;/* Callback executed when help is requested  */
    unsigned char   Type;	/* message box dialog type		      */
/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   FUNCTION VALUE                                                           */
/*                                                                            */
/*       Widget id of the created message box widget is returned.             */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       A message box widget is created.                                     */
/*                                                                            */
/******************************************************************************/

	{
	int ac = 0;
	Arg al[K_ARG_COUNT];
	Widget	box, cancel_button;

	/****************************************************/
	/* Set up the arguements to the message box widget. */
	/****************************************************/

	XtSetArg( al[ac], XmNdefaultPosition, Pos );
	ac++;

	XtSetArg( al[ac], XmNx, X );
	ac++;

	XtSetArg( al[ac], XmNy, Y );
	ac++;

	if ( Style != 0 ) 
	    {
	    XtSetArg( al[ac], XmNdialogStyle, Style );
	    ac++;
	    }

	if ( Type != 0 )
	    {
	    XtSetArg( al[ac], XmNdialogType, Type );
	    ac++;
	    }

	if ( Ok_callback != NULL ) 
	    {
	    XtSetArg( al[ac], XmNokCallback, Ok_callback );
	    ac++;
	    }

	if ( Help_callback != NULL ) 
	    {
	    XtSetArg( al[ac], XmNhelpCallback, Help_callback );
	    ac++;
	    }

	/*********************************************************/
	/* Now create the message box with the specified params. */
	/* and unmanage the cancel button since we don't want it */
	/*********************************************************/

	box = XmCreateMessageDialog( Parent, Name, al, ac );

	cancel_button = XmMessageBoxGetChild(box, XmDIALOG_CANCEL_BUTTON);
	XtUnmanageChild(cancel_button);

	return(box);
	}


static int
CSMsgRoutine( Cs_Msg, Call_Block )

/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*       This routine builds up the entire message by                         */
/*       appending the new message to the currently built up one,             */
/*       separating them with a <cr><lf> pair.                                */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */

    XmString	    *Cs_Msg;	/* Text of error msg already formatted*/
    struct args             *Call_Block;/* struct containing additional info  */

/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   FUNCTION VALUE                                                           */
/*                                                                            */
/*	none                                                                  */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       Virtual memory is allocated                                          */
/*                                                                            */
/******************************************************************************/

	{
	int		S, crlf_len;
	XmString	CrLf, *Addr, New_addr;
	static char	crlf_ascii[] = "\r\n";

	/******************************************/
	/* Get address of new message. 		  */
	/******************************************/

	Addr = Cs_Msg;

	/******************************************/
	/* Call back to the user if so requested. */
	/******************************************/

	if ( Call_Block->user != NULL ) 
	    {
	    S = (*Call_Block->user)(Addr);
	    if ( S == 0 ) return 0;
	    }

	/*****************************************************/
	/* Build up one string containing multiple messages. */
	/*****************************************************/

	if ( !XmStringEmpty ( (XmString) Addr ))
	    {

	    if ( Call_Block->len != 0 )
		{

		/***********************************************/
		/* Convert <cr><lf> pair to a compound string. */
		/***********************************************/

		CrLf = XmStringLtoRCreate( crlf_ascii, "ISO8859-1" );
		crlf_len = XmStringLength( CrLf );

		/*******************************************/
		/* Copy previous message to new string.    */
		/*******************************************/
		 
		New_addr = XmStringCopy( (XmString) Call_Block->address );

		/*******************************************/
		/* Add <cr><lf> pair to separate messages. */
		/*******************************************/

		New_addr = XmStringConcat( New_addr, CrLf );

		/***************************************************/
		/* Append the new message to the end of the buffer */
		/***************************************************/

		New_addr = XmStringConcat( New_addr, (XmString) Addr );

		/*****************************************/
		/* Free the previously allocated memory. */
		/*****************************************/

		XtFree( Call_Block->address );
		XmStringFree( CrLf );

		}
	    else
		{

		/*****************************************/
	        /* Copy the new message into the buffer. */
		/*****************************************/

		New_addr = XmStringCopy( (XmString) Addr );

		}

	    /***************************************/
            /* Update our info for the new buffer. */
	    /***************************************/

	    Call_Block->len = XmStringLength( New_addr );
	    Call_Block->address = (char *)New_addr;

	    }

	return 0;

	}


static Widget
DisplayCSMessage( Parent, Name, Pos, X, Y, Style, Msgvec, Widget_id, 
		    User_routine, Ok_callback, Help_callback, Flags )

/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*       This routine accepts a message vector where the message codes        */
/*       are pointers to CS strings, creates a message box if necessary,      */
/*	 and outputs the error message(s) to the specified message box.  It   */
/*       is then up to the caller to MANAGE the message widget.               */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */

    Widget Parent;		/* Widget id of parent widget                 */
    char   *Name;		/* Name used to define message box            */
    int    Pos; 		/* Flag indicating positioning of message box */
    int    X;			/* Placement of left side of message box      */
    int    Y;			/* Placement of upper side of message box     */
    int    Style;		/* Flag indicating modal or modeless          */
    long   *Msgvec;		/* Message vector containing CS pointers      */
    Widget *Widget_id;		/* Widget id of existing msg box or 0 if none */
    int    (*User_routine)();	/* Routine called after each message is       */
				/* formatted but before it is displayed       */
    XtCallbackList Ok_callback;	/* Callback executed when "Acknowledged"      */
				/* button is selected                         */
    XtCallbackList Help_callback;/* Callback executed when help is requested  */
    int    Flags;		/*  Flag indicating C format or VMS format    */

/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*       none                                                                 */
/*                                                                            */
/*   FUNCTION VALUE                                                           */
/*                                                                            */
/*       Widget id of message box widget is returned.                         */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       A message box widget may be created.                                 */
/*                                                                            */
/******************************************************************************/

	{
	int 		S;
	long 		*msg_code, *fao_args, *fao_vector;
	short int 	i = 0, num_fao_args;
	long		num_longwords;
	XmString	msg_buffer = NULL;
	struct args 	Call_Block;
	Arg 		al[K_ARG_COUNT];
	Widget		w_id = 0;

	/*************************************************/
	/* Initialize descriptor for total error message.*/
	/* Setup the user's callback routine, if any.    */
	/*************************************************/

	Call_Block.len     = 0;
	Call_Block.address = 0;
	Call_Block.flags   = K_FORMAT_CS;
	Call_Block.user    = User_routine;

	/*******************************************/
	/* The first word in the first longword    */
	/* is the number of longwords that follow. */
	/*******************************************/

	if ( Msgvec == NULL ) return 0;

	msg_code = Msgvec;
	num_longwords = *msg_code;

	while (i<num_longwords)
	    {

	    /***************************************************/
	    /* Set up pointers into the message vector for the */
	    /* next message and its FAO parameters (if any).   */
	    /***************************************************/

	    fao_args     = ++msg_code;	  /* point to next message string */
	    num_fao_args = (short int) *(++fao_args); /* point to num of
							 FAO args */
	    fao_vector   = ++fao_args;	  /* point to actual FAO args */

	    /***********************************************/
	    /* Now call Fao to format the message for us.  */
	    /***********************************************/

	    S = FaoDL( *msg_code, &msg_buffer, fao_vector);
	    if ( S != 1 ) msg_buffer = (XmString)*msg_code;

	    msg_code += (long)num_fao_args + 1;
	    i += num_fao_args + 2;

	    /************************************************/
	    /* Call common routine to do message processing */
	    /************************************************/

	    CSMsgRoutine( msg_buffer, &Call_Block );
	    if ( S == 1 ) XtFree((char *) msg_buffer );

	    }			/* end of while */

	/*************************************************/
	/* If the message is empty, nothing to do!	 */
	/*************************************************/

	if ( Call_Block.len == 0 ) return 0;

	/*************************************************/
	/* Create the message box widget if necessary.   */
	/*************************************************/

	if ( Widget_id == NULL ) Widget_id = &w_id;

	if ( *Widget_id == 0 ) {
	    *Widget_id = CreateMessageBox( Parent, Name, Pos, X, Y, Style,
					  Ok_callback, Help_callback, 
					  XmDIALOG_INFORMATION );
	    }
	/*************************************************/
	/* Set up the label for the message box.	 */
	/*************************************************/

	XtSetArg( al[0], XmNmessageString, Call_Block.address );
	XtSetValues( *Widget_id, al, 1 );

	/****************************************************/
	/* Free the memory used to hold the message strings */
	/* and return the id of the created message widget. */
	/****************************************************/

	XtFree( Call_Block.address );

	return *Widget_id;

	}

