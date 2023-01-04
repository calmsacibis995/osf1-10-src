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
#define Module DVR_PMGMT
#define Ident  "V02-017"

/*
**++
**   COPYRIGHT (c) 1989, 1991 BY
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
** MODULE_NAME
**	dvr_page_mgmt.c (vms)
**	dvr_page_mgmt.c (ultrix)
**	dvr_pmgt.c	(os/2)
**
** FACILITY:
**
**	Compound Document Architecture (CDA)
**	Compound Document Viewers
**	DIGITAL Document Interchange Format (DDIF)
**
** ABSTRACT:
**	High level page management routines.
**
** ENVIORNMENT
**	vms, ultrix, os/2
**
** AUTHORS:
**	Marc A. Carignan,  23-Nov-1988
**
**
** MODIFIED BY:
**
**	V02-001		PBD0001		Peter B. Derr		1-Mar-1989
**		Use new flattened include files
**	V02-002		DAM0002		Dennis McEvoy		26-Jul-1989
**		return file-not-found status to user
**	V02-003		DAM0003		Dennis McEvoy		05-mar-1990
**		changes for os/2 port
**	V02-004		DAM0004		Dennis McEvoy		05-jul-1990
**		check for memory allocation failure error
**	V02-005		DAM0004		Dennis McEvoy		11-jul-1990
**		fix goto past end of document bug
**	V02-006		SJM0000		Stephen Munyan		28-Jun-1990
**		Conversion to Motif
**	V02-007		DAM0000		Stephen Munyan		01-mar-1991
**		convert to new typedefs
**	V02-008		DAM0000		Stephen Munyan		04-mar-1991
**		cleanup new typedefs
**	V02-009		RTG0000		Dick Gumbel		05-Mar-1991
**		Cleanup #include's
**	V02-010		DAM0000		Stephen Munyan		03-apr-1991
**		cleanup new typedefs
**	V02-011		RAM0000		Ralph A. Mack		24-Apr-1991
**		Add #ifdefs for MS-Windows
**      V02-012	        DAM0001		Dennis McEvoy		06-may-1991
**		move dvr_int include to cleanup ultrix build
**      V02-013	        DAM0001		Dennis McEvoy		05-aug-1991
**		renamed headers, removed dollar signs
**      V02-014	        DAM0001		Dennis McEvoy		01-jun-1991
**		type cleanups for alpha/osf1
**	V02-015		DAM000		Dennis McEvoy		03-Jun-1992
**		use engine user private fields as CDAuserparam
**	V02-016		KLM001		Kevin McBride		12-Jun-1992
**		Alpha OSF/1 port
**
**	V02-017		RDH003		Don Haney		15-Jul-1992
**		Specify same-directory includes with "", not <>
**
**--
*/

/*
 *  Include files
 */
#include <cdatrans.h>

#ifdef __vms__

#include <rmsdef.h>				/* RMS definitions	    */
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
#define NOUSER					/* from parse of windows.h */
#define NOMETAFILE				/* to save memory on compile. */
#include <windows.h>				/* MS-Windows definitions. */

#include <dvrint.h>				/* DVR internal definitions */
#endif

#ifdef __unix__
#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#include <dvrint.h>
#endif

#include <dvsdef.h>				/* DVS Viewer Engine Defns  */
#include "dvrwdef.h"			/* Public Defns w/dvr_include */
#include "dvrwint.h"			/* Viewer Constants and structures  */
#include "dvrwptp.h"			/* dvr windowing prototypes */

/* local routine prototypes */

PROTO( CDAstatus dvr_get_next_page,
		(DvrViewerWidget) );

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_goto_next_page
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Goto to next page in document.  End of document return status implies
 *	that no new data has been read, only that the end of the document
 *	has been detected, and therefore, that display page need not be
 *	invoked (otherwise it will just redisplay the current page).
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	format_flag	- Format next page if not yet formatted
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *	DVR_EOD	End of document
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus dvr_goto_next_page (
    vw,
    format_flag )

DvrViewerWidget vw;		/* Viewer widget context pointer */
boolean format_flag;		/* Format next page if not yet formatted */

