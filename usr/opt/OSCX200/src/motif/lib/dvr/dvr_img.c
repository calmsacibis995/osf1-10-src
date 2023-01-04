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
#define Module DVR_IMAGE
#define Ident  "V02-041"

/*
**++
**   COPYRIGHT (c) 1989, 1992 BY
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
** MODULE NAME
**	dvr_image.c (vms)
**	dvr_image.c (ultrix)
**	dvr_img.c   (os/2)
**
** FACILITY:
**
**	Compound Document Architecture (CDA)
**	Compound Document Viewers
**	DIGITAL Document Interchange Format (DDIF)
**
** ABSTRACT:
**
**	Contains image specific format and display code.
**
**
** ENVIORNMENT
**	vms, ultrix, os/2
**
** AUTHORS:
**
**	Barbara F. Bazemore,  28-Nov-1988
**
**
** MODIFIED BY:
**
**	V02-001		DAM0001		Dennis A. McEvoy	30-Mar-1989
**		Do not realize ids child; not necessary; not proper to
**		call realize at this level;
**			DAM0002		Dennis McEvoy		19-may-89
**		correct/uncomment call to FreeColors()
**	V02-002		DAM0003		Dennis A. McEvoy	1-jun-89
**		updates for ultrix isl routines
**	V02-003		DAM0003		Dennis A. McEvoy	14-sep-89
**	 	pass fg/bg to IDS widget to use user's defaults
**	V02-004		KMR000?		Kathy Robinsom		11-Dec-1989
**		Origin of a image is at origin of bounding box, not
**		origin of frame. Changed format_image accordingly.
**	V02-005		DAM0005		Dennis A. McEvoy	05-mar-1990
**	 	changes for OS/2 port
**	V02-006		DAM0006		Dennis A. McEvoy	09-jul-1990
**	 	add condition handling for os/2 isl
**	V02-007		DAM0007		Dennis A. McEvoy	16-jul-1990
**	 	use y dpi for non 1:1 pixel displays
**
**	V02-008		SJM0000		Stephen Munyan		27-Jun-1990
**		Conversion to Motif
**	V02-009		BFB0008		Barbara Bazemore	21-Aug-1990
**		merge in Kenji Yamada-san's image scaling support
**	V02-010		BFB0009		Barbara Bazemore	 9-Nov-1990
**		change from image widgets to image renderings
**	V02-011		DAM0011		Dennis McEvoy		26-nov-1990
**		correct changes in 010 to work on os/2
**	V02-012		BFB0010	        Barbara Bazemore	 7-Dec-1990
**		make image error handling more forgiving.  Continue and
**		try to display when warnings or infos are signalled.
**
**	V02-013		SJM000		Stephen Munyan		13-Feb-1991
**		Fixed logical anding math to work properly in the
**		signal catcher.  As a result ERROR and INFO were both
**		being treated as ERROR cases.
**	V02-014		DAM0014		Dennis McEvoy		19-feb-1991
**		fixes for ultrix/Motif
**
**	V02-015		SJM0000		Stephen Munyan		19-Feb-1991
**		Move presentation surface deletion to the page cleanup
**		routine to avoid corruption problems in Image Services.
**	V02-016		KMRK		Kathy Robinson		23-Feb-1991
**		Query and store image polarity info
**	V02-017		RTG000		Dick Gumbel		05-Mar-1991
**		Cleanup #include's
**
**	V02-018		MHB0000		Mark Bramhall		 8-Mar-1991
**		Use layout engine's scaling information for images.
**
**	V02-020		RAM0001		Ralph A. Mack		24-Apr-1991
**		Add #ifdefs for MS-Windows. Also provided code to
**		initialize the unused "aspect" field until we can do it
**		right. The field is now being initialized to 1.0.
**      V02-021	        DAM0001		Dennis McEvoy		06-may-1991
**		move dvr_int include to cleanup ultrix build
**      V02-022	        DAM0001		Dennis McEvoy		13-may-1991
**		update DAS condition handler of osf/1; note this will
**		change again shortly when Chf goes away for bl5 when
**		condition handling moves to libimg.a
**      V02-023	        DAM0001		Dennis McEvoy		15-may-1991
**		change to ImgDef.h
**      V02-024	        DAM0001		Dennis McEvoy		07-jun-1991
**	 	update chf to use setjmp for unix
**      V02-025	        DAM0001		Dennis McEvoy		10-jun-1991
**	 	take out chfrevert for os/2 msdos
**
**	V02-026		SJM0000		Stephen Munyan		17-Jun-1991
**		DEC C cleanups for Alpha
**	V02-027		SJM0000		Stephen Munyan		24-Jun-1991
**		DEC C cleanups for Alpha - added IdsImage.H to get rid of
**		compilation warnings on Image Services Calls.
**      V02-028	        DAM0001		Dennis McEvoy		05-aug-1991
**	 	renamed header files, removed dollar signs
**      V02-029	        DAM0001		Dennis McEvoy		12-aug-1991
**	 	rename setjmp vars
**	V02-030		RAM0001		Ralph A. Mack		17-Aug-1991
**		Fixing includes for MS-DOS
**	V02-031		DAM0001		Dennis McEvoy		26-mar-1992
**		use portable das bits for sun
**	V02-032		ECR0001		Elizabeth C. Rust	30-Mar-1992
**		Merge in audio code.
**	V02-033		KLM0001		Kevin McBride		12-Jun-1992
**		Merge in audio code.
**      V02-034         RDH034          Don Haney               15-Jul-1992
**              Specify same-directory includes with "", not <>; and
**              conversely for non-same.
**	V02-035		RKN000		Ram Kumar Nori		23-Jul-1992
**		Modified the bounding box coordinates calculations in routine
**		dvr_format_image_object.
**	V02-036		SJM000		Stephen Munyan		23-Jul-1992
**		Changed all occurances of sts to CDAstatus
**	V02-037		RDH006		Don Haney		24-Aug-1992
**		Repair #include ChfDef statement
**	V02-038		DAM0001		Dennis McEvoy		29-sep-1992
**		include image calls for Windows
**	V02-039		DAM0001		Dennis McEvoy		30-sep-1992
**		set image pallette for Windows
**	V02-040		JJT0000		Jeff Tancill		13-Oct-1992
**		Replace use of #elif to accomodate some older compilers,
**		use CDAlocal_DAS symbol to determine which DAS to use.
**      V02-041         RJD000          Ronan Duke		30-Aug-1992
**              Include Xm/MAnagerP.h if linking against Motif V1.2
**
*/

