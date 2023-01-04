
/***********************************************************************
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
**      This module contains code which creates, accesses and manages the IDS
**	presentation surface object.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0, DECwindows V1.0
**
**  AUTHOR(S):
**
**      John Weber
**
**  CREATION DATE:
**
**      December 27, 1987
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

/*
**  Include files
*/
#include <img/ChfDef.h>			    /* Condition handling functions */

#include <img/IdsStatusCodes.h>

#include <img/ImgDef.h>			    /* ISL Definitions		    */

#ifdef IDS_NOX
#include <ids__widget_nox.h>                /* IDS internal definitions     */
#else
#include <ids__widget.h>
#endif

#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif


/*
**  Table of contents
*/
#ifdef NODAS_PROTO
#if defined(__VMS) || defined(VMS)
RenderContext		 IDS$CREATE_PRESENT_SURFACE();
RenderContext		 IDS$SET_SURFACE_ATTRIBUTES();
RenderContext		 IDS$GET_SURFACE_ATTRIBUTES();
void			 IDS$DELETE_PRESENT_SURFACE();
#endif

unsigned long int	 IdsCreatePresentSurface();
unsigned long int	 IdsSetSurfaceAttributes() ;
unsigned long int	 IdsGetSurfaceAttributes() ;
void			 IdsDeletePresentSurface();

void			 IdsVerifyPs();

static RenderContext	 IDS_COPY_TEMPLATE_ATTRIBUTES();
static RenderContext	 IDS_COPY_WS_ATTRIBUTES();
static RenderContext	 IDS_COPY_WINDOW_ATTRIBUTES();
static void              InitializeForXie();
#else
PROTO(static RenderContext IDS_COPY_TEMPLATE_ATTRIBUTES, (RenderContext /*psid*/, int /*template*/));
PROTO(static RenderContext IDS_COPY_WS_ATTRIBUTES, (RenderContext /*ctx*/));
PROTO(static RenderContext IDS_COPY_WINDOW_ATTRIBUTES, (RenderContext /*psid*/));
PROTO(static void InitializeForXie, (RenderContext /*ctx*/));
#endif


#define BPI 1200                        /* BMUs per inch                    */
#define BPMM (1200/25.4)                /* BMUs per MM                      */
  
/*
**  MACRO definitions
*/
#define RcbEntry_(device,x_pels_per_bmu,y_pels_per_bmu) \
    {0,				/* Reason	    */	\
     0,				/* Event	    */	\
     0,				/* Render Mode	    */	\
     0,				/* Render Scheme    */	\
     0,				/* Protocol	    */	\
     0,				/* FID		    */	\
     0,				/* ROI		    */	\
     0,				/* Rotate Mode	    */	\
     0,				/* Rotate Options   */	\
     0,				/* Angle	    */	\
     0,				/* Flip Options	    */	\
     0,				/* Scale Mode	    */	\
     0,				/* Scale Options    */	\
     0,				/* X Scale Factor   */	\
     0,				/* Y Scale Factor   */	\
     x_pels_per_bmu,		/* X pels per BMU   */	\
     y_pels_per_bmu,		/* Y pels per BMU   */	\
     0,				/* Punch Factor 1   */	\
     0,				/* Punch Factor 2   */	\
     0,				/* Dither Algorithm */	\
     0,				/* Dither Threshold */	\
     0,				/* Levels Gray	    */	\
     0,				/* Levels Red	    */	\
     0,				/* Levels Green	    */	\
     0,				/* Levels Blue	    */	\
     0,				/* Fit Levels	    */	\
     0,				/* Fit Width	    */	\
     0,				/* Fit Height	    */  \
     0,                         /* Xieimage         */  \
     0,                         /* Rotate Width     */  \
     0,                         /* Rotate Height    */  \
     0,                         /* Rotate Fill 1    */  \
     0,                         /* Rotate Fill 2    */  \
     0}                         /* Rotate Fill 3    */  \

 
#define Template_(device,levels,levels_grey,levels_red,levels_green,levels_blue,protocol,height,width,depth,scanline_modulo,Z_bits_per_pixel,rendering_class)\
       {device,					/* IDS device type	*/  \
        0,                                      /* IDS or XIE switch    */  \
        0,                                      /* error function       */  \
        0,                                      /* save mode for wid    */  \
        0,                                      /* IDS, XIE cmpres mode */  \
        0,                                      /* IDS, XIE corg mode   */  \
        0,                                      /* IDS, switch mode     */  \
        NULL,					/* Pipe Descriptor	*/  \
	&IDS__AR_RCB_TEMPLATE_TABLE[device],    /* Proposed RCB		*/  \
	NULL,					/* Pixel alloc struct	*/  \
	NULL,					/* Pixel index list	*/  \
	0,					/* Pixel index count	*/  \
	NULL,					/* Pallette index list	*/  \
	0,					/* Pallette index count	*/  \
	0,					/* Colormap mode	*/  \
	0,					/* Colormap Update	*/  \
	NULL,					/* appl color list	*/  \
	NULL,					/* Pixel match data	*/  \
	0,					/* Color space		*/  \
	0,					/* Match limit		*/  \
	0,					/* Gray  limit		*/  \
	NULL,					/* XImage		*/  \
	NULL,					/* Requantize item list */  \
	NULL,					/* Tone scale item list */  \
	NULL,					/* Dither item list     */  \
	0,					/* Requantize levels    */  \
	0,					/* Tone scale levels    */  \
	0,                                                                  \
	0,                                                                  \
	levels_grey,							    \
	levels_red,							    \
	levels_green,							    \
	levels_blue,							    \
	0,					/* Fit Levels		*/  \
	0,					/* Fit Width		*/  \
	0,					/* Fit Height		*/  \
	protocol,							    \
	rendering_class,						    \
	NULL,					/* X11 Display ID	*/  \
	NULL,					/* X11 Screen pointer	*/  \
	NULL,					/* X11 Visual pointer	*/  \
	0,					/* X11 Window ID	*/  \
	width,								    \
	height,								    \
	depth,								    \
	0,					/* X11 Colormap ID	*/  \
	levels,					/* No of colormap cells */  \
	scanline_modulo,						    \
	Z_bits_per_pixel,						    \
	0}
