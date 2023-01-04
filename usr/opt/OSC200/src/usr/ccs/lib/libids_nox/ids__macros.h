
/***************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
***************************************************************************/

/*******************************************************************************
**
**  FACILITY:
**
**      Image Display Services (IDS)
**
**  ABSTRACT:
**
**      This is a C include file containing MACRO definitions used by IDS
**	routines.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**
**  AUTHOR(S):
**
**      Subu Garikapati
**      Robert NC Shelley
**
**  CREATION DATE:
**
**      January 7, 1988
**
**  MODIFICATION HISTORY:
**
**	See CMS generated modification history
**
*******************************************************************************/

/*
**  Functions which are used in the macros must be forward declared
*/
extern unsigned long _ImgGet();

/*
**  MACROS which make interfacing to ISL friendlier for IDS ...
*/
    /*
    **  Deallocate memory via _ImgCfree, pointer passed by ref.
    */
#define MemFree_(mem) {if(*mem != NULL) _ImgCfree(*mem); *mem = NULL;}

    /*
    **  Simplify calling _ImgGet(...)
    */
#define GetIsl_(fid,item,buffer,index) \
	    _ImgGet((fid),(long)(item),&(buffer),(long)(sizeof(buffer)),(long)0,(long)(index))

    /*
    **  Create a static ISL get-item-list ("buffer" must also be static)
    **  These are obselete in V3 version of ISL
    */
#define BeginGetIsl_(name) static struct GET_ITMLST name[] = {
#define GetIslEntry_(item,buffer,index) \
            {(unsigned long int) item, (unsigned long int) sizeof(buffer),\
             (char *) &buffer, (unsigned long int *) 0,\
             (unsigned long int) index},
#define EndGetIsl_  \
	    {(unsigned long int)0, (unsigned long int)0, (char *)NULL,\
             (unsigned long int *)0, (unsigned long int)0 }};

    /*
    **  Create a static ISL put-item-list ("buffer" must also be static)
    */
#define BeginPutIsl_(name) static struct PUT_ITMLST name[] = {
#define PutIslEntry_(item,buffer,index) \
	    {(unsigned long int) item, (unsigned long int) sizeof(buffer),\
	     (char *) &buffer, (unsigned long int) index},
#define EndPutIsl_ \
	    {(unsigned long int)0, (unsigned long int)0, (char *)NULL,\
	     (unsigned long int)0 }};

#define INIT_ITM_(struct,ctr,code,length,buffer,retlen,index) \
    struct[ctr].ItmL_Code   = (unsigned long)    code;\
    struct[ctr].ItmL_Length = (unsigned long)    length;\
    struct[ctr].ItmA_Buffer = (char *)         (buffer);\
    struct[ctr].ItmA_Retlen = (unsigned long *) (retlen);\
    struct[ctr].ItmL_Index  = (unsigned long)    index;\
    ctr++

#define END_ITEM_(struct,ctr)\
    struct[ctr].ItmL_Code = 0;\
    struct[ctr].ItmL_Length = 0;\
    struct[ctr].ItmA_Buffer = 0;\
    struct[ctr].ItmA_Retlen = 0;\
    struct[ctr].ItmL_Index = 0;\
    ctr++

    /*
    **  Fetch the number of spectral components in an image and then
    **	fetch the number of Bits_per_Component for each component.
    */
#define GetBitsPerComponent_(fid,cnt,buffer) \
	    {int _i; GetIsl_(fid, Img_NumberOfComp, cnt, 0); \
	     for(_i=0; _i < cnt; ++_i) \
	        GetIsl_(fid,Img_ImgBitsPerComp,buffer[_i],_i); \
	     while(_i < Ids_MaxComponents) buffer[_i++]=0; }

/*
**  MACROS which make maintaining IDS friendlier for the engineer ...
*/
    /*
    **  Calculate the minimum number of bits per spectral component 
    **	needed, based on the number of levels for that component.
    */
#define SetBitsPerComponent_(bits,levels) \
            {unsigned long _i; if(levels < 2) bits = 1; else { \
	    for(bits=0, _i=(unsigned long)levels; (_i >>= 1) != 0; ++bits);\
	    if((1<<bits)-1 & levels) ++bits; }}

