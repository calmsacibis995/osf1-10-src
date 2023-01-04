
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
**      Robert NC Shelley
**      Subu Garikapati
**
**  CREATION DATE:  February 12, 1988
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

    /*
    **  Standard C include files
    */
#include <math.h>			    /* math routines		    */
    /*
    **  XIE and IDS include files
    **
    ** XieAppl.h    XIE piblic symbols
    ** XieLib.h	    XIE public entry pionts
    ** Xlibint.h    X11 internal lib - transport defaults for XieCheckFunc()
    ** XieProto.h   XIE contains defs required by XIE wire protocol
    **		    Sole purpose is to get used in XieCheckFunc()
    */
#if defined(__VMS) || defined (VMS)
#include <XieAppl.h>
#include <XieLib.h>
#include <Xlibint.h> 
#include <XieProto.h>
#else
#include <X11/extensions/XieAppl.h>
#include <X11/extensions/XieLib.h>
#include <X11/Xproto.h>
#include <X11/extensions/XieProto.h>
#endif

    /*
    **  ISL and IDS include files
    */
#include    <ids__widget.h> /* IDS public/private, Dwtoolkit, Xlib defs.*/
#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif

/*
**  Table of contents
*/

#ifdef NODAS_PROTO
Boolean		CompareRenderingsFlo(); /* Comp flo Prop/Cur render         */
void            TranslatePointFlo();    /* Compute pre-rendered coordinates */

    /*
    **  internal routines
    */
static Boolean		CompareRotate();	/* check for rotate changes */
static Boolean		CompareFlip();		/* check for flip changes   */
static Boolean		CompareScale();		/* check for scale changes  */
static Boolean		CompareToneScale();	/* check tone-scale changes */
static Boolean		CompareDither();	/* check for dither changes */
static Boolean		CompareSharpen();	/* check for sharpn changes */
static Boolean          CheckXieSubsetSupport(); /* check for sharpn changes */
#else
PROTO(static Boolean CompareRotate, (IdsRenderCallback /*old*/, IdsRenderCallback /*new*/));
PROTO(static Boolean CompareFlip, (IdsRenderCallback /*old*/, IdsRenderCallback /*new*/));
PROTO(static Boolean CompareScale, (IdsRenderCallback /*old*/, IdsRenderCallback /*new*/));
PROTO(static Boolean CompareToneScale, (IdsRenderCallback /*old*/, IdsRenderCallback /*new*/));
PROTO(static Boolean CompareDither, (IdsRenderCallback /*old*/, IdsRenderCallback /*new*/));
PROTO(static Boolean CompareSharpen, (IdsRenderCallback /*old*/, IdsRenderCallback /*new*/));
PROTO(static Boolean CheckXieSubsetSupport, (Display */*dpy*/, unsigned long /*scheme*/, unsigned long /*cmpress*/));
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

/*
**	Local Storage
*/