/*
**  Equated Symbols
*/

/*
**  External References
*/
#ifdef NODAS_PROTO
extern void *_ImgCalloc();	    /* ISL calloc() routine		    */
extern void  _ImgFree();	    /* ISL free() routine		    */
#endif

/*
**	Local Storage
*/

/*
**  This table is an ordered list of all valid PRESENTATION SURFACE item code
**  name strings.  It is used by IdsNameToIndex to translate name strings
**  passed into CREATE_PRESENTATION_SURFACE into a unique table index.
**
**  NOTE: Changes made to this table must maintain the following characteristics
**	  1) The table must be ordered.
**	  2) The constants defined after each table entry must reflect the
**	     corresponding string's offset in the table.
*/
static char *ps_items[] = {
    IdsNdisplayDepth,			/* Depth			    */
#define DISPLAY_DEPTH	    0
    IdsNdisplayHeight,			/* Display Height		    */
#define DISPLAY_HEIGHT	    1
    IdsNdisplayWidth,			/* Display Width		    */
#define DISPLAY_WIDTH	    2
    IdsNgrid,				/* Grid Type			    */
#define GRID		    3
    IdsNlevelsBlue,			/* Gray Levels			    */
#define LEVELS_BLUE	    4
    IdsNlevelsGray,			/* Gray Levels			    */
#define LEVELS_GRAY	    5
    IdsNlevelsGreen,			/* Gray Levels			    */
#define LEVELS_GREEN	    6
    IdsNlevelsRed,			/* Gray Levels			    */
#define LEVELS_RED	    7
    IdsNprotocol,			/* Protocol			    */
#define PROTOCOL	    8
    IdsNrenderingClass,			/* RenderingClass		    */
#define RENDERING_CLASS	    9
    IdsNtemplate,			/* Device Template		    */
#define TEMPLATE	    10
    IdsNunits,				/* Unit Specifier		    */
#define UNITS		    11
    IdsNwindowHeight,			/* Window Height		    */
#define WINDOW_HEIGHT	    12
    IdsNwindowWidth,			/* Window Width			    */
#define WINDOW_WIDTH	    13
    IdsNwindowX,			/* Window ULX			    */
#define WINDOW_X	    14
    IdsNwindowY,			/* Window ULY			    */
#define WINDOW_Y	    15
    IdsNworkstation,			/* Workstation Template		    */
#define WORKSTATION	    16
    IdsNwsWindow,			/* Workstation window		    */
#define WS_WINDOW	    17
    IdsNxDistance,			/* X Resolution			    */
#define X_DISTANCE	    18
    IdsNyDistance			/* Y Resolution			    */
#define Y_DISTANCE	    19
};
#define PS_ITEMS (sizeof(ps_items) / sizeof(char *))

static IdsRenderCallbackStruct IDS__AR_RCB_TEMPLATE_TABLE[] = {
/*
**  First entry is blank
*/
    RcbEntry_(0,0,0),
/*
**   Terminal devices (in general, undocumented)
*/
    RcbEntry_(Ids_TmpltVt125,	0.06500,    0.03166),
    RcbEntry_(Ids_TmpltVt240,	0.06500,    0.03166),
    RcbEntry_(Ids_TmpltVt330,	0.06833,    0.06666),
    RcbEntry_(Ids_TmpltVt340,	0.06833,    0.06666),
/*
**  Workstations
*/
    RcbEntry_(Ids_TmpltVsII,	0.07083,    0.06500),
    RcbEntry_(Ids_TmpltVsGPX,	0.07083,    0.06500),
    RcbEntry_(Ids_TmpltVs2000,	0.07083,    0.06500),
    RcbEntry_(Ids_TmpltVs3200,	0.07083,    0.06500),
    RcbEntry_(Ids_TmpltVs3500,	0.07083,    0.06500),
    RcbEntry_(Ids_TmpltVs3520,	0.07083,    0.06500),
    RcbEntry_(Ids_TmpltVs3540,	0.07083,    0.06500),
/*
**  Printers
*/
    RcbEntry_(Ids_TmpltLa50,	0.12000,    0.06000),
    RcbEntry_(Ids_TmpltLa75,	0.12000,    0.06000),
    RcbEntry_(Ids_TmpltLa100,	0.12000,    0.06000),
    RcbEntry_(Ids_TmpltLn03s,	0.25000,    0.25000),
    RcbEntry_(Ids_TmpltLn03r,	0.25000,    0.25000),
    RcbEntry_(Ids_TmpltLps20,	0.25000,    0.25000),
    RcbEntry_(Ids_TmpltLps40,	0.25000,    0.25000),
    RcbEntry_(Ids_TmpltLcg01,	0.12000,    0.06000),
    RcbEntry_(Ids_TmpltLj250,	0.15000,    0.15000),
    RcbEntry_(Ids_TmpltLa210,	0.12000,    0.06000),
    RcbEntry_(Ids_TmpltMonoPs,	1.0,	    1.0),
    RcbEntry_(Ids_TmpltColorPs,	1.0,	    1.0),
    RcbEntry_(Ids_TmpltLj250Lr,	0.07500,    0.07500),
    RcbEntry_(Ids_TmpltDefault, 0.07083,    0.06500),
    RcbEntry_(0,0,0)
};

