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
/*
**	If not being used by a widget programmer, include DECwindows.
*/
#ifndef IDSIMAGE_H
#define IDSIMAGE_H

#ifndef IDS_NOX

/* turn XIE off for customers */

#ifndef IDS
typedef unsigned long XieImage;
typedef unsigned long XiePhotomap;
#endif

#if defined(__VMS) || defined(VMS)
#include <decw$include:Xlib.h>
#include <decw$include:Intrinsic.h>
#ifdef IDS
#include <XieAppl.h>    /* XIE public definitions */
#include <XieLib.h>     /* XIE public definitions */
#endif
#else
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#ifdef IDS
#include <X11/extensions/XieAppl.h>    /* XIE public definitions */
#include <X11/extensions/XieLib.h>     /* XIE public definitions */
#endif
#endif
#endif

/*
**  Equated Symbols
*/
#define MM_per_BMU  25.40005/1200.0 /* conversion factor: mm to BMU	    */
#define CVT_STRING_SIZE	1000	    /* maximum type converter string size   */


/*
**  MACRO definitions
*/
    /*
     *	$SETARG: an alternative to the XtSetArg macro when using an array
     *
     *	    arg - address of array
     *	    ind - next array index to use ( auto-incremented on return )
     *	    n   - resource name to change
     *	    v   - new value
     */
#define SETARG_( arg, ind, n, v ) \
	( (arg)[(ind)].name = (n), (arg)[(ind)].value = (XtArgVal)(v), ++ind )

/*******************************************************************************
** TYPE_C_, TYPE_F_, TYPE_I_, TYPE_P_, TYPE_S_, TYPE_U_
**
**  DESCRIPTION:
**
**	Allows store/retrieve of the specified variable type to/from any 
**	longword-size variable in spite of its actual type.
**
**  FORMAL PARAMETERS:
**
**	v  - the name of the longword-size variable to access. 
**
**  RETURN VALUE:
**
**	TYPE_C_	    - (char    )v    
**	TYPE_F_	    - (float   )v    
**	TYPE_I_	    - (int     )v    
**	TYPE_P_	    - (int    *)v    
**	TYPE_S_	    - (char   *)v    
**	TYPE_U_	    - (unsigned)v    
**
**  Note:   'v' must be a valid operand for a "& (address-of)" operator.
**
**		    			==oOo==
**
**	This macro type casts the address of "v" as a pointer to the 
**	specified type, and then allows access to "v" as though it was 
**	originally declared as that type.
*/

#define TYPE_C_(v)    (*((	   char	     *)&(v)))
#define TYPE_F_(v)    (*((	   float     *)&(v)))
#define TYPE_I_(v)    (*((  signed long int  *)&(v)))
#define TYPE_P_(v)    (*((unsigned long int **)&(v)))
#define TYPE_S_(v)    (*((unsigned char	    **)&(v)))
#define TYPE_U_(v)    (*((unsigned long int  *)&(v)))
#define TYPE_L_(v)    (*((unsigned      int  *)&(v)))

    /* image crunching at ..*/
#define Ids_IslClient	1   /* image processing: at client using ISL        */
#define Ids_XieServer	2   /* image processing: at server using XIE        */

    /* image saved fid or xieimage compression mode */
#define Ids_UnCompress      1     /* uncompressed   mode PCM               */
#define Ids_CompressG31D    2     /* compresssed bitonal mode              */
#define Ids_CompressG32D    3     /* compresssed bitonal mode              */
#define Ids_CompressG42D    4     /* compresssed bitonal mode              */
#define Ids_CompressDCT     5     /* compresssed color   mode              */

    /* image saved fid or xieimage componenet space mode */
#define Ids_BandByPixel     1     /* Band By Pixel                          */
#define Ids_BandByPlane     2     /* Band By Plane                          */
#define Ids_BitByPlane      3     /* Bit By Plane                           */

    /* color constants */
#define Ids_MaxComponents 3 /* maximum number of spectral components	    */
#define Ids_RED		0   /* index of red   RGB component		    */
#define Ids_GREEN	1   /* index of green RGB component		    */
#define Ids_BLUE	2   /* index of blue  RGB component		    */

    /* image class constants */
#define Ids_Bitonal	1   /* rendered image: monochromatic, 2 intensities */
#define Ids_GrayScale	2   /* rendered image: monochromatic, N intensities */
#define Ids_Color	3   /* rendered image: multispectral, N intensities */

    /* presentation level protocols */
#define Ids_XImage	1   /* export final rendering to an XImage struct   */
#define Ids_Pixmap	2   /* export final rendering to an X server pixmap */
#define Ids_Sixel	3   /* export final rendering to a sixel buffer	    */
#define Ids_PostScript	4   /* export final rendering to a PostScript buffer*/
#define Ids_Fid		5   /* export final rendering as an ISL frame	    */
#define Ids_Photomap	6   /* export final xie rendering to a photomap	    */
#define Ids_Window	7   /* export final xie rendering to the window only*/

    /* rotate modes */
#define Ids_NoRotate	0   /* disable rotation during rendering	    */
#define Ids_Rotate	1   /* rotate by application specified angle	    */
#define Ids_BestFit	2   /* rotate to match aspect ratio of image/window */

    /* color modes  */
#define Ids_ShareColors	  0   /* use the shared colors model for allocation */
#define Ids_PrivateColors 1   /* use application supplied private colors    */

    /* flip options 
#define Ids_FlipVertical    ImgM_FlipVertical
#define Ids_FlipHorizontal  ImgM_FlipHorizontal
    */

#define Ids_FlipHorizontal  1
#define Ids_FlipVertical    2

    /* scale modes  */
#define Ids_NoScale	0   /* disable scaling during rendering		    */
#define Ids_Scale	1   /* scale by application specified factors	    */
#define Ids_Physical	2   /* scale to match original scanned size	    */
#define Ids_AspectOnly	3   /* scale to match image/display pel aspect ratio*/
#define Ids_FitWithin	4   /* scale to fit within window: keep aspect ratio*/
#define Ids_FitWidth	5   /* scal to fit window width:  keep aspect ratio*/
#define Ids_FitHeight	6   /* scale to fit window height: keep aspect ratio*/
#define Ids_Flood	7   /* scale to fill window:	 ignore aspect ratio*/

    /* Dither/Requantize modes  */
