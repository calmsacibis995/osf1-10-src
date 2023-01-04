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
*/

/*
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

#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
#ifdef DWTVMS
#else
#define DWTVMS
#endif
#endif

#ifndef VAXC
#define const
#endif


/*
 * externals for library routines go here
 */

extern void              DXmChangeWindowGeometry ();
extern XtGeometryResult  DXmMakeGeometryRequest ();

/*
 *  Macros
 */

#define XtNumChildren(w) (((CompositeWidget)(w))->composite.num_children)
#define XtChildren(w)	 (((CompositeWidget)(w))->composite.children)

#define WindowObjectParent(obj,w)				\
    for (w = (Widget) obj;					\
	 (w != NULL) && ( ! XtIsWindowObject (w)); 		\
	 w = w->core.parent)					\
	;

#define DXMGETWIDGETPARAMS(pw, px, py, pwidth, pheight, pborderWidth)     \
    (px)          = (pw)->core.x;                                         \
    (py)          = (pw)->core.y;                                         \
    (pwidth)      = (pw)->core.width;                                     \
    (pheight)     = (pw)->core.height;                                    \
    (pborderWidth)= (pw)->core.border_width;                              

#define DXMCOPYWIDGETGEOMETRY(sw, dw)                                     \
    (dw)->core.x            = (sw)->core.x;                               \
    (dw)->core.y            = (sw)->core.y;                               \
    (dw)->core.width        = (sw)->core.width;                           \
    (dw)->core.height       = (sw)->core.height;                          \
    (dw)->core.border_width = (sw)->core.border_width;                

#define DXM_EVENT_TO_TIME(time, event)					  \
    switch ((event)->type) {						  \
        case KeyPress: 							  \
        case KeyRelease: 						  \
		(time) = (event)->xkey.time; break;			  \
        case ButtonPress: 						  \
        case ButtonRelease: 						  \
 		(time) = (event)->xbutton.time; break;			  \
	case MotionNotify: 						  \
		(time) = (event)->xmotion.time; break;			  \
	case EnterNotify: 						  \
	case LeaveNotify: 						  \
		(time) = (event)->xcrossing.time; break;		  \
	case PropertyNotify:						  \
		(time) = (event)->xproperty.time; break;		  \
	case SelectionClear: 						  \
		(time) = (event)->xselectionclear.time; break; 		  \
	case SelectionRequest: 						  \
		(time) = (event)->xselectionrequest.time; break;	  \
	case SelectionNotify: 						  \
		(time) = (event)->xselection.time; break;		  \
	default: 							  \
		(time) = CurrentTime; break;				  \
    }

/*
 *  Error Message Macros
 */
#define DXMERROR(key, message)						  \
    XtErrorMsg (key, "dxmlibError", "DXmlibError", message, NULL, NULL)
#define DXMWARNING(key, message)					  \
    XtWarningMsg (key, "dxmlibWarning", "DXmlibWarning", message, NULL, NULL)
#define DXMDEBUGWARNING(key, message)

#define DXMAPPERROR(app_context, key, message, params, num_params)	  \
    XtAppErrorMsg (app_context, key, "dxmlibError", "DXmlibError", message, params, num_params)
#define DXMAPPWARNING(app_context, key, message, params, num_params)	  \
    XtAppWarningMsg (app_context, key, "dxmlibWarning", "DXmlibWarning", message, params, num_params)


#ifdef UNALIGNED

#define DXmBCopy(src, dst, size)				    \
    if (size == sizeof(int))				    \
	*((int *) (dst)) = *((int *) (src));		    \
    else if (size == sizeof(char))			    \
	*((char *) (dst)) = *((char *) (src));		    \
    else if (size == sizeof(short))			    \
	*((short *) (dst)) = *((short *) (src));	    \
    else						    \
	bcopy((char *) (src), (char *) (dst), (int) (size));

#else

#define DXmBCopy(src, dst, size)		\
	bcopy((char *) (src), (char *) (dst), (int) (size));

#endif /* UNALIGNED */



#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
