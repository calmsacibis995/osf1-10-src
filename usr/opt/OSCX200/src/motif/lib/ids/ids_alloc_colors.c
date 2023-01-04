
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
**      This module contains routines for allocating X11 colors.
**	
**	
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.1 or Ultrix V?.?
*	DECwindows V1.0
**
**  AUTHOR(S):
**
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      February 15, 1989
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Include files
*/
    /*
    **  Standard C include files
    */
#include <math.h>
    /*
    **  DECwindows and IDS include files
    */
#include    <img/ids_alloc_colors.h>
#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif

/*
**  Table of contents
*/
    /*
    **  PUBLIC -- high level entry points
    */
#ifdef NODAS_PROTO
IdsAllocStatistics IdsGetAllocStatistics(); /* get color match statistics   */
void		   IdsFreeXColors();	    /* free colors from XColor list */
IdsMatchData	   IdsAllocColors();	    /* IDS friendly color allocation*/
IdsMatchData	   IdsMatchColors();	    /* IDS appl req colors matching */
    /*
    **  PUBLIC -- low level entry points
    */
XColor		  *IdsGetColormapState();   /* get state of colormap cells  */
IdsColor	   IdsColorSpaceMinMax();   /* find min and max distances   */
#endif

/*
**  MACRO definitions
*/
    /*
    **  Shift unmeaningful bits out of RGB components and normalize the value.
    */
#define RED_(xcolor,shift,divisor) ((float)(xcolor->red  >> shift) / divisor)
#define GRN_(xcolor,shift,divisor) ((float)(xcolor->green>> shift) / divisor)
#define BLU_(xcolor,shift,divisor) ((float)(xcolor->blue >> shift) / divisor)
    /*
    **  Conversion macros from RGB to UVW, XYZ, and YIQ.
    */
#define Y_(c,s,d) (0.299*RED_(c,s,d) + 0.587*GRN_(c,s,d) + 0.114*BLU_(c,s,d))
#define U_(c,s,d) (0.405*RED_(c,s,d) + 0.116*GRN_(c,s,d) + 0.133*BLU_(c,s,d))
#define W_(c,s,d) (0.145*RED_(c,s,d) + 0.827*GRN_(c,s,d) + 0.627*BLU_(c,s,d))
#define X_(c,s,d) (0.607*RED_(c,s,d) + 0.174*GRN_(c,s,d) + 0.201*BLU_(c,s,d))
#define Z_(c,s,d)  /* no RED */      ( 0.066*GRN_(c,s,d) + 1.117*BLU_(c,s,d))
#define I_(c,s,d) (0.596*RED_(c,s,d) - 0.275*GRN_(c,s,d) - 0.321*BLU_(c,s,d))
#define Q_(c,s,d) (0.212*RED_(c,s,d) - 0.523*GRN_(c,s,d) + 0.311*BLU_(c,s,d))

#define FreeMem_(m) (XtFree((char *)*(m)),*(m)=0)
#define Min3_(a,b,c) ((a)<(b)?((a)<(c)?(a):(c)):(b)<(c)?(b):(c))
#define Max3_(a,b,c) ((a)>(b)?((a)>(c)?(a):(c)):(b)>(c)?(b):(c))

/*
**  Equated Symbols
*/
#define F_zero	((float)0.0)
#define F_half	((float)0.5)
#define F_one	((float)1.0)
#define F_two	((float)2.0)
#define F_three	((float)3.0)
#define F_five	((float)5.0)

#define LabrefX	0.982		/* X tristimulus  value for reference white */
#define LabrefY	1.000		/* Y tristimulus  value for reference white */
#define LabrefZ	1.183		/* Z tristimulus  value for reference white */
#define LUVrefU	0.201		/* u chromaticity value for reference white */
#define LUVrefV	0.461		/* v chromaticity value for reference white */
#define UVWrefU	0.201		/* u chromaticity value for reference white */
#define UVWrefV	0.307		/* v chromaticity value for reference white */

#define DoRGB	(DoRed|DoGreen|DoBlue)	/* use all RGB components of XColor */

    /*
    **  Struct containing variables required for color-match pixel allocation.
    */
typedef struct _ColorMatchStruct
    {
    Display	    *dpy;	/* X11 display pointer			    */
    Colormap	     cmap;	/* X11 colormap id			    */
    unsigned long    cells;	/* number of colormap entries		    */
    unsigned long    src_num;	/* number of colors to allocate		    */
    unsigned long    dst_num;	/* number of colors allocated		    */
    XColor	    *colors;	/* array of XColors to allocate		    */
    XColor	    *maplst;	/* array of XColors: snapshot of colormap   */
    unsigned long    space;	/* color space to match colors in	    */
    void	   (*convert)();/* RGB to color space conversion routine    */
    IdsColor	     request;	/* color space conversion of color list	    */
    IdsColor	     palette;	/* color space conversion of colormap	    */
    float	     range;	/* distance range of color match space	    */
    float	     limit;	/* distance limit for sharing colors	    */
    float	    *deltas;	/* array of distances from desired color    */
    float	     delta0;	/* beginning of distance array		    */
} ColorMatchStruct, *ColorMatch;

/*
**  External References
*/
    /* none */
/*
**  internal routines
*/
#ifdef NODAS_PROTO
static void		 AllocDistantColors();	/* allocate colors >= delta */
static Boolean		 AllocNewColor();	/* allocate a new color	    */
static void		 ShareClosestColors();	/* allocate colors <= delta */
static void		 SearchPalette();	/* match color from palette */
static void		 UpdateColors();	/* update closer distances  */
static void		 SetColorSpace();	/* set convert/range/limit  */
static void		 RGBtoHLS();		/* RGB to HLS color space   */
static void		 RGBtoLAB();		/* RGB to Lab color space   */
static void		 RGBtoLUV();		/* RGB to L*U*V* color space*/
static void		 RGBtoRGB();		/* RGB to RGB color space   */
static void		 RGBtoUVW();		/* RGB to U*V*W* color space*/
static void		 RGBtoYIQ();		/* RGB to YIQ color space   */

static IdsMatchData	_IdsAllocPseudoColor();
static IdsMatchData	_IdsAllocTrueColor();
static IdsMatchData	_IdsMatchStaticColor();
#else
PROTO(static IdsMatchData _IdsAllocPseudoColor, (Screen */*screen*/, ColorMatch /*cm*/, unsigned long /*match_space*/, double /*match_limit*/, double /*gray_limit*/));
PROTO(static IdsMatchData _IdsMatchStaticColor, (Screen */*screen*/, ColorMatch /*cm*/, unsigned long /*match_space*/, double /*match_limit*/, double /*gray_limit*/));
PROTO(static IdsMatchData _IdsAllocTrueColor, (Screen */*screen*/, ColorMatch /*cm*/));
PROTO(static void AllocDistantColors, (ColorMatch /*cm*/));
PROTO(static Boolean AllocNewColor, (ColorMatch /*cm*/, unsigned long /*index*/));
PROTO(static void ShareClosestColors, (ColorMatch /*cm*/));
PROTO(static void SearchPalette, (ColorMatch /*cm*/, unsigned long /*match*/));
PROTO(static void UpdateColors, (ColorMatch /*cm*/, unsigned long /*match*/));
PROTO(static void SetColorSpace, (ColorMatch /*cm*/, unsigned long /*space*/, double /*limit*/));
PROTO(static void RGBtoHLS, (XColor */*rgb*/, IdsColor /*new*/, unsigned long /*offset*/));
PROTO(static void RGBtoLAB, (XColor */*rgb*/, IdsColor /*new*/, unsigned long /*offset*/));
PROTO(static void RGBtoLUV, (XColor */*rgb*/, IdsColor /*new*/, unsigned long /*offset*/));
PROTO(static void RGBtoRGB, (XColor */*rgb*/, IdsColor /*new*/, unsigned long /*offset*/));
PROTO(static void RGBtoUVW, (XColor */*rgb*/, IdsColor /*new*/, unsigned long /*offset*/));
PROTO(static void RGBtoYIQ, (XColor */*rgb*/, IdsColor /*new*/, unsigned long /*offset*/));
#endif
/*
**	Local Storage
*/
    /* none */

