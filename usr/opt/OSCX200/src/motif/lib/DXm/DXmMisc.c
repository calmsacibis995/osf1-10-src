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
**                     COPYRIGHT (c) 1990, 1991 BY                          *
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
**  THE I0NFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
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
**	< to be supplied >
**
**  ABSTRACT:
**
**	< to be supplied >
**
**  ENVIRONMENT:
**
**	< to be supplied >
**
**  MODIFICATION HISTORY:
**
**	< to be supplied >
**
**--
**/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/DECwI18n.h>
#include <Xm/XmP.h>
#include "DXmPrivate.h"
#include <DXm/DECspecific.h>
#include <Xm/BulletinBP.h>
#include <Xm/FormP.h>
#include <Xm/DialogSP.h>
#include <Mrm/MrmPublic.h>
#include <Xm/LabelP.h>
#include <Xm/LabelGP.h>
#if defined(I18N_EXTENSION) && defined(IN_XMLIBSHR)
/* 
	Include CSText in decw$xmlibshr.
	CSText use DXmCvt* converters, thus DXmMisc.c is required.
	But decw$xmlibshr doesn't have DXm*ClassRec, so some part
	of this file is commented out to create DXmMisc.obj to be
	included in decw$xmlibshr
*/
#else
#ifndef WIN32
#include <DXm/DXmHelpBP.h>
#include <DXm/DXmPrintP.h>
#endif
#include <DXm/DXmColorP.h>
#include <DXm/DXmSvnP.h>
#include <DXm/DXmCSTextP.h>
#endif /* not (I18N_EXTENSION && IN_XMLIBSHR) */

#ifdef VMS
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif
#include <X11/cursorfont.h>

/* type modifier signed only works with __STDC__ */
#if defined(__STDC__) && __STDC__
#else
#define signed 
#endif


/* asn1 header stuff */

#define ASNHEADERLEN     3
#define ASNHEADER1	0xdf
#define ASNHEADER2	0x80
#define ASNHEADER3	0x06
static unsigned char 	ASNHeader[3] = {ASNHEADER1, ASNHEADER2, ASNHEADER3}; 

#define CSHEADERLEN     3

#define CSHEADER1       0xdf
#define CSHEADER2       0xff
#define CSHEADER3       0x79
static unsigned char CSHeader[3] = {CSHEADER1, CSHEADER2, CSHEADER3};

#define MAXSHORTVALUE   127             /* maximum len to be used for short 
                                           length form */
#define CSLONGLEN       3
#define CSSHORTLEN      1
#define CSLONGLEN1      0x82
#define CSLONGBIT	0x80

#define ASNTAG		1

#define HEADER 3	/* num bytes for tag & length */

/*
 * calculates the number of bytes in the header of an external compound
 * string, given the total length of the components.
 */

#define _calc_header_size(len) \
    ((((unsigned short)(len)) > MAXSHORTVALUE) ? (CSHEADERLEN + CSLONGLEN) : (CSHEADERLEN + CSSHORTLEN))

#define _asn1_size(len) \
    ((((unsigned short)(len)) > MAXSHORTVALUE) ? (ASNTAG + CSLONGLEN) : (ASNTAG + CSSHORTLEN))

#define _is_asn1_long(p) \
  ((*((unsigned char *)(p) + ASNTAG)) & ((unsigned char)CSLONGBIT))

#define Half(x)		(x >> 1)


/*
 * determines from ASN.1 header whether this is an ASN.1 conformant 
 * external compound string.  Returns T or F.
 */

static Boolean 
#ifdef _NO_PROTO
_is_asn1( string )
        XmString string ;
#else
_is_asn1( XmString string )
#endif /* _NO_PROTO */
{
    unsigned char * uchar_p = (unsigned char *) string;

    /*  Compare the ASN.1 header. */
    return (strncmp ((char *)uchar_p, (char *)ASNHeader, ASNHEADERLEN) == 0);
}


/*
 * determines from ASN.1 header whether this is an external compound string.
 * returns T or F.
 */
static Boolean 
#ifdef _NO_PROTO
_is_compound( string )
        XmString string ;
#else
_is_compound(
        XmString string )
#endif /* _NO_PROTO */
{
    unsigned char * uchar_p = (unsigned char *) string;

   /*
    *  Start with comparing the ASN.1 header.
    */
    return (strncmp ((char *) uchar_p, (char *) CSHeader, CSHEADERLEN) == 0);
}


/*
 * calculates length of component marked by a tag-length-value triple.
 */
static unsigned short 
#ifdef _NO_PROTO
_read_asn1_length( p )
        unsigned char *p ;
#else
_read_asn1_length(
        unsigned char *p )
#endif /* _NO_PROTO */
{
    unsigned char * uchar_p = (unsigned char *) p;
    unsigned short totallen = 0;

    /*
     * Read past the tag; get the first length byte and see if this
     * is a one or three byte length.
     */

    uchar_p += ASNTAG;

    if (_is_asn1_long(p))
      {
	unsigned short i;

	uchar_p++;
	i = ((unsigned short) *uchar_p) << 8;
	uchar_p++;
	i |= ((unsigned short) *uchar_p); /* Mask on the low byte */
	totallen += i;
      }
    else 
      {
	totallen += (unsigned short) *uchar_p;
      }
    return (totallen);
}

/*
 * Determines whether this string has a short or long length field
 */
static Boolean 
#ifdef _NO_PROTO
_is_short_length( p )
        unsigned char *p ;
#else
_is_short_length(
        unsigned char *p )
#endif /* _NO_PROTO */
{

    unsigned char *uchar_p = (unsigned char *) p;

    uchar_p += ASNHEADERLEN;

    if (*uchar_p & (char)CSLONGBIT)
       return (FALSE);
    else return (TRUE);
}

/*
 * extracts the ASN.1 header from the external compound string.
 */
static unsigned char * 
#ifdef _NO_PROTO
_read_header( p )
        unsigned char *p ;
#else
_read_header(
        unsigned char *p )
#endif /* _NO_PROTO */
{
    /*
     * Read past the ASN.1 header; get the first length byte and see if this
     * is a one or three byte length.
     */

    if (_is_short_length(p))
        return (p + ASNHEADERLEN + CSSHORTLEN);
    else
       return (p + ASNHEADERLEN + CSLONGLEN); 
}

/*
 * reads the length the ASN.1 header of an external
 * compound string.
 */
static unsigned short 
#ifdef _NO_PROTO
_read_header_length( p )
        unsigned char *p ;
#else
_read_header_length(
        unsigned char *p )
#endif /* _NO_PROTO */
{
    /*
     * Read past the ASN.1 header; get the first length byte and see if this
     * is a one or three byte length.
     */

    if (_is_short_length(p))
       return (ASNHEADERLEN + CSSHORTLEN);
    else
       return (ASNHEADERLEN + CSLONGLEN);

}

/*
 * calculates the length of the external compound string, excluding the
 * ASN.1 header.
 */
static unsigned short 
#ifdef _NO_PROTO
_read_string_length( p )
        unsigned char *p ;
#else
_read_string_length(
        unsigned char *p )
#endif /* _NO_PROTO */
{


    unsigned char * uchar_p = (unsigned char *) p;
    unsigned short totallen = 0;

    /*
     * Read past the ASN.1 header; get the first length byte and see if this
     * is a one or three byte length.
     */

    uchar_p += ASNHEADERLEN;

    if (_is_short_length(p))
    {
       totallen += (unsigned short) *uchar_p;
    }
    else {
       unsigned short i;

       uchar_p++;
       i = ((unsigned short) *uchar_p) << 8;
       uchar_p++;
       i |= ((unsigned short) *uchar_p);    /* Mask on the low byte */
       totallen += i;
    }
    return (totallen);
}




struct ctx_ddif {
    unsigned char *Ptr;               /* Location of current char in CS */
    long Size;      	     /* Size of remaining string to proccess. */
    unsigned long CharSetLen;/* Last CharSet in CS */
    unsigned char *CharSet;
   } ;

#define hunk_default_size 128
/* Define the hunk structures. */

struct hunk_desc
    {
    struct hunk_desc	*next;
    long		alloc_size;
    long		remain_size;
    long		used_size;
    unsigned char	data[hunk_default_size];
    };
static struct hunk_desc	*_new_hunk ();

struct ctx_cs {
    struct hunk_desc *content_hunk; /* Location of current hunk block */
    unsigned char *content_ptr;     /* Location within the hunk block*/
    unsigned long CharSetLen;       /* Last CharSet used */
    unsigned char *CharSet;
   } ;

/* Define structure to describe a tag, length, and status. */
struct tag_info
    {
    unsigned long	error;	/* Error indicator */
    unsigned char	*eoc;	/* Virtual EOC     */
    signed long		tag;	/* Current tag     */
    signed long		len;	/* Value length    */
    };

/*
 * forward decs
*/

#ifdef _NO_PROTO
extern Widget DXmCreatePrintWgt ( );
static unsigned long MCS_rout ( );
static unsigned long DDIF_frag ( );
static unsigned long DDIF_nl ( );
static unsigned long DDIF_dir ( );
static unsigned long _next_tag ( );
static unsigned long _skip_tag ( );
static unsigned long ddif_decode ( );
static struct hunk_desc *_new_hunk ( );
static unsigned long ddif_encode ( );
#else
extern Widget DXmCreatePrintWgt ( Widget ar_parent , char *at_name , ArgList ar_args , int l_arg_count );
static unsigned long MCS_rout ( struct ctx_ddif *CtxP , long *CharSetLen , unsigned char **CharSet , long *FragLen , unsigned char **Frag , unsigned long *nl , unsigned long *Direction , unsigned long *Nesting );
static unsigned long DDIF_frag ( struct ctx_cs *CtxP , unsigned long CharSetLen , unsigned char *CharSet , unsigned long FragLen , unsigned char *FragPtr , unsigned long Dir );
static unsigned long DDIF_nl ( struct ctx_cs *CtxP );
static unsigned long DDIF_dir ( struct ctx_cs *CtxP , unsigned long dir );
static unsigned long _next_tag ( signed long *cnt_ , unsigned char **ptr_ , struct tag_info *info_ );
static unsigned long _skip_tag ( signed long *cnt_ , unsigned char **ptr_ , struct tag_info *info_ );
static unsigned long ddif_decode ( signed long org_cnt , unsigned char *ptr , char *ctx , unsigned long (*rout_frg )(), unsigned long (*rout_nl )(), unsigned long (*rout_nest )());
static struct hunk_desc *_new_hunk ( struct hunk_desc *prv_hunk , signed long size );
static unsigned long ddif_encode
	( signed long *cnt , unsigned char **ptr ,
		char *ctx , unsigned long (*rout )() , int non_std_cs);
#endif /* _NO_PROTO undefined */


#ifdef _NO_PROTO
extern Opaque   _XmCvtCStoFC( );
extern XmString _XmCvtFCtoCS( );
extern Opaque   _XmCvtCStoOS( );
extern XmString _XmCvtOStoCS( );
#else
extern Opaque   _XmCvtCStoFC(XmString cs, int *byte_count, int *status);
extern XmString _XmCvtFCtoCS(Opaque fc,   int *byte_count, int *status);
extern Opaque   _XmCvtCStoOS(XmString cs, int *byte_count, int *status);
extern XmString _XmCvtOStoCS(Opaque os,   int *byte_count, int *status);
#endif


/*
 * should be in Xm.h....
 */
#define XmSTRING_DIRECTION_REVERT 2



#define ObjParent(x)	    (((Object) x)->object.parent)
#define ObjClass(x)	    (((Object) x)->object.widget_class)

#define RX(r)		    (((RectObj) r)->rectangle.x)
#define RY(r)		    (((RectObj) r)->rectangle.y)
#define RWidth(r)	    (((RectObj) r)->rectangle.width)
#define RHeight(r)	    (((RectObj) r)->rectangle.height)
#define RBorder(r)	    (((RectObj) r)->rectangle.border_width)

#define IsSensitive(r)	    (((RectObj) r)->rectangle.sensitive)




#if defined(I18N_EXTENSION) && defined(IN_XMLIBSHR)
/* 
	Include CSText in decw$xmlibshr.
	CSText use DXmCvt* converters, thus DXmMisc.c is required.
	But decw$xmlibshr doesn't have DXm*ClassRec, so some part
	of this file is commented out to create DXmMisc.obj to be
	included in decw$xmlibshr
*/
#else
/*
 * DESCRIPTION:
 *
 *	DXmInitialize is a routine parallel to MrmInitialize.  It registers
 *	the widget classes for each of the DXm widgets.  It must be called
 *	before any of the DXm widgets are created.
 */

static Boolean dxminit_complete = FALSE;

#ifdef _NO_PROTO
void DXmInitialize()
#else
void DXmInitialize(void)
#endif
{
	if (dxminit_complete) return;

#ifndef WIN32
	MrmRegisterClass
		(   0,
		    NULL,
		    "DXmCreateHelp",
		    DXmCreateHelp,
		    (WidgetClass) &dxmHelpClassRec
		);

	MrmRegisterClass
		(   0,
		    NULL,
		    "DXmCreateHelpDialog",
		    DXmCreateHelpDialog,
		    (WidgetClass) &dxmHelpClassRec
		);

	MrmRegisterClass
		(   0,
		    NULL,
		    "DXmCreatePrintBox",
		    DXmCreatePrintBox,
		    (WidgetClass) &dxmPrintWgtClassRec
		);

	MrmRegisterClass
		(   0,
		    NULL,
		    "DXmCreatePrintDialog",
		    DXmCreatePrintDialog,
		    (WidgetClass) &dxmPrintWgtClassRec
		);

	/* Obsolete, but saved for those who haven't changed*/
	/* to DXmCreatePrintDialog.			    */
	MrmRegisterClass
		(   0,
		    NULL,
		    "DXmCreatePrintWgt",
		    DXmCreatePrintWgt,
		    (WidgetClass) &dxmPrintWgtClassRec
		);
#endif

	MrmRegisterClass
		(   0,
		    NULL,
		    "DXmCreateColorMix",
		    DXmCreateColorMix,
		    (WidgetClass) &dxmColorMixClassRec
		);

	MrmRegisterClass
		(   0,
		    NULL,
		    "DXmCreateColorMixDialog",
		    DXmCreateColorMixDialog,
		    (WidgetClass) &dxmColorMixClassRec
		);


	MrmRegisterClass
		(   0,
		    NULL,
		    "DXmCreateSvn",
		    DXmCreateSvn,
		    (WidgetClass) &dxmSvnClassRec
		);

	MrmRegisterClass
		(   0,
		    NULL,
		    "DXmCreateCSText",
		    DXmCreateCSText,
		    (WidgetClass) &dxmCSTextClassRec
		);

	MrmRegisterClass
		(   0,
		    NULL,
		    "DXmCreateScrolledCSText",
		    DXmCreateScrolledCSText,
		    (WidgetClass) &dxmCSTextClassRec
		);

	dxminit_complete = True;
}
#endif /* not (I18N_EXTENSION && IN_XMLIBSHR) */


