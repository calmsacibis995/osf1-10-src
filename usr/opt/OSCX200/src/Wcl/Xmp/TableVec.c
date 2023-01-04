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
/*LINTLIBRARY*/
/*
 * SCCS_data:    @(#) TableVec.c	1.6 92/06/10 06:14:03
 */

#include <X11/Xmp/COPY>
#include <X11/IntrinsicP.h>
#include <X11/Xmp/TableP.h>

/* XmpTableVector Methods
**========================**
   Each XmpTable instance has two XmpTableVectors: one describes the
   columns, and the other describes the rows.

   The table vectors are created based on information in the real_layout
   XmpTableLoc array, hence they must be created after the real_layout,
   and they must be updated when the real_layout changes.  The real_layout
   data upon which the vectors depend is: number of cols, number of rows,
   options (only TBL_SM_WIDTH and TBL_SM_HEIGHT).
*/

XmpTableVector XmpTableVectorNew( size, tw, do_col )
    int			size;
    XmpTableWidget	tw;
    int			do_col;
{
    int			minimize, dontStretch, first_slot, last_slot, slot;
    XmpTableLoc		loc = tw->table.real_layout;
    XmpTableVector	vec;

    if ( (XmpTableLoc)0 == loc || 0 == size )
	return (XmpTableVector)0;

    vec = (XmpTableVector)XtCalloc( size+1, sizeof(XmpTableVectorRec));

    /* Determine which slots need to be minimized
    */
    for (  ;  loc->w  ;  loc++ )
    {
	if ( do_col )
	{
	    minimize	= loc->options & TBL_SM_WIDTH;
	    dontStretch	= loc->options & TBL_LK_WIDTH;
	    first_slot	= loc->col;
	    last_slot	= loc->col + loc->col_span;
	}
	else
	{
	    minimize	= loc->options & TBL_SM_HEIGHT;
	    dontStretch	= loc->options & TBL_LK_HEIGHT;
	    first_slot	= loc->row;
	    last_slot	= loc->row + loc->row_span;
	}

	if ( minimize )
	    for ( slot = first_slot ; slot < last_slot ; slot++ )
		vec[ slot ].options |= TBL_VEC_MINIMIZE;

	if ( dontStretch )
	    for ( slot = first_slot ; slot < last_slot ; slot++ )
		vec[ slot ].options |= TBL_VEC_LOCK;
    }
    XmpTableVectorMinimize( vec, size, tw, do_col );
    return vec;
}

void XmpTableVectorFree( vec )
    XmpTableVector vec ;
{
    XtFree( (char*)vec );
}


