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
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1987 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	General purpose
**
**  AUTHOR:
**
**	Doug Rayner
**
**  ABSTRACT:
**
**	Position a widget on the screen so as to avoid a list of existing
**	widgets.
**
**  ENVIRONMENT:
**
**	DECwindows, user mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	V1-001  Doug Rayner, 21-Mar-88, initial version.
**	V2-001  André Pavanello, 11-Apr-89, incorporate some edge conditions bug
**	                                    fixes 
**		Leo, 03-Aug-89, Q&D port to Motif
**--
**/

/*
**  Include Files
*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/*
**  Macro Definitions
*/

#define T 1
#define F 0

#ifdef MIN
#undef MIN
#endif
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

#ifdef MAX
#undef MAX
#endif
#define MAX(x,y) (((x) > (y)) ? (x) : (y))


/*
**  Tiling factors
*/

#define MIN_TILES 2
#define MAX_TILES 8

/*
**  Weighting factors
*/

#define MIN_WEIGHT 100
#define MAX_WEIGHT (256 * (MIN_WEIGHT))
#define WEIGHT_FACTOR 2

/*
**  Widget Border fudge factors
*/

#define LEFT_MARGIN 4
#define RIGHT_MARGIN 4
#define BOTTOM_MARGIN 4
#define TOP_MARGIN 25

/*
**  Type Definitions
*/

typedef struct rect {
	Position x1, y1, /* (X,Y) lower left coordinate of rectangle   */
	         x2, y2; /* (X,Y) upper right coordinate of rectangle  */
	Dimension width, /* Width of rectangle			       */
	         height; /* Height of rectangle			       */
	int      area;   /* Area of rectangle			       */
    } rect;

/*
**  Module static storage
*/

static Dimension tile_size;	/* Dimension of screen tiling matrix	    */
static int *tile_array;		/* Array of integer values for each tile    */
static int screen_id;		/* Default screen id			    */
static rect screen;		/* Rectangle describing screen		    */
static rect tile;		/* Rectangle describing screen tile	    */

/*
**  Table of Contents
*/

void DXmPositionWidget();
static void screen_to_rect();
static void widget_to_rect();
static void create_tiling();
static void weight_tiles();
static void weight_widget();
static void unweight_tiles();
static void calc_position();
static void set_position();
static Boolean intersect();


void DXmPositionWidget(new_widget, avoid_widgets, widget_cnt)
    Widget new_widget;
    Widget *avoid_widgets;
    int widget_cnt;
