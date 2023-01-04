
/******************************************************************************
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
**/

/*******************************************************************************
**
**  FACILITY:
**
**      Image Display Services (IDS)
**
**  ABSTRACT:
**
**      This module creates and manages the IDS rendering object. Rendition code
**	common with the RENDER WIDGET actually performs image rendition.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**
**  AUTHOR(S):
**
**      John Weber
**      Subu Garikapati
**
**  CREATION DATE:     November 21, 1987
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

/*
**  Standard C include files
*/
#include <img/ChfDef.h>			    /* Condition handling functions */
#include <math.h>			    /* Math function definitions    */
#include <stdio.h>			    /* Standard I/O symbols	    */

/*
**  IDS include fils
*/
#include <img/IdsStatusCodes.h>		    /* Condition codes		    */

/*
**  DECwindows include files
*/

#include "ids__macros.h"		    /* IDS MACRO definitions	    */
#ifdef IDS_NOX
#include <ids__widget_nox.h>
#else
#include <ids__widget.h>		    /* IDS private symbols/constants */
#endif

/*
#ifdef VMS
#include <decw$include/DwtAppl.h>   
#endif
*/

/*
**  VIS include files
*/
#include <img/ImgDef.h>			    /* ISL Definitions		    */
#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
#ifdef VMS
IdsRendering			*IDS$CREATE_RENDERING();
void				 IDS$DELETE_RENDERING();
void				 IDS$DISPLAY_RENDERING();
#endif
IdsRendering			*IdsCreateRendering();
void				 IdsDeleteRendering();
void                             IdsDisplayRendering();

IdsRendering			*IdsAllocateRendering();
RenderContext			 IdsApplyItmlst();
IdsRendering			*IdsExportRendition();
int				 IdsNameToIndex();
RenderContext			 IdsAllocateRenderContext();
void				 IdsDeallocRenderContext();

static void	                 DeleteRendition();
static IdsRendering		*ExportRendering();
#ifndef IDS_NOX
static void      		 ExportPixmapRendering();
#endif
static void			 ExportSixelRendering();
#ifndef IDS_NOX
static void			 SetImageGC();
#endif
static void                      PrePareFidForRend();
#else
PROTO(static IdsRendering *ExportRendering, (IdsRendering */*rendering*/, RenderContext /*render_ctx*/, unsigned long /*fid*/));
PROTO(static void ExportSixelRendering, (IdsRendering */*rendering*/, RenderContext /*render_ctx*/, unsigned long /*fid*/));
PROTO(static void DeleteRendition, (IdsRendering */*rendering*/));
PROTO(static void ExportPixmapRendering, (IdsRendering */*rendering*/, RenderContext /*ctx*/, unsigned long /*fid*/));
PROTO(static void SetImageGC, (IdsRendering */*rendering*/, RenderContext /*ctx*/, unsigned long /*polarity*/));
PROTO(static void PrePareFidForRend, (RenderContext /*ctx*/));
#endif

/*
**  MACRO definitions
*/
   /*
    **  Simplify calling ImgGet(...)
    */
#define ImgGet_(fid,item,buffer,index) \
            ImgGet((fid),(item),&(buffer),(sizeof(buffer)),0,(index))
                                                                         
/*
**  Equated Symbols
*/
#define ENCAPSULATED_COLOR_POSTSCRIPT_OVERHEAD 1800/* Overhead bytes + 10%  */
#define ENCAPSULATED_MONO_POSTSCRIPT_OVERHEAD 1100
#define COLOR_POSTSCRIPT_OVERHEAD 2100	/* Overhead bytes + 10%		    */
#define MONO_POSTSCRIPT_OVERHEAD 1300

#define POSTSCRIPT_MAXCHAR 511		/* Maximum length of Postscript     */
					/*  data			    */
#define COLOR_SIXEL_OVERHEAD 50		/* Non-data bytes in color sixels   */
#define SIXEL_OVERHEAD_PAD 7		/* Pad for possible worst sixel code */

#define formapGCmask    GCForeground | GCBackground | GCTile | GCFillStyle | GCLineWidth
#define highmapGCmask   GCForeground | GCBackground | GCTile | GCFillStyle | GCLineWidth
#define backmapGCmask   GCForeground | GCBackground | GCTile | GCFillStyle | GCLineWidth


#undef TRUE
#define TRUE 1
#define FALSE 0

#define X 0				/* Constants used for access to	    */
#define Y 1				/*  unit conversion array	    */

#define BPI 1200			/* BMUs per inch		    */
#define BPMM (1200/25.4)		/* BMUs per MM			    */

/*
**  External References
*/
#ifdef NODAS_PROTO
extern IdsPipeDesc	IdsCompileRendering();
extern IdsPipeDesc	IdsCompileExport();
extern unsigned long    IdsExecutePipe();
extern IdsPipeDesc      IdsAllocatePipeDesc();
extern void             IdsDeallocatePipeDesc();
extern void		IdsVerifyPs();
extern unsigned long    IdsExportPs();
extern struct FCT      *IdsConvertPlane();
#endif

/*
**	Local Storage
*/
  
/*
**  This table is an ordered list of all valid RENDERING item code name strings.
**  It is used by IDS_NAME_TO_INDEX to translate name strings passed into
**  CREATE_RENDERING into a unique table index.
**
**  NOTE: Changes made to this table must maintain the following characteristics
**	  1) The table must be ordered.
**	  2) The constants defined after each table entry must reflect the
**	     corresponding string's offset in the table.
*/
static char *render_items[] = {
    IdsNditherAlgorithm,		/* "ditherAlgorithm"		    */
#define DITHER_ALGORITHM    0
    IdsNditherThreshold,		/* "ditherThreshold"		    */
#define DITHER_THRESHOLD    1
    IdsNfitHeight,			/* "fitHeight"			    */
#define FIT_HEIGHT	    2
    IdsNfitLevels,			/* "fitLevels"			    */
#define FIT_LEVELS	    3
    IdsNfitWidth,			/* "fitWidth"			    */
#define FIT_WIDTH	    4
    IdsNlevelsBlue,			/* "levelsBlue"			    */
#define LEVELS_BLUE	    5
    IdsNlevelsGray,			/* "levelsGray"			    */
#define LEVELS_GRAY	    6
    IdsNlevelsGreen,			/* "levelsGreen"		    */
#define LEVELS_GREEN	    7
    IdsNlevelsRed,			/* "levelsRed"			    */
#define LEVELS_RED	    8
    IdsNprotocol,			/* "protocol"			    */
#define PROTOCOL	    9
    IdsNpsFlags,			/* "psFlags"			    */
#define PS_FLAGS	    10
    IdsNpunch1,				/* "punch1"			    */
#define PUNCH_1		    11
    IdsNpunch2,				/* "punch2"			    */
#define PUNCH_2		    12
    IdsNsharpen,			/* "sharpen"			    */
#define SHARPEN             13
    IdsNrenderMode,                     /* "renderMode"                     */
#define RENDER_MODE	    14
    IdsNroi,				/* "roi"			    */
#define ROI		    15
    IdsNrotateAngle,			/* "rotateAngle"		    */
#define ROTATE_ANGLE	    16
    IdsNrotateMode,			/* "rotateMode"			    */
#define ROTATE_MODE	    17
    IdsNrotateOptions,			/* "rotateOptions"		    */
#define ROTATE_OPTIONS	    18
    IdsNscaleMode,			/* "scaleMode"			    */
#define SCALE_MODE	    19
    IdsNscaleOptions,			/* "scaleOptions"		    */
#define SCALE_OPTIONS	    20
    IdsNsourceGravity,			/* "sourceGravity"		    */
#define SOURCE_GRAVITY	    21
    IdsNunits,				/* "units"			    */
#define UNITS		    22
    IdsNwindowGravity,			/* "windowGravity"		    */
#define WINDOW_GRAVITY	    23
    IdsNxScale,				/* "xScale"			    */
#define X_SCALE		    24
    IdsNyScale,				/* "yScale"			    */
#define Y_SCALE		    25
    IdsNflipOptions
#define FLIP		    26
};
#define RENDER_ITEMS (sizeof(render_items) / sizeof(int))