#if defined(ultrix) && defined(I18N_EXTENSION) && defined(IN_DXMLIBSHR)
/* 
	Include CSText in decw$xmlibshr.
	CSText use DXmCvt* converters, thus DXmMisc.c is required.
	But decw$xmlibshr doesn't have DXm*ClassRec, so some part
	of this file is commented out to create DXmMisc.obj to be
	included in decw$xmlibshr
*/
#else
/*
 * change the subwidget's window geometry
 *
 */

#ifdef _NO_PROTO
void DXmChangeWindowGeometry (w, size)
    Widget w;
    XtWidgetGeometry *size;
#else
void DXmChangeWindowGeometry (Widget w, XtWidgetGeometry *size)
#endif
{
    /*
     * split the changes into the two modes the intrinsic's understand,
     */

    if (XtIsWidget (w))
    {
        if (size->request_mode & (CWX | CWY))
        {
	    int newx, newy;
	    if (size->request_mode & CWX) 
	        newx = size->x;
	    else
	        newx = w->core.x;

	    if (size->request_mode & CWY)
	        newy = size->y;
	    else
	        newy = w->core.y;

	    XtMoveWidget (w, newx, newy);
	}

        if (size->request_mode & (CWWidth | CWHeight | CWBorderWidth))
        {
	   if (size->request_mode & CWWidth)       RWidth  (w) = size->width;
	   if (size->request_mode & CWHeight)	   RHeight (w) = size->height;
	   if (size->request_mode & CWBorderWidth) RBorder (w) = size->border_width;

 	   XtResizeWindow (w);
	}
    }
    else
    {
	/* have to clear the old gadget area and then clear the new
	 * gadget area so it gets updated correctly
	 */

	Widget pw = XtParent (w);
        RectObj r = (RectObj) w;
        int bw2   = RBorder (r) << 1;

        while ((pw != NULL) && ( ! XtIsWidget (pw)))
            pw = XtParent (pw);

        if ((pw != NULL) && XtIsRealized (pw)) 

            XClearArea (XtDisplay (pw), XtWindow (pw),
                    RX (r), RY (r), RWidth (r) + bw2, RHeight (r) + bw2, TRUE);

	if (size->request_mode & CWX)           RX      (r) = size->x;
	if (size->request_mode & CWY)           RY      (r) = size->y;
	if (size->request_mode & CWWidth)       RWidth  (r) = size->width;
	if (size->request_mode & CWHeight)	RHeight (r) = size->height;

	if (size->request_mode & CWBorderWidth) 
	{
	    RBorder (r) = size->border_width;
            bw2 = RBorder (r) << 1;
	}

        if ((pw != NULL) && XtIsRealized (pw)) 

            XClearArea (XtDisplay (pw), XtWindow (pw),
                    RX (r), RY (r), RWidth (r) + bw2, RHeight (r) + bw2, TRUE);
    }
}




/* this routine will call the geom mgr. to try a resize */
/* it will return the geom mgr return                   */

#ifdef _NO_PROTO
XtGeometryResult DXmMakeGeometryRequest(w,geom)
    Widget w;
    XtWidgetGeometry *geom;
#else
XtGeometryResult DXmMakeGeometryRequest(Widget w, XtWidgetGeometry *geom)
#endif
{ 
    XtWidgetGeometry allowed;
    XtGeometryResult answer = XtGeometryNo;

    /*
     *  Ask geometry manager
     */
    answer = XtMakeGeometryRequest (w, geom, &allowed);

    if ( answer == XtGeometryAlmost )
    {
	/*
	 * take what he recommended, don't fight it
	 */
	*geom = allowed;	
        answer = XtMakeGeometryRequest (w, geom, &allowed);

        if ( answer == XtGeometryAlmost )
	{
	    /* something's screwy, just say no
	    */
	    answer = XtGeometryNo;
	}
    }
    return answer;
}



/*
 * widget SetValues convenience proc for handling string changes.  Updates
 * pointers structures and returns TRUE if external format changes.
 */

#ifdef _NO_PROTO
Boolean
DXmStringCheck (old_external, new_external)
    XmString 	*old_external, *new_external;
#else
Boolean
DXmStringCheck (XmString *old_external, XmString *new_external)
#endif
{
    if (*old_external != *new_external)
    {
	if (*old_external)
	    XtFree((char *)*old_external);

	*new_external = XmStringCopy(*new_external);

	return (TRUE);
    }
    
    return (FALSE);		
}


/*
 *  Convenience routines to access a composite widget's children.
 */

#ifdef _NO_PROTO
Cardinal  DXmNumChildren(w)
CompositeWidget w;
#else
Cardinal  DXmNumChildren(CompositeWidget w)
#endif
{
    return (XtNumChildren(w));
}

#ifdef _NO_PROTO
WidgetList DXmChildren(w)
CompositeWidget w;
#else
WidgetList DXmChildren(CompositeWidget w)
#endif
{
    return (XtChildren(w));
}


#ifdef _NO_PROTO
void DXmActivateWidget(w)
Widget w;
#else
void DXmActivateWidget(Widget w)
#endif
{
    if (XmIsPushButton(w))
    	DXmActivatePBWidget(w);
    else if (XmIsPushButtonGadget(w))
    	DXmActivatePBGadget(w);
}


/* 
 * This routine is a NO-OP; It exists to have a routine which
 * can be called to load the Xm shareable without any
 * side effect.
 */
#ifdef _NO_PROTO
void _DXmLoadShareable()
#else
void _DXmLoadShareable(void)
#endif
{
    return;
}

/* 
 * Called from a widget's Realize routine, returns FALSE if the widget 
 * cannot fit in the screen (I14Y: low-res support)
 */
#ifdef _NO_PROTO
Boolean _DXmCheckFit(w, height_clipped, width_clipped)
Widget w;
Dimension *height_clipped;
Dimension *width_clipped;
#else
Boolean _DXmCheckFit(Widget w, Dimension *height_clipped, Dimension *width_clipped)
#endif
{
    /* 
     * design note (i.e. hack):  height does not include window manager title 
     * bar (if any).  "Ideal" solution would be to dynamically determine if
     * the widget is a popup with a title bar and, if so, add the title bar 
     * height to the widget height.  This could be accomplished by comparing 
     * the y position of the widget shell against that of the window manager 
     * window.  The resulting delta y would indicate (1) if there was a title 
     * bar and (2) the height of the bar in pixels.  Unfortunately, currently
     * the only mechanism for obtaining the widget X window parent is calling 
     * XQueryTree, which would introduce a significant performance hit.
     * Since running on a low-res pc screen is relatively rare, play it safe by
     * always assuming popup w/title and hard-code 20 pixel height.  
     *
     * Jay Bolgatz  Jun 90
     */

    Screen  *screen;
    Dimension width, height;
    Dimension tb_height = 20;

    screen = (Screen *) XtScreen(w);
    width  = (Dimension) XtWidth(w)  + XtBorderWidth(w);
    height = (Dimension) XtHeight(w) + XtBorderWidth(w) + tb_height;

    if (WidthOfScreen(screen) < width)
	*width_clipped = (Dimension) (width - WidthOfScreen(screen)
				      + (WidthOfScreen(screen)/100));
    else
	*width_clipped = (Dimension) (0);

    if (HeightOfScreen(screen) < height)
	*height_clipped = (Dimension) (height - HeightOfScreen(screen)
				       + (HeightOfScreen(screen)/100));
    else
	*height_clipped = (Dimension) (0);

    if (*width_clipped == 0 && *height_clipped == 0)
	return (Boolean) (TRUE);
    else
	return (Boolean) (FALSE);
}


/*
************************************************************************
* Font Fallback routines
*
************************************************************************
*/

#ifdef _NO_PROTO
char *DXmFindFontFallback(fontname)
    char *fontname;
#else
char *DXmFindFontFallback(char *fontname)
#endif
{

    return ((char *)_Xt_FontFallback(fontname));

}


#ifdef _NO_PROTO
XFontStruct *DXmLoadQueryFont(d, fontname)
    Display *d;
    char    *fontname;
#else
XFontStruct *DXmLoadQueryFont(Display *d, char    *fontname)
#endif

{

    return ((XFontStruct *)_Xt_LoadQueryFont(d, fontname));

}





/************************************************************************/
/*									*/
/* _DXmCvtCStoI								*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Converts CS to integer.						*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	CS		The compound string.				*/
/*									*/
/*	size		Size of the convert from CS to FC.		*/
/*									*/
/*	status		Status of the convert.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	The integer equivalent of the CS.				*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
int _DXmCvtCStoI(ar_cs,al_size,al_status)
    XmString	ar_cs;
    long	*al_size;
    long	*al_status;
#else
int _DXmCvtCStoI(XmString ar_cs, long *al_size, long *al_status)
#endif
{
    Opaque	ar_converted_value;
    int		l_value = 0;
    Boolean	asn1;
    XmString	savestring;

    /********************************************************************/
    /*									*/
    /* If the CS is NULL, return a value of 0.				*/
    /*									*/
    /********************************************************************/

    if (!ar_cs || ((!(asn1 = _is_asn1(ar_cs))) && !_is_compound(ar_cs)))
    {
	*al_size = 0;
	*al_status = (long) DXmCvtStatusFail;
	return(0);
    }

    /* If cs isn't an asn.1 conformant string, convert it first. */
    if (!asn1) 
    {
       savestring = ar_cs;
       ar_cs = XmStringCopy(savestring);
    }
    
    /********************************************************************/
    /*									*/
    /* Convert the CS to asciz.  If the asciz string is NULL, return	*/
    /* a value of 0, otherwise, perform an atoi on the string.		*/
    /*									*/
    /********************************************************************/
    ar_converted_value = (Opaque) DXmCvtCStoFC(ar_cs,al_size,al_status);

    if (!asn1)
    {
        XmStringFree(ar_cs);
        ar_cs = savestring;
    }
   
    if (!ar_converted_value)
    {
	*al_size = 0;
	return(0);
    }
    
    l_value = atoi(ar_converted_value);
    
    /********************************************************************/
    /*									*/
    /* Free the space allocated and return the integer value.		*/
    /*									*/
    /********************************************************************/
    XtFree(ar_converted_value);
    
    return(l_value);
    
} /* _DXmCvtCStoI */


/************************************************************************/
/*									*/
/* _DXmCvtItoCS								*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Converts integer to CS.						*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	i		The integer.					*/
/*									*/
/*	size		Size of the convert from FC to CS.		*/
/*									*/
/*	status		Status of the convert.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	The CS equivalent of the integer.				*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
XmString _DXmCvtItoCS(l_i,al_size,al_status)
    int	l_i;
    long	*al_size;
    long	*al_status;
#else
XmString _DXmCvtItoCS(int l_i, long *al_size, long *al_status)
#endif
{
    char 	at_temp_buf[20];
    
    /********************************************************************/
    /*									*/
    /* Convert the integer to asciz, then perform an FC to CS on it.	*/
    /*									*/
    /********************************************************************/
    sprintf(at_temp_buf,"%d",l_i);
    
    return (XmString) DXmCvtFCtoCS( (Opaque) at_temp_buf,al_size,al_status);

} /* _DXmCvtItoCS */











/*
 ************************************************************************
 *
 * I18n and compound string conversion code
 */





/*
 *************************************************************************
 *
 * Provide locale sensitive Motif compound string version of toolkit 
 * ASCII default text values such as "OK", "Cancel" etc.
 */

#ifdef _NO_PROTO
XmString
DXmGetLocaleString (context, ascii, word_type)
    I18nContext  context;
    char * ascii;
    I18nWordType word_type;
#else
XmString
DXmGetLocaleString (I18nContext context, char * ascii, I18nWordType word_type)
#endif
{
/* I18N START */
	return ( (XmString) XmGetLocaleString ( context, ascii, word_type ) );
/* I18N END */
}




/*
 *************************************************************************
 *
 * Convert Motif Compound String into IText widget internal format
 */

/* TBS */



/*
 *************************************************************************
 *
 * Convert IText widget internal format into Motif Compound String
 */

/* TBS */

/* I18N START */

/*
 *************************************************************************
 *
 * Convert Motif compound string into 'file code' format.  
 */


#ifdef _NO_PROTO
Opaque
DXmCvtCStoFC (cs, byte_count, status)
    XmString cs;
    long * byte_count;
    long * status;
#else
Opaque
DXmCvtCStoFC (XmString cs, long * byte_count, long * status)
#endif
{
    Opaque  fc;
    int     count;
    int     ret_status;
    Boolean	asn1;
    XmString	savestring;

    if (!cs || ((!(asn1 = _is_asn1(cs))) && !_is_compound(cs)))
    {
	*byte_count = 0;
	*status = (long) DXmCvtStatusFail;
	return ((Opaque) NULL);
    }

    /* If cs isn't an asn.1 conformant string, convert it first. */
    if (!asn1) 
    {
       savestring = cs;
       cs = XmStringCopy(savestring);
    }

    /*
     *  The _Xm converter use int instead of long.  Also, existing
     *  code assumes that fc is null-terminating.
     */
    
    fc = (Opaque) _XmCvtCStoFC(cs, &count, &ret_status);

    if (!asn1)
    {
        XmStringFree(cs);
        cs = savestring;
    }

    if ( ret_status == DXmCvtStatusFail || count < 0 )
    {
        if (fc) XtFree(fc);
	*byte_count = 0;
	*status = (long) ret_status;
	return ((Opaque) NULL);
    } else {
	fc = XtRealloc(fc, count + 1);
	((char *)fc)[count] = '\0';
    
	*byte_count = (long) count;
	*status = (long) ret_status;
    
	return (fc);
    }
}




