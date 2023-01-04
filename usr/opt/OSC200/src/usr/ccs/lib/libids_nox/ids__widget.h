
/**************************************************************************
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

#ifndef IDS__WIDGET_H
#define IDS__WIDGET_H

/* define IdsVMS if we want the VMS function bindings to compile */
#ifdef VMS
#define IdsVMS
#endif

/*
** VAX VMS path (no ALPHA)
*/
#if (defined(VAXC) && !defined(ALPHA) && !defined(__alpha))
#define external globalref
/*
** Ultrix path (still no ALPHA)
*/
#elif ( !defined(ALPHA) && !defined(__alpha) )
#define external extern
#endif

#include <img/ids_alloc_colors.h>   /* IDS color match allocation definitions */
#include <ids__macros.h>	    /* IDS private macros                     */
#include <ids__pipe.h>		    /* IDS rendering pipe definitions         */
#include <IdsImage.h>


/*
**  Equated Symbols
*/
#define GPX_HACKS   /*	Compile code which gets around GPX limitations (BUGS):
		    **	- Don't create Pixmaps wider or taller than the screen,
		    **	- Don't use a WrkArea wider than screen_width - 32.
		    */
#define DEG_RAD 0.0174532925199432959	/* to convert degrees to radians    */
#define ClassCvtLevels	256		/* minimum ClassCvt output levels   */

#define GRA     0			/* Gray  bits/component index	    */
#define RED     0			/* Red   bits/component index	    */
#define GRN     1			/* Green bits/component index	    */
#define BLU     2			/* Blue  bits/component index	    */
#define GRAY    3			/* Gray  index	                    */

#define DoRGB	(DoRed|DoGreen|DoBlue)	/* use all RGB components of XColor */

#define PLANE	1			/* XYBitmap format plane to use	    */

#ifndef MIN
#define MIN(x,y)        ((x) < (y) ? (x) : (y))
#endif /* MIN */

#ifdef IdsVMS
typedef struct dsc$descriptor_s *VMS_STRING;
#endif



/*
**  struct defining a look up table for IdsPixelRemap.
*/
typedef struct _RemapLutStruct
    {
    unsigned long    stride;	/* total number of bits making up an entry  */
    unsigned long    mask;	/* mask of significant bits within an entry */
    unsigned long    bytes;	/* length of LUT in bytes		    */
    unsigned char   *base;	/* address of LUT array			    */
} RemapLutStruct, *RemapLut;
/*
**  struct for returning image histogram.
*/
typedef struct _HistogramDataStruct
    {
    unsigned long    count;		/* length  of the histogram array   */
    unsigned long   *pointer;		/* address of the histogram array   */
} IdsHistogramDataStruct, *IdsHistogramData;
/*
**  struct containing variables required for IdsApplyModel.
*/
typedef struct _ModelDataStruct
    {
    int		 spect_type;		    /* spectral type		    */
    int		 spect_cnt;		    /* spectral component count	    */
    int		 width;			    /* image width  in pixels	    */
    int		 height;		    /* image height in pixels	    */
    int		 pixel_bits;		    /* total bits per pixel	    */
    int		 bpc[Ids_MaxComponents];    /* bits per component (RGB)	    */
    int		 levels[Ids_MaxComponents]; /* levels per component (RGB)   */
    int          comp_space_org;            /* component space organizaion  */
} ModelDataStruct, *ModelData;
    /*
    **  Struct containing variables required during pixel allocation.
    */
typedef struct _PixelAllocStruct
    {
    unsigned long    spect_type;		/* spectral type	    */
    unsigned long    spect_count;		/* spectral count	    */
    unsigned long    pixel_bits;		/* bits per pixel	    */
    unsigned long    cmp_bpc[Ids_MaxComponents];/* bits per component	    */
    unsigned long    cmp_off[Ids_MaxComponents];/* component offset	    */
    unsigned long    cmp_msk[Ids_MaxComponents];/* component mask	    */
    unsigned long    cmp_lvl[Ids_MaxComponents];/* component maximum level  */
    unsigned long    polarity;			/* 0xffff to invert XColor  */
    unsigned long    count;			/* number of pixels required*/
    unsigned long   *pixels;			/* list of pixels required  */
    XColor          *colors;			/* list of XColors required */
} PixelAllocStruct, *PixelAlloc;

/*
**  struct containing rendered image and Work area display coordinates.
*/
typedef struct _DisplayCoords
    {
    int	    source_x;			/* IDS rendered image  X coordinate */
    int	    source_y;			/* IDS rendered image  Y coordinate */
    int	    work_x;			/* IDS image work area X coordinate */
    int	    work_y;			/* IDS image work area Y coordinate */
    } DisplayCoordsStruct, *DisplayCoords;