/*
**++
**  Functional Description:
**	Position a new Widget on the screen in such a way as to avoid occluding
**	an ordered list of existing widgets.
**
**	The algorithm is generally as follows:
**
**	    - Tile the screen with a matrix of rectangles.  The size of this
**	    matrix is between MIN_TILES x MIN_TILES and MAX_TILES x MAX_TILES
**	    depending on the relative area of the new widget and the screen.
**
**	    - Give the list of widgets-to-avoid an exponentially decreasing
**	    weight (from MAX_WEIGHT to MIN_WEIGHT by WEIGHT_FACTOR) and then
**	    add up the contributions of these weights on each tile that a
**	    widget covers.
**
**	    - Unweight tiles surrounding the first widget to avoid.  This will
**	    tend to have the new widget placed nearby if there are multiple,
**	    equivalent positions for the new widget.
**
**	    - Take the new widget and place it on the screen at each valid tile
**	    coordinate (never place the new widget off-screen), and see how
**	    heavy the tiles are that get covered.
**
**	    - Place the new widget at the position which minimized the weight
**	    of the occluded tiles.  
**
**  Keywords:
**	Position, Widget
**
**  Arguments:
**	new_widget   : The ID of the Widget to be positioned (must be realized)
**	avoid_wigets : An array of ID's for the Widgets to avoid
**	widget_count : The length of avoid_widgets
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    rect widget_rect;		/* Rectangle describing new widget	    */
    rect avoid_rect;		/* Rectangle describing a widget to avoid   */

    /*
    **  Get the screen coordinates.
    */

    screen_to_rect(new_widget);

    /*
    **  Get the rectangular coordinates of the new widget.
    */

    widget_to_rect(new_widget, &widget_rect);

    /*
    **  If the new Widget is larger than the screen, just position it at (0,0).
    **	If there are no widgets to avoid, center the new widget in the screen.
    */

    if (widget_rect.area >= screen.area) {
	set_position(new_widget, 0, 0);
	return;
    }

    if (widget_cnt == 0 || avoid_widgets == NULL) {
	set_position(new_widget,
		     (screen.width - widget_rect.width) / 2,
		     (screen.height - widget_rect.height) / 2);
	return;
    }

    /*
    **  Set up the screen tiling
    */

    create_tiling(&widget_rect);

    /*
    **	Fill the tiling array with values which reflect the widgets to avoid.
    **	Widgets are weighted according to their position in the list of Widgets
    **	to avoid.  The weight is based on an exponentially decreasing value
    **	between MAX_WEIGHT and MIN_WEIGHT.
    */

    weight_tiles(widget_cnt, avoid_widgets);

    /*
    **	Now, unweight tiles surrounding the Widget at the top of the avoid
    **	list.  This will tend to have the new Widget gravitate to that Widget.
    */

    widget_to_rect(avoid_widgets[0], &avoid_rect);

    unweight_tiles(&avoid_rect);

    /*
    **  Calculate the best location for the new Widget, and then position it.
    */

    calc_position(&widget_rect);
    
    set_position(new_widget, widget_rect.x1, widget_rect.y1);

    /*
    **  Free allocated memory
    */

    free(tile_array);

    return;
    }

static void screen_to_rect(widget)
    Widget widget;
/*
**++
**  Functional Description:
**	Get the coordinates of the screen
**
**  Keywords:
**	None
**
**  Arguments:
**	widget : any Widget on the screen
**	
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    screen_id  = DefaultScreen(XtDisplay(widget));

    screen.width = DisplayWidth(XtDisplay(widget), screen_id);
    screen.height = DisplayHeight(XtDisplay(widget), screen_id);

    screen.area = screen.height * screen.width;

    screen.x1 = 0;
    screen.y1 = 0;
    screen.x2 = screen.width;
    screen.y2 = screen.height;

    return;
    }


static void widget_to_rect(widget, widget_rect)
    Widget widget;
    rect *widget_rect;
/*
**++
**  Functional Description:
**	Determine the screen coordinates of a Widget.
**
**  Keywords:
**	None
**
**  Arguments:
**	widget : Widget to analyze
**	widget_rect : rect structure to fill
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    int x, y;
    Dimension width, height;
    Arg	arglist[2];
    Window window;

    /*
    **	Get the size and the coordinates of the Widget.  If it is not realized
    **	we can't get the coordintates, so create a dummy rectangle.
    */

    if (XtIsRealized(widget)) {
	XtSetArg(arglist [0], XtNwidth,  &width);
	XtSetArg(arglist [1], XtNheight, &height);
	XtGetValues(widget, arglist, 2);

	XTranslateCoordinates (XtDisplay(widget), XtWindow(widget),
			       XRootWindow(XtDisplay(widget), screen_id),
			       0, 0, &x, &y, &window);

	/*
	**  Fudge in a border for the Widget
	*/

	widget_rect->width = width + LEFT_MARGIN + RIGHT_MARGIN;
	widget_rect->height = height + TOP_MARGIN + BOTTOM_MARGIN;
	widget_rect->x1 = x - LEFT_MARGIN;
	widget_rect->y1 = y - TOP_MARGIN;
    }
    else {
	widget_rect->width = 1;
	widget_rect->height = 1;
	widget_rect->x1 = 0;
	widget_rect->y1 = 0;
    }

    /*
    **  Calculate the other rectangular coordinates
    */

    widget_rect->x2 = widget_rect->x1 + widget_rect->width;
    widget_rect->y2 = widget_rect->y1 + widget_rect->height;
    widget_rect->area = widget_rect->height * widget_rect->width;

    return;
    }