/*****************************************************************************
**  CompareRenderingsFlo
**
**  FUNCTIONAL DESCRIPTION:
**
**      Compare Proposed and Current renderings.
**
**  FORMAL PARAMETERS:
**
**      old,new         - Render Image Widget current and proposed rcb's
**      ctx             - Render Image Widget render context struct
**      igc             - Render Image Widget gc used for bitonal
**      pfid            - Boolean if CopyFid_(rw) is true then pfid true
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
Boolean CompareRenderingsFlo( old, new, ctx, xiectx, igc, pfid )
 IdsRenderCallback  old, new;
 RenderContext	    ctx;
 RenderContextXie   xiectx;
 GC		    igc;
 Boolean	    pfid;
{
    int xie_parm,  cmpress;
    Boolean nosupport = False;	

#ifdef TRACE
printf( "Entering Routine CompareRenderingsFlo in module IDS_RENDER_XIE \n");
#endif
    if( iXieimage_(new) != 0 )
	{
	/*
	**  Make sure we support the spectral component mapping of this fid.
	*/
        xie_parm = CmpMap_((XieImage)iXieimage_(new));

       if( xie_parm != XieK_Bitonal && xie_parm != XieK_GrayScale &&
                                                         xie_parm != XieK_RGB )
	    {
	    XtWarningMsg("unsSpcMap","IdsCompareRenderings","IdsImageError",
			 "image pixel spectral mapping unsupported",0,0);
	    /*
	    **	Delete the fid if it is a private copy.
	    */
	    if( pfid )
                ImgDeleteFrame( iFid_(new) );
	    /*
	    **	Disavow all knowledge of this image.
	    */
            iFid_(new) = 0;
	    }
	}

    if( iXieimage_(new) == 0 )
	/*
	**  No proposed fid, so Abort.  Or Purge current rendering.
	*/
	iRender_(new) = iXieimage_(old) == 0 ? Ids_Abort : Ids_Purge;

    else if( iXieimage_(old) != iXieimage_(new) )
	/*
	**  New proposed fid: if Passive mode, change mode to force rendering.
	*/
	if( iRender_(new) == Ids_Passive )
	    iRender_(new)  = Ids_Normal;


    /*
    **  RenderScheme is a read-only resource, so ignore anything written to it.
    */
    iScheme_(new) = 0;

    switch( iRender_(new) )
	{
    case Ids_Abort :
	/*
	**  The application doesn't like our rendering proposal.
	*/
	iReason_(new) = IdsCRNoChange;
	break;

    case Ids_Purge :
	/*
	**  The application just wants the window cleared.
	*/
	iReason_(new) = IdsCRPurge;
	break;

    case Ids_Passive :
    case Ids_Normal :
    case Ids_Override :
	/*
	**  Calculate an IDS Scheme and/or rendering parameters,
	**  then compare the Proposed rendering to the Current rendering.
	*/
	IdsApplyModelXie( ctx, xiectx );
        /*
        ** See if the image is G3 compressed image
        */
        cmpress = Cmpres_((XieImage)iXieimage_(new));
        if( CheckXieSubsetSupport( Dpy_(ctx), iScheme_(new), cmpress))
            {
            nosupport = True;
            break;
            }
	if((iScheme_(new) ^ iScheme_(old))	/* new rendering Scheme	    */
			  & ~Ids_Decompress	/* (we only decompress once)*/
         || iXieimage_(new) != iXieimage_(old)  /* new xieimage             */
	 || iCompute_(new) !=   iCompute_(old)	/* new ISL ROI-id	    */
	 || iROI_(new) !=   iROI_(old)	        /* new ISL ROI-id	    */
	 || iProto_(new) != iProto_(old) 	/* new presentation protocol*/
	 || CompareRotate(old,new)		/* new rotation parameters  */
	 || CompareFlip(old,new)		/* new flip parameters	    */
	 || CompareScale(old,new)		/* new scaling parameters   */
	 || CompareToneScale(old,new)		/* new tone-scale parameters*/
	 || CompareDither(old,new)		/* new dither parameters    */
	 || CompareSharpen(old,new)		/* new sharpen parameters   */
	 || igc == NULL )			/* new GC required	    */
	    /*
	    **	Something changed;  give application a reason to (re)render.
	    */
	    iReason_(new) = iRender_(new) == Ids_Override ? IdsCROverride
							  : IdsCRNormal;
	else
	    /*
	    **	Proposed is identical to Current.
	    */
	    iReason_(new)  = IdsCRNoChange;

	if( iRender_(new) == Ids_Passive )
	    /*
	    **	Passive mode:  re-render only if something actually changed.
	    */
	    if( iReason_(new) == IdsCRNoChange )
		iRender_(new)  = Ids_Abort;	    /* no need to re-render */
	    else
		iRender_(new)  = Ids_Normal;	    /* guarantee re-render  */
	break;
    default:
	XtWarningMsg("InvRndMod","CompareRenderings","IdsImageError",
		     "invalid Render Mode", 0,0);
	iReason_(new) = IdsCRNoChange;
	iRender_(new) = Ids_Abort;
	}
#ifdef TRACE
printf( "Leaving Routine CompareRenderingsFlo in module IDS_RENDER_XIE \n");
#endif
    return(nosupport);
}