#define Ids_Requantize	0		     /* Requantize, == don't dither */
#define Ids_Clustered ImgK_DitherClustered /* Clustered dither algorithm  */
#define Ids_BlueNoise ImgK_DitherBluenoise /* Blue Noise dither algorithm */
#define Ids_Dispersed ImgK_DitherDispersed /* Dispersed dither algorithm  */

    /* RenderMode */
#define Ids_Passive	    0x00000000 /* Mode: re-render only if changed   */
#define Ids_Normal	    0x00000001 /* Mode: render using IDS model	    */
#define Ids_Override	    0x00000002 /* Mode: override IDS model (manual) */
#define Ids_Purge	    0x00000003 /* Mode: purge current rendering	    */
#define Ids_Abort	    0x00000004 /* Mode: abort proposed rendering    */

    /* RenderScheme: things to do mask */
#define Ids_Decompress	       0x00000001 /* Scheme: decompress image	    */
#define Ids_UseROI	       0x00000002 /* Scheme: ROI		    */
#define Ids_UseClassCvt	       0x00000004 /* Scheme: cvt color to grayscale */
#define Ids_UseScale_1	       0x00000008 /* Scheme: scale (before rotation)*/
#define Ids_UseAngle	       0x00000010 /* Scheme: rotation angle	    */
#define Ids_UseFlip	       0x00000020 /* Scheme: flip (mirror)	    */
#define Ids_UseScale_2	       0x00000040 /* Scheme: scale (after  rotation)*/
#define Ids_UseToneScale       0x00000080 /* Scheme: tone-scale adjustment  */
#define Ids_UseSharpen         0x00000100 /* Scheme: sharpening adjustment  */
#define Ids_UseDither	       0x00000200 /* Scheme: dither		    */
#define Ids_UseConvert         0x00000400 /* Scheme: class convert          */
#define Ids_UseReversePolarity 0x00000800 /* Scheme: reverse polarity       */
#define Ids_UseNativeFormatCvt 0x00001000 /* Scheme: Cvt to intrlvd by plane */
#define Ids_UseOldFormatCvt    0x00002000 /* Scheme: Cvt to intrlvd by pixel */
#define Ids_UsePlaneSwapByPtr  0x00004000 /* Scheme: Swap red and blue ptrs  */
#define Ids_UsePlaneSwapByData 0x00008000 /* Scheme: Swap red and blue planes*/
#define Ids_UseArea 	       0x00010000 /* Scheme: Area		     */
#define Ids_UseConstrain       0x00020000 /* Scheme: Constrain		     */
#define Ids_UseIROI	       0x00040000 /* Scheme: Internal ROI	     */


    /* save image format protocols */
#define Ids_SaveNone    0x00000000 /* return no final rend fid            */
#define Ids_SaveFid     0x00000001 /* return final rend as a fid           */
#define Ids_SaveXieimg  0x00000002 /* return final rend as a Xieimage      */
#define Ids_SavePhoto   0x00000004 /* return final rend as a photo         */
#define Ids_SaveDec     0x00000008 /* return final rend as an DDIF file    */   
#define Ids_SaveXimage  0x00000010 /* return final rend as a XImage struc  */
    
    /* Xie related masks used in work notify callback */
#define Ids_XiePipeStart    0x00000000 /* Xie Pipe Execution start          */
#define Ids_XiePipeDone     0x0000FFFF /* Xie Pipe Execution done           */

    /* WorkNotify Function: next function to execute */
#define Ids_FunctionROI		 1  /* ROI				    */
#define Ids_FunctionDecompress   2  /* decompress			    */
#define Ids_FunctionClassCvt	 4  /* convert to lower spectral class	    */
#define Ids_FunctionScale	 6  /* scale				    */
#define Ids_FunctionRotate	 8  /* rotate				    */
#define Ids_FunctionFlip	10  /* flip				    */
#define Ids_FunctionToneScale	12  /* tone-scale adjustment		    */
#define Ids_FunctionSharpen	14  /* sharpen, enhance high frequencies    */
#define Ids_FunctionDither	16  /* dither to lower Z resolution	    */
#define Ids_FunctionPad		18  /* pad scanlines (bitonal)		    */
#define Ids_FunctionHistogram	20  /* histogram of color (grayscale) usage */
#define Ids_FunctionAllocColor	22  /* allocate color cells		    */
#define Ids_FunctionRemapColor	24  /* re-map image pixels to pixel indices */
#define Ids_FunctionExport	26  /* export image per protocol	    */
#define Ids_FunctionCspConvert  28  /* component space conversion           */
#define Ids_FunctionMatchColor  30  /* match color cells                    */
#define Ids_FunctionCombine     32  /* reverse polarity w/ ImgCombineFrame  */
#define Ids_FunctionExportPipeTap 34  /* tap export pipe to get fid         */

    /* Gravity definitions  */
#define Ids_NoGravity	0   /* display: using specified/default coordinates */
#define Ids_Top		1   /* display: top side    of image or window	    */
#define Ids_Bottom	2   /* display: bottom side of image or window	    */
#define Ids_Right	4   /* display: right side  of image or window	    */
#define Ids_Left	8   /* display: left side   of image or window	    */
#define Ids_NorthEast   ( Ids_Top       | Ids_Right	)
#define Ids_NorthWest	( Ids_Top       | Ids_Left	)
#define Ids_SouthEast   ( Ids_Bottom    | Ids_Right	)
#define Ids_SouthWest	( Ids_Bottom    | Ids_Left	)
#define Ids_North	( Ids_NorthWest | Ids_NorthEast	)
#define Ids_South	( Ids_SouthWest | Ids_SouthEast	)
#define Ids_East	( Ids_NorthEast | Ids_SouthEast	)
#define Ids_West	( Ids_NorthWest | Ids_SouthWest	)
#define Ids_CenterHorz	( Ids_Left      | Ids_Right	)
#define Ids_CenterVert	( Ids_Top       | Ids_Bottom	)
#define Ids_Center	( Ids_CenterHorz| Ids_CenterVert)