static void create_tiling(widget_rect)
    rect *widget_rect;
/*
**++
**  Functional Description:
**	Determine some tiling parameters, and allocate the tiling array.
**
**  Keywords:
**	None
**
**  Arguments:
**	widget_rect : rect structure for new Widget
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    /*
    **  Tile the display with a matrix of rectangles.  The number of tiles
    **	depends on the relative size of the display and the new widget (with a
    **	minimum of a 2X2 matrix, and a maximum of an 8X8 matrix).
    */

    tile_size = screen.area / widget_rect->area;
    tile_size = MIN((int) tile_size, MAX_TILES);
    tile_size = MAX((int) tile_size, MIN_TILES);

    /*
    **	Calculate the basic dimensions of a tile rectangle.  Then, redefine the
    **	screen to be exactly tile_size tiles by tile_size tile (avoid problems
    **	with round off errors).
    */
    
    tile.width = screen.width / tile_size;;
    tile.height = screen.height / tile_size;
    tile.area = tile.width * tile.height;

    screen.width = screen.x2 = tile_size * tile.width;
    screen.height = screen.y2 = tile_size * tile.height;
    screen.area = screen.width * screen.height;

    /*
    **  Allocate an array of integers large enough for the tiling (tile
    **	dimension squared), and initialize to zeros.
    */

    tile_array = (int *) calloc((tile_size * tile_size), sizeof(int *));

    return;
    }

static void weight_tiles(widget_cnt,avoid_widgets)
    int widget_cnt;
    Widget *avoid_widgets;
/*
** ++ Functional Description:
**
**	Given an ordered list of widgets, calculate the their contributive
**	weight on the screen tiling.
**
**  Keywords:
**	None
**
**  Arguments:
**	widget_cnt : number of Widget ID's in avoid_widgets
**	avoid_widgets : array of the ID's of Widgets
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    int i;
    int weight;
    rect avoid;

    weight = MAX_WEIGHT * WEIGHT_FACTOR;

    for (i = 0; i < widget_cnt; i++) {
	/*
	**  Get the screen position of this Widget
	*/
	
	widget_to_rect(avoid_widgets[i], &avoid);

	/*
	**  Calculate this Widget's weight.  Weights are expontentially
	**  decreasing values between 256 and 1 based on the position of the
	**  Widget in the list of Widgets to avoid.
	*/
	
	weight = MAX((weight / WEIGHT_FACTOR), MIN_WEIGHT);

	/*
	**  Go fill in the tile array for this Widget.
	*/
	
	weight_widget(&avoid, weight);
    }

    return;
    }

static void weight_widget(avoid, weight)
    rect *avoid;
    int weight;
/*
** ++
** Functional Description:
**     Given a widget and its weight, calculate the contributive weight on each
**     screen tile which the widget occludes.
**
**  Keywords:
**	None
**
**  Arguments:
**	avoid : rectangle of the widget
**	weight : weight of the widget
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    int row, column;
    rect inter;

    /*
    **  Set coordinates for first tile rectangle
    */

    tile.x1 = tile.y1 = 0;
    tile.x2 = tile.width;
    tile.y2 = tile.height;

    /*
    **	For each tile, calculate the intersection of the tile rectangle and the
    **	widget.  Then, add the widget weight factor to the tile in proportion
    **	to the relative area of the intersecting rectangle and the tile
    **	rectangle.
    */

    for (row = 0; row < tile_size; row++) {

	for (column = 0; column < tile_size; column++) {

	    if (intersect(&tile, avoid, &inter)) {
		tile_array[column + (row * tile_size)] += (int)
		    (((double) inter.area * (double) weight)
		    / (double) tile.area);
	    }

	    /*
	    **  Calculate the next tile rectangle in this row
	    */

	    tile.x1 = tile.x1 + tile.width;
	    tile.x2 = tile.x2 + tile.width;
	}

	/*
	**  Calculate the first tile rectangle in the next row
	*/

	tile.x1 = 0;
	tile.y1 = tile.y1 + tile.height;
	tile.x2 = tile.width;
	tile.y2 = tile.y2 + tile.height;
    }    

    return;
    }

