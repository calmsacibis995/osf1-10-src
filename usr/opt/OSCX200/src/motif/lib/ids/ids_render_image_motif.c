
/****************************************************************************
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

/*****************************************************************************
**
**  FACILITY:
**
**      Image Display Services (IDS)
**
**  ABSTRACT:
**
**      This module implements the IDS widget for rendering ISL image frames 
**	to be displayed within IDS image widgets.  The RenderImageWidget 
**	provides support for the IDS ImageWidget and PannedImageWidget 
**	subclasses and cannot be used as a stand-alone widget.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**
**  AUTHOR(S):
**
**     Subu Garikapati
**
**  CREATION DATE:  February 12, 1988
**
**  MODIFICATION HISTORY:
**
**
*****************************************************************************/

    /*
    **  Standard C include files
    */

#include <math.h>                           /* math routines                */

    /*
    **  CHF,ISL,XIE and IDS include files
    */
#define	    MOTIFRENDERIMAGE    /* defined so IDS$$WIDGET.H knows who we are */


#include    <img/ImgDef.h>         /* ISL public symbols		     */
/*
** Change the way NULL is defined by ImgDef.h (really IPSCOMMON.SDL)
** from (void *) 0 to just plane 0 because XtIsRealized barfs whith 
** the (void *) usage. mdo 17-apr-1991
*/
#ifdef ASK_MARK
#if defined(NULL)
#undef NULL
#endif
#define NULL 0
#endif

#include    <img/ChfDef.h>         /* Condition handling functions           */
#include    <img/IdsStatusCodes.h>

#include <ids__widget_motif.h> /* IDS public/private, Xmoolkit, Xlib defs.    */
#ifndef NODAS_PROTO
#include <idsprot.h>            /* IDS prototypes */
#endif


/*
**  Table of contents
*/
    /*
    **  PUBLIC entry points
    */
#ifdef NODAS_PROTO
#ifdef VMS
IdsAllocStatistics IDSXM$GET_COLOR_STATISTICS(); /* VMS: color alloc' stat's */
#endif
IdsAllocStatistics IdsXmGetColorStatistics();	 /* C:   color alloc' stat's */
    /*
    **  Private IDS entry points
    */
Boolean IdsXmCompareRenderings(); /* Compare Proposed/Current renderings    */
void	IdsXmTranslatePoint();	  /* Compute pre-rendered coordinates	    */

    /*
    **  Standard widget functions exported to Intrinsics via class record.
    */
static void		ClassInitialize();	/* Render class init	    */
static void		ClassPartInitialize();	/* RenderImage inheritance  */
static void		DefaultResolution();	/* Init resolution resources*/
static void		DefaultLevels();	/* Init dither resources    */
static void		Initialize();		/* Render instance init	    */
static void		Destroy();		/* destroy render widget    */
static Boolean		SetValues();		/* change widget values	    */

    /*
    **  Actions exported to Intrinsics Translation Manager via class record.
    */
static void		Help();			/* user requested help	    */

    /*
    **  Routines exported to subclasses via the Render Image Class Record.
    */
static void		UpdateCallback();	/* update callback list	    */
static Boolean		RenderImage();		/* image rendition driver   */

    /*
    **  internal routines
    */
static RenderContext	SetRenderContext();	/* set rendering defaults   */
static void		CopyFid();		/* pre-process new images   */
static void		CopyColors();		/* load appl supplied colors*/
static unsigned long	PostRendering();	/* post rendering for PLP   */
static void             PostRenderingXie();     /* post rendering for PixPho*/
static void		SetImageGC();		/* create/select image GC   */
static void             SetGCForXie();          /* create image GC for pipe */
static void		DestroyImageGCs();	/* destroy private GC's	    */
static void		DeleteRendering();	/* free rendering resources */
static void		DeallocateColors();	/* free colormap  resources */
static void		UnwindPipe();		/* unwind from aborted pipe */
static void             InitializeForXie();     /* connect to XIE if present*/
static void             SetXieData();           /* Set info for XIE         */
static Boolean		PipeNotify();		/* pipe call-ahead routine  */
static void             XiePipeNotify();        /* XIE pipe complte routine */
static DataForXie       PrivateXieData();       /* Collect Xie related data */
static Boolean          ErrorCallback();        /* Error msg display routine*/
static void             CallErrorCallback();    /* Calls ErrorCallback      */
static void             SaveRenderedXieData();  /* Save rend fid, xieimage,pho*/
static unsigned long    SaveRenderedIslFid();   /* Save rend fid ISL       */
static void             CreateEmptyPhoto();     /* create a server photo    */
static void             FidToXieImage();        /* convert fid to xieimage  */
static void             DeleteRenderingFlo();   /* free rendering photo res */
static void             DeleteRenderingFid();   /* free rendering fid   res */
static void             SwitchMode();           /* switch from ISL to Xie.. */

#else
PROTO(static void ClassInitialize, (void));
PROTO(static void ClassPartInitialize, (RenderImageMotifClass /*rc*/));
PROTO(static void DefaultResolution, (Widget /*rw*/, int /*offset*/, XrmValue */*value*/));
PROTO(static void DefaultLevels, (Widget /*rw*/, int /*offset*/, XrmValue */*value*/));
PROTO(static void Initialize, (Widget /*req*/, Widget /*new*/, ArgList /*args*/, Cardinal */*num_args*/));
PROTO(static void Destroy, (Widget /*rw*/));
PROTO(static Boolean SetValues, (Widget /*old*/, Widget /*req*/, Widget /*new*/));
PROTO(static void Help, (Widget /*rw*/, XEvent */*event*/));
PROTO(static void UpdateCallback, (Widget /*old*/, XtCallbackList /*old_list*/, Widget /*new*/, XtCallbackList /*new_list*/, char */*name*/));
PROTO(static Boolean RenderImage, (Widget /*rw*/, XEvent */*event*/));
PROTO(static RenderContext SetRenderContext, (Widget /*iw*/));
PROTO(static void CopyFid, (Widget /*rw*/, unsigned long /*old_fid*/));
PROTO(static void CopyColors, (Widget /*rw*/));
PROTO(static unsigned long PostRendering, (Widget /*rw*/, unsigned long /*fid*/));
PROTO(static void PostRenderingXie, (Widget /*rw*/));
PROTO(static void SetImageGC, (Widget /*rw*/, int /*polarity*/));
PROTO(static void SetGCForXie, (Widget /*rw*/, DataForXie /*xiedat*/));
PROTO(static void DestroyImageGCs, (Widget /*rw*/));
PROTO(static void DeleteRendering, (Widget /*rw*/));
PROTO(static void DeleteRenderingFlo, (Widget /*rw*/));
PROTO(static void DeleteRenderingFid, (Widget /*rw*/));
PROTO(static void DeallocateColors, (Widget /*rw*/));
PROTO(static void UnwindPipe, (Widget /*rw*/, caddr_t /*last*/));
PROTO(static Boolean PipeNotify, (IdsPipeDesc /*pd*/, unsigned long /*id*/, Widget /*rw*/));
PROTO(static void XiePipeNotify, (IdsPipeDesc /*pd*/, unsigned long /*id*/, Widget /*rw*/));
PROTO(static Boolean ErrorCallback, (char * /*rw*/, int /*code*/));
PROTO(static void CallErrorCallback, (int /*code*/));
PROTO(static void InitializeForXie, (Widget /*rw*/));
PROTO(static void SetXieData, (Widget /*old*/, Widget /*new*/));
PROTO(static DataForXie PrivateXieData, (Widget /*rw*/));
PROTO(static void SaveRenderedXieData, (RenderContext /*ctx*/, unsigned long /*photo*/, unsigned long /*xieimage*/, unsigned long /*fid*/));
PROTO(static unsigned long SaveRenderedIslFid, (RenderContext /*ctx*/, unsigned long /*id*/, unsigned long /*fid*/));
PROTO(static void FidToXieImage, (Widget /*rw*/));
PROTO(static void CreateEmptyPhoto, (Widget /*rw*/));
PROTO(static void SwitchMode, (Widget /*old*/, Widget /*new*/));
#endif
/*
**  MACRO definitions -- ( see also: IdsImage.h and IDS$$MACROS.H )
*/

/*
**  Equated Symbols
*/
    /* none */
/*
**  External References
*/
#ifdef NODAS_PROTO
extern XiePhoto         IdsCompileServerPipe();
extern IdsPipeDesc      IdsCompileRendering();
extern IdsPipeDesc      IdsCompileExport();
extern unsigned long    IdsExecutePipe();
extern IdsPipeDesc      IdsAllocatePipeDesc();
extern void             IdsDeallocatePipeDesc();
extern struct FCT      *IdsConvertPlane();
extern void             CompareRenderingsFid();
extern Boolean          CompareRenderingsFlo();
extern int              Error_condition_handler(); /* Conditon handler       */
extern void             TranslatePointFid();
extern void             TranslatePointFlo();
extern void             CopyFidIsl();
extern void             CopyFidXie();

extern unsigned long    IdsFidToXieImage();
extern unsigned long    IdsXieImageToFid();
extern unsigned long    IdsPhotoToXieImage();
extern DataForXie       IdsDataForXie();             
extern unsigned long    CollectRoiData();     /* Collect ISL ROI attribut */
extern unsigned long    ConvertRenderedFid();             
#endif
/* global variables */
                                                                               
/*
**	Local Storage
*/
static IdsErrorFunc IdsErrorCb;            /* addr ptr comes from now widget to wid */
    /*
    **	default translation and action recs
    */
static char RenderTranslations[] =
	"@Help<Btn1Up>:		IDS-image-help()";

static XtActionsRec RenderActions[] = {
       {"IDS-image-help",	      (XtActionProc)Help},
       };

    /*
    **	Externally accessable resources that this widget adds.
    **
    **	layout of Resource structure:
    **	    typedef struct _XmPartResource {
    **		String  resource_name;	String	 resource_class;
    **	        String  resource_type;	Cardinal resource_size;
    **          Cardinal resource_offset;
    **	        String  default_type;	String	 default_address;
    **	    } XmPartResource;
    */