{
    /* Local variables */
    DvrStruct *dvr_struct;		/* DVR struct pointer */
    PAG next_page;			/* Next page */
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */

    /*
     *	Retrieve next page in document.
     */

    /* Check if we must read in another page from the layout engine */

    if (( dvr_struct->current_page == NULL ) ||
        ( QUE_EMPTY ( dvr_struct->current_page->pag_str.str_queue )) ||
	((QUE) ( next_page =
          (PAG) dvr_struct->current_page->pag_str.str_queue.que_flink )
	== ((QUE) &( dvr_struct->page_list )))) {

	/* Either no curent page (nothing yet read in), only one page
	   has been read in (page queue is empty), or no next page
	   found (last page in list points back to head of queue) */


	/*
	 *  Try to read in next page in document from front end if
	 *  not already at end of document.
	 */

	/* Check if end of document already detected */
	if ( dvr_struct->DocRead ) {
	    /* Don't alter current page reference (assumedly last page in
	       document); simply return end of document status (may have
	       been set due to actual end of document, or error) */
	    return DVR_EOD;
	}


	/* End of document not yet encountered; need to read in another
	   page from Viewer back end procedure; procedure does NOT update
	   the Viewer current page reference */

	status = dvr_get_next_page (
	    vw);

	/* Check for end of file; details handled by get next page routine */
	if ( status == DVR_EOD ) return DVR_EOD;

	/* Check for all other errors */
	if ( DVR_FAILURE ( status )) {
	    /* first check for memory allocation failure status */
	    if (status == CDA_ALLOCFAIL)
                (void) dvr_error_callback ( vw, 0L, DVR_MEMALLOFAIL, 0, 0L );

	    /*  if the status is file-not-found (open-fail on ultrix), then
	     *  return this status to the user; usually this means an
	     *	externally referenced file could not be found; else, return
	     *  the generic format-error instead of sending low level status
	     *  back to user.
	     */
#ifdef __vms__
	    else if (status == RMS$_FNF)
#else
	    else if (status == CDA_OPENFAIL)
#endif
	        /* Report return status */
                (void) dvr_error_callback ( vw, 0L, status, 0, 0L );
	    else
                (void) dvr_error_callback ( vw, 0L, DVR_FORMATERROR, 0, 0L );


	    /* Abort further document processing on layout engine errors;
	       mark document as read, allowing user to continue reading
	       already processed pages; layout engine errors are treated as
	       recoverable viewer errors, but prohibit the viewer from
	       obtaining any more document information */

	    dvr_struct->DocRead = TRUE;
	    return DVR_EOD;  /* Treat as end of document as reporting error */
	}


	/*
	 *  Next page found; set page obtained from backend, and stored as
	 *  last element in page queue, and the new Viewer current page
	 *  reference; format if flag is on.
	 */

	/* Actually moving to new page; perform any necessary current page
	   cleanup activities before moving on to next page processing */

	if ( dvr_struct->current_page != NULL ) {
	    /* Current page exists */
	    status = dvr_cleanup_page( vw );
	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error */
		return status;
	    }
	}

	/* Update current page; stored as last page in page queue */
	dvr_struct->current_page = (PAG) dvr_struct->page_list.que_blink;

	/* Continue with format operation, if specified */
	if ( format_flag ) {
	    /* Format flag on */
	    status = dvr_format_page ( vw );
	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error during page processing */
		return status;
	    }
	}
    }

    else {

	/*
	 *  Next page already in memory; save as new current page; if format
	 *  flag on, format if not already formatted.
	 */

	/* Actually moving to new page; perform any necessary current page
	   cleanup activities before moving on */

	status = dvr_cleanup_page( vw );
	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error */
            return status;
	}

	/* Continue with format operation, if specified */
	dvr_struct->current_page = next_page;	/* Save next page as current */
   	if (( format_flag ) &&
	    ( dvr_struct->current_page->pag_user_private == NULL )) {
	    /* Format flag on & page not yet formatted; format page */
	    status = dvr_format_page ( vw );
	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error during page processing */
		return status;
	    }
	}
    }

    /* Successful completion */
    return DVR_NORMAL;
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_goto_page_number
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Goto to page number specified.  If page specified is greater than
 *	number of pages in document, the last page is returned, and the page
 *	number updated.  Pages are read in, but not final formatted, until
 *	the requested page or end of file is detected.  The specified format
 *	flag applies only to the formatting of the goal page (as opposed to
 *	the semantics of this status code in goto next and previous page
 *	routines).
 *
 *	End of document status DOES NOT imply that no new data has been read
 *	in this routine.  In searching for the specified page, end of document
 *	may have been detected.  In such case, end of document status is
 *	returned along with the last page of the document as the current page.
 *	As long as the last page of the document wasn't the current page, end
 *	of document WILL imply that a new page is available and should be
 *	displayed by the caller.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	format_flag	- Format page if not yet formatted
 *	page_number	- Desired page number, by reference
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *	DVR_EOD	End of document
 *	DVR_PAGENOTFOUND Specified page not found
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus dvr_goto_page_number (
    vw,
    format_flag,
    page_number )