#define Ids_RenderedCoordinates	1
#define Ids_WindowCoordinates	2

#define Ids_UnitsPxl		1
#define Ids_UnitsBMU		2
#define Ids_UnitsMM		3
#define Ids_UnitsInch		4
#define Ids_UnitsMax		5

#define Ids_TmpltVt125		 1
#define Ids_TmpltVt240		 2
#define Ids_TmpltVt330		 3
#define Ids_TmpltVt340		 4
#define Ids_TmpltVsII		 5
#define Ids_TmpltVsGPX		 6
#define Ids_TmpltVs2000		 7
#define Ids_TmpltVs3200		 8
#define Ids_TmpltVs3500		 9
#define Ids_TmpltVs3520		10
#define Ids_TmpltVs3540		11
#define Ids_TmpltLa50		12
#define Ids_TmpltLa75		13
#define Ids_TmpltLa100		14
#define Ids_TmpltLn03s		15
#define Ids_TmpltLn03r		16
#define Ids_TmpltLps20		17
#define Ids_TmpltLps40		18
#define Ids_TmpltLcg01		19
#define Ids_TmpltLj250		20
#define Ids_TmpltLa210		21
#define Ids_TmpltMonoPs         22
#define Ids_TmpltColorPs        23
#define Ids_TmpltLj250Lr	24
#define Ids_TmpltDefault        25
	
	
    /* IDS Presentation Surface Flags */
/* Hex Postscript or Ascii Postscript */
#define Ids_SerialBinaryEncoding 1
#define Ids_Lj250lr_Mode	 2
/* Encapsulated Postscript, for ColorPS and MonoPS Template	         */
/* Bit Mask used in IDS$RENDERING_MGMT.C, IDS$PS_MGMT.C, IDS$EXPORT_PS.C */
#define Ids_EncapsulatedPS	 4


    /* 
    **  Resource names
    */
	/* Render Image Class resources */
#define	IdsNimageBackground	"imageBackground"
#define	IdsNimageForeground	"imageForeground"
#define	IdsNframeDepth	        "frameDepth"
#define	IdsNframeWidth	        "frameWidth"
#define	IdsNframeHeight		"frameHeight"
#define	IdsNrenderMode		"renderMode"
#define	IdsNcomputeMode		"computeMode"
#define	IdsNrenderScheme	"renderScheme"
#define	IdsNsaveRendition	"saveRendition"
#define	IdsNcompressMode	"compressMode"
#define	IdsNcomporgMode		"comporgMode"
#define	IdsNprotocol		"protocol"
#define	IdsNfid			"fid"
#define IdsNxieimage            "xieimage"
#define	IdsNroi		        "roi"
#define	IdsNcolormapMode	"colormapMode"
#define	IdsNcolormapUpdate	"colormapUpdate"
#define	IdsNrotateMode		"rotateMode"
#define	IdsNrotateOptions	"rotateOptions"
#define	IdsNrotateAngle		"rotateAngle"
#define	IdsNrotateWidth		"rotateWidth"
#define	IdsNrotateHeight	"rotateHeight"
#define	IdsNrotateFirstFill	"rotateFirstFill"
#define	IdsNrotateSecondFill	"rotateSecondFill"
#define	IdsNrotateThirdFill	"rotateThirdFill"
#define	IdsNscaleMode	        "scaleMode"
#define	IdsNflipOptions		"flipOptions"
#define	IdsNscaleOptions	"scaleOptions"
#define	IdsNxScale	        "xScale"
#define	IdsNyScale	        "yScale"
#define	IdsNxPelsPerBMU		"xPelsPerBMU"
#define	IdsNyPelsPerBMU		"yPelsPerBMU"
#define	IdsNpunch1		"punch1"
#define	IdsNpunch2		"punch2"
#define	IdsNsharpen		"sharpen"
#define	IdsNditherAlgorithm	"ditherAlgorithm"
#define	IdsNditherThreshold	"ditherThreshold"
#define	IdsNpixelList		"pixelList"
#define	IdsNpixelCount		"pixelCount"
#define	IdsNpaletteList 	"paletteList"
#define	IdsNpaletteCount	"paletteCount"
#define	IdsNcolorSpace		"colorSpace"
#define	IdsNmatchLimit		"matchLimit"
#define	IdsNgrayLimit		"grayLimit"
#define	IdsNrenderingClass	"renderingClass"
#define	IdsNlevelsGray		"levelsGray"
#define	IdsNlevelsRed		"levelsRed"
#define	IdsNlevelsGreen		"levelsGreen"
#define	IdsNlevelsBlue		"levelsBlue"
#define	IdsNfitLevels		"fitLevels"
#define	IdsNfitWidth		"fitWidth"
#define	IdsNfitHeight		"fitHeight"
#define IdsNcopyFid		"copyFid"
#define IdsNrenderCallback	"renderCallback"
#define IdsNxieListCallback	"xieListCallback"
#define IdsNsaveImageCallback   "saveImageCallback"
#define IdsNworkNotifyCallback	"workNotifyCallback"
#define IdsNerrorCallback	"errorCallback"
	/* (Static) Image Class resources */
#define	IdsNsourceX	        "sourceX"
#define	IdsNsourceY		"sourceY"
#define	IdsNsourceWidth	        "sourceWidth"
#define	IdsNsourceHeight	"sourceHeight"
#define	IdsNsourceGravity       "sourceGravity"
#define	IdsNwindowGravity       "windowGravity"
#define	IdsNwindowX		"windowX"
#define	IdsNwindowY		"windowY"
#define	IdsNwindowWidth		"windowWidth"
#define	IdsNwindowHeight	"windowHeight"
#define IdsNscrollHorizontal	"scrollHorizontal"
#define IdsNscrollVertical	"scrollVertical"
#define IdsNscrollDynamic	"scrollDynamic"
#define IdsNexposeCallback	"exposeCallback"
#define IdsNviewCallback	"viewCallback"
#define IdsNdragCallback	"dragCallback"
#define IdsNzoomCallback	"zoomCallback"
	/* Panned Image Class resources -- none at present */

    /*
    **	Presentation surface attribute names
    */
