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

/*******************************************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*******************************************************************************/

/*******************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	This Xie module consists of wrapper routines for all Xie protocol
**	requests.  These routines perform the byte swapping necessary
**      when the client and server are of different endian-ness.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Gary L. Grebus
**
**  CREATION DATE:
**
**      Fri Dec  7 11:45:27 1990
**
*******************************************************************************/

/*
**  Include files
*/
#include <stdio.h>

/*
**  Core X Includes
*/
#include <X.h>
#include <Xproto.h>
#include <misc.h>
#include <extnsionst.h>
#include <dixstruct.h>
/*
**  XIE Includes
*/
#include <XieAppl.h>
#include <XieDdx.h>
#include <XieProto.h>
#include <XieServer.h>
#include <XieMacros.h>

/*
**  Table of contents
*/
/* Utility Routines */
#if defined(VMS) && !defined(VXT) && !defined(ALPHA)
void SwapLongs();
void SwapShorts();
#endif
void SwapTmpEntries();
int SProcXieSimpleReq();
int SProcXieResourceReq();

/* Routines for swapping the Xie requests */
int SProcInitSession();
int SProcQueryEvents();
int SProcSelectEvents();
int SProcSetOpDefaults();
int SProcCreateResource();
int SProcBindMapToFlo();
int SProcAbortFlo();
int SProcExecuteFlo();
int SProcTapFlo();
int SProcAbortTransport();
int SProcGetStream();
int SProcGetTile();
int SProcPutStream();
int SProcPutTile();
int SProcSetTransport();
int SProcExport();
int SProcFreeExport();
int SProcImport();
int SProcQueryExport();
int SProcArea();
int SProcAreaStats();
int SProcArith();
int SProcCalcHistogram();
int SProcChromeCom();
int SProcChromeSep();
int SProcCompare();
int SProcConstrain();
int SProcCrop();
int SProcDither();
int SProcFill();
int SProcLogical();
int SProcLuminance();
int SProcMatchHistogram();
int SProcMath();
int SProcMirror();
int SProcPoint();
int SProcPointStats();
int SProcRotate();
int SProcScale();
int SProcTranslate();


/*
**  Equated Symbols
*/

/*
**  MACRO definitions
*/

/*
** External references
*/
externalref int (*xieDixProc[X_ieLastRequest])();



/*-----------------------------------------------------------------------
------------- Request swapping utility routines, as lifted from ---------
--------------------- the X11R4 MIT sample server -----------------------
------------------------------------------------------------------------*/
/* Thanks to Jack Palevich for testing and subsequently rewriting all this */

#if defined(VMS) && !defined(VXT) && !defined(ALPHA)
/* Byte swap a list of longs */

void
SwapLongs (list, count)
	register long *list;
	register unsigned long count;
{
	register int n;

	while (count >= 8) {
	    swapl(list+0, n);
	    swapl(list+1, n);
	    swapl(list+2, n);
	    swapl(list+3, n);
	    swapl(list+4, n);
	    swapl(list+5, n);
	    swapl(list+6, n);
	    swapl(list+7, n);
	    list += 8;
	    count -= 8;
	}
	if (count != 0) {
	    do {
		swapl(list, n);
		list++;
	    } while (--count != 0);
	}
}


/* Byte swap a list of shorts */

void
SwapShorts (list, count)
	register short *list;
	register unsigned long count;
{
	register int n;

	while (count >= 16) {
	    swaps(list+0, n);
	    swaps(list+1, n);
	    swaps(list+2, n);
	    swaps(list+3, n);
	    swaps(list+4, n);
	    swaps(list+5, n);
	    swaps(list+6, n);
	    swaps(list+7, n);
	    swaps(list+8, n);
	    swaps(list+9, n);
	    swaps(list+10, n);
	    swaps(list+11, n);
	    swaps(list+12, n);
	    swaps(list+13, n);
	    swaps(list+14, n);
	    swaps(list+15, n);
	    list += 16;
	    count -= 16;
	}
	if (count != 0) {
	    do {
		swaps(list, n);
		list++;
	    } while (--count != 0);
	}
}
#endif					/* VMS & !VXT */

/* Byte swap a list of template IDC entries */

void
SwapTmpEntries(list, count)
	register XieTmpEntryRec *list;
	register unsigned long count;
{
	register int n;
	int i;

	while (count > 0) {
	    swapl(&(list->x), n);
	    swapl(&(list->y), n);
	    swapfloat(&(list->value), n);
	    list++; count--;
	}
}