/*******************************************************************************
**  IdsGetColormapState
**
**  FUNCTIONAL DESCRIPTION:
**
**	Get the contents of the colormap -- determine the state of each cell.
**
**  FORMAL PARAMETERS:
**
**	screen	- X11 screen structure pointer
**	cmap	- X11 colormap id to allocate color cells from
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
**      *XColor - list of XColor structures containing a snapshot of 'colormap'.
**		  -- each 'pad' field contains state:
**
**			Ids_AllocShared  - read-only sharable color cell
**			Ids_AllocPrivate - read/write private color cell
**			Ids_AllocFree    - available colormap cell
**
**		   Deallocate XColor list using XtFree.
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
XColor *IdsGetColormapState( screen, cmap )
 Screen	    *screen;
 Colormap    cmap;
{
    unsigned long free, i, *pxl, cells = CellsOfScreen( screen );
    Display *dpy = DisplayOfScreen( screen );
    XColor  *map;

#ifdef TRACE
printf( "Entering Routine IdsGetColormapState in module IDS_ALLOC_COLORS \n");
#endif
    /*
    **	Allocate the colormap state list and also a temporary pixel list.
    */
    map = (XColor *)        XtMalloc( cells * sizeof(XColor));
    pxl  = (unsigned long *) XtMalloc( cells * sizeof( long ));

    /*
    **  Be unsociable and grab the server (actually we're being friendly
    **  by not letting anyone notice that the colormap will be temporarily
    **  exhausted -- what they don't know may be good for them).
    **
    **	WARNING: DON'T STEP THRU HERE WITH THE DEBUGGER IF YOU'RE RUNNING
    **		 THE DEBUGGER ON THE SAME DISPLAY -- YOU'LL HANG YOURSELF!
    */
    XGrabServer( dpy );

    /*
    **  Allocate all available color cells privately (binary search technique).
    */
    for( free = 0, i = cells; i > 0; i >>= 1 )
	if( XAllocColorCells( dpy, cmap, False, 0,0, pxl+free, i ))
	    free += i++;

    /*
    **  Read the entire colormap.
    */
    for( i = 0; i < cells; map[i].pixel = i++ );
    XQueryColors( dpy, cmap, map, cells );

    /*
    **  Initialize each cell's state as "Private", then change our
    **  own privately allocated color cells' state to "Free".
    */
    for(i = 0; i < cells; map[i++].pad      = Ids_AllocPrivate);
    for(i = 0; i < free;  map[pxl[i++]].pad = Ids_AllocFree   );

    /*
    **	Figure out which "Private" cells are actually allocated "Shared".
    */
    for( i = 0; i < cells; i++ )
	if( map[i].pad == Ids_AllocPrivate && XAllocColor( dpy, cmap, map+i ))
	    {
	    map[map[i].pixel].pad = Ids_AllocShared;
	    pxl[free++]  = map[i].pixel;
	    map[i].pixel = i;
	    }

    /*
    **  Now we can quit being a resource hog.
    */
    XFreeColors( dpy, cmap, pxl, free, 0 );
    XUngrabServer( dpy );
    XSync( dpy, False );	/* WITHOUT THIS THE WORKSTATION WILL HANG ! */
    XtFree( (char *)pxl );

    /*
    **  Return our snapshot of the colormap.
    */
#ifdef TRACE
printf( "Leaving Routine IdsGetColormapState in module IDS_ALLOC_COLORS \n");
#endif
    return( map );
}

/*******************************************************************************
**  IdsColorSpaceMinMax
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert a list of RGB XColor values into another color space.  
**	Optionally calculate the distance between each color and every other 
**	color, using the selected color space, returning the minimum and
**	maximum distances, and the pixel indices for these pairs of colors.
**
**  FORMAL PARAMETERS:
**
**	colors	 - list of XColors for making comparisons
**	count	 - number of XColors
**	space	 - color space to calculate distances for
**	sig_bits - number of significant RGB bits (ie. visual->bits_per_rgb)
**	find	 - Boolean flag: TRUE = find min and max, FALSE = don't.
**	min_dist - address of where to store the minimum distance
**	min_in_1 - address of where to store color list index -- 1st of pair
**	min_in_2 - address of where to store color list index -- 2nd of pair
**	max_dist - address of where to store the maximum distance
**	max_in_1 - address of where to store color list index -- 1st of pair
**	max_in_2 - address of where to store color list index -- 2nd of pair
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
**      IdsColor - list of colors converted to the requested color space.
**
**		   Deallocate IdsColor list using XtFree.
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
IdsColor IdsColorSpaceMinMax( colors, count, space, sig_bits, find,
				      min_dist, min_in_1, min_in_2,
				      max_dist, max_in_1, max_in_2 )
 XColor		*colors;
 unsigned long	 count, space, sig_bits, find;
 double		*min_dist, *max_dist;
 unsigned long	*min_in_1, *min_in_2, *max_in_1, *max_in_2;

{
    ColorMatchStruct cm;
    double distance, minimum, maximum, t1, t2, t3;
    unsigned long i, j, offset = 16 - sig_bits;

    IdsColor list = (IdsColor)   XtMalloc( count * sizeof(IdsColorStruct));

#ifdef TRACE
printf( "Entering Routine IdsColorSpaceMinMax in module IDS_ALLOC_COLORS \n");
#endif

    /*
    **	Create the color match space equivalent for each color.
    */
    SetColorSpace( &cm, space, 0.0 );
    for( i = 0; i < count; ++i )
	(*cm.convert)( colors+i, list+i, offset );

    if( find )
	/*
	**	compare every color to every other color.
	*/
	for( minimum = 1e10, maximum = 0.0, i = 0; i < count; ++i )
	    for( j = 0;  j < count;  j++ )
		{
		if( j == i )
		    continue;
		t1 = list[i].t1 - list[j].t1;
		t2 = list[i].t2 - list[j].t2;
		t3 = list[i].t3 - list[j].t3;

		if( space == Ids_HLSSpace )
		    {
		    if( t1  < (float)-F_one )
			t1 += F_two;
		    else   if((float) F_one < t1 )
			t1 -= F_two;
		    }
		distance = t1*t1 + t2*t2 + t3*t3;   /* do square root later */

		if( distance < minimum )
		    {
		     minimum  = distance;	    /* new minimum distance */
		    *min_in_1 = i;
		    *min_in_2 = j;
		    }
		if( distance > maximum )
		    {
		     maximum  = distance;	    /* new maximum distance */
		    *max_in_1 = i;
		    *max_in_2 = j;
		    }
		}
    *min_dist = sqrt( minimum );
    *max_dist = sqrt( maximum );