#define BitsFromLevels_(levels) ((int)(ceil(log10((double)levels)/log10(2.0))))
    /*
    **  Return the integer amount that must be added to "len" so that
    **	it would be an exact multiple of pad.
    */
#define AlignBits_(len,pad) (((pad) - (len) % (pad)) % (pad))

    /*
    **  Return "sizeof()" as bits rather than bytes.
    */
#define BitsOf_(b) (sizeof(b)<<3)

    /*
    **  Return a float or double, as an int, rounded to the nearest integer.
    */
#define Round_(value) ((int)((value)+0.5))

    /*
    **	If "condition" is satisfied, Replace "this" with "that".
    **
    **  Return TRUE if replacement is made, otherwise return FALSE.
    */
#define IfReplace_(condition,   this,  that) \
		 ((condition)?((this)=(that),TRUE):FALSE)

    /*
    **	If "condition" is satisfied, Add "action" as pipe element.
    */
#define AddPipeElement_(condition,   this,  that) \
		 ((condition)?((this)=(that),TRUE):FALSE)


    /*
    **	Byte swapping.
    */
#define SwapFourBytes_(base,bytes) \
    {char c, *b, *e; for(b=base, e=b+bytes; b<e; b+=4) \
    {c= *b; *b= *(b+3); *(b+3)=c; c= *(b+1); *(b+1)= *(b+2); *(b+2)=c;}}
#define SwapThreeBytes_(base,bytes) \
    {char c, *b, *e; for(b=base, e=b+bytes; b<e; b+=3) \
    {c= *b; *b= *(b+2); *(b+2)=c;}}
#define SwapTwoBytes_(base,bytes) \
    {char c, *b, *e; for(b=base, e=b+bytes; b<e; b+=2) \
    {c= *b; *b= *(b+1); *(b+1)=c;}}

/*
**  For non-VMS architectures,
**  don't access longwords on non-longword aligned boundaries.
*/
#ifdef VMS
#define READ32(address) \
    (*((int *)(address)))

#define WRITE32(address,value) \
    (*((int *)(address)) = (int)value)
#else


#define READ32(address) \
(unsigned int)(*((unsigned char *)(address)) + \
   ((unsigned int)(*(((unsigned char *)(address))+1)) << 8) + \
   ((unsigned int)(*(((unsigned char *)(address))+2)) << 16) + \
   ((unsigned int)(*(((unsigned char *)(address))+3)) << 24))

#define WRITE32(address,value) {\
   *((unsigned char *)(address)) = ((int)value) & 0xff; \
   *(((unsigned char *)(address))+1) = (((int)value) >> 8) & 0xff; \
   *(((unsigned char *)(address))+2) = (((int)value) >> 16) & 0xff; \
   *(((unsigned char *)(address))+3) = (((int)value) >> 24) & 0xff; }
#endif

    /*
    **  Extract a bit field from an unaligned bit stream.
    */
#define GetField_(base,offset,mask) \
          (READ32(base+(offset >> 3)) >> (offset & 0x7) & mask)

    /*
    **  XOR a field of bits into an unaligned bit stream.
    */
#define PutField_(base,offset,mask,value) \
          WRITE32(base+(offset >> 3),READ32(base+(offset >> 3)) ^ \
     (((value) & mask) << (offset & 0x7)))

    /*
    **  convenience macros for accessing Core part
    */
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define Colormap_(w)    ((w)->core.colormap)
#define BackPix_(w)     ((w)->core.background_pixel)
#define Depth_(w)       ((w)->core.depth)
#define Parent_(w)      ((w)->core.parent)
  
/*
**  MACROS which make interfacing to X11 friendlier for IDS ...
*/
    /*
    **  Copy image data from RPhoto_(w) to XtWindow(w).
    **  Return TRUE if copy is performed (ie. photmap exists), otherwise FALSE
#define PhotomapToWindow_(w,px,py,wide,high,wx,wy) \
    ((XieContext_(w)->disp_photo == NULL) ? FALSE \
        : ( XieExport(RPhoto_(w),XtWindow(w),GC_(w),\
                         (px),(py),(wx),(wy),(wide),(high)),      TRUE))
    */
   
    /*
    **	Copy image data from Pix_(w) to XtWindow(w).  If the image is
    **  continuous tone use XCopyArea, otherwise use XCopyPlane.
    **
    **  Return TRUE if copy is performed (ie. pixmap exists), otherwise FALSE.
    */
