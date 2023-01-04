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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "@(#)$RCSfile: logo.c,v $ $Revision: 1.1.7.8 $ (DEC) $Date: 1993/12/15 20:53:52 $";
#endif		/* BuildSystemHeader */
/*****************************************************************************
 * logo.c - Displays the bitmapped "digital" logo			     *
 *****************************************************************************/

/*****************************************************************************
 * Includes 								     *
 *****************************************************************************/

#include "logo.h"
#include <stdio.h>
#include <strings.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <X11/extensions/shape.h>

/* These files contain the actual XY bit maps of the logo */
#include "digital_logo.bmp"
#include "digital_logo_mask.bmp"

#ifdef __alpha
#include "alpha_axp_logo.bmp"
#include "alpha_axp_logo_mask.bmp"
#endif

/*****************************************************************************
 * Resources								     *
 *****************************************************************************/

typedef struct _OptionRec
  {
    Pixel	logoColor;
    Pixel	logoBackground;
    Pixel	logoBW;
    Pixel	logoBackgroundBW;
    Position	logoX;
    Position	logoY;
    Pixel	rootColor;
    Boolean	setRootColor;
#ifdef __alpha
    Boolean	showAxpLogo;
#endif
    String	bitmapFile;
    String	bitmapMaskFile;
  } LogoOptionRec;

#define XmNlogoColor		"logoColor"
#define XmCLogoColor		"LogoColor"
#define XmNlogoBackground	"logoBackground"
#define XmCLogoBackground	"LogoBackground"
#define XmNlogoBW		"logoBW"
#define XmCLogoBW		"LogoBW"
#define XmNlogoBackgroundBW	"logoBackgroundBW"
#define XmCLogoBackgroundBW	"LogoBackgroundBW"
#define XmNshowAxpLogo		"showAxpLogo"
#define XmCShowAxpLogo		"ShowAxpLogo"
#define XmNlogoX		"logoX"
#define XmCLogoX		"LogoX"
#define XmNlogoY		"logoY"
#define XmCLogoY		"LogoY"
#define XmNrootColor		"rootColor"
#define XmCRootColor		"RootColor"
#define XmNsetRootColor		"setRootColor"
#define XmCSetRootColor		"SetRootColor"
#define XmNbitmapFile     	"bitmapFile"
#define XmCBitmapFile     	"BitmapFile"
#define XmNbitmapMaskFile     	"bitmapMaskFile"
#define XmCBitmapMaskFile     	"BitmapMaskFile"

#define	Offset( field )	XtOffsetOf( LogoOptionRec, field )

static XtResource Resources[] =
  {
    { XmNlogoColor, XmCLogoColor, XtRPixel, sizeof(char *),
	Offset( logoColor ), XtRString, "rgb:8182/0604/2c28"
	/* "rgb:9999/0000/0000" or "rgb:bbbb/0101/6565" */
    },
    { XmNlogoBackground, XmCLogoBackground, XtRPixel, sizeof(char *),
	Offset( logoBackground ), XtRString, "White"
    },
    { XmNlogoBW, XmCLogoBW, XtRPixel, sizeof(char *),
	Offset( logoBW ), XtRString, "Black"
    },
    { XmNlogoBackgroundBW, XmCLogoBackgroundBW, XtRPixel, sizeof(char *),
	Offset( logoBackgroundBW ), XtRString, "White"
    },
    { XmNlogoX, XmCLogoX, XtRPosition, sizeof(Position),
	Offset( logoX ), XtRImmediate, (caddr_t) -1
    },
    { XmNlogoY, XmCLogoY, XtRPosition, sizeof(Position),
	Offset( logoY ), XtRImmediate, (caddr_t) -1
    },
    { XmNrootColor, XmCRootColor, XtRPixel, sizeof(char *),
	Offset( rootColor ), XtRString, "rgb:3030/5050/6060"
    },
    { XmNsetRootColor, XmCSetRootColor, XtRBoolean, sizeof(Boolean),
	Offset( setRootColor ), XtRImmediate, (caddr_t) True
    },
#ifdef __alpha
    { XmNshowAxpLogo, XmCShowAxpLogo, XtRBoolean, sizeof(Boolean),
	Offset( showAxpLogo ), XtRImmediate, (caddr_t) True
    },
#endif
    { XmNbitmapFile, XmCBitmapFile, XtRString, sizeof(String),
	Offset( bitmapFile ), XtRImmediate, NULL
    },
    { XmNbitmapMaskFile, XmCBitmapFile, XtRString, sizeof(String),
	Offset( bitmapMaskFile ), XtRImmediate, NULL
    },
};