#ifdef TRACE
printf( "Leaving Routine IdsColorSpaceMinMax in module IDS_ALLOC_COLORS \n");
#endif

    return( list );
}

/*******************************************************************************
**  IdsGetAllocStatistics
**
**  FUNCTIONAL DESCRIPTION:
**
**      Get statistics of allocations whose match distance exceeded 'threshold'.
**
**  FORMAL PARAMETERS:
**
**	data	    - IdsMatchData pointer
**	threshold   - match distance threshold above which to gather statistics
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
**      IdsAllocStatistics - statistics of matched colors exceeding 'threshold'.
**
**		   Deallocate IdsAllocStatistics using XtFree.
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
IdsAllocStatistics IdsGetAllocStatistics( data, threshold )
 IdsMatchData	data;
 double		threshold;
{
    ColorMatch    cm = (ColorMatch) data;
    IdsAllocStatistics stat;
    double	  sd0, sd1, sd2;
    double	  sqroots = 0.0, squares = 0.0;
    unsigned long index;

#ifdef TRACE
printf( "Entering Routine IdsGetAllocStatistics in module IDS_ALLOC_COLORS \n");
#endif
    /*
    **	Instantiate an IdsAllocStatisticsStruct.
    */
    stat = (IdsAllocStatistics) XtCalloc( 1, sizeof(IdsAllocStatisticsStruct));
    stat->total = cm->src_num;

    /*
    **	Convert the threshold to a distance in the color space originally used.
    */
    SetColorSpace( cm, cm->space, threshold );

    /*
    **  Find out how many matches exceed 'threshold', and how far they deviated.
    **	NOTE: cm->deltas[] contains the squares of the distances.
    */
    for( index = 0; index < cm->src_num; index++ )
	if( cm->deltas[index] > cm->limit )
	    {
	    stat->matches++;
	    sqroots += sqrt( (double) cm->deltas[index] );
	    squares += (double) cm->deltas[index];
	    }

    if( stat->matches > 0 )
	{
	/*
	**  Compute Mean and StdDev of error from color matches.
	*/
	stat->mean    = sqroots / stat->matches;
	stat->std_dev = sqrt(( squares - stat->matches * stat->mean*stat->mean )
				       /(stat->matches
				       -(stat->matches > 1 ? 1 : 0 )));
	sd0 =  stat->mean *   stat->mean;
	sd1 = (stat->mean +   stat->std_dev) * (stat->mean +   stat->std_dev);
	sd2 = (stat->mean + 2*stat->std_dev) * (stat->mean + 2*stat->std_dev);

	/*
	**  Count matches exceeding the mean error.
	*/
	for( index = 0; index < cm->src_num; index++ )
	    if( (double) cm->deltas[index] > sd0 )
		if( (double) cm->deltas[index] < sd1 )
		    stat->sdev_1++;
		else if( (double) cm->deltas[index] < sd2 )
		    stat->sdev_2++;
		else
		    stat->sdev_3++;
	}

    /*
    **  Normalize the results to the range 0.0 thru 1.0.
    */
    stat->mean     /= sqrt( (double) cm->range );
    stat->std_dev  /= sqrt( (double) cm->range );

#ifdef TRACE
printf( "Leaving Routine IdsGetAllocStatistics in module IDS_ALLOC_COLORS \n");
#endif
    return( stat );
}

/*******************************************************************************
**  IdsFreeXColors
**
**  FUNCTIONAL DESCRIPTION:
**
**      Deallocate pixels specified in a list of XColor structures.
**
**  FORMAL PARAMETERS:
**
**	screen	     - X11 screen structure pointer
**	colormap     - X11 colormap id to free color cells from
**	colors	     - list of XColor structures (with pixels to be freed)
**	count	     - number of colors in list
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
void IdsFreeXColors( screen, colormap, colors, count )
 Screen		*screen;
 Colormap	 colormap;
 XColor		*colors;
 unsigned long	 count;
{
    unsigned long i, *list;

#ifdef TRACE
printf( "Entering Routine IdsFreeXColors in module IDS_ALLOC_COLORS \n");
#endif
    if (colors != 0 && 
	count != 0  && 
	DefaultVisualOfScreen(screen)->class != TrueColor)
	{
	list = (unsigned long *) XtMalloc( count * sizeof(long));
	for( i = 0; i < count; i++ )
	    list[i] = colors[i].pixel;

	XFreeColors( DisplayOfScreen(screen), colormap, list, count, 0 );
	XtFree( (char *)list );
	}
#ifdef TRACE
printf( "Leaving Routine IdsFreeXColors in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  IdsAllocColors
**
**  FUNCTIONAL DESCRIPTION:
**
**      Allocate XColors using IDS color matching algorithm.
**
**  FORMAL PARAMETERS:
**
**	screen	     - X11 screen structure pointer
**	colormap     - X11 colormap id to allocate color cells from
**	colors	     - list of XColor structures (colors requested)
**	count	     - number of colors in list
**	match_space  - color space to use for color matching:
**			    (HLS, Lab, L*U*V*, RGB, U*V*W*, YIQ)
**	match_limit  - describes the distance beyond which new colors will be
**		       allocated: 0.0 = all new colors, 1.0 = match all colors,
**		       other values will yield a mixed result.
**	gray_limit   - describes the amount a sharable colormap color may 
**		       deviate from pure gray: 0.0 = accept only pure grays,
**			    1.0 = accept all colors (Use 1.0 for color images)
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      Contents of XColor cells modified to indentify pixel index allocated,
**	actual RGB values allocated, and status (exact, match, or failed).
**
**  FUNCTION VALUE:
**
**      IdsMatchData - IDS color match statistical data.
**
**	    or NULL if no colors allocated (eg. colormap is AllocAll==private)
**
**		   Deallocate IdsMatchData using XtFree.
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
IdsMatchData IdsAllocColors( screen, colormap, colors, count, 
			     match_space, match_limit, gray_limit )
 Screen		*screen;
 Colormap	 colormap;
 XColor		*colors;
 unsigned long	 count, match_space;
 double		 match_limit, gray_limit;

{
    ColorMatch	cm;
#ifdef TRACE
printf( "Entering Routine IdsAllocColors in module IDS_ALLOC_COLORS \n");
#endif


    /*
    **	Create the color match environment.
    */
    cm		= (ColorMatch) XtCalloc(1, sizeof(ColorMatchStruct)
					    + count * sizeof(float));
    cm->dpy     = DisplayOfScreen( screen );
    cm->cmap    = colormap;
    cm->cells	= CellsOfScreen( screen );
    cm->src_num = count;
    cm->colors  = colors;

    switch (DefaultVisualOfScreen(screen)->class)
    {
    case TrueColor:
	return(_IdsAllocTrueColor(screen,cm));
    case StaticColor:
	return(_IdsMatchStaticColor
		(screen,cm,match_space,match_limit,gray_limit));
    case StaticGray:
    case GrayScale:
    case PseudoColor:
	return(_IdsAllocPseudoColor
		(screen,cm,match_space,match_limit,gray_limit));
    /*
    **	Don't know what to do in this case yet...
    */
    case DirectColor:
	return (IdsMatchData)cm;
    }