/*
 *************************************************************************
 *
 * Convert 'file code' text into Motif compound string
 *
 * need richer ability to have fc build multi-segemnt cs's
 */

#ifdef _NO_PROTO
XmString
DXmCvtFCtoCS (fc, byte_count, status)
    Opaque fc;
     long * byte_count;
     long * status;
#else
XmString
DXmCvtFCtoCS (Opaque fc, long * byte_count, long * status)
#endif
{
    XmString  cs;
    int       ret_status;
    
    /*
     *  The _Xm converter uses a different interface.  Do the
     *  conversions here.
     */
    
    if (fc)
	cs = (XmString) _XmCvtFCtoCS(fc, (int *) strlen(fc), &ret_status);
    else
	ret_status = DXmCvtStatusFail;
    
    if ( ret_status == DXmCvtStatusFail )
    {
	*byte_count = 0;
	*status = (long) ret_status;

	return ((XmString) NULL);

    } else {

	*byte_count = (long) XmStringLength(cs);
	*status = (long) ret_status;
    
	return (cs);
    }
}








/*
 *************************************************************************
 *
 * Convert Motif compound string to operating system dependent string
 * representation
 */

#ifdef _NO_PROTO
Opaque
DXmCvtCStoOS (cs, byte_count, status)
    XmString cs;
    long * byte_count;
    long * status;
#else
Opaque
DXmCvtCStoOS (XmString cs, long * byte_count, long * status)
#endif
{
    Opaque  os;
    int     count;
    int     ret_status;
    Boolean	asn1;
    XmString	savestring;

    if (!cs || ((!(asn1 = _is_asn1(cs))) && !_is_compound(cs)))
    {
	*byte_count = 0;
	*status = (long) DXmCvtStatusFail;
	return ((Opaque) NULL);
    }

    /* If cs isn't an asn.1 conformant string, convert it first. */
    if (!asn1) 
    {
       savestring = cs;
       cs = XmStringCopy(savestring);
    }
   
    /*
     *  The _Xm converter use int instead of long.  Also, existing
     *  code assumes that os is null-terminating.
     */
    
    os = (Opaque) _XmCvtCStoOS((XmString) cs, &count, &ret_status);

    if (!asn1)
    {
        XmStringFree(cs);
        cs = savestring;
    }

    if ( ret_status != DXmCvtStatusOK || count < 0 )
    {
	if (os) XtFree(os);
	*byte_count = 0;
	*status = (long) ret_status;

	return ( (Opaque) NULL );

    } else {

	os = XtRealloc(os, count + 1);
	((char *)os)[count] = '\0';

	*byte_count = (long) count;
	*status = (long) ret_status;
    
	return (os);
    }
}







/*
 *************************************************************************
 *
 * Convert operating system dependent string representation to Motif
 * compound string
 */

#ifdef _NO_PROTO
XmString
DXmCvtOStoCS (os_string, byte_count, status)
    Opaque os_string;
    long * byte_count;
    long * status;
#else
XmString
DXmCvtOStoCS (Opaque os_string, long * byte_count, long * status)
#endif
{
    XmString  cs;
    int       ret_status;
    
    /*
     *  The _Xm converter uses a different interface.  Do the
     *  conversion here.
     */
    
    if ( os_string )
        cs = (XmString) _XmCvtOStoCS(os_string, (int *) strlen(os_string), &ret_status);
    else
	ret_status = DXmCvtStatusFail;

    if ( ret_status == DXmCvtStatusFail )
    {
	*byte_count = 0;
	*status = (long) DXmCvtStatusFail;

	return( (XmString) NULL );
    } else {

	*byte_count = (long) XmStringLength(cs);
	*status = (long) ret_status;
    
	return (cs);
    }
}

/* I18N END */





/*
 *************************************************************************
 *
 * Convert a Motif compound string into .UIL syntax, used by VUIT
 */

/* TBS */








/*
Mike,

1. I am using the constant XmSTRING_DIRECTION_REVERT which I assume is defined
   in Xm.h .   (simething like: #define XmSTRING_DIRECTION_REVERT 2 )
  
*
* Author/Date: Moshe Loterman, 4-Jul-1990
*
* Modifications:
*
*/

#ifdef malloc
#undef malloc
#endif
#define malloc XtMalloc

#ifdef free
#undef free
#endif
#define free XtFree


/***************************************************************************
 * The following is a charcter set tables based on DEC STD 169. This table *
 * is used for DDIF documents which have no character set table in the     *
 * header.
 ***************************************************************************/

#define DEC_STD_169_MAX_CODE 65

static char *DEC_STD_169_id[] = { 
 "","ISO8859-1","ISO8859-2","ISO8859-6","ISO8859-7","","ISO8859-8","","","",
 "","","","","","","","","","",
 "","","","","","","","","","",
 "","","JISX0201.1976-0","DEC-DECTech","DEC-DECMath_Italic",
     "DEC-DECMath_Symbol","DEC-DECMATH_Extension","DEC-DECPub","","",
 "","","","","","","","","","",
 "","","","","","","","","","",
 "","","","","JISX0208.1983-1","GB2312.1980-0" };

static const unsigned long DEC_STD_169_len[] = {
 0,9,9,9,9,0,9,0,0,0,
 0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,
 0,0,15,11,18,
     18,21,10,0,0,
 0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,15,13};

/***************************************************************************
 * The following nesting directive define as bit per value                 *
 * DIR_SAME means no directive, DIR_PUSH and DIR_POP means perform the     *
 * directive. DIR_PUSH|DIR_POP is also valid.                              *
 ***************************************************************************/

#define DIR_SAME 0
#define DIR_PUSH 1
#define DIR_POP 2


/***********************************************************************
* Opaque DXmCvtCStoDDIF(CS,ByteCount,Status)
*
* The routine convert a Motif Compound String to DDIF structure
* Input: CS - XmString 
*        ByteCount -  Return by the routine.
*        Status - Return by the routine. 
* Output: Return pointer to Opaque
*
* The routine allocate the space which is required to hold the DDIF string.
* The user is expected to free up the space by using XmFree()
***********************************************************************/



#ifdef _NO_PROTO
Opaque DXmCvtCStoDDIF (cs, byte_count, status)
XmString cs;
long *byte_count;
long *status;
#else
Opaque DXmCvtCStoDDIF (XmString cs, long *byte_count, long *status)
#endif
{
  unsigned char *Ptr;
  struct ctx_ddif Ctx, *CtxP = &Ctx;
  Opaque *Tmp;
  int	non_std_cs = 0;
  int	count, found;
  XmStringContext	context;
  char			*text;
  XmStringCharSet	charset;
  XmStringDirection	direction;
  Boolean		separator, asn1;
  XmString		savestring;

  CtxP->Size = 0;

  /* Check to see if valid Motif Compound String. If not return error to user */
  if (!cs || ((!(asn1 = _is_asn1(cs))) && !_is_compound(cs)))
  {
	*status = DXmCvtStatusFail;
	return( (Opaque)0 );
  }

  if (!asn1) 
  {
       savestring = cs;
       cs = XmStringCopy(savestring);
  }

  Ptr = (unsigned char *) cs;

  CtxP->Size = _read_string_length(Ptr);

  CtxP->Ptr = Ptr + _read_header_length(Ptr); /* Save pointer to the next char to proccess */
  CtxP->CharSetLen = 0;
  CtxP->CharSet = (unsigned char *)0;

  /* Check whether CS has a charset which is not in DEC_STD_169_id[]
   * If all charsets in CS are in DEC_STD_169_id[], create DDIF without CCS,
   * i.e. use the default charset table in DDIF.
   * DDIF will have a private CCS only if CS has unknown charset.
   * This is necessary because charsets like DECkanji are in standard charset
   * table, and thus Japanese CDA toolkit does not recognize DECkanji if
   * DECkanji text is encoded with private CCS.
   */
  non_std_cs = 0;

  if( XmStringInitContext ( &context, cs ) ){
    while( XmStringGetNextSegment ( context, &text, &charset,
					&direction, &separator )
         && (!non_std_cs)){
      found = 0;
      for(count=0; count<DEC_STD_169_MAX_CODE+1; count++){
	if(!strcmp ( charset, DEC_STD_169_id[count] ) ){
	  found = 1;
	  break;
	}
      }

      if (!found)
	non_std_cs = 1;

      XtFree( text );
      XtFree( charset );
    }

    XmStringFreeContext( context );

  }
  if ( (*status = ddif_encode (byte_count, (unsigned char **)&Tmp,
                               (char *) CtxP,MCS_rout,
						non_std_cs)) )
  {
	*byte_count = 0;
	*status = DXmCvtStatusFail;
	if (!asn1)
	{
	    XmStringFree(cs);
	    cs = savestring;
	}

	return (Opaque)0;
  }
  else {
	*status = DXmCvtStatusOK;
	if (!asn1)
	{
	    XmStringFree(cs);
	    cs = savestring;
	}
	return ((Opaque)Tmp);
  }
}



#ifdef _NO_PROTO
XmString DXmCvtDDIFtoCS (ddif,size,return_status)
  Opaque ddif;
  long *size;
  long *return_status;
#else
XmString DXmCvtDDIFtoCS (Opaque ddif, long *size, long *return_status)
#endif
{
  unsigned char *Tmp, *real_stream;
  unsigned long TotalSize, length;
  struct hunk_desc	default_content_hunk;
  struct hunk_desc	*content_hunk,*next_hunk;
  struct ctx_cs Ctx, *CtxP = &Ctx;

/* Init hunk storage */
  /* Set up the default content hunk. */
  default_content_hunk.next = 0;
  default_content_hunk.alloc_size = 0;
  default_content_hunk.remain_size = hunk_default_size;
  default_content_hunk.used_size = hunk_default_size;
  CtxP->content_hunk = &default_content_hunk;
  CtxP->content_ptr = &CtxP->content_hunk->data[0];
 

/* Init last CharSet used */
  CtxP->CharSetLen = 0;

/* find size of DDIFPtr (Header size (3) + Length)  */
  {
    unsigned char  q;
    unsigned char *p = (unsigned char *)ddif;
    unsigned long i,j;

    p += 3;	/* Skip Tag and DDIF Id */

    q = *p;
    if (q <= 127) length = (unsigned long) q + 3 + 1;
    else {
         q = q & 0x7f;
         if (q == 0) length = 0;
         else {
            length = 0;
            j = (unsigned long) q;
            if ((j > 4) || (j < 1))
                return ((XmString) 1);
            for (i = 0; i < j; i++)
            {
                 p++;
                 q = *p;
                 length <<= 8;                                         
                 length += (unsigned long) q;
            }
            length += 3 + 1 + j;
         }
     }
  }

/* If error in ddif_decode, deallocate space and return whatever we have */

  if (ddif_decode(length,ddif,(char *) CtxP,DDIF_frag,DDIF_nl,DDIF_dir))
  {
    *size = 0;
    *return_status = DXmCvtStatusFail;
    content_hunk = &default_content_hunk;
    real_stream = content_hunk->data;
    while (content_hunk) {
      real_stream += content_hunk->used_size;
      next_hunk = content_hunk->next;
      if ( content_hunk->alloc_size) free((char *)content_hunk);
      content_hunk = next_hunk;
    }
    return((XmString)0);
  }
  else *return_status = DXmCvtStatusOK;

  content_hunk = CtxP->content_hunk;

  /* Tidy up the hunks */
  content_hunk->used_size -= content_hunk->remain_size;

  /* Sum up the content sizes */
  TotalSize = 0;
  content_hunk = &default_content_hunk;
  do {
    TotalSize += content_hunk->used_size;
  } while( (content_hunk=content_hunk->next) );

  *size = TotalSize+((TotalSize > 127)? 3+3 : 3+1);
  if ( !(Tmp = (unsigned char *) malloc(*size)) ) {
    *return_status = DXmCvtStatusFail;
    *size = 0;
    content_hunk = &default_content_hunk;
    real_stream = content_hunk->data;
    while (content_hunk) {
      real_stream += content_hunk->used_size;
      next_hunk = content_hunk->next;
      if ( content_hunk->alloc_size) free((char *)content_hunk);
      content_hunk = next_hunk;
    }
    return((XmString)0);
  }
  real_stream = Tmp;
  
  *real_stream++ = 0XDF;        /* Motif Compound String Header */
  *real_stream++ = 0XFF;        
  *real_stream++ = 0X79;

  if ( TotalSize > 127) {
  *real_stream++ = 0X82;      /* Length of Length field + ext bit */        
  *real_stream++ = TotalSize >> 8;
  *real_stream++ = TotalSize & 0XFF;
  }
  else  *real_stream++ = TotalSize;

/* Copy rest of string */

  content_hunk = &default_content_hunk;
  while (content_hunk) {
    memcpy(real_stream, &content_hunk->data[0], content_hunk->used_size);
    real_stream += content_hunk->used_size;
    next_hunk = content_hunk->next;
    if ( content_hunk->alloc_size) free((char *)content_hunk);
    content_hunk = next_hunk;
  }
  return ((XmString)Tmp);
}





/*
 * CS <-> DDIF support code
 * 
 * Note that the compound string operated upon must be asn1 compliant.  The DXm routines
 * that call these routines (like DXmCvtCSToDDIF) perform the conversion to asn1 compliancy
 * if neccesary.
 */