/* The following is used for all requests that have
   no fields to be swapped (except "length") */
int
SProcXieSimpleReq(client)
	register ClientPtr client;
{
    register char n;

    REQUEST(xieReq);
    swaps(&stuff->length, n);
    return( CallProc_(xieDixProc)(client) );
}

/* The following is used for all requests that have
   only a resource type / resource id pair to be swapped, coming
   right after the "length" field */
int
SProcXieResourceReq(client)
	register ClientPtr client;
{
    register char n;

    REQUEST(xieCreateByValueReq);
    swaps(&stuff->length, n);
    swapl(&stuff->resource_type, n);
    swapl(&stuff->resource_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------ Init Session Request ---------------------------
------------------------------------------------------------------------*/
int SProcInitSession( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieInitSessionReq);

    swaps(&stuff->length, n);
    swaps(&stuff->major_version, n);
    swaps(&stuff->minor_version, n);
    return( CallProc_(xieDixProc)(client) );
	
}

/*-----------------------------------------------------------------------
------------------------Query Events Procedure --------------------------
------------------------------------------------------------------------*/
int SProcQueryEvents( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieQueryEventsReq);

    swaps(&stuff->length, n);
    swapl(&stuff->event_param, n);
    return( CallProc_(xieDixProc)(client) );
	
}

/*-----------------------------------------------------------------------
------------------------Select Events Procedure -------------------------
------------------------------------------------------------------------*/
int SProcSelectEvents( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieSelectEventsReq);

    swaps(&stuff->length, n);
    swapl(&stuff->event_mask, n);
    return( CallProc_(xieDixProc)(client) );
	
}

/*-----------------------------------------------------------------------
------------------------Set Operational Defaults-------------------------
------------------------------------------------------------------------*/
int SProcSetOpDefaults( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieSetOpDefaultsReq);

    swaps(&stuff->length, n);
    SwapLongs(&stuff->levels[0], XieK_MaxComponents);
    return( CallProc_(xieDixProc)(client) );
	
}

/*-----------------------------------------------------------------------
------------------ Create Resource by Reference -------------------------
------------------------------------------------------------------------*/
int
SProcCreateResourceByRef(client)
	register ClientPtr client;
{
    register char n;

    REQUEST(xieCreateByReferenceReq);

    swaps(&stuff->length, n);
    swapl(&stuff->resource_type, n);
    swapl(&stuff->resource_id, n);
    swapl(&stuff->photomap_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------ Create Resource by Value -----------------------------
------------------------------------------------------------------------*/
int SProcCreateResourceByValue( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieCreateByValueReq);

    swaps(&stuff->length, n);
    swapl(&stuff->resource_type, n);
    swapl(&stuff->resource_id, n);

    switch( stuff->resource_type ) {

      case XieK_IdcCpp :
          {
	  REQUEST(xieCreateCppReq);
	  swapl(&stuff->photomap_id, n);
	  swapl(&stuff->x, n);
	  swapl(&stuff->y, n);
          }
	  break;

      case XieK_Photoflo :
      case XieK_Photomap :
          {
	  REQUEST(xieCreatePhotoReq);
	  swapl(&stuff->width, n);
	  swapl(&stuff->height, n);
	  swapfloat(&stuff->aspect_ratio, n);
	  SwapLongs(stuff->levels, XieK_MaxComponents);
          }
	  break;

	case XieK_IdcRoi :
	   {
	   REQUEST(xieCreateRoiReq);
	   swapl(&stuff->x, n);
	   swapl(&stuff->y, n);
	   swapl(&stuff->width, n);
	   swapl(&stuff->height, n);
           }
	   break;

	case XieK_IdcTmp :
	   {
	   REQUEST(xieCreateTmpReq);
	   swapl(&stuff->center_x, n);
	   swapl(&stuff->center_y, n);
	   swapl(&stuff->data_count, n);
	   SwapTmpEntries(&((&stuff->data_count)[1]), stuff->data_count);
           }
	   break;
      }
    return( CallProc_(xieDixProc)(client) );
	
}

/*-----------------------------------------------------------------------
--------------------Bind Map to Flo Request -----------------------------
------------------------------------------------------------------------*/
int SProcBindMapToFlo( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieBindPhotomapReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photoflo_id, n);
    swapl(&stuff->photomap_id, n);
    return( CallProc_(xieDixProc)(client) );
	
}