static void unweight_tiles(avoid)
    rect *avoid;
/*
** ++
** Functional Description:
**     Given a rectangle on the screen, unweight surrounding tiles so that
**     the new Widget will tend to be placed nearby.
**
**  Keywords:
**	None
**
**  Arguments:
**	avoid : rectangle of the top widget on the avoid list
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    int i;
    Position x1, y1, x2, y2;
    Dimension width, height;
    int tiles_to_weight;
    int weight;
    int weight_increment;
    int cycles;
    int row, column;

    /*
    **  Determine the tile coordinates of the widget-to-avoid.
    */

    x1 = MIN((int) (tile_size - 1), avoid->x1 / tile.width);
    y1 = MIN((int) (tile_size - 1), avoid->y1 / tile.height);
    x2 = MIN((int) (tile_size - 1), avoid->x2 / tile.width);
    y2 = MIN((int) (tile_size - 1), avoid->y2 / tile.height);
    width = (x2 - x1 + 1);
    height = (y2 - y1 + 1);

    /*
    **	Determine the weight for the initial tile and the increment that will
    **	be used for subsequent tiles.  If the widget-to-avoid nearly covers the
    **	screen, there is no point in proceeding.
    */

    tiles_to_weight = (tile_size * tile_size) - (width * height);

    if (tiles_to_weight <= 0)
	return;

    weight = (MAX_WEIGHT / WEIGHT_FACTOR / WEIGHT_FACTOR / WEIGHT_FACTOR);
    weight_increment = (weight - MIN_WEIGHT) / tiles_to_weight;

    /*
    **  Spiral around the widget-to-avoid in a counter clockwise direction
    **	unweighting tiles as we go.
    **
    **	To start with, determine the number of single tile wide cycles of the
    **	widget will be required so that every tile on the screen is considered.
    */

    cycles = MAX((int) x1, (int) y1);
    cycles = MAX(cycles, (int) (tile_size - (x1 + width)));
    cycles = MAX(cycles, (int) (tile_size - (y1 + height)));

    /*
    **	Now, circle the widget in a counter clockwise direction, unweighting
    **	each tile by a diminishing amount.  Be careful not to unweight tiles
    **	that are off the screen.
    */

    for (i = 0; i < cycles; i++) {

	/*
	**  Up the right side
	*/

	if ((column = x2 + 1 + i) < tile_size)
	    for (row = y2 + i; row >= y1 - i; row--)
		if (row >= 0 && row < tile_size) {
		    tile_array[column + (row * tile_size)] -= weight;
		    weight -= weight_increment;
		}

	/*
	**  Across the top
	*/

	if ((row = y1 - 1 - i) >= 0)
	    for (column = x2 + 1 + i; column >= x1 - 1 - i; column--)
		if (column >= 0 && column < tile_size) {
		    tile_array[column + (row * tile_size)] -= weight;
		    weight -= weight_increment;
		}

	/*
	**  Down the left side
	*/

	if ((column = x1 - 1 - i) >= 0)
	    for (row = y1 - i; row <= y2 + i; row++)
		if (row >= 0 && row < tile_size) {
		    tile_array[column + (row * tile_size)] -= weight;
		    weight -= weight_increment;
		}

	/*
	**  Back across the bottom
	*/

	if ((row = y2 + 1 + i) < tile_size)
	    for (column = x1 - 1 - i; column <= x2 + 1 + i; column++)
		if (column >= 0 && column < tile_size) {
		    tile_array[column + (row * tile_size)] -= weight;
		    weight -= weight_increment;
		}
    }

    return;
    }

static void calc_position(widget)
    rect *widget;
