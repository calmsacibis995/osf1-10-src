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

/***********************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
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

******************************************************************/

/************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	The XieMacros module consists of all XIE macros.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Richard J. Piccolo
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      May 3, 1989
**
************************************************************************/

    /*
    **	This macro sends "code" and "id" if "test" is FALSE, NULL, or = 0.
    **
    **	Error exit - code
    */
#define OkIf_(test,id,code) \
    if(!(test)) {SendErrorToClient(client,xieReqCode,stuff->opcode,id,code);\
		 return(code);} else

#define RequestOkIf_(test,id)	OkIf_(test,id,BadRequest)
#define ValueOkIf_(test,id)	OkIf_(test,id,BadValue)
#define MatchOkIf_(test,id)	OkIf_(test,id,BadMatch)
#define DrawableOkIf_(test,id)	OkIf_(test,id,BadDrawable)
#define AccessOkIf_(test,id)	OkIf_(test,id,BadAccess)
#define AllocOkIf_(test,id)	OkIf_(test,id,BadAlloc)
#define ColorOkIf_(test,id)	OkIf_(test,id,BadColor)
#define GCOkIf_(test,id)	OkIf_(test,id,BadGC)
#define IDChoiceOkIf_(test,id)	OkIf_(test,id,BadIDChoice)
#define LengthOkIf_(test,id)	OkIf_(test,id,BadLength)

#define BadRequest_(id)		OkIf_(FALSE,id,BadRequest)
#define BadValue_(id)		OkIf_(FALSE,id,BadValue)
#define BadMatch_(id)		OkIf_(FALSE,id,BadMatch)
#define BadDrawable_(id)	OkIf_(FALSE,id,BadDrawable)
#define BadAccess_(id)		OkIf_(FALSE,id,BadAccess)
#define BadAlloc_(id)		OkIf_(FALSE,id,BadAlloc)
#define BadColor_(id)		OkIf_(FALSE,id,BadColor)
#define BadGC_(id)		OkIf_(FALSE,id,BadGC)
#define BadIDChoice_(id)	OkIf_(FALSE,id,BadIDChoice)
#define BadLength_(id)		OkIf_(FALSE,id,BadLength)

    /*
    **	This macro guarantees that a Photomap contains PCM data.
    */
#define BePCM_(img,id)\
	if(IsPointer_(img) && (AnyPending_(img) || !IsPCM_(img)))\
	    {int _s = UtilBePCM(img); OkIf_(_s==Success, (id), _s);} else

    /*
    **	This macro calls a function using index [stuff->opcode]
    */
#define CallProc_(table) (*(table)[stuff->opcode])

    /*
    **	This macro checks that an XIE resource ID is valid as a new resource.
    **
    **	Error exit - BadIDChoice
    */
#if   defined(X11R3) && !defined(DWV3)
#define CheckNewResource_(id,rType)\
    IDChoiceOkIf_(client->clientAsMask==CLIENT_BITS(id) && !(id&SERVER_BIT)\
		  && LookupID(id,rType,RC_xie)==0, id)
#else /* X11R4 || DWV3 */
#define CheckNewResource_(id,rType)\
    IDChoiceOkIf_(client->clientAsMask==CLIENT_BITS(id) && !(id&SERVER_BIT)\
		  && LookupIDByType(id,rType)==0, id)
#endif

    /*
    **	This macro hides the difference between X11R3 and X11R4 AddResource.
    */
#if   defined(X11R3) && !defined(DWV3)
#define AddResource_(id,type,res)\
	ResId_(res)=id;\
	AddResource(id,type,res,UtilFreeResource,RC_xie)
#else /* X11R4 || DWV3 */
#define AddResource_(id,type,res)\
	ResId_(res)=id;\
	AllocOkIf_(AddResource(id,type,res),id)
#endif

    /*
    **	This macro hides the difference between X11R3 and X11R4 FreeResource.
    */
#if   defined(X11R3) && !defined(DWV3)
#define FreeResource_(id)\
	FreeResource(id,RC_NONE)
#else /* X11R4 || DWV3 */
#define FreeResource_(id)\
	FreeResource(id,RT_NONE)
#endif

    /*
    **	This macro hides the difference between X11R3 and X11R4 Id Lookup
    */
#if   defined(X11R3) && !defined(DWV3)
#define LookupId_(id,type)\
	LookupID(id,type,RC_xie)