#define PixmapToWindow_(w,px,py,wide,high,wx,wy) \
    ((Pix_(w) == NULL) ? FALSE \
	: Ifrmt_(w) == ZPixmap ?\
	  ( XCopyArea(XtDisplay(w),Pix_(w),XtWindow(w),GC_(w),\
			 (px),(py),(wide),(high),(wx),(wy)),      TRUE)\
	: (XCopyPlane(XtDisplay(w),Pix_(w),XtWindow(w),GC_(w),\
			 (px),(py),(wide),(high),(wx),(wy),PLANE),TRUE))

#define PixmapToXieWindow_(w,px,py,wide,high,wx,wy) \
    ((Pix_(w) == NULL) ? FALSE \
        : (Ideep_(w) > 1) ?\
          ( XCopyArea(XtDisplay(w),Pix_(w),XtWindow(w),GC_(w),\
                         (px),(py),(wide),(high),(wx),(wy)),      TRUE)\
        : (XCopyPlane(XtDisplay(w),Pix_(w),XtWindow(w),GC_(w),\
                         (px),(py),(wide),(high),(wx),(wy),PLANE),TRUE))
  
    /*
    **	Copy image data from Img_(w) to XtWindow(w).
    **
    **  Return TRUE if copy is performed (ie. it exists), otherwise FALSE.
    */
#define XImageToWindow_(w,ix,iy,wx,wy,wide,high) \
    ((Idata_(w) == NULL) ? FALSE \
	: ( XPutImage(XtDisplay(w),XtWindow(w),GC_(w),&Img_(w),\
			(ix),(iy),(wx),(wy),(wide),(high)), TRUE))


    /*
    **  Pass a variable number of arguments to the IdsInsertPipe routine.
    **
    **	The "cid" argument is an identifier that the user callahead routine
    **	(established in pd->base->callahead) can use to distinguish this 
    **	pipe element function.  The routine is called only if it exists AND
    **	"cid" is non-zero.
    **
    **	If the "dst" argument is the pipe token IdsTemp, then the temporary 
    **	storage address assigned for the value returned by function "func" is 
    **	returned in the IdsPipeDescStruct element 'tmp' (ie. pd->tmp).
    **
    **	If "func" requires arguments, "args" MUST begin with a left parenthesis,
    **	followed by any number of ArgByPtr_(), ArgByVal_(), and/or ArgByRef_()
    **	macros, followed by an EndArgs_ macro and a closing right parenthesis.
    **	If no arguments are required, just an EndArgs_ macro is required.
    */
#define InsertPipe_(pd,cid,dst,func,args) \
    {long *_p0 = (long *) (pd->pipe->arglst + *pd->pipe->arglst+1); \
     long *_p1 = _p0; /* _p0 and _p1 are used by the ArgBy...() macros */ \
     _p1 = (long *) IdsInsertPipe(pd,pd->pipe+1,pd->base->callahead,(cid), \
				    pd->base->call_data,dst,func, \
	(unsigned long *)args); \
    if((unsigned long *)dst == IdsTemp) pd->tmp = (unsigned long *)_p1;}

    /*
    **	Use ArgByPtr_(arg) to push a pointer into the pipe argument list.
    **	Upon pipe execution the contents of pointer (ie. *pointer) will be 
    **	passed to the piped function.  ArgByPtr_(arg) should be used if the
    **	data to be passed to the piped function is dynamic (eg. generated
    **	within the pipe line).  Use the symbol IdsNoPtr to pass a NULL pointer
    **	    (eg. value returned by the conditional operator: "? :").
    */
#define ArgByPtr_(arg)  (*++_p1 = (long)(arg))

    /*
    **	Use ArgByVal_(arg) to push static data into the pipe argument list.
    **	Upon pipe execution the stored value is passed to the piped function.
    */
#define ArgByVal_(arg)  (*++_p1 = (long)(IdsByVal), *++_p1 = (long)(arg))

    /*
    **	Use ArgByRef_(arg) to push the address of arg into the pipe argument 
    **	list.  Upon pipe execution the stored address of arg will be passed 
    **	to the piped function.   ArgByRef_() is exists only for ascetic 
    **	completeness,  i.e.  ArgByRef_(foo) is equivalent to ArgByVal_(&foo).
    */