/*
**  These arrays define ANSI sequences for SIXEL introducers
*/
#define DEC_LASER     "\233!p\233?20 J\233?52h\233\060;3300r\233\067 I\033P9;0;1q"
#define DEC_LJ250_HR  "\233!p\233?20 J\233?52h\233\060;1980r\233\067 I\033P9;0;1q"
#define DEC_LJ250_LR  "\233!p\233?20 J\233?52h\233\060;990r\233\067 I\033P0;0;9q\042\061;1;0;0"
/*
\233 = CSI
\033 = ESC
ESCP = DCS
CSI? = DSR Device Status Request


CSI!p	    - Soft Terminal Reset (DECSTR) page 5-9
CSI?20<SP>J - 
    DSR20<sp>J - Extended Page format, write beyond default margins.
CSI?52h
    DSR52h
	    - Setting a DEC private Mode set origin to upper left.

CSI0;3300r  - CSI ptop ; Pbottom r    Set top and bottom margins
CSI7<SP>I   - Horizontal Position Absolute (HPA)

DCS9;0;1q   - page 6-2 sixel graphics mode setup
  Ps1 =     9 means Horizontal grid size and pixel aspect ratio =
		     1/72		      100:100 or 1:1
					    Vertical:Horizontal

  Ps2 =	    0 background Select is ignored page 6-4 in companion color printer
  Ps3 =     1 horizontal grid (pixel width) in decipoints.
	      A decipoint = 1/720 inch.
	      1 means  1/180 (.0056)  this will be default
	      9 means  1/90  (.0111)  this will be lj250 low res.


*/
#define DEC_LAxx      "\033Pq"
#define DEC_VT2x      "\033P1;0q"
#define DEC_VT3x      "\033P7;0q"
#define EXIT_SIXEL    "\033\\"

static struct sixel_entry {
    unsigned long device_type;
    char *sixel_introducer;
} sixel_table[] = {
	{Ids_TmpltVt125,	DEC_VT2x},/* VTxx SIXEL capable terminals    */
	{Ids_TmpltVt240,	DEC_VT2x},
	{Ids_TmpltVt330,	DEC_VT3x},
	{Ids_TmpltVt340,	DEC_VT3x},
	{Ids_TmpltLa50, 	DEC_LAxx},/* LA series (dumb) printers	     */
	{Ids_TmpltLa75, 	DEC_LAxx},
	{Ids_TmpltLa100,	DEC_LAxx},
	{Ids_TmpltLn03s,	DEC_LASER},/* Laser printers (or equivalent) */
	{Ids_TmpltLn03r,	DEC_LASER},
	{Ids_TmpltLps20,	DEC_LASER},
	{Ids_TmpltLps40,	DEC_LASER},
	{Ids_TmpltLcg01,	DEC_LASER},
	{Ids_TmpltLj250,	DEC_LJ250_HR},
	{Ids_TmpltLa210,   	DEC_LASER},
	{Ids_TmpltLj250Lr, 	DEC_LJ250_LR},
	{99999,			DEC_LASER},/* Default if undefined	    */
};
#define ST_LENGTH (sizeof(sixel_table)/sizeof(struct sixel_entry))

/*******************************************************************************
**
**  IDS$CREATE_RENDERING
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine renders an image for display on a given presentation
**	surface. The characteristics of the rendered image depend on the
**	attributes of the target presentation surface and user specified
**	rendition parameters.
**
**  FORMAL PARAMETERS:
**
**      fid    - ISL frame identifier which specifies the source image
**	psid   - Presentation surface identifier which describes the target
**	         presentation surface for this rendition operation.
**	itmlst - User specified rendition parameters which may adjust or
**		 override the IDS rendition model.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      rndrng - address of the rendering. The rendering object is a public
**		 structure which describes the rendered image and the rendition
**		 steps which generated it.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*******************************************************************************/
/*DECwindows entry point definition                                           */
#ifdef VMS
IdsRendering *IDS$CREATE_RENDERING(fid, psid, itmlst)
unsigned long fid;                                /* ISL frame ID                     */
RenderContext psid;                     /* IDS rendition context            */
IdsItmlst2 *itmlst;              /* Rendition parameters item list   */
{
   return( IdsCreateRendering(fid, psid, itmlst));
}
#endif