static XtResource RenderResources[] = {

    {	IdsNimageForeground,IdsCImageForeground,
	XtRPixel,	    sizeof(Pixel),
#ifdef sparc
	(XmManagerIndex << XmHALFLONGBITS) + XtOffset(XmManagerPart *, foreground),
#else
	XmPartOffset(XmManager,foreground),
#endif
	XtRString,	    "Black" },

    {	IdsNimageBackground,IdsCImageBackground,
	XtRPixel,	    sizeof(Pixel),
	XmPartOffset(Core,background_pixel),
	XtRString,	    "White" },

    {	IdsNframeDepth,	    IdsCFrameDepth,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,ximage.depth),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNframeWidth,	    IdsCFrameWidth,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,ximage.width),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNframeHeight,    IdsCFrameHeight,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,ximage.height),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNrenderMode,	    IdsCRenderMode,
	IdsRRenderMode,	    sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.render_mode),
	XtRString,	    IdsSNormal },

    {	IdsNrenderScheme,   IdsCRenderScheme,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.render_scheme),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNfid,	    IdsCFid,
	XtRPointer,	    sizeof(int *),
        IdsPartOffset_(Render,RenderImage,proposed.fid),
	XtRPointer,	    NULL },

    {	IdsNroi,	    IdsCRoi,
	XtRPointer,	    sizeof(int *),
        IdsPartOffset_(Render,RenderImage,proposed.roi),
	XtRPointer,	    NULL },

    {	IdsNrotateMode,	    IdsCRotateMode,
	IdsRRotateMode,	    sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.rotate_mode),
	XtRString,	    IdsSNoRotate },

    {	IdsNrotateOptions,  IdsCRotateOptions,
	IdsRRotateOptions,  sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.rotate_options),
	XtRString,	    IdsSBilinear },

    {	IdsNrotateAngle,    IdsCRotateAngle,
	IdsRFloat,	    sizeof(float),
        IdsPartOffset_(Render,RenderImage,proposed.angle),
	XtRString,	    "0.0" },

    {   IdsNrotateWidth,    IdsCRotateWidth,
        XtRInt,             sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.rotate_width),
        XtRImmediate,       (caddr_t) 0 },

    {   IdsNrotateHeight,   IdsCRotateHeight,
        XtRInt,             sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.rotate_height),
        XtRImmediate,       (caddr_t) 0 },

    {   IdsNrotateFirstFill,   IdsCRotateFirstFill,
        XtRInt,             sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.rotate_fill[0]),
        XtRImmediate,       (caddr_t) 0 },

    {   IdsNrotateSecondFill,  IdsCRotateSecondFill,
        XtRInt,             sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.rotate_fill[1]),
        XtRImmediate,       (caddr_t) 0 },

    {   IdsNrotateThirdFill,   IdsCRotateThirdFill,
        XtRInt,             sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.rotate_fill[2]),
        XtRImmediate,       (caddr_t) 0 },

    {   IdsNcomputeMode,    IdsCComputeMode,
        IdsRComputeMode,    sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.compute_mode),
        XtRString,          IdsSXieServer },
      
    {	IdsNflipOptions,    IdsCFlipOptions,
	IdsRFlipOptions,    sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.flip_options),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNscaleMode,	    IdsCScaleMode,
	IdsRScaleMode,	    sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.scale_mode),
	XtRString,	    IdsSNoScale },

    {	IdsNscaleOptions,   IdsCScaleOptions,
	IdsRScaleOptions,   sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.scale_options),
	XtRString,	    "savehorizontal+savevertical+bilinear" },

    {	IdsNxScale,	    IdsCScaleFactor,
	IdsRFloat,	    sizeof(float),
        IdsPartOffset_(Render,RenderImage,proposed.x_scale),
	XtRString,	    "1.0" },

    {	IdsNyScale,	    IdsCScaleFactor,
	IdsRFloat,	    sizeof(float),
        IdsPartOffset_(Render,RenderImage,proposed.y_scale),
	XtRString,	    "1.0" },

    {	IdsNxPelsPerBMU,    IdsCPelsPerBMU,
	IdsRFloat,	    sizeof(float),
        IdsPartOffset_(Render,RenderImage,proposed.x_pels_per_bmu),
	XtRCallProc,	    (caddr_t)DefaultResolution },

    {	IdsNyPelsPerBMU,    IdsCPelsPerBMU,
	IdsRFloat,	    sizeof(float),
        IdsPartOffset_(Render,RenderImage,proposed.y_pels_per_bmu),
	XtRCallProc,	    (caddr_t)DefaultResolution },

    {	IdsNpunch1,	    IdsCPunch1,
	IdsRFloat,	    sizeof(float),
        IdsPartOffset_(Render,RenderImage,proposed.punch1),
	XtRString,	    "0.0" },

    {	IdsNpunch2,	    IdsCPunch2,
	IdsRFloat,	    sizeof(float),
        IdsPartOffset_(Render,RenderImage,proposed.punch2),
	XtRString,	    "1.0" },

    {   IdsNsharpen,	    IdsCSharpen,
	IdsRFloat,	    sizeof(float),
        IdsPartOffset_(Render,RenderImage,proposed.sharpen),
	XtRString,	    "0.0" } ,

    {	IdsNditherAlgorithm,IdsCDitherAlgorithm,
	IdsRDitherMode,	    sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.dither_algorithm),
	XtRString,	    IdsSBlueNoise },

    {	IdsNditherThreshold,IdsCDitherThreshold,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,proposed.dither_threshold),
	XtRImmediate,	    (caddr_t) 5 },

    {   IdsNsaveRendition,   IdsCSaveRendition,
        XtRInt,             sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.save_mode),
        XtRImmediate,       (caddr_t) 0 },
    
    {   IdsNcompressMode,   IdsCCompressMode,
        IdsRCompressMode,   sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.cmpres_mode),
        XtRString,          IdsSUnCompress},

   {    IdsNcomporgMode,   IdsCComporgMode,
        IdsRComporgMode,   sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.cmporg_mode),
        XtRString,          IdsSBandByPixel},
       
    {	IdsNpixelList,	    IdsCPixelList,
	XtRPointer,	    sizeof(Pixel *),
        IdsPartOffset_(Render,RenderImage,context.pixel_index_list),
	XtRPointer,	    NULL },

    {	IdsNpixelCount,	    IdsCPixelCount,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.pixel_index_count),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNpaletteList,    IdsCPaletteList,
	XtRPointer,	    sizeof(Pixel *),
        IdsPartOffset_(Render,RenderImage,context.palette_index_list),
	XtRPointer,	    NULL },

    {	IdsNpaletteCount,  IdsCPaletteCount,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.palette_index_count),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNcolormapMode,   IdsCColormapMode,
	IdsRColormapMode,   sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.colormap_mode),
	XtRString,	    IdsSShareColors },

    {	IdsNcolormapUpdate, IdsCColormapUpdate,
	XtRBoolean,	    sizeof(Boolean),
        IdsPartOffset_(Render,RenderImage,context.colormap_update),
	XtRImmediate,	    (caddr_t) False },

    {	IdsNcolorSpace,	    IdsCColorSpace,
	IdsRColorSpace,	    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.color_space),
	XtRString,	    IdsSYIQSpace },

    {	IdsNmatchLimit,	    IdsCMatchLimit,
	IdsRFloat,	    sizeof(float),
        IdsPartOffset_(Render,RenderImage,context.match_limit),
	XtRString,	    "0.0" },

    {	IdsNgrayLimit,	    IdsCGrayLimit,
	IdsRFloat,	    sizeof(float),
        IdsPartOffset_(Render,RenderImage,context.gray_limit),
	XtRString,	    "0.0" },

    {	IdsNrenderingClass, IdsCRenderingClass,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.rendering_class),
	XtRCallProc,	    (caddr_t)DefaultLevels },

    {	IdsNlevelsGray,	    IdsCLevels,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.levels_gray),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNlevelsRed,	    IdsCLevels,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.levels_rgb[RED]),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNlevelsGreen,    IdsCLevels,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.levels_rgb[GRN]),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNlevelsBlue,	    IdsCLevels,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.levels_rgb[BLU]),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNfitLevels,	    IdsCLevels,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.fit_levels),
	XtRCallProc,	    (caddr_t)DefaultLevels },

    {	IdsNfitWidth,	    IdsCFitWidth,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.fit_width),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNfitHeight,	    IdsCFitHeight,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.fit_height),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNprotocol,	    IdsCProtocol,
	IdsRProtocol,	    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.protocol),
	XtRString,	    IdsSXImage },

    {   IdsNcopyFid,	    IdsCCopyFid,
	XtRBoolean,	    sizeof(Boolean),
        IdsPartOffset_(Render,RenderImage,copy_fid),
	XtRImmediate,	    (caddr_t) False },

    {   IdsNrenderCallback, XtCCallback, 
	XtRCallback,	    sizeof(XtCallbackList),
        IdsPartOffset_(Render,RenderImage,render_callback),
	XtRCallback,	    NULL },

    {   IdsNxieListCallback, XtCCallback,
        XtRCallback,        sizeof(XtCallbackList),
        IdsPartOffset_(Render,RenderImage,xielist_callback),
        XtRCallback,        NULL },

    {   IdsNsaveImageCallback, XtCCallback,
        XtRCallback,        sizeof(XtCallbackList),
        IdsPartOffset_(Render,RenderImage, save_callback),
        XtRCallback,        NULL },
      
    {   IdsNworkNotifyCallback, XtCCallback, 
	XtRCallback,	    sizeof(XtCallbackList),
        IdsPartOffset_(Render,RenderImage,work_notify_callback),
	XtRCallback,	    NULL },

    {   IdsNerrorCallback, XtCCallback,
        XtRCallback,        sizeof(XtCallbackList),
        IdsPartOffset_(Render,RenderImage,error_callback),
        XtRCallback,        NULL },
};


/*
    {	IdsNXieRoix,         IdsCXieRoix,
	XtRInt,		    sizeof(int),
        IdsPartOffset_(Render,RenderImage,xiecontext.roigeom.x),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNXieRoiy,         IdsCXieRoiy,
	XtRInt,		     sizeof(int),
        IdsPartOffset_(Render,RenderImage,xiecontext.roigeom.y),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNXieRoiw,         IdsCXieRoiw,
	XtRInt,		     sizeof(int),
        IdsPartOffset_(Render,RenderImage,xiecontext.roigeom.w),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNXieRoih,         IdsCXieRoih,
	XtRInt,		     sizeof(int),
        IdsPartOffset_(Render,RenderImage,xiecontext.roigeom.h),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNXieRoiType,      IdsCXieRoiType,
	XtRInt,		     sizeof(int),
        IdsPartOffset_(Render,RenderImage,xiecontext.roigeom.type),
	XtRImmediate,	    (caddr_t) 0 },

    {	IdsNXieRoi,         IdsCXieRoi,
	XtRInt,		     sizeof(int),
        IdsPartOffset_(Render,RenderImage,xiecontext.roigeom.roi),
	XtRImmediate,	    (caddr_t) 0 },
*/

    /*
    **  Class record set at compile/link time, passed to the widgetcreate
    **  routine as the the class.
    */
externaldef(renderImageWidgetMotifClassRec) RenderImageMotifClassRec
	    renderImageWidgetMotifClassRec = {

  /*	Core Class Part		*/
  {
    /* superclass	    */	(WidgetClass) &xmManagerClassRec,
    /* class_name	    */	IdsSClassRenderImage,
#ifdef __alpha
/*
** this resolves unalligned access errors but keeps the message box from
** going away automatically in the demo_motif program.  ASK_MARK
*/
    /* widget_size	    */	sizeof(RenderImageWidgetRec),
#else
    /* widget_size	    */	sizeof(RenderImagePart),
#endif
    /* class_initialize	    */  ClassInitialize,
    /* class_part_initialize*/	ClassPartInitialize,
    /* class_inited	    */	FALSE,
    /* initialize	    */	Initialize,
    /* initialize hook	    */	NULL,
    /* realize		    */	NULL,
    /* actions		    */	RenderActions,
    /* num_actions	    */	XtNumber(RenderActions),
    /* resources	    */	(XtResourceList) RenderResources,
    /* num_resources	    */  XtNumber(RenderResources),
    /* xrm_class	    */	NULLQUARK,
    /* compress_motion	    */	TRUE,
    /* compress_exposure    */	TRUE,
    /* compress_enterleave  */	TRUE,
    /* visible_interest	    */	FALSE,
    /* destroy		    */	Destroy,
    /* resize		    */	NULL,
    /* expose		    */	NULL,
    /* set_values	    */	(XtSetValuesFunc)SetValues,
    /* set_values_hook	    */	NULL,
    /* set_values_almost    */	(XtAlmostProc)_XtInherit,
    /* get_values_hook	    */	NULL,
    /* accept_focus	    */	NULL,
    /* version		    */	XtVersionDontCheck,
    /* callback_offsets	    */	NULL,
    /* tm_table		    */	RenderTranslations,
    /* query_geometry	    */	NULL,
    /* display_accelerator  */	NULL,
    /* extension	    */	NULL,
    },

  /*    Composite class fields  */
    {
        /* geometry_manager     */      (XtGeometryHandler) _XtInherit,
        /* change_managed       */      (XtWidgetProc) _XtInherit,
        /* insert_child         */      (XtWidgetProc) _XtInherit,
        /* delete_child         */      (XtWidgetProc) _XtInherit,
        /* extension            */      NULL,
    },

  /*    Constraint class fields  */
    {
        /* constrain_resources  */      NULL,
        /* num_resources        */      0,
        /* size_record          */      0,
        /* constrain_initialize */      NULL,
        /* constrain_destroy    */      NULL,
        /* constrain_setvalues  */      NULL,
        /* extension            */      NULL,
    },

  /*    XmManager class fields  */
    {
#ifdef sparc
        /* translations _XtInherit, */ (XtTranslations) XtInheritTranslations,
#else
        /* translations _XtInherit, */                  XtInheritTranslations,
#endif
        /* get_resources                */    NULL,
        /* num_get_resources            */    0,
        /* get_constraint_resources     */    NULL,
        /* num_get_constraint_resources */    0,
        /* extension                    */    NULL,
    },
  
  /*	RenderImage Class Part	*/
    {
    /* offsets		    */	NULL,
    /* update_callback	    */	UpdateCallback,
    /* render_image	    */	RenderImage,
    /* ErrorCallback        */  (XtProc) ErrorCallback,
    /* pad0		    */	NULL,
    /* extension	    */	NULL,
   }
};