#define ArgByRef_(arg)  (*++_p1 = (long)(IdsByVal), *++_p1 = (long)(&arg))

    /*
    **	The variable length argument list MUST be terminated by EndArgs_
    **	(and followed by a right parenthesis if any arguments were supplied).
    **	Remember: a leading left parenthesis preceeding the list of arguments
    **	is also MANDATORY.
    */
#define EndArgs_        (*_p0 = _p1-_p0, (long)_p0)


    /*
    **  convenience macros for accessing RENDER part.
    */
#define Img_(w)		(IdsPart_(w)->render.ximage)
#define	    Iwide_(w)	    (IdsPart_(w)->render.ximage.width)
#define	    Ihigh_(w)	    (IdsPart_(w)->render.ximage.height)
#define	    Ioffs_(w)	    (IdsPart_(w)->render.ximage.xoffset)
#define	    Ifrmt_(w)	    (IdsPart_(w)->render.ximage.format)
#define	    Idata_(w)	    (IdsPart_(w)->render.ximage.data)
#define	    Ideep_(w)	    (IdsPart_(w)->render.ximage.depth)
#define	    Ibpln_(w)	    (IdsPart_(w)->render.ximage.bytes_per_line)
#define	    Ibppx_(w)	    (IdsPart_(w)->render.ximage.bits_per_pixel)
#define Pix_(w)		(IdsPart_(w)->render.pixmap)
#define StdGC_(w)	(IdsPart_(w)->render.zero_max_gc)
#define RevGC_(w)	(IdsPart_(w)->render.zero_min_gc)
#define GC_(w)		(IdsPart_(w)->render.image_gc)
#define	RFid_(w)	(IdsPart_(w)->render.rendered_fid)
#define	RPhoto_(w)	(IdsPart_(w)->render.rendered_pho)
#define WorkCB_(w)	(IdsPart_(w)->render.work_notify_callback)
#define ErrorCB_(w)	(IdsPart_(w)->render.error_callback)
#define RenderCB_(w)	(IdsPart_(w)->render.render_callback)
#define XieListCB_(w)	(IdsPart_(w)->render.xielist_callback)
#define SaveCB_(w)      (IdsPart_(w)->render.save_callback)
#define Context_(w)	(IdsPart_(w)->render.context)
#define XieContext_(w)	(IdsPart_(w)->render.xiecontext)
#define Current_(w)	(IdsPart_(w)->render.current)
#define	    cRender_(w)	    (IdsPart_(w)->render.current.render_mode)
#define	    cScheme_(w)	    (IdsPart_(w)->render.current.render_scheme)
#define	    cProto_(w)	    (IdsPart_(w)->render.current.protocol)
#define	    cFid_(w)	    (IdsPart_(w)->render.current.fid)
#define	    cXieImg_(w)	    (IdsPart_(w)->render.current.xieimage)
#define	    cROI_(w)	    (IdsPart_(w)->render.current.roi)
#define	    cRAng_(w)	    (IdsPart_(w)->render.current.angle)
#define	    cFlip_(w)	    (IdsPart_(w)->render.current.flip_options)
#define	    cXSc_(w)	    (IdsPart_(w)->render.current.x_scale)
#define	    cYSc_(w)	    (IdsPart_(w)->render.current.y_scale)
#define Proposed_(w)	(IdsPart_(w)->render.proposed)
#define	    pReason_(w)	    (IdsPart_(w)->render.proposed.reason)
#define	    pEvent_(w)	    (IdsPart_(w)->render.proposed.event)
#define	    pRender_(w)	    (IdsPart_(w)->render.proposed.render_mode)
#define	    pScheme_(w)	    (IdsPart_(w)->render.proposed.render_scheme)
#define	    pProto_(w)	    (IdsPart_(w)->render.proposed.protocol)
#define	    pFid_(w)	    (IdsPart_(w)->render.proposed.fid)
#define	    pXieImg_(w)	    (IdsPart_(w)->render.proposed.xieimage)
#define	    pROI_(w)	    (IdsPart_(w)->render.proposed.roi)
#define	    pRMode_(w)	    (IdsPart_(w)->render.proposed.rotate_mode)
#define	    pROpts_(w)	    (IdsPart_(w)->render.proposed.rotate_options)
#define	    pRAng_(w)	    (IdsPart_(w)->render.proposed.angle)
#define	    pFlip_(w)	    (IdsPart_(w)->render.proposed.flip_options)
#define	    pSMode_(w)	    (IdsPart_(w)->render.proposed.scale_mode)
#define	    pSOpts_(w)	    (IdsPart_(w)->render.proposed.scale_options)
#define	    pXSc_(w)	    (IdsPart_(w)->render.proposed.x_scale)
#define	    pYSc_(w)	    (IdsPart_(w)->render.proposed.y_scale)
#define	    pXres_(w)	    (IdsPart_(w)->render.proposed.x_pels_per_bmu)
#define	    pYres_(w)	    (IdsPart_(w)->render.proposed.y_pels_per_bmu)
#define	    pPun1_(rcb)	    (IdsPart_(w)->render.proposed.punch1)
#define	    pPun2_(rcb)	    (IdsPart_(w)->render.proposed.punch2)
#define	    pSharp_(rcb)    (IdsPart_(w)->render.proposed.sharpen)
#define	    pAlgor_(w)	    (IdsPart_(w)->render.proposed.dither_algorithm)
#define	    pThresh_(w)	    (IdsPart_(w)->render.proposed.dither_threshold)
#define	    pGRA_(w)	    (IdsPart_(w)->render.proposed.levels_gray)
#define	    pRGB_(w)	    (IdsPart_(w)->render.proposed.levels_rgb)
#define	    pLevs_(w)	    (IdsPart_(w)->render.proposed.fit_levels)
#define	    pWide_(w)	    (IdsPart_(w)->render.proposed.fit_width)
#define	    pHigh_(w)	    (IdsPart_(w)->render.proposed.fit_height)
#define	    pCompute_(w)    (IdsPart_(w)->render.proposed.compute_mode)
#define	    CopyFid_(w)	    (IdsPart_(w)->render.copy_fid)
#define	    ForceCopyFid_(w) (IdsPart_(w)->render.force_copy_fid)
#define	    ECwidget_(cb)   ((cb)->render_widget)
#define	    ECXieErr_(cb)   ((cb)->XIE_error)
#define     ECcall_(cb)     ((cb)->error_proc)
    /*
    **  convenience macros for accessing IMAGE part
    */