#undef Offset

/*****************************************************************************
 * Prototypes 								     *
 *****************************************************************************/

static XtCallbackProc HandleExpose( Widget	widget,
				    caddr_t	client_data,
				    caddr_t	call_data );


static XtCallbackProc RestoreRoot( Widget	widget,
				    caddr_t	client_data,
				    caddr_t	call_data );

/*****************************************************************************
 * Other Global Types							     *
 *****************************************************************************/

typedef struct _LogoData
  {
    Display*	display;
    Window	window;
    GC		gc;
    Pixmap	pixmap;
    int		height;
    int		width;
  } LogoDataRec, * PLogoData;

/*****************************************************************************
 * Other Global variables 						     *
 *****************************************************************************/

static LogoDataRec LogoData;
#ifdef __alpha
static LogoDataRec AXPLogoData;
#endif
/*****************************************************************************
 * Entry point								     *
 *****************************************************************************/

void ShowLogo( Widget topWidget )
  {
    LogoOptionRec	logoOptions;
    Widget		logoWidget, drawingWidget;
#ifdef __alpha
    Widget		axpLogoWidget, axpDrawingWidget;
#endif
    Display		*display;
    Window		window;
    int			cArgs;
    Arg			pArgs[20];
    XtCallbackRec	exposeCallbacks[2];
    XtCallbackRec	destroyCallbacks[2];
    int			windowWidth, windowHeight;
    Position		x, y;
    Pixmap		pixmap;
    Pixmap 		logo_mask, axp_logo_mask;
    GC			gc;
    XGCValues 		gcvalues;
    int 		i,d1,d2, xhot, yhot;
    XColor		root_color;
    Screen		*screen;
    char		*bitmap_data;
    char		*bitmap_mask_data;
    Boolean		digital_logo_used;


    XtGetApplicationResources( topWidget,
			       (XtPointer) &logoOptions,
			       Resources,
			       XtNumber( Resources ),
			       (Arg*) NULL, 0 );
    
    LogoData.display = display = XtDisplay( topWidget );
    screen = XtScreen( topWidget );

    /* Set the root window background to a solid color */

    if ((logoOptions.setRootColor) && 
		(DisplayPlanes(display, DefaultScreen(display)) > 2 ))
	{
	XSetWindowBackground(display,
			    RootWindowOfScreen(screen),
			    logoOptions.rootColor);
	XClearWindow(display,
		     RootWindowOfScreen(screen));

	/* Set all other screens to the same background color */

	for (i=ScreenCount(display) - 1; i>=0; i--)
	    {
	    if ((i != DefaultScreen(display))
		   && (DisplayPlanes(display, i) > 2 ))
		{
		root_color.pixel = logoOptions.rootColor;
		XQueryColor(display, 
			    DefaultColormapOfScreen(screen),
			    &root_color);
		XAllocColor(display,
			   DefaultColormapOfScreen(ScreenOfDisplay(display, i)),
			   &root_color);
		XSetWindowBackground(display, 
				     RootWindow(display, i), 
				     root_color.pixel);
		XClearWindow(display, RootWindow(display, i));
		}
	    }
	}

    if ((logoOptions.bitmapFile == NULL)
	|| (BitmapSuccess != XmuReadBitmapDataFromFile(logoOptions.bitmapFile,
						&windowWidth, &windowHeight, 
						&bitmap_data, &xhot, &yhot)))
	{
	digital_logo_used = True;
	windowWidth = digital_logo_width;
	windowHeight = digital_logo_height;
	bitmap_data = digital_logo_bits;
	}
    else
	{
	digital_logo_used = False;
	}

    LogoData.height = windowHeight;
    LogoData.width = windowWidth;

    /* Set up window parameters, create and map window */
    /* create x and y from screen attributes */
    x = logoOptions.logoX;
    if (x == -1)
       x = (XWidthOfScreen(XDefaultScreenOfDisplay(display)) - windowWidth) / 2;
    y = logoOptions.logoY;
    Debug ("logo vertical position: %d\n", y);
    if (y == -1)
       y = (HeightOfScreen(DefaultScreenOfDisplay(display))/3 - windowHeight)/2;

								   cArgs = 0;
    XtSetArg( pArgs[cArgs], XmNx, 		 x		); cArgs++;
    XtSetArg( pArgs[cArgs], XmNy, 		 y		); cArgs++;
    XtSetArg( pArgs[cArgs], XmNwidth, 		 windowWidth	); cArgs++;
    XtSetArg( pArgs[cArgs], XmNheight, 		 windowHeight	); cArgs++;
    XtSetArg( pArgs[cArgs], XmNborderWidth, 	 0		); cArgs++;
    XtSetArg( pArgs[cArgs], XmNoverrideRedirect, TRUE		); cArgs++;
    XtSetArg( pArgs[cArgs], XmNsaveUnder, 	 FALSE		); cArgs++;
    XtSetArg( pArgs[cArgs], XmNbackgroundPixmap, ParentRelative	); cArgs++;
    logoWidget = XtCreatePopupShell( "Logo",
				     topLevelShellWidgetClass,
				     topWidget, pArgs, cArgs );

    exposeCallbacks[0].callback = (XtCallbackProc) HandleExpose;
    exposeCallbacks[0].closure  = (XtPointer) &LogoData;
    exposeCallbacks[1].callback = NULL;
    exposeCallbacks[1].closure  = 0;


    								   cArgs = 0;
    XtSetArg( pArgs[cArgs], XmNx,		0		); cArgs++;
    XtSetArg( pArgs[cArgs], XmNy,		0		); cArgs++;
    XtSetArg( pArgs[cArgs], XmNwidth,		windowWidth	); cArgs++;
    XtSetArg( pArgs[cArgs], XmNheight, 		windowHeight	); cArgs++;
    XtSetArg( pArgs[cArgs], XmNexposeCallback,	exposeCallbacks	); cArgs++;
    if (logoOptions.setRootColor)
	{
	destroyCallbacks[0].callback = (XtCallbackProc) RestoreRoot;
	destroyCallbacks[0].closure  = NULL;
	destroyCallbacks[1].callback = NULL;
	destroyCallbacks[1].closure  = 0;
	XtSetArg( pArgs[cArgs], XmNdestroyCallback, destroyCallbacks); cArgs++;
	}
    drawingWidget = XtCreateWidget( "Logo Display", xmDrawingAreaWidgetClass,
				    logoWidget, pArgs, cArgs );

    XtRealizeWidget( logoWidget );
    LogoData.window = window = XtWindow( drawingWidget );

    gcvalues.foreground = logoOptions.logoColor;
    gcvalues.background = logoOptions.logoBackground;
		
    LogoData.gc = gc = XtGetGC(drawingWidget, GCForeground | GCBackground, 
				&gcvalues);

    if ( DisplayPlanes( display, DefaultScreen( display )) > 2 )
	{
	XSetForeground( display, gc, logoOptions.logoColor );
	XSetBackground( display, gc, logoOptions.logoBackground );
	}
    else
	{
	XSetForeground( display, gc, logoOptions.logoBW );
	XSetBackground( display, gc, logoOptions.logoBackgroundBW );
	}

    LogoData.pixmap = pixmap = XCreatePixmapFromBitmapData(
                XtDisplay(drawingWidget),
                XtWindow(drawingWidget),
                bitmap_data,
                windowWidth, windowHeight,
                gcvalues.foreground,
                gcvalues.background,
                drawingWidget->core.depth);
    if(!pixmap) 
	{
	LogError("xdm: logo pixmap create failed\n");
	return;
	}
    if (!digital_logo_used)
	{
	XFree(bitmap_data);

	logo_mask = NULL;
	if ((logoOptions.bitmapMaskFile != NULL)
	    && (BitmapSuccess == 
		    XmuReadBitmapDataFromFile(logoOptions.bitmapMaskFile,
					    &windowWidth, &windowHeight, 
					    &bitmap_mask_data, &xhot, &yhot)))
	    {
	    logo_mask = XCreatePixmapFromBitmapData(
			    XtDisplay(drawingWidget),
			    XtWindow(drawingWidget),
			    bitmap_mask_data,
			    windowWidth, windowHeight,
			    1, 0, /* foreground/background */
			    1);
	    if(!logo_mask) 
		{
		LogError("xdm: Logo Mask pixmap create failed\n");
		}
	    XFree(bitmap_mask_data);
	    }
	}
    else
	{
	logo_mask = XCreatePixmapFromBitmapData(
		XtDisplay(drawingWidget),
		XtWindow(drawingWidget),
		digital_logo_mask_bits,
		digital_logo_mask_width, digital_logo_mask_height,
		1, 0, /* foreground/background */
		1);

	if(!logo_mask) 
	    {
	    LogError("xdm: Logo Mask pixmap create failed\n");
	    return;
	    }
	}

#ifdef __alpha
    /*
     * Now do the AXP logo
     */

    if (logoOptions.showAxpLogo)
	{
	x = 0;
	y = XHeightOfScreen(XDefaultScreenOfDisplay(display)) 
					    - alpha_axp_logo_height;
	Debug ("AXP logo vertical position: %d\n", y);
	if (y < 0) y = 0;
	cArgs = 0;
	XtSetArg( pArgs[cArgs], XmNx, 		 x		); cArgs++;
	XtSetArg( pArgs[cArgs], XmNy, 		 y		); cArgs++;
	XtSetArg( pArgs[cArgs], XmNwidth, 	alpha_axp_logo_width); cArgs++;
	XtSetArg( pArgs[cArgs], XmNheight, 	alpha_axp_logo_height); cArgs++;
	XtSetArg( pArgs[cArgs], XmNborderWidth,	 0		); cArgs++;
	XtSetArg( pArgs[cArgs], XmNoverrideRedirect, TRUE	); cArgs++;
	XtSetArg( pArgs[cArgs], XmNsaveUnder, 	 FALSE		); cArgs++;
	XtSetArg( pArgs[cArgs], XmNbackgroundPixmap, ParentRelative); cArgs++;
	axpLogoWidget = XtCreatePopupShell( "AXPLogo",
					 topLevelShellWidgetClass,
					 topWidget, pArgs, cArgs );

	exposeCallbacks[0].callback = (XtCallbackProc) HandleExpose;
	exposeCallbacks[0].closure  = (XtPointer) &AXPLogoData;
	exposeCallbacks[1].callback = NULL;
	exposeCallbacks[1].closure  = 0;
	cArgs = 0;
	XtSetArg( pArgs[cArgs], XmNx,		0		); cArgs++;
	XtSetArg( pArgs[cArgs], XmNy,		0		); cArgs++;
	XtSetArg( pArgs[cArgs], XmNwidth,	alpha_axp_logo_width); cArgs++;
	XtSetArg( pArgs[cArgs], XmNheight, 	alpha_axp_logo_height); cArgs++;
	XtSetArg( pArgs[cArgs], XmNexposeCallback, exposeCallbacks); cArgs++;
	axpDrawingWidget = XtCreateWidget( "AXP Logo Display", 
					    xmDrawingAreaWidgetClass,
					    axpLogoWidget, pArgs, cArgs );

	XtRealizeWidget( axpLogoWidget );
	AXPLogoData.display = display = XtDisplay( topWidget );
	AXPLogoData.window = XtWindow( axpDrawingWidget );

	AXPLogoData.width = alpha_axp_logo_width;
	AXPLogoData.height = alpha_axp_logo_height;

	if ( DisplayPlanes( display, DefaultScreen( display )) > 2 )
	    {
	    XmGetColors(XtScreen(logoWidget), 
			DefaultColormapOfScreen(XtScreen(logoWidget)),
			logoOptions.rootColor,
			NULL,
			&gcvalues.foreground,
			&gcvalues.background,
			NULL);
	    }
	else
	    {
	    gcvalues.foreground = WhitePixelOfScreen(XtScreen(logoWidget));
	    gcvalues.background = BlackPixelOfScreen(XtScreen(logoWidget));
	    }

	AXPLogoData.gc = XtGetGC(axpDrawingWidget, GCForeground | GCBackground,
				    &gcvalues);

     
    	AXPLogoData.pixmap = XCreatePixmapFromBitmapData(
                XtDisplay(drawingWidget),
                XtWindow(drawingWidget),
                alpha_axp_logo_bits,
                alpha_axp_logo_width, alpha_axp_logo_height,
                gcvalues.foreground,
                gcvalues.background,
                axpDrawingWidget->core.depth);
	if(!AXPLogoData.pixmap) 
	    {
	    LogError("xdm: AXP logo pixmap create failed\n");
	    return;
	    }
	axp_logo_mask = XCreatePixmapFromBitmapData(
		XtDisplay(drawingWidget),
		XtWindow(drawingWidget),
		alpha_axp_logo_mask_bits,
		alpha_axp_logo_mask_width, alpha_axp_logo_mask_height,
		1, 0, /* foreground/background */
		1);

	if(!axp_logo_mask) {
	    LogError("xdm: AXP Logo Mask pixmap create failed\n");
	    return;
	    }
	}
#endif /* __alpha */


    if (XShapeQueryExtension(XtDisplay(logoWidget),&d1,&d2))
	{
	if (logo_mask)
	    XShapeCombineMask( XtDisplay(logoWidget),
		    XtWindow(logoWidget),
		    ShapeBounding,
		    0, 0,
		    logo_mask,
		    ShapeSet);
#ifdef __alpha
	if (logoOptions.showAxpLogo)
	    XShapeCombineMask( XtDisplay(axpLogoWidget),
		    XtWindow(axpLogoWidget),
		    ShapeBounding,
		    0, 0,
		    axp_logo_mask,
		    ShapeSet);
#endif
	}
    else
	{
	if (digital_logo_used)
	    {
	    XSetClipMask( display, gc, logo_mask );
	    XSetWindowBackgroundPixmap( display, window, ParentRelative );
	    }
#ifdef __alpha
	if (logoOptions.showAxpLogo)
	    {
	    XSetClipMask( display, AXPLogoData.gc, axp_logo_mask );
	    XSetWindowBackgroundPixmap( display, AXPLogoData.window, 
					ParentRelative );
	    }
#endif
	}

    /*
     * The mask can be freed now, but don't free the pixmap itself.
     */
    if (digital_logo_used)
	XFreePixmap( display, logo_mask );
    XtManageChild( drawingWidget );
    XtPopup( logoWidget, XtGrabNone );
#ifdef __alpha
	if (logoOptions.showAxpLogo)
	{
	XFreePixmap( display, axp_logo_mask );
	XtManageChild( axpDrawingWidget );
	XtPopup( axpLogoWidget, XtGrabNone );
	}
#endif
  }

/*****************************************************************************
 * Callbacks 								     *
 *****************************************************************************/

static XtCallbackProc HandleExpose( Widget	widget,
				    caddr_t	client_data,
				    caddr_t	call_data )
  {
    PLogoData pLogoData = (PLogoData) client_data;
    GC	      gc = pLogoData->gc;
    Pixmap     pixmap = pLogoData->pixmap;

    XCopyArea( XtDisplay(widget), pixmap, XtWindow(widget), gc, 0, 0, 
		pLogoData->width, pLogoData->height,
	       0, 0 );
  }



static XtCallbackProc RestoreRoot( Widget	widget,
				    caddr_t	client_data,
				    caddr_t	call_data )
    {
    Display*	display;
    int		i;

    display = XtDisplay(widget);
    for (i=ScreenCount(display) - 1; i>=0; i--)
	{
	XSetWindowBackgroundPixmap(display, RootWindow(display, i), 
					(Pixmap) None);
	XClearWindow(display, RootWindow(display, i));
	}
    }