#ifdef _NO_PROTO
static unsigned long MCS_rout(CtxP,CharSetLen,CharSet,FragLen,Frag,
                        nl,Direction,Nesting)
  struct ctx_ddif *CtxP; 
  long *CharSetLen;
  unsigned char **CharSet;
  long *FragLen;
  unsigned char **Frag;
  unsigned long *nl;
  unsigned long *Direction;
  unsigned long *Nesting;
#else
static unsigned long MCS_rout(
  struct ctx_ddif *CtxP, 
  long *CharSetLen,
  unsigned char **CharSet,
  long *FragLen,
  unsigned char **Frag,
  unsigned long *nl,
  unsigned long *Direction,
  unsigned long *Nesting)
#endif
{
  unsigned long CharSetFound;
  unsigned char Tag;
  long length;
    
/* Init all lengths to 0 */
 
  *CharSetLen = 0;
  *FragLen = 0;
  *nl = 0;

  
  *Nesting = DIR_SAME;  /* Assume neither PUSH nor POP */

/* Get the next TAG and update remaining size */
  if (CtxP->Size <= 0)
     return (1);
  Tag = *CtxP->Ptr;
  length = _read_asn1_length (CtxP->Ptr);

  do {
    switch (Tag) {

     case XmSTRING_COMPONENT_TEXT:
       /* get next 2 bytes as length */
       *FragLen = length;
       *Frag = CtxP->Ptr + _asn1_size(length);	    /* Set address of Text String */
       CtxP->Ptr += _asn1_size(length) + length;    /* Update currect location */
       CtxP->Size -= _asn1_size(length) + length;   /* Update remaining size */

       *CharSetLen = CtxP->CharSetLen;		    /* Restore last saved CharSet */
       *CharSet = CtxP->CharSet;

       CharSetFound = 0;
       break;

     case XmSTRING_COMPONENT_LOCALE_TEXT:
       /*
       ** Added for Motif 1.2.  CS 5/20/93
       ** Treat this case the same as XmSTRING_COMPONENT_TEXT but use the
       ** default fontlist tag as the character set.
       */

       /* get next 2 bytes as length */
       *FragLen = length;
       *Frag = CtxP->Ptr + _asn1_size(length);	    /* Set address of Text String */
       CtxP->Ptr += _asn1_size(length) + length;    /* Update currect location */
       CtxP->Size -= _asn1_size(length) + length;   /* Update remaining size */

       *CharSetLen = strlen(XmFONTLIST_DEFAULT_TAG);
       *CharSet = (unsigned char *) XmFONTLIST_DEFAULT_TAG;

       CtxP->CharSetLen = *CharSetLen;		    /* Update last used CharSet */
       CtxP->CharSet = *CharSet;

       CharSetFound = 0;
       break;

     case XmSTRING_COMPONENT_DIRECTION:
       /* next 2 bytes are length which must be 1 */
       CtxP->Ptr += _asn1_size(length);
       switch (*CtxP->Ptr) {
         case XmSTRING_DIRECTION_L_TO_R: 
         case XmSTRING_DIRECTION_R_TO_L:        
           *Nesting = DIR_PUSH;
           *Direction = *CtxP->Ptr;	/*Direction is privous byte */
           break;
         case XmSTRING_DIRECTION_REVERT:        
           *Nesting = DIR_POP;
           break;
         default: /* Do nothing this is unknown direction */
           break;
       }
       CtxP->Ptr += length;
       CtxP->Size -= _asn1_size(length) + length;
       CharSetFound = 0;
       break;

     case XmSTRING_COMPONENT_SEPARATOR:
       /* next 2 bytes are length which must be 0 */
       *nl = 1; /* Set new line directive */
       CtxP->Ptr += _asn1_size(length) + length;
       CtxP->Size -= _asn1_size(length) + length;
       CharSetFound = 0;
       break;

     case XmSTRING_COMPONENT_CHARSET:
       *CharSetLen = length ;
       *CharSet = CtxP->Ptr + _asn1_size(length);

       CtxP->CharSetLen = *CharSetLen; /* Update last used CharSet */
       CtxP->CharSet = *CharSet;
       CtxP->Ptr += _asn1_size(length) + length;
       CtxP->Size -= _asn1_size(length) + length;

/* Get the next TAG and update remaining size */
       Tag = *CtxP->Ptr;
       length = _read_asn1_length (CtxP->Ptr);
       CharSetFound = 1;

       break;

     default: /* Found uknown segment. ignore it and continue */
       /* get next 2 bytes as length */
       {
         CtxP->Ptr += _asn1_size(length) + length; /* Update currect location */
         CtxP->Size -= _asn1_size(length) + length; /* Update remaining size */
       }
       CharSetFound = 0;
       break;

    }

  } while ( CharSetFound && (CtxP->Size > 0) );

/* If end of string then signal to stop proccessing */

  if( CtxP->Size < 0 ) return (1);
  else return(0);
}



#ifdef _NO_PROTO
static unsigned long DDIF_frag(CtxP,CharSetLen,CharSet,FragLen,FragPtr,Dir)
  struct ctx_cs *CtxP;
  unsigned long CharSetLen;
  unsigned char *CharSet;
  unsigned long FragLen;
  unsigned char *FragPtr;
  unsigned long Dir;
#else
static unsigned long DDIF_frag(
  struct ctx_cs *CtxP,
  unsigned long CharSetLen,
  unsigned char *CharSet,
  unsigned long FragLen,
  unsigned char *FragPtr,
  unsigned long Dir)
#endif
{
  unsigned long real_length,NewCharSet;

/* Check if we have a new character set. 
 		   If yes insert a character set segment */
  NewCharSet = 0;
  real_length = FragLen + 3 ;
  if (CtxP->CharSetLen != CharSetLen ||
                strncmp((char *) CtxP->CharSet,(char *) CharSet,CharSetLen)) {
    NewCharSet++;
    CtxP->CharSetLen = CharSetLen;
    CtxP->CharSet = CharSet;
    real_length += CharSetLen + 3;
  }

/* Allocate hunk block if needed */
  CtxP->content_hunk->remain_size -= real_length;
  if (CtxP->content_hunk->remain_size < 0 ) {
    CtxP->content_hunk = _new_hunk(CtxP->content_hunk,real_length);
    if (!CtxP->content_hunk) return (1);
    CtxP->content_ptr = &CtxP->content_hunk->data[0];
  }

  if (NewCharSet) {
    *CtxP->content_ptr++ = XmSTRING_COMPONENT_CHARSET;
    *CtxP->content_ptr++ = CharSetLen & 0XFF;/*Small Indians Vs. Little Indians*/
    *CtxP->content_ptr++ = CharSetLen >> 8;/*Small Indians Vs. Little Indians*/
    memcpy(CtxP->content_ptr,CharSet,CharSetLen);
    CtxP->content_ptr += CharSetLen;
  }

  *CtxP->content_ptr++ = XmSTRING_COMPONENT_TEXT;
  *CtxP->content_ptr++ = FragLen & 0XFF;/*Small Indians Vs. Little Indians*/
  *CtxP->content_ptr++ = FragLen >> 8;/*Small Indians Vs. Little Indians*/
  memcpy(CtxP->content_ptr,FragPtr,FragLen);
  CtxP->content_ptr += FragLen;

  return (0);
}

#ifdef _NO_PROTO
static unsigned long DDIF_nl(CtxP)
  struct ctx_cs *CtxP;
#else
static unsigned long DDIF_nl(struct ctx_cs *CtxP)
#endif
{
  CtxP->content_hunk->remain_size -= 3;
  if (CtxP->content_hunk->remain_size < 0 ) {
    CtxP->content_hunk = _new_hunk(CtxP->content_hunk,3);
    if (!CtxP->content_hunk) return (1);
    CtxP->content_ptr = &CtxP->content_hunk->data[0];
  }

  *CtxP->content_ptr++ = XmSTRING_COMPONENT_SEPARATOR;
  *CtxP->content_ptr++ = 0;
  *CtxP->content_ptr++ = 0;

  return (0);
}


#ifdef _NO_PROTO
static unsigned long DDIF_dir(CtxP,dir)
  struct ctx_cs *CtxP;
  unsigned long dir;
#else
static unsigned long DDIF_dir(
  struct ctx_cs *CtxP,
  unsigned long dir)
#endif
{
  CtxP->content_hunk->remain_size -= 3+1;
  if (CtxP->content_hunk->remain_size < 0 ) {
    CtxP->content_hunk = _new_hunk(CtxP->content_hunk,3+1);
    if (!CtxP->content_hunk) return (1);
    CtxP->content_ptr = &CtxP->content_hunk->data[0];
  }

  *CtxP->content_ptr++ = XmSTRING_COMPONENT_DIRECTION;
  *CtxP->content_ptr++ = 1;/*Small Indians Vs. Little Indians*/
  *CtxP->content_ptr++ = 0;/*Small Indians Vs. Little Indians*/
  *CtxP->content_ptr++ = dir;

  return (0);
}


/*
* ddif_enc_dec.c
*
* Simple DDIS(tm)-encoded DDIF(tm) encode/decode routines.
*
* Interesting #define statements that can be used:
*
*	#define malloc your-favorite-malloc-routine
*	#define free   your-favorite-free-routine
*
*	#define DDIF_PROD_IDENT_LEN n
*	#define DDIF_PROD_IDENT_CHR 'X', 'Y', ...
*	[These establish the DDIF product ident.  The "n" is the  ]
*	[length of the ident and "'X', 'Y', ..." is the ident     ]
*	[itself.  The default is the three character ident "CDA". ]
*
*	#define NULL_STRINGS 1
*	[Allow encode/decode of null (zero length) strings. ]
*	[The default is to completely discard null strings. ]
*
* Author/Date: Mark H. Bramhall, 24-Apr-1989
*
* Modifications:
*
* 4-July-1990, Moshe Loterman,
*                   
*                   Modification to handle nesting of segments.
*
* 22-Mar-1990, MHB, Fix misunderstanding between the outside world and
*		    DDIF on the values to be used for the direction codes.
*		    DDIF always uses 1 for LtoR and 2 for RtoL. But, Motif
*		    and DWT use 0 for LtoR and 1 for RtoL. So, the routine
*		    must do the mapping between these code spaces. But,
*		    this was discovered after DDIF V1.1 had been used for
*		    a while, so this routine must specially handle DDIF V1.1
*		    encodings and, only for DDF V1.1, not do the transform.
*		    The DDIF version number is bumped to V1.2 to indicate
*		    we are creating valid DDIF encodings.
*
* 28-Jun-1989, MHB, Fix long form tag length handling.
*
* 27-Apr-1989, MHB, Add null string capability.
*/


/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
/*#define malloc XtMalloc*/		/*REMOVERD*/
/*#define free XtFree*/                 /*REMOVERD*/

#define NULL_CHARSET 1			/* Insert ChsrSet to CharSet table
					   in DDIF header regardless if
      					   there is a text using it      */
/*************************************************************************/
#define DDIF_PROD_IDENT_LEN 3
#define DDIF_PROD_IDENT_CHR 'D', 'W', 'T'
#define NULL_STRINGS 1

/*
**************************************************************************
**  COPYRIGHT (c) 1990, 1990 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
**  ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**************************************************************************
*/

/* Default our compilation parameters. */
#ifndef DDIF_PROD_IDENT_LEN
#define DDIF_PROD_IDENT_LEN 3
#define DDIF_PROD_IDENT_CHR 'C', 'D', 'A'
#endif

#ifndef DDIF_PROD_IDENT_CHR
#define DDIF_PROD_IDENT_LEN 3
#define DDIF_PROD_IDENT_CHR 'C', 'D', 'A'
#endif

#ifndef NULL_STRINGS
#define NULL_STRINGS 0
#endif

/* Because I like to explicitly state signed or unsigned! */
#ifndef signed
#define signed
#endif

/* Define the fields of a tag. */
#define TAG_M_SCOPE           0xC0
#define TAG_M_FORM            0x20
#define TAG_M_ID              0x1F

/* Define the scopes. */
#define TAG_SCOPE_PRIVATE     0xC0
#define TAG_SCOPE_CONTEXT     0x80
#define TAG_SCOPE_APPLICATION 0x40
#define TAG_SCOPE_UNIVERSAL   0x00

/* Define the forms. */
#define TAG_FORM_CONSTRUCTOR  0x20
#define TAG_FORM_PRIMITIVE    0x00

/* Define the extended id. */
#define TAG_ID_EXTENDED       0x1F

/* A commonly used character set. */
static const unsigned char	latin_1[] = "ISO8859-1";


/* Get the next tag. */
#ifdef _NO_PROTO
static unsigned long	_next_tag (cnt_, ptr_, info_)
    signed long		*cnt_;
    unsigned char	**ptr_;
    struct tag_info	*info_;
#else
static unsigned long	_next_tag (
    signed long		*cnt_,
    unsigned char	**ptr_,
    struct tag_info	*info_)
#endif
{
unsigned char		*ptr;
struct tag_info		*info;
signed long		ext_id;
signed long		nxt_id;
signed long		len_len;

    /* Dereference pointer; get info struct pointer. */
    ptr = *ptr_;
    info = info_;

    /* Check for virtual EOC. */
    if (ptr == info->eoc)
	goto tag_eoc;

    /* Get the tag. */
    if (--*cnt_ < 0)
	goto tag_error;
    info->tag = *ptr++;

    /* Process extended id if needed. */
    if (!(TAG_ID_EXTENDED & (~info->tag)))
	{
	ext_id = 0;
	for (;;)
	    {
	    if (--*cnt_ < 0)
		goto tag_error;
	    ext_id <<= 7;
	    nxt_id = *((signed char *)ptr++);
	    ext_id |= nxt_id & 127;
	    if (nxt_id >= 0)
		break;
	    }
	if (ext_id < TAG_ID_EXTENDED)
	    {
	    info->tag &= TAG_M_SCOPE | TAG_M_FORM;
	    info->tag |= ext_id;
	    ext_id = 0;
	    }
	ext_id <<= 8;
	info->tag |= ext_id;
	}

    /* Get the length. */
    if (--*cnt_ < 0)
	goto tag_error;
    info->len = *((signed char *)ptr++);

    /* Process indefinite or multi-byte lengths. */
    if (info->len < 0)
	{
	len_len = info->len & 127;
	if (!len_len)
	    {
	    /* Indefinite length; must not be a primitive. */
	    if ((info->tag & TAG_M_FORM) == TAG_FORM_PRIMITIVE)
		goto tag_error;
	    goto tag_eoc_check;
	    }
	info->len = 0;
	for (;;)
	    {
	    if (--*cnt_ < 0)
		goto tag_error;
	    info->len <<= 8;
	    info->len |= *ptr++;
	    if (--len_len <= 0)
		break;
	    }
	}

    /* Validate length. */
    if (info->len > *cnt_)
	goto tag_error;

    /* EOC must be primitive and length = 0. */
tag_eoc_check:
    if ((info->tag & (TAG_M_SCOPE | TAG_M_ID)) != (TAG_SCOPE_UNIVERSAL | 0))
	goto tag_exit;
    if (info->tag == (TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0))
	if (!info->len)
	    goto tag_exit;

    /* Some error has occured... */
tag_error:
    info->error = 1;

    /* Set the tag to EOC. */
tag_eoc:
    info->tag = TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0;
    info->len = 0;

tag_exit:
    /* Update pointer and exit. */
    *ptr_ = ptr;
    return info->error;
}