externaldef(renderImageWidgetMotifClass) RenderImageMotifClass
    renderImageWidgetMotifClass = (RenderImageMotifClass) &renderImageWidgetMotifClassRec;

/*****************************************************************************
**  IDSXM$GET_COLOR_STATISTICS
**
**  FUNCTIONAL DESCRIPTION:
**
**      VMS public entry: get color usage/matching statistics.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget id
**
**  FUNCTION VALUE:
**
**      IdsAllocStatistics - statistics of matched colors exceeding 'threshold'.
**
*****************************************************************************/
#ifdef IdsVMS
IdsAllocStatistics IDSXM$GET_COLOR_STATISTICS( rw )
 Widget		*rw;
{
    /*
    **	Call the C version with d'referenced arguments.
    */
    return( IdsXmGetColorStatistics( *rw ));
}
#endif

/*****************************************************************************
**  IdsXmGetColorStatistics
**
**  FUNCTIONAL DESCRIPTION:
**
**      C public entry: get color usage/matching statistics.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget id
**
**  FUNCTION VALUE:
**
**      IdsAllocStatistics - statistics of matched colors exceeding 'threshold'.
**
*****************************************************************************/
IdsAllocStatistics IdsXmGetColorStatistics( rw )
 Widget	    rw;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext ctx  = &Context_(rw);
    IdsAllocStatistics  stat = NULL;