#define Visibility_(w)	(IdsPart_(w)->image.visibility)
#define SrcW_(w)	(IdsPart_(w)->image.source_width)
#define SrcH_(w)	(IdsPart_(w)->image.source_height)
#define SGrav_(w)	(IdsPart_(w)->image.source_gravity)
#define WGrav_(w)	(IdsPart_(w)->image.window_gravity)
#define WrkW_(w)	(IdsPart_(w)->image.work_width)
#define WrkH_(w)	(IdsPart_(w)->image.work_height)
#define cCoords_(w)	(IdsPart_(w)->image.current_coords)
#define	    cSrcX_(w)	    (IdsPart_(w)->image.current_coords.source_x)
#define	    cSrcY_(w)	    (IdsPart_(w)->image.current_coords.source_y)
#define	    cWrkX_(w)	    (IdsPart_(w)->image.current_coords.work_x)
#define	    cWrkY_(w)	    (IdsPart_(w)->image.current_coords.work_y)
#define rCoords_(w)	(IdsPart_(w)->image.request_coords)
#define	    rSrcX_(w)	    (IdsPart_(w)->image.request_coords.source_x)
#define	    rSrcY_(w)	    (IdsPart_(w)->image.request_coords.source_y)
#define	    rWrkX_(w)	    (IdsPart_(w)->image.request_coords.work_x)
#define	    rWrkY_(w)	    (IdsPart_(w)->image.request_coords.work_y)
#define ViewCB_(w)	(IdsPart_(w)->image.view_callback)
#define ExpCB_(w)	(IdsPart_(w)->image.expose_callback)
#define DragCB_(w)	(IdsPart_(w)->image.drag_callback)
#define ZoomCB_(w)	(IdsPart_(w)->image.zoom_callback)
#define SPix_(w)	(IdsPart_(w)->image.scroll_box_pix)
#define SBox_(w)	(IdsPart_(w)->image.scroll_box)
#define Hbar_(w)	(IdsPart_(w)->image.horizontal_bar)
#define Vbar_(w)	(IdsPart_(w)->image.vertical_bar)
#define Incr_(w)	(IdsPart_(w)->image.increment_bar)
#define EnableH_(w)	(IdsPart_(w)->image.enable_horizontal)
#define EnableV_(w)	(IdsPart_(w)->image.enable_vertical)
#define DynBar_(w)	(IdsPart_(w)->image.enable_dynamic)
#define ActBar_(w)	(IdsPart_(w)->image.scroll_active)
#define DoBars_(w)	(IdsPart_(w)->image.do_bar_callbacks)
#define Redraw_(w)	(IdsPart_(w)->image.redraw)
    /*
    **  convenience macros for accessing PANNED part
    */
