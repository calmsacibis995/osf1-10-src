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

/*****************************************************************************
**
**  FACILITY:
**
**      IDS Examples
**
**  ABSTRACT:
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0, DECwindows V1.0
**
**  AUTHOR(S):
**
**      John Manyjars
**
**  CREATION DATE:
**
**	21-JUN-1988
**
**  MODIFICATION HISTORY:
**	B.C. Krishna	- Ultrix port
**	Dave Wecker	- not use John's macros
**      John O'Connor   - Clean up
**
*****************************************************************************/

/*
**  Include files
*/
#include <stdio.h>
#ifdef VMS
#include <decw$include/xatom.h>
#include <img/IdsImage.h>
#include <img/ImgDef.h>
#include <img/ImgEntry.h>
#else
#include <X11/Xatom.h>
#include <img/IdsImage.h>
#include <img/ImgDef.h>
#include <img/ImgEntry.h>
#endif

/*
**  Table of contents
*/

/*
**  Macro definitions
*/
#define X_DISPLAY_(lst) (Display *) (lst[0].value)
#define X_WINDOW_(lst)  (Window) (lst[1].value)
/*
**  Equated symbols
*/
#define INPUT_FILE1 1
#define INPUT_FILE2 2
#define INPUT_FILE3 3
#define TYPE        4