#ifdef TRACE
printf( "Leaving Routine IdsAllocColors in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  _IdsAllocPseudoColor
**
**  FUNCTIONAL DESCRIPTION:
**
**      Allocate XColors using IDS color matching algorithm for PseudoColor 
**	visuals.
**
**  FORMAL PARAMETERS:
**
**	screen	     - X11 screen structure pointer
**	cm	     - color match data structure pointer
**	match_space  - color space to use for color matching:
**			    (HLS, Lab, L*U*V*, RGB, U*V*W*, YIQ)
**	match_limit  - describes the distance beyond which new colors will be
**		       allocated: 0.0 = all new colors, 1.0 = match all colors,
**		       other values will yield a mixed result.
**	gray_limit   - describes the amount a sharable colormap color may 
**		       deviate from pure gray: 0.0 = accept only pure grays,
**			    1.0 = accept all colors (Use 1.0 for color images)
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      Contents of XColor cells modified to indentify pixel index allocated,
**	actual RGB values allocated, and status (exact, match, or failed).
**
**  FUNCTION VALUE:
**
**      IdsMatchData - IDS color match statistical data.
**
**	    or NULL if no colors allocated (eg. colormap is AllocAll==private)
**
**		   Deallocate IdsMatchData using XtFree.
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
static IdsMatchData _IdsAllocPseudoColor(screen, cm, match_space, match_limit, 
				     gray_limit)
 Screen		*screen;
 ColorMatch	 cm;
 unsigned long	 match_space;
 double		 match_limit;
 double		 gray_limit;
{
    unsigned long index;
    unsigned long offset;
    unsigned long gray = gray_limit * 65535.0 + 0.5;
    XColor   *colors   = cm->colors;

#ifdef TRACE
printf( "Entering Routine _IdsAllocPseudoColor in module IDS_ALLOC_COLORS \n");
#endif

    /*
    **  Locate the current set of sharable colors in the colormap.
    */
    cm->maplst	= IdsGetColormapState( screen, cm->cmap );	
    cm->request = (IdsColor) XtMalloc( cm->src_num * sizeof(IdsColorStruct));
    cm->palette = (IdsColor) XtMalloc( cm->cells   * sizeof(IdsColorStruct));
    cm->deltas  = &cm->delta0;

    /*
    **  Compute offset for shifting out unmeaningful XColor RGB bits.
    */
    offset = 16 - DefaultVisualOfScreen(screen)->bits_per_rgb;

    /*
    **	Set the color space conversion routine, its range, and 'match_limit'.
    */
    SetColorSpace( cm, match_space, match_limit );
    /*
    **	Initialize the colormap and palette lists.
    */
    for( index = 0; index < cm->cells; ++index )
	if( cm->maplst[index].pad == Ids_AllocShared )
	    if( abs(cm->maplst[index].red  - cm->maplst[index].green) > gray
	     || abs(cm->maplst[index].red  - cm->maplst[index].blue ) > gray
	     || abs(cm->maplst[index].blue - cm->maplst[index].green) > gray )
		/*
		**  This sharable color deviates too far from pure gray.
		*/
		cm->maplst[index].pad = Ids_AllocPrivate;
	    else
		/*
		**  Create an color match space equivalent for this color.
		*/
		(*cm->convert)( cm->maplst+index, cm->palette+index, offset );

    /*
    **	Initialize each requested XColor, create the color match space 
    **	equivalent for each color, and find an initial match from the palette.
    */
    for( index = 0; index < cm->src_num; ++index )
	{
	colors[index].pad = Ids_AllocShared;
	(*cm->convert)( cm->colors+index, cm->request+index, offset );
	SearchPalette( cm, index );
	}

    if( cm->limit < cm->range )
	/*
	**  Allocate a new XColor for each color which has no match within the
	**  application defined 'match_limit'.  Colors are allocated in groups
	**  as determined by a progressively shrinking limit. This tends to
	**  distribute allocations throughout the spectrum of requested colors,
	**  creating better match candidates if we exhaust the colormap.
	*/
	AllocDistantColors( cm );

    if( cm->dst_num < cm->src_num )
	{
	/*
	**  No more new colors need to be allocated (or the colormap is full).
	**  Assign all the remaining colors to their closest sharable match.
	*/
	ShareClosestColors( cm );
	}

    FreeMem_( &cm->maplst  );		    /* deallocate internal arrays   */
    FreeMem_( &cm->palette );
    FreeMem_( &cm->request );

    if( cm->dst_num == 0 )
	{
	/*
	**  No pixels were allocated, mark all the XColors "Ids_AllocFailed".
	**  Free our ColorMatchStruct -- there's nothing useful to return.
	*/
	for (index=0; index<cm->src_num; colors[index++].pad=Ids_AllocFailed);
	FreeMem_( &cm );
	}

#ifdef TRACE
printf( "Leaving Routine _IdsAllocPseudoColor in module IDS_ALLOC_COLORS \n");
#endif
    return( (IdsMatchData) cm );
}

/*******************************************************************************
**  _IdsMatchStaticColor
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**	screen	     - X11 screen structure pointer
**	cm	     - color match data structure pointer
**	match_space  - color space to use for color matching:
**			    (HLS, Lab, L*U*V*, RGB, U*V*W*, YIQ)
**	match_limit  - describes the distance beyond which new colors will be
**		       allocated: 0.0 = all new colors, 1.0 = match all colors,
**		       other values will yield a mixed result.
**	gray_limit   - describes the amount a sharable colormap color may 
**		       deviate from pure gray: 0.0 = accept only pure grays,
**			    1.0 = accept all colors (Use 1.0 for color images)
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      Contents of XColor cells modified to indentify pixel index matched,
**	actual RGB values allocated, and status (exact, match, or failed).
**
**  FUNCTION VALUE:
**
**      IdsMatchData - IDS color match statistical data.
**
**	    or NULL if no colors allocated (eg. colormap is AllocAll==private)
**
**		   Deallocate IdsMatchData using XtFree.
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
static IdsMatchData _IdsMatchStaticColor(screen, cm, match_space, match_limit, 
				     gray_limit)
 Screen		*screen;
 ColorMatch	 cm;
 unsigned long	 match_space;
 double		 match_limit;
 double		 gray_limit;
{
    unsigned long index;
    unsigned long offset;
    unsigned long gray = gray_limit * 65535.0 + 0.5;
    XColor   *colors   = cm->colors;
    unsigned long i, cells = CellsOfScreen( screen );
    Display *dpy = DisplayOfScreen( screen );

#ifdef TRACE
printf( "Entering Routine _IdsMatchStaticColor in module IDS_ALLOC_COLORS \n");
#endif

    /*
    **  Allocate the colormap state list and also a temporary pixel list.
    */
    cm->maplst = (XColor *)        XtMalloc( cells * sizeof(XColor));

    /*
    **  Read the entire colormap.
    */
    for( i = 0; i < cells; cm->maplst[i].pixel = i++ );
    XQueryColors( dpy, cm->cmap, cm->maplst, cells );
     
    /*
    **  Initialize each cell's state as "Shared", 
    */
    for(i = 0; i < cells; cm->maplst[i++].pad    = Ids_AllocShared);
    
    /*
    **  Locate the current set of sharable colors in the colormap.
    */
    cm->request = (IdsColor) XtMalloc( cm->src_num * sizeof(IdsColorStruct));
    cm->palette = (IdsColor) XtMalloc( cm->cells   * sizeof(IdsColorStruct));
    cm->deltas  = &cm->delta0;

    /*
    **  Compute offset for shifting out unmeaningful XColor RGB bits.
    */
    offset = 16 - DefaultVisualOfScreen(screen)->bits_per_rgb;

    /*
    **	Set the color space conversion routine, its range, and 'match_limit'.
    */
    SetColorSpace( cm, match_space, match_limit );
    /*
    **	Initialize the colormap and palette lists.
    */
    for( index = 0; index < cm->cells; ++index )
	if( cm->maplst[index].pad == Ids_AllocShared )
	    if( abs(cm->maplst[index].red  - cm->maplst[index].green) > gray
	     || abs(cm->maplst[index].red  - cm->maplst[index].blue ) > gray
	     || abs(cm->maplst[index].blue - cm->maplst[index].green) > gray )
		/*
		**  This sharable color deviates too far from pure gray.
		*/
		cm->maplst[index].pad = Ids_AllocPrivate;
	    else
		/*
		**  Create an color match space equivalent for this color.
		*/
		(*cm->convert)( cm->maplst+index, cm->palette+index, offset );

    /*
    **	Initialize each requested XColor, create the color match space 
    **	equivalent for each color, and find an initial match from the palette.
    */
    for( index = 0; index < cm->src_num; ++index )
	{
	colors[index].pad = Ids_AllocShared;
	(*cm->convert)( cm->colors+index, cm->request+index, offset );
	SearchPalette( cm, index );
	}

    FreeMem_( &cm->maplst  );		    /* deallocate internal arrays   */
    FreeMem_( &cm->palette );
    FreeMem_( &cm->request );

#ifdef TRACE
printf( "Leaving Routine _IdsMatchStaticColor in module IDS_ALLOC_COLORS \n");
#endif
    return( (IdsMatchData) cm );
}