/* Skip over the current tag. */
#ifdef _NO_PROTO
static unsigned long	_skip_tag (cnt_, ptr_, info_)
    signed long		*cnt_;
    unsigned char	**ptr_;
    struct tag_info	*info_;
#else
static unsigned long	_skip_tag (
    signed long		*cnt_,
    unsigned char	**ptr_,
    struct tag_info	*info_)
#endif
{
struct tag_info		*info;
unsigned char		*old_eoc;
signed long		lvl;

    /* Get info struct pointer. */
    info = info_;

    /* Non-indefinite lengths are easy. */
    if (info->len >= 0)
	{
	*ptr_ += info->len;
	*cnt_ -= info->len;
	if (*cnt_ < 0)
	    goto skip_error;
	goto skip_exit;
	}

    /* Save old virtual EOC; init level and no virtual EOC. */
    old_eoc = info->eoc;
    lvl = 0;
    info->eoc = 0;

    /* Scan for the matching EOC. */
    for (;;)
	{
	if (_next_tag (cnt_, ptr_, info))
	    goto skip_exit;
	if (info->tag == (TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0))
	    {
	    if (--lvl < 0)
		break;
	    }
	else
	    if (info->len < 0)
		lvl += 1;
	    else
		{
		*ptr_ += info->len;
		*cnt_ -= info->len;
		if (*cnt_ < 0)
		    goto skip_error;
		}
	}

    /* Restore old virtual EOC. */
    info->eoc = old_eoc;
    goto skip_exit;

    /* Some error has occured... */
skip_error:
    info->error = 1;

    /* Exit. */
skip_exit:
    return info->error;
}

/*
* ddif_decode -- Decode simple DDIS encoded DDIF.
*
* Parameters:
*
*   cnt		Length, in bytes, of the DDIS stream (0 => indefinite).
*   ptr		Address of the DDIS stream.
*   ctx		Context to be passed to the call out routines.
*   rout_frg	Call out routine for a new text fragment.
*   rout_nl	Call out routine for a new line.
***************************************************************************
********* Modified ML July-4-1990 *****************************************
*   rout_nest	Call out routine for nesting.
***************************************************************************
*
* Return value:
*
*   Zero	Success.
*   Non-zero	Failure.
*
* New text fragment call out routine:
*
*   ctx		Context.
*   cs_len	Length, in bytes, of character set id.
*   cs_id	Address of character set id.
*   len		Length, in bytes, of text fragment.
*   ptr		Address of text fragment.
*   dir		Direction (0 => LtoR, 1 => RtoL). !@#$ LULU - is it needed??
*
*   Zero return is success; non-zero is failure.
*
* New line call out routine:
*
*   ctx		Context.
*
*   Zero return is success; non-zero is failure.
*
***************************************************************************
********* Modified ML July-4-1990 *****************************************
* Nesting call out routine:
*
*   ctx		Context.
*   nesting directive (0=>XmSTRING_DIRECTION_L_TO_R 
*               1=>XmSTRING_DIRECTION_R_TO_L 2=>XmSTRING_DIRECTION_REVERT)
*
*   Zero return is success; non-zero is failure.
***************************************************************************
*/
#ifdef _NO_PROTO
static unsigned long		ddif_decode (org_cnt, ptr, ctx, rout_frg, rout_nl,
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
                                      rout_nest)
/*************************************************************************/
    signed long		org_cnt;
    unsigned char	*ptr;
    char		*ctx;
    unsigned long	(*rout_frg)();
    unsigned long	(*rout_nl)();
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
    unsigned long	(*rout_nest)();
/*************************************************************************/
#else
static unsigned long		ddif_decode (
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
/*************************************************************************/
    signed long		org_cnt,
    unsigned char	*ptr,
    char		*ctx,
    unsigned long	(*rout_frg)(),
    unsigned long	(*rout_nl)(),
    unsigned long	(*rout_nest)())
#endif
{
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
/*#define MAX_EOC_LVL 10	*/     			/*DELETED*/
/*#define MAX_SEG_LVL 10	*/     			/*DELETED*/
#define MAX_NESTING 25
/*************************************************************************/

struct tag_info		info;
signed long		eoc_lvl;
signed long		seg_lvl;
unsigned long		ddif_v11;
signed long		max_cs;
signed long		cnt;
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
/*unsigned char		*eocs[MAX_EOC_LVL];	*/       /*DELETED*/
unsigned char		*eocs[MAX_NESTING];
/*unsigned long		direction[MAX_SEG_LVL];	*/       /*DELETED*/
unsigned long		direction[MAX_NESTING];

long NoCharSetTable = 0;        /* Assume DDIF has CharSet Table        */
/*************************************************************************/
signed long		cs_len[256];
unsigned char		*cs_id[256];
signed long		cs;
    
    /* Init things. */
    info.error = 0;	/* No error. */
    info.eoc   = 0;	/* No virtual EOC. */
    eoc_lvl    = 0;	/* Outermost level. */
    seg_lvl    = 0;	/* Outermost level. */
    ddif_v11   = 0;	/* Not DDIF V1.1. */
    max_cs     = -1;	/* No character sets. */

    /* Set up our working count. */
    cnt = org_cnt;
    if (!cnt)
	cnt = 0x7FFFFFFF;

    /* Default settings: Direction = LtoR. */
    direction[seg_lvl] = 0;

    /* Get the outermost tag. */
    /* It must be [PRIVATE 16383], the DDIF domain tag. */
    if (_next_tag (&cnt, &ptr, &info))
	goto decode_exit;
    if (info.tag != (TAG_SCOPE_PRIVATE
		   | TAG_FORM_CONSTRUCTOR
		   | TAG_ID_EXTENDED
		   | (16383 << 8)))
	goto decode_error;

    /* Push virtual EOC context. */
    eocs[eoc_lvl] = info.eoc;
    eoc_lvl += 1;
    info.eoc = 0;
    if (info.len >= 0)
	info.eoc = ptr + info.len;

    /* This must be [0], the DDIF descriptor tag. */
    if (_next_tag (&cnt, &ptr, &info))
	goto decode_exit;
    if (info.tag != (TAG_SCOPE_CONTEXT | TAG_FORM_CONSTRUCTOR | 0))
	goto decode_error;

    /* Push virtual EOC context. */
    eocs[eoc_lvl] = info.eoc;
    eoc_lvl += 1;
    info.eoc = 0;
    if (info.len >= 0)
	info.eoc = ptr + info.len;

    /* Get next tag; must be [0], DDIF major version. */
    if (_next_tag (&cnt, &ptr, &info))
	goto decode_exit;
    if (info.tag != (TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 0))
	goto decode_exit;

    /* Major version must be 1 (i.e., some V1.*). */
    if (info.len != 1)
	goto decode_error;
    if (*ptr != 1)
	goto decode_error;

    /* Skip this tag. */
    if (_skip_tag (&cnt, &ptr, &info))
	goto decode_exit;

    /* Get next tag; must be [1], DDIF minor version. */
    if (_next_tag (&cnt, &ptr, &info))
	goto decode_exit;
    if (info.tag != (TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 1))
	goto decode_exit;

    /* DDIF V1.1 has to be specially decoded. */
    if (info.len != 1)
	goto decode_error;
    if (*ptr == 1)
	ddif_v11 = 1;

    /* Skip this tag. */
    if (_skip_tag (&cnt, &ptr, &info))
	goto decode_exit;

    /* Skip the rest of the document descriptor. */
    for (;;)
	{
	/* Get next tag; finished if EOC. */
	if (_next_tag (&cnt, &ptr, &info))
	    goto decode_exit;
	if (info.tag == (TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0))
	    break;

	/* Skip this tag. */
	if (_skip_tag (&cnt, &ptr, &info))
	    goto decode_exit;
	}

    /* Pop virtual EOC context. */
    eoc_lvl -= 1;
    info.eoc = eocs[eoc_lvl];

    /* This must be [1], the DDIF header tag. */
    if (_next_tag (&cnt, &ptr, &info))
	goto decode_exit;
    if (info.tag != (TAG_SCOPE_CONTEXT | TAG_FORM_CONSTRUCTOR | 1))
	goto decode_error;

    /* Push virtual EOC context. */
    eocs[eoc_lvl] = info.eoc;
    eoc_lvl += 1;
    info.eoc = 0;
    if (info.len >= 0)
	info.eoc = ptr + info.len;

    /* Scan the document header. */
    for (;;)
	{
	/* Get next tag; finished if EOC. */
	if (_next_tag (&cnt, &ptr, &info))
	    goto decode_exit;
	if (info.tag == (TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0))
	    break;

	/* Look for [APPLICATION 0], character set id table. */
	if (info.tag == (TAG_SCOPE_APPLICATION | TAG_FORM_CONSTRUCTOR | 0))
	    {
	    /* Push virtual EOC context. */
	    eocs[eoc_lvl] = info.eoc;
	    eoc_lvl += 1;
	    info.eoc = 0;
	    if (info.len >= 0)
		info.eoc = ptr + info.len;

	    /* Scan the character set ids. */
	    for (;;)
		{
		/* Get next tag; finished if EOC. */
		if (_next_tag (&cnt, &ptr, &info))
		    goto decode_exit;
		if (info.tag == (TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0))
		    break;

		/* Count another table entry. */
		if (++max_cs > 255)
		    goto decode_error;
		cs_len[max_cs] = 0;
		cs_id[max_cs] = 0;

		/* Set up the table entry. */
		switch (info.tag)
		    {
		    /* [0], empty. */
		    case TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 0 :
			break;

		    /* [1], ? octets/character id. */
		    case TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 1 :
		    /* [2], 1 octet/character w/ ASCII in GL id. */
		    case TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 2 :
		    /* [3], 1 octet/character id. */
		    case TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 3 :
		    /* [4], 2 octets/character id. */
		    case TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 4 :
		    /* [5], 4 octets/character id. */
		    case TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 5 :
			cs_len[max_cs] = info.len;
			cs_id[max_cs] = ptr;
			break;

		    default:
			goto decode_error;
		    };

		/* Skip this tag. */
		if (_skip_tag (&cnt, &ptr, &info))
		    goto decode_exit;
		}

	    /* Pop virtual EOC context. */
	    eoc_lvl -= 1;
	    info.eoc = eocs[eoc_lvl];
	    }
	else
	    /* Skip this tag. */
	    if (_skip_tag (&cnt, &ptr, &info))
		goto decode_exit;
	}

    /* Pop virtual EOC context. */
    eoc_lvl -= 1;
    info.eoc = eocs[eoc_lvl];

    /* Need a default character set id table? */
    if (max_cs < 0)
	{
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
/*	cs_len[0] = 0;                          */              /*DELETED*/
/*	cs_id[0] = 0;                           */              /*DELETED*/
/*	cs_len[1] = sizeof (latin_1) - 1;       */              /*DELETED*/
/*	cs_id[1] = &latin_1[0];                 */              /*DELETED*/
/*	max_cs = 1;                             */              /*DELETED*/

        NoCharSetTable = 1;        /* DDIF has no CharSet Table        */
	max_cs = DEC_STD_169_MAX_CODE;
/*************************************************************************/

	}

    /* This must be [2], the DDIF content tag. */
    if (_next_tag (&cnt, &ptr, &info))
	goto decode_exit;
    if (info.tag != (TAG_SCOPE_CONTEXT | TAG_FORM_CONSTRUCTOR | 2))
	goto decode_error;

    /* Push virtual EOC context. */
    eocs[eoc_lvl] = info.eoc;
    eoc_lvl += 1;
    info.eoc = 0;
    if (info.len >= 0)
	info.eoc = ptr + info.len;

    /* Scan the document content. */
    for (;;)
	{
	/* Get next tag; finished if EOC. */
	if (_next_tag (&cnt, &ptr, &info))
	    goto decode_exit;
	if (info.tag == (TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0))
	    break;

	/* Dispatch [APPLICATION ??] tags. */
	if ((info.tag & TAG_M_SCOPE) == TAG_SCOPE_APPLICATION)
	    {
	    switch (info.tag & TAG_M_ID)
		{
		/* [APPLICATION 1], EOS. */
		case 1 :
		    /* Back out of a segment level. */
		    if (--seg_lvl < 0)
			goto decode_error;
/*************************************************************************/
/******** Modified ML July-4-1990 ****************************************/
                    if ( (*rout_nest)(ctx,XmSTRING_DIRECTION_REVERT) ) return 1;
/*************************************************************************/
		    goto skip_over_tag;

		/* [APPLICATION 2], SEG. */
		case 2 :
		    /* Go into a new segment level. */
		    if (++seg_lvl >= MAX_NESTING)
			goto decode_error;

		    /* Propagate outer segment settings. */
		    direction[seg_lvl] = direction[seg_lvl - 1];

		    /* Push virtual EOC context. */
		    eocs[eoc_lvl] = info.eoc;
		    eoc_lvl += 1;
		    info.eoc = 0;
		    if (info.len >= 0)
			info.eoc = ptr + info.len;

		    /* Scan the segment begin. */
		    for (;;)
			{
			/* Get next tag; finished if EOC. */
			if (_next_tag (&cnt, &ptr, &info))
			    goto decode_exit;
			if (info.tag == (TAG_SCOPE_UNIVERSAL
				       | TAG_FORM_PRIMITIVE
				       | 0))
			    break;

			/* Look for [3], segment attributes. */
			if (info.tag == (TAG_SCOPE_CONTEXT
				       | TAG_FORM_CONSTRUCTOR
				       | 3))
			    {
			    /* Push virtual EOC context. */
			    eocs[eoc_lvl] = info.eoc;
			    eoc_lvl += 1;
			    info.eoc = 0;
			    if (info.len >= 0)
				info.eoc = ptr + info.len;

			    /* Scan the segment attributes. */
			    for (;;)
				{
				/* Get next tag; finished if EOC. */
				if (_next_tag (&cnt, &ptr, &info))
				    goto decode_exit;
				if (info.tag == (TAG_SCOPE_UNIVERSAL
					       | TAG_FORM_PRIMITIVE
					       | 0))
				    break;

				/* Look for [18], text attributes. */
				if (info.tag == (TAG_SCOPE_CONTEXT
					       | TAG_FORM_CONSTRUCTOR
					       | 18))
				    {
				    /* Push virtual EOC context. */
				    eocs[eoc_lvl] = info.eoc;
				    eoc_lvl += 1;
				    info.eoc = 0;
				    if (info.len >= 0)
					info.eoc = ptr + info.len;

				    /* Scan the text attributes. */
				    for (;;)
					{
					/* Get next tag; finished if EOC. */
					if (_next_tag (&cnt, &ptr, &info))
					    goto decode_exit;
					if (info.tag == (TAG_SCOPE_UNIVERSAL
						       | TAG_FORM_PRIMITIVE
						       | 0))
					    break;

					/* Look for [5], text direction. */
					if (info.tag == (TAG_SCOPE_CONTEXT
						       | TAG_FORM_PRIMITIVE
						       | 5))
					    {
					    /* Set new text direction. */
					    if (info.len != 1)
						goto decode_error;
					    direction[seg_lvl] = *ptr & 1;
					    if (!ddif_v11)
						direction[seg_lvl] ^= 1;
					    }

					/* Skip this tag. */
					if (_skip_tag (&cnt, &ptr, &info))
					    goto decode_exit;
					}

				    /* Pop virtual EOC context. */
				    eoc_lvl -= 1;
				    info.eoc = eocs[eoc_lvl];
				    }
				else
				    /* Skip this tag. */
				    if (_skip_tag (&cnt, &ptr, &info))
					goto decode_exit;
				}

			    /* Pop virtual EOC context. */
			    eoc_lvl -= 1;
			    info.eoc = eocs[eoc_lvl];
			    }
			else
			    /* Skip this tag. */
			    if (_skip_tag (&cnt, &ptr, &info))
				goto decode_exit;
			}

		    /* Pop virtual EOC context. */
		    eoc_lvl -= 1;
		    info.eoc = eocs[eoc_lvl];
/*************************************************************************/
/******** Modified ML July-4-1990 ****************************************/
                    if ((*rout_nest)(ctx,direction[seg_lvl])) return 1;
/*************************************************************************/
		    break;

		/* [APPLICATION 3], TXT. */
		case 3 :
		    /* Insist on the primitive form. */
		    if ((info.tag & TAG_M_FORM) != TAG_FORM_PRIMITIVE)
			goto decode_error;

		    /* Call out with this piece of text. */
#if (!NULL_STRINGS)
		    if (info.len)
#endif
			{
			info.error = (*rout_frg)(ctx,
						 sizeof (latin_1) - 1,
						 &latin_1[0],
						 info.len,
						 ptr,
						 direction[seg_lvl]);
			if (info.error)
			    goto decode_error;
			}
		    goto skip_over_tag;

		/* [APPLICATION 4], GTX. */
		case 4 :
		    /* Insist on the primitive form. */
		    if ((info.tag & TAG_M_FORM) != TAG_FORM_PRIMITIVE)
			goto decode_error;

		    /* Insist on a character set selector. */
		    if (--info.len < 0)
			goto decode_error;
		    cnt -= 1;
		    cs = *ptr++;
		    if ( cs >= 0x40 ){
			/* strange encoding of charset */
			unsigned char byte1, byte2;
			byte1 = (unsigned char) cs;
			byte2 = *ptr++;
			cs = (long) (byte2 << 6) + (byte1 & 0x3F);
			cnt -= 1;
			info.len -= 1;
		    }
		    if (cs > max_cs)
			goto decode_error;
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
		    if (!((NoCharSetTable)?DEC_STD_169_len[cs]:cs_len[cs]))
/*************************************************************************/
			goto decode_error;

		    /* Call out with this piece of text. */
#if (!NULL_STRINGS)
		    if (info.len)
#endif
		     	{
			info.error = (*rout_frg)(ctx,
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
                           ((NoCharSetTable)?DEC_STD_169_len[cs]:cs_len[cs]),
                           ((NoCharSetTable)?(unsigned long)DEC_STD_169_id[cs]
                                            :(unsigned long)cs_id[cs]),
/*************************************************************************/
			   info.len,
			   ptr,
			   direction[seg_lvl]);
			if (info.error)
			    goto decode_error;
			}
		    goto skip_over_tag;

		/* [APPLICATION 9], HRD. */
		case  9 :
		/* [APPLICATION 10], SFT. */
		case 10 :
		    /* Look for primitive form of the new-line(2) directive. */
		    if ((info.tag & TAG_M_FORM) == TAG_FORM_PRIMITIVE)
			if (info.len == 1)
			    if (*ptr == 2)
				{
				/* Call out for the new line. */
				info.error = (*rout_nl)(ctx);
				if (info.error)
				    goto decode_error;
				}
		    goto skip_over_tag;

		default :
		    goto skip_over_tag;
		};
	    }
	else
	    {
	    /* Skip this tag. */
skip_over_tag:
	    if (_skip_tag (&cnt, &ptr, &info))
		goto decode_exit;
	    }
	}

    /* Pop virtual EOC context. */
    eoc_lvl -= 1;
    info.eoc = eocs[eoc_lvl];

    /* The next tag must be the EOC for the DDIF domain. */
    if (_next_tag (&cnt, &ptr, &info))
	goto decode_exit;
    if (info.tag != (TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0))
	goto decode_error;

    /* Pop virtual EOC context. */
    eoc_lvl -= 1;
    info.eoc = eocs[eoc_lvl];

    /* We must have popped out of all segment levels by now. */
    if (seg_lvl)
	goto decode_error;

    /* The stream count must be exhausted by now. */
    if (org_cnt)
	if (cnt)
	    goto decode_error;
    goto decode_exit;

    /* Some error has occured... */
decode_error:
    info.error = 1;

    /* Exit. */
decode_exit:
    return info.error;
}