#define IdsNdisplayDepth	"displayDepth"
#define IdsNdisplayHeight	"displayHeight"
#define IdsNdisplayWidth	"displayWidth"
#define IdsNgrid		"grid"
#define IdsNinteractive		"interactive"
#define IdsNprotocol		"protocol"
#define IdsNtemplate		"template"
#define IdsNunits		"units"
#define IdsNwindowHeight	"windowHeight"
#define IdsNwindowWidth		"windowWidth"
#define IdsNwindowX		"windowX"
#define IdsNwindowY		"windowY"
#define IdsNworkstation		"workstation"
#define IdsNwsWindow		"wsWindow"
#define IdsNxDistance		"xDistance"
#define IdsNyDistance		"yDistance"
    /*
    **	Hardcopy specific rendition attributes
    */
#define IdsNpsFlags		"psFlags"

    /*
    **  Resource classes
    */
	/* Render Image Class class names */
#define	IdsCImageBackground	"ImageBackground"
#define	IdsCImageForeground	"ImageForeground"
#define	IdsCFrameDepth	        "FrameDepth"
#define	IdsCFrameWidth	        "FrameWidth"
#define	IdsCFrameHeight		"FrameHeight"
#define	IdsCRenderMode		"RenderMode"
#define	IdsCComputeMode		"ComputeMode"
#define	IdsCRenderScheme	"RenderScheme"
#define	IdsCSaveRendition	"SaveRendition"
#define	IdsCCompressMode	"CompressMode"
#define	IdsCComporgMode		"ComporgMode"
#define	IdsCProtocol		"Protocol"
#define	IdsCFid			"Fid"
#define IdsCXieimage            "Xieimage"
#define	IdsCRoi		        "Roi"
#define IdsCColormapMode        "ColormapMode"
#define IdsCColormapUpdate      "ColormapUpdate"
#define	IdsCRotateMode		"RotateMode"
#define	IdsCRotateOptions	"RotateOptions"
#define	IdsCRotateAngle	        "RotateAngle"
#define	IdsCRotateWidth	        "RotateWidth"
#define	IdsCRotateHeight        "RotateHeight"
#define	IdsCRotateFirstFill	"RotateFirstFill"
#define	IdsCRotateSecondFill	"RotateSecondFill"
#define	IdsCRotateThirdFill	"RotateThirdFill"
#define	IdsCFlipOptions		"FlipOptions"
#define	IdsCScaleMode		"ScaleMode"
#define	IdsCScaleOptions	"ScaleOptions"
#define	IdsCScaleFactor		"ScaleFactor"
#define	IdsCPelsPerBMU		"PelsPerBMU"
#define	IdsCPunch1		"Punch1"
#define	IdsCPunch2		"Punch2"
#define	IdsCSharpen		"Sharpen"
#define	IdsCDitherAlgorithm	"DitherAlgorithm"
#define	IdsCDitherThreshold	"DitherThreshold"
#define IdsCPaletteList         "PaletteList"
#define IdsCPaletteCount        "PaletteCount"
#define	IdsCPixelList		"PixelList"
#define	IdsCPixelCount		"PixelCount"
#define	IdsCColorSpace		"ColorSpace"
#define	IdsCMatchLimit		"MatchLimit"
#define	IdsCGrayLimit		"GrayLimit"
#define	IdsCRenderingClass	"RenderingClass"
#define	IdsCLevels		"Levels"
#define	IdsCFitWidth		"FitWidth"
#define	IdsCFitHeight		"FitHeight"
#define IdsCCopyFid		"CopyFid"
	/* (Static) Image Class class names */
#define	IdsCSourceX	        "SourceX"
#define	IdsCSourceY		"SourceY"
#define	IdsCSourceWidth	        "SourceWidth"
#define	IdsCSourceHeight	"SourceHeight"
#define	IdsCSourceGravity       "SourceGravity"
#define	IdsCWindowGravity       "WindowGravity"
#define	IdsCWindowX		"WindowX"
#define	IdsCWindowY		"WindowY"
#define	IdsCWindowWidth		"WindowWidth"
#define	IdsCWindowHeight	"WindowHeight"
#define IdsCScrollDynamic	"ScrollDynamic"
#define IdsCScrollHorizontal	"ScrollHorizontal"
#define IdsCScrollVertical	"ScrollVertical"
	/* Panned Image Class class names -- none at present */

    /* 
    **  Resource representation types.
    */
#define IdsRFloat		"Float"
#define IdsRRenderMode		"RenderMode"
#define IdsRComputeMode		"ComputeMode"
#define IdsRRenderingClass	"RenderingClass"
#define IdsRProtocol		"Protocol"
#define IdsRColormapMode        "ColormapMode"
#define IdsRSaveRendition       "SaveRendition"
#define IdsRCompressMode        "CompressMode"
#define IdsRComporgMode         "ComporgMode"
#define IdsRRotateMode		"RotateMode"
#define IdsRRotateOptions	"RotateOptions"
#define IdsRFlipOptions		"FlipOptions"
#define IdsRScaleMode		"ScaleMode"
#define IdsRScaleOptions	"ScaleOptions"
#define IdsRDitherMode		"DitherMode"
#define IdsRColorSpace		"ColorSpace"
#define IdsRGravity		"Gravity"

    /* 
    **  Resource conversion strings
    */
#define IdsSPassive		"passive"
#define IdsSNormal		"normal"
#define IdsSOverride		"override"
#define IdsSPurge		"purge"
#define IdsSAbort		"abort"

#define IdsSBitonal		"bitonal"
#define IdsSGrayScale		"grayscale"
#define IdsSColor		"color"

#define IdsSXImage		"ximage"
#define IdsSPixmap		"pixmap"
#define IdsSSixel		"sixel"
#define IdsSPostScript		"postscript"
#define IdsSFid			"fid"
#define IdsSPhotomap            "photomap"