/*****************************************************************************
**  main
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**      argc - Count of arguments passed on the command line.
**
**	argv - Array of pointers to command line arguments. If specified,
**	       argv[1] is assumed to point to the input file name string.
**
*****************************************************************************/
main(argc, argv)
int	argc;
char	*argv[];
{
    
    Display *dpy;
    IdsItmlst2 ps_itmlst[10], rn_itmlst[10];
    int input_width, input_height;
    struct GET_ITMLST image_itmlst[3];
    Screen *screen; 
    XSetWindowAttributes xswa;
    long fid, fid2, fid3;
    long ctx, ctx2, ctx3;
    int img_wd, img_ht, proto;
    unsigned long int psid, psid2, psid3;
    IdsRendering *rendering, *rendering2, *rendering3;
    XImage *ximage, *ximage2, *ximage3;

    Window root;		    /* Returned root window identifier	    */
    int X,Y;			    /* X,Y coordinates of this window	    */
				    /* relative to its parent		    */
    int HEIGHT,WIDTH;		    /* Height and width of window in pels   */
    int border_width;		    /* Width of window border in pels	    */
    int DEPTH;			    /* Z depth of window		    */
    /*
    **	Character string to receive the input file name and a descriptor to
    **	specify it to ImgOpenFile().
    */
    char *filename;                 /* Pointer to resulting file name       */
    char name[80];		    /* Character string data	    */
    char *filename2;                 /* Pointer to resulting file name       */
    char name2[80];		    /* Character string data	    */
    char *filename3;                 /* Pointer to resulting file name       */
    char name3[80];		    /* Character string data	    */

    /*
    **  Get the input file name either from the command line arguments, or
    **	by prompting the user. A descriptor for the file name string is 
    **	initialized so it may be passed to OPEN_DDIF_FILE.
    */
    if ((argc == INPUT_FILE3 + 1) | (argc == TYPE + 1))
      {
	strcpy(name ,argv[INPUT_FILE1]);
	strcpy(name2,argv[INPUT_FILE2]);
	strcpy(name3,argv[INPUT_FILE3]);
      }
    else
      {
	printf( "\n image file 1?   " );
	if(gets(name) == NULL)
	  exit(1);

	printf( "\n image file 2?   " );
	if(gets(name2) == NULL)
	  exit(1);

	printf( "\n image file 3?   " );
	if(gets(name3) == NULL)
	  exit(1);
      }

    filename  = name;
    filename2 = name2;
    filename3 = name3;
   
    /*
    **	Protocol XImage or Pixmap.
    */
    if (argc == TYPE + 1)
	    proto = *argv[TYPE] - '0';
    else
      {
	printf("\n   0 -- XImage 1 -- Pixmap   ? ");
	scanf("%d", &proto);
      }

    img_wd = img_ht = 150;
    ps_itmlst[0].item_name = (char *) IdsNworkstation;
    ps_itmlst[0].value =  0;
    ps_itmlst[1].item_name = (char *) IdsNwsWindow;
    ps_itmlst[1].value = 0;
    ps_itmlst[2].item_name = (char *) IdsNprotocol;
    /*
    **  Define the protocol to either XImage or Pixmap
    */
    switch( proto )
        {
        case  1:
	        ps_itmlst[2].value = Ids_Pixmap;
                break;
        case  0 :
        default :
		ps_itmlst[2].value = Ids_XImage; 
                break;
        }
    ps_itmlst[3].item_name = (char *) IdsNwindowWidth;
    ps_itmlst[3].value = (unsigned long int)img_wd;
    ps_itmlst[4].item_name = (char *) IdsNwindowHeight;
    ps_itmlst[4].value = (unsigned long int)img_ht;
    ps_itmlst[5].item_name = (char *) NULL;
    ps_itmlst[5].value = NULL;

    rn_itmlst[0].item_name = (char *) IdsNscaleMode;
    rn_itmlst[0].value =  Ids_Flood;
    rn_itmlst[1].item_name = (char *) NULL;
    rn_itmlst[1].value = NULL;

    image_itmlst[0].GetL_Code    = (unsigned long int) Img_PixelsPerLine;
    image_itmlst[0].GetL_Length  = (unsigned long int) sizeof(int);
    image_itmlst[0].GetA_Buffer  = (char *) &input_width;
    image_itmlst[0].GetA_Retlen  = (unsigned long int *) NULL;
    image_itmlst[0].GetL_Index   = (unsigned long int) 0;

    image_itmlst[1].GetL_Code    = (unsigned long int) Img_NumberOfLines;
    image_itmlst[1].GetL_Length  = (unsigned long int) sizeof(int);
    image_itmlst[1].GetA_Buffer  = (char *) &input_height;
    image_itmlst[1].GetA_Retlen  = (unsigned long int *) NULL;
    image_itmlst[1].GetL_Index   = (unsigned long int) 0;

    image_itmlst[2].GetL_Code    = (unsigned long int) NULL;
    image_itmlst[2].GetL_Length  = (unsigned long int) NULL;
    image_itmlst[2].GetA_Buffer  = (char *) NULL;
    image_itmlst[2].GetA_Retlen  = (unsigned long int *) NULL;
    image_itmlst[2].GetL_Index   = (unsigned long int) NULL;

    /*
    **	Open connect to display
    */
    if (!(ps_itmlst[0].value = 
	  (unsigned long int) XOpenDisplay((char *) getenv("DECW$DISPLAY"))))
	{
	    perror("Can't open display");
	    exit(-1);
	}
    screen = XDefaultScreenOfDisplay(X_DISPLAY_(ps_itmlst));
    /*
    **	Get image from DDIF stream file. Retrieve the width and height of the
    **	image in pixels.
    */
    ctx = ImgOpenFile(ImgK_ModeImport,
		      ImgK_FtypeDDIF,
		      strlen(filename),
		      filename,
		      0,               /* itmlst */
		      0);              /* flags  */
    fid = ImgImportFrame(ctx,0);
    ImgGetFrameAttributes(fid, image_itmlst);
    ImgCloseFile(ctx,0);
    printf("\nSource Image: %5d by %5d", input_width, input_height);

    ctx2 = ImgOpenFile(ImgK_ModeImport,
		       ImgK_FtypeDDIF,
		       strlen(filename2),
		       filename2,
		       0,               /* itmlst */
		       0);              /* flags  */
    fid2 = ImgImportFrame(ctx2,0);
    ImgGetFrameAttributes(fid2, image_itmlst);
    ImgCloseFile(ctx2,0);
    printf("\nSource Image: %5d by %5d", input_width, input_height);

    ctx3 = ImgOpenFile(ImgK_ModeImport,
		       ImgK_FtypeDDIF,
		       strlen(filename3),
		       filename3,
		       0,               /* itmlst */
		       0);              /* flags  */
    fid3 = ImgImportFrame(ctx3,0);
    ImgGetFrameAttributes(fid3, image_itmlst);
    ImgCloseFile(ctx3,0);
    printf("\nSource Image: %5d by %5d", input_width, input_height);

    /*
    **	Create a window to display the image in
    */
    xswa.background_pixel = 
	        WhitePixel(X_DISPLAY_(ps_itmlst), 0);
    xswa.border_pixel = 
	        BlackPixel(X_DISPLAY_(ps_itmlst), 0);

    ps_itmlst[1].value = (unsigned long int) 
      XCreateWindow(
	X_DISPLAY_(ps_itmlst),			    /* Display ID   */
	RootWindow((X_DISPLAY_(ps_itmlst)),0),          /* Parent	    */
	0,					    /* X coordinate */
	0,					    /* Y coordinate */
	800, 800,
	5,					    /* Border width */
	XDefaultDepth(X_DISPLAY_(ps_itmlst),0),	    /* Depth	    */
	InputOutput,				    /* Class	    */
	DefaultVisual(X_DISPLAY_(ps_itmlst), 0),/* Visual type  */
	CWBackPixel | CWBorderPixel,		    /* attr. mask   */
	&xswa);					    /* Window struct*/
    /*
    **	Set window manager title
    */
    XChangeProperty(X_DISPLAY_(ps_itmlst),	/* X display identifier	    */
		    X_WINDOW_(ps_itmlst),	/* X window identifier	    */
		    XA_WM_NAME,			/* Name property	    */
		    XA_STRING,			/* Property type constant   */
		    8,				/* Eight-bit format	    */
		    PropModeReplace,		/* Change mode = replace    */
		    (unsigned char *) filename, /* Value of property	    */
		    strlen(filename));      	/* Size of property value   */
    /*
    **	Map the window and wait for stuff to happen...
    */
    XMapWindow(X_DISPLAY_(ps_itmlst),X_WINDOW_(ps_itmlst));
    XSync(X_DISPLAY_(ps_itmlst), 0);	    
    /*
    **	Define a presentation surface which describes this window
    */
    psid  = IdsCreatePresentSurface(ps_itmlst);
    psid2 = IdsCreatePresentSurface(ps_itmlst);
    psid3 = IdsCreatePresentSurface(ps_itmlst);
    /*
    **	Render the image for the window
    */
    rendering = IdsCreateRendering(fid,psid,rn_itmlst);

    /*
    **	Render the image for the window
    */
    rendering2 = IdsCreateRendering(fid2,psid2,rn_itmlst);

    /*
    **	Render the image for the window
    */
    rendering3 = IdsCreateRendering(fid3,psid3,rn_itmlst);

    XSync(X_DISPLAY_(ps_itmlst), 0);	    

    /*
    **	Copy the image or the pixmap into the window and wait for it to happen
    */
    ximage  = (XImage *) rendering->type_spec_data.xlib.ximage;
    printf("\nRendered Image 1: %5d by %5d", ximage->width, ximage->height);
    ximage2 = (XImage *) rendering2->type_spec_data.xlib.ximage;
    printf("\nRendered Image 2: %5d by %5d", ximage2->width, ximage2->height);
    ximage3 = (XImage *) rendering3->type_spec_data.xlib.ximage;
    printf("\nRendered Image 3: %5d by %5d", ximage3->width, ximage3->height);
    printf("\n");

    /*
    **	Copy the image or the pixmap into the window and wait for it to happen
    */
    if( rendering->type == Ids_XImage)
     {
     XPutImage(X_DISPLAY_(ps_itmlst),		    /* display id	    */
	      X_WINDOW_(ps_itmlst),		    /* window id	    */
	      rendering->type_spec_data.xlib.image_gc, /* GC returned     */
	      ximage,				    /* X image structure    */
	      0,				    /* Src X coordinate	    */
	      0,				    /* Src Y coordinate	    */
	      0,				    /* Dst X coordinate	    */
	      400,				    /* Dst Y coordinate	    */
	      ximage->width,			    /* Width of image	    */
	      ximage->height);		    /* Height of image	    */
    XSync(X_DISPLAY_(ps_itmlst), 0);	    
    XPutImage(X_DISPLAY_(ps_itmlst),		    /* display id	    */
	      X_WINDOW_(ps_itmlst),		    /* window id	    */
	      rendering2->type_spec_data.xlib.image_gc, /* GC returned     */
	      ximage2,				    /* X image structure    */
	      0,				    /* Src X coordinate	    */
	      0,				    /* Src Y coordinate	    */
	      0,				    /* Dst X coordinate	    */
	      0,				    /* Dst Y coordinate	    */
	      ximage2->width,			    /* Width of image	    */
	      ximage2->height);			    /* Height of image	    */
     XSync(X_DISPLAY_(ps_itmlst), 0);	    
     XPutImage(X_DISPLAY_(ps_itmlst),		    /* display id	    */
	      X_WINDOW_(ps_itmlst),		    /* window id	    */
	      rendering3->type_spec_data.xlib.image_gc, /* GC returned     */
	      ximage3,				    /* X image structure    */
	      0,				    /* Src X coordinate	    */
	      0,				    /* Src Y coordinate	    */
	      400,				    /* Dst X coordinate	    */
	      0,				    /* Dst Y coordinate	    */
	      ximage3->width,			    /* Width of image	    */
	      ximage3->height);			    /* Height of image	    */
    XSync(X_DISPLAY_(ps_itmlst), 0);	    
     }

    else if( rendering->type == Ids_Pixmap && rendering2->type == Ids_Pixmap &&
         rendering3->type == Ids_Pixmap )
     {
       if( ximage->format == ZPixmap && 
				   rendering->type_spec_data.xlib.pixmap != 0 )
	{
        XCopyArea( X_DISPLAY_(ps_itmlst),
                    rendering->type_spec_data.xlib.pixmap,
                    X_WINDOW_(ps_itmlst),
	      	    rendering->type_spec_data.xlib.image_gc, /* GC returned    */
                    0, 0,
                    ximage->width,
                    ximage->height, 0, 400);
	XSync(X_DISPLAY_(ps_itmlst), 0);	    
        }
       else
	{
        XCopyPlane( X_DISPLAY_(ps_itmlst),
                    rendering->type_spec_data.xlib.pixmap,
                    X_WINDOW_(ps_itmlst),
	      	    rendering->type_spec_data.xlib.image_gc, /* GC returned    */
                    0, 0,ximage->width,
                    ximage->height,
                    0, 400, 1);
	XSync(X_DISPLAY_(ps_itmlst), 0);	    
	}
       if( ximage2->format == ZPixmap && 
		                   rendering2->type_spec_data.xlib.pixmap != 0)
	{
        XCopyArea( X_DISPLAY_(ps_itmlst),
                    rendering2->type_spec_data.xlib.pixmap,
                    X_WINDOW_(ps_itmlst),
	      	    rendering2->type_spec_data.xlib.image_gc, /* GC returned   */
                    0, 0,
                    ximage2->width,
                    ximage2->height, 0, 0);
	XSync(X_DISPLAY_(ps_itmlst), 0);	    
        }
       else
	{
        XCopyPlane( X_DISPLAY_(ps_itmlst),
                    rendering2->type_spec_data.xlib.pixmap,
                    X_WINDOW_(ps_itmlst),
	      	    rendering2->type_spec_data.xlib.image_gc, /* GC returned   */
                    0, 0,ximage2->width,
                    ximage2->height,
                    0, 0, 1);
	XSync(X_DISPLAY_(ps_itmlst), 0);	    
	}
       if( ximage3->format == ZPixmap && 
				    rendering3->type_spec_data.xlib.pixmap != 0)
	{
        XCopyArea( X_DISPLAY_(ps_itmlst),
                    rendering3->type_spec_data.xlib.pixmap,
                    X_WINDOW_(ps_itmlst),
	      	    rendering3->type_spec_data.xlib.image_gc, /* GC returned   */
                    0, 0,
                    ximage3->width,
                    ximage3->height, 400, 0);
	XSync(X_DISPLAY_(ps_itmlst), 0);	    
        }
       else
	{
        XCopyPlane( X_DISPLAY_(ps_itmlst),
                    rendering3->type_spec_data.xlib.pixmap,
                    X_WINDOW_(ps_itmlst),
	      	    rendering3->type_spec_data.xlib.image_gc, /* GC returned   */
                    0, 0,ximage3->width,
                    ximage3->height,
                    400, 0, 1);
	XSync(X_DISPLAY_(ps_itmlst), 0);	    
	}
     }

    
    /*
    **	Wait for CR
    */
    getchar();
    getchar();

    IdsDeletePresentSurface(psid);
    IdsDeletePresentSurface(psid2);
    IdsDeletePresentSurface(psid3);
    IdsDeleteRendering(rendering);
    IdsDeleteRendering(rendering2);
    IdsDeleteRendering(rendering3);
    ImgDeleteFrame(fid);
    ImgDeleteFrame(fid2);
    ImgDeleteFrame(fid3);
}