/*
**++
**  Functional Description:
**	Calculate the best (least weight) location for a widget over the screen
**	tiles.
**
**  Keywords:
**	None
**
**  Arguments:
**	widget : rectangle structure for the widget to be positioned
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    Position best_x, best_y;
    int weight, best_weight;
    int row, column;
    int tile_row, tile_column;
    int widget_row, widget_column;
    int width_in_tiles, height_in_tiles;
    rect inter;

    /*
    **  Figure out how many tiles are covered by the new Widget.  We only need
    **	to consider that number of tiles as we test various placements of the
    **	new Widget on the screen.
    */

    width_in_tiles = (widget->width + tile.width - 1) / tile.width;
    height_in_tiles = (widget->height + tile.height - 1) / tile.height;

    /*
    **  Place the widget at each valid tile coordinate (widget must not overrun
    **	the screen boundaries), and sum up the weight of the occluded tiles.
    **	Keep track of the position that results in the lowest total weight.
    */

    best_x = 0;
    best_y = 0;
    best_weight = MAX_WEIGHT * tile_size * tile_size * 100;

    for (row = 0; row <= tile_size - (height_in_tiles - 1); row++) {

	for (column = 0; column <= tile_size - (width_in_tiles - 1); column++) {

	    /*
	    **  Calculate the coordinates of the tile at these coordinates
	    */

	    tile.x1 = column * tile.width;
	    tile.y1 = row * tile.height;
	    tile.x2 = tile.x1 + tile.width;
	    tile.y2 = tile.y1 + tile.height;

	    /*
	    **  Calculate the coordinates of the widget (don't exceed the
	    **	screen boundaries).  The widget coordinates include its screen
	    **	coordinates and its tile coordinates.
	    */

	    if (tile.x1 + widget->width <= screen.width) {
		widget->x1 = tile.x1;
		widget_column = column;
	    }
	    else {
		widget->x1 = screen.width - widget->width;
		widget_column = column - 1;	/* Back up one tile */
		tile.x1 = tile.x1 - tile.width;
		tile.x2 = tile.x2 - tile.width;
	    }
	    
	    if (tile.y1 + widget->height <= screen.height) {
		widget->y1 = tile.y1;
		widget_row = row;
	    }
	    else {
		widget->y1 = screen.height - widget->height;
		widget_row = row - 1;		/* Back up one tile */
		tile.y1 = tile.y1 - tile.height;
		tile.y2 = tile.y2 - tile.height;
	    }

	    widget->x2 = widget->x1 + widget->width;
	    widget->y2 = widget->y1 + widget->height;

	    /*
	    ** For each tile under the widget, calculate the intersection of
	    ** the tile rectangle and the widget.  Then, add in the tile weight
	    ** in proportion to the relative area of the intersecting rectangle
	    ** and the tile rectangle.
	    */

	    weight = 0;

	    for (tile_row = widget_row;
		 tile_row < widget_row + height_in_tiles;
		 tile_row++) {

		for (tile_column = widget_column;
		     tile_column < widget_column + width_in_tiles;
		     tile_column++) {

		    if (intersect(&tile, widget, &inter)) {
			weight += (int) ((double)
			    tile_array[tile_column + (tile_row * tile_size)]
			      * (double) inter.area / (double) tile.area);
		    }

		    /*
		    **  Calculate the next tile rectangle in this row
		    */

		    tile.x1 = tile.x1 + tile.width;
		    tile.x2 = tile.x2 + tile.width;
		}

		/*
		**  Calculate the first tile rectangle in the next row
		*/

		tile.x1 = widget_column * tile.width;
		tile.y1 = tile.y1 + tile.height;
		tile.x2 = tile.x1 + tile.width;
		tile.y2 = tile.y1 + tile.height;
	    }

	    /*
	    **	Now, see how good this position is relative the the others we
	    **	have tried.
	    */
	    
	    if (weight <= best_weight) {
		/*
		**  This is a lower weight position, save it.
		*/
		best_weight = weight;
		best_x = widget->x1;
		best_y = widget->y1;
	    }
	}
    }

    /*
    **	Set the rectangle coordinates of the new widget to what we think is
    **	best.
    */
    
    widget->x1 = best_x;
    widget->y1 = best_y;
    widget->x2 = widget->x1 + widget->width;
    widget->y2 = widget->y1 + widget->height;
    
    return;
    }