#define IdsSIslClient		"islclient"
#define IdsSXieServer		"xieserver"

#define IdsSSaveNone		"savenone"
#define IdsSSaveFid		"savefid"
#define IdsSSaveXieimg		"savexieimg"
#define IdsSSavePhoto	        "savephoto"
#define IdsSSaveDec	        "savedec"
#define IdsSSaveXimage	        "saveximage"

#define IdsSUnCompress		"uncompress"
#define IdsSBitonalG42d		"bitonalg42d"
#define IdsSColorDCT		"colordct"

#define IdsSBandByPixel		"bandbypixel"
#define IdsSBandByPlane		"bandbyplane"
#define IdsSBitByPlane 		"bitbyplane"

#define IdsSShareColors		"sharecolors"
#define IdsSPrivateColors	"privatecolors"

#define IdsSNoRotate		"norotate"
#define IdsSRotate		"rotate"
#define IdsSBestFit		"bestfit"
#define IdsSReverseEdgeFill	"reverseedgefill"

#define IdsSFlipVertical	"flipvertical"
#define IdsSFlipHorizontal	"fliphorizontal"

#define IdsSBilinear		"bilinear"
#define IdsSNearestNeighbor	"nearestneighbor"

#define IdsSNoScale		"noscale"
#define IdsSScale		"scale"
#define IdsSPhysical		"physical"
#define IdsSAspectOnly		"aspectonly"
#define IdsSFitWithin		"fitwithin"
#define IdsSFitWidth		"fitwidth"
#define IdsSFitHeight		"fitheight"
#define IdsSFlood		"flood"

#define IdsSSubsampleH		"subsamplehorizontal"
#define IdsSSubsampleV		"subsamplevertical"
#define IdsSSubsampleBoth	"subsamplehorizontal|subsamplevertical"
#define IdsSSaveH		"savehorizontal"
#define IdsSSaveV		"savevertical"
#define IdsSSaveBoth		"savehorizontal|savevertical"
#define IdsSSubsampleHSaveV	"subsamplehorizontal|savevertical"
#define IdsSSaveHSubsampleV	"savehorizontal|subsamplevertical"
#define IdsSReversePreference	"reversepreference"
#define IdsSDisablePreference	"disablepreference"

#define IdsSRequantize		"requantize"
#define IdsSClustered		"clustered"
#define IdsSBlueNoise		"bluenoise"
#define IdsSDispersed		"dispersed"

#define IdsSHLSSpace		"hlsspace"
#define IdsSLabSpace		"labspace"
#define IdsSLUVSpace		"luvspace"
#define IdsSRGBSpace		"rgbspace"
#define IdsSUVWSpace		"uvwspace"
#define IdsSYIQSpace		"yiqspace"

#define IdsSNoGravity		"nogravity"
#define IdsSTop			"top"
#define IdsSBottom		"bottom"
#define IdsSRight		"right"
#define IdsSLeft		"left"
#define IdsSCenterHorz		"centerhorizontal"
#define IdsSCenterVert		"centervertical"
#define IdsSCenter		"center"
#define IdsSNorth		"north"
#define IdsSSouth		"south"
#define IdsSEast		"east"
#define IdsSWest		"west"
#define IdsSNorthEast		"northeast"
#define IdsSNorthWest		"northwest"
#define IdsSSouthEast		"southeast"
#define IdsSSouthWest		"southwest"

    /*
    **  IDS widget class name strings
    */
#define IdsSClassRenderImage	"RenderImage"
#define IdsSClassImage		"Image"
#define IdsSClassPannedImage	"PannedImage"

    /*
    **  IDS MOTIF widget class name strings
    */
#define IdsSMotifClassRenderImage	"MotifRenderImage"
#define IdsSMotifClassImage		"MotifImage"
#define IdsSMotifClassPannedImage	"MotifPannedImage"
    /*
    **  Callback reasons 
    */
#define IdsCRNoChange	    4359900 /* Render: no change from current rend. */
#define IdsCRPurge	    4359901 /* Render: purging current rendering    */
#define IdsCRNormal	    4359902 /* Render: using IDS rendering model    */
#define IdsCRRealized	    4359903 /* Render: using IDS model via realize  */
#define IdsCRResized	    4359904 /* Render: using IDS model via resize   */
#define IdsCROverride	    4359905 /* Render: application spec'd rendering */
#define IdsCRViewChanged    4359906 /* Image:  image view changed in window */
#define IdsCRDragImage	    4359907 /* Panned: image panned to X,Y (dynamic)*/
#define IdsCRWorkNotify	    4359908 /* Render: rendering function pending   */
#define IdsCRXieList	    4359909 /* Render: list XIE functions           */
#define IdsCRError	    4359910 /* Render: Error occured                */
#define IdsCRSaveImage      4359911 /* Render: save rendered image          */
#define IdsCRZoomImage	    4359912 /* Panned: image zoom to X,Y,W,H (dynamic)*/
    /*
    **	IDS rendering callback structure.
    */
