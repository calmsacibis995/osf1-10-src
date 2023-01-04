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
#define Module DVR_UTIL
#define Ident  "V02-017"

/*
**++
**   COPYRIGHT (c) 1988, 1991 BY
**   DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**   ALL RIGHTS RESERVED.
**
**   THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**   ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**   INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**   COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**   OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**   TRANSFERRED.
**
**   THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**   AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**   CORPORATION.
**
**    DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**    SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**
** MODULE NAME:
**	dvr_util.c (vms, os/2)
**	dvr_util.c (ultrix)
**
** FACILITY:
**
**	Compound Document Architecture (CDA)
**	Compound Document Viewers
**	DIGITAL Document Interchange Format (DDIF)
**
** ABSTRACT:
**	This module contains various utility routines used by the viewer
**
** ENVIORNMENT:
**	vms, ultrix, os/2
**
** AUTHORS:
**      Dennis McEvoy,  02-Dec-1988
**
**
** MODIFIED BY:
**
**	V02-017		RKN000		Ram Kumar Nori		23-Jul-1992
**		Removed PARENT_X_OFFSET & PARENT_Y_OFFSET macros from 
**		dvr_calculate_max_bbox routines.
**
**	V02-016		RDH003		Don Haney		15-Jul-1992
**		Specify same-directory includes with "", not <>
**
**      V02-015	        KLM0001		Kevin McBride		22-Jun-1992
**		make dvs error callback routine match protos with engine
**
**      V02-014	        KLM0001		Kevin McBride		12-Jun-1992
**		Alpha OSF/1 port
**
**      V02-013	        DAM0001		Dennis McEvoy		18-oct-1991
**		more cleanups to match protos
**
**      V02-012         JJT0001         Jeff Tancill            16-sep-1991
**              remove local XtIsRealized definition, not needed
**              anymore since NULL is defined as 0 now.
**
**      V02-011	        DAM0001		Dennis McEvoy		05-aug-1991
**		renamed headers, removed dollar signs
**
**      V02-010	        DAM0001		Dennis McEvoy		06-may-1991
**		move dvr_int include to cleanup ultrix build
**
**	V02-009		RAM000		Ralph A. Mack		24-Apr-1991
**		Adding #ifdefs for MS-Windows
**
**	V02-008		DAM000		Dennis McEvoy		03-apr-1991
**		cleanup typedefs
**
**	V02-007		RTG001		Dick Gumbel		05-Mar-1991
**		Cleanup #include's
**
**	V02-006		DAM000		Dennis McEvoy		01-mar-1991
**		convert to new typedefs
**
**	V02-005		DAM000		Dennis McEvoy		08-Jan-1991
**		add tollkit message logging routine
**
**	V02-004		SJM000		Stephen Munyan		28-Jun-1990
**		Conversion to Motif
**
**	V02-003		DAM001		Dennis McEvoy		05-mar-1990
**		changes for os/2 port
**
**	V02-002		RTG001		Richard T. Gumbel	3-Aug-1989
**		Remove duplicate include of stsdef.h
**
**	V02-001		DAM001		Dennis A. McEvoy	24-Apr-1989
**		Remove call to XSetFont() from dvr_get_font_info();
**		dvr_get_font_info() is now called from format-phase;
**		XSetFont() is called in display phase;
**--
*/


/*
**
**  INCLUDE FILES
**
**/
#include <cdatrans.h>

#ifdef __vms__
#include <dvrint.h>				/* DVR internal definitions */

#pragma nostandard				/* turn off /stand=port for
						   "unclean" X include files */

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#pragma standard				/* turn /stand=port back on */

#endif

#ifdef OS2

#define INCL_PM 				/* tell OS/2 to include Presentation Manager defs */
#define PMMLE_INCLUDED                          /* do not include multi-line editing defs */
#include <os2.h>                                /* OS/2 defs */

#include <dvrint.h>				/* DVR internal definitions */
#endif

#ifdef MSWINDOWS
#define NOKERNEL				/* Omit unused definitions */
#define NOCTLMGR				/* from parse of windows.h */
#define NOMETAFILE				/* to save memory on compile. */
#define NOMSG					/* Because we can't say NOUSER */
#define NOSOUND					/* for this module, we must  */
#define NOCOMM					/* flip a whole lot of switches */
#define NOSCROLL				/* for the components of USER */
#define NOSHOWWINDOW				/* that we don't need */
#define NOCLIPBOARD
#define NOMENUS
#define NOSYSMETRICS
#define NOWH
#define NOMB
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#include <windows.h>				/* MS-Windows definitions. */
#undef ERROR