/* Minimize Column Widths and Row Heights
**========================================**
   Change the vector to be its minimum size in the direction indicated.
   If TBL_VEC_MINIMIZE (i.e., TBL_SM_WIDTH (W) or TBL_SM_HEIGHT (H)) then
   the widget is kept to its original size. 
   TBL_VEC_LOCK (i.e., TBL_LK_WIDTH (w) or TBL_LK_HEIGHT (h) ) not checked,
   because such widgets DO grow to the minimum size of the column or row.
*/
void XmpTableVectorMinimize( vec, vec_len, tw, do_col )
    XmpTableVector	vec;
    int			vec_len;
    XmpTableWidget	tw;
    int			do_col;
{
    int			i;
    int			shrink_simple	= tw->table.shrink_simple;
    XmpTableLoc		loc		= tw->table.real_layout;

    if ( (XmpTableVector)0 == vec || 0 == vec_len )
	return;

    /* Sort real_layout (in-place) by the number of columns or rows each child
    ** spans so we first compute sizes of individual columns or rows, then 
    ** compute the spanned columns or rows.
    */
    if ( do_col )
	qsort( (char*)loc,			/* sort real_layout	*/
		XmpTableLocLen(loc),
		sizeof(XmpTableLocRec),
		XmpTableLocCompareColSpan );	/* compare column spans	*/
    else
	qsort( (char*)loc,			/* sort real_layout	*/
		XmpTableLocLen(loc),
		sizeof(XmpTableLocRec),
		XmpTableLocCompareRowSpan );	/* compare row spans	*/

    /* Reset all width|heights to zero, then expand to fit
    */
    for ( i = 0 ; i < vec_len ; i++ )
	vec[i].value = 0;

    for (  ;  loc->w  ;  loc++ )
    {
	int pref;

	/* Check for simple case (span of 1), where col or row just becomes
	** large enough for largest child in that col or row.
	*/
	if (  do_col && loc->col_span == 1 )
	{
	    if ( vec[ loc->col ].options & TBL_VEC_MINIMIZE )
		pref = loc->orig_width;
	    else
		pref = XmpTableLocPreferredWidth( loc, shrink_simple );

	    if ( pref > vec[ loc->col ].value )
		vec[ loc->col ].value = pref;
	}
	else if ( !do_col && loc->row_span == 1 )
	{
	    if ( vec[ loc->row ].options & TBL_VEC_MINIMIZE )
		pref = loc->orig_height;
	    else
		pref = XmpTableLocPreferredHeight( loc, shrink_simple );

	    if ( pref > vec[ loc->row ].value )
		vec[ loc->row ].value = pref;
	}

	else
	{
	    /* Spans multiple columns or rows.  We have already set each
	    ** column or row to the individual size requirements, now we can
	    ** see which spanned columns or rows need to be stretched.  The
	    ** span width includes inter-column or inter-row spacing.
	    */
	    int to_stretch, span_size, first_slot, stop_before, slot;
	    int can_stretch = 0;

	    if ( do_col )
	    {
		span_size   = tw->table.col_spacing * (loc->col_span-1);
		first_slot  = loc->col;
		stop_before = loc->col + loc->col_span;
	    }
	    else
	    {
		span_size   = tw->table.row_spacing * (loc->row_span-1);
		first_slot  = loc->row;
		stop_before = loc->row + loc->row_span;
	    }
	    for ( slot = first_slot  ;  slot < stop_before  ;  slot++ )
	    {
		if ( 0 == (vec[ slot ].options & TBL_VEC_LOCK) )
		    can_stretch++;
		span_size += vec[ slot ].value;
	    }

	    /* If none of the slots can stretch, then we still must force
	    ** them all to stretch at least to the orig_size of this widget.
	    */
	    if ( 0 == can_stretch )
	    {
		if ( do_col )
		{
		    to_stretch	= loc->col_span;
		    pref	= loc->orig_width;
		}
		else
		{
		    to_stretch	= loc->row_span;
		    pref	= loc->orig_height;
		}
	    }
	    else
	    {
		to_stretch = can_stretch;
		if ( do_col )
		    pref = XmpTableLocPreferredWidth( loc, shrink_simple );
		else
		    pref = XmpTableLocPreferredHeight( loc, shrink_simple );
	    }

	    if ( span_size < pref )
	    {
		/* Increase size of some or all slots: if nothing
		** can stretch, expand every slot, else expand only
		** those which are not locked small.
		*/
		int excess	= pref - span_size;
		int amt		= excess / to_stretch;
		int truncated	= excess - amt*to_stretch;

		for ( slot = first_slot  ;  slot < stop_before  ;  slot++ )
		{
		    if ( 0 == can_stretch
		      || 0 == (vec[ slot ].options & TBL_VEC_LOCK) )
			vec[ slot ].value += amt + truncated;
		    truncated = 0;
		}
	    }
	}
    }
    /* The vector is minimized: set pref_value from value
    */
    for ( i = 0 ; i < vec_len ; i++ )
	vec[i].pref_value = vec[i].value;
}

/* Total Width or Height
**=======================**
    Including inter-column and inter-row spacing, and margins.  Works
    even when there are no columns or rows (vec==num==0).
*/
#define DO_ACTUAL 1
#define DO_PREFERRED 0

static int XmpTableVectorSize( vec, num, tw, do_col, do_actual )
    XmpTableVector	vec;
    int			num;
    XmpTableWidget	tw;
    int			do_col;
    int			do_actual;
{
    int slot, size, space;

    if (do_col)
    {
	space = tw->table.col_spacing;
	size  = 2*tw->bulletin_board.margin_width;
    }
    else
    {
	space = tw->table.row_spacing;
	size  = 2*tw->bulletin_board.margin_height;
    }

    if ( 0 != num && (XmpTableVector)0 != vec )
    {
	if (do_actual)
	{
	    for ( size -= space, slot = 0  ;  slot < num  ;  slot++ )
		size += vec[ slot ].value + space;
	}
	else
	{
	    for ( size -= space, slot = 0  ;  slot < num  ;  slot++ )
		size += vec[ slot ].pref_value + space;
	}
    }

    return size;
}

int XmpTableVectorTotalSize( vec, num, tw, do_col )
    XmpTableVector	vec;
    int			num;
    XmpTableWidget	tw;
    int			do_col;
{
    return XmpTableVectorSize( vec, num, tw, do_col, DO_ACTUAL );
}