typedef struct _RenderCallback
    {
    int		    reason;		/* IDS callback reason		    */
#ifdef IDS_NOX
    unsigned long   event;              /* SHOULD NOT BE USED IN NO X11 MODE*/
#else
    XEvent	   *event;		/* X11 pointer to triggering event  */
#endif
    unsigned long   render_mode;	/* IDS rendering mode		    */
    unsigned long   render_scheme;	/* IDS rendering functions/sequence */
    unsigned long   protocol;		/* IDS protocol (ximage, pixmap...) */
    unsigned long   fid;		/* ISL frame-id of image to render  */
    unsigned long   roi;		/* ISL ROI-id			    */
    unsigned long   rotate_mode;	/* IDS rotate mode		    */
    unsigned long   rotate_options;	/* ISL rotate option flags	    */
    float	    angle;		/* ISL rotation angle		    */
    unsigned long   flip_options;	/* ISL flip option flags	    */
    unsigned long   scale_mode;		/* IDS scale mode		    */
    unsigned long   scale_options;	/* ISL scale option flags	    */
    float	    x_scale;		/* ISL scale X factor		    */
    float	    y_scale;		/* ISL scale Y factor		    */
    float	    x_pels_per_bmu;	/* IDS scale for X res. in pels/BMU */
    float	    y_pels_per_bmu;	/* IDS scale for Y res. in pels/BMU */
    float	    punch1;		/* IDS tone-scale factor	    */
    float	    punch2;		/* IDS tone-scale factor	    */
    float	    sharpen;		/* IDS sharpening factor	    */
    unsigned long   dither_algorithm;	/* ISL dither algorithm/requantize  */
    unsigned long   dither_threshold;	/* ISL dither M-factor/order	    */
    unsigned long   levels_gray;	/* IDS maximum levels of gray	    */
    unsigned long   levels_rgb[3];	/* IDS maximum levels of R,G,B	    */
    unsigned long   fit_levels;		/* IDS maximum levels allowed	    */
    unsigned long   fit_width;		/* IDS maximum width  allowed	    */
    unsigned long   fit_height;		/* IDS maximum height allowed	    */
#ifdef IDS_NOX
    unsigned long   xieimage;           /* SHOULD NOT BE USED IN NO X11 MODE*/
#else
    XieImage	    xieimage;           /* XIE client image struct to render*/
#endif
    unsigned long   rotate_width;	/* XIE width of dst photo ID	    */
    unsigned long   rotate_height;	/* XIE height of dst photo ID	    */
    unsigned long   rotate_fill[3];	/* XIE fill val that dont map in src*/
    unsigned long   compute_mode;	/* IDS compute at ISL(CLI)||XIE(Ser)*/
    unsigned long   internal_compute_mode;/* IDS compute at ISL(CLI)||XIE(Ser)*/
    float	    zoom_x_scale;	/* ISL Zoom scale X factor	    */
    float	    zoom_y_scale;	/* ISL Zoom scale Y factor          */
    } IdsRenderCallbackStruct, *IdsRenderCallback;

    /*
     * the application is returned this struct on view callbacks
     */
typedef struct _ViewCallback
    {
    int		    reason;		/* IDS callback reason		    */
#ifdef IDS_NOX
    unsigned long   event;              /* SHOULD NOT BE USED in NO X11 MODE*/
#else
    XEvent	   *event;		/* X11 pointer to triggering event  */
#endif
    int		    source_x;		/* IDS displayed image region X	    */
    int		    source_y;		/* IDS displayed image region Y	    */
    unsigned long   source_width;	/* IDS displayed image region width */
    unsigned long   source_height;	/* IDS displayed image region height*/
    int		    window_x;		/* IDS X offset within widget window*/
    int		    window_y;		/* IDS Y offset within widget window*/
    unsigned long   window_width;	/* IDS width  of image work area    */
    unsigned long   window_height;	/* IDS height of image work area    */
    unsigned long   frame_depth;	/* ISL depth  of rendered frame	    */
    unsigned long   frame_width;	/* ISL width  of rendered frame	    */
    unsigned long   frame_height;	/* ISL height of rendered frame	    */
    } IdsViewCallbackStruct, *IdsViewCallback;

    /*
     * the application is returned this struct on drag callbacks
     */
typedef struct _DragCallback
    {
    int		    reason;		/* IDS callback reason		    */
#ifdef IDS_NOX
    unsigned long   event;              /* SHOULD NOT BE USED in NO X11 MODE*/
#else
    XEvent	   *event;		/* X11 pointer to triggering event  */
#endif
    int		    source_x;		/* IDS displayed image region X	    */
    int		    source_y;		/* IDS displayed image region Y	    */
    int		    window_x;		/* IDS X offset within widget window*/
    int		    window_y;		/* IDS Y offset within widget window*/
    } IdsDragCallbackStruct, *IdsDragCallback;

    /*
     * the application is returned this struct on zoom callbacks
     */
typedef struct _ZoomCallback
    {
    int		    reason;		/* IDS callback reason		    */
#ifdef IDS_NOX
    unsigned long   event;              /* SHOULD NOT BE USED in NO X11 MODE*/
#else
    XEvent	   *event;		/* X11 pointer to triggering event  */
#endif
    int		    start_x;		/* IDS displayed image region X	    */
    int		    start_y;		/* IDS displayed image region Y	    */
    int		    width;		/* IDS width of region */
    int		    height;		/* IDS height of region */
    } IdsZoomCallbackStruct, *IdsZoomCallback;

    /*
     * the application is returned this struct on work in progress callbacks
     */
typedef struct _WorkNotifyCallback
    {
    int		    reason;		/* IDS callback reason		    */
#ifdef IDS_NOX
    unsigned long   event;              /* SHOULD NOT BE USED in NO X11 MODE*/
#else
    XEvent	   *event;		/* X11 pointer to triggering event  */
#endif
    unsigned long   function;		/* IDS function(s) to execute next  */
    unsigned long   process;            /* IDS executed at client or server */
    } IdsWorkNotifyCallbackStruct, *IdsWorkNotifyCallback;

    /*
     * on error the application is returned this struct in a callback
     */
typedef struct _ErrorCallback
    {
    int		    reason;		/* IDS callback reason		    */
#ifdef IDS_NOX
    unsigned long   event;              /* SHOULD NOT BE USED in NO X11 MODE*/
#else
    XEvent	   *event;		/* X11 pointer to triggering event  */
#endif
    int		    code;		/* Error message code               */
    } IdsErrorCallbackStruct, *IdsErrorCallback;

    /*
     * the application is returned this struct to list xie functions
     */
typedef struct _FuncListCallback
    {
    int		    reason;		/* IDS callback reason		    */
#ifdef IDS_NOX
    unsigned long   event;              /* SHOULD NOT BE USED in NO X11 MODE*/
#else
    XEvent	   *event;		/* X11 pointer to triggering event  */
#endif
    char          **names;		/* IDS XIE list of functions	    */
    } IdsFuncListCallbackStruct, *IdsFuncListCallback;

     /*
     **  application is returned with this struct to return fid,photo,xieimage. 
     */
