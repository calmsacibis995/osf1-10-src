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
#define Module DVR_WCBR
#define Ident  "V01-008"

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
** FACILITY:
**
**    Compound Document Architecture (CDA)
**    Compound Document Viewers
**    DIGITAL Document Interchange Format (DDIF)
**
** ABSTRACT:
**    This module contains the action routines for the
**    cbr support for the ddif viewer widget.
**
** ENVIORNMENT:
**    VMS DECwindows and ultrix uws
**
** AUTHORS:
**
**
**
** MODIFIED BY:
**
**    V01-000        FSC0000        	Charles Chan    	27-Jun-1990
**        initial experiments
**
**    V01-001	     SJM0000	    	Stephen J. Munyan	 4-Oct-1990
**	  Minor cleanups to make the code match the rest of the Viewer.
**
**    V01-002	     DAM0000	    	Dennis McEvoy		 3-apr-1991
**	  cleanup typedefs
**
**    V02-003	     DAM0001		Dennis McEvoy		06-may-1991
**		move dvr_int include to cleanup ultrix build
**
**    V02-004	     DAM0001		Dennis McEvoy		05-aug-1991
**		rename headers, remove dollar signs
**    V02-005	     DAM0001		Dennis McEvoy		02-Jun-1992
**		type cleanups for alpha/osf1
**    V02-006	     KLM0001		Kevin McBride		12-Jun-1992
**		Alpha OSF/1 port
**    V02-007	     DAM0001		Dennis McEvoy		22-Jun-1992
**		make CBR parameter a CDAuserparam to match dvs
**
**    V02-008	     RDH003		Don Haney		15-Jul-1992
**		Specify same-directory includes with "", not <>
**--
*/

/*
 *  Include files
 */
#include <cdatrans.h>

#ifdef __vms__

#include <rmsdef.h>                		/* RMS definitions        		*/
#include <dvrint.h>                		/* DVR internal definitions 		*/

#pragma nostandard				/* turn off /stand=port for
						   "unclean" X include files */

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#pragma standard				/* turn /stand=port back on */

#include <ssdef.h>

#endif

#ifdef OS2

#define INCL_PM                 /* tell OS/2 to include Presentation Manager defs */
#define PMMLE_INCLUDED                          /* do not include multi-line editing defs */
#include <os2.h>                                /* OS/2 defs */

#endif

#ifdef __unix__

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#include <dvrint.h>                		/* DVR internal definitions 		*/
#endif

#include <dvsdef.h>                		/* DVS Viewer Engine Defns  		*/
#include "dvrwdef.h"            		/* Public Defns w/dvr_include 		*/
#include "dvrwint.h"            		/* Viewer Constants and structures  	*/
#include <cdaptp.h>                		/* cda toolkit prototypes   		*/
#include "dvrwptp.h"            		/* dvr windowing prototypes 		*/


/* local routine prototypes */

PROTO( unsigned long dvr_get_next_page,
        (DvrViewerWidget) );

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *    DvrCbrPrevRef
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *    Goto to page containing the CBR reference.
 *    Pages are read in, but not final formatted, until
 *    the requested page or end of file is detected.  The specified format
 *    flag applies only to the formatting of the goal page (as opposed to
 *    the semantics of this status code in goto next and previous page
 *    routines).
 *
 *    End of document status DOES NOT imply that no new data has been read
 *    in this routine.  In searching for the specified page, end of document
 *    may have been detected.  In such case, end of document status is
 *    returned along with the last page of the document as the current page.
 *    As long as the last page of the document wasn't the current page, end
 *    of document WILL imply that a new page is available and should be
 *    displayed by the caller.
 *
 *  FORMAL PARAMETERS:
 *
 *    vw        	- Viewer widget context structure
 *    format_flag    	- Format page if not yet formatted
 *    page_number 	- returned page number
 *
 *  FUNCTION VALUE:
 *
 *    DVR_NORMAL    	Normal successful completion
 *    DVR_BADPARAM    	Bad parameter
 *    DVR_EOD    	End of document
 *    DVR_PAGENOTFOUND Specified page not found
 *      		Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus DvrCbrPrevRef (
    vw,
    format_flag,
    page_number )

DvrViewerWidget vw;        			/* Viewer widget context pointer 	*/
boolean format_flag;        			/* Format page if not yet formatted 	*/
int  *page_number;       			/* returned page number 		*/