#define BegX_(w)	(IdsPart_(w)->panned.begin_x)
#define BegY_(w)	(IdsPart_(w)->panned.begin_y)
#define PreviousX_(w)   (IdsPart_(w)->panned.previous_x)
#define PreviousY_(w)   (IdsPart_(w)->panned.previous_y)
#define PanX_(w)	(IdsPart_(w)->panned.pan_x)
#define PanY_(w)	(IdsPart_(w)->panned.pan_y)
#define Panning_(w)	(IdsPart_(w)->panned.panning)
#define Drag_(w)	(IdsPart_(w)->panned.drag)
    /*
    **  convenience macros for accessing ZOOMED part
    */
#define ZstartX_(w)	(IdsPart_(w)->panned.start_x)
#define ZstartY_(w)	(IdsPart_(w)->panned.start_y)
#define Zwidth_(w)	(IdsPart_(w)->panned.width)
#define Zheight_(w)	(IdsPart_(w)->panned.height)
#define Zooming_(w)	(IdsPart_(w)->panned.zooming)
#define ZGC_(w)		(IdsPart_(w)->panned.gc)
    /*
    ** Convenience macros for indirect access to IdsRenderCallback components.
    */
#define iReason_(rcb)	((rcb)->reason)
#define	iEvent_(rcb)	((rcb)->event)
#define	iCompute_(rcb)	((rcb)->compute_mode)
#define	intCompute_(rcb) ((rcb)->internal_compute_mode)
#define	iRender_(rcb)	((rcb)->render_mode)
#define	iScheme_(rcb)	((rcb)->render_scheme)
#define	iProto_(rcb)	((rcb)->protocol)
#define	iXieimage_(rcb)	((rcb)->xieimage)
#define	iFid_(rcb)	((rcb)->fid)
#define	iROI_(rcb)	((rcb)->roi)
#define	iRMode_(rcb)	((rcb)->rotate_mode)
#define	iROpts_(rcb)	((rcb)->rotate_options)
#define	iRAng_(rcb)	((rcb)->angle)
#define iRWid_(rcb)     ((rcb)->rotate_width)
#define iRHei_(rcb)     ((rcb)->rotate_height)
#define iRFil_(rcb)     ((rcb)->rotate_fill)
#define	iFlip_(rcb)	((rcb)->flip_options)
#define	iSMode_(rcb)	((rcb)->scale_mode)
#define	iSOpts_(rcb)	((rcb)->scale_options)
#define	iXSc_(rcb)	((rcb)->x_scale)
#define	iYSc_(rcb)	((rcb)->y_scale)
#define	iZXSc_(rcb)	((rcb)->zoom_x_scale)
#define	iZYSc_(rcb)	((rcb)->zoom_y_scale)
#define	iXres_(rcb)	((rcb)->x_pels_per_bmu)
#define	iYres_(rcb)	((rcb)->y_pels_per_bmu)
#define	iPun1_(rcb)	((rcb)->punch1)
#define	iPun2_(rcb)	((rcb)->punch2)
#define	iSharp_(rcb) 	((rcb)->sharpen)
#define	iAlgor_(rcb)	((rcb)->dither_algorithm)
#define	iThrsh_(rcb)	((rcb)->dither_threshold)
#define	iGRA_(rcb)	((rcb)->levels_gray)
#define	iRGB_(rcb)	((rcb)->levels_rgb)
#define	iLevs_(rcb)	((rcb)->fit_levels)
#define	iWide_(rcb)	((rcb)->fit_width)
#define	iHigh_(rcb)	((rcb)->fit_height)
    /*
    **  Convenience macros for access to RenderContext components.
    */
