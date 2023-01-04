
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

#ifndef IDS_ALLOC_COLORS_H
#define IDS_ALLOC_COLORS_H

#ifndef IDS_NOX

#ifdef VMS
#include <decw$include/Xlib.h>
#else
#include <X11/Xlib.h>
#endif

#endif

/*
**  Equated Symbols
*/
    /*
    **  State of colormap cells returned by IdsGetColorMapState.
    */
#define Ids_AllocShared	    0	/* read-only sharable color cell	      */
#define Ids_AllocPrivate    1	/* read/write private color cell	      */
#define Ids_AllocFree	    2	/* available colormap cell		      */
    /*
    **  State of XColors allocated by IdsAllocColors.
    */
#define Ids_AllocExact	    3	/* color allocated was exact 		      */
#define Ids_AllocMatch	    4	/* color allocated was the closest match      */
#define Ids_AllocFailed	    5	/* no color allocated: no cells available     */
    /*
    **  IdsAllocColors color space definitions.
    */
#define Ids_HLSSpace	    1	/* match colors using HLS the color space     */
#define Ids_LabSpace	    2	/* match colors using the Lab color space     */
#define Ids_LUVSpace	    3	/* match colors using the L*U*V* color space  */
#define Ids_RGBSpace	    4	/* match colors using RGB the color space     */
#define Ids_UVWSpace	    5	/* match colors using the U*V*W* color space  */
#define Ids_YIQSpace	    6	/* match colors using the YIQ color space     */

    /*
    **  IDS color match data -- returned by IdsAllocColors.
    **	( free using XtFree )	  (input to IdsGetAllocStatistics)
    */
typedef char *IdsMatchData;

    /*
    **  IDS color match statistics -- returned by IdsGetAllocStatistics.
    **	( free using XtFree )
    */
typedef struct _AllocStatisticsStruct
    {
    unsigned long total;     /* total number of XColors requested	      */
    unsigned long matches;   /* # XColors with match distance > threshold     */
    unsigned long sdev_1;    /* # XColors:      mean < distance < 1 std_dev   */
    unsigned long sdev_2;    /* # XColors: 1 std_dev < distance < 2 std_dev   */
    unsigned long sdev_3;    /* # XColors: 2 std_dev < distance		      */
    double	  mean;	     /* mean distance from desired colors	      */
    double	  std_dev;   /* standard deviation of distances		      */
} IdsAllocStatisticsStruct, *IdsAllocStatistics;

    /*
    **  IDS color space components -- returned by IdsColorSpaceMinMax.
    **	( free using XtFree )
    */
typedef struct _IdsColorStruct
    {
    float   t1;                 /* t1 value (H, L, L*, R, U*, Y)              */
    float   t2;                 /* t2 value (L, a, U*, G, V*, I)	      */
    float   t3;                 /* t3 value (S, b, V*, B, W*, Q)	      */
} IdsColorStruct, *IdsColor;

    /*
    **  IDS Widget Entry points 
    */
#ifdef VMS
extern IdsAllocStatistics IDS$GET_COLOR_STATISTICS();/* VMS: IDS$RENDER_IMAGE */
#endif
extern IdsAllocStatistics IdsGetColorStatistics();   /* C:   IDS$RENDER_IMAGE */


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
#ifndef DAS_PROTO
#if DAS_EXPAND_PROTO == 1
#define PROTO(name, arg_list) name arg_list
#else
#define PROTO(name, arg_list) name ()
#endif
#endif

/*******************************************************************************
**  IdsAllocColors
**
**  FUNCTIONAL DESCRIPTION:
**
**      Allocate XColors using IDS color matching algorithm.
**
**  FORMAL PARAMETERS:
*/
#ifndef IDS_NOX
#ifdef NODAS_PROTO
extern IdsMatchData IdsAllocColors( /* screen, colormap, colors, count, 
				       match_space, match_limit, gray_limit */);
#else
PROTO( IdsMatchData IdsAllocColors, (Screen */*screen*/, Colormap /*colormap*/, 
  XColor */*colors*/, unsigned long /*count*/, unsigned long /*match_space*/, 
  double /*match_limit*/, double /*gray_limit*/));
#endif
#endif
/*
**	Screen       *screen;	    - X11 screen structure pointer
**
**	Colormap      colormap;	    - X11 colormap id
**
**	XColor       *colors;	    - List of XColor structures
**
**	unsigned long count;	    - Number of colors in list
**
**	unsigned long match_space;  - Color space to use for color matching:
**				      (HLS, Lab, L*U*V*, RGB, U*V*W*, YIQ)
**
**	double        match_limit;  - Describes the distance beyond which new 
**				      colors will be allocated:
**					 0.0 = allocated all new colors,
**					 1.0 = match all colors (no new colors).
**				      Other values will yield a mixed result.
**
**	double        gray_limit;  - Describes the amount a sharable colormap 
**				     color may deviate from pure gray:
**					 0.0 = accept only pure grays,
**					 1.0 = accept all colors.
**				     Use 1.0 for color images.
**
**  IMPLICIT OUTPUTS:
**
**      the 'pad' field of each XColor structure passed in contains:
**
**		Ids_AllocExact	- the requested color was allocated
**		Ids_AllocMatch	- the requested color was matched
**		Ids_AllocFailed - no colors were allocated
**
**  FUNCTION VALUE:
**
**      IdsMatchData - IDS color match statistical data.
**
**	    or NULL if no colors allocated (eg. colormap is AllocAll == private)
**
**		   Deallocate IdsMatchData using XtFree.
**
*******************************************************************************/