{
/* stuff from dvr_actions.c */
    struct pag  *current_page 	= vw->dvr_viewer.Dvr.current_page;
    int	    page_num;

    struct 	pag *first_page =
		  	(struct pag *) vw->dvr_viewer.Dvr.page_list.que_flink;
    int  	first_page_num;

    if (current_page == NULL)
	page_num = 1;
    else
	page_num = current_page->pag_page_number;

    if ( (first_page == NULL) || (current_page == NULL) )
	first_page_num = 1;
    else
	first_page_num = first_page->pag_page_number;

    if (page_num == first_page_num)
	return(DVR_TOPOFDOC);

    dvr_set_watch_cursor(vw);
    vw->dvr_viewer.Dvr.WidgetStatus = DvrCbrFindPage (vw, page_number, -1);

    if (DVRFailure(vw->dvr_viewer.Dvr.WidgetStatus))
	{
	dvr_error_callback(vw, 0, vw->dvr_viewer.Dvr.WidgetStatus, 0, 0);
	dvr_reset_cursor(vw);
	return(vw->dvr_viewer.Dvr.WidgetStatus);
	}

    if (vw->dvr_viewer.Dvr.WidgetStatus != DVR_TOPOFDOC)
	dvr_initialize_window(vw);
    else
	dvr_set_page_number(vw);

    dvr_reset_cursor(vw);
    dvr_update_page_number(vw);
    return(vw->dvr_viewer.Dvr.WidgetStatus);
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *    DvrCbrNextRef
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *    Goto to page containing the CBR reference.
 *    Pages are read in, but not final formatted, until
 *    the requested page or end of file is detected.  The specified format
 *    flag applies only to the formatting of the goal page (as opposed to
 *    the semantics of this status code in goto next and previous page
 *    routines).
 *
 *    End of document status DOES NOT imply that no new data has been read
 *    in this routine.  In searching for the specified page, end of document
 *    may have been detected.  In such case, end of document status is
 *    returned along with the last page of the document as the current page.
 *    As long as the last page of the document wasn't the current page, end
 *    of document WILL imply that a new page is available and should be
 *    displayed by the caller.
 *
 *  FORMAL PARAMETERS:
 *
 *    vw        	- Viewer widget context structure
 *    format_flag    	- Format page if not yet formatted
 *    page_number 	- returned page number
 *
 *  FUNCTION VALUE:
 *
 *    DVR_NORMAL    	Normal successful completion
 *    DVR_BADPARAM    	Bad parameter
 *    DVR_EOD    	End of document
 *    DVR_PAGENOTFOUND Specified page not found
 *      		Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus DvrCbrNextRef (
    vw,
    format_flag,
    page_number )

DvrViewerWidget vw;        			/* Viewer widget context pointer 	*/
boolean format_flag;        			/* Format page if not yet formatted 	*/
int  *page_number;       			/* returned page number 		*/

{
/* stuff from dvr_actions.c */
    struct pag  *current_page = vw->dvr_viewer.Dvr.current_page;
    int	   page_num;

    struct 	pag *last_page =
		  	(struct pag *) vw->dvr_viewer.Dvr.page_list.que_blink;
    int  	last_page_num;

    if (vw->dvr_viewer.Dvr.DocRead)
        {
                /*  if the document is fully read, check to see if we're
	         *  already at the bottom; if so, return
	         */
        if (current_page == NULL)
            page_num = 1;
        else
            page_num = current_page->pag_page_number;

        if ( (last_page == NULL) || (current_page == NULL) )
            last_page_num = 1;
        else
            last_page_num = last_page->pag_page_number;

        if (page_num == last_page_num)
            return(DVR_EOD);
        }

    dvr_set_watch_cursor(vw);
    vw->dvr_viewer.Dvr.WidgetStatus = DvrCbrFindPage (vw, page_number, 1);

    if (DVRFailure(vw->dvr_viewer.Dvr.WidgetStatus))
	{
	dvr_error_callback(vw, 0, vw->dvr_viewer.Dvr.WidgetStatus, 0, 0);
	dvr_reset_cursor(vw);
	return(vw->dvr_viewer.Dvr.WidgetStatus);
	}

    if (vw->dvr_viewer.Dvr.WidgetStatus != DVR_EOD)
	dvr_initialize_window(vw);
    else
	dvr_set_page_number(vw);

    dvr_reset_cursor(vw);
    dvr_update_page_number(vw);
    return(vw->dvr_viewer.Dvr.WidgetStatus);
}

/*
**++
**  ROUTINE NAME:
**    DvrCbrFindPage
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**        Widget     vw;              viewer widget
**        int     *page_number;        page number
**        int     direction;           forward or backward
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

CDAstatus DvrCbrFindPage (vw, page_number, direction)
    DvrViewerWidget     vw;
    int     *page_number;
    int     direction;
{
    int i,j,k;
    CDAstatus status, saved_status;

    DvrStruct *Dvr;
    CbrStruct *Cbr;
    PAG page, saved_page;
    CDAconstant saved_page_number;

    if ( vw == NULL ) return DVR_BADPARAM;

    Dvr = (DvrStruct *) &( vw->dvr_viewer.Dvr );
    Cbr = (CbrStruct *) &( vw->dvr_viewer.Cbr );
    page = saved_page = Dvr->current_page;
    saved_page_number = saved_page->pag_page_number;

    if (direction < 0) {    /* backwards */
        do {
            saved_status = status = dvr_goto_previous_page (vw, 0);
            if ( DVR_FAILURE ( status )) {  return status; }
            if (status == DVR_TOPOFDOC)  break;
            page = Dvr->current_page;
        } while (page->pag_cbr_marked == 0);
    } else {
        do {
            saved_status = status = dvr_goto_next_page (vw, 0);
            if ( DVR_FAILURE ( status )) {  return status; }
            if (status == DVR_EOD)  break;
            page = Dvr->current_page;
        } while (page->pag_cbr_marked == 0);
    }

    if (page->pag_cbr_marked) {
        *page_number = Dvr->current_page->pag_page_number;
        status = dvr_format_page ( vw );
        return(saved_status);
    } else {
	status = dvr_goto_page_number ( vw, FALSE, &saved_page_number );
        *page_number = saved_page->pag_page_number;
        return DVR_PAGENOTFOUND;	/* Error message box to be displayed */
    }
}