IdsRendering  *IdsCreateRendering(fid, psid, itmlst)
unsigned long fid;				/* ISL frame ID			    */
RenderContext psid;			/* IDS rendition context	    */
IdsItmlst2 *itmlst;		/* Rendition parameters item list   */
{
    IdsRendering *rendering;		/* Rendering object to be created   */
    RenderContext render_ctx;		/* Render process context	    */
    IdsRenderCallback rcb;
    int spectral_map;

#ifdef TRACE
printf( "Entering Routine IdsCreateRendering in module IDS_RENDERING_MGMT \n");
#endif
    /*
    **	If a psid is supplied, check its integrity.
    */
    if (psid != NULL)
	IdsVerifyPs(psid);

    /*
    **  Make sure we support the spectral component mapping of this fid.
    */
    GetIsl_( fid, Img_SpectralMapping, spectral_map, 0 );

    if (spectral_map != ImgK_MonochromeMap && spectral_map != ImgK_RGBMap)
	ChfStop (1,IdsX_UnsSpcTyp);	    /* Unsupported spectral type    */

    /*
    **	Initialize a render context structure based on the current presentation
    **	surface parameters.
    */
    render_ctx = IdsAllocateRenderContext(psid, fid);

    /*
    ** Bug fix for protcol Fid. Initialize scnline modulo to 32
    ** to avoid ISL memory alloc problem
     if( iProto_(rcb) == Ids_Fid && RClass_(ctx) == Ids_Bitonal )
    */
     if( render_ctx->proposed_rcb->protocol == Ids_Fid )
            PrePareFidForRend( render_ctx );

    /*
    **  If user supplied rendition parameters in an item list, then apply 
    **  those to the current rendition model.
    */
    if (itmlst != NULL)
	IdsApplyItmlst(render_ctx, itmlst);

    /*
    **	Allocate and initialize a rendering structure.
    */
    rendering = IdsAllocateRendering(render_ctx,psid);
    Hsfid_(rendering)   = fid;

    /*
    **	Set Some of the Render Control Block values
    **
    **  The rendering->rcb = render_ctx was set in IdsAllocateRendering
    **	The render_ctx->proposed_rcb = the current render callback
    **  This was done is IdsAllocateRenderContext
    **
    */
    render_ctx->proposed_rcb->protocol  = Proto_(render_ctx);
    render_ctx->proposed_rcb->fit_width = 
	FitW_(render_ctx) != 0 ? FitW_(render_ctx) : WinW_(render_ctx);    
    render_ctx->proposed_rcb->fit_height = 
	FitH_(render_ctx) != 0 ? FitH_(render_ctx) : WinH_(render_ctx);    

    /*
    ** If protocol is not pixmap or ximage, and if no template is used, 
    ** x_bmu and y_bmu were not set then signal and stop
    */
    if( render_ctx->proposed_rcb->x_pels_per_bmu == 0.0 || 
                    render_ctx->proposed_rcb->y_pels_per_bmu == 0.0 )
	ChfStop (1,IdsX_UnsSpcTyp);
	    /* Unsupported spectral type    */

    /*
    **  The rendition model is now established; render the image, 
    **  then export it according to the protocol.
    */
    IdsApplyModel(render_ctx);

    /*
    ** If Protocol is Ids_PostScript || Ids_Sixel no need to call IdsCompileExport(
    ** But the pipe does not work without it. For this protocol this function
    ** just returns. This needs to be fixed.
    */
    if( Process_( render_ctx ) == Ids_XieServer )
        Process_( render_ctx ) = Ids_IslClient;

    if( Process_( render_ctx ) == Ids_IslClient ) 
	ExportRendering(rendering, render_ctx,
	        IdsExecutePipe(
			IdsCompileExport(render_ctx, PDfid_(IdsCompileRendering
			    (render_ctx, &render_ctx->proposed_rcb->fid,TRUE)), FALSE)));
#ifdef TRACE
printf( "Leaving Routine IdsCreateRendering in module IDS_RENDERING_MGMT \n");
#endif
    return (rendering);
}

/*******************************************************************************
**  IDS$DELETE_RENDERING
**
**  FUNCTIONAL DESCRIPTION:
**
**      Deallocate rendering structure and all associated resources.
**
**  FORMAL PARAMETERS:
**
**      rendering - pointer to IdsRendering structure to deallocate
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*******************************************************************************/
/*DECwindows entry point definition                                           */
#ifdef VMS
void IDS$DELETE_RENDERING(rendering)
IdsRendering *rendering;
{
 IdsDeleteRendering(rendering);
}
#endif
void IdsDeleteRendering(rendering)
IdsRendering *rendering;
{
RenderContext  ctx = (RenderContext)Hrcb_(rendering);  
IdsRenderCallback rcb = (IdsRenderCallback)PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine IdsDeleteRendering in module IDS_RENDERING_MGMT \n");
#endif
    switch(rendering->type)
    {
    case Ids_Fid:
    case Ids_Sixel:
    case Ids_PostScript:
      /*
      ** Temp XImage struct was alloacted before for the ExportImage 
      ** function to work .So now is time to dealloc that struct.
      */
      if (Image_(ctx) != NULL) _ImgFree(Image_(ctx));
      break;

    case Ids_Pixmap:
#ifdef IDS_NOX
	     ChfStop(1,IdsX_XNotInUse);
#else
         if( Htype_(rendering) != 0 )
            XFreePixmap( Dpy_(ctx), Hpixmap_(rendering));
#endif

    case Ids_XImage:
#ifdef IDS_NOX
	     ChfStop(1,IdsX_XNotInUse);
#else
         /*
         ** Free the GC
         */
      if ((rendering->type == Ids_XImage) ||
	  ((rendering->type == Ids_Pixmap) && (Htype_(rendering) != 0)))
            XFreeGC( Dpy_(ctx), HGC_(rendering));

         /*
         ** Free the list of pixels allocated
         */
         if( PxlDat_(ctx) != NULL )
           XtFree( PxlDat_(ctx) );
         PxlDat_(ctx) = NULL;
 
         /*
         ** Deallocate the colors
         */
         if( PxlLst_(ctx) != 0 )
            XFreeColors( Dpy_(ctx), Cmap_(ctx), PxlLst_(ctx), PxlCnt_(ctx), 0 );
         MemFree_( &PxlLst_(ctx) );
         PxlCnt_(ctx) = 0;
#endif
         break;
    default:
         ChfSignal(1,IdsX_InvPrtTyp);
    }/*end of switch */  
    /*
    ** Delete the remaining rendering resources
    */
    IdsFreePutList( &RqLst_(ctx) );
    IdsFreePutList( &TsLst_(ctx) );
    IdsFreePutList( &DiLst_(ctx) );
    IdsDeallocatePipeDesc( Pipe_(ctx) );

    /* Delete some lingering fids */
    if (iFid_(rcb) != 0 && iFid_(rcb) != Hsfid_(rendering))
	    {	    
            ImgDeleteFrame(iFid_(rcb));
            if (iFid_(rcb) == Hrnfid_(rendering))
                Hrnfid_(rendering) = 0;
	    iFid_(rcb) = 0;
	    }
    DeleteRendition(rendering);
/* The rcb Memory from Create Rendering (IdsAllocateRenderContext)	 */
    _ImgFree(rcb);  
/* The new_ctx Memory from Create Rendering (IdsAllocateRendering)	 */
    _ImgFree(ctx);  
/* The rendering Memory from Create Rendering (IdsAllocateRenderContext) */
    _ImgFree(rendering); 

#ifdef TRACE
printf( "Leaving Routine IdsDeleteRendering in module IDS_RENDERING_MGMT \n");
#endif
    return;
}

