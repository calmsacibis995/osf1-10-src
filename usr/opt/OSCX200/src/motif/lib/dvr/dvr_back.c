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
#define Module DVR_BACKEND
#define Ident  "V02-021"

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
** MODULE_NAME:
**	dvr_backend.c 	(vms)
**	dvr_backend.c	(ultrix)
**	dvr_back.c	(OS/2)
**
** FACILITY:
**
**	Compound Document Architecture (CDA)
**	Compound Document Viewers
**	DIGITAL Document Interchange Format (DDIF)
**
** ABSTRACT:
**	This is the DECwindows VIEWER back end.  It is invoked through
**	CDA_CONVERT.
**
** ENVIORNMENT:
**	vms, ultrix, OS/2
**
** AUTHORS:
**	Barbara Bazemore, 11-Nov-1988
**
** MODIFIED BY:
**
**	V02-021		Ronan Duke			        30-Aug-1992
**		Include Xm/MAnagerP.h if linking against Motif V1.2
**	V02-020		DAM001		Dennis McEvoy		09-sep-1992
**		pass print driver string down to dvs (Windows only.)
**	V02-019		DAM001		Dennis McEvoy		17-Jul-1992
**		check for existing print hdc when setting up
**		font source for dvs (Windows only.)
**	V02-018		RDH003		Don Haney		15-Jul-1992
**		Specify same-directory includes with "", not with <>
**	V02-017		DAM0001		Dennis McEvoy		03-jun-1992
**		add cast for cbr frg param; should eventually be
**		changed in header file as well
**	V02-016		DAM0001		Dennis McEvoy		01-jun-1992
**		type cleanups for alpha/osf1
**	V02-015		DAM0001		Dennis McEvoy		05-aug-1991
**		rename, rename headers, remove dollar signs
**	V02-014		RAM0001		Ralph A. Mack		28-may-1991
**		Added #ifdef code to pass a device context through to
**		DVS for font management.
**	V02-013		DAM0001		Dennis McEvoy		06-may-1991
**		move dvr_int include to cleanup ultrix build
**	V02-012		RAM0000		Ralph A. Mack		24-apr-1991
**		Added #ifdefs for MS-Windows
**	V02-011		DAM0000		Dennis McEvoy		03-apr-1991
**		cleanup typedefs
**	V02-010		RTG0000		Dick Gumbel		05-Mar-1991
**		cleanup #include's
**	V02-009		DAM0000		Dennis McEvoy		04-mar-1991
**		cleanup new typedefs
**	V02-008		DAM0000		Dennis McEvoy		01-mar-1991
**		convert to new typedefs
**	V02-007		DAM0000		Dennis McEvoy		21-dec-1990
**		take out os/2 font scaling option; now done in dvr_cre.c
**	V02-006		SJM0000		Stephen Munyan		 5-Oct-1990
**		Merge in changes for CBR done by Charlie Chan
**
**	V02-005		SJM0000		Stephen Munyan		21-Jun-1990
**		Conversion to Motif
**
**	V02-004		DAM0000		Dennis McEvoy		26-jul-1990
**		pass font scaling option to engine for os/2
**
**	V02-003		DAM0000		Dennis McEvoy		05-mar-1990
**		changes for OS/2 port
**
**	V02-002		MHB0000		Mark Bramhall		29-Jun-1989
**		Only pass in explicit formatting options so that the
**		automatic switch to "verbatim" mode can operate.
**
**	V02-001		DAM0001		Dennis A. McEvoy	4-April-1989
**		Change to millimeters for default paper size
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

/* RD:
 * if linking against V1.2 of Motif then need to explicitly include 
 * Xm/ManagerP.h to get XmManagerPart definition
 */
#ifdef MOTIF_V12
#include <Xm/ManagerP.h>	
#endif


#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#pragma standard				/* turn /stand=port back on */

#endif

#ifdef OS2

#define INCL_PM 				/* tell OS/2 to include Presentation Manager defs */
#define PMMLE_INCLUDED                          /* do not include multi-line editing defs */
#include <os2.h>                                /* OS/2 defs */

#include <dvrint.h>				/* DVR internal definitions */
#include <dvsdef.h>				/* DVS Viewer Engine Defns  */
#endif