/*
 *  Include files
 */
#include <cdatrans.h>

#ifdef __vms__

#include <dvrint.h>				/* DVR internal definitions */

#pragma nostandard				/* turn off /stand=port for
						   "unclean" X include files */

#include <Xm/Xm.h>				/* Motif definitions */

/* RD:
 * if linking against V1.2 of Motif then need to explicitly include 
 * Xm/ManagerP.h to get XmManagerPart definition
 */
#ifdef MOTIF_V12
#include <Xm/ManagerP.h>	
#endif

#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#pragma standard				/* turn /stand=port back on */

#include <chfdef.h>                             /* condition handler structs */
#include <descrip.h>				/* __vms__ descriptor defs  */
#include <ImgDef.h>
#include <IdsImage.h>				/* Image Services PROTO definitions */
#endif

#ifdef OS2

#define INCL_PM 				/* tell OS/2 to include Presentation Manager defs */
#define PMMLE_INCLUDED                          /* do not include multi-line editing defs */
#include <os2.h>                                /* OS/2 defs */

#include <dvrint.h>				/* DVR internal definitions */
#include <ImgDef.h>
#include <imgrtn.h>

#include <setjmp.h>  				/* for condition handling */

jmp_buf dvr_env;

#endif

#ifdef MSWINDOWS
#define NOKERNEL				/* Omit unused definitions */
#define NOUSER					/* from parse of windows.h */
#define NOMETAFILE				/* to save memory on compile. */
#include <windows.h>				/* MS-Windows definitions. */

#include <dvrint.h>				/* DVR internal definitions */

#include <setjmp.h>  				/* for condition handling */

jmp_buf   dvr_env;

#endif

#ifdef __unix__

