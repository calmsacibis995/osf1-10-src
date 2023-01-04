/*
** idsprot.h - internal prototype file for IDS
*/

/*
 * The DAS_EXPAND_PROTO flag along with the PROTO macro allow for tailoring
 * routine declarations to expand to function prototypes or not depending
 * on the particular platform (compiler) capabilities.
 * If DAS_EXPAND_PROTO is defined, the PROTO macro will expand to function
 * prototypes.  If OS2 or msdos turn on flag as prototypes must be used
 * on these platforms.  For other platforms it is left to the application
 * to #define DAS_EXPAND_PROTO before #include of this file if function
 * prototyping is desired.
 */
#if defined(OS2) || defined(msdos) || defined(vaxc) || defined(__STDC__)
#ifndef DAS_EXPAND_PROTO
#define DAS_EXPAND_PROTO 1
#endif
#endif

/*
 * usage: PROTO (return_type function, (arg1, arg2, arg3))
 */
#ifndef DAS_PROTO
#if DAS_EXPAND_PROTO == 1
#define PROTO(name, arg_list) name arg_list
#else
#define PROTO(name, arg_list) name ()
#endif
#endif

#ifndef IDSPROT_H

#define IDSPROT_H

#ifdef IDS_NOX
#include <ids__widget_nox.h>
#else
#include <ids__widget.h>
#endif
#include <IdsImage.h>
#include <ids_alloc_colors.h>

#include <img/imgprot_ids.h>


/* ids__pipe.c */

PROTO( IdsPipeDesc IdsAllocatePipeDesc, (
	void));

PROTO( unsigned long *IdsInsertPipe, (
	IdsPipeDesc /*pd*/, 
	IdsPipeElement /*new*/, 
	IdsPipeCallProc /*call*/, 
	unsigned long /*id*/, 
	char */*data*/, 
	unsigned long */*dst*/, 
	IdsPipeFunction /*function*/, 
	unsigned long */*args*/));

PROTO( unsigned long IdsExecutePipe, (
	IdsPipeDesc /*pd*/));

PROTO( void IdsDeallocatePipeDesc, (
	IdsPipeDesc /*pd*/));

PROTO( unsigned long IdsCallG, (
	long /*arglist*/[], 
	unsigned long (*/*procedure*/)()));

/* ids_alloc_colors.c */

#ifndef IDS_NOX
PROTO( IdsMatchData IdsMatchColors, (
	Screen */*screen*/, 
	Colormap /*colormap*/, 
	XColor */*img_colors*/, 
	unsigned long /*img_cnt*/, 
	XColor */*appl_colors*/, 
	unsigned long /*appl_cnt*/, 
	unsigned long /*match_space*/));
#endif

/* ids_build_xie_pipe.c */

#ifndef IDS_NOX
PROTO( void IdsApplyModelXie, (
	RenderContext /*ctx*/, 
	RenderContextXie /*xiectx*/));

PROTO( XiePhoto IdsCompileServerPipe, (
	RenderContext /*ctx*/, 
	RenderContextXie /*xiectx*/));

PROTO( XiePhoto CropPhoto, (
	XiePhoto /*photo*/, 
	DataForXie /*xiedat*/, 
	Display */*dpy*/));

PROTO( XiePhoto LuminancePhoto, (
	XiePhoto /*photo*/, 
	Display */*dpy*/));

PROTO( XiePhoto ScalePhoto, (
	IdsRenderCallback /*rcb*/, 
	XiePhoto /*photo*/, 
        DataForXie /*xiedat*/,
	Display */*dpy*/));

PROTO( XiePhoto RotatePhoto, (
	IdsRenderCallback /*rcb*/, 
	XiePhoto /*photo*/, 
	Display */*dpy*/));

PROTO( XiePhoto MirrorPhoto, (
	RenderContextXie /*xiectx*/, 
	XiePhoto /*photo*/, 
	Display */*dpy*/));

PROTO( XiePhoto ToneScalePhoto, (
	IdsRenderCallback /*rcb*/, 
	RenderContext /*ctx*/, 
	XiePhoto /*photo*/, 
	PhotoContext /*phoctx*/, 
	Display */*dpy*/));