#ifdef MSWINDOWS
#define NOKERNEL				/* Omit unused definitions */
#define NOUSER					/* from parse of windows.h */
#define NOMETAFILE				/* to save memory on compile. */
#include <windows.h>				/* MS-Windows definitions. */

#include <dvrint.h>				/* DVR internal definitions */
#include <dvsdef.h>				/* DVS Viewer Engine Defns  */
#endif

#ifdef __unix__

#include <Xm/ManagerP.h>			/* rd: 23/7: need XmManagerPart def  */
#include <Xm/AtomMgr.h>

#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#include <dvrint.h>				/* DVR internal definitions */

#endif


#include "dvrwdef.h"			/* Public Defns w/dvr_include */
#include "dvrwint.h"			/* Viewer Constants and structures  */
#include "dvrwptp.h"                       /* dvr windowing prototypes */

/*
**
**  EXTERNALS
**
**/

/*
**
**  FORWARD
**
**/

#define MAX_FALLBACKS 12

/* local prototypes */

PROTO( void prepare_item_list,
		(CDAitemlist *,
       		 DvrViewerWidget) );

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The DECwindows DDIF Viewer Backend is a very thin layer between the
**	CDA converter and the Format Engine.  This backend sets up
**	calls to the Format Engine, depending on whether it is invoked
**	with the Start, Continue, or Stop function code.
**
**  FORMAL PARAMETERS:
**
**	Standard parameter passed in from CDA_CONVERT:
**
**		*function_code - CDA_START, CDA_CONTINUE, CDA_STOP
**		*standard_item_list - CDA standard item list
**		**private_item_list - Viewer context
**		*front_end_handle - DDIF front end converter handle
**		*backend_context  - DDIF converter context
**
**  IMPLICIT INPUTS:
**
**      Format Engine context contained in the Viewer context structure.
**
**  IMPLICIT OUTPUTS:
**
**	The status from the Format Engine is returned in the Viewer
**	context structure.   The page display list returned from the
**	Format Engine is also returned in the context structure in
**	the Continue case.
**
**  FUNCTION VALUE:
**
**      This function usually returns CDA_SUSPEND.
**      CDA_NORMAL is returned after shutdown only.
**
**  SIDE EFFECTS:
**
**	None
**
**--
**/
CDAstatus CDA_APIENTRY Dvr_Viewer_Backend( function_code,
				       standard_item_list,
				       private_item_list,
				       front_end_handle,
				       backend_context)