#ifdef TRACE
printf( "Entering Routine IdsXmGetColorStatistics in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( !XtIsSubclass( rw, (WidgetClass)imageWidgetMotifClass ))
	/*
	**  No renagade widgets allow here.
	*/
	XtErrorMsg("notImgWgt","IdsGetColorStatistics","IdsImageError",
		   "widget is not of ImageWidgetMotifClass or subclass",NULL,NULL);

    if( PxlDat_(ctx) == NULL && cFid_(rw) != 0 )
	{
	/*
	**  Either color matching was not required, or image was Bitonal.
	*/
	stat = (IdsAllocStatistics)XtCalloc(1,sizeof(IdsAllocStatisticsStruct));
	stat->total = PxlCnt_(ctx);
	}
    if( PxlDat_(ctx) != NULL )
	/*
	**  Get color match statistics for threshold == 0.0
	*/
	stat = IdsGetAllocStatistics( PxlDat_(ctx), 0.0 );

#ifdef TRACE
printf( "Leaving Routine IdsXmGetColorStatistics in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    return( stat );
}

/*****************************************************************************
**  IdsXmCompareRenderings
**
**  FUNCTIONAL DESCRIPTION:
**
**      Compare Proposed and Current renderings.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget id
**
**  FUNCTION VALUE:
**
**	Boolean	- TRUE  if an IDS render mode and scheme is generated.
**	Boolean	- FLASE if Ids_Abort requested or no changes detected.
**
*****************************************************************************/
Boolean IdsXmCompareRenderings( rw )
 Widget rw;
{
    DefineWidgetParts_(RenderImage,rw);
    IdsRenderCallback old = &Current_(rw), new = &Proposed_(rw);
    RenderContextXie  xiectx = &XieContext_(rw);
    RenderContext     ctx;  
    Boolean           nosupport = False;
    Boolean           retval;
    IdsRenderCallback rcb;

#ifdef TRACE
printf( "Entering Routine IdsXmCompareRenderings in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if (!XtIsRealized(rw) || 
	!XtIsSubclass(rw, (WidgetClass)imageWidgetMotifClass))
        XtErrorMsg("notImgWgt","IdsXmCompareRenderings","IdsImageError",
                   "not a realized Image Widget",0,0);
   
    ctx = SetRenderContext(rw);
    rcb   = PropRnd_(ctx);

    if (rcb != 0)
        intCompute_(rcb) = iCompute_(rcb);

    if (( intCompute_(rcb) == Ids_XieServer ) || ((rcb == 0) &&
                         (iCompute_(new) == Ids_XieServer)))
        {
         nosupport = CompareRenderingsFlo( old, new, ctx, xiectx, GC_(rw), 
					ForceCopyFid_(rw) || CopyFid_(rw) );
         if ( nosupport )
            {
            /*
            ** Free the xieimage struct and xiedat structs
            */
	    if( iXieimage_(new) != 0 && PriXie_(xiectx) != 0 ) 
		{
		iXieimage_(new) = XieFreeImage( iXieimage_(new) );
		_ImgFree( PriXie_(xiectx) );
		PriXie_(xiectx) = 0;
		}
	    /*
	    ** XIE has denied us.
	    ** Call CompareRenderingsFid() since this might be are only
	    ** pass through this routine.  (Actually, this seems only to
	    ** happen in the XUI case, but the solution is duplicated here
	    ** anyway).
	    */
	    intCompute_(rcb) = Ids_IslClient;
            iCompute_(old) = Ids_IslClient;
	    CompareRenderingsFid( old, new, ctx, GC_(rw), 
				ForceCopyFid_(rw) || CopyFid_(rw) );
            }
        }
    else 
	CompareRenderingsFid( old, new, ctx, GC_(rw), 
				ForceCopyFid_(rw) || CopyFid_(rw) );

    /*
    **  Return TRUE unless the rendering request is aborting.
    */
    retval = IfReplace_( iRender_(new) == Ids_Abort,
			 iRender_(new),   Ids_Passive );
#ifdef TRACE
printf( "Leaving Routine IdsXmCompareRenderings in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    return !(retval);
}

/*******************************************************************************
**  IdsXmTranslatePoint
**
**  FUNCTIONAL DESCRIPTION:
**
**      Translate an (x,y) point back to its pre-rendered coordinate values.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget 
**	coords	- coordinate point pair
**
*******************************************************************************/
void IdsXmTranslatePoint( rw, coords )
 Widget rw;
 XPoint *coords;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext ctx = &Context_(rw);
    IdsRenderCallback rcb   = PropRnd_(ctx);
    RenderContextXie  xiectx = &XieContext_(rw);
    DataForXie xiedat = PriXie_(xiectx);
    IdsRenderCallback cur = &Current_(rw), new = &Proposed_(rw);
    unsigned long width, height;

#ifdef TRACE
printf( "Entering Routine IdsXmTranslatePoint in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    width  = Iwide_(rw);
    height = Ihigh_(rw);

    if ( intCompute_(rcb) == Ids_XieServer )
	TranslatePointFlo( xiedat, cur, coords, width, height );
    else  
	TranslatePointFid( cur, coords, width, height );
#ifdef TRACE
printf( "Leaving Routine IdsXmTranslatePoint in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ClassInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      Class initialization routine.
**	Called only the first time a widget of this class is initialized.
**
*****************************************************************************/
static void ClassInitialize()
{
#ifdef TRACE
printf( "Entering Routine ClassInitialize in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    **	Create actual offsets to instance record parts and to resources.
    */
    XmResolvePartOffsets(renderImageWidgetMotifClass,
		    &renderImageWidgetMotifClassRec.render_image_class.offsets);
    /*
    **	Register various type converters.
    */
    XtAddConverter(XtRString, IdsRFloat,         IdsStringToFloat,      0,0);
    XtAddConverter(XtRString, IdsRRenderMode,	 IdsStringToRenderMode, 0,0);
    XtAddConverter(XtRString, IdsRRenderingClass,IdsStringToRenderClass,0,0);
    XtAddConverter(XtRString, IdsRProtocol,      IdsStringToProtocol,   0,0);
    XtAddConverter(XtRString, IdsRColormapMode,  IdsStringToColormapMode, 0,0);
    XtAddConverter(XtRString, IdsRComputeMode,   IdsStringToComputeMode, 0,0);
    XtAddConverter(XtRString, IdsRSaveRendition, IdsStringToSaveRendMode, 0,0);
    XtAddConverter(XtRString, IdsRCompressMode,  IdsStringToCompressMode, 0,0);
    XtAddConverter(XtRString, IdsRComporgMode,   IdsStringToComporgMode, 0,0);
    XtAddConverter(XtRString, IdsRRotateMode,    IdsStringToRotateMode, 0,0);
    XtAddConverter(XtRString, IdsRRotateOptions, IdsStringToRotateOpts, 0,0);
    XtAddConverter(XtRString, IdsRFlipOptions,	 IdsStringToFlipOpts,   0,0);
    XtAddConverter(XtRString, IdsRScaleMode,     IdsStringToScaleMode,  0,0);
    XtAddConverter(XtRString, IdsRScaleOptions,  IdsStringToScaleOpts,  0,0);
    XtAddConverter(XtRString, IdsRDitherMode,    IdsStringToDitherMode, 0,0);
    XtAddConverter(XtRString, IdsRColorSpace,    IdsStringToColorSpace, 0,0);
#ifdef TRACE
printf( "Leaving Routine ClassInitialize in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ClassPartInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**	Called only the first time a widget of this class is initialized.
**      Passes inheritance of RenderImageClass functions to subclasses.
**
**  FORMAL PARAMETERS:
**
**	rc	- RenderImage Class record
**
*****************************************************************************/
static void ClassPartInitialize( rc )
    RenderImageMotifClass	rc;
{
    RenderImageMotifClass   super = (RenderImageMotifClass)rc->core_class.superclass;

#ifdef TRACE
printf( "Entering Routine ClassPartInitialize in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    **	Pass inheritance of RenderImageMotifClass functions to subclasses.
    */
    if( (IDS_VoidProc) rc->render_image_class.update_callback
     == (IDS_VoidProc) _XtInherit )
	rc->render_image_class.update_callback =
	    super->render_image_class.update_callback;

    if( (IDS_VoidProc) rc->render_image_class.render_image
     == (IDS_VoidProc) _XtInherit )
	rc->render_image_class.render_image =
	    super->render_image_class.render_image;
#ifdef TRACE
printf( "Leaving Routine ClassPartInitialize in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  DefaultResolution
**
**  FUNCTIONAL DESCRIPTION:
**
**      Resource initialize routine, called once for each widget created.
**	If the default horizontal screen resolution and the default vertical 
**	screen resolution are within 5% of each other, then make both equal.
**	    (ie. the pixels are close enough to being square).
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget
**	offset	- resource offset
**	value	- resource value structure
**
*****************************************************************************/
static void DefaultResolution( rw, offset, value )
    Widget	rw;
    int		offset;
    XrmValue   *value;
{
    DefineWidgetParts_(RenderImage,rw);

    static float x_resolution = 0.0, y_resolution = 0.0;
    int resource = offset - renderImageWidgetMotifClassRec.
				render_image_class.offsets[RenderIndex];

#ifdef TRACE
printf( "Entering Routine DefaultResolution in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( resource == XtOffset( RenderImagePart*, proposed.x_pels_per_bmu ))
	{
	/*
	**  Default horizontal screen resolution in Pels-per-BMU.
	*/
	x_resolution = (MM_per_BMU) *
			(
			    (float)  WidthOfScreen(XtScreen(rw))
				/ 
				    (float) WidthMMOfScreen(XtScreen(rw))
			);

	value->addr = (caddr_t) &x_resolution;
	}

    else if( resource == XtOffset( RenderImagePart*, proposed.y_pels_per_bmu ))
	{
	/*
	**  Default vertical screen resolution in Pels-per-BMU.
	*/
	y_resolution = (MM_per_BMU) * 
			(
			    (float) HeightOfScreen(XtScreen(rw))
				/ 
				    (float) HeightMMOfScreen(XtScreen(rw))
			);

	value->addr = ( fabs( x_resolution / y_resolution - 1.0 ) > 0.05 )
		    ? (caddr_t) &y_resolution : (caddr_t) &x_resolution;
	}
#ifdef TRACE
printf( "Leaving Routine DefaultResolution in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  DefaultLevels
**
**  FUNCTIONAL DESCRIPTION:
**
**      Resource initialize routine for "visualClass" and "fitLevels".
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget
**	offset	- resource offset
**	value	- resource value structure
**
*****************************************************************************/
static void DefaultLevels( rw, offset, value )
    Widget	rw;
    int		offset;
    XrmValue   *value;
{
    DefineWidgetParts_(RenderImage,rw);
    static int return_value;
    Visual *visual;
    int resource = offset - renderImageWidgetMotifClassRec.
			    render_image_class.offsets[RenderIndex];

    visual = DefaultVisualOfScreen( XtScreen(rw) );

#ifdef TRACE
printf( "Entering Routine DefaultLevels in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( resource == XtOffset(RenderImagePart*,context.rendering_class))
	/*
	**  Convert visual class into a rendered image class we can support.
	*/
	return_value = visual->class == GrayScale   ? Ids_GrayScale
		     : visual->class == PseudoColor ? Ids_Color
                     : visual->class == DirectColor ? Ids_Color
                     : visual->class == TrueColor   ? Ids_Color
                     : visual->class == StaticColor ? Ids_Color
                     : visual->class == StaticGray  ? Ids_GrayScale
 		     : Ids_Bitonal;

    else if( resource == XtOffset(RenderImagePart*,context.fit_levels))
	{
	/*
	**  Round up the number of colormap entries to a power of 2,
	**	(some servers lie about their colormap size because 
	**	 of colors that are reserved for the cursor, etc.).
	*/
	SetBitsPerComponent_(return_value, visual->map_entries);
	return_value  = 1 << return_value;
        if( visual->class == DirectColor || visual->class == TrueColor )
            return_value = return_value * return_value * return_value;
	}
    else
	return_value = 0;

    value->addr = (caddr_t) &return_value;
#ifdef TRACE
printf( "Leaving Routine DefaultLevels in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  Initialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      Instance initialize routine, called once for each widget created.
**
**  FORMAL PARAMETERS:
**
**	req	- Render Image Widget, as built from arglist.
**	new	- Render Image Widget, as modified by superclasses.
**
*****************************************************************************/
static void Initialize( req, new, args, num_args )
    Widget  req, new;
    ArgList args;
    Cardinal *num_args;
{
    DefineWidgetParts_(RenderImage,new);
    RenderContext    ctx     = &Context_(new);
    RenderContextXie xiectx  = &XieContext_(new);
    XImage *tmp;
    int    opcode, event0, error0, function;
    char       *text, **names;

#ifdef TRACE
printf( "Entering Routine Initialize in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    **  Create a skeleton XImage structure, and copy it to our instance record.
    */
    tmp = XCreateImage( XtDisplay(new), DefaultVisualOfScreen(XtScreen(new)),
					DefaultDepthOfScreen(XtScreen(new)),
					ZPixmap, 0, 0, 0, 0,
					BitmapPad(XtDisplay(new)), 1 );

    Img_(new)   = *tmp;
    XDestroyImage( tmp );
    Image_(ctx)	  = &Img_(new);
    Pad_(ctx)	  = BitmapPad(XtDisplay(new));
    PxlStr_(ctx)  = Ibppx_(new);
    PxlAlloc_(ctx)= NULL;
    PxlLst_(ctx)  = NULL;
    PxlCnt_(ctx)  = 0;
    PxlDat_(ctx)  = NULL;
    RqLst_(ctx)	  = NULL;
    TsLst_(ctx)	  = NULL;
    DiLst_(ctx)	  = NULL;
    Pipe_(ctx)    = IdsAllocatePipeDesc();
    Error_(ctx)   =  (IdsErrorFunc)_ImgCalloc(1,sizeof(IdsErrorFuncStruct));
    ECwidget_(Error_(ctx))  = (char *)new;    /* Pointer to render context    */
    ECcall_(Error_(ctx))  = (ErrorCallProc) CallErrorCallback;
    IdsErrorCb    = Error_(ctx);
    Switch_(ctx)  = False;
    PDcall_(Pipe_(ctx)) = PipeNotify;
    PDdata_(Pipe_(ctx)) = (char *) new;
    PropRnd_(ctx) = NULL;
    Pix_(new)	  = NULL;
    StdGC_(new)   = NULL;
    RevGC_(new)   = NULL;
    GC_(new)	  = NULL;
    RFid_(new)	  = 0;
    RPhoto_(new)    = 0;
    pReason_(new) = 0;
    pXieImg_(new) = NULL;
    pROI_(new) = 0;
    if((CmpMode_(ctx) == Ids_PrivateColors) && CmpUpd_(ctx) )
        CopyColors( new );          /* if appl given colors load internally */
    ForceCopyFid_(new) = False;
    if( CopyFid_(new) && pFid_(new) != 0 )
	CopyFid( new, 0 );	    /* pre-process (copy) image frame	    */
    SetRenderContext(new);	    /* initialize 'proposed' from defaults  */
    Current_(new) = Proposed_(new); /* current = proposed, except...	    */
    cFid_(new)	  = 0;		    /* ... current is invalid (ie. no fid). */
    cXieImg_(new) = 0;		    /*  current is invalid (ie. no xieimg). */
    Photo_(xiectx)  = 0;            /* ... current is invalid (ie. no flo). */
    cROI_(new) = 0;
    memset(xiectx,NULL,sizeof(RenderContextStructXie));/* DEX explodes ... */
    RRend_(xiectx) = False;
    InitializeForXie(new); 
    if( Process_(ctx) == Ids_XieServer )
        FidToXieImage( new );
#ifdef TRACE
printf( "Leaving Routine Initialize in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  Destroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      Widget's been requested to destroy its resources.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget 
**
*****************************************************************************/
static void Destroy( rw )
    Widget rw;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext    ctx    = &Context_(rw);
    RenderContextXie xiectx = &XieContext_(rw);
    IdsRenderCallback rcb   = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine Destroy in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    **  Delete rendering resources.
    */
    DeleteRendering(rw);
    DeallocateColors(rw);

    if( intCompute_(rcb) == Ids_IslClient )   
	{	
	/*
	**	Delete rendering resources.
	*/
        IdsFreePutList( &RqLst_(ctx) );
        IdsFreePutList( &TsLst_(ctx) );
        IdsFreePutList( &DiLst_(ctx) );
        IdsDeallocatePipeDesc( Pipe_(ctx) );
        /*
        **	Delete private copy of application fid.
        */
        if ((CopyFid_(rw) || ForceCopyFid_(rw)) && pFid_(rw) != 0 )
	    {
	    ImgDeleteFrame( pFid_(rw) );
	    pFid_(rw) = 0;
	    }
	}
     else
        /*
        ** Free the list of Xie protocol functions
        */
        if( Callback_(XieListCB_(rw)))
            XieFree( XieFunc_(xiectx) );
  
    /*
    **	Destroy private GC's.
    */
    DestroyImageGCs( rw );
#ifdef TRACE
printf( "Leaving Routine Destroy in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  SetValues
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will take care of any changes that the application makes.
**
**  FORMAL PARAMETERS:
**
**	old	- Render Image Widget, copy of original.
**	req	- Render Image Widget, as built from arglist.
**	new	- Render Image Widget, as modified by superclasses.
**
**  FUNCTION VALUE:
**
**	TRUE	something changed, and not a subclass of Image.
**	FALSE	no change, or ImageClass; re-display never requested via Toolkit
**
*****************************************************************************/
static Boolean SetValues( old, req, new )
    Widget  old, req, new;
{
    DefineWidgetParts_(RenderImage,old);
    DefineWidgetParts_(RenderImage,new);
    RenderContext Octx = &Context_(old);
    RenderContext Nctx = &Context_(new);
    unsigned long   fid;
    IdsRenderCallback new_rcb = &Proposed_(new);

#ifdef TRACE
printf( "Entering Routine SetValues in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /* Initialize RGB in the new widget */
    iRGB_(new_rcb)[RED] = RGB_(Nctx)[RED];
    iRGB_(new_rcb)[GRN] = RGB_(Nctx)[GRN];
    iRGB_(new_rcb)[BLU] = RGB_(Nctx)[BLU];
  
    /*
    ** If a new fid is being passed into the widget, then ForceCopyFid_(widget)
    ** must be reset.
    ** SwitchMode() is the only routine that sets ForceCopyFid_() to True.
    ** If that happens, then SwitchMode copies the fid.
    */
    if (pFid_(old) != pFid_(new))
        ForceCopyFid_(new) = False;

    /*
    ** Switch from XIE to ISL compute mode and in ISL mode convert fid to native
    */
    SwitchMode(old, new);

    /*
    **	If bitonal pixel indices have changed, update our image GC's
    */
    if( ForePix_(old) != ForePix_(new) || BackPix_(old) != BackPix_(new) )
	DestroyImageGCs( new );

    Iwide_(new)   = Iwide_(old);	/* preserve our read only resources */
    Ihigh_(new)   = Ihigh_(old);
    Ideep_(new)   = Ideep_(old);
    pScheme_(new) = pScheme_(old);
    PxlLst_(Nctx) = PxlLst_(Octx);
    PxlCnt_(Nctx) = PxlCnt_(Octx);
    CopyFid_(new) = CopyFid_(old);

    /*
    ** If colormap is changed by appl associate the colormap with parent window
    */ 
    if( Colormap_(old) != Colormap_(new) )
     XSetWindowColormap( XtDisplay(new),XtWindow(Parent_(new)), Colormap_(new));
    /*
    **  If "IdsNcolormapMode" is Ids_PrivateColors & IdsNcolormapUpdate is TRUE 
    **  load the appl given private colors into IDS internally
    */
    if( CmpMode_(Nctx) == Ids_PrivateColors && CmpUpd_(Nctx)) 
/*
** Do'nt need to check this flag
** && pRender_(new) == Ids_Passive )
*/
       {
        CopyColors( new );
        pRender_(new) = Ids_Normal;
        /*
        **  Set the colormap_update flag back to false
        */
        CmpUpd_(Nctx) = FALSE;  
       }
    /*
    **	If "IdsNcopyFid" is true we must pre-process new image frames.
    */
    if( CopyFid_(new) && (pFid_(old) != pFid_(new)))
	CopyFid( new, pFid_(old) );

    /*
    ** If XIE Ser present convert fid to xieimage,figure how & when to send data
    */
    if(pCompute_(new) == Ids_XieServer)
        /*
        ** Convert fid to xieimage, create a photoflo, obtain a phototap
        */
        SetXieData( old, new );

    /*
    **	Update the render callback.
    UpdateCallback(old,&RenderCB_(old),new,&RenderCB_(new),IdsNrenderCallback);
    */
    /*
    **  Always return FALSE -- renderImageWidgetMotifClass has no window to redraw.
    */
#ifdef TRACE
printf( "Leaving Routine SetValues in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    return( FALSE );
}

/*****************************************************************************
**  Help
**
**  FUNCTIONAL DESCRIPTION:
**
**      IDS help routine, just callback the application: XmCR_HELP.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget 
**	event	- X event structure
**
*****************************************************************************/
static void Help( rw, event )
    Widget  rw;
    XEvent *event;
{
    static XmAnyCallbackStruct help;

#ifdef TRACE
printf( "Entering Routine Help in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    help.reason = XmCR_HELP;
    help.event  = event;

    XtCallCallbacks( rw, XmNhelpCallback, &help );
#ifdef TRACE
printf( "Leaving Routine Help in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  UpdateCallback
**
**  FUNCTIONAL DESCRIPTION:
**
**	Called from SetValues: if a new callback has been specified,
**				  remove/deallocate old callback and init new.
**
**  FORMAL PARAMETERS:
**
**    old	- the old widget (a copy)
**    old_list	- the old callback list
**    new	- the new widget (the real widget)
**    new_list	- the new callback list
**    name	- the callback to remove and update
**
*****************************************************************************/
static void UpdateCallback( old, old_list, new, new_list, name)
    Widget	      old;
    XtCallbackList    old_list;
    Widget	      new;
    XtCallbackList    new_list;
    char	     *name;
{
    XtCallbackList tmp_list;

#ifdef TRACE
printf( "Entering Routine UpdateCallback in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( new_list->callback != old_list->callback )
	{
	/*
	** A new callback has been specified, save the new list, and temporarily
	** replace it with the old, so we can officially remove the old list.
	*/
	tmp_list  = (XtCallbackList)new_list->callback;
        *new_list = *old_list;
	XtRemoveAllCallbacks( new, name );
	new_list->callback = NULL;

	/*
	** Now officially install the new list.
	*/
	XtAddCallbacks( new, name, tmp_list );
    }
#ifdef TRACE
printf( "Leaving Routine UpdateCallback in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  RenderImage
**
**  FUNCTIONAL DESCRIPTION:
**
**	o   Run the Proposed rendering thru the IDS model and compare the 
**		resulting parameters with the Current rendering (if any).
**	o   If anything has changed, notify the application with the Proposed 
**	    rendering parameters.  During the callback the application may:
**	    o	abort the proposed rendering (RenderMode = Ids_Abort),
**	    o	modify any resource using the XtSetValues interface,
**		    (this causes re-execution of the IDS model)
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget
**	event	- XEvent that triggered this call
**
**  FUNCTION VALUE:
**
**	Boolean	- TRUE = created OR deleted a rendering.
**
*****************************************************************************/
static Boolean RenderImage( rw, event )
    Widget  rw;
    XEvent *event;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext           ctx = &Context_(rw);
    IdsRenderCallback 	    rcb   = PropRnd_(ctx);
    RenderContextXie        xiectx  = &XieContext_(rw);
    IdsRenderCallbackStruct cbs;
    DataForXie       xiedat = PriXie_(xiectx);

#ifdef TRACE
printf( "Entering Routine RenderImage in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
#ifdef VMS
    ChfEstablish( &Error_condition_handler );
#else
#if !(defined sparc) && !(defined __osf__) 
    ChfEstablish( Error_condition_handler );
#endif
#endif
 
    if( intCompute_(rcb) == Ids_IslClient )
    {
    do	{
	Pipe_(ctx)->pipe = Pipe_(ctx)->base;	/* make the pipe busy	    */
	pEvent_(rw) = event;			/* event that got us here   */

	if( Callback_(RenderCB_(rw)))
	    {
	    cbs = Proposed_(rw);
	    XtCallCallbacks( rw, IdsNrenderCallback, &cbs );
	    /****************************************************************
	    **  The application is in charge at this point
	    *****************************************************************/
	    }

	switch( pRender_(rw) )
	    {
	case Ids_Normal :
	case Ids_Override :
	    /*
	    **  Delete old rendering and zero "Reason" for pipe abort detection.
	    */
	    DeleteRendering(rw);
	    pReason_(rw) = 0;
	    /*
	    **  Compile and execute a new rendering/export pipe.
	    */
	    RFid_(rw) = IdsExecutePipe(
			    IdsCompileExport( ctx, PDfid_( IdsCompileRendering
					( ctx, &pFid_(rw), TRUE )), FALSE ));

	    if( Pipe_(ctx)->pipe == NULL )
		{
		RFid_(rw) = PostRendering( rw, RFid_(rw) );
		Current_(rw) = Proposed_(rw); /* save current render */
		}
	    else
		UnwindPipe( rw, (caddr_t)RFid_(rw) );  /* the pipe was aborted    */
	    break;

	case Ids_Purge :
	    /*
	    **  Delete current rendering and colormap resources.
	    */
	    DeleteRendering(rw);
	    DeallocateColors(rw);

	case Ids_Passive :
	case Ids_Abort :
	default :
	    Pipe_(ctx)->pipe = NULL;
	    }
	/*
	**  Free item lists.
	*/
	IdsFreePutList( &RqLst_(ctx) );
	IdsFreePutList( &TsLst_(ctx) );
	IdsFreePutList( &DiLst_(ctx) );
	}
    while( Pipe_(ctx)->pipe != NULL );	/* if pipe was aborted, try again   */
    }

    else if( intCompute_(rcb) == Ids_XieServer )
    {
    /*
    ** Since the export to pixmap is placed in the server pipe the info of
    ** GC has to be obtained well before and passed over toRenderContextXie.
    ** Polarity judgement is made later in the pipe build. In Case of xie
    ** the GC has to be figured before the compilation of server pipe.
    */
    if( xiedat != 0 )
	SetGCForXie(rw, xiedat);
    CForePix_(xiedat)  =  ForePix_(rw);
    CBackPix_(xiedat)  =  BackPix_(rw);
/*
    do	{
*/
	pEvent_(rw) = event;			/* event that got us here   */
	Pipe_(ctx)->pipe = NULL;
	if( Callback_(RenderCB_(rw)))
	    {
	    cbs = Proposed_(rw);
	    XtCallCallbacks( rw, IdsNrenderCallback, &cbs );
	    /****************************************************************
	    **  The application is in charge at this point
	    *****************************************************************/
	    }

	switch( pRender_(rw) )
	    {
	case Ids_Normal :
	case Ids_Override :
	    /*
	    **  Delete old rendering and zero "Reason" for pipe abort detection.
	    */
	    DeleteRendering(rw);
	    pReason_(rw) = 0;

            /*
            ** Create a server empty photo, only for the new image.
            */
	    if( !RRend_(xiectx) )
		    CreateEmptyPhoto( rw );
	    /*
            ** An extra expose event( work notify box) brings execution here
            ** when no rerendition or an new image is asked for and so the
            ** photoflo is Zero. Hack for that is to check for the photoflo
            ** not to be zero. So in Build XiePipe module The PipeDone macros
            ** if commented out would not genereate an extra expose event
            */

	    /*
	    **  Compile and execute a new rendering/export pipe.
	    */
	    RPhoto_(rw) =  (unsigned long) IdsCompileServerPipe( ctx, xiectx );
	    /*
	    **  Copy the GC's actually used in pipe into the widget struct
	    */
	    PostRenderingXie( rw ); 
	    Current_(rw) = Proposed_(rw);   /* save current rendering   */
	    break;

	case Ids_Purge :
	    /*
	    **  Delete current rendering and colormap resources.
	    */
	    DeleteRendering(rw);
	    DeallocateColors(rw);
	    break;
	case Ids_Abort :
            XClearArea( Dpy_(ctx), Win_(ctx), 0, 0, WinW_(ctx), WinH_(ctx), 0 );
	case Ids_Passive :
	default :
	    Pipe_(ctx)->pipe = NULL;
	    }
/*
	}
    while( Pipe_(ctx)->pipe != NULL );	
*/
    }
    /*
    **  Tell the caller whether or not we did something.
    */
#ifdef TRACE
printf( "Leaving Routine RenderImage in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    return IfReplace_( pRender_(rw) != Ids_Passive, pRender_(rw), Ids_Passive );

}

/*****************************************************************************
**  SetRenderContext
**
**  FUNCTIONAL DESCRIPTION:
**
**      Load dynamic rendering parameters into image widget context block.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget
**
**  FUNCTION VALUE:
**
**	returns address of RenderContext struct
**
*****************************************************************************/
static RenderContext SetRenderContext( iw )
    Widget  iw;
{
    DefineWidgetParts_(Image,iw);
    RenderContext ctx = &Context_(iw);
    IdsRenderCallback rcb = PropRnd_(ctx);
    XieImage          img;
    Visual *visual;

#ifdef TRACE
printf( "Entering Routine SetRenderContext in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
/*
** Set the rendering class every time in the ctx or bitonal with scale will 
** fail
*/
	visual = DefaultVisualOfScreen( XtScreen(iw) );

	/*
        **  Convert visual class into a rendered image class we can support.
	**  This is to limited display on bitonal workstations.
	*/
	if (visual->class == StaticGray)
	    RClass_(ctx) = Ids_GrayScale;

    /*
    **	Load dynamic widget resources.
    */
    WinW_(ctx)  = XtWidth(iw);			/* current window width	    */
    WinH_(ctx)  = XtHeight(iw);			/* current window height    */
    Cmap_(ctx)  = Colormap_(iw);		/* current widget colormap  */

    /*
    ** Default protocol for IDS connected to XIE server is Pixmap
    */
    if(  rcb != 0 && intCompute_(rcb) == Ids_XieServer )
	{
        switch( pProto_(iw) )
	    {
	case Ids_XImage :
	case Ids_Pixmap :
	    pProto_(iw) = Ids_Pixmap;	    
	    break;
	case Ids_Photomap:
           pProto_(iw) = Ids_Photomap;
           break;
	case Ids_Window:
           pProto_(iw) = Ids_Window;
           break;
	default :
	    XtWarningMsg("InvPlpTyp","SetRenderContext","IdsImageError",
		   "invalid image presentation level protocol",0,0);
	    pProto_(iw) = Ids_Pixmap;	    
	    break;
	    }
	}
    else    /* Client ISL */
	{
	/*
	** Set protocol as requested, or choose the default according to class.
	*/
	pProto_(iw) = Proto_(ctx) != 0
                ? Proto_(ctx) : XtIsSubclass(iw, (WidgetClass)pannedImageWidgetMotifClass)
                ? Ids_Pixmap  : Ids_XImage;

	switch( pProto_(iw) )
	    {
	case Ids_XImage :
        case Ids_Pixmap :
	    break;
       case Ids_Photomap  :
            pProto_(iw) = Ids_Pixmap ;
            break;
	default :
	    XtWarningMsg("InvPlpTyp","SetRenderContext","IdsImageError",
		   "invalid image presentation level protocol",0,0);
	    pProto_(iw) = Ids_XImage;
	    break;
	    }
	}
    /*
    **  Set Fit width and height.
    */
    pWide_(iw) = FitW_(ctx) != 0 ? FitW_(ctx) : WrkW_(iw);
    pHigh_(iw) = FitH_(ctx) != 0 ? FitH_(ctx) : WrkH_(iw);

#ifdef TRACE
printf( "Leaving Routine SetRenderContext in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    return( ctx );
}

/*****************************************************************************
**  CopyFid
**
**  FUNCTIONAL DESCRIPTION:
**
**      When IdsNcopyFid is set, copy new image frames immediately.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget
**	old_fid	- previous fid -- to be deleted
**
*****************************************************************************/
static void CopyFid( rw, old_fid )
    Widget        rw;
    unsigned long old_fid;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext ctx  = &Context_(rw);
    struct PUT_ITMLST *itmlst = NULL;
    int ctype, stride, width;

#ifdef TRACE
printf( "Entering Routine CopyFid in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( pFid_(rw) != 0 )
	{
	/*
	**  Determine if the image needs to be decompressed first.
	*/
	GetIsl_( pFid_(rw), Img_CompressionType, ctype, 0 );
	if( ctype != ImgK_PcmCompression )
	    ImgDecompress( pFid_(rw), ImgM_InPlace, 0 );
	/*
	**  Get image width and pixel stride, then set scanline stride.
	*/
	GetIsl_( pFid_(rw), Img_PixelsPerLine, width,  0 );
	GetIsl_( pFid_(rw), Img_PixelStride,	  stride, 0 );
	stride *= width;
	stride += AlignBits_(stride, Pad_(ctx));
	IdsPutItem(&itmlst, Img_ScanlineStride, stride, 0 );
	/*
	**  Now copy the image -- while padding scanlines.  It would be nice
	**  if we could extract the ROI here as well -- but that complicates
	**  IdsGetCoordinates.  It's OK that we overwrite the frame-id, the 
	**  application is responcible for that Fid (trust me).
	*/
	pFid_(rw) = ImgCopy( pFid_(rw), 0, itmlst );
	IdsFreePutList( &itmlst );
	}
    if( old_fid != 0 )
	/*
	**  Delete our previous private copy of the source fid.
	*/
	ImgDeleteFrame( old_fid );
#ifdef TRACE
printf( "Leaving Routine CopyFid in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  CopyColors
**
**  FUNCTIONAL DESCRIPTION:
**
**  If "IdsNcolormapMode" is Ids_PrivateColors & IdsNcolormapUpdate is TRUE 
**  load the appl given private colors into IDS internally
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget
**	old_fid	- previous fid -- to be deleted
**
*****************************************************************************/
static void CopyColors( rw )
    Widget        rw;
{
    unsigned long  i;

    DefineWidgetParts_(RenderImage,rw);
    RenderContext ctx  = &Context_(rw);

#ifdef TRACE
printf( "Entering Routine CopyColors in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    **  Allocate the colormap state list.
    */
    ClrMap_(ctx) = (XColor *)        XtMalloc( PltCnt_(ctx)  * sizeof(XColor));
    /*
    **  Read the entire colormap.
    */
    for( i = 0; i < PltCnt_(ctx); ++i) 
                    ClrMap_(ctx)[i].pixel = PltLst_(ctx)[i];
    XQueryColors( XtDisplay(rw), Colormap_(rw), ClrMap_(ctx), PltCnt_(ctx) );
#ifdef TRACE
printf( "Leaving Routine CopyColors in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*******************************************************************************
**  PostRendering
**
**  FUNCTIONAL DESCRIPTION:
**
**	Do post rendering chores prior to image display, such as Pixmap create.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget
**	fid	- rendered fid
**
**  FUNCTION VALUE:
**
**	returns fid passed in for XImage protocol, NULL for Pixmap protocol.
**
*******************************************************************************/
static unsigned long PostRendering( rw, fid )
 Widget		rw;
 unsigned long	fid;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext ctx = &Context_(rw);
    GC     pixmapGC;
    XGCValues   gcv;
    unsigned long polarity, ret_fid = fid;

#ifdef TRACE
printf( "Entering Routine PostRendering in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    **  Choose a GC according to brightness polarity and image depth.
    */
    GetIsl_( fid, Img_BrtPolarity, polarity, 0 );
    SetImageGC(rw, polarity);

    /*
    **  For Ids_Pixmap protocol, create a server pixmap for our image.
    */
    if( pProto_(rw) == Ids_Pixmap )
	{
	Pix_(rw) = XCreatePixmap( XtDisplay(rw), XtWindow(rw),
				  Iwide_(rw), Ihigh_(rw), Ideep_(rw));
	if( Pix_(rw) != NULL )
	    {
	    /*
	    **  Load pixmap, then release temporary XImage resources.
	    */
	    if( Ifrmt_(rw) == ZPixmap )
		XPutImage( XtDisplay(rw), Pix_(rw), GC_(rw),  &Img_(rw),
				     0, 0, 0, 0, Iwide_(rw), Ihigh_(rw));
	    else
		{
		Ifrmt_(rw) =  XYPixmap; /* format = single plane pixmap */
		/*
		**  We need a temporary GC for loading the pixmap; can't
		**  use one of our standard GCs because the drawable,
		**  Pix_(rw), may not be of the same depth as XtWindow(rw).
		gcv.foreground = ForePix_(rw);
		gcv.background = BackPix_(rw);
		pixmapGC = XCreateGC( XtDisplay(rw), Pix_(rw),
				      GCForeground | GCBackground, &gcv);
		*/
		pixmapGC = XCreateGC( XtDisplay(rw), Pix_(rw), 0, &gcv);
		XPutImage( XtDisplay(rw), Pix_(rw), pixmapGC, &Img_(rw),
				     0, 0, 0, 0, Iwide_(rw), Ihigh_(rw));
		XFreeGC( XtDisplay(rw), pixmapGC );
		}
	    if( fid != pFid_(rw) )
		ImgDeleteFrame( fid );
	    Idata_(rw) = NULL;
	    ret_fid    = 0;
	    }
	else
	    XtWarningMsg("PixAllErr","PostRendering","IdsImageError",
			 "can't allocate Pixmap, using XImage instead",0,0);
	}

#ifdef TRACE
printf( "Leaving Routine PostRendering in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    return( ret_fid );
}

/*******************************************************************************
**  PostRenderingXie
**
**  FUNCTIONAL DESCRIPTION:
**
**	Do post rendering chores prior to image display, such as Pixmap create.
**      for the XIE.
**
**  FORMAL PARAMETERS:
**
**	rw	        - Render Image Widget
**	photoflo	- rendered photo
**
**  FUNCTION VALUE:
**
**	returns photo passed in for XImage protocol, NULL for Pixmap protocol.
**
*******************************************************************************/
static void  PostRenderingXie( rw )
 Widget		rw;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext    ctx    = &Context_(rw);
    RenderContextXie xiectx = &XieContext_(rw);
    IdsRenderCallback rcb   = PropRnd_(ctx);
    DataForXie       xiedat = PriXie_(xiectx);
    XGCValues gcv;

#ifdef TRACE
printf( "Entering Routine PostRenderingXie in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    ** Copy back GC value and pixmap from xiectx struct to the widget struct
    */
    Pix_(rw) = CPix_(xiedat);
    CPix_(xiedat) = 0;	    
    GC_(rw)  = CGC_(xiedat);
/*
    if( iProto_(rcb) == Ids_Pixmap )
	{
	gcv.foreground = ForePix_(rw);
	gcv.background = BackPix_(rw);
	GC_(rw) = XtGetGC (rw, ( GCForeground | GCBackground ), &gcv);
	}
    else
	GC_(rw)  = CGC_(xiedat);    
*/
#ifdef TRACE
printf( "Leaving Routine PostRenderingXie in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  SetImageGC
**
**  FUNCTIONAL DESCRIPTION:
**
**      Setup image GC according to image depth and brightness polarity.
**
**  FORMAL PARAMETERS:
**
**	rw	    - Render Image Widget 
**	polarity    - selects StdGC_() or RevGC_() for bitonal images.
**
*****************************************************************************/
static void SetImageGC( rw, polarity )
    Widget  rw;
    int	    polarity;
{
    DefineWidgetParts_(RenderImage,rw);
    XGCValues gcv;

#ifdef TRACE
printf( "Entering Routine SetImageGC in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( XtIsRealized(rw) )
	if( Ideep_(rw) > 1 )
	    /*
	    **  Use common GC, image is continuous tone (or doesn't exist).
	    */
            {
	    gcv.foreground = ForePix_(rw);
	    gcv.background = BackPix_(rw);
	    GC_(rw) = XtGetGC (rw, ( GCForeground | GCBackground ), &gcv);
            }
	else
	    {
	    /*
	    **  Image is bitonal.
	    */
	    if( StdGC_(rw) == NULL )
		{
		gcv.foreground = ForePix_(rw);
		gcv.background = BackPix_(rw);
		gcv.plane_mask = gcv.foreground ^ gcv.background;
		StdGC_(rw) = XtGetGC( rw, GCForeground | GCPlaneMask
					| GCBackground,  &gcv );
		gcv.foreground = BackPix_(rw);
		gcv.background = ForePix_(rw);
		RevGC_(rw) = XtGetGC( rw, GCForeground | GCPlaneMask
					| GCBackground,  &gcv );
		}
	    /*
	    **  Choose GC to use with bitonal image.
	    */
	    GC_(rw) = polarity == ImgK_ZeroMaxIntensity ? StdGC_(rw)
							   : RevGC_(rw);
	    }
#ifdef TRACE
printf( "Leaving Routine SetImageGC in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  SetGCForXie
**
**  FUNCTIONAL DESCRIPTION:
**
**      Setup image GC according to image depth and brightness polarity.
**
**  FORMAL PARAMETERS:
**
**	rw	    - Render Image Widget 
**	polarity    - selects StdGC_() or RevGC_() for bitonal images.
**
*****************************************************************************/
static void SetGCForXie( rw, xiedat )
    Widget  rw;
    DataForXie   xiedat;
{
    DefineWidgetParts_(RenderImage,rw);
    XGCValues gcv;

#ifdef TRACE
printf( "Entering Routine SetGCForXie in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( XtIsRealized(rw) )
	{
        /*
        ** When rendered image is continous use the Common GC in the pipe
        */
        gcv.foreground = ForePix_(rw);
        gcv.background = BackPix_(rw);
        CGC_(xiedat) = XtGetGC (rw, ( GCForeground | GCBackground ), &gcv);
 	/*
        ** When rendered image is bitonal use back or foreground GC in the pipe
	*/
	if( StdGC_(rw) == NULL )
	    {
	    gcv.foreground  = ForePix_(rw);
	    gcv.background  = BackPix_(rw);
	    gcv.plane_mask  = gcv.foreground ^ gcv.background;
	    CStdGC_(xiedat) = StdGC_(rw) = XtGetGC( rw, GCForeground 
					| GCPlaneMask | GCBackground,  &gcv );
	    gcv.foreground  = BackPix_(rw);
	    gcv.background  = ForePix_(rw);
	    CRevGC_(xiedat) = RevGC_(rw) = XtGetGC( rw, GCForeground 
					| GCPlaneMask | GCBackground,  &gcv );
	    }
	else
	    {
	    CStdGC_(xiedat) = StdGC_(rw);	 
	    CRevGC_(xiedat) = RevGC_(rw);
	    }
	}
#ifdef TRACE
printf( "Leaving Routine SetGCForXie in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  DestroyImageGCs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Delete private image GC's
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget 
**
*****************************************************************************/
static void DestroyImageGCs( rw )
    Widget  rw;
{
    DefineWidgetParts_(RenderImage,rw);

#ifdef TRACE
printf( "Entering Routine DestroyImageGCs in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( GC_(rw) == StdGC_(rw) || GC_(rw) == RevGC_(rw) )
	GC_(rw)  = NULL;

    if( StdGC_(rw) != NULL )
	XtDestroyGC(StdGC_(rw));

    if( RevGC_(rw) != NULL )
	XtDestroyGC(RevGC_(rw));

    StdGC_(rw) = NULL;
    RevGC_(rw) = NULL;
#ifdef TRACE
printf( "Leaving Routine DestroyImageGCs in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*******************************************************************************
**  DeleteRendering
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deletes resources acquired during a previous rendering.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget 
**
*******************************************************************************/
static void   DeleteRendering(rw)
Widget rw;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext     ctx     = &Context_(rw);
    RenderContextXie  xiectx  = &XieContext_(rw);
    IdsRenderCallback rcb     = PropRnd_(ctx);
    unsigned long     *list, count;
    char               cmp_cnt;
    DataForXie         xiedat;
    xiedat = PriXie_(xiectx);
 
#ifdef TRACE
printf( "Entering Routine DeleteRendering in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( Pix_(rw) != NULL )
	{
	XFreePixmap( XtDisplay(rw), Pix_(rw) );
/*   printf("\n Protocol -- Pixmap Freed old Pixmap  DeleteRendering\n" );*/
        }
    Pix_(rw)   = NULL;
    Idata_(rw) = NULL;
    Iwide_(rw) = 0;
    Ihigh_(rw) = 0;
    Ideep_(rw) = 0;

    /*
    ** Clean up the photomap relevant stuff
    */
    if( intCompute_(rcb) == Ids_XieServer  )
	DeleteRenderingFlo( rw );
    /*
    ** Clean up the fid and its relevant stuff
    */
    else if(intCompute_(rcb) == Ids_IslClient )
	DeleteRenderingFid( rw );

    if( Switch_(ctx) )
	{
       Switch_(ctx) = False;
       if( intCompute_(rcb) == Ids_IslClient )
	    {
            XClearArea( Dpy_(ctx), Win_(ctx), 0, 0, WinW_(ctx),
                                                        WinH_(ctx), 0 );
	    DeleteRenderingFlo( rw );
	    cXieImg_(rw) = pXieImg_(rw) = 0;
	    }
	else if ((intCompute_(rcb) == Ids_XieServer ) &&
                ((!(VxtImagingOption(Dpy_(ctx), ctx)) && (PxlStr_(ctx) == 1)) ||
                 ((xiedat != 0) && (Iroih_(xiedat) == 0) && 
			(Iroiw_(xiedat) == 0)))
                || ((iZXSc_(rcb) != 0.0) || (iZYSc_(rcb) != 0.0)) )

	    {
            XClearArea( Dpy_(ctx), Win_(ctx), 0, 0, WinW_(ctx),
                                                        WinH_(ctx), 0 );
	    DeleteRenderingFid( rw );
	    pFid_(rw) = cFid_(rw) = 0;
	    }
	}
#ifdef TRACE
printf( "Leaving Routine DeleteRendering in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*******************************************************************************
**  DeleteRenderingFlo
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deletes resources acquired during a previous rendering.
**      of Photomap with XIE.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget 
**
*******************************************************************************/
static void   DeleteRenderingFlo(rw)
Widget rw;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext     ctx     = &Context_(rw);
    RenderContextXie  xiectx  = &XieContext_(rw);
    IdsRenderCallback rcb     = PropRnd_(ctx);
 
#ifdef TRACE
printf( "Entering Routine DeleteRenderingFlo in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    DeallocateColors(rw);
    /*
    ** Free  the rendered photomap
    */
    if( DPhoto_(xiectx) != 0 )
	{
	/*
	** Free the color pixel lut photo
	*/
	if( LPhoto_(xiectx) != 0 )
	    {
	    LPhoto_(xiectx) = XieFreeResource( LPhoto_(xiectx) );
/* printf("\n Protocol --Photomap Freed Lut  Photomap DeleteRendering\n");*/
	    }
	/*
	** Free the tapped photomap
	*/
	if( DPhoto_(xiectx) == TPhoto_(xiectx) )
	    DPhoto_(xiectx) = 0;
	else
	    DPhoto_(xiectx) = XieFreeResource( DPhoto_(xiectx) );
/* printf("\n Protocol --Photomap Freed Display Photoflo DeleteRendering\n");*/
	}

    /*
    ** Free the past raw photmap, a new image is to be rendered
    */
    if( cXieImg_(rw) != pXieImg_(rw) && TPhoto_(xiectx) != 0 )
	{
	TPhoto_(xiectx) = XieFreeResource( TPhoto_(xiectx) );
/*	   printf("\n Freed Raw Photoflo DeleteRendering\n"); */
	}
#ifdef TRACE
printf( "Leaving Routine DeleteRenderingFlo in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*******************************************************************************
**  DeleteRenderingFid
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deletes resources acquired during a previous rendering.
**      of Fid at ISL
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget 
**
*******************************************************************************/
static void   DeleteRenderingFid(rw)
Widget rw;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext     ctx     = &Context_(rw);
    RenderContextXie  xiectx  = &XieContext_(rw);
    IdsRenderCallback rcb     = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine DeleteRenderingFid in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if(RFid_(rw) != 0 && RFid_(rw) != pFid_(rw) && RFid_(rw) != cFid_(rw))
	{
	ImgDeleteFrame( RFid_(rw) );
	RFid_(rw)  = 0;
	cFid_(rw)  = 0;
	}
    if( PxlDat_(ctx) != NULL )
	{
	XtFree( PxlDat_(ctx) );
	PxlDat_(ctx) = NULL;
	}
#ifdef TRACE
printf( "Leaving Routine DeleteRenderingFid in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}
/*******************************************************************************
**  DeallocateColors
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates colors acquired during a previous rendering.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget 
**
*******************************************************************************/
static void   DeallocateColors(rw)
Widget rw;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext ctx = &Context_(rw);
    IdsRenderCallback rcb   = PropRnd_(ctx);

    RenderContextXie  xiectx  = &XieContext_(rw);
    DataForXie         xiedat;
    xiedat = PriXie_(xiectx);

#ifdef TRACE
printf( "Entering Routine DeallocateColors in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    **	Free the colormap entries we own.
    */
    if ((PxlLst_(ctx) != 0 ) &&
	(CmpMode_(ctx) != Ids_PrivateColors) &&
	(Vis_(ctx)->class != StaticColor) &&
	(Vis_(ctx)->class != StaticGray))
	XFreeColors( Dpy_(ctx), Cmap_(ctx), PxlLst_(ctx), PxlCnt_(ctx), 0 );

   if( intCompute_(rcb) == Ids_XieServer )
        {
	if( PxlLst_(ctx) != 0 )
	    PxlLst_(ctx) = XieFree( PxlLst_(ctx) );
        /*
        **  Clear the window so the old image doesn't go technicolor.
        */
        if ((xiedat == 0) || ((Iroiw_(xiedat) == 0) && (Iroih_(xiedat) == 0)))
            XClearArea( Dpy_(ctx), Win_(ctx), 0, 0, WinW_(ctx), WinH_(ctx), 0 );
        }
    else
        MemFree_( &PxlLst_(ctx) );
        
    PxlCnt_(ctx) = 0;

    /* 
    ** Free the context if there is a Photo_(xiectx).
    ** This fixes a bug where the colors are freed and XIE tries
    ** to use the colormap anyway. Freeing the export context causes
    ** XIE to allocate new colors.
    */

    if (( intCompute_(rcb) == Ids_XieServer ) && (Photo_(xiectx) != 0)
        && ( RRend_(xiectx) ))
        XieFreeExport ( Photo_(xiectx) );
#ifdef TRACE
printf( "Leaving Routine DeallocateColors in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  UnwindPipe
**
**  FUNCTIONAL DESCRIPTION:
**
**	Deallocate resources belonging to IDS pipe fuctions which were 
**	abandoned by aborting the pipe mid-stream.
**
**  FORMAL PARAMETERS:
**
**	rw	- Render Image Widget
**	last	- last result returned from pipe.
**
*****************************************************************************/
static void UnwindPipe( rw, last )
 Widget  rw;
 caddr_t last;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext ctx = &Context_(rw);
    IdsPipeDesc	   pd = Pipe_(ctx);
    IdsPipeElement pe = PDpipe_(pd);

#ifdef TRACE
printf( "Entering Routine UnwindPipe in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    switch( PEcid_(pe) & ~Ids_FunctionROI )
	{
    case Ids_FunctionAllocColor :
	/*
	**  Free the IdsHistogramData.
	*/
	_ImgCfree( ((IdsHistogramData)last)->pointer );
	_ImgCfree( last );
	RFid_(rw) = *PDfid_(pd);
	break;

    case Ids_FunctionRemapColor :
	/*
	**  Free the RemapLut.
	*/
	_ImgCfree( ((RemapLut)last)->base );
	_ImgCfree( last );
	RFid_(rw) = *PDfid_(pd);
	break;
	}
    /*
    **	Deallocate the remaining rendering and colormap resources.
    */
    DeleteRendering(rw);
    DeallocateColors(rw);
#ifdef TRACE
printf( "Leaving Routine UnwindPipe in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*******************************************************************************
**  PipeNotify
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called from the IDS pipe to notify of pending fucntion.
**
**  FORMAL PARAMETERS:
**
**	pd	- pipe descriptor.
**	id	- identifier for next function to be executed.
**	rw	- Render Image Widget - passed as private data.
**
**  FUNCTION VALUE:
**
**	Boolean	- true if pipe execution should be aborted.
**
*******************************************************************************/
static Boolean PipeNotify( pd, id, rw )
 IdsPipeDesc	pd;
 unsigned long	id;
 Widget		rw;
{
    DefineWidgetParts_(RenderImage,rw);
    static WorkNotifyStruct work;
    static IdsSaveImageCallbackStruct save;
    RenderContext ctx = &Context_(rw);
    RenderContextXie  xiectx  = &XieContext_(rw);
    IdsRenderCallback rcb   = PropRnd_(ctx);
    DataForXie       xiedat = PriXie_(xiectx);
    unsigned long    photo, fid,  xieimage = 0;

#ifdef TRACE
printf( "Entering Routine PipeNotify in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( Callback_( SaveCB_(rw) ))
	{
	save.reason      = IdsCRSaveImage;
	save.event       = pEvent_(rw);
	if( intCompute_(rcb) == Ids_XieServer )
	    {
            photo = (unsigned long) SPhoto_(xiectx);
	    SaveRenderedXieData( ctx, (unsigned long)SPhoto_(xiectx), xieimage, fid );
            save.xieimage = xieimage;
            save.photo    = photo;
            save.fid      = fid;
	    XtCallCallbacks( rw, IdsNsaveImageCallback, &save );
	    }
        else
	    {	
	    fid = SaveRenderedIslFid( ctx, id, *pd->rfid );
	    save.fid      = fid;
	    save.photo    = 0;
	    save.xieimage = 0;
	    if ( fid != 0 )
		XtCallCallbacks( rw, IdsNsaveImageCallback, &save );
	    }
	}

    if(( Callback_( WorkCB_(rw) )) && (!VxtImagingOption(Dpy_(ctx), ctx)) )
	{
	work.reason    = IdsCRWorkNotify;
	work.event     = pEvent_(rw);
        work.process   = intCompute_(rcb);
	work.function  = id;
	work.pipe_desc = pd;
	XtCallCallbacks( rw, IdsNworkNotifyCallback, &work );
	/****************************************************************
	**  The application is in charge at this point
	*****************************************************************/
	}
    /*
    **	Abort pipe if Reason changes -- IdsXmCompareRenderings will generate
    **	a new rendering callback reason if SetValues is called.
    */
#ifdef TRACE
printf( "Leaving Routine PipeNotify in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    return( pReason_(rw) != 0 );
}

/*******************************************************************************
**  XiePipeNotify
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called from the IDS Build pipe 
**      to notify that XIE computation is done.
**
**  FORMAL PARAMETERS:
**
**	pd	- pipe descriptor.
**	id	- identifier for next function to be executed.
**	rw	- Render Image Widget - passed as private data.
**
**  FUNCTION VALUE:
**
**	Boolean	- true if pipe execution should be aborted.
**
*******************************************************************************/
static void XiePipeNotify( pd, id, rw )
 IdsPipeDesc	pd;
 unsigned long	id;
 Widget		rw;
{
    DefineWidgetParts_(RenderImage,rw) ;
    static WorkNotifyStruct work;
    static IdsSaveImageCallbackStruct save;
    RenderContext    ctx    = &Context_(rw);
    RenderContextXie xiectx = &XieContext_(rw);
    IdsRenderCallback rcb   = PropRnd_(ctx);
    DataForXie xiedat = PriXie_(xiectx);

#ifdef TRACE
printf( "Entering Routine XiePipeNotify in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if(( Callback_( WorkCB_(rw) )) && (!VxtImagingOption(Dpy_(ctx), ctx)) )
	{
	work.reason    = IdsCRWorkNotify;
	work.event     = pEvent_(rw);
	work.process   = intCompute_(rcb);
	work.function  = id;
	work.pipe_desc = pd;
	XtCallCallbacks( rw, IdsNworkNotifyCallback, &work );
	/****************************************************************
	**  The application is in charge at this point
	*****************************************************************/
	}
#ifdef TRACE
printf( "Leaving Routine XiePipeNotify in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*******************************************************************************
**  ErrorCallback
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called from the IDS pipe to display error messages.
**
**  FORMAL PARAMETERS:
**
**      rw      - Render Image Widget - passed as private data.
**      code    - Error code passed to application as an int.
**
**  FUNCTION VALUE:
**
**      Boolean - true if pipe execution should be aborted.
**
*******************************************************************************/
static Boolean ErrorCallback(pd,code)
char *pd;
int    code;
{
    Widget rw = (Widget)pd;
    DefineWidgetParts_(RenderImage,rw);
    static IdsErrorCallbackStruct ecs;

#ifdef TRACE
printf( "Entering Routine ErrorCallback in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
        if( Callback_(ErrorCB_(rw)))
        {
            ecs.reason    = IdsCRError;
            ecs.event     = pEvent_(rw);
            ecs.code      = code;
        }
        XtCallCallbacks( rw, IdsNerrorCallback, &ecs );
#ifdef TRACE
printf( "Leaving Routine ErrorCallback in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    return( pReason_(rw) != 0 );
}


/*****************************************************************************
**  CallErrorCallback
**
**  FUNCTIONAL DESCRIPTION:
**
**  Condition handler to trap and funnel ISL errors through IDS and back to
**  the application via IDS widget callback.
**
**  FORMAL PARAMETERS:
**
**      code    - Error code passed to application as an int.
**
**  FUNCTION VALUE:
**
**      None
**
******************************************************************************/
static void CallErrorCallback(code)
int              code;
{
#ifdef TRACE
printf( "Entering Routine CallErrorCallback in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    /*
    ** IdsErrorCb is global to this module and is defined as ErrorCallbackFunc
    ** It holds a pointer to the render context widget.
    */

    ErrorCallback(ECwidget_(IdsErrorCb), code);
#ifdef TRACE
printf( "Leaving Routine CallErrorCallback in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
return;
}

/*****************************************************************************
**  InitializeForXie
**
**  FUNCTIONAL DESCRIPTION:
**
**      Connect to use server XIE for image processing( crunching ).
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget
**
**  FUNCTION VALUE:
**
**	returns void
**
*****************************************************************************/
static void  InitializeForXie( rw )
    Widget  rw;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext    ctx    = &Context_(rw);
    RenderContextXie xiectx = &XieContext_(rw);
    char   xie_present = FALSE;
    char **xtensions, *str;
    int    numxtensions, i;
    int    opcode, event0, error0, function;
    IdsFuncListCallbackStruct cbs;

#ifdef TRACE
printf( "Entering Routine InitializeForXie in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
#ifdef __alpha

    Process_(ctx) = Ids_IslClient;

#else
    /*
    ** List the extensions supported by the server
    */
    xtensions = XListExtensions( XtDisplay(rw), &numxtensions );
    for ( i = 0; i < numxtensions; ++i )
        if(  strncmp( xtensions[i], "Xie", 3 ) == 0 )
	    {
            xie_present = TRUE;
	    break;
	    }
    /*
    ** Connect to Xie if it exists. The extensions is loaded and initialized
    */
    if( xie_present &&
        XQueryExtension( XtDisplay(rw), XieS_Name, &opcode, &event0, &error0 ) )
	{
	    Process_(ctx) = Ids_XieServer;
	    if( Callback_(XieListCB_(rw)))
		{
		/*
		**  Get a list of all the Xie proto funct supported by display.
		*/
		XieFunc_(xiectx) = XieListFunctions( XtDisplay(rw) );
		cbs.reason    =  IdsCRXieList;
		cbs.event     =  pEvent_(rw);
		cbs.names     =  XieFunc_(xiectx);
		XtCallCallbacks( rw, IdsNxieListCallback, &cbs );
		/*************************************************************
		**  The application is handed with the available xie functi **
		*************************************************************/
		}
	    /*    
	    **  Enable completion events.
	    */
	    XieSelectEvents( XtDisplay(rw), XieM_ComputationEvent | 
                                     XieM_DisplayEvent  | XieM_PhotofloEvent );
	}
   else
        Process_(ctx) = Ids_IslClient;

#endif
#ifdef TRACE
printf( "Leaving Routine InitializeForXie in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**   SetXieData()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Connect to use server XIE for image processing( crunching ).
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget
**
**  FUNCTION VALUE:
**
**	returns void
**
*****************************************************************************/
static void  SetXieData( old, new )
    Widget  old, new;
{
    DefineWidgetParts_(RenderImage,old);
    DefineWidgetParts_(RenderImage,new);
    RenderContextXie Octx = &XieContext_(old);
    RenderContextXie Nctx = &XieContext_(new);

#ifdef TRACE
printf( "Entering Routine SetXieData in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if (( pFid_(old) != pFid_(new) ) || (pXieImg_(new) == 0))
	{
	/*
        ** delete the old xieimage. If CopyXieimg flag is True iamge data 
        ** is deleted and when it is false only xieimage record and UdpRec's 
	** struct are deallocated. This feature is not yet implemente.
        */
	if ( pXieImg_(old) != 0 || pXieImg_(new) != 0)
	    {
	    XieFreeImage( pXieImg_(old) );
	    pXieImg_(old) = cXieImg_(old)  = 0;
	    pXieImg_(new) = cXieImg_(new)  = 0;
/*            printf("\n New Image ready -- Freed old Xieimage SetXieData\n");*/
	    /*
	    ** Free the temp data  struct for xie used to pass for non widget modules
	    */
	    if ( PriXie_(Nctx) != 0 )
		{
                _ImgFree( PriXie_(Octx));
                PriXie_(Octx) = PriXie_(Nctx) = 0;
/*		printf("\n Freed DataForXie data structure \n");  */
		}
	    }
        /*
        ** Convert Fid to Xieimage and collect roi data
        */
        FidToXieImage( new );

	/*
	** Set the rendering flag to False so that  pipe building would make
	** data flow all through the pipe to the pixmap or to the tapped photo
	*/
	RRend_(Nctx) = RRend_(Octx) = False;
	}
        /*
        ** Collect ISL ROI  coordiantes and dimensions data
        */
        if ((PriXie_(Nctx) != 0) &&  (pROI_(new) != 0))
            CollectRoiData( PriXie_(Nctx), pROI_(new) );
#ifdef TRACE
printf( "Leaving Routine SetXieData in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**   PrivateXieData()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static 	DataForXie  PrivateXieData(rw)
    Widget  rw;
{
    DefineWidgetParts_(RenderImage,rw);
    DataForXie   xiedat;

#ifdef TRACE
printf( "Entering Routine PrivateXieData in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    ** Allocate DataForXie struct and fill up it box dimensions fields
    */
    xiedat = (DataForXie) IdsDataForXie( pFid_(rw) );

    /*
    ** Since the export to pixmap is placed in the server pipe the info of
    ** GC has to be obtained well before and passed over to RenderContextXie.
    ** Polarity judgement is made later in the pipe build.
    */
    SetGCForXie(rw, xiedat);

#ifdef TRACE
printf( "Leaving Routine PrivateXieData in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    return( (DataForXie) xiedat );
}

/*****************************************************************************
**   SaveRenderedXieData()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static 	void  SaveRenderedXieData(ctx,photo,xieimage,fid)
    RenderContext    ctx;
    unsigned long    photo, xieimage, fid;
{
#ifdef TRACE
printf( "Entering Routine SaveRenderedXieData in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    if( Save_(ctx) & Ids_SaveFid )
	{
    	if( Save_(ctx) & Ids_SaveXieimg )
	    {
	    xieimage = IdsPhotoToXieImage( (XiePhotomap)photo, Cmpress_(ctx), Cmporg_(ctx)); 
	    fid      = IdsXieImageToFid( (XieImage)xieimage );
	    }
	else
	    {
	    fid = IdsPhotoToFid( (XiePhotomap)xieimage, Cmpress_(ctx), Cmporg_(ctx) );
	    xieimage = 0;
	    }		    
	if( !(Save_(ctx) & Ids_SavePhoto) )
    	    {
	    photo   = (unsigned long) XieFreeResource( photo );
	    }
	}

    else if( Save_(ctx) & Ids_SaveXieimg )
	{
	fid = 0;
	xieimage = IdsPhotoToXieImage( (XiePhotomap)photo, Cmpress_(ctx), Cmporg_(ctx) ); 
	if( !(Save_(ctx) & Ids_SavePhoto) )
	    {
	    photo = (unsigned long) XieFreeResource( photo );
	    }
	}

    else if( Save_(ctx) & Ids_SavePhoto )
	fid = xieimage = 0;
#ifdef TRACE
printf( "Leaving Routine SaveRenderedXieData in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**   SaveRenderedIslFid()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static 	unsigned long  SaveRenderedIslFid(ctx, id, fid)
    RenderContext    ctx;
    unsigned long    id;
    unsigned long    fid;
{
unsigned long	returnValue;

#ifdef TRACE
printf( "Entering Routine SaveRenderedIslFid in module IDS_RENDER_IMAGE_MOTIF \n");
#endif

    if (( id  & ~Ids_FunctionROI ) == Ids_FunctionExportPipeTap)
	returnValue = ConvertRenderedFid( fid, Cmpress_(ctx), Cmporg_(ctx) );
    else
        returnValue = NULL;

#ifdef TRACE
printf( "Leaving Routine SaveRenderedIslFid in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    return returnValue;
}

/*****************************************************************************
**   FidToXieImage()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static void FidToXieImage( rw )
    Widget  rw;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContext ctx = &Context_(rw);
    RenderContextXie xiectx = &XieContext_(rw);

#ifdef TRACE
printf( "Entering Routine FidToXieImage in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    PipeDone_(xiectx) = (PipeDoneCallProc) XiePipeNotify;
    XieWidget_(xiectx) = (char *) rw;
    if( pFid_(rw) != 0 )
	{
	/*
	** Create an XieImage from FID
	*/
	pXieImg_(rw) = (XieImage)IdsFidToXieImage(pFid_(rw), False );
	/*
	**  Collect GC data and isl frame box coords for xie pipe use
	*/
	PriXie_(xiectx) = PrivateXieData(rw);
	}
#ifdef TRACE
printf( "Leaving Routine FidToXieImage in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**   CreateEmptyPhoto()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static void CreateEmptyPhoto( rw )
    Widget  rw;
{
    DefineWidgetParts_(RenderImage,rw);
    RenderContextXie xiectx = &XieContext_(rw);

#ifdef TRACE
printf( "Entering Routine CreateEmptyPhoto in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    ** Create a server photo
    */
    if( XtIsRealized(rw) && pXieImg_(rw) != 0 )
	{
	/*
	** Create a server photoflo with no image data
	*/
	Photo_(xiectx) = XiePutImage(XtDisplay(rw), /* display id        */
		               pXieImg_(rw),            /* XieImage          */
		               XieK_DataFlo              /* transport img data*/
			       );
	/*
	** Set the rendering flag to False so that  pipe building would make
	** data flow all through the pipe to the pixmap or to the tapped photo
	*/
	RRend_(xiectx) = False;
	}
#ifdef TRACE
printf( "Leaving Routine CreateEmptyPhoto in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  SwitchMode
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will switch from XIE or ISl and vice versa. Also Converts
**      ISL fid to native format.
**
**  FORMAL PARAMETERS:
**
**	old	- Render Image Widget, copy of original.
**	new	- Render Image Widget, as modified by superclasses.
**
**  FUNCTION VALUE:
**
**	TRUE	something changed, and not a subclass of Image.
**	FALSE	no change, or ImageClass; re-display never requested via Toolkit
**
*****************************************************************************/
static void SwitchMode( old, new )
    Widget  old, new;
{
    DefineWidgetParts_(RenderImage,old);
    DefineWidgetParts_(RenderImage,new);
    RenderContext Octx = &Context_(old);
    RenderContext Nctx = &Context_(new);
    IdsRenderCallback rcb   = PropRnd_(Nctx);
    unsigned long   fid;

#ifdef TRACE
printf( "Entering Routine SwitchMode in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
    /*
    ** If there is no XIE server 
    **		OR 
    ** if user specified private colormaps 
    ** then set compute mode to be 
    **	    client ISL
    */
    if ( (pCompute_(new) == Ids_XieServer && Process_(Nctx) != Ids_XieServer) ||
         (CmpMode_(Nctx) == Ids_PrivateColors && CmpUpd_(Nctx)) ) 
        pCompute_(new) = pCompute_(old) = Ids_IslClient;
	/* intCompute_(rcb) = Ids_IslClient;    */

    /*
    ** If Computation at ... changed from the previous rendition use new one
    */
    if( pCompute_(old) != pCompute_(new) )
        {
        Switch_(Octx) = Switch_(Nctx) = True;
        }

    /*
    ** If the fid is not in the native format like the 
    ** || if appl called ImgOpenDDIFFile with ImgM_NoStandardize flag set
    ** convert the fid to native format. The Old fid is deleted. Convert
    ** frame if the compute mode is ISL and either if a new image has to be
    ** displayed or a switch from xie to isl happens.
    if( pFid_(new) != 0 && ( pFid_(new) != pFid_(old) || Switch_(Nctx) ) && 
					      pCompute_(new) != Ids_XieServer )
	{
	fid = IdsConvertPlane( pFid_(new) );
	if (fid != pFid_(new))
	{
	    if (ForceCopyFid_(new) && (pFid_(new) != 0))
                ImgDeleteFrame(pFid_(new));
	    pFid_(old) = pFid_(new) = fid;
            ForceCopyFid_(new) = True;
           }
       }       
    */
#ifdef TRACE
printf( "Leaving Routine SwitchMode in module IDS_RENDER_IMAGE_MOTIF \n");
#endif
}