#include <dvrint.h>				/* DVR internal definitions */
#endif

#ifdef __unix__

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#include <dvrint.h>				/* DVR internal definitions */

/*
 * redefine XtIsRealized() to be portable
 */
#ifdef XtIsRealized
#undef XtIsRealized
#endif
#define XtIsRealized(widget) 					\
   (XtIsWidget(widget)  ?					\
      ((Widget)(widget))->core.window != (Window) NULL  :	\
      ((Object)(widget))->object.parent->core.window != (Window) NULL)

#endif

#include "dvrwdef.h"				/* Public Defns w/dvr_include */
#include "dvrwint.h"				/* Viewer Constants and structures  */
#include "dvrwptp.h"				/* dvr windowing prototypes */



/*
**++
**  ROUTINE NAME:
**	dvr_error_callback(parameter, severity, status, buffer, page_count)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine is called to report an error in the viewer or the
**	DVS engine. It calls the viewer widget's callback routines with
**	a status and optionally, an ASCIZ text string.
**
**	The status argument is optional if the severity and buffer
**	arguments are supplied.  Likewise, the severity and buffer
**	arguments are optional if status is supplied.  The page_count
**	is currently not used.
**
**  FORMAL PARAMETERS:
**    DvrViewerWidget	parameter;  required, DvrViewerWidget (vw) context
**    CDAstatus 	severity;   optional, DVS_WARNING, DVS_ERROR, etc
**    CDAstatus		status;	    optional, error status to report
**    unsigned char 	buffer[512];optional, error text message
**    CDAcount		page_count; optional, page # error appears on.
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	DVR_NORMAL
**
**  SIDE EFFECTS:
**	Calls back to the application using XtCallCallbacks.
**--
**/

CDAstatus dvr_error_callback(parameter, severity, status, buffer, page_count)
    DvrViewerWidget 	parameter;
    CDAconstant		severity;
    CDAstatus 		status;
    CDAenvirontext 	buffer[512];
    CDAuint32		page_count;