CDAconstant	  *function_code;
CDAitemlist	  *standard_item_list;
DvrViewerWidget	  *private_item_list;
CDAfrontendhandle *front_end_handle;
CDAuserparam 	  *backend_context;
 {
/* #define MAX_FALLBACKS 10 */
CDAitemlist 	fallback_options[MAX_FALLBACKS]; /* format engine options */
ENG		*format_engine_context;
PAG		new_page;		/* pointer to formatted page info   */
DvrViewerPtr    pWidget;		/* Viewer context   */
CDAstatus status;			/* local status	    */
DvrViewerWidget	pContext;		/* Widget context   */

    /* update converter context */

    pContext = *private_item_list;
    pWidget = &(pContext->dvr_viewer);

    format_engine_context = &(pWidget->Dvr.engine_context);

    /* decide what we were called for */

    switch ((int) *function_code)
    {
    case CDA_START:
	/* if this is the first time through for this document, do init stuff */

	/* save front end context on converter initialization, to be used
	   throughout in subsequent converter calls */
        pWidget->Dvr.ConverterContext = (CDAconverterhandle *) backend_context;

	prepare_item_list( fallback_options, pContext );

	status = dvs_start_conversion(  fallback_options
				      , front_end_handle
				      , format_engine_context
				      , &pWidget->Dvr.root_agg_handle   /* root aggregate handle */
				      , &pWidget->Dvr.dsc_agg_handle   /* descriptor handle */
				      , &pWidget->Dvr.dhd_agg_handle   /* header handle */
				      );
/* cbr change */

        if DVRSuccess(status) {
            if (pWidget->Cbr.mode != FALSE)
                DvrCbrInitEng(pWidget->Dvr.engine_context,
			      (CDAstatus (*)()) pWidget->Cbr.FrgRefCb,
			      (CDAuserparam) pWidget->Cbr.FrgRefParm);
            else
                DvrCbrClearEng(pWidget->Dvr.engine_context);
        }

/* end cbr change */

	break;	/* end CDA_START   */

    case CDA_CONTINUE:
	/* if this is a continuation of a document, we're asking the
	 * format engine for a new page.
	 */

	status = dvs_get_next_page(  format_engine_context
				   , &new_page );

	/* Exit without saving new page on error (non-recoverable for
	   layout enginer) or NULL page reference (invalid) */
	if ( DVR_FAILURE ( status ) || ( new_page == NULL )) break;

	/* put the new page after the last page on the list */

	QUE_APPEND(pWidget->Dvr.page_list, new_page);

	break;

    case CDA_STOP:
	/* Time to shut down.  All outstanding pages should already
	 * be deleted at this point.
	 */
	status = dvs_stop_conversion( format_engine_context );
	break;

    default:
	/* Should never get an unexpected CDA function code */

	status = DVR_FATALERROR;
	break;
    }

    /*
     * Stash the format engine status where the rest of the
     * Viewer will find it
     */

     pWidget->Dvr.WidgetStatus = status;

    /* if everything is going fine, and we are not being requested to stop,
     * keep the converter going for the next call, regardless of any errors
     * encountered; calling application MUST check the widget status field
     * to determine whether CDA_CONTINUE can be used in a follow up call
     * (if the status is not fatal, otherwise, only CDA_STOP should be
     * called, when the application is ready to quit).
     */

    if ( *function_code != CDA_STOP )

	/* CDA_START or CDA_CONTINUE specified; widget status contains
	   actual success of failure status of converter operation */

	status = CDA_SUSPEND;

    else

	/* CDA_STOP specified; again, widget status contains actual
	   status of converter operation, in this case, converter shutdown */

	status = CDA_NORMAL;


    return( status );	    /* return CDA status */

} /* end of X Viewer CDA backend */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Prepare_Item_List converts user/application specifiable
**	options into an item list for the format engine.
**
**  FORMAL PARAMETERS:
**
**	*fallback_options - IN/OUT pointer to an allocated item list
**	pWidget - IN pointer to Viewer context
**
**  IMPLICIT INPUTS:
**
**      Format Engine context contained in the Viewer context structure.
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	none
**
**  SIDE EFFECTS:
**
**	None
**
**--
**/
void	prepare_item_list( fallback_options, pContext)

CDAitemlist    	    *fallback_options; /* format engine options */
DvrViewerWidget	    pContext;