typedef struct _SaveImageCallback
    {
    int             reason;             /* IDS callback reason              */
#ifdef IDS_NOX
    unsigned long   event;              /* SHOULD NOT BE USED in NO X11 MODE*/
#else
    XEvent	   *event;		/* X11 pointer to triggering event  */
#endif
    unsigned long   fid;                /* rendered fid returned            */
    unsigned long   photo;              /* rendered photo returned          */
    unsigned long   xieimage;           /* rendered xieimage returned       */
    } IdsSaveImageCallbackStruct, *IdsSaveImageCallback;
    /*
    **	Item list 2 structure
    */
typedef struct _itmlst_2
    {
	char	    *item_name;
	long	    value;
    } IdsItmlst2;

    /*
    **	Rendering structure
    */
typedef struct _Rendering
    {
    unsigned long int		 srcfid;
    unsigned long int		 psid;
    unsigned char		 type;
    unsigned  : 8;
    unsigned short int		 size;
    struct _RenderCallback	*rcb;
    union  {
	unsigned long int        fid;
	struct  {
#ifdef IDS_NOX
	    unsigned long        ximage;
	    unsigned long        pixmap;
#else
	    XImage		*ximage;
	    Pixmap		 pixmap;
#endif
            unsigned long       *pixel_index_list;
            unsigned long        pixel_index_count;
#ifdef IDS_NOX
	    unsigned long        image_gc;
#else    
            GC                   image_gc;
#endif
	    unsigned long        background_pixel;
            unsigned long        foreground_pixel;
	    } xlib;
        struct  {
            unsigned long int	 bytcnt;
            char		*bufptr;
            } sixel;
        struct  {
            unsigned long int	 bytcnt;
            char		*bufptr;
            } postscript;
        } type_spec_data;
    unsigned long int        rndfid;
    } IdsRendering;

/*---------------------*/
/* IDS public routines */
/*---------------------*/
    /*
    **	(ll) == low  level widget creation entry point
    **	(hl) == high level widget creation entry point
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
#if defined(OS2) || defined(msdos) || defined(__vaxc__) || defined(__STDC__)
#ifndef DAS_EXPAND_PROTO
#define DAS_EXPAND_PROTO 1
#endif
#endif

/*
 * usage: PROTO (return_type function, (arg1, arg2, arg3))
 */
#ifndef PROTO
#if DAS_EXPAND_PROTO == 1
#define PROTO(name, arg_list) name arg_list
#else
#define PROTO(name, arg_list) name ()
#endif
#endif

#ifndef IDS
/* these prototypes are for users */

	/* IDS$CONVERTERS public routines */
#ifndef IDS_NOX
#if defined(__VMS) || defined(VMS)
void IDS$STRING_TO_FLOAT();	    /* VMS - ids$converters	    */
void IDS$STRING_TO_RENDER_MODE();    /* VMS - ids$converters	    */
void IDS$STRING_TO_COMPUTE_MODE();   /* VMS - ids$converters	    */
void IDS$STRING_TO_RENDER_CLASS();   /* VMS - ids$converters	    */
void IDS$STRING_TO_PROTOCOL();	    /* VMS - ids$converters	    */
void IDS$STRING_TO_ROTATE_MODE();    /* VMS - ids$converters	    */
void IDS$STRING_TO_ROTATE_OPTS();    /* VMS - ids$converters	    */
void IDS$STRING_TO_FLIP_OPTS();	    /* VMS - ids$converters	    */
void IDS$STRING_TO_SCALE_MODE();	    /* VMS - ids$converters	    */
void IDS$STRING_TO_SCALE_OPTS();	    /* VMS - ids$converters	    */
void IDS$STRING_TO_DITHER_MODE();    /* VMS - ids$converters	    */
void IDS$STRING_TO_COLORMAP_MODE();  /* VMS - ids$converters	    */
void IDS$STRING_TO_SAVEREND_MODE();  /* VMS - ids$converters	    */
void IDS$STRING_TO_COMPRESS_MODE();  /* VMS - ids$converters	    */
void IDS$STRING_TO_COMPORG_MODE();  /* VMS - ids$converters	    */
void IDS$STRING_TO_COLOR_SPACE();    /* VMS - ids$converters	    */
void IDS$STRING_TO_GRAVITY();	    /* VMS - ids$converters	    */
#endif
PROTO( void IdsStringToFloat, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToRenderMode, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToComputeMode, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToRenderClass, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToProtocol, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToRotateMode, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToRotateOpts, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToFlipOpts, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToScaleMode, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToScaleOpts, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToDitherMode, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToColormapMode, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToSaveRendMode, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToCompressMode, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToComporgMode, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToColorSpace, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));

PROTO( void IdsStringToGravity, (		/* C   - ids$converters	    */
	XrmValuePtr /*args*/, 
	Cardinal */*num_args*/, 
	XrmValuePtr /*from*/, 
	XrmValuePtr /*to*/));
#endif


	/* IDS$RENDER_IMAGE public routines */
	/* IDS$STATIC_IMAGE public routines */