/*******************************************************************************
**  TranslatePointFlo
**
**  FUNCTIONAL DESCRIPTION:
**
**      Translate an (x,y) point back to its pre-rendered coordinate values.
**
**  FORMAL PARAMETERS:
**
**      rcb     - Render Image Widget rcb
**      coords  - coordinate point pair
**      w,h     - width of the image as in the Ximage struct
**
*******************************************************************************/
void TranslatePointFlo( xiedat, rcb, coords, width, height )
 DataForXie        xiedat;
 IdsRenderCallback rcb;
 XPoint *coords;
 unsigned long width, height;
{
    double   x  = coords->x;
    double   y  = coords->y;
    double   ca, sa, x0, y0, xr, yr;
    int	     w, h;
    int	     i, comps;
    XieImage   img = iXieimage_(rcb);    


#ifdef TRACE
printf( "Entering Routine TranslatePointFlo in module IDS_RENDER_XIE \n");
#endif
    if( iScheme_(rcb) & Ids_UseFlip  &&  iFlip_(rcb) & Ids_FlipHorizontal )
	x = width  - x;	
    if( iScheme_(rcb) & Ids_UseFlip  &&  iFlip_(rcb) & Ids_FlipVertical )
	y = height - y;	

    if( iScheme_(rcb) & Ids_UseScale_2 )
	{
	x /= iXSc_(rcb);			    /* unPostScale	    */
	y /= iYSc_(rcb);
	}
    if( iScheme_(rcb) & Ids_UseAngle )
	{
	ca = cos( DEG_RAD * iRAng_(rcb) );	    /* unRotate Cos(angle)  */
	sa = sin( DEG_RAD * iRAng_(rcb) );	    /* unRotate Sin(angle)  */
	x0 = width  / 2.0;			    /* center pivot point   */
	y0 = height / 2.0;
	if( iScheme_(rcb) & Ids_UseScale_2 )
	    {
	    x0 /= iXSc_(rcb);			    /* unScale pivot point  */
	    y0 /= iYSc_(rcb);
	    }
	xr = x0 + ca * (x - x0) + sa * (y - y0);    /* unRotate coordinate  */
	yr = y0 + ca * (y - y0) - sa * (x - x0);

	w = Width_(img);		            /* compute width  used  */
	h = Height_(img);			    /* compute height used  */

	if( iScheme_(rcb) & Ids_UseROI )
	    {
	    w = Croiw_(xiedat);		            /* compute width  used  */
	    h = Croih_(xiedat);			    /* compute height used  */
	    }

	if( iScheme_(rcb) & Ids_UseScale_1 )
	    {
	    w *= iXSc_(rcb);			    /* pre-rotate width	    */
	    h *= iYSc_(rcb);			    /* pre-rotate height    */
	    }
	/*
	**  Crop the increase in image dimensions caused by rotation.
	*/
	x = xr + w / 2.0 - x0;
	y = yr + h / 2.0 - y0;
	}
    if( iScheme_(rcb) & Ids_UseScale_1 )
	{
	x /= iXSc_(rcb);			    /* unPreScale	    */
	y /= iYSc_(rcb);
	}
    if( iScheme_(rcb) & Ids_UseROI )
	{					    /* Add ROI origin	    */
	x += Croix_(xiedat);
	y += Croiy_(xiedat);
	}
    coords->x = x;				    /* final X	    	    */
    coords->y = y;				    /* final Y		    */
#ifdef TRACE
printf( "Leaving Routine TranslatePointFlo in module IDS_RENDER_XIE \n");
#endif
}

/*****************************************************************************
**  CompareRotate
**
**  FUNCTIONAL DESCRIPTION:
**
**      Compare current and proposed renderings for anything affecting rotate.
**
**  FORMAL PARAMETERS:
**
**      old, new        - Render Image Widget current and proposed rcb's
**
**  FUNCTION VALUE:
**
**	Boolean - TRUE if a difference is detected.
**
*****************************************************************************/
static Boolean CompareRotate( old, new )
    IdsRenderCallback old, new;
{
    Boolean changed = FALSE;
    char spectral_type;

#ifdef TRACE
printf( "Entering Routine CompareRotate in module IDS_RENDER_XIE \n");
#endif
    if( iScheme_(new) & Ids_UseAngle )
	{
	changed = iRAng_(new) != iRAng_(old);

	if( !changed  &&  iRAng_(new) != 0.0 )
	    {
/*	    changed = (iROpts_(new) ^ iROpts_(old)) & ImgM_ReverseEdgeFill; */
            spectral_type = CmpMap_((XieImage)iXieimage_(new));
	    if( spectral_type != XieK_Bitonal )
		changed |= iROpts_(new) != iROpts_(old);
	    }
	}
#ifdef TRACE
printf( "Leaving Routine CompareRotate in module IDS_RENDER_XIE \n");
#endif
    return changed;
}