PROTO( XiePhoto DitherPhoto, (
	IdsRenderCallback /*rcb*/, 
	XiePhoto /*photo*/, 
	Display */*dpy*/));

PROTO(Boolean VxtImagingOption, (
	Display */*dpy*/, 
	RenderContext /*ctx*/));


PROTO(static XiePhoto InternalRoiPhoto, (
	XiePhoto /*photo*/,
	DataForXie /*xiedat*/, 
	Display */*dpy*/));

PROTO(static XiePhoto AreaPhoto, (
	IdsRenderCallback /*rcb*/,
	RenderContext     /*ctx*/, 
	XiePhoto          /*photo*/, 
	Display		 */*dpy */, 
	DataForXie	  /*xiedat*/, 
	PhotoContext	  /*phoctx*/));

PROTO(static XiePhoto ConstrainPhoto, (
	XiePhoto /*photo*/));

#endif

/* ids_color_sixel.c */

PROTO( unsigned long IdsExportHrColorSixels, (
	unsigned long /*fid*/, 
	unsigned long /*roi*/, 
	char */*bufadr*/, 
	int /*buflen*/, 
	int */*bytcnt*/, 
	int /*flags*/, 
	long (*/*action*/)(), 
	long /*usrprm*/));

PROTO( unsigned long IdsExportLrColorSixels, (
	unsigned long /*fid*/, 
	unsigned long /*roi*/, 
	char */*bufadr*/, 
	int /*buflen*/, 
	int */*bytcnt*/, 
	int /*flags*/, 
	long (*/*action*/)(), 
	long /*usrprm*/));

/* ids_converters.c */

#ifndef IDS_NOX
PROTO( void IdsStringToFloat, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToRenderMode, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToComputeMode, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToRenderClass, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToProtocol, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToColormapMode, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToSaveRendMode, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToCompressMode, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToComporgMode, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToRotateMode, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToRotateOpts, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToFlipOpts, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToScaleMode, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToScaleOpts, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToDitherMode, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToColorSpace, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToGravity, (
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));
#endif

PROTO( int IdsNameToIndex, (
	char */*string*/, 
	char */*strtab*/[], 
	int /*tabsiz*/));

/* ids_export_ps.c */

PROTO( unsigned long IdsExportPs, (
	unsigned long /*nfid*/, 
	RenderContext /*render_ctx*/, 
	unsigned long /*roi*/, 
	char */*bufadr*/, 
	int /*buflen*/, 
	int */*bytcnt*/, 
	int /*flags*/, 
	long (*/*action*/)(), 
	long /*usrprm*/));

/* ids_export_utils.c */

PROTO( IdsPipeDesc IdsCompileExport, (
	RenderContext /*ctx*/, 
	unsigned long */*fid_pointer*/, 
	Boolean /*init*/));

PROTO( unsigned long IdsPlaneSwapByPtr, (
	unsigned long /*fid*/, 
	RenderContext /*ctx*/, 
	Boolean /*copy_fid*/, 
	unsigned long /*roi*/));

/* ids_extensions.c */

PROTO( IdsHistogramData IdsHistogram, (
	unsigned long /*fid*/, 
	unsigned long /*roi*/));

PROTO( unsigned long IdsScale, (
	unsigned long /*fid*/, 
	float */*x_scale*/, 
	float */*y_scale*/, 
	unsigned long /*roi*/, 
	int /*s_flags*/, 
	int /*pad*/));

PROTO( unsigned long IdsConvertFrame, (
	unsigned long /*fid*/));

PROTO( unsigned long IdsConvertPlane, (
	unsigned long /*fid*/));

PROTO( unsigned long IdsSharpen, (
	unsigned long /*fid*/, 
	float */*beta*/, 
	unsigned long /*roi*/));

PROTO( unsigned long IdsExportSixels, (
	unsigned long /*fid*/, 
	unsigned long /*roi*/, 
	char */*bufadr*/, 
	int /*buflen*/, 
	int */*bytcnt*/, 
	int /*flags*/, 
	long (*/*action*/)(), 
	long /*usrprm*/));