/*******************************************************************************
**  IdsFreeXColors
**
**  FUNCTIONAL DESCRIPTION:
**
**      Deallocate pixels specified in a list of XColor structures.
**
**  FORMAL PARAMETERS:
*/
#ifndef IDS_NOX
#ifdef NODAS_PROTO
extern void IdsFreeXColors( /* screen, colormap, colors, count */ );
#else
PROTO( void IdsFreeXColors, (Screen */*screen*/, Colormap /*colormap*/, 
  XColor */*colors*/, unsigned long /*count*/));
#endif
#endif
/*
**	Screen         *screen;	    - X11 screen structure pointer
**
**	Colormap        colormap;   - X11 colormap id
**
**	XColor         *colors;	    - list of XColor structures
**
**	unsigned long   count;	    - number of colors in list
**
**  IMPLICIT INPUTS:
**
**      Pixel values within XColor structures are the pixels to be freed.
**
*******************************************************************************/

/*******************************************************************************
**  IdsGetAllocStatistics
**
**  FUNCTIONAL DESCRIPTION:
**
**      Get statistics of allocations whose match distance exceeded 'threshold'.
**
**  FORMAL PARAMETERS:
*/
#ifdef NODAS_PROTO
extern IdsAllocStatistics IdsGetAllocStatistics( /* data, threshold */ );
#else
PROTO( IdsAllocStatistics IdsGetAllocStatistics, (IdsMatchData /*data*/, 
  double /*threshold*/));
#endif
/*
**	IdsMatchData   data;	  - IdsMatchData pointer
**
**	double         threshold; - match distance threshold, statistics are
**				    gathered only for matched colors with
**				    match distances exceeding this threshold.
**				    Range = 0.0 (include all)
**					    1.0 (include none)
**
**  FUNCTION VALUE:
**
**      IdsAllocStatistics - statistics of matched colors exceeding 'threshold'.
**
**		   Deallocate IdsAllocStatistics using XtFree.
**
*******************************************************************************/

/*******************************************************************************
**  IdsGetColormapState
**
**  FUNCTIONAL DESCRIPTION:
**
**	Get the contents of the colormap -- determine the state of each cell.
**
**  FORMAL PARAMETERS:
*/
#ifndef IDS_NOX
#ifdef NODAS_PROTO
extern XColor *IdsGetColormapState( /* screen, cmap */ );
#else
PROTO( XColor *IdsGetColormapState, (Screen */*screen*/, Colormap /*cmap*/));
#endif
#endif
/*
**	Screen	*screen; - X11 screen structure pointer
**
**	Colormap cmap;	 - X11 colormap id to allocate color cells from
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
*******************************************************************************/

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
*/
#ifndef IDS_NOX
#ifdef NODAS_PROTO
extern IdsColor IdsColorSpaceMinMax( /* colors, count, space, sig_bits, find,
				        min_dist, min_in_1, min_in_2,
				        max_dist, max_in_1, max_in_2 */ );
#else
PROTO( IdsColor IdsColorSpaceMinMax, (XColor */*colors*/,
  unsigned long /*count*/, unsigned long /*space*/, unsigned long /*sig_bits*/, 
  unsigned long /*find*/, double */*min_dist*/, unsigned long */*min_in_1*/, 
  unsigned long */*min_in_2*/, double */*max_dist*/, 
  unsigned long */*max_in_1*/, unsigned long */*max_in_2*/));
#endif
#endif

/*
**	XColor        *colors;   - list of XColors for making comparisons
**
**	unsigned long  count;	 - number of XColors
**
**	unsigned long  space;	 - color space to calculate distances for
**
**	unsigned long  sig_bits; - number of significant RGB bits in XColor
**				    (ie. (Visual *) visual->bits_per_rgb)
**
**	unsigned long  find;	 - Boolean: TRUE  = find min and max distances
**					    FALSE = only return converted colors
**
**	NOTE: if find is FLASE, the following arguments are ignored.
**
**	double	      *min_dist; - where to store the minimum distance
**
**	unsigned long *min_in_1; - where to store color list index (1st of pair)
**
**	unsigned long *min_in_2; - where to store color list index (2nd of pair)
**
**	double	      *max_dist; - where to store the maximum distance
**
**	unsigned long *max_in_1; - where to store color list index (1st of pair)
**
**	unsigned long *max_in_2; - where to store color list index (2nd of pair)
**
**  FUNCTION VALUE:
**
**      IdsColor - list of colors converted to the requested color space.
**
**		   Deallocate IdsColor list using XtFree.
**
*******************************************************************************/
#endif /* IDS_ALLOC_COLORS_H */