/*******************************************************************************
**  _IdsAllocTrueColor
**
**  FUNCTIONAL DESCRIPTION:
**
**      Allocate XColors using IDS color matching algorithm for TrueColor 
**	visuals
**
**  FORMAL PARAMETERS:
**
**	screen	     - X11 screen structure pointer
**	cm	     - color match data structure pointer
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      Contents of XColor cells modified to indentify pixel index allocated,
**	actual RGB values allocated, and status (exact, match, or failed).
**
**  FUNCTION VALUE:
**
**      IdsMatchData - IDS color match statistical data.
**
**	    or NULL if no colors allocated (eg. colormap is AllocAll==private)
**
**	Deallocate IdsMatchData using XtFree.
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
static IdsMatchData	_IdsAllocTrueColor(screen, cm)
 Screen		*screen;
 ColorMatch	 cm;
{
    unsigned long index;
    unsigned long offset;
    unsigned long red_length, green_length, blue_length;
    unsigned long red_offset, green_offset, blue_offset;
    unsigned long red_mask,   green_mask,   blue_mask;
    unsigned long mask;
    unsigned long end_bit;

    XColor *colors = cm->colors;
    Visual *visual = DefaultVisualOfScreen(screen);

#ifdef TRACE
printf( "Entering Routine _IdsAllocTrueColor in module IDS_ALLOC_COLORS \n");
#endif

    /*
    **  Calculate number of size and offset of RGB masks within the pixel.
    */
    for (red_offset = 0, mask = 1; 
	    (red_offset < 31) && ((visual->red_mask & mask) == 0);  
		red_offset++, mask *= 2);
    for (end_bit = red_offset + 1, mask *= 2; 
	    (end_bit < 32) && ((visual->red_mask & mask) != 0); 
		end_bit++, mask *= 2);
    red_length = end_bit - red_offset;
    red_mask = (1 << red_length) - 1;

    for (green_offset = 0, mask = 1; 
	    (green_offset < 31) && ((visual->green_mask & mask) == 0);  
		green_offset++, mask *= 2);
    for (end_bit = green_offset + 1, mask *= 2; 
	    (end_bit < 32) && ((visual->green_mask & mask) != 0); 
		end_bit++, mask *= 2);
    green_length = end_bit - green_offset;
    green_mask = (1 << green_length) - 1;

    for (blue_offset = 0, mask = 1; 
	    (blue_offset < 31) && ((visual->blue_mask & mask) == 0);  
		blue_offset++, mask *= 2);
    for (end_bit = blue_offset + 1, mask *= 2; 
	    (end_bit < 32) && ((visual->blue_mask & mask) != 0); 
		end_bit++, mask *= 2);
    blue_length = end_bit - blue_offset;
    blue_mask = (1 << blue_length) - 1;
    /*
    **	Initialize each requested XColor
    */
    for (index = 0; index < cm->src_num; ++index)
	{
	    int value;
	    colors[index].pad = Ids_AllocShared;
	    colors[index].pixel = 0;
	    /*
	    **	For each RGB value, calculate the closest match given the
	    **	precision available in this TrueColor visual. Create pixel
	    **	value as we go as well.
	    */
	    value = (float)colors[index].red / 65535.0 * (float)red_mask + 0.5;
	    colors[index].red =  value << (16 - red_length);
	    colors[index].pixel |= value << red_offset;

	    value = (float)colors[index].green/65535.0*(float)green_mask + 0.5;
	    colors[index].green =  value << (16 - green_length);
	    colors[index].pixel |= value << green_offset;

	    value = (float)colors[index].blue / 65535.0 * (float)blue_mask + 0.5;
	    colors[index].blue =  value << (16 - blue_length);
	    colors[index].pixel |= value << blue_offset;
	}

#ifdef TRACE
printf( "Leaving Routine _IdsAllocTrueColor in module IDS_ALLOC_COLORS \n");
#endif
    return( (IdsMatchData) cm );
}