#ifdef IMGDEF_H
PROTO( unsigned long IdsPixelRemap, (
	unsigned long /*fid*/, 
	RemapLut /*lut*/, 
	struct PUT_ITMLST */*attr*/, 
	unsigned long /*roi*/));

PROTO( unsigned long IdsPixelTrueRemap, (
	unsigned long /*fid*/, 
	struct PUT_ITMLST */*attr*/, 
	unsigned long /*roi*/));

PROTO( unsigned long IdsToneScale, (
	unsigned long /*fid*/, 
	float */*punch1*/, 
	float */*punch2*/, 
	struct PUT_ITMLST */*attr*/, 
	unsigned long /*roi*/, 
	unsigned long /*dlev*/[]));

PROTO( unsigned long IdsDither, (
	unsigned long /*fid*/, 
	struct PUT_ITMLST */*attr*/, 
	unsigned long /*algor*/, 
	unsigned long /*thresh*/, 
	unsigned long /*roi*/, 
	unsigned long */*levels*/));

PROTO( unsigned long IdsRequantize, (
	unsigned long /*fid*/, 
	struct PUT_ITMLST */*attr*/, 
	unsigned long /*roi*/, 
	unsigned long */*levels*/));

PROTO( void IdsPutItem, (
	struct PUT_ITMLST **/*itmlst*/, 
	unsigned long int /*code*/, 
	unsigned long int /*value*/, 
	unsigned long int /*index*/));

PROTO( void IdsSetItmlst, (
	struct ITMLST **/*itmlst*/, 
	unsigned long int /*code*/, 
	unsigned long int /*value*/, 
	unsigned long int /*index*/));

PROTO( void IdsFreePutList, (
	struct PUT_ITMLST **/*itmlst*/));

PROTO( void IdsFreeItmlst, (
	struct ITMLST **/*itmlst*/));
#endif

/* ids_panned_image.c */

#ifndef IDS_NOX
#ifdef APPLPROG
/* DECwindows entry points */

PROTO( Widget IdsPannedImage, (
	Widget /*parent*/, 
	char */*name*/, 
	int /*x*/, 
	int /*y*/, 
	int /*w*/, 
	int /*h*/, 
	long /*fid*/, 
	DwtCallbackPtr /*rend*/, 
	DwtCallbackPtr /*view*/, 
	DwtCallbackPtr /*drag*/, 
	DwtCallbackPtr /*help*/));

PROTO( Widget IdsPannedImageCreate, (
	Widget /*parent*/, 
	char */*name*/, 
	Arg */*arglist*/, 
	int /*argCount*/));
#endif
#endif

/* ids_panned_image_motif.c */

#ifndef IDS_NOX
PROTO( Widget IdsXmPannedImage, (
	Widget /*parent*/, 
	char */*name*/, 
	int /*x*/, 
	int /*y*/, 
	int /*w*/, 
	int /*h*/, 
	unsigned long /*fid*/, 
	XtCallbackList /*rend*/, 
	XtCallbackList /*view*/, 
	XtCallbackList /*drag*/, 
	XtCallbackList /*help*/ ));

PROTO( Widget IdsXmPannedImageCreate, (
	Widget /*parent*/, 
	char */*name*/, 
	Arg */*arglist*/, 
	int /*argCount*/));
#endif

/* ids_ps_mgmt.c */

PROTO( unsigned long int IdsCreatePresentSurface, (
	IdsItmlst2 */*itmlst*/));

PROTO( unsigned long int IdsSetSurfaceAttributes, (
	RenderContext /*psid*/, 
	IdsItmlst2 */*itmlst*/));

PROTO( unsigned long int IdsGetSurfaceAttributes, (
	RenderContext /*psid*/, 
	IdsItmlst2 */*itmlst*/));

PROTO( void IdsDeletePresentSurface, (
	RenderContext /*psid*/));

PROTO( void IdsVerifyPs, (
	RenderContext/*psid*/));

/* ids_render_image.c */

#ifndef IDS_NOX
PROTO( IdsAllocStatistics IdsGetColorStatistics, (
	Widget /*rw*/));