int XmpTableVectorPreferredSize( vec, num, tw, do_col )
    XmpTableVector	vec;
    int			num;
    XmpTableWidget	tw;
    int			do_col;
{
    return XmpTableVectorSize( vec, num, tw, do_col, DO_PREFERRED );
}

#undef DO_ACTUAL
#undef DO_PREFERRED


/* Adjust rows or columns
**========================**
   When a parent re-sizes a Table, it can make it larger or smaller.

   If the table wants to restrict making things smaller than preferred,
   then it must simply never respond to resize commands which make
   the table smaller than its preferred size.  Nowhere in the logic below
   is there any mechanism which prevents things from shrinking smaller
   than the preferred size.  There is, however, mechanisms to prevent any
   col or row from becoming smaller than 1.

   If resize makes the Table larger than before:  First, all col/row
   smaller that preferred size are stretched up until their preferred
   size.  The rest of the change is distributed evenly to un-locked col/row,
   but if all are locked, then all are stretched.

   If the table is being made smaller, then the inverse is applied: all
   unlocked (or all if all are locked) are made smaller down to their
   preferred sizes, then all are made smaller by the same amount.
*/

void XmpTableVectorAdjust( vec, vec_len, change )
    XmpTableVector      vec;
    int                 vec_len, change;
{
    int vec_inx, current, prefer, too_small, can_change;
    int to_change, amt, num_larger_than_1;

    if ( (XmpTableVector)0 == vec || 0 == vec_len || 0 == change )
	return;

    current = prefer = too_small = can_change = 0;

    for ( vec_inx = 0  ;  vec_inx < vec_len  ;  vec_inx++ )
    {
	current += vec[ vec_inx ].value;
	prefer  += vec[ vec_inx ].pref_value;
	if ( vec[ vec_inx ].value < vec[ vec_inx ].pref_value )
	    ++too_small;
	if ( 0 == ( vec[ vec_inx ].options & TBL_VEC_LOCK ) )
	    can_change++;
    }

    if ( change > 0 )
    {
	/*
	 * Make columns wider or rows taller
	 *
	 * First allow all to become preferred size
	 */
	if ( too_small )
	{
	    /* Something is smaller than preferred
	    */
	    if ( current + change < prefer )
	    {
		/* Spread as evenly as possible to all smaller than preferred.
		 * Two things to watch for: lots of too_small and not much
		 * change (integer division then leaves amt == 0).  In this
		 * case we make amt = 1, which means the change gets used up
		 * before we look at all the col/row  The second problem is
		 * when some of the col/row are closer to their pref_value than
		 * amt, which means that they take less than amt, and others
		 * must take more.
		 */
		while ( change && too_small )  /* check too_small for safety */
		{
		    amt = change / too_small;
		    if ( 0 == amt )
		        amt = 1;	/* we will use up change before vecs */

		    too_small = 0;

		    for ( vec_inx=0  ;  change && vec_inx<vec_len  ;  vec_inx++)
		    {
			if ( vec[ vec_inx ].value < vec[ vec_inx ].pref_value )
			{
			    /* Make this one bigger, up to preferred size
			    */
			    if (  vec[vec_inx].value + amt
				< vec[vec_inx].pref_value )
			    {
				vec[ vec_inx ].value += amt;
				change -= amt;
				too_small++;
			    }
			    else
			    {
				int a = vec[vec_inx].pref_value -
					vec[vec_inx].value;
				vec[vec_inx].value = vec[vec_inx].pref_value;
				change -= a;
			    }
			}
		    }
		}

		/* We are done now.
		*/
		return;
	    }
	    else /*  ( current + change >= prefer )  */
	    {
		/* Make everything the preferred size.
		*/
		for ( vec_inx = 0  ;  vec_inx < vec_len  ;  vec_inx++ )
		    vec[ vec_inx ].value = vec[ vec_inx ].pref_value;

		if ( current + change == prefer )
		    return;

		change -= prefer - current;
		current = prefer;
	    }
	    /* Change has been decr'd by amt used to make all preferred size.
	    */
	}

	/* Everything is at least preferred size.  
	 * If none of the vector slots can stretch, then we still must
	 * force them all to stretch.
	 */
	if ( 0 == can_change )
	    to_change = vec_len;
	else
	    to_change = can_change;

	while ( change )
	{
	    /* Add same amount to all which can change.
	    */
	    amt = change / to_change;
	    if ( 0 == amt )
		amt = 1;

	    for ( vec_inx = 0  ;  0 < change && vec_inx < vec_len  ;  vec_inx++)
	    {
		if ( 0 == can_change
		  || 0 == ( vec[ vec_inx ].options & TBL_VEC_LOCK ) )
		{
		    vec[ vec_inx ].value += amt;
		    change -= amt;
		}
	    }
	}
    }
    else /*  (change < 0)  */
    {
	/*
	 * Make columns more narrow or rows shorter
	 *
	 * See how much can be taken out to get all down to preferred size
	 */
	int num_stretched, total_stretch, stretch;
	num_stretched = total_stretch = 0;

	/* For conceptual clarity, switch the sign on change
	*/
	change = -change;

	for ( vec_inx = 0  ;  vec_inx < vec_len  ;  vec_inx++ )
	{
	    if ( 0 == can_change
	      || 0 == ( vec[ vec_inx ].options & TBL_VEC_LOCK ) )
	    {
		stretch = vec[ vec_inx ].value - vec[ vec_inx ].pref_value;
		if ( 0 < stretch )
		{
		    num_stretched++;
		    total_stretch += stretch;
		}
	    }
	}

	if (num_stretched)
	{
	    if ( change < total_stretch )
	    {
		/* Spread change as evenly as possible, and then we are done.
		*/
		while ( change )
		{
		    amt = change / num_stretched;
		    if ( 0 == amt )
			amt = 1;

		    /* Shrink all which were stretched until change is absorbed
		    */
		    for ( vec_inx=0 ; 0<change && vec_inx<vec_len ; vec_inx++)
		    {
			if ( 0 == can_change
			  || 0 == ( vec[ vec_inx ].options & TBL_VEC_LOCK ) )
			{
			    stretch = vec[vec_inx].value -
					vec[vec_inx].pref_value;
			    if ( amt < stretch )
			    {
				vec[vec_inx].value -= amt;
				change -= amt;
			    }
			    else
			    {
				vec[vec_inx].value -= stretch;
				change -= stretch;
			    }
			}
		    }
		}

		/* We are done.
		*/
		return;
	    }
	    else  /* if ( change >= total_stretch ) */
	    {
		/* Shrink all stretched to preferred sizes
		*/
		for ( vec_inx = 0  ;  vec_inx < vec_len  ;  vec_inx++ )
		{
		    if ( 0 == can_change
		      || 0 == ( vec[ vec_inx ].options & TBL_VEC_LOCK ) )
		    {
			stretch = vec[vec_inx].value - vec[vec_inx].pref_value;
			if ( 0 < stretch )
			{
			    vec[vec_inx].value -= stretch;
			    change -= stretch;
			}
		    }
		    
		}

		/* If change was equal to total_stretch then we are done.
		*/
		if ( 0 == change )
		    return;
	    }
	}
	/* Now all stretchable are preferred sizes, or all were already smaller
	 * than preferred sizes, yet more change is to be absorbed.
	 *
	 * Shrink evenly, but since none can become smaller than 1, we may need
	 * to make multiple passes over vector until total change is absorbed,
	 * or all are of size 1.
	 */
	num_larger_than_1 = vec_len;

	while ( 0 < change && num_larger_than_1 )
	{
	    amt = change / num_larger_than_1;
	    if ( 0 == amt )
	        amt = 1;

	    num_larger_than_1 = 0;

	    for ( vec_inx = 0  ; 0 < change && vec_inx < vec_len  ;  vec_inx++ )
	    {
		if ( amt < vec[vec_inx].value )
		{
		    vec[vec_inx].value -= amt;
		    change -= amt;
		}
		else
		{
		    change -= vec[vec_inx].value - 1;
		    vec[vec_inx].value = 1;
		}
		if ( 1 < vec[vec_inx].value )
		    ++num_larger_than_1;
	    }
	}
    }
}

/* Set Upper Left Corner Coordinates of Each Cell
**================================================**
   Note that it is not worth doing this until the actual correct size of
   the rows and columns have been computed.
*/

void XmpTableVectorComputeOffsets( vec, vec_len, margin, gap )
    XmpTableVector	vec;
    int			vec_len, margin, gap;
{
    int i;
    int offset = margin;

    if ( (XmpTableVector)0 == vec || 0 == vec_len )
	return;

    for ( i = 0  ;  i < vec_len  ;  i++ )
    {
	vec[i].offset = offset;
	offset = offset + vec[i].value + gap;
    }
}