/*******************************************************************************
**  IdsMatchColors
**
**  FUNCTIONAL DESCRIPTION:
**
**      Match XColors using IDS color matching algorithm.
**
**  FORMAL PARAMETERS:
**
**	screen	     - X11 screen structure pointer
**	colormap     - appl specified colormap id with allocated color cells
**	img_colors   - img list of XColor structures(colors requested)
**	img_cnt      - number of colors img requested in the colormap
**	appl_colors  - appl list of XColor structures(private colors )
**	appl_cnt     - number of colors appl requested in the colormap 
**	match_space  - color space to use for color matching:
**			    (HLS, Lab, L*U*V*, RGB, U*V*W*, YIQ)
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      Contents of XColor cells modified to indentify pixel index allocated,
**	actual RGB values allocated, and status (exact, match, or failed).
**
**  FUNCTION VALUE:
**
**      IdsMatchData - IDS color match statistical data.
**
**	    or NULL if no colors allocated (eg. colormap is AllocAll==private)
**
**		   Deallocate IdsMatchData using XtFree.
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
IdsMatchData IdsMatchColors( screen, colormap, img_colors, img_cnt, 
                             appl_colors, appl_cnt, match_space )
 Screen		*screen;
 Colormap	 colormap;
 XColor		*img_colors;
 unsigned long	 img_cnt;
 XColor		*appl_colors;
 unsigned long	 appl_cnt, match_space;

{
    ColorMatch	  cm;
    double	  match_limit, gray_limit;
    unsigned long index, offset;

#ifdef TRACE
printf( "Entering Routine IdsMatchColors in module IDS_ALLOC_COLORS \n");
#endif
    match_limit = 0.0;
    gray_limit  = 0.0; 
    /*
    **	Create the color match environment.
    */
    cm		= (ColorMatch) XtCalloc(1, sizeof(ColorMatchStruct)
					    + img_cnt * sizeof(float));
    cm->dpy     = DisplayOfScreen( screen );
    cm->cmap    = colormap;
    cm->cells	= appl_cnt;                       /* is this should be count */
    cm->src_num = img_cnt;
    cm->colors  = img_colors;
        /*
        **  Locate the current set of sharable colors in the colormap.
        */
    cm->maplst  = appl_colors;
    cm->request = (IdsColor) XtMalloc(   img_cnt * sizeof(IdsColorStruct));
    cm->palette = (IdsColor) XtMalloc( cm->cells * sizeof(IdsColorStruct));
    cm->deltas  = &cm->delta0;

    /*
    **  Compute offset for shifting out unmeaningful XColor RGB bits.
    */
    offset = 16 - DefaultVisualOfScreen(screen)->bits_per_rgb;

    /*
    **	Set the color space conversion routine, its range, and 'match_limit'.
    */
    SetColorSpace( cm, match_space, match_limit );

    /*
    **  Initialize the palette lists.
    */
    for( index = 0; index < cm->cells; ++index )
	{
	    cm->maplst[index].pad = Ids_AllocShared;    
	    (*cm->convert)( cm->maplst+index, cm->palette+index, offset );
	}

    /*
    **	Initialize each requested XColor, create the color match space 
    **	equivalent for each color, and find an initial match from the palette.
    */
    for( index = 0; index < img_cnt; ++index )
	{
	img_colors[index].pad = Ids_AllocShared;
	(*cm->convert)( cm->colors+index, cm->request+index, offset );
	SearchPalette( cm, index );
	}
     
    FreeMem_( &cm->maplst  );		    /* deallocate internal arrays   */
    FreeMem_( &cm->palette );
    FreeMem_( &cm->request );

    if( cm->dst_num == 0 )
	{
	/*
	**  No pixels were allocated, mark all the XColors "Ids_AllocFailed".
	**  Free our ColorMatchStruct -- there's nothing useful to return.
	*/
	for( index = 0; index < img_cnt; img_colors[index++].pad = Ids_AllocFailed );
	FreeMem_( &cm );
	}

#ifdef TRACE
printf( "Leaving Routine IdsMatchColors in module IDS_ALLOC_COLORS \n");
#endif
    return( (IdsMatchData) cm );
}