/*-----------------------------------------------------------------------
------------------------- Abort Photoflo Request ------------------------
------------------------------------------------------------------------*/
int SProcAbortFlo( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieAbortPhotofloReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photoflo_id, n);
    return( CallProc_(xieDixProc)(client) );
	
}

/*-----------------------------------------------------------------------
------------------------- Execute Photoflo Request ----------------------
------------------------------------------------------------------------*/
int SProcExecuteFlo( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieExecutePhotofloReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photoflo_id, n);
    return( CallProc_(xieDixProc)(client) );
	
}

/*-----------------------------------------------------------------------
---------------------------- Tap Photoflo Request -----------------------
------------------------------------------------------------------------*/
int SProcTapFlo( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieTapPhotofloReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photoflo_id, n);
    swapl(&stuff->photo_id, n);
    return( CallProc_(xieDixProc)(client) );
	
}

/*-----------------------------------------------------------------------
-------------------------- Abort Transport Request ----------------------
------------------------------------------------------------------------*/
int SProcAbortTransport( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieAbortTransportReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id, n);
    swapl(&stuff->plane_mask, n);
    return( CallProc_(xieDixProc)(client) );
	
}

/*-----------------------------------------------------------------------
---------------------------- Get Stream Request -------------------------
------------------------------------------------------------------------*/
int SProcGetStream( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieGetStreamReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id, n);
    swapl(&stuff->max_bytes, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
----------------------------- Get Tile Request --------------------------
------------------------------------------------------------------------*/
int SProcGetTile( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieGetTileReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photomap_id, n);
    swapl(&stuff->x, n);
    swapl(&stuff->y, n);
    swapl(&stuff->width, n);
    swapl(&stuff->height, n);
    swapl(&stuff->plane_mask, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
---------------------------- Put Stream Request -------------------------
------------------------------------------------------------------------*/
int SProcPutStream( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xiePutStreamReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id, n);
    swapl(&stuff->byte_count, n);
    /* No swapping of the image data */
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
----------------------------- Put Tile Request --------------------------
------------------------------------------------------------------------*/
int SProcPutTile( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xiePutTileReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photomap_id, n);
    swapl(&stuff->x, n);
    swapl(&stuff->y, n);
    swapl(&stuff->width, n);
    swapl(&stuff->height, n);
    swapl(&stuff->plane_mask, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
-------------------------Set Transport Request --------------------------
------------------------------------------------------------------------*/
int SProcSetTransport( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieSetTransportReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id, n);
    swapl(&stuff->compression_parameter, n);
    swapl(&stuff->plane_mask, n);
    SwapLongs(stuff->scanline_stride, XieK_MaxPlanes);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------- Export Image Request --------------------------
------------------------------------------------------------------------*/
int SProcExport( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieExportReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id, n);
    swapl(&stuff->drawable_id, n);
    swapl(&stuff->gc_id, n);
    swapl(&stuff->src_x, n);
    swapl(&stuff->src_y, n);
    swapl(&stuff->width, n);
    swapl(&stuff->height, n);
    swapl(&stuff->dst_x, n);
    swapl(&stuff->dst_y, n);
    swapl(&stuff->photo_lut_id, n);
    swapl(&stuff->colormap_id, n);
    swapfloat(&stuff->match_limit, n);
    swapfloat(&stuff->gray_limit, n);
    
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
-------------------------- Free Export Request --------------------------
------------------------------------------------------------------------*/
int SProcFreeExport( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieFreeExportReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------- Import Image Request --------------------------
------------------------------------------------------------------------*/
int SProcImport( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieImportReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photomap_id, n);
    swapl(&stuff->drawable_id, n);
    swapl(&stuff->colormap_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
--------------------- Query Export Context Request ----------------------
------------------------------------------------------------------------*/
int SProcQueryExport( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieQueryExportReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id, n);
    swapl(&stuff->lut_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
--------------------------------- Area Request --------------------------
------------------------------------------------------------------------*/
int SProcArea( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieAreaReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->ipcc_id, n);
    swapl(&stuff->ipct_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------------ Area Stats Request -----------------------
------------------------------------------------------------------------*/
int SProcAreaStats( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieAreaStatsReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->idc_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
--------------------------------- Arith Request -------------------------
------------------------------------------------------------------------*/
int SProcArith( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieArithReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src1_photo_id, n);
    swapl(&stuff->src2_photo_id, n);
    SwapLongs(stuff->constants, XieK_MaxComponents);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->idc_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
-------------------------Calculate Histogram Request --------------------
------------------------------------------------------------------------*/
int SProcCalcHistogram( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieCalcHistReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id, n);
    swapl(&stuff->idc_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------Chrominance Combine Request ---------------------
------------------------------------------------------------------------*/
int SProcChromeCom( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieChromeComReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id1, n);
    swapl(&stuff->photo_id2, n);
    swapl(&stuff->photo_id3, n);
    swapl(&stuff->dst_photo_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------ Chrominance Separate Request -------------------
------------------------------------------------------------------------*/
int SProcChromeSep( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieChromeSepReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo, n);
    swapl(&stuff->dst_photo, n);
    swapl(&stuff->component, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------------- Compare Request -------------------------
------------------------------------------------------------------------*/
int SProcCompare( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieCompareReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src1_photo_id, n);
    swapl(&stuff->src2_photo_id, n);
    SwapLongs(stuff->constants, XieK_MaxComponents);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->idc_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------------ Constrain Request ------------------------
------------------------------------------------------------------------*/
int SProcConstrain( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieConstrainReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    SwapLongs(stuff->levels, XieK_MaxComponents);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
-------------------------------- Crop Request ---------------------------
------------------------------------------------------------------------*/
int SProcCrop( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieCropReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->idc_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------------ Dither Request ---------------------------
------------------------------------------------------------------------*/
int SProcDither( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieDitherReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    SwapLongs(stuff->levels, XieK_MaxComponents);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
-------------------------------- Fill Request ---------------------------
------------------------------------------------------------------------*/
int SProcFill( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieFillReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->dst_idc_id, n);
    SwapLongs(stuff->constant, XieK_MaxComponents);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
---------------------------- Logical Request ----------------------------
------------------------------------------------------------------------*/
int SProcLogical( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieLogicalReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src1_photo_id, n);
    swapl(&stuff->src2_photo_id, n);
    SwapLongs(stuff->constants, XieK_MaxComponents);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->idc_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
-------------------------- Luminance Request ----------------------------
------------------------------------------------------------------------*/
int SProcLuminance( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieLuminanceReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
----------------------------- Math Request ------------------------------
------------------------------------------------------------------------*/
int SProcMath( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieMathReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->idc_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
----------------------- Match Histogram Request -------------------------
------------------------------------------------------------------------*/
int SProcMatchHistogram( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieMatchHistogramReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id1, n);
    swapl(&stuff->photo_id2, n);
    swapl(&stuff->idc_id, n);
    swapfloat(&stuff->param1, n);
    swapfloat(&stuff->param2, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
----------------------------- Mirror Request ----------------------------
------------------------------------------------------------------------*/
int SProcMirror( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieMirrorReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------------ Point Request ----------------------------
------------------------------------------------------------------------*/
int SProcPoint( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xiePointReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->idc_id, n);
    swapl(&stuff->trans_photo_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
--------------------------- Point Statistics Request --------------------
------------------------------------------------------------------------*/
int SProcPointStats( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xiePointStatsReq);

    swaps(&stuff->length, n);
    swapl(&stuff->photo_id, n);
    swapl(&stuff->x, n);
    swapl(&stuff->y, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
---------------------------------- Rotate Request -----------------------
------------------------------------------------------------------------*/
int SProcRotate( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieRotateReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    swapfloat(&stuff->angle, n);
    swapl(&stuff->width, n);
    swapl(&stuff->height, n);
    SwapLongs(stuff->fill, XieK_MaxComponents);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
-------------------------------- Scale Request --------------------------
------------------------------------------------------------------------*/
int SProcScale( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieScaleReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src_photo_id, n);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->width, n);
    swapl(&stuff->height, n);
    return( CallProc_(xieDixProc)(client) );
}

/*-----------------------------------------------------------------------
------------------------------ Translate Request ------------------------
------------------------------------------------------------------------*/
int SProcTranslate( client )
 ClientPtr client;	/* client pointer				    */
{
    register char n;
    REQUEST(xieTranslateReq);

    swaps(&stuff->length, n);
    swapl(&stuff->src1_photo_id, n);
    swapl(&stuff->src2_photo_id, n);
    swapl(&stuff->src_idc_id, n);
    swapl(&stuff->dst_photo_id, n);
    swapl(&stuff->dst_idc_id, n);
    return( CallProc_(xieDixProc)(client) );
}

/* End of MODULE XIESWAPREQ.C */