/* Define the hunk structures. */
#define hunk_default_size 128


/**************************************************************************
********* Modified ML July-4-1990 *****************************************
struct hunk_desc					****DELETED****
    {							****DELETED****
    struct hunk_desc	*next;				****DELETED****
    signed long		alloc_size;                     ****DELETED****
    signed long		remain_size;                    ****DELETED****
    signed long		used_size;			****DELETED****
    unsigned char	data[hunk_default_size];	****DELETED****
    };							****DELETED****
**************************************************************************/

/* Allocate a new hunk. */
#ifdef _NO_PROTO
static struct hunk_desc	*_new_hunk (prv_hunk, size)
    struct hunk_desc	*prv_hunk;
    signed long		size;
#else
static struct hunk_desc	*_new_hunk (
    struct hunk_desc	*prv_hunk,
    signed long		size)
#endif
{
signed long		remain_size;
signed long		alloc_size;
struct hunk_desc	*nxt_hunk;

    /* Correct remaining size; calculate used size. */
    prv_hunk->remain_size += size;
    prv_hunk->used_size -= prv_hunk->remain_size;

    /* Find size of new hunk. */
    remain_size = size;
    if (remain_size < hunk_default_size)
	remain_size = hunk_default_size;
    alloc_size = sizeof (struct hunk_desc) - hunk_default_size + remain_size;

    /* Allocate the new hunk. */
    nxt_hunk = (struct hunk_desc *)malloc (alloc_size);
    if (nxt_hunk)
	{
	/* Set up new hunk. */
	nxt_hunk->next = 0;
	nxt_hunk->alloc_size = alloc_size;
	nxt_hunk->remain_size = remain_size - size;
	nxt_hunk->used_size = remain_size;

	/* Link new hunk to previous hunk. */
	prv_hunk->next = nxt_hunk;
	}

    return nxt_hunk;
}

/*
* ddif_encode -- Encode simple DDIS encoded DDIF.
*
* Parameters:
*
*   cnt		Address for length, in bytes, of the DDIS stream (0 => none).
*   ptr		Address for address of the DDIS stream.
*   ctx		Context to be passed to the call out routines.
*   rout	Call out routine for next fragment.
*
* Return value:
*
*   Zero	Success.
*   Non-zero	Failure.
*
* Call out routine:
*
*   ctx		Context.
*   cs_len	Address for length, in bytes, of character set id.
*   cs_id	Address for address of character set id.
*   len		Address for length, in bytes, of text fragment.
*   ptr		Address for address of text fragment.
*   nl		Address for new line boolean.
*   direction	Address for direction (0 => LtoR, 1 => RtoL).
***************************************************************************
********* Modified ML July-4-1990 *****************************************
*   nesting     Address for nesting directive (DIR_PUSH, DIR_POP,         *
*					      DIR_SAME, DIR_PUSH|DIR_POP) *
***************************************************************************
*
*   Zero return is "do it"; non-zero is "end".
*/
#ifdef _NO_PROTO
static unsigned long		ddif_encode (cnt, ptr, ctx, rout, non_std_cs)
    signed long		*cnt;                         
    unsigned char	**ptr;
    char		*ctx;
    unsigned long	(*rout)();
    int			non_std_cs;
#else
static unsigned long		ddif_encode (
    signed long		*cnt,
    unsigned char	**ptr,
    char		*ctx,
    unsigned long	(*rout)(),
    int			non_std_cs)