/*******************************************************************************
**  IdsAllocateRendering
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function allocates and initializes the RENDERING structure.
**
**  FORMAL PARAMETERS:
**
**  This is bogus from V1 and V2
**      fid	    - ISL frame id of image frame to be rendered.
**
**	render_ctx  - this is a render context to attach to the
**			rendering structure
**	psid	    - identifier of Presentation surface for image display to
**			the rendering structure
**
**  FUNCTION VALUE:
**
**      rndrng - address of the created rendering structure.
**
*******************************************************************************/
IdsRendering *IdsAllocateRendering (render_ctx,psid)
RenderContext render_ctx;
RenderContext psid;
{
   IdsRenderCallback rcb;
    IdsRendering *rendering =(IdsRendering *)_ImgCalloc(1,sizeof(IdsRendering));

#ifdef TRACE
printf( "Entering Routine IdsAllocateRendering in module IDS_RENDERING_MGMT \n");
#endif
    /*
    ** This next line looks really weird.
    ** Why would anyone ever want to assign a render context to a
    ** render callback structure?  The reasoning is that the rcb
    ** (RenderCallback) field of the IdsRendering is used to hold the
    ** RenderContext in HARDCOPY mode.  The RenderContext does have
    ** a field for an rcb.
    */
    Hrcb_(rendering)    = (struct _RenderCallback *) render_ctx;
    Htype_(rendering)   = Proto_(render_ctx);

    rcb = (IdsRenderCallback) PropRnd_(render_ctx);
    if (rcb != 0)
      Hsfid_(rendering)   = rcb->fid;
    Hpsid_(rendering)   = (unsigned long int) psid;

#ifdef TRACE
printf( "Leaving Routine IdsAllocateRendering in module IDS_RENDERING_MGMT \n");
#endif
    return(rendering);
}

/*******************************************************************************
**  IdsAllocateRenderContext
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function allocates and initializes the RenderContext structure.
**
**  FORMAL PARAMETERS:
**
**	ctx	- Render Context from Create Present Surface
**	rfid	- Fid to be rendered
**
**  FUNCTION VALUE:
**
**      new_ctx - address of allocated and initialized RenderContext structure.
**
*******************************************************************************/
RenderContext	 IdsAllocateRenderContext(ctx, rfid)
RenderContext	 ctx;
unsigned long rfid;
{
    int i, levs, levels[Ids_MaxComponents], component_count;
    IdsRenderCallback rcb;
    unsigned long   fid;         /* ISL frame-id of image to render  */ 
    int bpc = 0;
#ifndef IDS_NOX
    Screen *screen;
#endif
    RenderContext new_ctx;

#ifdef TRACE
printf( "Entering Routine IdsAllocateRenderContext in module IDS_RENDERING_MGMT \n");
#endif
    new_ctx = (RenderContext) _ImgCalloc(1,sizeof(RenderContextStruct));

    /*
    **	Allocate a virgin render context structure. If a default was provided,
    **	then set the new one to default values.
    */
    if (ctx != NULL)
	*new_ctx = *ctx;

    /*
    ** If the fid is not in the native format like the CDA folk's have it
    ** || if appl called ImgOpenDDIFFile with ImgM_NoStandardize flag set
    ** convert fid to native format. The Old fid is deleted in the called fn
    */
    if( rfid != 0 )
      {
	/*
	** Do Not delete the return fid.  Users have complained if we do.
	*/
        fid = IdsConvertPlane( rfid );
      }
    else
      {
        fid = 0;    /* rfid was zero, fid must also become zero */
      }

    /*
    **	Allocate an empty XImage struct if it is not preallocated. This happens
    **  if the protocol is not XImage or Pixmap.
    */
#ifndef IDS_NOX
    if( Image_(new_ctx) == 0 )
       if( Dpy_(new_ctx) == 0 )
	    Image_(new_ctx) = (XImage *) _ImgCalloc(1,sizeof(XImage));
       else
	    {
	    /*
	    **  Create a skeleton XImage structure, and copy it to our instance
	    **  record.
	    */
	    screen = XDefaultScreenOfDisplay(Dpy_(new_ctx));
	    Image_(new_ctx) = XCreateImage( Dpy_(new_ctx), Vis_(new_ctx),
                                DefaultDepthOfScreen(screen),
                                ZPixmap, 0, 0, 0, 0, Pad_(new_ctx), 1 );
	    }  
#endif
    /*
    **	Allocate new rendition callback structure. If default context was 
    **  provided, and an RCB exists, copy it.
    */
    rcb = (IdsRenderCallback) _ImgCalloc(1,sizeof(IdsRenderCallbackStruct));
    PropRnd_(new_ctx) = rcb;
    if (ctx != NULL && PropRnd_(ctx) != NULL)
	*PropRnd_(new_ctx) = *PropRnd_(ctx);

    /*
    **	Set reasonable default values for render specific parameters (i.e. not
    **	device attributes.
    */	
    iRender_(rcb) = Ids_Normal;
    iScheme_(rcb) = 0;
    iFid_(rcb)    = fid;
    iROI_(rcb)    = 0;
    iRMode_(rcb)  = Ids_NoRotate;
    /* ROTATE OPTIONS */
    iRAng_(rcb)   = iFlip_(rcb) = 0;
/*    iSMode_(rcb)  = Ids_FitWithin; */
    iSMode_(rcb)  = Ids_NoScale;
    iXSc_(rcb)    = iYSc_(rcb)  = 1;
    /* DEFAULT RESOLUTION */
    iPun1_(rcb)   = 0.0;
    iPun2_(rcb)   = 1.0;
    iSharp_(rcb)  = 0.0;
    iAlgor_(rcb)  = Ids_BlueNoise;
    iThrsh_(rcb)  = 5;
    iGRA_(rcb)    = 0;
    iLevs_(rcb)   = 0;
    iWide_(rcb)   = iHigh_(rcb)  = 0;
    if( iXres_(rcb) == 0 || iYres_(rcb) == 0 )
	iXres_(rcb) = iYres_(rcb) = 1.0;

    /*
    **	Initialize render pipeline variables.
    */
    Pipe_(new_ctx) = (IdsPipeDesc) IdsAllocatePipeDesc();
    /*
    **  If the protocol is Postscript then select the smallest of
    **  the RGB levels.  
    **  If this selected level happens to be 8 then set levels to 4,
    **  else if it selected level is 32 or 64 then set it to 16.  
    **  This is because Postscript only supports 2, 4, 16 and 256 levels 
    **  of RGB.
    */  
    if ( Proto_(new_ctx)  == Ids_PostScript )
	{
	GetIsl_( fid, Img_NumberOfComp, component_count, 0);
	for ( levs = 0, i = 0; i < component_count; i++)
          {
	  GetIsl_( fid, Img_QuantLevelsPerComp, levels[i], i);
          if ( i == 0) 
            levs = levels[i];
          else if( levels[i] < levels[i-1] ) 
             levs = levels[i-1] = levels[i];
          }
        SetBitsPerComponent_( bpc, levs );
        switch (bpc)
	    {
            case 3:  levs = 4; /* if levels is 8 then set to 4 */
                 break;
            case 5:
            case 6: levs = 16; /* if levels is 32 or 64 then set to 16 */
                 break;
	    }
	RGB_(new_ctx)[0] = RGB_(new_ctx)[1] = RGB_(new_ctx)[2] = levs;
	}
    /*	 
    **	Fill the Render Callback with the New Context's RGB levels
    */	 
    iRGB_(rcb)[0] = RGB_(new_ctx)[0];
    iRGB_(rcb)[1] = RGB_(new_ctx)[1];
    iRGB_(rcb)[2] = RGB_(new_ctx)[2];

#ifdef TRACE
printf( "Leaving Routine IdsAllocateRenderContext in module IDS_RENDERING_MGMT \n");
#endif
    return(new_ctx);
}