/*
**  Duplicate of application IdsWorkNotifyCallbackStruct PLUS "pipe_desc" field.
*/
typedef struct _WorkNotify
    {
    int		    reason;		/* IDS callback reason		    */
    XEvent	   *event;		/* X11 same as RenderCallback event */
    unsigned long   function;		/* IDS function(s) to execute next  */
    unsigned long   process;            /* IDS executed at client or server */
    IdsPipeDesc	    pipe_desc;		/* IDS pipe descriptor @ notify	    */
    } WorkNotifyStruct, *WorkNotify;

/*
**  Error callback function data structure
*/
typedef char (*ErrorCallProc) ();
typedef struct _ErrorProcCallback
    {
      ErrorCallProc     error_proc;
      char              *render_widget;
      Boolean           XIE_error;
    } IdsErrorFuncStruct, *IdsErrorFunc;

/*
**  struct containing the IDS rendering parameters.
*/
typedef struct _RenderContextStruct
    {
    int		     device_type;	/* IDS device type constant	      */
    int              xie_ext;           /* IDS crunching at client or server  */
    IdsErrorFunc     IdsErrorCb;        /* IDS to hold error cb function addr*/
    int              save_mode;         /* IDS save fid, xieimg, photo req    */
    int              cmpres_mode;       /* IDS save fid, xieimg, cmpress mode */
    int              cmporg_mode;       /* IDS save fid, xieimg, compmap mode */
    Boolean	     switch_mode;	/* IDS switch from ISL or XIE and ..  */
    IdsPipeDesc	     pipe_desc;		/* IDS rendering pipe descriptor      */
    IdsRenderCallback proposed_rcb;	/* IDS proposed rendering parameters  */
    PixelAlloc	     pixel_alloc;	/* IDS pixel allocation structure     */
    unsigned long   *pixel_index_list;	/* IDS list  of pixels allocated      */
    unsigned long    pixel_index_count; /* IDS count of pixels allocated      */
    unsigned long   *palette_index_list;/* APPL supplied list of pixels      */
    unsigned long    palette_index_count;/* APPL count of pixels             */
    unsigned long    colormap_mode;     /* IDS colormap mode                  */
    Boolean	     colormap_update;   /* IDS use appl supplied colormap     */
    XColor          *appl_color_list;   /* IDS pointer to store appl color lst*/
    IdsMatchData     pixel_match_data;	/* IDS color match statistics data    */
    unsigned long    color_space;	/* IDS color matching space	      */
    float	     match_limit;	/* IDS color sharing limit 0.0 to 1.0 */
    float	     gray_limit;	/* IDS max gray deviation  0.0 to 1.0 */
    XImage	    *ximage;		/* IDS pointer to output XImage	      */
    struct PUT_ITMLST *requant_itmlst;	/* IDS requantize item list	      */
    struct PUT_ITMLST *t_scale_itmlst;	/* IDS tone-scale item list	      */
    struct PUT_ITMLST *dither_itmlst;	/* ISL dither item list		      */
    unsigned long    requant_levels;	/* IDS spectral class convert levels  */
    unsigned long    t_scale_levels[3];	/* IDS array of levels for tone-scale */
    unsigned long    levels_gray;	/* IDS maximum levels of gray         */
    unsigned long    levels_rgb[3];	/* IDS maximum levels of R,G,B        */
    unsigned long    fit_levels;	/* IDS maximum colors to allocate     */
    unsigned long    fit_width;		/* IDS maximum width  requested	      */
    unsigned long    fit_height;	/* IDS maximum height requested	      */
    unsigned long    protocol;		/* IDS protocol (ximage, pixmap...)   */
    unsigned long    rendering_class;	/* IDS human visual class of rendering*/
    Display	    *display;		/* X11 display id		      */
    Screen	    *screen;		/* X11 screen pointer		      */
    Visual	    *visual;		/* X11 visual pointer		      */
    Window	     window;		/* X11 window id		      */
    Dimension        width, height;     /* X11 window dimensions              */
    Cardinal         depth;             /* X11 number of planes in window     */
    Colormap         colormap;          /* X11 colormap                       */
    int		     colormap_size;	/* X11 number of hdwr colormap cells  */
    int		     scanline_modulo;	/* X11 BitmapPad		      */
    int		     Z_bits_per_pixel;	/* X11 ZPixmap bits_per_pixel	      */
    int		     ps_flags;		/* ISL Postscript flags argument      */
} RenderContextStruct, *RenderContext;

typedef struct _RoiGeometryStruct{
    long int w;
    long int h;
    unsigned long x;
    unsigned long y;
    unsigned char cmd;
    unsigned char valid;
} RoiGeometryStruct, *RoiGeometry;