#define Device_(ctx)	((ctx)->device_type)
#define Pipe_(ctx)	((ctx)->pipe_desc)
#define Error_(ctx)     ((ctx)->IdsErrorCb)
#define PropRnd_(ctx)	((ctx)->proposed_rcb)
#define PxlAlloc_(ctx)	((ctx)->pixel_alloc)
#define PxlLst_(ctx)	((ctx)->pixel_index_list)
#define PxlCnt_(ctx)	((ctx)->pixel_index_count)
#define PltLst_(ctx)	((ctx)->palette_index_list)
#define PltCnt_(ctx)	((ctx)->palette_index_count)
#define CmpMode_(ctx)	((ctx)->colormap_mode)
#define ClrMap_(ctx)	((ctx)->appl_color_list)
#define CmpUpd_(ctx)	((ctx)->colormap_update)
#define PxlDat_(ctx)	((ctx)->pixel_match_data)
#define	CSpace_(ctx)	((ctx)->color_space)
#define	MchLim_(ctx)	((ctx)->match_limit)
#define	GraLim_(ctx)	((ctx)->gray_limit)
#define Image_(ctx)	((ctx)->ximage)
#define RqLst_(ctx)	((ctx)->requant_itmlst)
#define TsLst_(ctx)	((ctx)->t_scale_itmlst)
#define DiLst_(ctx)	((ctx)->dither_itmlst)
#define RqLev_(ctx)	((ctx)->requant_levels)
#define TsLev_(ctx)	((ctx)->t_scale_levels)
#define GRA_(ctx)	((ctx)->levels_gray)
#define	RGB_(ctx)	((ctx)->levels_rgb)
#define FitL_(ctx)	((ctx)->fit_levels)
#define	FitW_(ctx)	((ctx)->fit_width)
#define	FitH_(ctx)	((ctx)->fit_height)
#define Proto_(ctx)	((ctx)->protocol)
#define RClass_(ctx)	((ctx)->rendering_class)
#define Dpy_(ctx)	((ctx)->display)
#define Scr_(ctx)	((ctx)->screen)
#define Vis_(ctx)	((ctx)->visual)
#define Win_(ctx)	((ctx)->window)
#define WinW_(ctx)	((ctx)->width)
#define WinH_(ctx)	((ctx)->height)
#define WinD_(ctx)	((ctx)->depth)
#define Cmap_(ctx)	((ctx)->colormap)
#define Cells_(ctx)	((ctx)->colormap_size)
#define Pad_(ctx)	((ctx)->scanline_modulo)
#define PxlStr_(ctx)	((ctx)->Z_bits_per_pixel)
#define PsFlags_(ctx)	((ctx)->ps_flags)
    /*
    **  Convenience macros for access to RenderContextXie components.
    */
#define Process_(ctx)	((ctx)->xie_ext)
#define Save_(ctx)	((ctx)->save_mode)
#define Cmpress_(ctx)	((ctx)->cmpres_mode)
#define Cmporg_(ctx)	((ctx)->cmporg_mode)
#define Switch_(ctx)	((ctx)->switch_mode)
#define XieFunc_(ctx)	((ctx)->xie_functions)
#define Photo_(ctx)	((ctx)->photoflo)
#define TPhoto_(ctx)	((ctx)->raw_photo)
#define SPhoto_(ctx)	((ctx)->save_photo)
#define DPhoto_(ctx)	((ctx)->disp_photo)
#define LPhoto_(ctx)	((ctx)->lut_photo)
#define RRend_(ctx)	((ctx)->rerender)
#define PriXie_(ctx)	((ctx)->xiedat)
#define PipeDone_(ctx)  ((ctx)->pipedone)
#define XieWidget_(ctx)  ((ctx)->render_widget)
#define Crop_(ctx)	((ctx)->crop_roi)
#define Fill_(ctx)	((ctx)->fill_roi)
#define Tsrc_(ctx)	((ctx)->tsrc_roi)
#define Tdst_(ctx)	((ctx)->tdst_roi)
#define Point_(ctx)	((ctx)->point_roi)
#define Logic_(ctx)	((ctx)->logic_roi)
#define Math_(ctx)	((ctx)->math_roi)
#define Rxmir_(ctx)	((ctx)->x_mirror)
#define Rymir_(ctx)	((ctx)->y_mirror)
    /*
    **  Convenience macros for photomap data
    */