#endif
{
static const unsigned char	descriptor[] = {
    /* [0], descriptor, length = "4 tags". */
    TAG_SCOPE_CONTEXT | TAG_FORM_CONSTRUCTOR | 0,
    3 + 3 + (2 + DDIF_PROD_IDENT_LEN) + 2,
	/* [0], major version, length = 1. */
	TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 0,
	1,
	    /* Major version = 1. */
	    1,
	/* [1], minor version, length = 1. */
	TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 1,
	1,
	    /* Minor version = 2. */
	    2,
	/* [2], product identifier, length = DDIF_PROD_IDENT_LEN. */
	TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 2,
	DDIF_PROD_IDENT_LEN,
	    /* Product identifier = ???. */
	    DDIF_PROD_IDENT_CHR,
	/* [3], product name, length = 0. */
	TAG_SCOPE_CONTEXT | TAG_FORM_CONSTRUCTOR | 3,
	0};

unsigned long		error;
signed long		max_cs;
unsigned long		non_latin_1;
struct hunk_desc	default_header_hunk;
struct hunk_desc	*header_hunk;
unsigned char		*header_ptr;
struct hunk_desc	default_content_hunk;
struct hunk_desc	*content_hunk;
unsigned char		*content_ptr;
signed long		real_len;
signed long		new_cs_len;
unsigned char		*new_cs_id;
signed long		new_frg_len;
unsigned char		*new_frg;
unsigned long		new_nl;
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
/*unsigned long		new_direction;	*/        /*DELETED*/
unsigned long		direction;	
/*************************************************************************/
signed long		cs;
signed long		cs_len[256];
unsigned char		*cs_id[256];
unsigned long		cs_non_latin_1[256];
signed long		header_size;
signed long		header_extra;
signed long		content_size;
signed long		content_extra;
signed long		domain_size;
signed long		domain_extra;
signed long		total_size;
unsigned char		*real_stream;
struct hunk_desc	*next_hunk;
int			std_cs_number;

/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
unsigned long nesting, nesting_stack[MAX_NESTING] ;
long nesting_level = 0 ;
/*************************************************************************/

    /* Init things. */
    error       = 0;
    max_cs      = -1;
    non_latin_1 = 0;

    /* Set up the default header hunk. */
    default_header_hunk.next = 0;
    default_header_hunk.alloc_size = 0;
    default_header_hunk.remain_size = hunk_default_size;
    default_header_hunk.used_size = hunk_default_size;
    header_hunk = &default_header_hunk;
    header_ptr = &header_hunk->data[0];

    /* Set up the default content hunk. */
    default_content_hunk.next = 0;
    default_content_hunk.alloc_size = 0;
    default_content_hunk.remain_size = hunk_default_size;
    default_content_hunk.used_size = hunk_default_size;
    content_hunk = &default_content_hunk;
    content_ptr = &content_hunk->data[0];

    /* Until we're given no more... */
    for (;;)
	{
	new_cs_len    = 0;
	new_cs_id     = 0;
	new_frg_len   = 0;
	new_frg       = 0;
	new_nl        = 0;
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
/*	new_direction = direction;	*/    	/*DELETED*/
        nesting = DIR_SAME;
/*************************************************************************/
	if ((*rout)(ctx,
		    &new_cs_len, &new_cs_id,
		    &new_frg_len, &new_frg,
		    &new_nl,
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
/*		    &new_direction,	*/   	/*DELETED*/
		    &direction,
/*************************************************************************/
		    &nesting))
	    break;

/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
	/* Verify arguments. */		
/*	if (direction & (~1))			*/	/*DELETED*/
/*     	    goto encode_error;			*/	/*DELETED*/

	/* If there is a text fragment to process... */
/*#if (!NULL_STRINGS)				*/	/*DELETED*/
/*	if (new_frg_len)			*/	/*DELETED*/
/*#else						*/	/*DELETED*/
/*	if ((new_cs_len) || (new_frg_len))	*/	/*DELETED*/
/*#endif					*/	/*DELETED*/

	/* If there is a character set fragment to process... */
#if (NULL_CHARSET)
	if (new_cs_len)
#else
	if ((new_cs_len) || (new_frg_len))
#endif

/*************************************************************************/
	    {
	    /* Determine the character set index. */
	    if ((new_cs_len <= 0) || (new_cs_len > 127))
		goto encode_error;
	    cs = -1;
	    for (;;)
		{
		cs += 1;
		if (cs > max_cs)
		    {
		    /* Set up a new character set. */
		    real_len = 1 + 1 + new_cs_len;
		    header_hunk->remain_size -= real_len;
		    if (header_hunk->remain_size < 0)
			{
			header_hunk = _new_hunk (header_hunk, real_len);
			if (!header_hunk)
			    goto encode_error;
			header_ptr = &header_hunk->data[0];
			}
		    *header_ptr++ = TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 1;
		    *header_ptr++ = new_cs_len;

		    cs_len[cs] = new_cs_len;
		    cs_id[cs] = header_ptr;

		    memcpy (header_ptr, new_cs_id, new_cs_len);
		    header_ptr += new_cs_len;

		    cs_non_latin_1[cs] = 0;
		    if ((new_cs_len != (sizeof (latin_1) - 1))
		     || (strncmp ((char *) new_cs_id, (char *) &latin_1[0], sizeof (latin_1) - 1)))
			{
			non_latin_1 = 1;
			cs_non_latin_1[cs] = 1;
			}

		    max_cs = cs;
		    break;
		    }

		if (cs_len[cs] == new_cs_len)
		    if (!strncmp ((char *) cs_id[cs], (char *) new_cs_id, new_cs_len))
			break;
		}
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
            }
        /* Set a new direction if needed. */            /*DELETED*/ 
/*      if (new_direction != direction)		*/   	/*DELETED*/ 
        if (nesting & DIR_PUSH)
	    {
              if ( nesting_level == 0 || 
			nesting_stack[nesting_level-1] != direction)
              {		/* Do Push and Direction */
/*************************************************************************/

  	        content_hunk->remain_size -=
		  1 + 1 + (1 + 1 + (1 + 1 + (1 + 1 + (1))));
	        if (content_hunk->remain_size < 0)
	          {
		    content_hunk = _new_hunk (content_hunk,
		          1 + 1 + (1 + 1 + (1 + 1 + (1 + 1 + (1)))));
		    if (!content_hunk) goto encode_error;
		    content_ptr = &content_hunk->data[0];
		  }

	        *content_ptr++ = 
			TAG_SCOPE_APPLICATION | TAG_FORM_CONSTRUCTOR | 2;
	        *content_ptr++ = 1 + 1 + (1 + 1 + (1 + 1 + (1)));
	        *content_ptr++ = TAG_SCOPE_CONTEXT | TAG_FORM_CONSTRUCTOR | 3;
	        *content_ptr++ = 1 + 1 + (1 + 1 + (1));
	        *content_ptr++ = TAG_SCOPE_CONTEXT | TAG_FORM_CONSTRUCTOR | 18;
	        *content_ptr++ = 1 + 1 + (1);
	        *content_ptr++ = TAG_SCOPE_CONTEXT | TAG_FORM_PRIMITIVE | 5;
	        *content_ptr++ = 1;

/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
/*	        *content_ptr++ = new_direction + 1;	*/       /*DELETED*/
	        *content_ptr++ = direction + 1;
	      }
	      else {	/* Do Push only */

  	        content_hunk->remain_size -= 1 + 1;
	        if (content_hunk->remain_size < 0)
	          {
		    content_hunk = _new_hunk (content_hunk, 1 + 1);
		    if (!content_hunk) goto encode_error;
		    content_ptr = &content_hunk->data[0];
		  }
	        *content_ptr++ = 
			TAG_SCOPE_APPLICATION | TAG_FORM_CONSTRUCTOR | 2;
	        *content_ptr++ = 0;
	      }
              nesting_stack[nesting_level++] = direction;
/*************************************************************************/
	    }
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
	/* If there is a text fragment to process... */
#if (!NULL_STRINGS)
	if (new_frg_len)
#else
	if ((new_cs_len) || (new_frg_len))
#endif
            {
/*************************************************************************/
	    /* Ensure space for the fragment. */
	    real_len = 1 + (1 + 2) + (1 + new_frg_len);
	    content_hunk->remain_size -= real_len;
	    if (content_hunk->remain_size < 0)
		{
		content_hunk = _new_hunk (content_hunk, real_len);
		if (!content_hunk)
		    goto encode_error;
		content_ptr = &content_hunk->data[0];
		}

	    /* Start the fragment. */
	    real_len = new_frg_len;
	    if (cs_non_latin_1[cs])
		{
	      if( non_std_cs ){
		real_len += 1;
	      } else {
		int count;

		/* if !non_std_cs, cs_id[cs] is one of DEC_STD_169_id[] */
		std_cs_number = -1;
		for ( count=0; count<DEC_STD_169_MAX_CODE+1; count++ ){
		  if(!strncmp
			( (char *) cs_id[cs], DEC_STD_169_id[count], cs_len[cs] ) )
		    break;
		}
		std_cs_number = count;

		/* charset number encoding is either 1 or 2 byte */
		if ( std_cs_number < 0x40 )
		  real_len += 1;
		else {
		  real_len += 2;
		  content_hunk->remain_size -= 1;
		}
	      }
		*content_ptr++ = TAG_SCOPE_APPLICATION | TAG_FORM_PRIMITIVE | 4;
		}
	    else
		{
		content_hunk->remain_size += 1;
		*content_ptr++ = TAG_SCOPE_APPLICATION | TAG_FORM_PRIMITIVE | 3;
		}

	    /* Set the fragment's length. */
	    if (real_len < 128)
		{
		content_hunk->remain_size += 2;
		*content_ptr++ = real_len;
		}
	    else
		if (real_len < 256)
		    {
		    content_hunk->remain_size += 1;
		    *content_ptr++ = 1 | 128;
		    *content_ptr++ = real_len;
		    }
		else
		    {
		    *content_ptr++ = 2 | 128;
		    *content_ptr++ = real_len >> 8;
		    *content_ptr++ = real_len & 255;
		    }

	    /* Do character set for non-Latin-1 fragments. */
	    if (cs_non_latin_1[cs]){
	      if( non_std_cs ){
		  *content_ptr++ = cs;
	      } else {	/* if !non_std_cs, std_cs_number should exist */
		if ( std_cs_number < 0x40 ){
		  *content_ptr++ = (unsigned char) std_cs_number;
		} else {
		  /* strange encoding of charset */
		  *content_ptr++ = (unsigned char) 0x40 | (0x3F & std_cs_number);
		  *content_ptr++ = (unsigned char) std_cs_number >> 6;
		}
	      }
	    }

	    /* Now the fragment itself. */
	    memcpy (content_ptr, new_frg, new_frg_len);
	    content_ptr += new_frg_len;
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
            }
	/* Close off any new direction. */      	/*DELETED*/
/*      if (new_direction != direction)		*/      /*DELETED*/
        if ( (nesting & DIR_POP) && (nesting > 1) ) /* Ignore Extra REVERT */ 
/*************************************************************************/
	    {
	    content_hunk->remain_size -= 1 + 1;
	    if (content_hunk->remain_size < 0)
		{
		content_hunk = _new_hunk (content_hunk, 1 + 1);
		if (!content_hunk)
			goto encode_error;
		content_ptr = &content_hunk->data[0];
	        }
	    *content_ptr++ = TAG_SCOPE_APPLICATION | TAG_FORM_PRIMITIVE | 1;
	    *content_ptr++ = 0;
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
            direction = nesting_stack[--nesting_level];
/*************************************************************************/
	    }
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
/*	    }					*/         /*DELETED*/
/*************************************************************************/

	/* If there is a new line to process... */
	if (new_nl)
	    {
	    content_hunk->remain_size -= 1 + 1 + 1;
	    if (content_hunk->remain_size < 0)
		{
		content_hunk = _new_hunk (content_hunk, 1 + 1 + 1);
		if (!content_hunk)
		    goto encode_error;
		content_ptr = &content_hunk->data[0];
		}
	    *content_ptr++ = TAG_SCOPE_APPLICATION | TAG_FORM_PRIMITIVE | 9;
	    *content_ptr++ = 1;
	    *content_ptr++ = 2;
	    }
	}

/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
    /* Build as many EOS as needed to balance the structure */
    for (;nesting_level>0;nesting_level--) {
/*************************************************************************/
        /* Build the root segment end. */
        content_hunk->remain_size -= 1 + 1;
        if (content_hunk->remain_size < 0)
	    {
	    content_hunk = _new_hunk (content_hunk, 1 + 1);
	    if (!content_hunk)
	        goto encode_error;
	    content_ptr = &content_hunk->data[0];
	    }
        *content_ptr++ = TAG_SCOPE_APPLICATION | TAG_FORM_PRIMITIVE | 1;
        *content_ptr++ = 0;
/*************************************************************************/
/********* Modified ML July-4-1990 ***************************************/
    }
/*************************************************************************/
    /* Tidy up the hunks. */
    header_hunk->used_size -= header_hunk->remain_size;
    content_hunk->used_size -= content_hunk->remain_size;

    /* Sum up the header sizes. */
    header_size = 0;
    header_extra = 0;
    if (non_latin_1 && non_std_cs)
	{
	header_hunk = &default_header_hunk;
	for (;;)
	    {
	    header_size += header_hunk->used_size;
	    header_hunk = header_hunk->next;
	    if (!header_hunk)
		break;
	    }
	if (header_size > 127)
	    header_size += 1 + 1;
	header_size += 1 + 1;
	if (header_size > 127)
	    header_extra += 1 + 1;
	}

    /* Sum up the content sizes. */
    content_size = 0;
    content_extra = 0;
    content_hunk = &default_content_hunk;
    for (;;)
	{
	content_size += content_hunk->used_size;
	content_hunk = content_hunk->next;
	if (!content_hunk)
	    break;
	}
    if (content_size > 127)
	content_extra += 1 + 1;

    /* Find the domain size. */
    domain_size = sizeof (descriptor)
		+ 1 + 1 + header_size + header_extra
		+ 1 + 1 + content_size + content_extra;
    domain_extra = 0;
    if (domain_size > 127)
	domain_extra += 1 + 1;

    /* Finally, the total size. */
    total_size = (1 + 1 + 1) + 1 + domain_size + domain_extra;

    /* Build the real stream. */
    if (cnt)
	*cnt = total_size;
    *ptr = (unsigned char *)malloc (total_size);
    if (!(*ptr))
	goto encode_error;

    /* Start the domain. */
    real_stream = *ptr;
    *real_stream++ = TAG_SCOPE_PRIVATE | TAG_FORM_CONSTRUCTOR | TAG_ID_EXTENDED;
    *real_stream++ = ((16383 >> (7 * 1)) & 127) | 128;
    *real_stream++ = ((16383 >> (7 * 0)) & 127) |   0;
    if (domain_extra)
	*real_stream++ = 128;
    else
	*real_stream++ = domain_size;

    /* Do the descriptor. */
    memcpy (real_stream, &descriptor[0], sizeof (descriptor));
    real_stream += sizeof (descriptor);

    /* Do the header. */
    *real_stream++ = TAG_SCOPE_CONTEXT | TAG_FORM_CONSTRUCTOR | 1;
    if (header_extra)
	*real_stream++ = 128;
    else
	*real_stream++ = header_size;
    if (header_size)
	{
	*real_stream++ = TAG_SCOPE_APPLICATION | TAG_FORM_CONSTRUCTOR | 0;
	real_len = header_size - (1 + 1);
	if (real_len > 127)
	    real_len = 128;
	*real_stream++ = real_len;
	header_hunk = &default_header_hunk;
	for (;;)
	    {
	    memcpy (real_stream, &header_hunk->data[0], header_hunk->used_size);
	    real_stream += header_hunk->used_size;
	    next_hunk = header_hunk->next;
	    if (header_hunk->alloc_size)
		free ((char *)header_hunk);
	    header_hunk = next_hunk;
	    if (!header_hunk)
		break;
	    }
	if (real_len == 128)
	    {
	    *real_stream++ = TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0;
	    *real_stream++ = 0;
	    }
	}
    if (header_extra)
	{
	*real_stream++ = TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0;
	*real_stream++ = 0;
	}

    /* Do the content. */
    *real_stream++ = TAG_SCOPE_CONTEXT | TAG_FORM_CONSTRUCTOR | 2;
    if (content_extra)
	*real_stream++ = 128;
    else
	*real_stream++ = content_size;
    content_hunk = &default_content_hunk;
    for (;;)
	{
	memcpy (real_stream, &content_hunk->data[0], content_hunk->used_size);
	real_stream += content_hunk->used_size;
	next_hunk = content_hunk->next;
	if (content_hunk->alloc_size)
	    free ((char *)content_hunk);
	content_hunk = next_hunk;
	if (!content_hunk)
	    break;
	}
    if (content_extra)
	{
	*real_stream++ = TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0;
	*real_stream++ = 0;
	}

    /* End the domain. */
    if (domain_extra)
	{
	*real_stream++ = TAG_SCOPE_UNIVERSAL | TAG_FORM_PRIMITIVE | 0;
	*real_stream++ = 0;
	}

    goto encode_exit;

    /* Some error has occured... */
encode_error:
    error = 1;

    /* Exit. */
encode_exit:
    return error;
}


/************************************************************************/
/*									*/
/* _DXmCreateWaitCursor							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Creates a wait cursor for the DXm widgets.			*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	w		Widget to create wait cursor for.		*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	A Cursor which means "wait."					*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
Cursor _DXmCreateWaitCursor(ar_w)
    Widget ar_w;
#else
Cursor _DXmCreateWaitCursor(Widget ar_w)
#endif
{
    return (DXmCreateCursor(ar_w, DXm_WAIT_CURSOR));

} /* _DXmCreateWaitCursor */