/*******************************************************************************
**  IdsApplyItmlst
**
**  FUNCTIONAL DESCRIPTION:
**
**      Apply user specified parameters to the currently defined rendition
**	model.
**
**  FORMAL PARAMETERS:
**
**      render_ctx  - pointer to RenderContext structure
**      itmlst	    - user rendition parameters item list
**
**  FUNCTION VALUE:
**
**      render_ctx  - render context structure passed in
**
**  SIGNAL CODES:
**
**      IdsX_InvItmCod - invalid item code in item list
**
*******************************************************************************/
RenderContext	IdsApplyItmlst(render_ctx,itmlst)
RenderContext	 render_ctx;
IdsItmlst2	*itmlst;
{
    IdsRenderCallbackStruct *rcb = PropRnd_(render_ctx);
    IdsItmlst2		    *item;

    int		units = Ids_UnitsPxl;
    double	conversion[Ids_UnitsMax][2];

#ifdef TRACE
printf( "Entering Routine IdsApplyItmlst in module IDS_RENDERING_MGMT \n");
#endif
    conversion[0][0] = 0.0;                     /* Reserved     */
    conversion[0][1] = 0.0;
    conversion[1][0] = 1.0;                     /* Pixels       */
    conversion[1][1] = 1.0;
    conversion[2][0] = rcb->x_pels_per_bmu;     /* BMUs         */
    conversion[2][1] = rcb->y_pels_per_bmu;
    conversion[3][0] = rcb->x_pels_per_bmu*BPMM;/* MM           */
    conversion[3][1] = rcb->y_pels_per_bmu*BPMM;
    conversion[4][0] = rcb->x_pels_per_bmu*BPI; /* Inches       */
    conversion[4][1] = rcb->y_pels_per_bmu*BPI;
      
    for (item = itmlst;  item->item_name != NULL;  item++)
	switch (IdsNameToIndex(item->item_name,render_items,RENDER_ITEMS))
	    {
	    case DITHER_ALGORITHM:
		rcb->dither_algorithm = item->value;
		break;
		
	    case DITHER_THRESHOLD:
		rcb->dither_threshold = item->value;
		break;

	    case FIT_HEIGHT:
		switch (units)
		    {
		    case Ids_UnitsPxl:
		    case Ids_UnitsBMU:
			rcb->fit_height = (unsigned long)
			    ((double)item->value * conversion[units][Y]);
                        FitH_(render_ctx) = (unsigned long)
                            ((double)item->value * conversion[units][Y]);
 			break;
		    case Ids_UnitsInch:
		    case Ids_UnitsMM:
			rcb->fit_height = 
			    TYPE_F_(item->value) * conversion[units][Y];
                        FitH_(render_ctx) =
                            TYPE_F_(item->value) * conversion[units][Y];
  			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		    }
		break;

	    case FIT_LEVELS:
		rcb->fit_levels = item->value;
		break;

	    case FIT_WIDTH:
		switch (units)
		    {
		    case Ids_UnitsPxl:
		    case Ids_UnitsBMU:
			rcb->fit_width = (unsigned long)
			    ((double)item->value * conversion[units][X]);
                       FitW_(render_ctx) = (unsigned long)
                            ((double)item->value * conversion[units][X]);
 			break;
		    case Ids_UnitsInch:
		    case Ids_UnitsMM:
			rcb->fit_width = 
                            TYPE_F_(item->value) * conversion[units][X];
                         FitW_(render_ctx) =
   			    TYPE_F_(item->value) * conversion[units][X];
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		    }
		break;

	    case LEVELS_BLUE:
		rcb->levels_rgb[Ids_BLUE] = item->value;
		break;
		
	    case LEVELS_GRAY:
		rcb->levels_gray = item->value;
		break;

	    case LEVELS_GREEN:
		rcb->levels_rgb[Ids_GREEN] = item->value;
		break;

	    case LEVELS_RED:
		rcb->levels_rgb[Ids_RED] = item->value;
		break;

	    case PROTOCOL:
		Proto_(render_ctx) = item->value;
		rcb->protocol = item->value;
		break;

	    case PS_FLAGS:
		PsFlags_(render_ctx) = item->value;
		break;

	    case PUNCH_1:
		rcb->punch1 = TYPE_F_(item->value);
		break;
		
	    case PUNCH_2:
		rcb->punch2 = TYPE_F_(item->value);
		break;

            case SHARPEN:
                rcb->sharpen = TYPE_F_(item->value);
                break;
 
	    case RENDER_MODE:
		rcb->render_mode = item->value;
		break;

	    case ROI:
		rcb->roi = item->value;
		break;

	    case ROTATE_ANGLE:
		rcb->angle = TYPE_F_(item->value);
		rcb->rotate_mode    = Ids_Rotate;   /* ??? */
		break;

	    case ROTATE_MODE:
		rcb->rotate_mode = item->value;
		break;

	    case ROTATE_OPTIONS:
		rcb->rotate_options = item->value;

	    case SCALE_MODE:
		rcb->scale_mode = item->value;
		break;

	    case SCALE_OPTIONS:
		rcb->scale_options = item->value;
		break;

	    case SOURCE_GRAVITY:
		break;

	    case WINDOW_GRAVITY:
		break;

	    case X_SCALE:
		rcb->x_scale = TYPE_F_(item->value);
		rcb->scale_mode = Ids_Scale;	    /* ??? */
		break;

	    case Y_SCALE:
		rcb->y_scale = TYPE_F_(item->value);
		rcb->scale_mode = Ids_Scale;	    /* ??? */
		break;

	    case FLIP:
		rcb->flip_options = item->value;	    
		break;

	    case UNITS:
		units = item->value;
		break;

	    default:
		ChfSignal(1,IdsX_InvItmCod);
		break;
	    }

#ifdef TRACE
printf( "Leaving Routine IdsApplyItmlst in module IDS_RENDERING_MGMT \n");
#endif
    return(render_ctx);
}