#include <Xm/Xm.h>				/* Motif definitions */
#include <Xm/ManagerP.h>			/* rd: 23/7: need XmManagerPart def  */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#ifdef CDAlocal_DAS
#include <chfdef.h>
#else
#include <ChfDef.h>                             /* condition handler structs */
#endif

#include <dvrint.h>				/* DVR internal definitions */

#ifdef CDAlocal_DAS
#include <imgconst.h>
#include <imgtyp.h>
#include <imgptp.h>
#include <idsconst.h>
#include <idsptp.h>
#else
#include <ImgDef.h>
#include <IdsImage.h>				/* Image Services PROTO definitions */
#endif

#include <setjmp.h>  				/* for condition handling */

jmp_buf   dvr_env;
CDAstatus dvr_chf_status;

#endif

#include "dvrwdef.h"                    	/* Public Defns w/dvr_include */
#include "dvrwint.h"                    	/* Viewer Constants and structures  */
#include "dvrwptp.h"                    	/* dvr windowing prototypes */
/*
 * The following prototype has been added since adding in IMGENTRY.H
 * causes DEC C to generate large numbers of errors.
*/

#ifndef MSWINDOWS
#ifndef CDAlocal_DAS
PROTO(void ImgGetFrameAttributes, ());
#endif
#endif


/*
 * Prototypes for DEC C since we can't use lib$routines.h or starlet.h
 * yet since they don't yet work with DEC C.  Hopefully this limitation
 * will be raised by the time we ship so we can remove this kludge.
*/

#ifdef __vms__

PROTO(void * LIB$ESTABLISH, ());
PROTO(CDAstatus LIB$SIG_TO_RET, ());