/************************************************************************/
/*									*/
/* Cursor DXmCreateCursor						*/
/*		Widget	w						*/
/*		int	cursorkind					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Creates a cursor of the specified kind.				*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	w		Widget to obtain display/screen info from	*/
/*	cursorkind	which type of cursor - constants defined in	*/
/*			DECspecific.h					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	A Cursor							*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
Cursor DXmCreateCursor(w, cursorkind)
	Widget	w;
	int	cursorkind;
#else
Cursor DXmCreateCursor(
	Widget	w,
	int	cursorkind)
#endif
{
    Cursor		the_cursor = (Cursor) NULL;
    char *		cursor_name;
    XrmValue            from_val, to_val;
    int status;

    switch (cursorkind) {
	case decw$c_select_cursor:
	    cursor_name = "decw_select_cursor";
	    break;
	case decw$c_help_select_cursor:
	    cursor_name = "decw_help_select_cursor";
	    break;
	case 1:		/* For backward compatibility with old numbers */
	case decw$c_wait_cursor:
	    cursor_name = "decw_wait_cursor";
	    break;
	case 3:		/* For backward compatibility with old numbers */
	case decw$c_inactive_cursor:
	    cursor_name = "decw_inactive_cursor";
	    break;
	case decw$c_resize_cursor:
	    cursor_name = "decw_resize_cursor";
	    break;
	case decw$c_vpane_cursor:
	    cursor_name = "decw_vpane_cursor";
	    break;
	case decw$c_hpane_cursor:
	    cursor_name = "decw_hpane_cursor";
	    break;
	case decw$c_text_insertion_cursor:
	    cursor_name = "decw_text_insertion_cursor";
	    break;
	case decw$c_text_insertion_bl_cursor:
	    cursor_name = "decw_text_insertion_bl_cursor";
	    break;
	case decw$c_cross_hair_cursor:
	    cursor_name = "decw_cross_hair_cursor";
	    break;
	case decw$c_draw_cursor:
	    cursor_name = "decw_draw_cursor";
	    break;
	case decw$c_pencil_cursor:
	    cursor_name = "decw_pencil_cursor";
	    break;
	case decw$c_center_cursor:
	    cursor_name = "decw_center_cursor";
	    break;
	case decw$c_rightselect_cursor:
	    cursor_name = "decw_rightselect_cursor";
	    break;
	case decw$c_wselect_cursor:
	    cursor_name = "decw_wselect_cursor";
	    break;
	case decw$c_eselect_cursor:
	    cursor_name = "decw_eselect_cursor";
	    break;
	case decw$c_x_cursor:
	    cursor_name = "decw_x_cursor";
	    break;
	case decw$c_circle_cursor:
	    cursor_name = "decw_circle_cursor";
	    break;
	case decw$c_mouse_cursor:
	    cursor_name = "decw_mouse_cursor";
	    break;
	case decw$c_lpencil_cursor:
	    cursor_name = "decw_lpencil_cursor";
	    break;
	case decw$c_leftgrab_cursor:
	    cursor_name = "decw_leftgrab_cursor";
	    break;
	case decw$c_grabhand_cursor:
	    cursor_name = "decw_grabhand_cursor";
	    break;
	case decw$c_rightgrab_cursor:
	    cursor_name = "decw_rightgrab_cursor";
	    break;
	case decw$c_leftpointing_cursor:
	    cursor_name = "decw_leftpointing_cursor";
	    break;
	case decw$c_uppointing_cursor:
	    cursor_name = "decw_uppointing_cursor";
	    break;
	case decw$c_rightpointing_cursor:
	    cursor_name = "decw_rightpointing_cursor";
	    break;
	case decw$c_check_cursor:
	    cursor_name = "decw_check_cursor";
	    break;
#ifdef decw$c_questionmark_cursor
	case decw$c_questionmark_cursor:
#else
        case 54:
#endif
	    cursor_name = "decw_questionmark_cursor";
	    break;
	default:
	    cursor_name = "decw_select_cursor";
	} /* end switch */

    /*
     *  Call the string to cursor converter
     */
    from_val.size = strlen(cursor_name) + 1;
    from_val.addr = (XtPointer)cursor_name;
    to_val.addr = (XtPointer)&the_cursor;
    to_val.size = sizeof(Cursor);
    status = XtConvertAndStore(w, XtRString, &from_val, XtRCursor, &to_val);

    return (the_cursor);

} /* DXmCreateCursor */

/************************************************************************/
/*									*/
/* DXmGetLocaleCharset							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Returns the charset string.					*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	Charset string.							*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
XmStringCharSet DXmGetLocaleCharset( )
#else
XmStringCharSet DXmGetLocaleCharset(void)
#endif
{
    return( (XmStringCharSet)I18nGetLocaleCharset((char*)NULL) );

} /* DXmGetLocaleCharset */

/****************************************************************/
/*              This routine should move to I18N layer          */
/****************************************************************/
#define STRING_CHARSET          "ISO8859-1"
#ifdef _NO_PROTO
Boolean
DXmCSContainsStringCharSet( str )
XmString        str ;
/****************
 * Check if all XmString segments contains ISO_LATIN_1.
 * Returns TRUE if it is , FALSE otherwise.
 ****************/
#else
Boolean
DXmCSContainsStringCharSet(XmString str)
#endif
{
            Boolean         retVal ;
            XmStringContext context ;
            char *          text ;
            XmStringCharSet charset ;
            XmStringDirection direction ;
            Boolean         separator ;
/****************/

    retVal = TRUE ;

    if(    XmStringInitContext( &context, str)    )
    {
        while(    XmStringGetNextSegment( context, &text, &charset,
                                                   &direction, &separator)    )
            {
                retVal = (strcmp(charset,STRING_CHARSET) == 0 ? TRUE : FALSE);
                XtFree( text) ;
                XtFree( (char *) charset) ;
                if (!retVal) break;
            }

        XmStringFreeContext( context) ;
        }
    return( retVal) ;
}

#ifdef _NO_PROTO
KeySym  DXmGetLocaleMnemonic(context,w,mnemonic,charset)
I18nContext context;
Widget w;                       /* Widget to help I18N layer to translate */
char * mnemonic;                /* mnemonic string                        */
XmStringCharSet charset;        /* The requested charset of the mnemonic  */
#else
KeySym  DXmGetLocaleMnemonic(
I18nContext context,
Widget w,
char * mnemonic,
XmStringCharSet charset)
#endif
{
    char   *widget_class;
    char   *returned_mnemonic;
    WidgetClass wc;

    if (w == (Widget) NULL)
        return( NoSymbol );

    wc = XtClass(w);
    widget_class = wc->core_class.class_name;

    if (I18nGetLocaleMnemonic(context, widget_class, mnemonic, charset,
                                                &returned_mnemonic))
        return(XStringToKeysym(returned_mnemonic));
    else
        return( NoSymbol );

}

/* New routines added here in order to follow the latest I18n spec
 */

  /*
   * routine DXmGetLocaleCharsets() returns a NULL terminated array of 
   * default char sets according to the locale.  This routine is similar
   * to that of DXmGetLocaleCharset() except that an array is returned instead
   * of one single char set.
   */

#ifdef _NO_PROTO
XmStringCharSet *DXmGetLocaleCharsets( )
#else
XmStringCharSet *DXmGetLocaleCharsets( void )
#endif
{
    return( (XmStringCharSet*)I18nGetLocaleCharsets((char*)NULL) );

} /* DXmGetLocaleCharsets */


  /*
   * routine calls the I18n layer directly to get a default font list built
   */
/* I18N START */
#ifdef _NO_PROTO
XmFontList DXmFontListCreateDefault (widget, resource_name)
Widget widget;
String resource_name;
#else
XmFontList DXmFontListCreateDefault (
Widget widget,
String resource_name)
#endif
{
    return ((XmFontList) XmFontListCreateDefault (widget, resource_name));
}
/* I18N END */



/*
** NOTE - this routine provided by Jim Ferguson (VMS DECwindows).
** Refer any problems to him.
**
** V1.1 BL5 JAF 13-Feb-1992
**  	  - add recompute_size code to work-around possible
**  	     bug in Label Gadget's SetValues method.
** V1.1 update JAF 7-Nov-1991
**        - Replace some XtGetValues/XtSetValues calls with
**          pointers into widget structures for performance.
**        - Use a new algorithm to space the buttons (Fix for
**          I18N QAR 2072 DECW-V3-INTERNAL).
**        - Remove recomputeSize and resizePolicy override
**          settings (Fix for QAR 1549 DECW-V3-INTERNAL).
*/
#ifdef _NO_PROTO
void 
DXmFormSpaceButtonsEqually (parent, widget_list, num_widgets)
    Widget	parent;
    Widget	*widget_list;
    Cardinal	num_widgets;
#else
void
DXmFormSpaceButtonsEqually (
    Widget	parent,
    Widget	*widget_list,
    Cardinal	num_widgets)
#endif
{
    Widget w, *w_ptr;
    Arg arglist[5];
    Dimension max_width = 0;
    int i, fraction_base;
    XmFormWidget fw;
    Boolean reset_recompute;

    if ((!parent) || (!num_widgets) || (!widget_list))
        return;
    if (!XmIsForm (parent))
        return;
    fw = (XmFormWidget) parent;

    /* Get some resource values from the parent.
     */
    fraction_base = fw->form.fraction_base;

    /* Find the widest push button.
     */
    for (i = 0, w_ptr = widget_list; i < num_widgets; i++, w_ptr++) {
        w = *w_ptr;
        if ((w) && (XtWidth(w) > max_width))
            max_width = XtWidth(w);
    }

    /*  Equally space the buttons.
     */
    XtSetArg (arglist[0], XmNwidth, max_width);
    XtSetArg (arglist[1], XmNleftAttachment, XmATTACH_POSITION);
    XtSetArg (arglist[2], XmNleftPosition, 0);
    XtSetArg (arglist[3], XmNleftOffset, 0);
    for (i = 0, w_ptr = widget_list; i < num_widgets; i++, w_ptr++) {
        arglist[2].value = ((i + 1) * fraction_base) / ((int)num_widgets + 1);
        arglist[3].value = ((int)max_width * (i - (int)num_widgets)) /
                                    ((int)num_widgets + 1);

    	/*  Set the value of 'w' and check for empty entries in the 
    	 *  list passed in.
    	 */
        if (w = *w_ptr) {
            reset_recompute = FALSE;

    	    /* Make sure recomputeSize doesn't screw us up while doing SetValues.
    	     */
    	    if (XmIsLabel(w)) {
    	    	XmLabelWidget lw = (XmLabelWidget) w;

    	    	if (lw->label.recompute_size) {
    	    	    lw->label.recompute_size = FALSE;
    	    	    reset_recompute = TRUE;
    	    	}

    	    	XtSetValues (w, arglist, 4);

    	    	if (reset_recompute)
    	    	    lw->label.recompute_size = TRUE;

    	    } else if (XmIsLabelGadget(w)) {

    	    	if (LabG_RecomputeSize(w)) {
    	    	    LabG_RecomputeSize(w) = FALSE;
    	    	    reset_recompute = TRUE;
    	    	}

    	    	XtSetValues (w, arglist, 4);

    	    	if (reset_recompute)
    	    	    LabG_RecomputeSize(w) = TRUE;

    	    } else
    	    	/* The object's not a subclass of LabelWidget or LabelGadget 
    	    	 * so do the SetValues anyways.
    	    	 */
    	    	XtSetValues (w, arglist, 4);
    	}
    }

}   /* end DXmFormSpaceButtonsEqually */
#endif /* not (I18N_EXTENSION && IN_DXMLIBSHR) */

#if defined(I18N_EXTENSION) && defined(IN_XMLIBSHR)
/* 
	Include CSText in decw$xmlibshr
	CSText use DXmCvt* converters, thus DXmMisc.c is required.
	But decw$xmlibshr doesn't have DXm*ClassRec, so some part
	of this file is commented out to create DXmMisc.obj to be
	included in decw$xmlibshr
*/
#else

#ifdef _NO_PROTO
void DXmHelpOnContext (w, confine)
    Widget      w;
    Boolean     confine;
#else
void DXmHelpOnContext (Widget    w,
                              Boolean   confine)
#endif /* _NO_PROTO */

/*
**++
**  Functional Description:
**      Provides Context Sensitive Help
**
**  Arguments:
**      w:              widget id to pass to XmTrackingLocate
**      confine:        boolean to pass to XmTrackingLocate
**                      If true, confine cshelp cursor to 'w' widget.
**
**  Result:
**      None
**
**  Exceptions:
**      None
**--
*/
{
    Widget        track_widget;
    Cursor        cursor;

    cursor = DXmCreateCursor(w, DXm_HELP_CURSOR);

    track_widget = XmTrackingLocate(w, cursor, confine);

    if (track_widget != (Widget)NULL)
        {
        Widget widget;
        for (widget = track_widget; widget != NULL; widget = XtParent(widget))
            {
            if (XtHasCallbacks(widget, XmNhelpCallback) == XtCallbackHasSome)
                {
                XtCallCallbacks(widget, XmNhelpCallback, NULL);
                break;
                }
	    else
	    if ((XtClass(widget) == (WidgetClass)&dxmSvnClassRec) &&
		(XtHasCallbacks(widget, DXmSvnNhelpRequestedCallback) 
					== XtCallbackHasSome) )
		{
		XtCallCallbacks(widget, DXmSvnNhelpRequestedCallback, NULL);
		break;
		}
            }
        }
    else
        {
        /* just revert to normal cursor.                                      */
        /*                                                                    */
        /* When XmTrackingLocate returns a NULL track_widget, it also fails to*/
        /* Ungrab the cursor.  Not sure if that is a bug, but for now we do   */
        /* the Ungrab ourselves.                                              */

        XtUngrabPointer(w, XtLastTimestampProcessed(XtDisplay(w)));
        }
}


#endif /* not (I18N_EXTENSION && IN_XMLIBSHR) */