/*******************************************************************************
**  ExportRendering
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a buffer and export the image data from the rendered 
**	fid.
**
**  FORMAL PARAMETERS:
**
**      rendering   - pointer to Ids_Rendering structure
**	render_ctx  - pointer to RenderContext structure
**	fid	    - rendered image frame identifier
**
**  FUNCTION VALUE:
**
**      rendering - rendering structure passed in
**
*******************************************************************************/
static IdsRendering *ExportRendering(rendering,render_ctx,fid)
IdsRendering	    *rendering;
RenderContext	     render_ctx;
unsigned long fid;
{
    int	     bytcnt;			/* Number of bytes EXPORTED by ISL  */
    int	     size;
    int	     width,height;
    int	     spectral_type;
    int	     bits_per_component;
    int	     component_count;
    int	     i;
    int	     sixel_lines;
    char    *base;
    unsigned long polarity;
    IdsRenderCallback rcb = PropRnd_(render_ctx);

#ifdef TRACE
printf( "Entering Routine ExportRendering in module IDS_RENDERING_MGMT \n");
#endif
    /*
    **  Choose a GC according to brightness polarity and image depth.
    */
    GetIsl_( fid, Img_BrtPolarity, polarity, 0 );
  
    switch(rendering->type)
    {
    case Ids_Fid:
	/*
	**  Just stuff the rendered fid into the rendering structure.
	*/
	Hrfid_(rendering) = fid;
	Hrnfid_(rendering) = fid;
	break;

    case Ids_XImage:
#ifndef IDS_NOX
        Hrnfid_(rendering) = fid;
	Hximage_(rendering) = Image_(render_ctx);
        SetImageGC( rendering, render_ctx, polarity );
#else
	ChfStop(1,IdsX_XNotInUse);
#endif

	break;

   case Ids_Pixmap:
#ifndef IDS_NOX
        Hrnfid_(rendering) = fid;
	Hximage_(rendering) = Image_(render_ctx);
        SetImageGC( rendering, render_ctx, polarity );
        ExportPixmapRendering(rendering,render_ctx,fid);
#else
	ChfStop(1,IdsX_XNotInUse);
#endif
        break;

    case Ids_Sixel:
        Hrnfid_(rendering) = fid;
	ExportSixelRendering(rendering,render_ctx,fid);
	break;

    case Ids_PostScript:
        Hrnfid_(rendering) = fid;
	GetIsl_( fid, Img_PixelsPerLine, width,  0 );
	GetIsl_( fid, Img_NumberOfLines, height, 0 );
	GetIsl_( fid, Img_SpectType, spectral_type, 0 );


        if ( spectral_type == ImgK_ClassBitonal ||
             spectral_type == ImgK_ClassGrayscale )
	  {
            SetBitsPerComponent_( bits_per_component, iGRA_(rcb) );
	  }
        else
	  {
            SetBitsPerComponent_( bits_per_component, iRGB_(rcb)[0] );
	  }

        GetIsl_( fid, Img_NumberOfComp, component_count, 0);

	/*
	**  Compute the expected size of the Postscript buffer depending on
	**  the postscript format requested.
	*/
	size = ((width * bits_per_component + 7) / 8); /* Bytes per scanline */
	if (!(PsFlags_(render_ctx) & Ids_SerialBinaryEncoding))
	    {
		/*
		**  For HEX Ascii encoding, the number of output bytes is equal
                **  to the number of bytes per scanline * 2
		*/
		size *= 2; 
	    }

	/*
	** Add number of LF characters added per scanline and multiply by the
	** number of scanlines and the number of components. This is the total
        ** number of image data bytes generated.
	*/
	size += ((size / POSTSCRIPT_MAXCHAR) + 1);
	size *= height * component_count;
	/*
	**  Add PostScript overhead bytes
	*/
        switch (spectral_type)
        {
            case ImgK_StypeBitonal:
            case ImgK_StypeGrayscale:
		if (PsFlags_(render_ctx) & Ids_EncapsulatedPS)
		    size += ENCAPSULATED_MONO_POSTSCRIPT_OVERHEAD;
		else
		    size += MONO_POSTSCRIPT_OVERHEAD;
                break;
            case ImgK_StypeMultispect:
		if (PsFlags_(render_ctx) & Ids_EncapsulatedPS)
		    size += ENCAPSULATED_COLOR_POSTSCRIPT_OVERHEAD;
		else
		    size += COLOR_POSTSCRIPT_OVERHEAD;
                break;
            default:
                ChfSignal(1,IdsX_InvSpcTyp);
        }
	/*
	**  Allocate the buffer and generate Postscript output.
	*/
	rendering->type_spec_data.postscript.bytcnt = size;
	rendering->type_spec_data.postscript.bufptr = (char *) _ImgMalloc(size);
	IdsExportPs(fid, render_ctx, 
		    0,
		    rendering->type_spec_data.postscript.bufptr,
		    rendering->type_spec_data.postscript.bytcnt,
		    &bytcnt, PsFlags_(render_ctx), (long)0, (long)0);

	rendering->type_spec_data.sixel.bytcnt = bytcnt;
	break;

    default:
	ChfSignal(1,IdsX_InvPrtTyp);
    }

#ifdef TRACE
printf( "Leaving Routine ExportRendering in module IDS_RENDERING_MGMT \n");
#endif
    return(rendering);
}

/*****************************************************************************
**  ExportSixelRendering
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will export the rendering in SIXEL protocol.
**
**  FORMAL PARAMETERS:
**
**      rendering
**	render_ctx
**	fid
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
static void	     ExportSixelRendering(rendering, render_ctx, fid)
IdsRendering	    *rendering;
RenderContext	     render_ctx;
unsigned long fid;
{
    int	     width,height;
    int	     component_cnt;

    int	     bytcnt;			/* Number of bytes EXPORTED by ISL  */
    int	     size;
    int	     i;
    char    *base;
    int	    sixel_flag;