DvrViewerWidget vw;		/* Viewer widget context pointer */
boolean format_flag;		/* Format page if not yet formatted */
CDAconstant *page_number;	/* Desired page number */

{
    /* Local variables */
    DvrStruct *dvr_struct;		/* DVR struct pointer */
    PAG page;				/* Page */
    long page_list_number;		/* Page list element number */
    CDAstatus status;


    /* Check parameter validity */
    if (( vw == NULL ) || ( page_number == NULL ))
	return DVR_BADPARAM;


    /*
     *	Retrieve specified page in document.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */


    /* Currently, the following check represents an impossible case, because
       the Viewer ALWAYS calls goto next page as part of new document
       initialization, before the option to invoke this routine is presented */

    if ( dvr_struct->current_page == NULL ) {
	/* No current page -- nothing yet read in, therefore, we are
	   at page '0'; read in first page */

	status = dvr_goto_next_page (
	    vw,
	    format_flag );

	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error */
	    return status;
	}

	/* First page read in; continue with rest of algorithm, now that
	   a page number reference, assumedly page 1, is available as the
	   current page; if this is the currently requested page, return */
	page_list_number =
	    ((PAG) dvr_struct->page_list.que_blink)->pag_page_number;

	if ( page_list_number == *page_number ) {
	    /* Desired page found */
	    return DVR_NORMAL;
	}
    }


    /* Cleanup current page displayed before moving on; assume that if the
       desired page is not the current page, that we will not be redisplaying
       the current page at this time (in general, this is probably true; if
       not, we just land up cleaning up the current page unnecessarily, and
       immediately redisplaying it; otherwise, much more detailed checking
       would be necessary) */

    status = dvr_cleanup_page( vw );
    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }


    /* Check if specified page is already in memory; compare desired page
       number with last page number in memory; if number of last page in
       memory is less than the desired page, need to read in more pages */
    page_list_number =
	((PAG) dvr_struct->page_list.que_blink)->pag_page_number;

    if ( page_list_number < *page_number ) {

	/* Desired page not yet read in; assign last page in page list as
	   current page beforing continuing */
	dvr_struct->current_page =
	    (PAG) dvr_struct->page_list.que_blink;
	page_list_number =
	    dvr_struct->current_page->pag_page_number;

	while ( !( dvr_struct->DocRead ) &&
	    ( page_list_number < *page_number )) {

	    /* Read in next page in document; update current page after
	       desired, or last, page found; procedure does NOT update
	       the Viewer current page reference; routine will set document
	       read flag if an attempt to run off the end of the doc is made */

	    status = dvr_get_next_page (
		vw);

	    if ( DVR_FAILURE ( status )) {
		/* Unexpected error during backend call */
		return status;
	    }

	    /* Get new page number, from page just read in and stored as
	       last element in page list queue */
	    page_list_number =
		((PAG) dvr_struct->page_list.que_blink)->pag_page_number;
	}

	/* Update current page; stored as last page in page queue */
	dvr_struct->current_page = (PAG) dvr_struct->page_list.que_blink;


	/* Check if desired page found */
	if ( page_list_number == *page_number ) {
	    /* Specified page found; format if requested */
	    if ( format_flag ) {
		/* Format flag on */
		status = dvr_format_page ( vw );
		if ( DVR_FAILURE ( status )) {
		    /* Unexpected error during page processing */
		    return status;
		}
	    }
	}
	else {
	    /* End of file; specified page not found; return updated
	       page number of last page in document */
	    *page_number = page_list_number;
	    /* Format if requested */
	    if ( format_flag ) {
		/* Format flag on */
		status = dvr_format_page ( vw );
		if ( DVR_FAILURE ( status )) {
		    /* Unexpected error during page processing */
		    return status;
		}
	    }
	    /* Return end of document status */
	    return DVR_EOD;
	}
    }

    else {
	/* Specified page already in memory; search through list for
	   desired page; search is reverse through list, although
	   this is just arbitrary; forward through list would also work */

	FOREACH_REVERSELIST ( dvr_struct->page_list, page, PAG ) {

	    /* Get page number of current page */
	    page_list_number = page->pag_page_number;

	    if ( page_list_number == *page_number ) {
		/* Desired page found; save as current page */
		dvr_struct->current_page = page;

		/* Specified page found; set page as new current page;
		   format if requested */

		page_list_number = dvr_struct->current_page->pag_page_number;

		if (( format_flag ) &&
		    ( dvr_struct->current_page->pag_user_private == NULL )) {
		    /* Format flag on & page not yet formatted; format page */
		    status = dvr_format_page ( vw );
		    if ( DVR_FAILURE ( status )) {
			/* Unexpected error during page processing */
			return status;
		    }
		}

		/* Exit loop if desired page found */
		break;
	    }

	    /* Else - keeping searching through list */
	}

	/* Verify that page was found on loop exit */
	if ( page_list_number != *page_number ) {
	    /* Page not found; current page remains unchanged */
	    return DVR_PAGENOTFOUND;	/* Error message box to be displayed */
	}
    }

    /* Successful completion */
    return DVR_NORMAL;
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_goto_previous_page
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Goto to previous page in document.  End of document return status
 *	implies that no new data has been read, only that the end of the
 *	document has been detected, and therefore, that display page need
 *	not be invoked (otherwise it will just redisplay the current page).
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	format_flag	- Format next page if not yet formatted
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *	DVR_TOPOFDOC	Top of document
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus dvr_goto_previous_page (
    vw,
    format_flag )