PROTO( Boolean IdsCompareRenderings, (
	Widget /*rw*/));
PROTO( void IdsTranslatePoint, (
	Widget /*rw*/, 
	XPoint */*coords*/));
PROTO(long XieRerenderImage, (
	Display */*display*/, 
	XErrorEvent */*error*/));

#endif

/* ids_render_image_motif.c */

#ifndef IDS_NOX
PROTO( IdsAllocStatistics IdsXmGetColorStatistics, (
	Widget /*rw*/));

PROTO( Boolean IdsXmCompareRenderings, (
	Widget /*rw*/));

PROTO( void IdsXmTranslatePoint, (
	Widget /*rw*/, 
	XPoint */*coords*/));
#endif

/* ids_render_isl.c */

#ifndef IDS_NOX
PROTO( void CompareRenderingsFid, (
	IdsRenderCallback /*old*/, 
	IdsRenderCallback /*new*/, 
	RenderContext /*ctx*/, 
	GC /*igc*/, 
	Boolean /*pfid*/));

PROTO( void TranslatePointFid, (
	IdsRenderCallback /*rcb*/, 
	XPoint */*coords*/, 
	unsigned long /*width*/, 
	unsigned long /*height*/));
#endif

/* ids_render_utils.c */

PROTO( void IdsApplyModel, (
	RenderContext /*ctx*/));

PROTO( IdsPipeDesc IdsCompileRendering, (
	RenderContext /*ctx*/, 
	unsigned long */*fid_pointer*/, 
	Boolean /*init*/));

#ifdef VMS
#ifndef CHFDEF
#include <ChfDef.h>
#endif
PROTO( int Error_condition_handler, (
	int */*signal*/, 
	ChfMchArgsPtr /*mechanism*/));
#else
PROTO( int Error_condition_handler, (
	int */*signal*/, 
	int */*mechanism*/));
#endif

/* ids_render_xie.c */

#ifndef IDS_NOX
PROTO( Boolean CompareRenderingsFlo, (
	IdsRenderCallback /*old*/, 
	IdsRenderCallback /*new*/, 
	RenderContext /*ctx*/, 
	RenderContextXie /*xiectx*/, 
	GC /*igc*/, 
	Boolean /*pfid*/));

PROTO( void TranslatePointFlo, (
	DataForXie /*xiedat*/, 
	IdsRenderCallback /*rcb*/, 
	XPoint */*coords*/, 
	unsigned long /*width*/, 
	unsigned long /*height*/));
#endif

/* ids_rendering_mgmt.c */

PROTO( IdsRendering *IdsCreateRendering, (
	unsigned long /*fid*/, 
	RenderContext /*psid*/, 
	IdsItmlst2 */*itmlst*/));

PROTO( void IdsDeleteRendering, (
	IdsRendering */*rendering*/));

PROTO( IdsRendering *IdsAllocateRendering, (
	RenderContext /*render_ctx*/, 
	RenderContext /*psid*/));

PROTO( RenderContext IdsAllocateRenderContext, (
	RenderContext /*ctx*/, 
	unsigned long /*rfid*/));

PROTO( RenderContext IdsApplyItmlst, (
	RenderContext /*render_ctx*/, 
	IdsItmlst2 */*itmlst*/));

#ifndef IDS_NOX
PROTO(void ZoomingCheck, (
	Widget /*iw*/, 
	unsigned long */*ix*/, 
	unsigned long */*iy*/,
	unsigned long */*iheight*/, 
	unsigned long */*iwidth */));
#endif


/* ids_static_image.c */

#ifndef IDS_NOX
#ifdef APPLPROG
/* DECwindows entry points */

PROTO( Widget IdsStaticImage, (
	Widget /*parent*/, 
	char */*name*/, 
	int /*x*/, 
	int /*y*/, 
	int /*w*/, 
	int /*h*/, 
	unsigned long /*fid*/, 
	DwtCallbackPtr /*rend*/, 
	DwtCallbackPtr /*view*/, 
	DwtCallbackPtr /*help*/));

