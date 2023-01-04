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
*****************************************************************************

	      Copyright (c) Digital Equipment Corporation, 1990  
	      All Rights Reserved.  Unpublished rights reserved
	      under the copyright laws of the United States.
	      
	      The software contained on this media is proprietary
	      to and embodies the confidential technology of 
	      Digital Equipment Corporation.  Possession, use,
	      duplication or dissemination of the software and
	      media is authorized only pursuant to a valid written
	      license from Digital Equipment Corporation.

	      RESTRICTED RIGHTS LEGEND   Use, duplication, or 
	      disclosure by the U.S. Government is subject to
	      restrictions as set forth in Subparagraph (c)(1)(ii)
	      of DFARS 252.227-7013, or in FAR 52.227-19, as
	      applicable.

*****************************************************************************
**++
**  FACILITY:
**
**	DECwindows Toolkit
**
**  ABSTRACT:
**
**	International Text Widget String Source Data Handling Code 
**
**
**  MODIFICATION HISTORY:
**
**	12 APR 1990  Begin rework of old CSText widget code.
**
**--
**/

#ifndef _CSTextOut_h
#define _CSTextOut_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

/* look at how motif text widget does this 
*/
#define DXmShouldWordWrap( output ) ( output->wordwrap \
				    && ( !output->scrollhorizontal )  \
				    && ( output->numLines > 1  \
				         || output->resizeheight ) )


/* BOGUS
 * some wierd magic numbers...
 */
#define NOLINE		30000

/*
 * useful typedefs for the procs that an output module must supply
 */

typedef Boolean          (*BooleanProc)();
typedef XmFontList       (*GetFontListProc)();

typedef DXmCSTextPosition (*DXmCSTextXYToPosProc)();
typedef DXmCSTextStatus	  (*DXmCSTextStatusProc)();




/****************************************************************
 *
 * Public definitions for modules implementing and using text output routines.
 *
 ****************************************************************/

/*
 * Return the line number containing the given position.  If text currently
 * knows of no line containing that position, returns NOLINE.
 */
extern int DXmCSTextPosToLine(); /* widget, position */
    /* CSTextWidget	widget; */
    /* DXmCSTextPosition position; */



/*
 * ????
 */

extern void DXmCSTextMarkRedraw(); /* widget, left, right */
    /* CSTextWidget		widget; */
    /* DXmCSTextPosition 	left, right; */


#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _CSTextOut_h */