/*
**      Module wide static storage.  Note that these are all
**      non-sharable.  Therefore, if IDS is built as a sharable
**      library, all images will receive their own copy of
**      these data areas.
*/

static RenderContextStruct IDS__AR_TEMPLATE_TABLE[] = {
/*
**  First entry is blank
*/
    Template_(0,0,0,0,0,0,0,0,0,0,0,0,0),
/*
**   Terminal devices (in general, undocumented)
*/
    Template_(Ids_TmpltVt125,2,2,0,0,0,Ids_Sixel,240,800,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltVt240,2,2,0,0,0,Ids_Sixel,240,800,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltVt330,2,2,0,0,0,Ids_Sixel,480,800,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltVt340,2,2,0,0,0,Ids_Sixel,480,800,1,1,1,Ids_Bitonal),
/*
**  Workstations XImage
*/
    Template_(Ids_TmpltVsII,2,2,0,0,0,Ids_XImage,864,1024,1,32,1,Ids_Bitonal),
    Template_(Ids_TmpltVsGPX,256,256,256,256,256,Ids_XImage,864,1024,8,32,8,Ids_Color),
    Template_(Ids_TmpltVs2000,2,2,0,0,0,Ids_XImage,864,1024,1,32,1,Ids_Bitonal),
    Template_(Ids_TmpltVs3200,2,2,0,0,0,Ids_XImage,864,1024,1,32,1,Ids_Bitonal),
    Template_(Ids_TmpltVs3500,2,2,0,0,0,Ids_XImage,864,1024,1,32,1,Ids_Bitonal),
    Template_(Ids_TmpltVs3520,256,256,256,256,256,Ids_XImage,1024,1280,24,32,32,Ids_Color),
    Template_(Ids_TmpltVs3540,256,256,256,256,256,Ids_XImage,1024,1280,24,32,32,Ids_Color),


/*
** Printers
*/
    Template_(Ids_TmpltLa50,2,2,0,0,0,Ids_Sixel,792,1152,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltLa75,2,2,0,0,0,Ids_Sixel,792,1152,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltLa100,2,2,0,0,0,Ids_Sixel,720,1152,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltLn03s,2,2,0,0,0,Ids_Sixel,3150,2400,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltLn03r,2,2,0,0,0,Ids_PostScript,3150,2400,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltLps20,2,2,0,0,0,Ids_PostScript,3150,2400,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltLps40,2,2,0,0,0,Ids_PostScript,3150,2400,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltLcg01,2,2,0,0,0,Ids_Sixel,1152,720,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltLj250,8,2,2,2,2,Ids_Sixel,1980,1530,1,1,1,Ids_Color),
    Template_(Ids_TmpltLa210,2,2,0,0,0,Ids_Sixel,1152,720,1,1,1,Ids_Bitonal),
    Template_(Ids_TmpltMonoPs,256,256,0,0,0,Ids_PostScript,1,1,8,8,8,Ids_GrayScale),
    Template_(Ids_TmpltColorPs,16777216,256,256,256,256,Ids_PostScript,1,1,24,8,24,Ids_Color),
    Template_(Ids_TmpltLj250Lr,8,2,2,2,2,Ids_Sixel,990,765,1,1,1,Ids_Color),
    Template_(Ids_TmpltDefault,256,256,256,256,256,Ids_XImage,1024,1280,24,32,32,Ids_Color),
/*
**  End of the list
*/
    Template_(0,0,0,0,0,0,0,0,0,0,0,0,0)
};

/*******************************************************************************
**  IDS$CREATE_PRESENT_SURFACE
**
**  FUNCTIONAL DESCRIPTION:
**
**      Create and initialize a presentation surface definition based on
**	user specified attributes.
**
**  FORMAL PARAMETERS:
**
**      itmlst - pointer to an array of IdsItmlst2 structures containing
**		 presentation surface attribute values.
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
**      psid - presentation surface identifier. A pointer to the resulting 
**	       RenderContext block.
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
#if defined(__VMS) || defined(VMS)
RenderContext		 IDS$CREATE_PRESENT_SURFACE(itmlst)
IdsItmlst2	*itmlst;
{
    return ((RenderContext)IdsCreatePresentSurface(itmlst));
}
#endif
unsigned long int	 IdsCreatePresentSurface(itmlst)
IdsItmlst2	*itmlst;
{
    RenderContext	ctx;
    IdsRenderCallback	rcb;

#ifdef TRACE
printf( "Entering Routine IdsCreatePresentSurface in module IDS_PS_MGMT \n");
#endif

    ctx = (RenderContext) _ImgCalloc(1,sizeof(RenderContextStruct));
    /*
    **	Initialize nonzero fields in the RenderContext
    */
    rcb = PropRnd_(ctx) = (IdsRenderCallback)
                                 _ImgCalloc(1,sizeof(IdsRenderCallbackStruct));
    rcb  =  PropRnd_(ctx);
    /*
    ** All initialization other then for workstation has to be done over here
    */
    PxlAlloc_(ctx) = NULL;
    PxlLst_(ctx)   = NULL;
    PxlCnt_(ctx)   = 0;
    PltLst_(ctx)   = NULL;
    PltCnt_(ctx)   = 0;
    CmpMode_(ctx)  = Ids_ShareColors;
    ClrMap_(ctx)   = 0;
    CmpUpd_(ctx)   = 0;