#else /* X11R4 || DWV3 */
#define LookupId_(id,type)\
	LookupIDByType(id,type)
#endif

    /*
    **	This macro calls the LookupPhotos routine and tests the result returned.
    **
    **	Error exit - result of LookupPhotos if result not "Success"
    */
#define LookupPhotos_(pflo,pmap,src_id,dst_id)\
	{int status = LookupPhotos(client,(pflo),(pmap),(src_id),(dst_id));\
	 if( status ) return status;}

    /*
    **	This macro calls the LookupIdc routine and tests the result returned.
    **
    **	Error exit - result of LookupIdc if result not "Success"
    */
#define LookupIdc_(pidc,idc_id,type)\
	{int status = LookupIdc(client,(pidc),(idc_id),(type));\
	 if( status ) return status;}
    /*
    **	This macro calls the LookupCppRoi routine and tests the result.
    **
    **	Error exit - result of LookupCppRoi if result not "Success"
    */
#define LookupCppRoi_(pres, pudp,idc_id)\
	{int status = LookupCppRoi(client,(pres),(pudp),(idc_id));\
	 if( status ) return status;}

    /*
    **	This macro looks up a colormap.
    */
#if   defined(X11R3) && !defined(DWV3)
#define LookupColormap_(pColormap,colormapID)\
	pColormap = (ColormapPtr)LookupID(colormapID,RT_COLORMAP,RC_CORE)
#else /* X11R4 || DWV3 */
#define LookupColormap_(pColormap,colormapID)\
	pColormap = (ColormapPtr)LookupIDByType(colormapID,RT_COLORMAP)
#endif

    /*
    **	This macro looks up a drawable.
    */
#if   defined(X11R3) && !defined(DWV3)
#define LookupDrawable_(client,drawID)\
        if(client->lastDrawableID != drawID)\
	    {\
	    client->lastDrawable = (DrawablePtr)\
				    LookupID(drawID,RT_DRAWABLE,RC_CORE);\
	    client->lastDrawableID = drawID;\
	    }\
	if(client->lastDrawable==0 || \
	   client->lastDrawable->type != DRAWABLE_WINDOW && \
	   client->lastDrawable->type != DRAWABLE_PIXMAP)\
	   {\
	   client->lastDrawable = NULL;\
	   client->lastDrawableID = -1;\
	   }
#else /* X11R4 || DWV3 */
#define LookupDrawable_(client,drawID)\
	if(client->lastDrawableID != drawID)\
	    {\
	    client->lastDrawable = (DrawablePtr)LookupDrawable(drawID,client);\
	    client->lastDrawableID = drawID;\
	    }\
	if(client->lastDrawable == NULL)\
	   client->lastDrawableID = -1
#endif

    /*
    **	This macro looks up a GC.
    */
#if   defined(X11R3) && !defined(DWV3)
#define LookupGC_(client,GCID)\
        if(client->lastGCID != GCID)\
	    {\
            client->lastGC = (GC *)LookupID(GCID, RT_GC, RC_CORE);\
            client->lastGCID = GCID;\
	    }\
	if(client->lastGC == 0)\
	    client->lastGCID = -1
#else /* X11R4 || DWV3 */
#define LookupGC_(client,GCID)\
        if(client->lastGCID != GCID)\
	    {\
            client->lastGC = (GC *)LookupIDByType(GCID, RT_GC);\
            client->lastGCID = GCID;\
	    }\
	if(client->lastGC == 0)\
	    client->lastGCID = -1
#endif
    /*
    **  This macro helps with the book keeping when we are using scratch GCs.
    */
#define UsingScratchGC_(client,gcptr) \
	client->lastGC = gcptr; \
	client->lastGCID = -1

    /*
    **	This macro validates the most recently used Drawable and GC.
    */
#define ValidateDrawableAndGC_(client)\
    if(client->lastGC->serialNumber != client->lastDrawable->serialNumber)\
	ValidateGC(client->lastDrawable, client->lastGC)

    /*
    **	This macro makes sure that the drawable and GC are compatible.
    **
    **	Error exit - BadDrawable, BadGC, BadMatch
    */