/*****************************************************************************
**  CompareFlip
**
**  FUNCTIONAL DESCRIPTION:
**
**      Compare current and proposed renderings for anything affecting flip.
**
**  FORMAL PARAMETERS:
**
**      old, new        - Render Image Widget current and proposed rcb's
**
**  FUNCTION VALUE:
**
**	Boolean - TRUE if a difference is detected.
**
*****************************************************************************/
static Boolean CompareFlip( old, new )
    IdsRenderCallback old, new;
{
    Boolean changed = FALSE;

#ifdef TRACE
printf( "Entering Routine CompareFlip in module IDS_RENDER_XIE \n");
#endif
    if( iScheme_(new) & Ids_UseFlip )
	changed = iFlip_(new) != iFlip_(old);

#ifdef TRACE
printf( "Leaving Routine CompareFlip in module IDS_RENDER_XIE \n");
#endif
    return changed;
}

/*****************************************************************************
**  CompareScale
**
**  FUNCTIONAL DESCRIPTION:
**
**      Compare current and proposed renderings for anything affecting scale.
**
**  FORMAL PARAMETERS:
**
**      old, new        - Render Image Widget current and proposed rcb's
**
**  FUNCTION VALUE:
**
**	Boolean - TRUE if a difference is detected.
**
*****************************************************************************/
static Boolean CompareScale( old, new )
    IdsRenderCallback old, new;
{
    Boolean changed = FALSE;
    int spectral_type;

#ifdef TRACE
printf( "Entering Routine CompareScale in module IDS_RENDER_XIE \n");
#endif
    if( iScheme_(new) & (Ids_UseScale_1 | Ids_UseScale_2))
	changed =  iXSc_(new) != iXSc_(old)
		|| iYSc_(new) != iYSc_(old);

#ifdef TRACE
printf( "Leaving Routine CompareScale in module IDS_RENDER_XIE \n");
#endif
    return changed;
}

/*****************************************************************************
**  CompareToneScale
**
**  FUNCTIONAL DESCRIPTION:
**
**      Compare current and proposed renderings for tone-scale changes.
**
**  FORMAL PARAMETERS:
**
**      old, new        - Render Image Widget current and proposed rcb's
**
**  FUNCTION VALUE:
**
**	Boolean - TRUE if a difference is detected.
**
*****************************************************************************/
static Boolean CompareToneScale( old, new )
    IdsRenderCallback old, new;
{
    Boolean changed = FALSE;
    int spectral_type;

#ifdef TRACE
printf( "Entering Routine CompareToneScale in module IDS_RENDER_XIE \n");
#endif
    if( iScheme_(new) & Ids_UseToneScale )
	{
	changed |= iPun1_(new) != iPun1_(old)	    /* new punch 1	    */
		|| iPun2_(new) != iPun2_(old);	    /* new punch 2	    */
	}

#ifdef TRACE
printf( "Leaving Routine CompareToneScale in module IDS_RENDER_XIE \n");
#endif
    return( changed );
}

/*****************************************************************************
**  CompareDither
**
**  FUNCTIONAL DESCRIPTION:
**
**      Compare current and proposed renderings for anything affecting dither.
**
**  FORMAL PARAMETERS:
**
**      old, new        - Render Image Widget current and proposed rcb's
**
**  FUNCTION VALUE:
**
**	Boolean - TRUE if a difference is detected.
**
*****************************************************************************/
static Boolean CompareDither( old, new )
    IdsRenderCallback old, new;
{
    Boolean changed = FALSE;

#ifdef TRACE
printf( "Entering Routine CompareDither in module IDS_RENDER_XIE \n");
#endif
    if( iScheme_(new) & Ids_UseDither )
	{
	changed  = iAlgor_(new) != iAlgor_(old)	    /* new dither algorithm */
		|| iThrsh_(new) != iThrsh_(old)	    /* new dither threshold */
		&& iAlgor_(new) != Ids_Requantize   /* and threshold is used*/
		&& iAlgor_(new) != Ids_Requantize;  /* and threshold is used*/
/*		&& iAlgor_(new) != Ids_BlueNoise;*/

	if( !changed )
	    if( iGRA_(new) != 0 )
		/*
		**  Grayscale, or converted from multi-spectral.
		*/
		changed = iGRA_(new) != iGRA_(old);
	    else
		/*
		**  Multi-spectral.
		*/
		changed = iRGB_(new)[RED] != iRGB_(old)[RED]
		       || iRGB_(new)[GRN] != iRGB_(old)[GRN]
		       || iRGB_(new)[BLU] != iRGB_(old)[BLU];
	}

#ifdef TRACE
printf( "Leaving Routine CompareDither in module IDS_RENDER_XIE \n");
#endif
    return changed;
}