DvrViewerWidget vw;		/* Viewer widget context pointer */
boolean format_flag;		/* Format previous page if not yet formatted */

{
    /* Local variables */
    DvrStruct *dvr_struct;		/* DVR struct pointer */
    PAG prev_page;
    CDAstatus status;

    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */

    /*
     *	Retrieve previous page in document.
     */

    prev_page = (PAG) dvr_struct->current_page->pag_str.str_queue.que_blink;
    if ( prev_page == (PAG) &( dvr_struct->page_list )) {

	/* At top of document;  no previous page exists (first page in list
	   points back to head of queue); return top of document status;
	   keep first page as current page reference in DVR structure */

	return DVR_TOPOFDOC;
    }

    /* Actually moving to new page; perform any necessary current page cleanup
        activities before moving on */

    status = dvr_cleanup_page( vw );
    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }


    /* Previous page exists; save as new current page; if format flag on,
       format if not already formatted */
    dvr_struct->current_page = prev_page;  /* Save previous page as current */
    if (( format_flag ) &&
        ( dvr_struct->current_page->pag_user_private == NULL )) {
	/* Format flag on & page not yet formatted; format page */
	status = dvr_format_page ( vw );
	if ( DVR_FAILURE ( status )) {
	    /* Unexpected error during page processing */
	    return status;
	}
    }

    /* Successful completion */
    return DVR_NORMAL;
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *  	dvr_get_next_page
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Read in next page from Viewer backend.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus dvr_get_next_page (
    vw )

DvrViewerWidget vw;		/* Viewer widget context pointer */

{
    /* Local variables */
    DvrStruct *dvr_struct;		/* DVR struct pointer */
    unsigned long cda_function_code = CDA_CONTINUE;  /* CDA converter function code */
    CDAstatus status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    /*
     *	Read in next page from backend.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */

    if ( dvr_struct->DocRead ) {
	/* Document already read into memory */
	return DVR_EOD;
    }

    /* Document not yet completely read into memory; try to read next
       page in document */

    status = cda_convert (
	(CDAconstant *) &cda_function_code,
	NULL,		/* No item list required for CDA_CONTINUE call */
	(CDAaddress) &vw,
	(CDAconverterhandle *) &( dvr_struct->ConverterContext ) );

    /* Check for end of document; if found, set document read flag in
       DVR context and return end of document status [Note: Status returned
       from CDA_CONVERT will always be CDA_SUSPEND (or CDA_NORMAL in
       return from a CDA_STOP call); actual success of failure status
       of requested operation is held and passed back in widget status */

    if ( dvr_struct->WidgetStatus == CDA_ENDOFDOC ) {
	dvr_struct->DocRead = TRUE;
	return DVR_EOD;
    }

    /* Catch any other errors (ignore 'status'; use widget status) */
    if ( DVR_FAILURE ( dvr_struct->WidgetStatus )) {
	/* Unexpected error */
	return dvr_struct->WidgetStatus;    /* not 'status' */
    }

    /* CDA call to backend to read in next page in document successful;
       new page stored as last element in page list queue; caller must
       decide when it is appropriate to save this new page as the Viewer
       current page reference */


    /* Succesful completion */
    return DVR_NORMAL;
}