{
    int		    count;		/* fallback option index  */
    DvrViewerPtr    pWidget;		/* Viewer context   */
    unsigned long   off_options;	/* Formatting "off" options */

#ifdef OS2
    long	    font_scale;	   	/*  font scale value for os/2 viewer
					 *  where our fonts do not match screen
				   	 *  dpi
					 */
#endif

	/* init */

	count = 0;

	pWidget = &(pContext->dvr_viewer);

	/* setup format engine fallback parameters  */
	/* by default we have : page is 8.5 x 11 inches,
	 *			top & bottom margins are .25 inches
	 *			left & right margins are .25 inches
	 *  soft_directives, word wrap, and page wrap are done by default
	 *  specific and generic page layout is processed by default
	 *  and, if the document so indicates, "verbatim" mode (turning
	 *  off word wrap and page wrap) will be automatically entered
	 *
	 * The X viewer will always obey the page size defined by
	 * the DDIF document.  The viewer will also always request
	 * specific and generic page layout.  Since these are present
	 * by default we can skip making fallback options for them.
	 * Besides, we have to leave all formatting options as their
	 * default so the engine can automatically use "verbatim" mode.
	 */

	/* if Soft Directives or WordWrap is off, we have to specifically
	 * tell the format engine to ignore them.
	 * Do the same with Layout and Specific Layout
	 */
	off_options = 0;

	if ( !(pWidget->processing_options & DvrSoftDirectives))
	  off_options |= DVS_M_SOFT_DIRECTIVES;

	if ( !(pWidget->processing_options & DvrWordWrap))
	  off_options |= DVS_M_WORD_WRAP;

	if ( !(pWidget->processing_options & DvrLayout))
	  off_options |= DVS_M_LAYOUT;

	if ( !(pWidget->processing_options & DvrSpecificLayout))
	  off_options |= DVS_M_SPECIFIC_LAYOUT;

	if (off_options != 0)
	   {
	   fallback_options[count].item_length = 0;
	   fallback_options[count].item_code = DVS_OFF_OPTIONS;
	   fallback_options[count].CDAitemparam.item_address = (CDAaddress) off_options;
	   count++;
	   }

 	/*  if a paper width and/or height has been specified (in millimeters)
	 *  convert to centipoints and pass to engine
	 */
	if (pWidget->paper_height != 0)
	   {
	      long  paper_height;
	      float float_paper_height;

	      float MM_PER_CPNT   = (float) ((float) DVR_MM_PER_INCH /
					     (float) DVR_CPNTS_PER_INCH);

	      float_paper_height = (float) pWidget->paper_height/
					   MM_PER_CPNT;

	      paper_height = (int) (float_paper_height + 0.5);

  	      fallback_options[count].item_length  = sizeof(long) ;
	      fallback_options[count].item_code    = DVS_PAPER_HEIGHT;
	      fallback_options[count].CDAitemparam.item_address = (CDAaddress) paper_height;
	      count++;
	   }

	if (pWidget->paper_width != 0)
	   {
	      long  paper_width;
              float float_paper_width;

	      float MM_PER_CPNT   = (float) ((float) DVR_MM_PER_INCH /
					     (float) DVR_CPNTS_PER_INCH);

	      float_paper_width = (float)  pWidget->paper_width/
					   MM_PER_CPNT;

	      paper_width = (int) (float_paper_width + 0.5);

  	      fallback_options[count].item_length  = sizeof(long) ;
	      fallback_options[count].item_code    = DVS_PAPER_WIDTH;
	      fallback_options[count].CDAitemparam.item_address = (CDAaddress) paper_width;
	      count++;
	   }


	/*
	 * Specify which fonts we have available to work with
	 */
#ifdef MSWINDOWS
	fallback_options[count].item_length = 0;
	fallback_options[count].item_code = DVS_FONT_SOURCE;

	/*  if there is no work window, and there is a HDC for this
	 *  vw instance, then assume the HDC is representing an 
	 *  off-screen device (e.g. printer,) which is valid
	 */
	if ( (Work(pContext) == 0) &&
	     (DvrHps(pContext) != 0) )
		fallback_options[count].CDAitemparam.item_value =
			DvrHps(pContext);
	else
		fallback_options[count].CDAitemparam.item_value =
	    		CreateDC ("DISPLAY", NULL, NULL, NULL);

	    

        count++;

	/*  pass printer driver string down if present;
	 *  printer driver string will be stored in cur_format field 
	 *  when printing 
	 */
	if ( (Work(pContext) == 0) &&
	     (pWidget->cur_format != NULL) )
	  {
	    fallback_options[count].item_length = strlen(pWidget->cur_format) ;
	    fallback_options[count].item_code = DVS_PRINT_DRIVER;
	    fallback_options[count].CDAitemparam.item_address = 
		(CDAaddress) pWidget->cur_format;
	    count++;
          }
#else
	fallback_options[count].item_length = (CDAitemlength)
                                                    pWidget->num_fonts;
	fallback_options[count].item_code = DVS_FONT_FALLBACK;
	fallback_options[count].CDAitemparam.item_address
	    = (CDAaddress) pWidget->available_fonts;
	count++;
#endif

	/* define error callback, pass back Viewer context as param */

	fallback_options[count].item_length = 0 ;
	fallback_options[count].item_code = DVS_ERROR_PROCEDURE;
	fallback_options[count].CDAitemparam.item_address
	    = (CDAaddress) dvr_error_callback;
	count++;

	fallback_options[count].item_length = 0 ;
	fallback_options[count].item_code = DVS_ERROR_PROCEDURE_PARM;
	fallback_options[count].CDAitemparam.item_address = (CDAaddress) pContext;
	count++;

	/* last item on the list is zeroed */

	fallback_options[count].item_length = 0 ;
	fallback_options[count].item_code = 0;
	fallback_options[count].CDAitemparam.item_address = 0;

    return;

} /* end prepare_item_list */