#endif


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *  	dvr_format_image_object
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine will query ISL for the actual size of the
 *      image (vs. the frame's size). If the size of the image
 *	is larger than the current bounding box, enlarge the
 *	bounding box.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	image		- Image structure
 *	state		- State data
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	One of the input parameters is NULL
 *      DVR_IMGFAIL	Warning, an ISL routine call failed.
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      The ISL routines do not return status, they signal
 *	errors.  If ISL signals an error our condition handler
 *	dvr_image_condition_handler will get control and cause
 *	this routine to return to its caller with DVR_IMGFAIL
 *	status.
 *
 *--
 */

CDAstatus dvr_format_image_object (vw, image, state)
     DvrViewerWidget vw;		/* Viewer widget context pointer */
     IMG image;				/* Image structure */
     STATEREF state;			/* State data */
{
  int		item_count = 0;			/* image_itmlist index		*/

#ifndef MSWINDOWS

#ifdef CDAlocal_DAS
  struct img_getitmlst image_itmlst[10];	/* Image Services Item list */
#else
  struct GET_ITMLST image_itmlst[10];		/* Image Services Item list */
#endif

  unsigned long   spectral_type;		/* image type, bitonal, gray-fixed, etc */
  unsigned long   polarity;        		/* zero is min/max intensity */
  unsigned long   aspect[2];        		/* zero is min/max intensity */

#endif

  OBJ_PRIVATE	object_private;
  unsigned long	status;

  if (  ( vw == (DvrViewerWidget)NULL )
      ||( image == (IMG)NULL )
      ||( state == (STATEREF)NULL ) )
    return DVR_BADPARAM;

#ifndef MSWINDOWS
  /*
   * Check if image frame is available.  If it isn't an
   * info message will already have been issued by DVS.
   */
  /* when isl is properly functioning, this will apply to os/2 as well */
  if ( image->img_fid == 0 )
    return DVR_NORMAL;
#endif

#ifdef __vms__
  LIB$ESTABLISH( dvr_image_condition_handler );
#endif

#if defined(OS2) || defined(__unix__)
  ChfEstablish (dvr_image_condition_handler);
  if (setjmp(dvr_env) != 0)
    {
      /* longjmp has been called indicating we have returned from
       * ISl call with an error, either ChfSignal or ChfStop was called.
       */
      dvr_error_callback(vw, 0L, DVR_IMAGEFAIL, NULL, 0L);
      
#ifdef __unix__
      dvr_error_callback(vw, 0L, dvr_chf_status, NULL, 0L);
#endif
      
      return DVR_IMAGEFAIL;
    }
#endif

#ifndef MSWINDOWS
  /* get attributes not necessary on windows */

  /*
   *   Find out compression and bitonal/greyscale/color type
   */
  
  image_itmlst[item_count].GetL_Code     = Img_SpectralMapping;
  image_itmlst[item_count].GetL_Length   = sizeof(spectral_type);
  image_itmlst[item_count].GetA_Buffer   = (char *)&spectral_type;
  image_itmlst[item_count].GetA_Retlen   = 0;
  image_itmlst[item_count].GetL_Index    = 0;
  item_count++;
  
  image_itmlst[item_count].GetL_Code     = Img_BrtPolarity;
  image_itmlst[item_count].GetL_Length   = sizeof(polarity);
  image_itmlst[item_count].GetA_Buffer   = (char *)&polarity;
  image_itmlst[item_count].GetA_Retlen   = 0;
  image_itmlst[item_count].GetL_Index    = 0;
  item_count++;
  /*
    image_itmlst[item_count].GetL_Code     = Img_PxlAspectRatio;
    image_itmlst[item_count].GetL_Length   = sizeof(aspect);
    image_itmlst[item_count].GetA_Buffer   = (char *)&aspect;
    image_itmlst[item_count].GetA_Retlen   = 0;
    image_itmlst[item_count].GetL_Index    = 0;
    item_count++;
    */
  aspect[0] = aspect[1] = 1;		/* For now... */
  
  image_itmlst[item_count].GetL_Code     = 0;
  image_itmlst[item_count].GetL_Length   = 0;
  image_itmlst[item_count].GetA_Buffer   = 0;
  image_itmlst[item_count].GetA_Retlen   = 0;
  image_itmlst[item_count].GetL_Index    = 0;
  
  ImgGetFrameAttributes(image->img_fid, image_itmlst);
  
  
#ifdef __unix__
  ChfRevert();
#endif

#endif /* not Windows */
  
  /*
   * Image object specific set-up.  Create an image_private
   * data structure off of the regular image object and init it.
   */
  object_private = ((OBJ_PRIVATE) ((OBJ) image)->obj_user_private);
  
  status = dvr_alloc_struct (vw, img_private, &(object_private->private_info) );

  if ( DVR_FAILURE ( status ))
    /* Unexpected memory allocation error */
    return status;

  /* init the image private structure */
#if defined(OS2) || defined(MSWINDOWS)
  ((IMG_PRIVATE)(object_private->private_info))->img_widget_created = FALSE;

  /*  initialize the image's device dependent bitmap to null on
   *  Windows; this field will be filled in the first time the
   *  the image is displayed in dvr_display_image_object in dvr_grad.c
   *  (ala dvr_grax.c)
   */
  ((IMG_PRIVATE)(object_private->private_info))->img_ddb = 0;
  ((IMG_PRIVATE)(object_private->private_info))->img_hpallette = 0;
#else
  ((IMG_PRIVATE)(object_private->private_info))->img_rendering = NULL;
#endif
  
#ifndef MSWINDOWS
  ((IMG_PRIVATE)(object_private->private_info))->img_polarity = (int)polarity;
  ((IMG_PRIVATE)(object_private->private_info))->img_aspect   =
    (float)aspect[0]/(float)aspect[1];
#endif

  /*
   * Get the bounding box size from our parent the image frame.
   * This allows us to save away the box size for use when we
   * display the image.
   */
  state->x_offset = state->x_relative + image->img_ll_x;
  state->y_offset = state->y_relative + image->img_ll_y;

  state->bbox_ll_x = state->x_offset +  (CDAmeasurement) (image->img_ll_x * state->x_scale);
  state->bbox_ll_y = state->y_offset +  (CDAmeasurement) (image->img_ll_y * state->y_scale);
  state->bbox_ur_x = state->bbox_ll_x + (CDAmeasurement) (image->img_width * state->x_scale);
  state->bbox_ur_y = state->bbox_ll_y + (CDAmeasurement) (image->img_height * state->y_scale);

   
  /* adjust the y positioning by the height of the image */
  state->x_offset = state->bbox_ll_x;
  state->y_offset = state->bbox_ur_y;
 
  /* NOTE: for os/2, the actual y_pos for the image window is not
   * calculated until dvr_display_image_object(); y_pos is the lower
   * left corner of the window for Presentation Manager; y_pos is the
   * upper left position on DECwindows, so it need not be adjusted there.
   */
  
  /* Successful completion of dvr_format_image */
  return DVR_NORMAL;
}

/*++
 *
 *  FUNCTION NAME:
 *
 *      dvr_image_condition_handler()
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      This routine is called when an exception or condition occurs which
 *      would ordinarily cause the Viewer, and anyone who invoked the Viewer,
 *      to exit.  This routine is specifically to take care of conditions
 *      raised by the GKS, ISL, and IDS support routines.  Some of these
 *      are "bad" in that they call LIB$STOP instead of just returning a
 *      bad status.  This routine catches the bad status and returns it as
 *      the value of the shell routine surrounding each support routine call.
 *
 *      To use this routine, write a shell routine which calls the desired
 *      support routine.  It should look like:
 *
 *          int Dvr$$foo_shell( foo_args )
 *              {
 *              int status;
 *
 *              status = DVR_NORMAL;
 *              VAXC$ESTABLISH( dvru$graphic_condition_handler );
 *              foo( foo_args );
 *              return( status );
 *              }
 *
 *  FORMAL PARAMETERS:
 *
 *      signal  - signal vector
 *      mechanism - error mechanism arguments
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      DVR_IMGFAIL
 *
 *  FUNCTION VALUE:
 *
 *      Whatever error status caused the condition
 *
 *  SIDE EFFECTS:
 *      None
 *
 *--
 */
#if defined(OS2) || defined(MSWINDOWS)

void OS2_APIENTRY dvr_image_condition_handler (sigarg, mcharg)
    unsigned short      sigarg;
    unsigned short      mcharg;
{
    /* Error has occured processing image content, longjmp back. */
    longjmp(dvr_env, -1);
}

#endif

#ifdef __vms__

unsigned long dvr_image_condition_handler(signal, mechanism)
 struct chf$signal_array    *signal;
 struct chf$mech_array      *mechanism;

{
 long			severity;		/* Severity of the error being returned */

 /* Only die if this is an error or a fatal.  Sometimes ISL signals
  * infos and warnings, which it can recover from.
  */

 /*
  * Note that we must mask out the status area then do comparisions
  * for the error type since error types are as follows:
  *
  * 0 = Warning, 1 = Success, 2 = Error, 3 = Info, 4 = Fatal
 */

 severity = (signal->chf$l_sig_name & STS$M_SEVERITY);

 if ((severity == STS$K_ERROR) || (severity ==  STS$K_SEVERE))
    {
     /*
      * Stuff in our error status over the one which was actually
      * signalled.
      */

#if 0
      /*
       * If Image Services starts acting up change the zero in the
       * line above to a 1 and recompile.  It will cause the status
       * code returned to be dumped to your terminal.
      */

      printf("Image Services Error = %d, returning DVR_IMAGEFAIL\n", signal->chf$l_sig_name);
#endif

      signal->chf$l_sig_name = DVR_IMAGEFAIL;

     /*
      * Return from the procedure which established this exception handler
      */
      LIB$SIG_TO_RET( signal, mechanism );
    }

 else
    /* This is just an info or warning.  Continue on our merry way. */
    return(DVR_NORMAL);
}
#endif

#ifdef __unix__
unsigned long dvr_image_condition_handler(signal, mechanism)
 unsigned int    *signal;
 unsigned int    *mechanism;

{
    /* Error has occured processing image content, longjmp back. */
    dvr_chf_status = signal[1];
    longjmp(dvr_env, -1);
}

#endif


/*
 *++
 *
 *  FUNCTION NAME:
 *
 *  	dvr_cleanup_page
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine will perform any tasks needed after
 *	a page is no longer displayed.  All image widgets renderings
 *	were created for this page will be deleted.
BEGIN AUDIO STUFF
 *      Also, any audio buttons will be removed.
END AUDIO STUFF
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *
 *  FUNCTION VALUE:
 *
 *	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	One of the input parameters is NULL
 *      DVR_IMGFAIL	Warning, an ISL routine call failed.
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      The ISL routines do not return status, they signal
 *	errors.  If ISL signals an error our condition handler
 *	dvr_image_condition_handler will get control and cause
 *	this routine to return to its caller with DVR_IMGFAIL
 *	status.
 *
 *--
 */

CDAstatus dvr_cleanup_page (vw)
     DvrViewerWidget vw;		/* Viewer widget context pointer */
{
  IMG_LIST_STRUCT 	isl_list;
  PAG_PRIVATE 	cur_page_private;
  PAG 		current_page;
  OBJ_PRIVATE		object_private;
  IMG_PRIVATE		image_private;

  /*
   *	cruise through the page structure down to the image widget list
   */

  current_page = vw->dvr_viewer.Dvr.current_page;
  if ( current_page == NULL ) return DVR_NORMAL;	/* No current page
							   yet; initial page */
  cur_page_private = (PAG_PRIVATE) current_page->pag_user_private;
  if ( cur_page_private == NULL ) return DVR_NORMAL;	/* Page not yet
							   formatted */
  /*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
  dvr_delete_audio_buttons(current_page);
#endif
  /*END AUDIO STUFF*/

#if defined(OS2) 

  /* Destroy each widget(window hwnd) on the current page. */
  FOREACH_INLIST(cur_page_private->img_list, isl_list, IMG_LIST_STRUCT)
    {
      dvr_kill_widget(isl_list->hwnd_id);

      /* Mark the widget as gone	*/
      object_private = (OBJ_PRIVATE) ((OBJ)isl_list->object_data);

      image_private = (IMG_PRIVATE)(object_private->private_info);
      image_private->img_widget_created = FALSE;
    }

#else
#if defined(MSWINDOWS)
  FOREACH_INLIST(cur_page_private->img_list, isl_list, IMG_LIST_STRUCT)
    {
      /*  for Windows, we need to free the Device Dependent Bitmap for
       *  each image
       */			
      object_private = isl_list->object_data;
      image_private = (IMG_PRIVATE)(object_private->private_info);
      
      if ( (image_private != NULL) && 
	   (image_private->img_ddb != NULL) )
	{
	  if (vw->dvr_viewer.img_free_ddb_rtn != NULL)
              (*vw->dvr_viewer.img_free_ddb_rtn) (image_private->img_ddb);
	  else 
	      /* must be using the default pattern, delete it */
	      DeleteObject(image_private->img_ddb);	

	  if (image_private->img_hpallette != 0)
	      /* delete pallette */
	      DeleteObject(image_private->img_hpallette);	

	  /* Mark the rendering as gone	    */
	  image_private->img_ddb = 0;
	  image_private->img_hpallette = 0;
	}

    }

#else  /* X windows, vms, ultrix */

  /* Destroy each rendering on the current page. */
  FOREACH_INLIST(cur_page_private->img_list, isl_list, IMG_LIST_STRUCT)
    {
      object_private = isl_list->object_data;
      image_private = (IMG_PRIVATE)(object_private->private_info);
      
      if (image_private != NULL
	  && image_private->img_rendering != NULL
	  && image_private->img_rendering != (struct IdsRendering *) TRUE )
	{
	  IdsDeleteRendering(image_private->img_rendering);

	  /* Mark the rendering as gone	    */
	  image_private->img_rendering = NULL;
	}

      if (image_private != NULL
	  && image_private->img_presentation != 0 )
	{
	  IdsDeletePresentSurface(image_private->img_presentation);
	  
	  /* Mark the presentation surface as gone	    */
	  
	  image_private->img_presentation = 0;
	}
    }
#endif
#endif

  /*
   * Remove all image private data associated with this page from the
   * page queue, and delete their associated structures.
   */
  FOREACH_POPLIST ( cur_page_private->img_list, isl_list,
		   IMG_LIST_STRUCT)
    {
      (void) dvr_dealloc_memory (vw,
				 (unsigned long) sizeof( struct img_list_struct),
				 (void * *) isl_list );
    }

  dvr_delete_page_colors(vw);

  return DVR_NORMAL;		/* end of dvr_cleanup_page  */
}