/*******************************************************************************
**  AllocDistantColors
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate all colors that have no match within the application's limit.
**
**  FORMAL PARAMETERS:
**
**	cm	- ColorMatch structure
**
*******************************************************************************/
static void AllocDistantColors( cm )
 ColorMatch   cm;
{
    float limit, count;
    unsigned long i;

#ifdef TRACE
printf( "Entering Routine AllocDistantColors in module IDS_ALLOC_COLORS \n");
#endif
    /*
    **	Allocate a color to create valid distances -- in case of empty colormap.
    */
    AllocNewColor( cm, 0 );

    while( cm->dst_num < cm->src_num )
	{
	/*
	**  Calculate limit: the mean match distance of all the pending colors.
	*/
	for( count = 0.0, limit = 0.0, i = 0; i < cm->src_num; ++i )
	    if( cm->deltas[i] != 0 )
		{
		count++;	    
		limit += cm->deltas[i];
		}
	limit = (limit < cm->limit*count) ? cm->limit : limit/(count+1e-6);

	/*
	**  Allocate all colors beyond the calculated mean match distance.
	**  Note: as allocations are made, pending colors may find a closer 
	**  match, so we'll probably allocate much less than half the colors.
	*/
	for( i = 0; i < cm->src_num; ++i )
	    if( cm->colors[i].pad == Ids_AllocShared && cm->deltas[i] >= limit )

		if( !AllocNewColor( cm, i ))  /* allocate a 'far out' color */
		    break;		      /* Oops! the colormap is full */

	if( limit == cm->limit || i < cm->src_num )
	    /*
	    **  No more new colors are needed, or the colormap is full.
	    */
	    break;
	}
#ifdef TRACE
printf( "Leaving Routine AllocDistantColors in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  AllocNewColor
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a new color.
**
**  FORMAL PARAMETERS:
**
**	cm	- ColorMatch structure
**	index	- color list index of color to allocate
**
**  FUNCTION VALUE:
**
**	Boolean	- TRUE if color allocation succeeds.
**
*******************************************************************************/
static Boolean AllocNewColor( cm, index )
 ColorMatch    cm;
 unsigned long index;
{
    Boolean allocated;

#ifdef TRACE
printf( "Entering Routine AllocNewColor in module IDS_ALLOC_COLORS \n");
#endif

    /*
    **	Allocate the requested color.
    */
    allocated = XAllocColor( cm->dpy, cm->cmap, cm->colors+index );

    if( allocated )
	{
	/*
	**  Copy the RGB and color match space values to the colormap list.
	*/
	cm->palette[cm->colors[index].pixel] = cm->request[index];
	cm->maplst [cm->colors[index].pixel] = cm->colors [index];

	/*
	**  Flag this color as an exact color -- and distance == 0.
	*/
	cm->colors[index].pad = Ids_AllocExact;
	cm->deltas[index]     = 0;
	cm->dst_num++;

	/*
	**  Update the distance to pending colors -- in case 
	**  our new color is a better match for any of them.
	*/
	UpdateColors( cm, cm->colors[index].pixel );
	}

#ifdef TRACE
printf( "Leaving Routine AllocNewColor in module IDS_ALLOC_COLORS \n");
#endif
    return( allocated );
}

/*******************************************************************************
**  ShareClosestColors
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate the closest existing match for each unallocated color.
**
**  FORMAL PARAMETERS:
**
**	cm	- ColorMatch structure
**
*******************************************************************************/
static void ShareClosestColors( cm )
 ColorMatch   cm;
{
    unsigned long index;
    XColor color;

#ifdef TRACE
printf( "Entering Routine ShareClosestColors in module IDS_ALLOC_COLORS \n");
#endif

    for( index = 0; index < cm->src_num; ++index )
	while( cm->colors[index].pad == Ids_AllocShared )
	    {
	    color = cm->maplst[cm->colors[index].pixel];

	    if( XAllocColor( cm->dpy, cm->cmap, &color ))
		{
		cm->maplst[color.pixel] = color;
		cm->colors[index]       = color;
		cm->colors[index].pad   = Ids_AllocMatch;
		cm->dst_num++;
		break;
		}
	    else
		{
		/*
		**  No longer available -- and the colormap is full.
		**  Update the colormap state, then find the next best choice.
		*/
		cm->maplst[color.pixel].pad = Ids_AllocPrivate;
		SearchPalette( cm, index );
		}
	    }
#ifdef TRACE
printf( "Leaving Routine ShareClosestColors in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  SearchPalette
**
**  FUNCTIONAL DESCRIPTION:
**
**	Find the closest 'match' from the palette of available colors.
**
**  FORMAL PARAMETERS:
**
**	cm	- ColorMatch structure
**	match	- index of color to find a match for
**
*******************************************************************************/
static void SearchPalette( cm, match )
 ColorMatch	cm;
 unsigned long	match;
{
    unsigned long index;
    float distance, t1, match_t1 = cm->request[match].t1,
		    t2, match_t2 = cm->request[match].t2,
		    t3, match_t3 = cm->request[match].t3;


#ifdef TRACE
printf( "Entering Routine SearchPalette in module IDS_ALLOC_COLORS \n");
#endif

    for( cm->deltas[match] = 1e10, index = 0;  index < cm->cells;  index++ )
	{
	if( cm->maplst[index].pad != Ids_AllocShared )
	    continue;

	t1 = match_t1 - cm->palette[index].t1;
	t2 = match_t2 - cm->palette[index].t2;
	t3 = match_t3 - cm->palette[index].t3;

	if( cm->space == Ids_HLSSpace )
	    {
	    /*
	    **  Hue is circular, so complement the angular distance if the two
	    **  values are more than half way around the circle from each other.
	    */
	    if( t1  < (float)-F_one )
		t1 += F_two;
	    else   if((float) F_one < t1 )
		t1 -= F_two;
	    }

	distance = t1*t1 + t2*t2 + t3*t3;

	if( cm->deltas[match] > distance )
	    {
	    cm->deltas[match] = distance;
	    cm->colors[match].pixel = cm->maplst[index].pixel; 
	    if( distance == 0 )
		break;
	    }
	}
#ifdef TRACE
printf( "Leaving Routine SearchPalette in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  UpdateColors
**
**  FUNCTIONAL DESCRIPTION:
**
**	For each pending color allocation: update its distance and pixel index 
**	if the palette entry specified by 'match' is a closer match than any
**	previously found.
**
**  FORMAL PARAMETERS:
**
**	cm	- ColorMatch structure
**	match	- index of colormap color to compare against
**
*******************************************************************************/
static void UpdateColors( cm, match )
 ColorMatch	cm;
 unsigned long  match;
{
    unsigned long index;
    float distance, t1, match_t1 = cm->palette[match].t1,
		    t2, match_t2 = cm->palette[match].t2,
		    t3, match_t3 = cm->palette[match].t3;

#ifdef TRACE
printf( "Entering Routine UpdateColors in module IDS_ALLOC_COLORS \n");
#endif

    for( index = 0;  index < cm->src_num;  index++ )
	{
	if( cm->colors[index].pad != Ids_AllocShared )
	    continue;

	t1 = match_t1 - cm->request[index].t1;
	t2 = match_t2 - cm->request[index].t2;
	t3 = match_t3 - cm->request[index].t3;

	if( cm->space == Ids_HLSSpace )
	    {
	    /*
	    **  Complement the angular distance if the two values are more than 
	    **  half way around the circle from each other -- hue is circular.
	    */
	    if( t1  < (float)-F_one )
		t1 += F_two;
	    else   if((float) F_one < t1 )
		t1 -= F_two;
	    }

	distance = t1*t1 + t2*t2 + t3*t3;

	if( distance < cm->deltas[index] )
	    {
	    cm->deltas[index] = distance;
	    cm->colors[index].pixel = cm->maplst[match].pixel;
	    }
	}

#ifdef TRACE
printf( "Leaving Routine UpdateColors in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  SetColorSpace
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set the color space conversion routine, set the distance range (the 
**	squares of the maximum distance between any pair of colors; these were
**	found using IdsColorSpaceMinMax), and the limit for sharing colors.
**
**  FORMAL PARAMETERS:
**
**	cm	- ColorMatch structure
**	space	- color space to use
**	limit	- match limit to be converted
**
*******************************************************************************/
static void SetColorSpace( cm, space, limit )
 ColorMatch	cm;
 unsigned long	space;
 double		limit;
{
#ifdef TRACE
printf( "Entering Routine SetColorSpace in module IDS_ALLOC_COLORS \n");
#endif

    switch( space )		    /* Color space maximum distance from:   */
	{			    /* [ Red, Grn, Blu] to [ RED, Grn, Blu] */
    case Ids_HLSSpace :
	cm->convert = RGBtoHLS;
	cm->range   = 2.250000;	    /* [0000,0000,0000] to [0000,FFFF,0000] */
	break;
    case Ids_LabSpace :
	cm->convert = RGBtoLAB;
	cm->range   = 73996.36;     /* [0000,FFFF,0000] to [FFFF,0000,FFFF] */
	break;
    case Ids_LUVSpace :
	cm->convert = RGBtoLUV;
	cm->range   = 129414.704;   /* [FFFF,0000,0000] to [0000,FFFF,0000] */
	break;
    case Ids_RGBSpace :
	cm->convert = RGBtoRGB;
	cm->range   = 3.000;	    /* [0000,0000,0000] to [FFFF,FFFF,FFFF] */
	break;
    case Ids_UVWSpace :
	cm->convert = RGBtoUVW;
	cm->range   = 123371.053;  /* [FFFF,0000,0000] to [0000,FFFF,0000] */
	break;
    case Ids_YIQSpace :
    default :
	cm->convert = RGBtoYIQ;
	cm->range   = 1.762245;	    /* [FFFF,0000,0000] to [0000,FFFF,FFFF] */
	}
    cm->space = space;
    cm->limit = cm->range * limit;

#ifdef TRACE
printf( "Leaving Routine SetColorSpace in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  RGBtoHLS
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert XColor tristimulus RGB values to the HLS color space.
**
**  FORMAL PARAMETERS:
**
**	rgb	- pointer to XColor structure
**	new	- pointer to IdsColorStruct for converted result
**	offset	- offset to significant RGB bits within the XColor struct
**
*******************************************************************************/
static void RGBtoHLS( rgb, new, offset )
 XColor	         *rgb;
 IdsColor         new;
 unsigned long offset;
{
    float min, max, sum, dif, red, grn, blu, div;

#ifdef TRACE
printf( "Entering Routine RGBtoHLS in module IDS_ALLOC_COLORS \n");
#endif

    div = (float)((1<<(16-offset))-1);

    red = RED_(rgb,offset,div);	    /* normalize the RGB components */
    grn = GRN_(rgb,offset,div);
    blu = BLU_(rgb,offset,div);
    min = Min3_(red, grn, blu);		    /* find the maximum component   */
    max = Max3_(red, grn, blu);		    /* find the minimum component   */
    sum = max + min;
    dif = max - min;

    new->t2 = sum / F_two;	    /* Luminance: black = 0.0, white = 1.0. */

    if( dif != F_zero )
	{
	/*
	**  Normalized hue angles to the range: 0.0 to 2.0.
	**	( 0/6=mag, 1/6=red, 2/6=yel, 3/6=grn, 4/6=cyn, 5/6=blu )
	*/
	new->t1 = ((max == red) ? ((grn - blu)/dif + F_one  )
		 : (max == grn) ? ((blu - red)/dif + F_three)
			        : ((red - grn)/dif + F_five )) / F_three;
	/*
	**  Saturation is the ratio between the dominant "color content"
	**  and the "color plus white": gray = 0.0, undiluted color = 1.0.
	*/
	new->t3 = dif / (new->t2 < F_half ? F_two - sum : sum);
	}
    else
	{
	new->t1 = dif;  /* hue and saturation are zero for gray shades  */
	new->t3 = dif;
	}
#ifdef TRACE
printf( "Leaving Routine RGBtoHLS in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  RGBtoLAB
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert XColor tristimulus RGB values to the Lab color space.
**
**  FORMAL PARAMETERS:
**
**	rgb	- pointer to XColor structure
**	new	- pointer to IdsColorStruct for converted result
**	offset	- offset to significant RGB bits within the XColor struct
**
*******************************************************************************/
static void RGBtoLAB( rgb, new, offset )
 XColor	         *rgb;
 IdsColor         new;
 unsigned long offset;
{
    float div;
    double  y;

#ifdef TRACE
printf( "Entering Routine RGBtoLAB in module IDS_ALLOC_COLORS \n");
#endif

    div = (float)((1<<(16-offset))-1);
    y = Y_(rgb,offset,div);

    new->t1 =  25.0* pow(100.0 * y /LabrefY, 1.0/3.0) - 16.0;
    new->t2 = 500.0*(pow((double)X_(rgb,offset,div)/LabrefX, 1.0/3.0)
		   - pow(y / LabrefY, 1.0/3.0));
    new->t3 = 200.0*(pow(y / LabrefY, 1.0/3.0)
		   - pow((double)Z_(rgb,offset,div)/LabrefZ, 1.0/3.0));

#ifdef TRACE
printf( "Leaving Routine RGBtoLAB in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  RGBtoLUV
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert XColor tristimulus RGB values to the L*U*V* color space.
**
**  FORMAL PARAMETERS:
**
**	rgb	- pointer to XColor structure
**	new	- pointer to IdsColorStruct for converted result
**	offset	- offset to significant RGB bits within the XColor struct
**
*******************************************************************************/
static void RGBtoLUV( rgb, new, offset )
 XColor	         *rgb;
 IdsColor         new;
 unsigned long offset;
{
    float sum, x, y, z, div;

#ifdef TRACE
printf( "Entering Routine RGBtoLUV in module IDS_ALLOC_COLORS \n");
#endif

    div = (float)((1<<(16-offset))-1);
    x   = X_(rgb,offset,div);
    y   = Y_(rgb,offset,div);
    z   = Z_(rgb,offset,div);
    sum = x + 15.0*y + 3.0*z;

    new->t1 = 25.0*pow((double)100.0 * y / LabrefY, 1.0/3.0) - 16.0;
    new->t2 = sum == 0.0 ? 0.0 : 13.0 * new->t1 * (4.0 * x / sum - LUVrefU);
    new->t3 = sum == 0.0 ? 0.0 : 13.0 * new->t1 * (9.0 * y / sum - LUVrefV);

#ifdef TRACE
printf( "Leaving Routine RGBtoLUV in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  RGBtoRGB
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert XColor tristimulus RGB values to the RGB color space.
**
**  FORMAL PARAMETERS:
**
**	rgb	- pointer to XColor structure
**	new	- pointer to IdsColorStruct for converted result
**	offset	- offset to significant RGB bits within the XColor struct
**
*******************************************************************************/
static void RGBtoRGB( rgb, new, offset )
 XColor	         *rgb;
 IdsColor         new;
 unsigned long offset;
{
    float div;

#ifdef TRACE
printf( "Entering Routine RGBtoRGB in module IDS_ALLOC_COLORS \n");
#endif

    div = (float)((1<<(16-offset))-1);
    new->t1 = RED_(rgb,offset,div);
    new->t2 = GRN_(rgb,offset,div);
    new->t3 = BLU_(rgb,offset,div);

#ifdef TRACE
printf( "Leaving Routine RGBtoRGB in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  RGBtoUVW
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert XColor tristimulus RGB values to the U*V*W* color space.
**
**  FORMAL PARAMETERS:
**
**	rgb	- pointer to XColor structure
**	new	- pointer to IdsColorStruct for converted result
**	offset	- offset to significant RGB bits within the XColor struct
**
*******************************************************************************/
static void RGBtoUVW( rgb, new, offset )
 XColor	         *rgb;
 IdsColor         new;
 unsigned long offset;
{
    float sum, u, v, w, div;

#ifdef TRACE
printf( "Entering Routine RGBtoUVW in module IDS_ALLOC_COLORS \n");
#endif

    div = (float)((1<<(16-offset))-1);
    u   = U_(rgb,offset,div);
    v   = Y_(rgb,offset,div);
    w   = W_(rgb,offset,div);
    sum = u + v + w;

    new->t3 = 25.0 * pow((double)100.0 * v / LabrefY, 1.0/3.0) - 17.0;
    new->t1 = sum == 0.0 ? 0.0 : 13.0 * new->t3 * (u / sum - UVWrefU);
    new->t2 = sum == 0.0 ? 0.0 : 13.0 * new->t3 * (v / sum - UVWrefV);

#ifdef TRACE
printf( "Leaving Routine RGBtoUVW in module IDS_ALLOC_COLORS \n");
#endif
}

/*******************************************************************************
**  RGBtoYIQ
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert XColor tristimulus RGB values to the YIQ color space.
**
**  FORMAL PARAMETERS:
**
**	rgb	- pointer to XColor structure
**	new	- pointer to IdsColorStruct for converted result
**	offset	- offset to significant RGB bits within the XColor struct
**
*******************************************************************************/
static void RGBtoYIQ( rgb, new, offset )
 XColor	         *rgb;
 IdsColor         new;
 unsigned long offset;
{
    float div;

#ifdef TRACE
printf( "Entering Routine RGBtoYIQ in module IDS_ALLOC_COLORS \n");
#endif

    div = (float)((1<<(16-offset))-1);
    new->t1 = Y_(rgb,offset,div);
    new->t2 = I_(rgb,offset,div);
    new->t3 = Q_(rgb,offset,div);

#ifdef TRACE
printf( "Leaving Routine RGBtoYIQ in module IDS_ALLOC_COLORS \n");
#endif
}
