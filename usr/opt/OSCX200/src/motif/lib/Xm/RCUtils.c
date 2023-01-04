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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: RCUtils.c,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/15 17:00:50 $"
#endif
#endif
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/XmP.h>
#include <Xm/RCUtilsP.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif

/**************************************************************** ARGSUSED
 * Assemble a kid box for each child widget and gadget, fill in data about
 *   each widget and optionally set up uniform border widths.
 * Returns a list of records, last one has a 'kid' field of NULL.  This memory
 *   for this list should eventually be freed with a call to XtFree().
 ****************/
XmRCKidGeometry
#ifdef _NO_PROTO
_XmRCGetKidGeo( wid, instigator, request, uniform_border, border, 
uniform_width_margins, uniform_height_margins, help, toc, geo_type )
        Widget wid ;
        Widget instigator ;
        XtWidgetGeometry *request ;
        int uniform_border ;
        Dimension border ;
        int uniform_width_margins ;
        int uniform_height_margins ;
        Widget help ;
	Widget toc ;
        int geo_type ;
#else
_XmRCGetKidGeo(
        Widget wid,                     /* Widget w/ children. */
        Widget instigator,              /* May point to a child who */
        XtWidgetGeometry *request,      /*   is asking to change. */
        int uniform_border,             /* T/F, enforce it. */
#if NeedWidePrototypes
        int border,
#else
        Dimension border,               /* Value to use if enforcing.*/
#endif /* NeedWidePrototypes */
        int uniform_width_margins,      /* T/F, enforce it. */
        int uniform_height_margins,     /* T/F, enforce it. */
        Widget help,                    /* May point to a help kid. */
	Widget toc,			/* May point to tear_off_control kid. */
        int geo_type )                  /* Actual or preferred. */
#endif /* _NO_PROTO */
{
    CompositeWidget	c = (CompositeWidget) wid ;
    XmRCKidGeometry	geo ;
    Widget		kidWid ;
    int			i ;
    int			j = 0 ;
    Boolean		helpFound = FALSE ;
    Boolean		tocFound;

    tocFound = (toc && XtIsManaged(toc)) & 0x1;

    geo = (XmRCKidGeometry) XtMalloc((_XmGeoCount_kids(c) + 1 + tocFound) * 
       sizeof (XmRCKidGeometryRec));

    i = 0;

    if (tocFound)
    {
       geo[j].kid = toc ;

       _XmGeoLoadValues( toc, geo_type, instigator, request, &(geo[j].box));

       geo[j].margin_top = 0;
       geo[j].margin_bottom = 0;
       geo[j].baseline = 0;


       if (uniform_border)     /* if asked override border */
       {   
	  geo[j].box.border_width = border ;
       }
       j++ ;
    }

    /* load all managed kids */
    for( ; i < c->composite.num_children ; i++    )
    {
       kidWid = c->composite.children[i] ;
       if (XtIsManaged( kidWid))
       {   
	  if(    kidWid == help    )
          {  /* Save to put help widget at the end of the widget list.*/
             helpFound = TRUE ;
          }
          else
	  {   
	     geo[j].kid = kidWid ;

	     _XmGeoLoadValues( kidWid, geo_type, instigator, request,
							       &(geo[j].box)) ;
             geo[j].margin_top = 0;
             geo[j].margin_bottom = 0;
             geo[j].baseline = 0;


	     if (uniform_border)     /* if asked override border */
	     {   
		geo[j].box.border_width = border ;
	     }
	     j++ ;
	  }
       }
    }

    if (helpFound)                 /* put help guy into list */
    {
        geo[j].kid = help ;

        _XmGeoLoadValues( help, geo_type, instigator, request, &(geo[j].box)) ;

        geo[j].margin_top = 0;
        geo[j].margin_bottom = 0;
        geo[j].baseline = 0;


        if (uniform_border)         /* if asked override border */
        {   
	   geo[j].box.border_width = border ;
	}
        j++ ;
    }
    geo[j].kid = NULL ;                /* signal end of list */

    return( geo) ;
}


/**************************************************************** ARGSUSED
 * Take the kid geometry array and change each kid to match them.
 *   remember not to do the resize of the instigator.
 * The kid geometry "kg" is assumed to be fully specified.
 ****************/
void
#ifdef _NO_PROTO
_XmRCSetKidGeo( kg, instigator )
        XmRCKidGeometry kg ;
        Widget instigator ;
#else
_XmRCSetKidGeo(
        XmRCKidGeometry kg,
        Widget instigator )
#endif /* _NO_PROTO */
{
    Widget          w ;
    XtWidgetGeometry * b ;
    int             i ;
/****************/

    for(i=0 ; kg[i].kid != NULL ; i++) {
        w = (Widget) kg[i].kid ;
        b = &(kg[i].box) ;

	if(    w != instigator    ) { 
	    _XmConfigureObject(w, b->x, b->y, b->width, b->height, 
			       b->border_width) ;
	} else {   
	    XtX( w) = b->x ;
	    XtY( w) = b->y ;
	    XtWidth( w) = b->width ;
	    XtHeight( w) = b->height ;
	    XtBorderWidth( w) = b->border_width ;
	}
    }
    return ;
}