/*    CSpace_(ctx)   = */
    PxlDat_(ctx)   = NULL;
    MchLim_(ctx)   = 0;
    GraLim_(ctx)   = 0;
    RqLst_(ctx)    = NULL;
    TsLst_(ctx)    = NULL;
    DiLst_(ctx)    = NULL;
    GRA_(ctx)      = 0;
    RGB_(ctx)[0] = RGB_(ctx)[1] = RGB_(ctx)[2] = 0;
    FitW_(ctx) = FitH_(ctx) = 0;
    Proto_(ctx)  = Ids_XImage;
    RClass_(ctx) = Ids_Bitonal;
    iProto_(rcb)  = Ids_XImage;
    iRGB_(rcb)[0] = iRGB_(rcb)[1] = iRGB_(rcb)[2] = 0;

    /*
    ** Default do the Rendition Computations on the
    ** client side (Use ISL/IPS NOT XIE) of the X Wire
    */
    Process_(ctx) = Ids_IslClient;

    /*
    **	Set attributes specified by item list parameters and return address of
    **	PSB as the presentation surface ID value.
    **  ctx = itmlst == 0 ? ctx : IdsSetSurfaceAttributes(ctx,itmlst);
    */
     if ( itmlst != 0 ) 
       ctx = (RenderContext) IdsSetSurfaceAttributes(ctx,itmlst);

#ifdef TRACE
printf( "Leaving Routine IdsCreatePresentSurface in module IDS_PS_MGMT \n");
#endif

    return( (unsigned long int)ctx);
}

/************************************************************************
**  IDS$SET_SURFACE_ATTRIBUTES
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set fields in an existing presentation surface based on user values
**	supplied in an item list.
**
**  FORMAL PARAMETERS:
**
**      psid -	    Presentation Surface Identifier, a pointer to the PSB
**	itmlst -    Item list, pointer to an array of IdsItmlst2 structures
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
**      psid -	    Returns the address of the modified PSB
**
**  SIGNAL CODES:
**
**	IdsX_InvArgCnt
**	IdsX_InvItmCod
**	IdsX_InvUnt
**	IdsX_UndUntVal
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/
/*DECwindows entry point definition                                           */
#if defined(__VMS) || defined(VMS)
RenderContext		 IDS$SET_SURFACE_ATTRIBUTES(psid,itmlst)
RenderContext		 psid;
IdsItmlst2	*itmlst;
{
   return ((RenderContext)IdsSetSurfaceAttributes(psid,itmlst));
}
#endif