PROTO( Widget IdsStaticImageCreate, (
	Widget /*parent*/, 
	char */*name*/, 
	Arg */*arglist*/, 
	int /*argCount*/));
#endif

PROTO( XPoint *IdsGetCoordinates, (
	Widget /*iw*/, 
	XPoint */*from*/, 
	XPoint */*to*/, 
	int /*num*/, 
	int /*type*/));

PROTO( void IdsRedisplayImage, (
	Widget /*iw*/, 
	int /*sx*/, 
	int /*sy*/, 
	int /*wx*/, 
	int /*wy*/));

PROTO( void IdsApplyGravity, (
	Widget /*iw*/, 
	unsigned long /*src_grav*/, 
	unsigned long /*win_grav*/));

PROTO(void ZoomingCheckMotif, (
	Widget /*iw*/, 
	unsigned long */*ix*/,
	unsigned long */*iy*/,
	unsigned long */*iheight*/, 
	unsigned long */*iwidth*/));

#endif

/* ids_static_image_motif.c */

#ifndef IDS_NOX
PROTO( Widget IdsXmStaticImage, (
	Widget /*parent*/, 
	char */*name*/, 
	int /*x*/, 
	int /*y*/, 
	int /*w*/, 
	int /*h*/, 
	unsigned long /*fid*/, 
	XtCallbackList /*rend*/, 
	XtCallbackList /*view*/, 
	XtCallbackList /*help*/));

PROTO( Widget IdsXmStaticImageCreate, (
	Widget /*parent*/, 
	char */*name*/, 
	Arg */*arglist*/, 
	int /*argCount*/));

PROTO( XPoint *IdsXmGetCoordinates, (
	Widget /*iw*/, 
	XPoint */*from*/, 
	XPoint */*to*/, 
	int /*num*/, 
	int /*type*/));

PROTO( void IdsXmRedisplayImage, (
	Widget /*iw*/, 
	int /*sx*/, 
	int /*sy*/, 
	int /*wx*/, 
	int /*wy*/));

PROTO( void IdsXmApplyGravity, (
	Widget /*iw*/, 
	unsigned long /*src_grav*/, 
	unsigned long /*win_grav*/));
#endif

/* ids_uil_support_motif.c */

PROTO( int IdsXmInitializeForDRM, (
	void));

/* ids_xieimage_ddif.c */

#ifndef IDS_NOX
PROTO( unsigned long IdsDecToXieImage, (
	char */*name*/));

PROTO( unsigned long IdsFidToXieImage, (
	unsigned long /*fid*/, 
	unsigned char /*copy*/));

PROTO( unsigned long IdsPhotoToXieImage, (
	XiePhotomap /*map*/, 
	unsigned long /*cmpres*/, 
	unsigned long /*cmap*/));

PROTO( unsigned long IdsPhotoToFid, (
	XiePhotomap /*map*/, 
	unsigned long /*cmpres*/, 
	unsigned long /*cmap*/));

PROTO( unsigned long IdsXieImageToFid, (
	XieImage /*img*/));

PROTO( unsigned long ConvertRenderedFid, (
	unsigned long /*fid*/, 
	unsigned long /*cmpres*/, 
	unsigned long /*cmap*/));

PROTO( void IdsPhotoToDec, (
	XiePhotomap /*map*/, 
	unsigned long /*cmpres*/, 
	unsigned long /*cmap*/, 
	char */*name*/));

PROTO( void IdsFidToDec, (
	unsigned long /*fid*/, 
	unsigned long /*cmpres*/, 
	unsigned long /*cmap*/, 
	char */*name*/));

PROTO( DataForXie IdsDataForXie, (
	unsigned long /*fid*/));

PROTO( void CollectRoiData, (
	DataForXie /*pridat*/, 
	unsigned long /*roi_id*/));
#endif

/* ids_ximage_fid.c */

#ifndef IDS_NOX
PROTO( unsigned long IdsXimageToFid, (
	Display */*display*/, 
	XImage */*ximage*/, 
	Visual */*visual*/, 
	Colormap /*cmap*/, 
	unsigned long /*output_class*/));
#endif

#endif /* IDSPROT_H */