#define RenW_(ctx)	((ctx)->rend_width)
#define RenH_(ctx)	((ctx)->rend_height)
#define RenD_(ctx)	((ctx)->rend_depth)
#define Rcmp_(ctx)	((ctx)->comp_map)
#define Rcnt_(ctx)	((ctx)->comp_cnt)
#define Rlevs_(ctx)	((ctx)->comp_levs)
#define RPol_(ctx)	((ctx)->pixel_pol)
#define RAsp_(ctx)	((ctx)->pixel_ratio)
#define RPixGC_(ctx)	((ctx)->pixmapGC)
    /*
    **  Convenience macros for access to DataForXie struct components. C -- copy
    */
#define CPix_(xiedat)	        ((xiedat)->pixmap)
#define CStdGC_(xiedat)	        ((xiedat)->zero_max_gc)
#define CForePix_(xiedat)	((xiedat)->foreground_pixel)
#define CBackPix_(xiedat)	((xiedat)->background_pixel)
#define CRevGC_(xiedat)	        ((xiedat)->zero_min_gc)
#define CGC_(xiedat)	        ((xiedat)->image_gc)
#define Cllx_(xiedat)	        ((xiedat)->ll_x)
#define Clly_(xiedat)	        ((xiedat)->ll_y)
#define Curx_(xiedat)	        ((xiedat)->ur_x)
#define Cury_(xiedat)	        ((xiedat)->ur_y)
#define Croix_(xiedat)	        ((xiedat)->roi_x)
#define Croiy_(xiedat)	        ((xiedat)->roi_y)
#define Croiw_(xiedat)	        ((xiedat)->roi_w)
#define Croih_(xiedat)	        ((xiedat)->roi_h)

#define Iroix_(xiedat)	        ((xiedat)->internal_roi_x)
#define Iroiy_(xiedat)	        ((xiedat)->internal_roi_y)
#define Iroiw_(xiedat)	        ((xiedat)->internal_roi_w)
#define Iroih_(xiedat)	        ((xiedat)->internal_roi_h)

#define Iwx_(xiedat)	        ((xiedat)->internal_w_x)
#define Iwy_(xiedat)	        ((xiedat)->internal_w_y)
#define Iww_(xiedat)	        ((xiedat)->internal_w_width)
#define Iwh_(xiedat)	        ((xiedat)->internal_w_height)

#define IULx_(xiedat)	        ((xiedat)->internal_ul_x)
#define IULy_(xiedat)	        ((xiedat)->internal_ul_y)

#define IrW_(xiedat)	        ((xiedat)->rotate_width)
#define IrH_(xiedat)	        ((xiedat)->rotate_height)

#define Cppd_(xiedat)           ((xiedat)->pp_dist)
#define Clpd_(xiedat)           ((xiedat)->lp_dist)
 
    /*
    **  Convenience macros for access to hardcopy IdsRendering components.
    */
#define Hsfid_(rendering)	((rendering)->srcfid)
#define Hpsid_(rendering)	((rendering)->psid)
#define Htype_(rendering)	((rendering)->type)
#define Hrcb_(rendering)	((rendering)->rcb)
#define Hrfid_(rendering)	((rendering)->type_spec_data.fid)
#define Hximage_(rendering)	((rendering)->type_spec_data.xlib.ximage)
#define Hpixmap_(rendering)	((rendering)->type_spec_data.xlib.pixmap)
#define HforePix_(rendering)    ((rendering)->type_spec_data.xlib.foreground_pixel)
#define HbackPix_(rendering)    ((rendering)->type_spec_data.xlib.background_pixel)
#define HpxlLst_(rendering)	((rendering)->type_spec_data.xlib.pixel_index_list)
#define HpxlCnt_(rendering)	((rendering)->type_spec_data.xlib.pixel_index_count)
#define HGC_(rendering)         ((rendering)->type_spec_data.xlib.image_gc)
#define HsixCnt_(rendering)     ((rendering)->type_spec_data.sixel.bytcnt)
#define HsixBuf_(rendering)     ((rendering)->type_spec_data.sixel.bufptr)
#define HposCnt_(rendering)     ((rendering)->type_spec_data.postscript.bytcnt)
#define HposBuf_(rendering)     ((rendering)->type_spec_data.postscript.bufptr)
#define Hrnfid_(rendering)	((rendering)->rndfid)