#ifdef TRACE
printf( "Entering Routine ExportSixelRendering in module IDS_RENDERING_MGMT \n");
#endif

    /*
    **  Determine SIXEL introducer based on device type. If device type is
    **  not found in table, the default SIXEL introducer is employed.
    */
    for (i = 0; i < ST_LENGTH - 1; i++)
	if (Device_(render_ctx) == sixel_table[i].device_type)
	    break;
    /*
    **	Get necessary ISL frame information.
    */
    GetIsl_(fid, Img_BPCListCnt,    component_cnt, 0);
    GetIsl_(fid, Img_PixelsPerLine, width,         0);
    GetIsl_(fid, Img_NumberOfLines, height,        0);
    /*
    **	Determine maximum buffer size for sixels and allocate a buffer of that
    **	extent. For color, extra overhead bytes are required to describe the
    **	colors and components on each scanline.
    */
    size = (((height + 5) / 6) * component_cnt) * (width + 1) +
		strlen(sixel_table[i].sixel_introducer) + strlen(EXIT_SIXEL) +
/*  This is added now because of a calculation on IMG_EXPORT_SIXELS.C   */
/*  There they provide a check for worst code possible (6) to be added  */
		    SIXEL_OVERHEAD_PAD;
    if (component_cnt > 1)
    {
	size += ((height + 5) / 6) * 2 * component_cnt + COLOR_SIXEL_OVERHEAD;
       /* Multiplier to accomodate 8 color low res color mapped sixel mode */
	size *= 5;
    }

    base = (char *) _ImgMalloc(size);
    /*
    **  Save buffer parameters in rendering structure.
    */
    rendering->type_spec_data.sixel.bytcnt = size;
    rendering->type_spec_data.sixel.bufptr = base;
    /*
    **  Store SIXEL data and appropriate ANSI control sequences in the
    **  rendered image buffer.
    */
    memcpy(base,sixel_table[i].sixel_introducer,
	   strlen(sixel_table[i].sixel_introducer));
    /* Look at template and set sixel_flag */
    if (Device_(render_ctx) == Ids_TmpltLj250Lr)
	sixel_flag = Ids_Lj250lr_Mode;
    IdsExportSixels(fid, 
		    0,
		    base + strlen(sixel_table[i].sixel_introducer),
		    size - strlen(sixel_table[i].sixel_introducer) - 
				 strlen(EXIT_SIXEL),
		    &bytcnt,sixel_flag,(long)0,(long)0);
    memcpy(base+strlen(sixel_table[i].sixel_introducer)+bytcnt,
	   EXIT_SIXEL,strlen(EXIT_SIXEL));

    rendering->type_spec_data.sixel.bytcnt = 
	bytcnt+strlen(sixel_table[i].sixel_introducer)+strlen(EXIT_SIXEL);

#ifdef TRACE
printf( "Leaving Routine ExportSixelRendering in module IDS_RENDERING_MGMT \n");
#endif
}

/*******************************************************************************
**  DeleteRendition
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deletes resources acquired during a previous rendering.
**
**  FORMAL PARAMETERS:
**
**      rendering - pointer to IdsR_Rendering structure
**
**  FUNCTION VALUE:
**
**  SIGNAL CODES:
**
**	IdsX_InvPrtTyp - invalid protocol type
**
*******************************************************************************/
static void	DeleteRendition(rendering)
IdsRendering	*rendering;
{
#ifdef TRACE
printf( "Entering Routine DeleteRendition in module IDS_RENDERING_MGMT \n");
#endif
    switch(rendering->type)
    {
    case Ids_Fid:
        /*
        ** If the rendered fid is diff to user supplied fid dele the rend fid
        */
        if (Hrnfid_(rendering) != 0 && Hrnfid_(rendering) != Hsfid_(rendering))
	    {
            ImgDeleteFrame(Hrnfid_(rendering));
	    Hrnfid_(rendering) = 0;
	    Hrfid_(rendering) = 0;
	    }
	break;

    case Ids_XImage:
        if (Hrnfid_(rendering) != 0 && Hrnfid_(rendering) != Hsfid_(rendering))
            ImgDeleteFrame(Hrnfid_(rendering));
	/*
	** Free the XImage structure only if IDS created it.
	** IDS only creates the XImage if there is no display head.
	*/
	if (Dpy_((RenderContext)Hrcb_(rendering)) == NULL)
	    _ImgFree((char *)Hximage_(rendering));
        Hximage_(rendering) = 0;
        Hrnfid_(rendering)  = 0;
	Hximage_(rendering) = 0;
	break;

    case Ids_Pixmap:
        if (Hrnfid_(rendering) != 0 && Hrnfid_(rendering) != Hsfid_(rendering))
            ImgDeleteFrame(Hrnfid_(rendering));
        Hrnfid_(rendering)   = 0;
	Hpixmap_(rendering)  = 0;
	Hximage_(rendering)  = 0;
        HpxlLst_(rendering)  = 0;
        HGC_(rendering)      = 0;
        HpxlCnt_(rendering)  = Hpsid_(rendering) = 0;
        HforePix_(rendering) = HbackPix_(rendering) = 0;
	break;

    case Ids_Sixel:
        if (Hrnfid_(rendering) != 0 && Hrnfid_(rendering) != Hsfid_(rendering))
            ImgDeleteFrame(Hrnfid_(rendering));
	_ImgFree(HsixBuf_(rendering));
        HsixBuf_(rendering) = NULL;
        HsixCnt_(rendering) = 0;
	break;

    case Ids_PostScript:
        if (Hrnfid_(rendering) != 0 && Hrnfid_(rendering) != Hsfid_(rendering))
            ImgDeleteFrame(Hrnfid_(rendering));
	_ImgFree(HposBuf_(rendering));
        HposBuf_(rendering) = NULL;
        HposCnt_(rendering) = 0;
	break;

    default:
	ChfSignal(1,IdsX_InvPrtTyp);
    }
#ifdef TRACE
printf( "Leaving Routine DeleteRendition in module IDS_RENDERING_MGMT \n");
#endif
}