unsigned long int	 IdsSetSurfaceAttributes(psid,itmlst)
RenderContext		 psid;
IdsItmlst2	*itmlst;
{
#define X 0
#define Y 1
    RenderContext	     render_ctx = psid;
    IdsRenderCallback	     rcb = PropRnd_(render_ctx);
    IdsItmlst2		    *item;
    int			     units = Ids_UnitsPxl;
    double		     conversion[Ids_UnitsMax][2];

#ifdef TRACE
printf( "Entering Routine IdsSetSurfaceAttributes in module IDS_PS_MGMT \n");
#endif
                
    for (item = (IdsItmlst2 *) itmlst;  item->item_name != 0;  item++)

	switch (IdsNameToIndex(item->item_name,ps_items,PS_ITEMS))
	{
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

	    case RENDERING_CLASS:
		RClass_(render_ctx) = item->value;
		break;

	    case PROTOCOL:
	        Proto_(render_ctx) = item->value;
                rcb->protocol = item->value;
		break;

	    case TEMPLATE:
		Device_(render_ctx) = item->value;
		IDS_COPY_TEMPLATE_ATTRIBUTES(psid,item->value);
/*
**  if the template calls for encasulated postscript, set ps flag
*/
		if ((Device_(render_ctx) == Ids_TmpltMonoPs) ||
		    (Device_(render_ctx) == Ids_TmpltColorPs))
		      PsFlags_(render_ctx) |= Ids_EncapsulatedPS;
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
		break;

	    case WORKSTATION:
#ifdef IDS_NOX
		ChfStop(1,IdsX_XNotInUse);
#else
 		Dpy_(render_ctx) = (Display *) item->value;
		IDS_COPY_WS_ATTRIBUTES(psid);
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
#endif
		break;

	    case WS_WINDOW:
#ifdef IDS_NOX
		ChfStop(1,IdsX_XNotInUse);
#else
		Win_(render_ctx) = item->value;
		IDS_COPY_WINDOW_ATTRIBUTES(psid);
#endif
		break;

	    case GRID:
		break;

	    case DISPLAY_DEPTH:
		WinD_(render_ctx) = item->value;
		break;

	    case UNITS:
		units = item->value;
		break;

/*	    case WINDOW_X:
**
**		switch (units)
**		{
**		    case Ids_UnitsPxl:
**		    case Ids_UnitsBMU:
**			PS_ULX_(psid) = item->value / conversion[units][X];
**			break;
**		    case Ids_UnitsInch:
**		    case Ids_UnitsMM:
**			PS_ULX_(psid) = 
**			    TYPE_F_(item->value)/conversion[units][X];
**			break;
**		    default:
**			ChfSignal(1,IdsX_UndUntVal);
**		}
**
**		break;
**
**	    case WINDOW_Y:
**		switch (units)
**		{
**		    case Ids_UnitsPxl:
**		    case Ids_UnitsBMU:
**			PS_ULY_(psid) = item->value / conversion[units][Y];
**			break;
**		    case Ids_UnitsInch:
**		    case Ids_UnitsMM:
**			PS_ULY_(psid) = 
**			    TYPE_F_(item->value)/conversion[units][Y];
**			break;
**		    default:
**			ChfSignal(1,IdsX_UndUntVal);
**		}
**
**		break;
*/
	    case WINDOW_WIDTH:
		switch (units)
		{
		    case Ids_UnitsPxl:
		    case Ids_UnitsBMU:
			WinW_(render_ctx) = ((float) item->value)/conversion[units][X];
			break;
		    case Ids_UnitsInch:
		    case Ids_UnitsMM:
			WinW_(render_ctx) = ((float) item->value)/conversion[units][X];
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		break;

	    case WINDOW_HEIGHT:
		switch (units)
		{
		    case Ids_UnitsPxl:
		    case Ids_UnitsBMU:
			WinH_(render_ctx) = ((float) item->value)/conversion[units][Y];
			break;
		    case Ids_UnitsInch:
		    case Ids_UnitsMM:
			WinH_(render_ctx) = ((float) item->value)/conversion[units][Y];
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		break;

	    case DISPLAY_WIDTH:
		switch (units)
		{
		    case Ids_UnitsPxl:
		    case Ids_UnitsBMU:
			WinW_(render_ctx) = ((float) item->value)/conversion[units][X];
			break;
		    case Ids_UnitsInch:
		    case Ids_UnitsMM:
			WinW_(render_ctx) = ((float) item->value)/conversion[units][X];
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		break;

	    case DISPLAY_HEIGHT:
		switch (units)
		{
		    case Ids_UnitsPxl:
		    case Ids_UnitsBMU:
			WinH_(render_ctx) = ((float) item->value)/conversion[units][Y];
			break;
		    case Ids_UnitsInch:
		    case Ids_UnitsMM:
			WinH_(render_ctx) = ((float)item->value)/conversion[units][Y];
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		break;

	    case X_DISTANCE:
		switch (units)
		{
		    case Ids_UnitsPxl:
			ChfSignal(1,IdsX_InvUnt);
			break;
		    case Ids_UnitsBMU:
		    	rcb->x_pels_per_bmu = 1.0/((float)item->value);
			break;
		    case Ids_UnitsInch:
			rcb->x_pels_per_bmu = 1.0/(((float)item->value)/1200.0);
			break;
		    case Ids_UnitsMM:
			rcb->x_pels_per_bmu = 
			    1.0/(((float)item->value)/(1200.0*25.4));
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		conversion[Ids_UnitsBMU][X]  = rcb->x_pels_per_bmu;
		conversion[Ids_UnitsInch][X] = rcb->x_pels_per_bmu * 1200.0;
		conversion[Ids_UnitsMM][X] =   rcb->x_pels_per_bmu * 1200.0 * 25.4;
		break;

	    case Y_DISTANCE:
		switch (units)
		{
		    case Ids_UnitsPxl:
			ChfSignal(1,IdsX_InvUnt);
			break;
		    case Ids_UnitsBMU:
			rcb->y_pels_per_bmu = 1.0/((float) item->value);
			break;
		    case Ids_UnitsInch:
			rcb->y_pels_per_bmu = 
			    1.0/(
				((float) item->value) / 1200.0
				);
			break;
		    case Ids_UnitsMM:
			rcb->y_pels_per_bmu = 
			    1.0/(
				((float) item->value)/(1200.0*25.4)
				);
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		conversion[Ids_UnitsBMU][Y]  = rcb->y_pels_per_bmu;
		conversion[Ids_UnitsInch][Y] = rcb->y_pels_per_bmu * 1200.0;
		conversion[Ids_UnitsMM][Y] =   rcb->y_pels_per_bmu * 1200.0 * 25.4;
		break;

	    default:
		ChfSignal(1,IdsX_InvItmCod);
	}

#ifdef TRACE
printf( "Leaving Routine IdsSetSurfaceAttributes in module IDS_PS_MGMT \n");
#endif

    return((unsigned long int)psid);
}

/*******************************************************************************
**  IDS$GET_SURFACE_ATTRIBUTES
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine returns the values of presentation surface attributes as
**	specified by a user item list.
**
**  FORMAL PARAMETERS:
**
**      psid -	    Presentation Surface Identifier, address of the PSB from 
**		    which attributes are to be returned.
**
**	itmlst -    Address of an array of IdsItmlst2 structures which 
**		    define what attributes are to be returned where.
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
**      psid -	    Returns the address of the PSB from which attribute values
**		    were extracted.
**
**  SIGNAL CODES:
**
**	IdsX_UniMplFnc in V1 and V2, It now functions!
**	IdsX_UndUntVal	- Undefined Units of measure Value (pixel,bmu,mm,inch)
**	IdsX_InvUnt	- Invalid Unit of measure Value Used
**	IdsX_InvItmCod	- Invalid Item Code
**
**  SIDE EFFECTS:
**
**      none
**
*******************************************************************************/
/*DECwindows entry point definition                                           */
#if defined(__VMS) || defined(VMS)
RenderContext		 IDS$GET_SURFACE_ATTRIBUTES(psid,itmlst)
RenderContext		 psid;
IdsItmlst2	*itmlst;
{
    return ((RenderContext)IdsGetSurfaceAttributes(psid,itmlst));
}
#endif
unsigned long int	 IdsGetSurfaceAttributes(psid,itmlst)
RenderContext		 psid;
IdsItmlst2	*itmlst;
{

    RenderContext	     render_ctx = psid;
    IdsRenderCallback	     rcb = PropRnd_(render_ctx);
    IdsItmlst2		    *item;
    int			     units = Ids_UnitsPxl;

#ifdef TRACE
printf( "Entering Routine IdsGetSurfaceAttributes in module IDS_PS_MGMT \n");
#endif

    for (item = (IdsItmlst2 *) itmlst;  item->item_name != 0;  item++)

	switch (IdsNameToIndex(item->item_name,ps_items,PS_ITEMS))
	{
	    case LEVELS_BLUE:
		item->value = rcb->levels_rgb[Ids_BLUE];
		break;
		
	    case LEVELS_GRAY:
		item->value = rcb->levels_gray;
		break;

	    case LEVELS_GREEN:
		item->value = rcb->levels_rgb[Ids_GREEN];
		break;

	    case LEVELS_RED:
		item->value = rcb->levels_rgb[Ids_RED];
		break;

	    case RENDERING_CLASS:
		item->value = RClass_(render_ctx);
		break;

	    case PROTOCOL:
		item->value = Proto_(render_ctx);
		break;

	    case TEMPLATE:
		item->value = Device_(render_ctx);
		break;

	    case WORKSTATION:
		item->value = (int) Dpy_(render_ctx);
		break;

	    case WS_WINDOW:
		item->value = Win_(render_ctx);
		break;

	    case GRID:
		break;

	    case DISPLAY_DEPTH:
		item->value = WinD_(render_ctx);
		break;

	    case UNITS:
		item->value = units;
		break;

/*	    case WINDOW_X:
**
**		switch (units)
**		{
**		    case Ids_UnitsPxl:
**		    case Ids_UnitsBMU:
**			item->value = PS_ULX_(psid)
**			break;
**		    case Ids_UnitsInch:
**		    case Ids_UnitsMM:
**			PS_ULX_(psid) = 
**			    item->value = TYPE_F_(item->value);
**			break;
**		    default:
**			ChfSignal(1,IdsX_UndUntVal);
**		}
**
**		break;
**
**	    case WINDOW_Y:
**		switch (units)
**		{
**		    case Ids_UnitsPxl:
**		    case Ids_UnitsBMU:
**			item->value = PS_ULY_(psid);
**			break;
**		    case Ids_UnitsInch:
**		    case Ids_UnitsMM:
**			TYPE_F_(item->value) =
**			    PS_ULY_(psid);
**			break;
**		    default:
**			ChfSignal(1,IdsX_UndUntVal);
**		}
**
**		break;
*/
	    case WINDOW_WIDTH:
		switch (units)
		{
		    case Ids_UnitsPxl:
		    case Ids_UnitsBMU:
			item->value = WinW_(render_ctx);
			break;
		    case Ids_UnitsInch:
		    case Ids_UnitsMM:
			item->value = WinW_(render_ctx);
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		break;

	    case WINDOW_HEIGHT:
		switch (units)
		{
		    case Ids_UnitsPxl:
		    case Ids_UnitsBMU:
			item->value = WinH_(render_ctx);
			break;
		    case Ids_UnitsInch:
		    case Ids_UnitsMM:
			item->value = WinH_(render_ctx);
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		break;

	    case DISPLAY_WIDTH:
		switch (units)
		{
		    case Ids_UnitsPxl:
		    case Ids_UnitsBMU:
			item->value = WinW_(render_ctx);
			break;
		    case Ids_UnitsInch:
		    case Ids_UnitsMM:
			item->value = WinW_(render_ctx);
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		break;

	    case DISPLAY_HEIGHT:
		switch (units)
		{
		    case Ids_UnitsPxl:
		    case Ids_UnitsBMU:
			item->value = WinH_(render_ctx);
			break;
		    case Ids_UnitsInch:
		    case Ids_UnitsMM:
			item->value = WinH_(render_ctx);
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		break;

	    case X_DISTANCE:
		switch (units)
		{
		    case Ids_UnitsPxl:
			ChfSignal(1,IdsX_InvUnt);
			break;
		    case Ids_UnitsBMU:
		    	item->value = rcb->x_pels_per_bmu;
			break;
		    case Ids_UnitsInch:
			item->value = rcb->x_pels_per_bmu;
			break;
		    case Ids_UnitsMM:
			item->value = rcb->x_pels_per_bmu;
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		break;

	    case Y_DISTANCE:
		switch (units)
		{
		    case Ids_UnitsPxl:
			ChfSignal(1,IdsX_InvUnt);
			break;
		    case Ids_UnitsBMU:
			item->value = rcb->y_pels_per_bmu;
			break;
		    case Ids_UnitsInch:
			item->value = rcb->y_pels_per_bmu;
			break;
		    case Ids_UnitsMM:
			item->value = rcb->y_pels_per_bmu;
			break;
		    default:
			ChfSignal(1,IdsX_UndUntVal);
		}
		break;

	    default:
		ChfSignal(1,IdsX_InvItmCod);
	}

#ifdef TRACE
printf( "Leaving Routine IdsGetSurfaceAttributes in module IDS_PS_MGMT \n");
#endif

    return((unsigned long int)psid);

} /* End of IdsGetSurfaceAttributes */

/*******************************************************************************
**  IDS$DELETE_PRESENT_SURFACE
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deletes the resources allocated by a presentation surface
**	definition. This amounts to the return of the PSB to the dynamic memory
**	pool.
**
**  FORMAL PARAMETERS:
**
**      psid -	Presentation Surface Identifier, address of the RenderContext 
**		structure which is to be deleted.
**
**  IMPLICIT INPUTS:
**
**      PropRnd_(psid) - Rendering Structure allocated 
**			    in IdsCreatePresentSurface
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
#if defined(__VMS) || defined(VMS)
void		 IDS$DELETE_PRESENT_SURFACE(psid)
RenderContext	 psid;
{
    IdsDeletePresentSurface(psid);
}
#endif
void		 IdsDeletePresentSurface(psid)
RenderContext	     psid;
{
#ifdef TRACE
printf( "Entering Routine IdsDeletePresentSurface in module IDS_PS_MGMT \n");
#endif

/* This is really rendering structure allocated in IdsCreatePresentSurface */
    _ImgCfree(PropRnd_(psid)); 
/* This is really render context allocated in IdsCreatePresentSurface */
    _ImgCfree(psid);

#ifdef TRACE
printf( "Leaving Routine IdsDeletePresentSurface in module IDS_PS_MGMT \n");
#endif

}

/*******************************************************************************
**  IdsVerifyPs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Verify that a presentation surface identifier is actually a presentation
**	surface.
**
**  FORMAL PARAMETERS:
**
**      psid - Presentation Surface Identifier, address of IdsR_Psb structure
**	       to verify.
**
**  FUNCTION VALUE:
**
*******************************************************************************/
void		    IdsVerifyPs(psid)
RenderContext	    psid;
{
/*
       TBS IdsVerifyPs(psid);
*/
}
/*
void		IdsVerifyPs(psid)
RenderContext	psid;
{
}
*/

/*******************************************************************************
**  IDS_COPY_TEMPLATE_ATTRIBUTES
**
**  FUNCTIONAL DESCRIPTION:
**
**      Copy attributes from the specified presentation surface template
**	into the given presentation surface.
**
**  FORMAL PARAMETERS:
**
**      psid -	    presentation surface identifier, address of PSB which is
**		    the target of the copy.
**	template -  device template to copy.
**
**  FUNCTION VALUE:
**
**      psid -	    address of the PSB which was modified.
**
*******************************************************************************/
static RenderContext	 IDS_COPY_TEMPLATE_ATTRIBUTES(psid,template)
RenderContext		 psid;
int			 template;
{
    IdsRenderCallback rcb = PropRnd_(psid);

#ifdef TRACE
printf( "Entering Routine IDS_COPY_TEMPLATE_ATTRIBUTES in module IDS_PS_MGMT \n");
#endif

    *rcb = *IDS__AR_TEMPLATE_TABLE[template].proposed_rcb;
    *psid = IDS__AR_TEMPLATE_TABLE[template];

    /*
    ** Default do the crunching at the client side
    */
    Process_(psid) = Ids_IslClient;

    PropRnd_(psid) = rcb;

#ifdef TRACE
printf( "Leaving Routine IDS_COPY_TEMPLATE_ATTRIBUTES in module IDS_PS_MGMT \n");
#endif

    return (psid);
}

/************************************************************************
**  IDS_COPY_WS_ATTRIBUTES
**
**  FUNCTIONAL DESCRIPTION:
**
**      Copy attributes of the specified workstation by calling XLIB functions
**	which return display attributes. The PSB contains the X Display 
**	Idenifier which will be the source of the attributes.
**
**  FORMAL PARAMETERS:
**
**      ctx -	Presentation Surface Identifier, address of the RenderContext 
**	        structure which is the target of the copy operation.
**
**  IMPLICIT INPUTS:
**
**      Dpy_(ctx)  X Display Identifier which is the source of attribute 
**		    information.
**
**  FUNCTION VALUE:
**
**      ctx -	Returns the value of the presentation surface identifier which
**		was modified.
**
*******************************************************************************/
#ifndef IDS_NOX   
static RenderContext	 IDS_COPY_WS_ATTRIBUTES(ctx)
RenderContext		 ctx;
{
    Screen *screen;
    XImage *tmp;
    IdsRenderCallback rcb;

#ifdef TRACE
printf( "Entering Routine IDS_COPY_WS_ATTRIBUTES in module IDS_PS_MGMT \n");
#endif

    screen = XDefaultScreenOfDisplay(Dpy_(ctx));
    rcb = PropRnd_(ctx);

    Cells_(ctx)  = CellsOfScreen(screen);
    Pad_(ctx)	 = BitmapPad(Dpy_(ctx));
    Scr_(ctx)	 = screen;
    Vis_(ctx)    = DefaultVisualOfScreen( screen );
    /*
    **  Convert visual class into a rendered image class we can support.
    */
    RClass_(ctx) = Vis_(ctx)->class == GrayScale   ? Ids_GrayScale
                         : Vis_(ctx)->class == PseudoColor ? Ids_Color
                         : Vis_(ctx)->class == DirectColor ? Ids_Color
                         : Vis_(ctx)->class == TrueColor   ? Ids_Color
                         : Ids_Bitonal;
    /*
    **  mm/bmu * pixels/mm = pixels/bmu
    */
    rcb->x_pels_per_bmu = (MM_per_BMU) * 
			    (
				(float) WidthOfScreen(screen) 
                                        /
				(float) WidthMMOfScreen(screen)
			    );

    rcb->y_pels_per_bmu	= (MM_per_BMU) * 
			    (
				(float) HeightOfScreen(screen) 
				    / 
					(float) HeightMMOfScreen(screen)
			    );

    /*
    **  Round up the number of colormap entries to a power of 2,
    **  Find the levles supported by the visual
    */
    SetBitsPerComponent_(FitL_(ctx), Vis_(ctx)->map_entries);
    FitL_(ctx) = 1 << FitL_(ctx);
    if( Vis_(ctx)->class == DirectColor || Vis_(ctx)->class == TrueColor )
            FitL_(ctx) = FitL_(ctx) * FitL_(ctx) * FitL_(ctx);

    /*
    **  Create a skeleton XImage structure, and copy it to our instance record.
    */
    Image_(ctx) = XCreateImage( Dpy_(ctx), Vis_(ctx), 
				DefaultDepthOfScreen(screen),
				ZPixmap, 0, 0, 0, 0, Pad_(ctx), 1 );


    PxlStr_(ctx)   = Image_(ctx)->bits_per_pixel;
    Cmap_(ctx)     = DefaultColormapOfScreen(screen); 
    
/* When we Eventually do this BUT NOT TODAY!
**    InitializeForXie( ctx );
*/

#ifdef TRACE
printf( "Leaving Routine IDS_COPY_WS_ATTRIBUTES in module IDS_PS_MGMT \n");
#endif

    return(ctx);

}
#endif


/*******************************************************************************
**  IDS_COPY_WINDOW_ATTRIBUTES
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function copies the attributes of an X window into RenderContext 
**	attributes which define the display region. 
**
**  FORMAL PARAMETERS:
**
**      psid -	Presentation Surface Identifier, address of the RenderContext
**		structure which defines this presentation surface.
**
**  IMPLICIT INPUTS:
**
**      Dpy_(psid) -		XLIB display identifier which specifies the
**				physical display which contains the window. This
**				parameter is necessary for XLIB calls.
**
**	Win_(psid) -		XLIB window identfier which specified the window
**				who's geometry will define the presentation 
**				surface region.
**
**  FUNCTION VALUE:
**
**      psid -	The address of the RenderContext structure which was modified.
**
*******************************************************************************/
#ifndef IDS_NOX
static RenderContext	 IDS_COPY_WINDOW_ATTRIBUTES(psid)
RenderContext		 psid;
{
    Window root;		    /* Root window of specified window	    */
    Window child;		    /* Mapped child window (not useful)	    */
    unsigned int border_width;	    /* Border width of window (not useful)  */
    unsigned int width,height;
    int x,y;

#ifdef TRACE
printf( "Entering Routine IDS_COPY_WINDOW_ATTRIBUTES in module IDS_PS_MGMT \n");
#endif

    /*
    **	Get necessary information about window.
    */
    XGetGeometry(
	Dpy_(psid),		    /* X display identifier		    */
	Win_(psid),		    /* X window identifier		    */
	&root,			    /* Returned root window identifier	    */
	&x,&y,			    /* X,Y coordinates of this window	    */
				    /* relative to its parent		    */
	&width,&height,             /* Width and height of window in pels   */
	&border_width,		    /* Width of window border in pels	    */
	&WinD_(psid));		    /* Z depth of window		    */
    WinW_(psid) = (short) width;
    WinH_(psid) = (short) height;
    /*
    **	Now, transform ULX, ULY coordinates which are relative to the parent
    **	window to be relative to the root window which should reflect the 
    **	offset within the display.
    */
    XTranslateCoordinates(
	Dpy_(psid),		    /* X display identifier		    */
	Win_(psid),		    /* X source window identifier	    */
	root,			    /* X destination window identifier	    */
	x,y,			    /* Source coordinates		    */
	&x,&y,			    /* Resulting destination coordinates    */
	&child);		    /* Identifier of mapped child	    */

#ifdef TRACE
printf( "Leaving Routine IDS_COPY_WINDOW_ATTRIBUTES in module IDS_PS_MGMT \n");
#endif
    return(psid);
}
#endif

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
#ifndef IDS_NOX
static void  InitializeForXie( ctx )
RenderContext    ctx;
{
    int    opcode, event0, error0, function;
    char       *text, **names;
    IdsRenderCallback rcb   = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine InitializeForXie in module IDS_PS_MGMT \n");
#endif

    /*
    ** Connect to Xie if it exists
    */
    if( XQueryExtension( Dpy_(ctx), XieS_Name, &opcode, &event0, &error0 ) )
	{
	intCompute_(rcb) = Ids_XieServer;
        /*    
        **  Enable completion events.
        */
        XieSelectEvents( Dpy_(ctx), XieM_ComputationEvent | 
                                     XieM_DisplayEvent  | XieM_PhotofloEvent );
	}
#ifdef TRACE
printf( "Leaving Routine InitializeForXie in module IDS_PS_MGMT \n");
#endif
}
#endif