static void set_position(widget, x, y)
    Widget widget;
    Position x, y;
/*
**++
**  Functional Description:
**	Set the postion of a widget to a given (X,Y) coordinate
**
**  Keywords:
**	Nonme
**
**  Arguments:
**	widget : ID of the widget to be positioned
**	x : X coordinate
**	y : Y coordinate
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    Arg	arglist[2];

    XtSetArg(arglist[0], XtNx, x + LEFT_MARGIN);
    XtSetArg(arglist[1], XtNy, y + TOP_MARGIN);
    XtSetValues(widget, arglist, 2);
    
    return;
    }


static Boolean intersect(rect1, rect2, inter)
    rect *rect1;
    rect *rect2;
    rect *inter;
/*
**++
**  Functional Description:
**	Determine the intersection of two rectangles.
**
**  Keywords:
**	Intersection
**
**  Arguments:
**	rect1	pointer to rectangle structure 1 (read-only)
**	rect2	pointer to rectangle structure 2 (read-only)
**	inter	pointer to intersection rectangle structure (write)
**
**  Result:
**	Boolean: T if there was an intersection (rectangle returned in inter),
**	F if there was no intersection.
**
**  Exceptions:
**	None
**--
*/
    {
    Boolean ll,	/* T if the left edge of box 1 is to the left of box 2   */
	    rr,	/* T if the right edge of box 1 is to the right of box 2 */ 
	    bb,	/* T if the bottom of box 1 is below box 2               */
	    tt;	/* T if the top of box 1 is above box 2                  */

    /*
    **  First, do some investigation to determine the relative location of the
    **	two rectangles, and identify those cases where there is clearly no
    **	intersection.
    */

    if (rect1->x1 < rect2->x1) {
	if (rect1->x2 < rect2->x1) {
	    return F;		/* 1 to left of 2, no intersection */
	}
	else {
	    ll = T;
	    if (rect1->x2 > rect2->x2)
		rr = T;
	    else
		rr = F;
	}
    }
    else {
	if (rect1->x1 > rect2->x2) {
	    return F;		/* 1 to right of 2, no intersection */
	}
	else {
	    ll = F;
	    if (rect1->x2 > rect2->x2)
		rr = T;
	    else
		rr = F;
	}
    }

    if (rect1->y1 < rect2->y1) {
	if (rect1->y2 < rect2->y1) {
	    return F;		/* 1 below 2, no intersection */
	}
	else {
	    bb = T;
	    if (rect1->y2 > rect2->y2)
		tt = T;
	    else
		tt = F;
	}
    }
    else {
	if (rect1->y1 > rect2->y2) {
	    return F;		/* 1 above 2, no intersection */
	}
	else {
	    bb = F;
	    if (rect1->y2 > rect2->y2)
		tt = T;
	    else
		tt = F;
	}
    }

    /*
    **	Now, using what we just determined about the relative locations of the
    **	edges of the two rectangles, deal with each coordinate of the
    **	intersection rectangle individually.
    */
    
    if (ll)
	inter->x1 = rect2->x1;
    else
	inter->x1 = rect1->x1;

    if (bb)
	inter->y1 = rect2->y1;
    else
	inter->y1 = rect1->y1;
	
    if (rr)
	inter->x2 = rect2->x2;
    else
	inter->x2 = rect1->x2;

    if (tt)
	inter->y2 = rect2->y2;
    else
	inter->y2 = rect1->y2;

    inter->width = inter->x2 - inter->x1;
    inter->height = inter->y2 - inter->y1;
    inter->area = inter->width * inter->height;

    return T;
    }
