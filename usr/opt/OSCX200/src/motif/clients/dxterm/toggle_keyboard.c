/*
 *  Title:	Toggle_Keyboard.c
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
 *  Modification history
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Alfred von Campe    14-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 *  Eric Osman          11-June-1992    Sun
 *      - Cast "NULL" to make compiler happier
 *
 *  Aston Chan		05-Mar-1992	V3.1/BL6
 *	- Merge in Shai's fix.  DECterm escape sequences for keyboard switching
 *	  from English to Hebrew do not work when DECterm is displayed on an
 *	  XUI workstation.
 *
 *  Aston Chan		15-Jan-1992	V3.1
 *	- Remove #define NULL 0 because it is complained by DECC compiler.
 *	- Also remove #define TRUE and FALSE.  Use False and True instead.
 *
 *	- Change the enum kb_request definition to enum kb_req and use
 *	  enum kb_req in set_kb_state().  It is complained by DECC compiler.
 *
 *  Aston Chan		19-Dec-1991	V3.1
 *	- Change toggle_keyboard() to DECwTermToggleKeyboard().
 */

/* 
**	Include Files
*/
#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
#include "Xlib"
#include "Xutil"
#include "Xatom"
#include "Km_defs.h"
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "Km_defs.h"
#endif

/*
#define PRIMARY_REQUEST 0
#define SECONDARY_REQUEST 1
#define TOGGLE_CURRENT 255
*/

/*
**	Table of Contents
*/

void _DECwTermToggleKeyboard();
Bool find_mapping();
void set_kb_state();

enum kb_req { PRIMARY_REQUEST =1, SECONDARY_REQUEST , 
                 DISPLAY_PRIMARY, DISPLAY_SECONDARY, TOGGLE_REQUEST = 255 };

Bool find_mapping(dpy, event, arg)
Display *dpy;
XEvent *event;
char   *arg;
{

/*	printf("Find Mapping - event type = %d\n", event->type); */

	if (event->type == MappingNotify && 
            event->xmapping.request == MappingKeyboard)
		{ return(True);}

	return(False);
}

void _DECwTermToggleKeyboard(dpy)
   Display *dpy;
{
   set_kb_state(dpy,TOGGLE_REQUEST);
}

void set_kb_state(dpy, kb_request)
   Display *dpy;
   enum kb_req kb_request;
{
Atom	km_window_id,
	km_group,
	km_group_v1,
	km_group_display,
	v3_ext,
	actual_type;

Window	km_window;

int	actual_format;
unsigned long	leftover;
unsigned long	nitems;
unsigned char	*data = NULL;
unsigned char	*data1 = NULL;
XEvent	ev;
static char *stuff = "ILEG";

XWindowAttributes window_attrib;

km_window_id 		= XInternAtom(dpy,DEC_KM_WINDOW_ID,False);
km_group_v1  		= XInternAtom(dpy,DEC_KM_GROUP,False);
km_group     		= XInternAtom(dpy,DEC_KM_GROUP_V3, False);
v3_ext       		= XInternAtom(dpy,V3_SME_EXT, False);
km_group_display        = XInternAtom(dpy,DEC_KM_GROUP_DISPLAY, False);

if (XGetWindowProperty(dpy,XDefaultRootWindow(dpy), km_window_id, 0L, 1L,
      False, XA_INTEGER, &actual_type, &actual_format, 
      &nitems, &leftover, &data) == 1)
 {
#ifdef DEBUG
	printf("km:  no km window created\n");
#endif
	return;
 }

if (nitems == 0) { 
#ifdef DEBUG
 	printf("km: toggle failed1\n");
#endif
 	return;
 }

km_window = *(Window *)data;

if (km_window == (Window)NULL)  {
#ifdef DEBUG
	printf("km1:  no km window created\n")
#endif
	return;
} 


if (XGetWindowAttributes(dpy, km_window, &window_attrib) == False)
	{
#ifdef DEBUG
		printf("km: Bad km window\n"); 
#endif
		return;
	}

if (XGetWindowProperty(dpy,km_window, km_group, 0L, 1L, False, XA_INTEGER,
      &actual_type, &actual_format, &nitems, &leftover, &data) == 1 )
	{
#ifdef DEBUG
		printf("km: bad km window resources\n");
#endif
		return;
	}

if (nitems == 0) {

        XGetWindowProperty(dpy,km_window, km_group_v1, 0L, 1L, False, XA_INTEGER,
            &actual_type, &actual_format, &nitems, &leftover, &data);

        }
 

switch (kb_request) {
  case PRIMARY_REQUEST :
	if ( (int) *data == 0 )
		return;
	else {
		data = (unsigned char *) ((*data + 1) %2);
		break;
             }
  case SECONDARY_REQUEST :
	if ( (int) *data == 1 )
		return;
	else {
		data = (unsigned char *) ((*data + 1) %2);
		break;
             }
  case TOGGLE_REQUEST :
	data = (unsigned char *) ((*data + 1) % 2);
	break;
  case DISPLAY_PRIMARY :
	data = (unsigned char *)0;
	break;
  case DISPLAY_SECONDARY :
	data = (unsigned char *)1;
	break;
   default :
	return; 				/* shouldn't happen */
 }

switch (kb_request) {
  case PRIMARY_REQUEST :
  case SECONDARY_REQUEST :
  case TOGGLE_REQUEST :

  if (XGetWindowProperty(dpy,km_window, v3_ext, 0L, 1L, False, XA_INTEGER,
      &actual_type, &actual_format, &nitems, &leftover, &data1) == 1 )
	{
#ifdef DEBUG
		printf("km: bad km window resources\n");
#endif
 		return;
	}


  if (nitems == 0)
        {

/*** put this stuff instead (since we are in old mode).***/
            XChangeProperty(dpy, km_window, km_group_v1, XA_INTEGER, 
			32, PropModeReplace, (unsigned char *)&data, 1);

            XFlush(dpy);

            XIfEvent(dpy, &ev, find_mapping, stuff);
            XRefreshKeyboardMapping((XMappingEvent *)&ev);

        }

/*** else continue.***/
  else


  if ( !((int) *data1)  ) {

    XChangeProperty(dpy, km_window, km_group_v1, XA_INTEGER, 32, PropModeReplace,
		(unsigned char *)&data, 1);

    XFlush(dpy);

    XIfEvent(dpy, &ev, find_mapping, stuff);
    XRefreshKeyboardMapping((XMappingEvent *)&ev);

  } else {
    XChangeProperty(dpy, km_window, km_group, XA_INTEGER, 32, PropModeReplace,
		(unsigned char *)&data, 1);
    XFlush(dpy);
  }

    break;

  case DISPLAY_PRIMARY :
  case DISPLAY_SECONDARY :
    XChangeProperty(dpy, km_window, km_group_display, XA_INTEGER, 32,
		    PropModeReplace, (unsigned char *)&data, 1);
    break;
}

return;
}