#ifndef IDS_NOX
#if defined(__VMS) || defined(VMS)
Widget	IDS$STATIC_IMAGE_CREATE();  /* VMS - ids$static_image (ll)  */
Widget	IDS$STATIC_IMAGE();	    /* VMS - ids$static_image (hl)  */
XPoint  *IDS$GET_COORDINATES();	    /* VMS - ids$static_image	    */
void	IDS$REDISPLAY_IMAGE();	    /* VMS - ids$static_image	    */
void	IDS$APPLY_GRAVITY();	    /* VMS - ids$static_image	    */
#endif
#ifdef APPLPROG
PROTO( Widget IdsStaticImage, (		/* C   - ids$static_image (ll)  */
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

PROTO( Widget IdsStaticImageCreate, (	/* C   - ids$static_image (hl)  */
	Widget /*parent*/, 
	char */*name*/, 
	Arg */*arglist*/, 
	int /*argCount*/));
#endif

PROTO( XPoint *IdsGetCoordinates, (	/* C   - ids$static_image	    */
	Widget /*iw*/, 
	XPoint */*from*/, 
	XPoint */*to*/, 
	int /*num*/, 
	int /*type*/));

PROTO( void IdsRedisplayImage, (	/* C   - ids$static_image	    */
	Widget /*iw*/, 
	int /*sx*/, 
	int /*sy*/, 
	int /*wx*/, 
	int /*wy*/));

PROTO( void IdsApplyGravity, (		/* C   - ids$static_image	    */
	Widget /*iw*/, 
	unsigned long /*src_grav*/, 
	unsigned long /*win_grav*/));

	/* IDS$PANNED_IMAGE public routines */
#if defined(__VMS) || defined(VMS)
Widget	IDS$PANNED_IMAGE_CREATE();  /* VMS - ids$panned_image (ll)  */ 
Widget	IDS$PANNED_IMAGE();	    /* VMS - ids$panned_image (hl)  */
#endif

#ifdef APPLPROG
PROTO( Widget IdsPannedImageCreate, (	/* C   - ids$panned_image (ll)  */
	Widget /*parent*/, 
	char */*name*/, 
	Arg */*arglist*/, 
	int /*argCount*/));

PROTO( Widget IdsPannedImage, (		/* C   - ids$panned_image (hl)  */
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
#endif


	/* IDS$UIL_SUPPORT public routines */
#if defined(__VMS) || defined(VMS)
int	IDS$INITIALIZE_FOR_DRM();   /* VMS - ids$uil_support        */ 
#endif
int      IdsInitializeForDRM();	    /* C   - ids$uil_support        */

	/* IDS$STATIC_IMAGE_MOTIF public routines */
#if defined(__VMS) || defined(VMS)
Widget	IDSXM$STATIC_IMAGE_CREATE();/* VMS - ids$static_image (ll)  */
Widget	IDSXM$STATIC_IMAGE();	    /* VMS - ids$static_image (hl)  */
XPoint  *IDSXM$GET_COORDINATES();    /* VMS - ids$static_image	    */
void	IDSXM$REDISPLAY_IMAGE();    /* VMS - ids$static_image	    */
void	IDSXM$APPLY_GRAVITY();	    /* VMS - ids$static_image	    */
#endif

PROTO( Widget IdsXmStaticImageCreate, (	/* C   - ids$static_image (ll)  */
	Widget /*parent*/, 
	char */*name*/, 
	Arg */*arglist*/, 
	int /*argCount*/));

PROTO( Widget IdsXmStaticImage, (	/* C   - ids$static_image (hl)  */
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

PROTO( XPoint *IdsXmGetCoordinates, (	/* C   - ids$static_image	    */
	Widget /*iw*/, 
	XPoint */*from*/, 
	XPoint */*to*/, 
	int /*num*/, 
	int /*type*/));

PROTO( void IdsXmRedisplayImage, (	/* C   - ids$static_image	    */
	Widget /*iw*/, 
	int /*sx*/, 
	int /*sy*/, 
	int /*wx*/, 
	int /*wy*/));

PROTO( void IdsXmApplyGravity, (	/* C   - ids$static_image	    */
	Widget /*iw*/, 
	unsigned long /*src_grav*/, 
	unsigned long /*win_grav*/));

	/* IDS$PANNED_IMAGE_MOTIF public routines */
#if defined(__VMS) || defined(VMS)
Widget	IDSXM$PANNED_IMAGE_CREATE();/* VMS - ids$panned_image (ll)  */ 
Widget	IDSXM$PANNED_IMAGE();	    /* VMS - ids$panned_image (hl)  */
#endif

PROTO( Widget IdsXmPannedImageCreate, (	/* C   - ids$panned_image (ll)  */
	Widget /*parent*/, 
	char */*name*/, 
	Arg */*arglist*/, 
	int /*argCount*/));

PROTO( Widget IdsXmPannedImage, (	/* C   - ids$panned_image (hl)  */
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
	XtCallbackList /*help*/));

	/* IDS$UIL_SUPPORT_MOTIF public routines */
#if defined(__VMS) || defined(VMS)
int	IDSXM$INITIALIZE_FOR_DRM(); /* VMS - ids$uil_support        */ 
#endif

PROTO( int IdsXmInitializeForDRM, (	/* C   - ids$uil_support        */
	void));
#endif

        /* IDS hardcopy public routines */
#ifndef IDS

PROTO( unsigned long int IdsCreatePresentSurface, (
	IdsItmlst2 */*itmlst*/));

PROTO( unsigned long int IdsSetSurfaceAttributes, (
	unsigned long /*psid*/, 
	IdsItmlst2 */*itmlst*/));

PROTO( unsigned long int IdsGetSurfaceAttributes, (
	unsigned long /*psid*/, 
	IdsItmlst2 */*itmlst*/));

PROTO( void IdsDeletePresentSurface, (
	unsigned long /*psid*/));

PROTO( IdsRendering *IdsCreateRendering, (
	unsigned long /*fid*/, 
	unsigned long /*psid*/, 
	IdsItmlst2 */*itmlst*/));

PROTO( void IdsDeleteRendering, (
	IdsRendering */*rendering*/));

#endif
	/* Conversion of Data format routines */


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

PROTO( void IdsPhotoToDec, (
	XiePhotomap /*map*/, 
	unsigned long /*cmpres*/, 
	unsigned long /*cmap*/, 
	char */*name*/));

PROTO( unsigned long IdsXieImageToFid, (
	XieImage /*img*/));


PROTO( void IdsFidToDec, (
	unsigned long /*fid*/, 
	unsigned long /*cmpres*/, 
	unsigned long /*cmap*/, 
	char */*name*/));

PROTO( unsigned long IdsXimageToFid, (
	Display */*display*/, 
	XImage */*ximage*/, 
	Visual */*visual*/, 
	Colormap /*cmap*/, 
	unsigned long /*output_class*/));
#endif


#endif /* IDS */

#endif /* IDSIMAGE_H */