#define LookupDrawableAndGC_(client,drawID,GCID)\
	LookupDrawable_(client,drawID);\
	DrawableOkIf_(client->lastDrawable,drawID);\
	LookupGC_(client,GCID);\
	GCOkIf_(client->lastGC,GCID);\
        if( client->lastDrawable->type == UNDRAWABLE_WINDOW \
	 || client->lastGC->depth      != client->lastDrawable->depth \
	 || client->lastGC->pScreen    != client->lastDrawable->pScreen)\
	    {\
	    client->lastGC = NULL;\
	    client->lastGCID = -1;\
	    BadMatch_(GCID);\
	    }\
	ValidateDrawableAndGC_(client)

    /*
    **	This macro calls the LookupTmp routine and tests the result returned.
    **
    **	Error exit - result of LookupTmp if result not "Success"
    */
#define LookupTmp_(pres, pudp,idc_id)\
	{int status = LookupTmp(client,(pres),(pudp),(idc_id));\
	 if( status ) return status;}

    /*
    **	These macros return the coordinates and dimensions of a drawable.
    */
#ifdef   X11R3
#define DrawableX_(pDraw) \
	(pDraw->type!=DRAWABLE_PIXMAP ? ((WindowPtr)pDraw)->clientWinSize.x : 0)
#define DrawableY_(pDraw) \
	(pDraw->type!=DRAWABLE_PIXMAP ? ((WindowPtr)pDraw)->clientWinSize.y : 0)
#ifdef	VMS
#define DrawableWidth_(pDraw) \
	(pDraw->type!=DRAWABLE_PIXMAP \
	? ((WindowPtr)pDraw)->clientWinSize.width \
	: ((PixmapPtr)pDraw)->mem_header->desc->width)
#define DrawableHeight_(pDraw) \
	(pDraw->type!=DRAWABLE_PIXMAP \
	? ((WindowPtr)pDraw)->clientWinSize.height \
	: ((PixmapPtr)pDraw)->mem_header->desc->height)
#else	/* NOT VMS */
#define DrawableWidth_(pDraw) \
	(pDraw->type!=DRAWABLE_PIXMAP \
	? ((WindowPtr)pDraw)->clientWinSize.width \
	: ((PixmapPtr)pDraw)->width)
#define DrawableHeight_(pDraw) \
	(pDraw->type!=DRAWABLE_PIXMAP \
	? ((WindowPtr)pDraw)->clientWinSize.height \
	: ((PixmapPtr)pDraw)->height)
#endif
#else /* X11R4 */
#define DrawableX_(pDraw)	(((DrawablePtr)pDraw)->x)
#define DrawableY_(pDraw)	(((DrawablePtr)pDraw)->x)
#define DrawableWidth_(pDraw)	(((DrawablePtr)pDraw)->width)
#define DrawableHeight_(pDraw)	(((DrawablePtr)pDraw)->height)
#endif

    /*
    **	This macro calls core PutImage
    */
#if defined(X11R3) && !defined(DWV3)
#define PutImage_(draw,gc,depth,x,y,w,h,pad,format,adr) \
    (*gc->PutImage)(draw,gc,depth,x,y,w,h,pad,format,adr) 
#else /* X11R4 || DWV3 */
#define PutImage_(draw,gc,depth,x,y,w,h,pad,format,adr) \
    (*gc->ops->PutImage)(draw,gc,depth,x,y,w,h,pad,format,adr) 
#define FillSpans_(draw,gc,count,points,widths) \
    (*gc->ops->FillSpans)(draw, gc, count, points, widths, 1)
#endif

/* Macros to support swapped byte clients */
#define WriteXieReplyToClient(pClient, size, pReply) \
   if ((pClient)->swapped) \
      CallProc_(xieDixSwapReply)(pClient, (int)(size), pReply); \
      else (void) WriteToClient(pClient, (int)(size), (char *)(pReply));

#define WriteSwappedXieDataToClient(pClient, size, pbuf) \
   if ((pClient)->swapped) \
      (*(pClient)->pSwapReplyFunc)(pClient, (int)(size), pbuf); \
   else (void) WriteToClient (pClient, (int)(size), (char *)(pbuf));

#define SetSwappedDataRtn(pClient, typ) \
    pClient->pSwapReplyFunc = xieDixSwapData[sizeof(typ)];

/* Macro to byte-swap the necessary fields of an XieFloat record */
#define swapfloat(fp, n) \
{ \
      swaps(fp.exponent, n); \
      swapl(fp.mantissa, n); \
}