/*****************************************************************************
**  ExportPixmapRendering
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will export the rendering in PIXMAP protocol.
**      The function hands over the rendering structure which contains the 
**      Pixmap. In bitonal case the pixmap is created with 1 plane deep
**      rather than the depth of the window. If the application has to use this
**      1 bit depth pixmap to put on to a window with depth > 1 then the 
**      GC passed in the rendering  struct should be used in the XCopyPlane
**      X call. 
**
**  FORMAL PARAMETERS:
**
**      rendering
**	render_ctx
**	fid
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
#ifndef IDS_NOX
static void    ExportPixmapRendering(rendering, ctx, fid)
IdsRendering	    *rendering;
RenderContext	     ctx;
unsigned long fid;
{
    IdsRenderCallbackStruct *rcb = PropRnd_(ctx);
    IdsHistogramData   histogram;
    RemapLut           remap_lut;
    GC     pixmapGC;
    XGCValues   gcv;
    unsigned long polarity, ret_fid = fid;
    unsigned long  new_fid, format;
    XColor black,white;

    /*
    ** Get the background and foreground pixels. This has to be moved in init
    */
    HbackPix_(rendering) = WhitePixel(Dpy_(ctx),0);
    HforePix_(rendering) = BlackPixel(Dpy_(ctx),0);
    
    /*
    ** Hand over list and count of pixels allocated 
    */
    HpxlLst_(rendering) = (unsigned long *)PxlLst_(ctx);
    HpxlCnt_(rendering) = PxlCnt_(ctx);

    /*
    **  For Ids_Pixmap protocol, create a pixmap for our image.
    */
    Hpixmap_(rendering)  = XCreatePixmap( Dpy_(ctx), 
                                          Win_(ctx),
                                          Hximage_(rendering)->width, 
                                          Hximage_(rendering)->height, 
                                          Hximage_(rendering)->depth );

    /*
    ** Set image GC according to image depth and brightness polarity
    */
    if( Hpixmap_(rendering) != 0 )
      {
       if( Hximage_(rendering)->format == ZPixmap )
        {
        /*
        **  Load pixmap, Use common GC, image is continuous tone. 
        */
        XPutImage( Dpy_(ctx), Hpixmap_(rendering), 
	                      HGC_(rendering),
		              Hximage_(rendering), 0, 0, 0, 0, 
			      Hximage_(rendering)->width,
			      Hximage_(rendering)->height);
        }
       else
        {
        /*
        **  Image is bitonal We need a temporary GC for loading the pixmap; 
        **  can't use one of our window GC  because the drawable now is pixmap
        **  may not be of the same depth as window the pixmap has to display
	*/
        gcv.foreground  = HforePix_(rendering);
        gcv.background  = HbackPix_(rendering);
        gcv.plane_mask  = gcv.foreground ^ gcv.background;
        format          = Hximage_(rendering)->format;
        /*
        ** Somehow we wre forced to do this
        */
        Hximage_(rendering)->format = XYPixmap;
        pixmapGC = XCreateGC( Dpy_(ctx), Hpixmap_(rendering), 
                       GCForeground | GCBackground | GCPlaneMask, &gcv );
        XPutImage( Dpy_(ctx), Hpixmap_(rendering), 
                     pixmapGC,
                     Hximage_(rendering), 0, 0, 0, 0, 
                     Hximage_(rendering)->width,
                     Hximage_(rendering)->height);
        XFreeGC( Dpy_(ctx), pixmapGC );
        /*
        ** But the Ximage format same as it was before.
        */
        Hximage_(rendering)->format = format;
        }
      }
#ifdef TRACE
printf( "Leaving Routine ExportPixmapRendering in module IDS_RENDERING_MGMT \n");
#endif
  }
#endif

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
#ifndef IDS_NOX
static void 	SetImageGC( rendering, ctx, polarity )  
    IdsRendering	*rendering;
    RenderContext	 ctx;
    unsigned long	 polarity;
{
      XGCValues   gcv;

#ifdef TRACE
printf( "Entering Routine SetImageGC in module IDS_RENDERING_MGMT \n");
#endif
      /*
      ** Get the background and foreground pixels. This has to be moved in init
      */
      if (Dpy_(ctx) == NULL)
    	{   	
    	HbackPix_(rendering) = 1;
    	HforePix_(rendering) = 0;
    	}	
      else	
	{	
	HbackPix_(rendering) = WhitePixel(Dpy_(ctx),0);
	HforePix_(rendering) = BlackPixel(Dpy_(ctx),0);
	}
      /*
      ** Set up the GC 
      */	
      gcv.foreground  = HforePix_(rendering);
      gcv.background  = HbackPix_(rendering);

      /*
      ** Set image GC according to image depth,window id and brightness polarity
      */
      if( Win_(ctx) != 0 )
	if( Hximage_(rendering)->format == ZPixmap )
	/*
	** Create a GC with window as the drawable. If appl has not passed
	** window id no GC is created. Image is continuous tone 
	*/
		HGC_(rendering) = XCreateGC( Dpy_(ctx), Win_(ctx), 
					    GCForeground | GCBackground, &gcv );
	else
	{
	    /*
	    ** Image is bitonal.
	    */
	    if( polarity == ImgK_ZeroMaxIntensity )
	    {
	    gcv.plane_mask = gcv.foreground ^ gcv.background;
	    HGC_(rendering) = XCreateGC( Dpy_(ctx), Win_(ctx),
                              GCForeground | GCBackground | GCPlaneMask, &gcv );
	    }		    
	    else
	    {    
	    gcv.foreground  = HbackPix_(rendering);
	    gcv.background  = HforePix_(rendering);
	    gcv.plane_mask  = gcv.foreground ^ gcv.background;
	    HGC_(rendering) = XCreateGC( Dpy_(ctx), Win_(ctx),
                              GCForeground | GCBackground | GCPlaneMask, &gcv );
	    }
	}
#ifdef TRACE
printf( "Leaving Routine SetImageGC in module IDS_RENDERING_MGMT \n");
#endif
}
#endif

/*****************************************************************************
**  PrepareFidForRend
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**	returns void
**
*****************************************************************************/
static void PrePareFidForRend( ctx )
RenderContext    ctx;
{
   IdsRenderCallback rcb = PropRnd_(ctx);
   unsigned long  imgCmpLen, imgCmpLev, cmp_lvl[3]; 
   unsigned long  fid, i, pxl_bits, cmp_cnt, cmp_len[3];

#ifdef TRACE
printf( "Entering Routine PrepareFidForRend in module IDS_RENDERING_MGMT \n");
#endif

/*   
**  NOTE:   The commented out code appears to be irrelevant, except that with
**	    it in place, rendition when the protocol is Fid has a bug in it.
**
**	    The only thing that really needs to be done is that scanline
**	    padding should be set to 32.
**
**fid = iFid_(rcb);
*/
   /*
   **  Get per-spectral-component information.
   */
/*   ImgGet_( fid, Img_NumberOfComp, cmp_cnt, 0 );
**   for( pxl_bits = 0, i = 0; i < cmp_cnt; i++ )
**        {
**        ImgGet_( fid, Img_QuantLevelsPerComp, imgCmpLev, i );
**        ImgGet_( fid, Img_ImgBitsPerComp, imgCmpLen, i );
**        cmp_lvl[i] =    imgCmpLev;
**        cmp_len[i] =    imgCmpLen;
**        pxl_bits  +=    imgCmpLen;
**        }
*/
    Pad_(ctx) = 32;
/*    FitL_(ctx) = 1 << pxl_bits;
**    rcb->x_pels_per_bmu = 1.0;
**    rcb->y_pels_per_bmu = 1.0;
**    PxlStr_(ctx)   = pxl_bits;
*/
#ifdef TRACE
printf( "Leaving Routine PrepareFidForRend in module IDS_RENDERING_MGMT \n");
#endif
}