{
    DvrCallbackStruct dvr_reason;

    /* the parameter passed in is the widget */
    DvrViewerWidget vw = (DvrViewerWidget) parameter;

    /* if a blank status is passed in by the engine, fill in
     * a bit more info based on the severity.
     */
    if (status == 0)
	{
	switch ((int) severity)
	{

	case DVS_K_SUCCESS:
	    /* engine shouldn't call us with success status,
	     * but handle it anyway.
	     */
	    status = DVR_NORMAL;
	    break;

	case DVS_K_WARNING:
	    status = DVR_FORMATWARN;
	    break;

	case DVS_K_INFORMATION:
	    status = DVR_FORMATINFO;
	    break;

	case DVS_K_ERROR:
	case DVS_K_FATAL:
	    status = DVR_FORMATERROR;
	    break;

	default:
	    /* shouldn't happen */
	    return DVR_INTERNALERROR;
	}  /* end severity switch */
	}  /* end no status set	    */

    /* plug status into widget and the callback structure */
    vw->dvr_viewer.Dvr.WidgetStatus = status;
    dvr_reason.status		    = status;

    dvr_reason.reason = DvrCRcdaError;
    dvr_reason.string_ptr = (char *) buffer;

    /* call back to the application with this  */

    if (status != DVR_NORMAL)
        dvr_application_callback (vw,
				  &dvr_reason);


    return DVR_NORMAL;
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	dvr_set_watch_cursor(vw)
**	 - If widget has not yet had a watch cursor created for it;
**	   make it;
**	 - increment the cursor reference count
**	 - call macro to set cursor to watch if it isn't already a watch.
**
**  FORMAL PARAMETERS:
**
**  pDvrWindow  - viewer work window widget
**
**--
*/

CDAstatus dvr_set_watch_cursor(vw)
    DvrViewerWidget vw;


{
    /* pointer to the viewer part of Widget */
    DvrViewerPtr pDvrWindow = &(vw->dvr_viewer);

    dvr_load_cursor(vw);

    /* increment the reference count.  Asynchronous waits may occur
    *  so we want to be careful not to set the cursor back from a
    *  watch before all the waits are finished.
    *
    * call macro to change cursor to watch
    */

    if (pDvrWindow->cursor_ref_count++ == 0)
	StartWait(vw);

    return(DVR_NORMAL);
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	dvr_reset_cursor(vw)
**	 - decrements cursor reference count
**	 - if all waits are satisfied, calls macro to reset cursor
**	   to original value
**
**  FORMAL PARAMETERS:
**
**  vw - viewer widget
**
**--
*/

CDAstatus dvr_reset_cursor(vw)

    DvrViewerWidget vw;		/* pointer to the viewer Widget */

{
    /* pointer to the viewer part of Widget */
    DvrViewerPtr pDvrWindow = &(vw->dvr_viewer);

    if (--pDvrWindow->cursor_ref_count == 0)
       StopWait(vw);

    return(DVR_NORMAL);
}



/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_init_font_table
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initializes font table.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *
 *  FUNCTION VALUE:
 *
 *    	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */



CDAstatus dvr_init_font_table (vw)

DvrViewerWidget vw;		/* Viewer widget context pointer */

{
    /* Local variables */
    DvrViewerPtr dvr_window;	/* Viewer widget */
    CDAsize table_size;   /* Font table byte size */
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Allocate font table of a size equal to the number of server
     *	fonts found in X list fonts call.
     */

    dvr_window = (DvrViewerPtr) &( vw->dvr_viewer );	/* Init */

#ifdef DEBUG

    /* *** Display all fonts in font list *** */

    {
    char *font_name, **font_list;
    int i;

      font_list = dvr_window->available_fonts;
      for ( i = 0; i < dvr_window->num_fonts; ++i ) {
        font_name = *font_list;
        printf ( "Font name [%d]: %s\n", i, font_name );
        font_list++;
      }
    }

    /* Also, synchronize X output */
    (void) XSynchronize ( XtDisplay (dvr_window->work_window), 1 );

#endif

    /* Calculate table size required for allocation */
    table_size = dvr_window->num_fonts * sizeof(FontStruct);

    status = dvr_alloc_memory (
	vw,
	table_size,
	(void **) &dvr_window->font_table );

    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }

    /* Succesful completion */
    return DVR_NORMAL;
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_dealloc_font_table
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Deallocates font table.  Fonts should be unloaded before calling
 *	this routine.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *
 *  FUNCTION VALUE:
 *
 *    	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus dvr_dealloc_font_table ( vw )

DvrViewerWidget vw;		/* Viewer widget context pointer */

{
    /* Local variables */
    DvrViewerPtr	dvr_window;	/* Viewer widget */
    CDAsize		table_size;	/* Font table byte size */
    CDAstatus		status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Deallocate font table.
     */

    dvr_window = (DvrViewerPtr) &( vw->dvr_viewer );	/* Init */

    /* Calculate table size required for deallocation */
    table_size = dvr_window->num_fonts * sizeof(FontStruct);

    status = dvr_dealloc_memory (
	vw,
	table_size,
	(void **) &dvr_window->font_table );

    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }

    /* Succesful completion */
}



/*
**++
**  ROUTINE NAME:
**    dvr_toolkit_message_routine
**
**  FUNCTIONAL DESCRIPTION:
**    gets called by the cda toolkit in case of informationla messages;
**    call the user supplied error routine, specifying the message as a
**    diagnostic;
**
**  FORMAL PARAMETERS:
**    CDAaddress	prm;
**    CDAstatus 	*stat;
**    CDAsize		*byte_cnt;
**    unsigned char 	*buff;
**    CDAsize	 	*next_len;
**    unsigned char	**next_buff;
**
**  IMPLICIT INPUTS:
**     none
**
**  IMPLICIT OUTPUTS:
**    none
**
**  FUNCTION VALUE:
**    none
**
**  SIDE EFFECTS:
**    none
**--
**/

CDAstatus CDA_CALLBACK dvr_toolkit_message_routine
  (prm, stat, byte_cnt, buff, next_len, next_buff)
CDAaddress	prm;
CDAstatus 	*stat;
CDAsize		*byte_cnt;
unsigned char 	*buff;
CDAsize 	*next_len;
unsigned char	**next_buff;
{
    DvrViewerWidget  vw = (DvrViewerWidget) prm;

    /* report message as a diagnostic  (0 status) */
    if (buff)
      {
	buff[*byte_cnt] = '\0';
	dvr_error_callback(vw,
			   DVS_K_INFORMATION,
			   0L,
			   (CDAenvirontext *) buff,
			   0L);
      }


    *next_len  = 512;
    *next_buff = buff;
    return CDA_NORMAL;
}


#ifdef MSWINDOWS

long dvr_work_width(vw)
    DvrViewerWidget vw;
{
    RECT rcl;
    GetClientRect(Work(vw), &rcl);
    return((long) rcl.right);
}

long dvr_work_height(vw)
    DvrViewerWidget vw;
{
    RECT rcl;
    GetClientRect(Work(vw), &rcl);
    return((long) rcl.bottom);
}

#endif

#ifdef OS2

long dvr_work_width(vw)
    DvrViewerWidget vw;

{
    RECTL rcl;
    WinQueryWindowRect(Work(vw), &rcl);
    return( (long) rcl.xRight );
}

long dvr_work_height(vw)
    DvrViewerWidget vw;

{
    RECTL rcl;
    WinQueryWindowRect(Work(vw), &rcl);
    return( (long) rcl.yTop );
}

#endif

#if defined(OS2) || defined(MSWINDOWS)

/*
 *  the following two routines are included for OS2 only; they are macros
 *  on the other platforms; the OS2 C compiler will not allow these macros
 *  because of size constraints.
 */

Boolean dvr_object_bbox_included(o, d)
    OBJ_PRIVATE o;
    BBOX_REF    d;

{
    /* have to split statement into parts to make compiler happy */

    Boolean part_a, part_b, part_c, part_d,
	    part_e, part_f, part_g;

    /* for initial testing, display everything, return true */
    /* return(TRUE); */

	/*  OS/2 has a lower left 0,0
	 *  Note, on OS/2 bbox_lr_y is really the upper right y value, and
	 *  bbox_ul_y is really the lower left value; the names were left
	 *  unchanged to reduce the amount of code change necessary to port
	 *  the code to OS/2; the viewer already had the varibles named
	 *  *lr_y and *ul_y, so it was easier to use the same var names.
	 *  The engine pages have the same 0,0 location as PM, and the engine
	 *  stores the values as *ur_y and *ll_y, so it was easier to just
	 *  grab the values directly from the engine, rather than converting
	 *  them. This also makes the bbox_included() logic the same on all
	 *  platforms.
	 */

    part_a = (Boolean)
     (( d->bbox_ul_x <= o->bbox_ul_x ) && ( d->bbox_ul_y <= o->bbox_ul_y ) &&
      ( o->bbox_ul_x <= d->bbox_lr_x ) && ( o->bbox_ul_y <= d->bbox_lr_y ));

    part_b = (Boolean)
    (( d->bbox_ul_x <= o->bbox_lr_x ) && ( d->bbox_ul_y <= o->bbox_lr_y ) &&
      ( o->bbox_lr_x <= d->bbox_lr_x ) && ( o->bbox_lr_y <= d->bbox_lr_y ));

    part_c = (Boolean)
    (( d->bbox_ul_x <= o->bbox_ul_x ) && ( o->bbox_ul_x <= d->bbox_lr_x ) &&
      ( o->bbox_ul_y <= d->bbox_ul_y ) && ( d->bbox_ul_y <= o->bbox_lr_y ));

    part_d = (Boolean)
    (( d->bbox_ul_y <= o->bbox_ul_y ) && ( o->bbox_ul_y <= d->bbox_lr_y ) &&
      ( o->bbox_ul_x <= d->bbox_ul_x ) && ( d->bbox_ul_x <= o->bbox_lr_x ));

    part_e = (Boolean)
    (( d->bbox_ul_x <= o->bbox_lr_x ) && ( o->bbox_lr_x <= d->bbox_lr_x ) &&
      ( o->bbox_ul_y <= d->bbox_lr_y ) && ( d->bbox_lr_y <= o->bbox_lr_y ));

    part_f = (Boolean)
    (( d->bbox_ul_y <= o->bbox_lr_y ) && ( o->bbox_lr_y <= d->bbox_lr_y ) &&
      ( o->bbox_ul_x <= d->bbox_lr_x ) && ( d->bbox_lr_x <= o->bbox_lr_x ));

    part_g = (Boolean)
    (( o->bbox_ul_x <= d->bbox_ul_x ) && ( o->bbox_ul_y <= d->bbox_ul_y ) &&
      ( d->bbox_lr_x <= o->bbox_lr_x ) && ( d->bbox_lr_y <= o->bbox_lr_y ));

    return( (Boolean) (part_a || part_b || part_c || part_d ||
	               part_e || part_f || part_g) );
}

CDAstatus dvr_calculate_max_bbox(state, child_state)
    STATEREF state;
    STATEREF child_state;

{
    if ( BBOX_DEFINED (child_state) ) {
	(state)->bbox_ll_x =
	    MIN ((state)->bbox_ll_x,
		((child_state)->bbox_ll_x ));
	(state)->bbox_ll_y =
	    MIN ((state)->bbox_ll_y,
		((child_state)->bbox_ll_y ));
	(state)->bbox_ur_x =
	    MAX ((state)->bbox_ur_x,
		((child_state)->bbox_ur_x ));
	(state)->bbox_ur_y =
	    MAX ((state)->bbox_ur_y,
		((child_state)->bbox_ur_y ));
    }

    return(DVR_NORMAL);
}

/*
 *  The following two routines are necessary because the pre-OS2 code
 *  cheated by passing in parameters of different types to these macros
 *  relying on the fact that the types had some fields with the same names;
 *  This won't work on OS2 where we have to use functions instead of
 *  Macros because of the type checking in the function parameters.
 *  These routines are exactly the same as above except for different
 *  types in the parameters
 */

Boolean dvr_object_bbox_included_b(o, d)
    BBOX_REF    o;
    BBOX_REF    d;

{
    /* have to split statement into parts to make compiler happy */

    Boolean part_a, part_b, part_c, part_d,
	    part_e, part_f, part_g;

	/*  OS/2 has a lower left 0,0
	 *  Note, on OS/2 bbox_lr_y is really the upper right y value, and
	 *  bbox_ul_y is really the lower left value; the names were left
	 *  unchanged to reduce the amount of code change necessary to port
	 *  the code to OS/2; the viewer already had the varibles named
	 *  *lr_y and *ul_y, so it was easier to use the same var names.
	 *  The engine pages have the same 0,0 location as PM, and the engine
	 *  stores the values as *ur_y and *ll_y, so it was easier to just
	 *  grab the values directly from the engine, rather than converting
	 *  them. This also makes the bbox_included() logic the same on all
	 *  platforms.
	 */

    part_a = (Boolean)
     (( d->bbox_ul_x <= o->bbox_ul_x ) && ( d->bbox_ul_y <= o->bbox_ul_y ) &&
      ( o->bbox_ul_x <= d->bbox_lr_x ) && ( o->bbox_ul_y <= d->bbox_lr_y ));

    part_b = (Boolean)
    (( d->bbox_ul_x <= o->bbox_lr_x ) && ( d->bbox_ul_y <= o->bbox_lr_y ) &&
      ( o->bbox_lr_x <= d->bbox_lr_x ) && ( o->bbox_lr_y <= d->bbox_lr_y ));

    part_c = (Boolean)
    (( d->bbox_ul_x <= o->bbox_ul_x ) && ( o->bbox_ul_x <= d->bbox_lr_x ) &&
      ( o->bbox_ul_y <= d->bbox_ul_y ) && ( d->bbox_ul_y <= o->bbox_lr_y ));

    part_d = (Boolean)
    (( d->bbox_ul_y <= o->bbox_ul_y ) && ( o->bbox_ul_y <= d->bbox_lr_y ) &&
      ( o->bbox_ul_x <= d->bbox_ul_x ) && ( d->bbox_ul_x <= o->bbox_lr_x ));

    part_e = (Boolean)
    (( d->bbox_ul_x <= o->bbox_lr_x ) && ( o->bbox_lr_x <= d->bbox_lr_x ) &&
      ( o->bbox_ul_y <= d->bbox_lr_y ) && ( d->bbox_lr_y <= o->bbox_lr_y ));

    part_f = (Boolean)
    (( d->bbox_ul_y <= o->bbox_lr_y ) && ( o->bbox_lr_y <= d->bbox_lr_y ) &&
      ( o->bbox_ul_x <= d->bbox_lr_x ) && ( d->bbox_lr_x <= o->bbox_lr_x ));

    part_g = (Boolean)
    (( o->bbox_ul_x <= d->bbox_ul_x ) && ( o->bbox_ul_y <= d->bbox_ul_y ) &&
      ( d->bbox_lr_x <= o->bbox_lr_x ) && ( d->bbox_lr_y <= o->bbox_lr_y ));

    return( (Boolean) (part_a || part_b || part_c || part_d ||
	               part_e || part_f || part_g) );
}

CDAstatus dvr_calculate_max_bbox_p(state, child_state)
    STATEREF    state;
    PAG_PRIVATE child_state;

{
    if ( BBOX_DEFINED (child_state) ) {
	(state)->bbox_ll_x =
	    MIN ((state)->bbox_ll_x,
		((child_state)->bbox_ll_x ));
	(state)->bbox_ll_y =
	    MIN ((state)->bbox_ll_y,
		((child_state)->bbox_ll_y ));
	(state)->bbox_ur_x =
	    MAX ((state)->bbox_ur_x,
		((child_state)->bbox_ur_x ));
	(state)->bbox_ur_y =
	    MAX ((state)->bbox_ur_y,
		((child_state)->bbox_ur_y ));
    }

    return(DVR_NORMAL);
}

long dvr_max(a, b)
    long a;
    long b;

{
    if (a > b)
	return(a);
    else
    	return(b);

}

long dvr_min(a, b)
    long a;
    long b;

{
    if (a < b)
	return(a);
    else
    	return(b);

}

#endif /* OS2 */