/*
**++
**  ROUTINE NAME:
**    DvrCbrFirstRef
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**        Widget     vw;              viewer widget
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

CDAstatus DvrCbrFirstRef (vw)
    DvrViewerWidget     vw;
{
    int i,j,k;
    CDAstatus status, saved_status;
    int first_page = 1;

    DvrStruct *Dvr;
    PAG page, saved_page;
    long saved_page_number;
    long page_list_number;		/* Page list element number */

    if ( vw == NULL ) return DVR_BADPARAM;

    Dvr = (DvrStruct *) &( vw->dvr_viewer.Dvr );
    page = Dvr->current_page;
    while (page->pag_cbr_marked == 0) {
        first_page = 0;
        saved_status = status = dvr_get_next_page (vw);
        if ( DVR_FAILURE ( status )) {  return status; }
        if (status == DVR_EOD)  break;
	page = (PAG) Dvr->page_list.que_blink;
    }
    Dvr->current_page = page;

    if (!(first_page))  status = dvr_format_page ( vw );
    return(saved_status);
}

/*
**++
**  ROUTINE NAME:
**    DvrCbrSetCtxt(vw, frg_ref_cb, frg_ref_parm, button_flag)
**
**  FUNCTIONAL DESCRIPTION:
**    initialize the CbrStruct in the viewer widget
**
**  FORMAL PARAMETERS:
**        Widget     vw;               viewer widget
**        unsigned long  (*frg_ref_cb) ();   callback to get next fragment reference
**        unsigned long frg_ref_parm;
**        unsigned long button_flag;
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

void DvrCbrSetCtxt(vw, frg_ref_cb, frg_ref_parm, button_flag)
    DvrViewerWidget     vw;
    PROTO(CDAstatus (*frg_ref_cb),  (CDAuserparam,
	  CDAconstant, CDAint32, CDAuint32 *, CDAuint32 **));	
    CDAuserparam 	frg_ref_parm;
    CDAuint32 	button_flag;
{
    vw->dvr_viewer.Cbr.mode = 1;
    vw->dvr_viewer.Cbr.FrgRefCb   = frg_ref_cb;
    vw->dvr_viewer.Cbr.FrgRefParm = frg_ref_parm;

    vw->dvr_viewer.Cbr.EnableRefButs = (Boolean) button_flag;
    DvrCbrManageRefs(vw);

}

/*
**++
**  ROUTINE NAME:
**    DvrCbrClearCtxt(vw)
**
**  FUNCTIONAL DESCRIPTION:
**    free the allocated memory used by CBR
**
**  FORMAL PARAMETERS:
**        Widget     vw;              viewer widget
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

void DvrCbrClearCtxt(vw)
    DvrViewerWidget     vw;
{
    vw->dvr_viewer.Cbr.mode = 0;
    vw->dvr_viewer.Cbr.FrgRefCb   = NULL;
    vw->dvr_viewer.Cbr.FrgRefParm = NULL;

    DvrCbrUnmanageRefs(vw);
    vw->dvr_viewer.Cbr.EnableRefButs = 0;

}

/*
**++
**  ROUTINE NAME:
**    DvrCbrManageRefs(vw)
**
**  FUNCTIONAL DESCRIPTION:
**    Manage the Next and Prev Reference buttons used by CBR
**
**  FORMAL PARAMETERS:
**        Widget     vw;              viewer widget
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

void DvrCbrManageRefs(vw)
    DvrViewerWidget     vw;
{
    if (vw->dvr_viewer.Cbr.EnableRefButs) {
        if (NextRef(vw) != NULL)
            if ( !(XtIsManaged(NextRef(vw))) )
                XtManageChild(NextRef(vw));

        if (PrevRef(vw) != NULL)
            if ( !(XtIsManaged(PrevRef(vw))) )
            XtManageChild(PrevRef(vw));
    }
}

/*
**++
**  ROUTINE NAME:
**    DvrCbrUnmanageRefs(vw)
**
**  FUNCTIONAL DESCRIPTION:
**    Unmanage the Next and Prev Reference buttons used by CBR
**
**  FORMAL PARAMETERS:
**        Widget     vw;              viewer widget
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

void DvrCbrUnmanageRefs(vw)
    DvrViewerWidget     vw;
{
    if (vw->dvr_viewer.Cbr.EnableRefButs) {
        if (NextRef(vw) != NULL)
            if (XtIsManaged(NextRef(vw)) )
                XtUnmanageChild(NextRef(vw));

        if (PrevRef(vw) != NULL)
            if (XtIsManaged(PrevRef(vw)) )
                XtUnmanageChild(PrevRef(vw));
    }
}