/*****************************************************************************
**  CompareSharpen
**
**  FUNCTIONAL DESCRIPTION:
**
**      Compare current and proposed renderings for sharpen  changes.
**
**  FORMAL PARAMETERS:
**
**      old, new        - Render Image Widget current and proposed rcb's
**
**  FUNCTION VALUE:
**
**	Boolean - TRUE if a difference is detected.
**
*****************************************************************************/
static Boolean CompareSharpen( old, new )
    IdsRenderCallback old, new;
{
    Boolean changed = FALSE;

#ifdef TRACE
printf( "Entering Routine CompareSharpen in module IDS_RENDER_XIE \n");
#endif
    if( iScheme_(new) & Ids_UseSharpen )
	changed |= iSharp_(new) != iSharp_(old);    /* new sharpen 	    */

#ifdef TRACE
printf( "Leaving Routine CompareSharpen in module IDS_RENDER_XIE \n");
#endif
    return( changed );
}

/*****************************************************************************
**  CheckXieSubsetSupport
**
**  FUNCTIONAL DESCRIPTION:
**
**      If render scheme pipe functionality is not supported by XIE server then
**      the function returns True else returns False. 
**
**  FORMAL PARAMETERS:
**
**      scheme        - Render Image Widget current and proposed rcb's
**
**  FUNCTION VALUE:
**
**	Boolean - TRUE if a difference is detected.
**
*****************************************************************************/
static Boolean CheckXieSubsetSupport( dpy, scheme, cmpress )
    Display      *dpy;
    unsigned long scheme;
    unsigned long cmpress;
{
    Boolean nosupport = FALSE;
    unsigned long       xie_scheme;
    char  *str;

#ifdef TRACE
printf( "Entering Routine CheckXieSubsetSupport in module IDS_RENDER_XIE \n");
#endif
    xie_scheme |= Ids_Decompress;
    if( scheme & Ids_UseDither )
	if( (str = XieCheckFunction( dpy, X_ieDither )) == NULL )
	    nosupport = TRUE;

    if( scheme & Ids_UseClassCvt )
	if( (str = XieCheckFunction( dpy, X_ieLuminance )) == NULL )
	    nosupport = TRUE;

    if( ( scheme & Ids_Decompress) || ( xie_scheme & Ids_Decompress) )
	    {

	    if (getenv( "IDS_XIE_G3" ))
		{
		if(cmpress != XieK_G42D && cmpress != XieK_DCT && 
		    cmpress != XieK_G31D &&
		    cmpress != XieK_G32D && cmpress != XieK_PCM )
		nosupport = TRUE;
		}
	    else
		{
		if(cmpress != XieK_G42D && cmpress != XieK_DCT && 
							cmpress != XieK_PCM )
		nosupport = TRUE;
		}/* of logical check */
	    }
   if( scheme & Ids_FunctionROI )
        if( (str = XieCheckFunction( dpy, X_ieCrop )) == NULL )
            nosupport = TRUE;
    
    if( ( scheme & Ids_UseScale_1 ) || ( scheme & Ids_UseScale_2 )  )
	if( (str = XieCheckFunction( dpy, X_ieScale )) == NULL )
	    nosupport = TRUE;

    if( scheme & Ids_UseAngle )
	if( (str = XieCheckFunction( dpy, X_ieRotate )) == NULL )
	    nosupport = TRUE;

    if( scheme & Ids_UseFlip )
	if( (str = XieCheckFunction( dpy, X_ieRotate )) == NULL )
	    nosupport = TRUE;

    if( scheme & Ids_UseToneScale )
	if( (str = XieCheckFunction( dpy, X_iePoint )) == NULL )
	    nosupport = TRUE;

    if( scheme & Ids_UseSharpen )
	if( (str = XieCheckFunction( dpy, X_ieArea )) == NULL )
	    nosupport = TRUE;

/*
**  This allows XIE to be disabled by a logical
*/
    if (getenv( "IDS_COMPUTE_MODE_ISL" ))
		nosupport = TRUE;

#ifdef TRACE
printf( "Leaving Routine CheckXieSubsetSupport in module IDS_RENDER_XIE \n");
#endif
    return( nosupport );
}