typedef struct _DataForXieStruct{
    int              ll_x;              /* Lower left X coord of  ISL frame   */
    int              ll_y;              /* Lower left Y coord of  ISL frame   */
    int              ur_x;              /* Upper right X coord of ISL frame   */
    int              ur_y;              /* Upper right Y coord of ISL frame   */
    unsigned long    roi_x;             /* Upper Right X coord of ROI frame  */ 
    unsigned long    roi_y;             /* Upper Right Y coord of ROI frame  */ 
    unsigned long    roi_w;             /* Width of ROI frame                */ 
    unsigned long    roi_h;             /* Height of ROI frame               */ 
    int              pp_dist;           /* Pixel Progression of image         */
    int              lp_dist;           /* Line Progression of image          */
    Pixmap	     pixmap;            /* Copy from widget stru for convenie */
    GC               zero_max_gc;       /* Copy StdGC from widget struct      */
    GC		     zero_min_gc;       /* Copy RevGC from widget struct     */ 
    GC               image_gc;		/* Copy GC ptr from widget struct     */
    Pixel            foreground_pixel;
    Pixel            background_pixel;
    unsigned long    internal_roi_x;  /* Upper left X coord of internal ROI */
    unsigned long    internal_roi_y;  /* Upper left Y coord of internal ROI */
    unsigned long    internal_roi_w;  /* Width of internal ROI               */
    unsigned long    internal_roi_h;  /* Height of internal ROI              */
    unsigned long    internal_ul_x;   /* Upper left X coord */
    unsigned long    internal_ul_y;   /* Upper left Y coord */
    unsigned long    internal_w_x;  /* Upper left X coord of window ROI */
    unsigned long    internal_w_y;  /* Upper left Y coord of window ROI */
    unsigned long    internal_w_width;  /* width of window ROI      */
    unsigned long    internal_w_height; /* Height of window ROI      */
    unsigned long    rotate_width;      /* Width after rotate */
    unsigned long    rotate_height;     /* Height after rotate */
} DataForXieStruct, *DataForXie;

typedef char (*PipeDoneCallProc) ();
typedef struct _RenderContextStructXie    {
    char           **xie_functions;     /* XIE list of supported funcs        */
    Boolean          rerender;          /* IDS if True do rerend on same Photo*/
    XiePhoto         photoflo;          /* XIE server image object for flow   */
    XiePhoto         raw_photo;         /* XIE server image object to rerender*/
    XiePhoto	     save_photo;        /* XIE photo after rendition          */
    XiePhoto	     disp_photo;        /* XIE photo just before display      */
    XiePhoto	     lut_photo;         /* XIE export lut photo for next expor*/
    DataForXie       xiedat;            /* all data needed to build the pipe  */
    PipeDoneCallProc pipedone;          /* execution of pipe done func ptr    */
    char            *render_widget;     /* keep wid id to pass back           */
    unsigned long    xie_trans_mode;	/* XIE transport mode   	      */
    RoiGeometry      roi;               /* IDS construct roi's for fill,crop..*/
    XieResource      fill_roi;          /* XIE fill geometry                  */
    XieResource      crop_roi;          /* XIE crop roi                       */
    XieResource      tsrc_roi;          /* XIE Move translate dst geometry    */
    XieResource      point_roi;         /* XIE Move translate dst geometry    */
    XieResource      tdst_roi;          /* XIE Move translate dst geometry    */
    XieResource	     logic_roi;         /* XIE Logical op ROI                 */
    XieResource	     math_roi;          /* XIE Logical op ROI                 */
    Boolean	     x_mirror;          /* XIE perform X flip                 */
    Boolean	     y_mirror;          /* XIE perform X flip                 */
} RenderContextStructXie, *RenderContextXie;

typedef struct _PhotoContextXie{
    unsigned long    rend_width;        /* XIE rendered width                 */
    unsigned long    rend_height;       /* XIE rendered height                */
    unsigned long    rend_depth;        /* XIE rendered depth                 */
    unsigned long    comp_levs[3];      /* XIE photo component count	      */
    char             comp_map;		/* XIE photo component mapping	      */
    char             comp_cnt;          /* XIE photo component count	      */
    char             pixel_pol;         /* XIE photo pixel polarity	      */
    char             pixel_prog;        /* XIE photo pixel progression        */
    char             line_prog;         /* XIE photo line progression         */
    char             constraint;	/* XIE photo constraint returned      */
    double	     pixel_ratio;	/* XIE photo aspect ratio	      */
    GC               pixmapGC;          /* temp bitonal GC until pipe lives   */
} PhotoContextStruct, *PhotoContext;

#endif /* IDS__WIDGET_H */
