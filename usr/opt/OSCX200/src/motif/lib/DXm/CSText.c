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

	      Copyright (c) Digital Equipment Corporation, 1990  
	      All Rights Reserved.  Unpublished rights reserved
	      under the copyright laws of the United States.
	      
	      The software contained on this media is proprietary
	      to and embodies the confidential technology of 
	      Digital Equipment Corporation.  Possession, use,
	      duplication or dissemination of the software and
	      media is authorized only pursuant to a valid written
	      license from Digital Equipment Corporation.

	      RESTRICTED RIGHTS LEGEND   Use, duplication, or 
	      disclosure by the U.S. Government is subject to
	      restrictions as set forth in Subparagraph (c)(1)(ii)
	      of DFARS 252.227-7013, or in FAR 52.227-19, as
	      applicable.

*****************************************************************************
**++
**  FACILITY:
**
**	DECwindows Toolkit
**
**  ABSTRACT:
**
**	International Text Widget Definition
**
**
**  MODIFICATION HISTORY:
**
**	12 APR 1990  Begin rework of old CSText widget code.
**
**	10 MAR 1992  Fix up problem in InitializeHook.  CSTextHasSelection
**		     was not being initialized to FALSE soon enough, and
**		     was being used in _DXmCSTextSetSelection which is
**		     called by _DXmCSTextSourcesSetValue which is called
**		     by InitializeHook. - WDW
**
**       3 AUG 1992  Change XtFree to XmStringFree for an XmString
**                   in InitializeHook. - CS
**
**	26 Jan 1993  Added WidePrototype support to DXmCSTextSetAddMode()
**		     Added NO_PROTO support to static routines
**
**	2 Feb 1993   Added DXmCSTextGetBaseline routines
**
**	15 Mar 1993  Added VerifyBell routines
**--
**/


#define CSTEXT

#ifdef VMS
#include "descrip.h"
#include "ssdef.h"
#include "libdef.h"
#include "rmsdef.h"
#endif

#include <Xm/XmP.h>
#include <Xm/CutPaste.h>
#include <Xm/ScrolledWP.h>
#include <Xm/DropSMgr.h>
#include "DXmPrivate.h"

#include "CSTextI.h"
#include "CSTextSrc.h"
#include "CSTextInP.h"
#include "CSTextOutP.h"

/* Resolution independence conversion functions */

extern XmImportOperator _XmToHorizontalPixels();
extern XmImportOperator _XmToVerticalPixels();
extern void _XmFromHorizontalPixels();
extern void _XmFromVerticalPixels();
extern void _XmIntializeGetValuesResources();

extern void DXmCSTextOutputCreate();
extern void DXmCSTextInputCreate();

extern XmString _DXmCSTextSourceGetValue();
extern DXmCSTextStatus _DXmCSTextSourceInsertChar();
extern DXmCSTextStatus _DXmCSTextSourceReplaceString();

#define MESSAGE1 "Invalid margin height, must be >= 0."
#define MESSAGE2 "Invalid margin width, must be >= 0."
#define MESSAGE3 "Invalid edit mode."
#define MESSAGE4 "Traversal_on must always be true."

#define XmDYNAMIC_BOOL 255


#undef InputMethod
#define OutputMethod(w, method)    (((DXmCSTextOutput) \
                               ((DXmCSTextWidget)(w))->cstext.output)->method)
#define InputMethod(w, method)     (((DXmCSTextInput) \
                               ((DXmCSTextWidget)(w))->cstext.input)->method)

#define PendingOff(w)      (InputData (w)->pendingoff)


/* Defines for fake defaults used with resolution independance */

#define RESOURCE_DEFAULT (Dimension)(-2657)

static int       defmaxlength = MAXINT;
static int       defzero      = 0;
static Dimension def_6        = 6;
static Dimension def_20       = 20;
static Dimension def_1        = 1;

externalref XtPointer defaultCSTextActionsTable;
#ifndef WIN32
externalref char    defaultCSTextEventBindings[];
#else
externalref char    defaultCSTextEventBindings1[];
externalref char    defaultCSTextEventBindings2[];
#endif
extern Cardinal     defaultCSTextActionsTableSize;


/* define the dynamic image activation stuff here
 */

#ifdef VMS
/* DIA = Dynamic Image Activation
 */

static unsigned int
exception_handler (sigargs, mchargs)
	unsigned long sigargs[];
	unsigned long mchargs[5];
{
    /*BOGUS should check to see if file-not-found or key-not-found before
       returning SS$_CONTINUE
     */
    return ( SS$_CONTINUE );

}


#endif /* VMS */



static void    ClassInitialize();
static void    Initialize();
static void    InitializeHook();
static void    GetValuesHook();
static Boolean SetValues();
static Boolean SetValuesHook();
static void    Realize();
static void    Resize();
static void    Destroy();
static void    DoExpose();

static void Redisplay();
static void AddRedraw();


static XtResource resources[] =
{
    { XmNvalue, DXmCCompString, XmRXmString, sizeof(char *),
      XtOffset(DXmCSTextWidget, cstext.value), XmRString, NULL},

    { XmNeditable, XmCEditable, XmRBoolean, sizeof(Boolean),
      XtOffset(DXmCSTextWidget, cstext.editable), 
      XmRImmediate, (XtPointer) TRUE},

    { XmNmaxLength, XmCMaxLength, XmRInt, sizeof(int),
      XtOffset(DXmCSTextWidget, cstext.max_length), 
      XmRInt, (XtPointer)&defmaxlength},

    { XmNactivateCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
      XtOffset (DXmCSTextWidget, cstext.activate_callback),
      XmRCallback, NULL },

    { XmNfocusCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
      XtOffset (DXmCSTextWidget, cstext.focus_callback),
      XmRCallback, NULL },

    { XmNlosingFocusCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
      XtOffset (DXmCSTextWidget, cstext.losing_focus_callback),
      XmRCallback, NULL },

    { XmNvalueChangedCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
      XtOffset (DXmCSTextWidget, cstext.value_changed_callback),
      XmRCallback, NULL },

    { XmNmodifyVerifyCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
      XtOffset (DXmCSTextWidget, cstext.modify_verify_callback),
      XmRCallback, NULL },

    { XmNmotionVerifyCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
      XtOffset (DXmCSTextWidget, cstext.motion_verify_callback),
      XmRCallback, NULL },

    { XmNmarginWidth, XmCMarginWidth, XmRDimension, sizeof(Dimension),
      XtOffset(DXmCSTextWidget, cstext.margin_width),
      XmRDimension, (XtPointer)&def_6 },
  
    { XmNmarginHeight, XmCMarginHeight, XmRDimension, sizeof(Dimension),
      XtOffset(DXmCSTextWidget, cstext.margin_height), 
      XmRDimension, (XtPointer)&def_6 },

    { XmNoutputCreate, XmCOutputCreate,  XmRFunction, sizeof(DXmCSTextOutputCreateProc),
      XtOffset(DXmCSTextWidget, cstext.output_create),
      XmRFunction, (XtPointer) NULL },

    { XmNinputCreate, XmCInputCreate, XmRFunction, sizeof(DXmCSTextInputCreateProc),
      XtOffset(DXmCSTextWidget, cstext.input_create),
      XmRFunction, (XtPointer) NULL },

    { XmNtopPosition, XmCTextPosition, XmRInt, sizeof(DXmCSTextPosition),
      XtOffset(DXmCSTextWidget, cstext.top_position),
      XmRImmediate, (XtPointer) 0 },

    { XmNcursorPosition, XmCCursorPosition, XmRInt,
      sizeof (DXmCSTextPosition),
      XtOffset (DXmCSTextWidget, cstext.cursor_position),
      XmRImmediate, (XtPointer) 0 },

    { XmNeditMode, XmCEditMode, XmREditMode, sizeof(int),
      XtOffset(DXmCSTextWidget, cstext.edit_mode),
      XmRImmediate, (XtPointer) XmSINGLE_LINE_EDIT },

    {
      XmNautoShowCursorPosition, XmCAutoShowCursorPosition, XmRBoolean,
      sizeof(Boolean),
      XtOffset(DXmCSTextWidget, cstext.auto_show_cursor_position),
      XmRImmediate, (XtPointer) True },

    { DXmNtextPath, DXmCTextPath, XmRInt, sizeof(int),
      XtOffset(DXmCSTextWidget, cstext.text_path), 
      XmRInt, (XtPointer)&defzero },

    { DXmNeditingPath, DXmCEditingPath, XmRInt, sizeof(int),
      XtOffset(DXmCSTextWidget, cstext.editing_path), 
      XmRInt, (XtPointer)&defzero },

    { DXmNnoFontCallback, XmCCallback, XmRCallback,sizeof(XtCallbackList),
      XtOffset (DXmCSTextWidget, cstext.nofont_callback), 
      XmRCallback, NULL },

    {
      XmNverifyBell, XmCVerifyBell, XmRBoolean, sizeof(Boolean),
      XtOffsetOf( struct _DXmCSTextRec, cstext.verify_bell),
      XmRImmediate, (XtPointer) XmDYNAMIC_BOOL
    },

    {
      XmNnavigationType, XmCNavigationType, XmRNavigationType,
      sizeof (unsigned char),
      XtOffset (XmPrimitiveWidget, primitive.navigation_type),
      XmRImmediate, (XtPointer) XmTAB_GROUP },

  {XmNcolumns,
     XmCColumns, 
     XmRDimension,
     sizeof(Dimension),
     XtOffset (DXmCSTextWidget, cstext.columns), 
     XmRDimension, 
     (XtPointer)&def_20},
  
  {XmNrows, 
     XmCRows, 
     XmRDimension, 
     sizeof(Dimension),
     XtOffset(DXmCSTextWidget, cstext.rows),
     XmRDimension, 
     (XtPointer)&def_1},
};


/* Definition for resources that need special processing in get values */

static XmSyntheticResource get_resources[] =
{
   {
     XmNmarginWidth,
     sizeof(short),
     XtOffset(DXmCSTextWidget, cstext.margin_width),
     _XmFromHorizontalPixels
   },

   {
     XmNmarginHeight,
     sizeof(short),
     XtOffset(DXmCSTextWidget, cstext.margin_height),
     _XmFromVerticalPixels
   },
};


static DXmCSTextClassExtRec	CSTextClassExtRec = {
    NULL,                                     /* Next extension       */
    NULLQUARK,                                /* record type XmQmotif */
    DXmCSTextClassExtVersion,                 /* version              */
    sizeof(DXmCSTextClassExtRec),             /* size                 */
    DXmCSTextGetLastPosition,                 /* DXmCSTextGetLastPosition */
    DXmCSTextSetSelection,                    /* DXmCSTextSetSelection */
};


externaldef(dxmcstextclassrec) DXmCSTextClassRec dxmCSTextClassRec = {
  {
/* core_class fields */	
    /* superclass         */    (WidgetClass) &xmPrimitiveClassRec,
    /* class_name	  */	"DXmCSText",
    /* widget_size	  */	sizeof(DXmCSTextRec),
    /* class_initialize   */    (XtProc) ClassInitialize,
    /* class_part_initiali*/    NULL,
    /* class_inited       */	FALSE,
    /* initialize         */    (XtInitProc) Initialize,  
    /* initialize_hook    */	(XtArgsProc) InitializeHook,   
    /* realize            */    (XtRealizeProc) Realize,   
    /* actions		  */    NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources, 
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	FALSE,   
    /* compress_enterleave*/	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy            */    (XtWidgetProc) Destroy,  
    /* resize             */    (XtWidgetProc) Resize,   
    /* expose             */    (XtExposeProc) DoExpose, 
    /* set_values	  */	(XtSetValuesFunc) SetValues, 
    /* set_values_hook	  */	(XtArgsFunc) SetValuesHook, 
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook    */	(XtArgsProc) GetValuesHook, 
    /* accept_focus	  */	NULL,	/* Copy from super class. */
    /* version		  */	XtVersion,
    /* callback_private   */	NULL,
    /* tm_table		  */	XtInheritTranslations, 
    /* query_geometry     */    NULL,
    /* display accel	  */	NULL,
    /* extension	  */	NULL,
  },

   {                            /* primitive_class fields       */
      (XtWidgetProc)_XtInherit, /* Primitive border_highlight   */
      (XtWidgetProc)_XtInherit, /* Primitive border_unhighlight */
      NULL,                     /* translations                 */
      NULL,                     /* arm_and_activate             */
      get_resources,            /* get resources                */
      XtNumber(get_resources),  /* num get_resources            */
      NULL,                     /* extension                    */
   },

   {                            /* cstext class fields          */
      (XtPointer)&CSTextClassExtRec, /* extension               */
   }
};

externaldef(dxmcstextwidgetclass) WidgetClass dxmCSTextWidgetClass
    = (WidgetClass) &dxmCSTextClassRec;



#ifdef _NO_PROTO
void DXmCSTextSetAddMode (widget, state)
DXmCSTextWidget  widget;
Boolean         state;
#else
void DXmCSTextSetAddMode (
    DXmCSTextWidget widget,
#if NeedWidePrototypes
    int state)
#else
    Boolean state)
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    (*OutputMethod (widget, SetInsertionPoint)) (widget, CursorPos (widget));

    CSTextAddMode (widget) = state;

    (*OutputMethod (widget, SetCursor)) (widget, DXmCURSOR_TYPE_ADD_MODE);
}

#ifdef _NO_PROTO
Boolean DXmCSTextRemove(widget)
DXmCSTextWidget widget;
#else
Boolean DXmCSTextRemove(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
      DXmCSTextPosition left, right;
      XmString             selected_str;

      /* remove the selected text from the widget
      */
      if( !DXmCSTextGetSelectionInfo( (Widget) widget, &left, &right ) )
      {
	  return False;
      };

      selected_str = DXmCSTextGetSelection( (Widget) widget );

      /* no string selected or left == right, ignore
      */
      if ( selected_str == NULL || left == right )
      {
         return False;
      }

      DXmCSTextDisableRedisplay ( widget, True );
    
      /* replace text enclosed by left, right by NULL
      */
      if( _DXmCSTextSourceReplaceString( widget, left, right, NULL )
							!= I_STR_EditDone )
      {
        DXmCSTextEnableRedisplay ( widget );
	return False;
      }

      DXmCSTextEnableRedisplay ( widget );
      return True;
}


#ifdef _NO_PROTO
Boolean DXmCSTextCopy(widget, time)
Widget widget;
Time   time;
#else
Boolean DXmCSTextCopy(Widget widget,
		      Time   time)
#endif /* _NO_PROTO */
{
      XmString selected_string;	        /* text selection    */
      long item_id;			/* clipboard item id */
      long      data_id;			/* clipboard data id */
      int      status;			/* clipboard status  */
      XmString clip_label;

      selected_string = DXmCSTextGetSelection ( widget );

      item_id = 0;
      data_id = 0;
      status  = 0;

      /*
       * Using the Xm clipboard facilities,
       * copy the selected text to the clipboard
       */
      if ( selected_string != NULL )
      {
         clip_label = XmStringCreateLtoR( "XM_TEXT", XmSTRING_DEFAULT_CHARSET );

         /* start copy to clipboard
         */
         status = XmClipboardStartCopy ( XtDisplay( widget ),
					 XtWindow( widget ),
					 clip_label,
					 time,
					 widget,
					 NULL,
					 &item_id );
	 XmStringFree (clip_label);

         if ( status != ClipboardSuccess )
	 {
	      XmStringFree (selected_string);
	      return False;
         }

	 {
	     Opaque ddif;
	     long length, dummy;

	     /* data conversion
	      */
	     ddif = (Opaque) DXmCvtCStoDDIF(selected_string, &length, &dummy);

	     /* move the data to the clipboard
	      */
	     if (ddif) {
		 status = XmClipboardCopy ( XtDisplay( widget ),
					    XtWindow( widget ),
					    item_id,
					    "DDIF",
					    ddif,
					    length,
					    0,
					    &data_id );

		 XtFree(ddif);
		 if ( status != ClipboardSuccess )
		 {
		     XmStringFree (selected_string);
		     return False;
		 }
	     }
	 }

	 {
	     char *ct;

	     /* data conversion
	      */
	     ct = XmCvtXmStringToCT(selected_string);

	     /* move the data to the clipboard
	      */
	     if (ct) {
		 status = XmClipboardCopy ( XtDisplay( widget ),
					    XtWindow( widget ),
					    item_id,
					    "COMPOUND_TEXT",
					    ct,
					    (long)strlen(ct) + 1,
					    0,
					    &data_id );

		 XtFree(ct);
		 if ( status != ClipboardSuccess )
		 {
		     XmStringFree (selected_string);
		     return False;
		 }
	     }
	 }

	 {
	     Opaque fc;
	     long length, dummy;

	     /* data conversion
	      */
	     fc = (Opaque) DXmCvtCStoFC(selected_string, &length, &dummy);

	     /* move the data to the clipboard
	      */
	     if (fc) {
		 status = XmClipboardCopy ( XtDisplay( widget ),
					    XtWindow( widget ),
					    item_id,
					    "STRING",
					    fc,
					    (long)length,
					    0,
					    &data_id );

		 XtFree(fc);
		 if ( status != ClipboardSuccess )
		 {
		     XmStringFree (selected_string);
		     return False;
		 }
	     }
	 }

         /* end the copy to the clipboard
	 */
         status = XmClipboardEndCopy ( XtDisplay( widget ),
				       XtWindow( widget ),
				       item_id );

         XmStringFree (selected_string);

         if ( status != ClipboardSuccess )
	 {
	      return False;
         } else {
	      return True;
	 }
      } else {
         return False;
      }
}


#ifdef _NO_PROTO
Boolean DXmCSTextCut(widget, time)
Widget widget;
Time   time;
#else
Boolean DXmCSTextCut(Widget widget,
		     Time   time)
#endif /* _NO_PROTO */
{
      Boolean copy, remove;

      DXmCSTextDisableRedisplay ( (DXmCSTextWidget) widget, True );
    
      /* use clipboard to copy selected text
      */
      copy = DXmCSTextCopy( widget, time );

      if ( copy )
      {
         remove = DXmCSTextRemove( (DXmCSTextWidget) widget );

         if ( remove )
         {
	    DXmCSTextEnableRedisplay ( (DXmCSTextWidget) widget );
            return True;
         }
      }; 

      DXmCSTextEnableRedisplay ( (DXmCSTextWidget) widget );
      return False;
}

 


/*
 * Retrieves the current data from the clipboard
 * and paste it at the current cursor position
 */
#ifdef _NO_PROTO
Boolean DXmCSTextPaste(widget)
DXmCSTextWidget widget;
#else
Boolean DXmCSTextPaste(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
      DXmCSTextPosition left, right;
      int              status;		/* clipboard status        */
      char             *buffer;		/* temporary text buffer   */
      unsigned long    length;		/* length of buffer        */
      unsigned long    outlength;	/* length of bytes copied  */
      long             private_id;	/* id of item on clipboard */
      register int     i;
      static char      *data_type[] = {"DDIF", "COMPOUND_TEXT", "STRING"};


      for (i = 0; i < sizeof(data_type) / sizeof(char *); i++) {
	  char *tmp_buf;
	  int len, status;

	  status     = 0;
	  outlength  = 0;
	  private_id = 0;
	  buffer     = NULL;

	  status = XmClipboardInquireLength( XtDisplay( widget ),
					     XtWindow ( widget ),
					     data_type[i],
					     &length );

	  if ( status == ClipboardNoData || length == 0 )

	      continue;

	  /* malloc length of clipboard data
	   */
	  tmp_buf = XtMalloc( length +1);	/* fix 18-oct-91 */

	  status = XmClipboardRetrieve( XtDisplay( widget ),
				        XtWindow ( widget ),
				        data_type[i],
				        tmp_buf,
				        length,
				        &outlength,
				        &private_id );

	  if ( status != ClipboardSuccess )
	  {
	      XtFree(tmp_buf);
	      continue;
	  }

	  /* convert data */
	  if (strcmp(data_type[i], "DDIF") == 0) {
	      buffer = (char *) DXmCvtDDIFtoCS(tmp_buf, &len, &status);
	  } else if (strcmp(data_type[i], "COMPOUND_TEXT") == 0) {
	      tmp_buf[length] = '\0';	 /* fix 18-oct-91 */
	      buffer = (char *) XmCvtCTToXmString(tmp_buf);
	  } else if (strcmp(data_type[i], "STRING") == 0) {
	      tmp_buf[length] = '\0';	 /* fix 18-oct-91 */
	      buffer = (char *) DXmCvtFCtoCS(tmp_buf, &len, &status);
	  }

	  XtFree(tmp_buf);
	  if (buffer)
	      break;
      }

      if (!buffer)
	  return False;

      /* add new text
      */
      if( DXmCSTextInsert( (Widget) widget, CursorPos (widget), (XmString) buffer )
	    != I_STR_EditDone)
      {
	   XtFree(buffer);
           return False;
      }

    XtFree(buffer);
    return True;
}
 




/*
 * Actually do some work.  This routine gets called to actually paint all the
 * stuff that has been pending.
 */

static void
#ifdef _NO_PROTO
Redisplay( widget )
      DXmCSTextWidget widget;
#else
Redisplay(
      DXmCSTextWidget widget )
#endif /* _NO_PROTO */

{
    if( CSTextDisableDepth(widget) != 0)
    {
        return;
    }

    (*OutputMethod (widget, Draw)) (widget);
}



/*
 * Create the cstext widget.  To handle default condition of the core
 * height and width after primitive has already reset it's height and
 * width, use request values and reset height and width to original
 * height and width state.
 */

static void
#ifdef _NO_PROTO
Initialize(request, new) 
      DXmCSTextWidget request, new;
#else
Initialize(
      DXmCSTextWidget request, DXmCSTextWidget new )
#endif /* _NO_PROTO */
{
    if ( XtWidth (request) == 0 )
         XtWidth (new) = XtWidth (request);

    if ( XtHeight (request) == 0 )
         XtHeight (new) = XtHeight (request);

    new->primitive.traversal_on = True;

    /* BOGUS
     * Flag used in losing focus verification to indicate that a traversal
     * key was pressed.  Must be initialized to False
     */
    CSTextTraversed (new) = False;

    if ( CSTextOutputCreate (new) == NULL )
	 CSTextOutputCreate (new) = DXmCSTextOutputCreate;

    if ( CSTextInputCreate (new) == NULL )
	 CSTextInputCreate (new) = DXmCSTextInputCreate;

    /*  Convert the fields from unit values to pixel values
    */

    if ( new->primitive.unit_type != XmPIXELS )
    {
      if ( CSTextMarginHeight (new) > 0 )
      {
	/* QAR 1008 - _XmToVerticalPixels() treats the last parameter as
	 *	      long where CSTextMarginHeight(new) is short
	 */
	    long temp_height = (long) CSTextMarginHeight(new);

            (void)_XmToVerticalPixels ( (Widget) new,
				  	new->primitive.unit_type,
                                  	&temp_height );

            CSTextMarginHeight(new) = (Dimension) temp_height;
	
      }

      if ( CSTextMarginWidth (new) > 0 )
      {
	/* QAR 1008 - _XmToVerticalPixels() treats the last parameter as
	 *	      long where CSTextMarginWidth(new) is short
	 */
	    long temp_width = (long) CSTextMarginWidth(new);

            (void)_XmToHorizontalPixels ( (Widget) new,
                                    	  new->primitive.unit_type,
                                    	  &temp_width );

	    CSTextMarginWidth(new) = (Dimension) temp_width;
      }
    }

   /* The following resources are defaulted to invalid values to indicate
    * that it was not set by the application.  If it gets to this point
    * and they are still invalid then set them to their appropriate default. 
    */

   if ( CSTextMarginHeight (new) == RESOURCE_DEFAULT )
        CSTextMarginHeight (new) = 3;

   if ( CSTextMarginHeight (new) < 0 )
   {
	_XmWarning ( (Widget) new, MESSAGE1 );
        CSTextMarginHeight (new) = 3;
   }

   if ( CSTextMarginWidth (new) == RESOURCE_DEFAULT )
        CSTextMarginWidth (new) = 3;

   if ( CSTextMarginWidth (new) < 0 )
   {
	_XmWarning ( (Widget) new, MESSAGE2 );
        CSTextMarginWidth (new) = 3;
   }

   if ( CSTextEditMode (new) != XmSINGLE_LINE_EDIT &&
        CSTextEditMode (new) != XmMULTI_LINE_EDIT )
   {
	_XmWarning ( (Widget) new, MESSAGE3 );
	CSTextEditMode (new) = XmSINGLE_LINE_EDIT;
   }

  if ( CSTextVerifyBell (new) == (Boolean) XmDYNAMIC_BOOL)
      if (_XmGetAudibleWarning(new) == XmBELL)  CSTextVerifyBell (new)= True;

}





static void
#ifdef _NO_PROTO
InitializeHook(widget, args, num_args_ptr)
    DXmCSTextWidget   widget;
    ArgList  args;
    Cardinal *num_args_ptr;
#else
InitializeHook(
    DXmCSTextWidget widget,
    ArgList  args,
    Cardinal *num_args_ptr )
#endif /* _NO_PROTO */
{
    Cardinal        num_args;
    XmStringCharSet set;
    XmString val = NULL;

    num_args = *num_args_ptr;

    /*
     * do the easy stuff first
     */

    CSTextLines    (widget) = NULL;
    CSTextLength   (widget) = 0;

    CSTextSelLeft  (widget) = 0;
    CSTextSelRight (widget) = 0;

    CSTextHasFocus (widget) = False;
    CSTextAddMode  (widget) = False;

    CSTextDisableDepth (widget)  = 0;

    PendingVScroll (widget)  =
    PendingHScroll (widget)  = 0;

    CSTextDefaultCharSet (widget) = (XmStringCharSet) NULL;

    CSTextHasSelection (widget) = False;

    BotPos (widget) = 0;

    ( *CSTextOutputCreate (widget) )( widget, args, num_args );
    ( *CSTextInputCreate  (widget) )( widget, args, num_args );

    if ( CSTextValue (widget) == NULL )
    {
      if (CSTextPath ( widget ) == DXmDIRECTION_LEFT_DOWN)
      {
          /* create a direction component too.*/
          XmString nullstr = XmStringCreateSimple ("");
          XmString path =
                     XmStringDirectionCreate(XmSTRING_DIRECTION_R_TO_L) ;
          val = XmStringConcat(path, nullstr) ;
          XmStringFree( path ) ;
          XmStringFree( nullstr ) ;
      }
      else 
          val = XmStringCreateSimple ("");

      CSTextValue (widget) = val;
    }

    _DXmCSTextSourceSetValue( widget, CSTextValue (widget) );

    if (val)
      XmStringFree (val);

    CSTextEditingPath (widget)  = CSTextPath (widget);

    (*OutputMethod (widget, RedisplayHBar)) ( widget );

    CSTextValue (widget) = NULL;

}
    

static void
#ifdef _NO_PROTO
Realize(w, valueMask, attributes)
      Widget  w;
      Mask    *valueMask;
      XSetWindowAttributes *attributes;
#else
Realize(
      Widget  w,
      Mask    *valueMask,
      XSetWindowAttributes *attributes )
#endif /* _NO_PROTO */
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    (*OutputMethod (widget, Realize)) 
	( (DXmCSTextWidget)widget, valueMask, attributes );
}


static Boolean
#ifdef _NO_PROTO
SetValues(oldw, request, neww)
      Widget oldw, request, neww;
#else
SetValues(
      Widget oldw,
      Widget request,
      Widget neww )
#endif /* _NO_PROTO */
{
   DXmCSTextWidget old;
   DXmCSTextWidget new;
   DXmCSTextPosition insertPos;

   old = (DXmCSTextWidget) oldw;
   new = (DXmCSTextWidget) neww;

   if ( CSTextMarginHeight (new) < 0 )
   {
      _XmWarning( (Widget) new, MESSAGE1 );
      CSTextMarginHeight (new) = CSTextMarginHeight (old);
   }

   if ( CSTextMarginWidth (new) < 0 )
   {
      _XmWarning( (Widget) new, MESSAGE2 );
      CSTextMarginWidth (new) = CSTextMarginWidth (old);
   }

   if ( new->primitive.traversal_on != True )
   {
      _XmWarning( (Widget) new, MESSAGE4 );
      new->primitive.traversal_on = old->primitive.traversal_on;
   }

   if ( CSTextEditMode (new) != XmSINGLE_LINE_EDIT &&
        CSTextEditMode (new) != XmMULTI_LINE_EDIT )
   {
	_XmWarning( (Widget) new, MESSAGE3 );
	CSTextEditMode (new) = CSTextEditMode (old);
   }

   /*  Convert the fields from unit values to pixel values
   */

   if ( new->primitive.unit_type != XmPIXELS )
   {
      if ( CSTextMarginHeight (new) != CSTextMarginHeight (old) -
                                       ( old->primitive.shadow_thickness +
                                         old->primitive.highlight_thickness ) )
      {
	   long temp_height = (long) CSTextMarginHeight(new);

           (void)_XmToVerticalPixels ( (Widget) new,
				       new->primitive.unit_type,
                                       &temp_height );
      }

      if ( CSTextMarginWidth (new) != CSTextMarginWidth (old) -
                                      ( old->primitive.shadow_thickness +
                                        old->primitive.highlight_thickness ) )
      {
	    long temp_width = (long) CSTextMarginWidth(new);

            (void)_XmToHorizontalPixels ( (Widget) new,
                                    	  new->primitive.unit_type,
                                    	  &temp_width );
      }
    }

    if (CursorPos (old) != CursorPos (new))
    {
	DXmCSTextSetCursorPosition( new, CursorPos (new));
    }

    /* Don't allow editing path to be changed
    */
    CSTextEditingPath (new)  = CSTextEditingPath (old);

    if (CSTextPath (old) != CSTextPath (new))
    {
       TextLine    line     = _DXmCSTextGetFirstLine (new);

       if (  CSTextLength( new) > 0 ||
             (line != (TextLine)NULL && line->next != (TextLine)NULL) )
       {
             /* if widget is not empty - cancel request 
             */
             CSTextPath (new)  = CSTextPath (old);
       }
       else  DXmCSTextSetTextPath( new, (XmStringDirection) CSTextPath (new)) ;
    }

    if ( CSTextValue (old) != CSTextValue (new) )
    {
        DXmCSTextPosition    cursorPos;
        XmAnyCallbackStruct cb;

        PendingOff (new) = TRUE;

        _DXmCSTextSourceSetValue( new, CSTextValue (new) );

	CSTextValue (new) = CSTextValue (old);         /* Fixed by JRD */
    }

    return FALSE;
}




static Boolean
#ifdef _NO_PROTO
SetValuesHook(w, args, num_args_ptr)
      Widget   w;
      ArgList  args;
      Cardinal *num_args_ptr;
#else
SetValuesHook(
      Widget   w,
      ArgList  args,
      Cardinal *num_args_ptr )
#endif /* _NO_PROTO */
{
    DXmCSTextWidget   widget;
    Cardinal         num_args;
    DXmCSTextPosition insertPos;
    int              i;

    widget    = (DXmCSTextWidget) w;
    num_args  = *num_args_ptr;

    DXmCSTextDisableRedisplay( widget, TRUE );

    (*OutputMethod (widget, SetValues)) ( widget, args, num_args );
    (*InputMethod  (widget, SetValues)) ( widget, args, num_args );

    DXmCSTextEnableRedisplay( widget );

    return FALSE;
}



static void
#ifdef _NO_PROTO
GetValuesHook(w, args, num_args_ptr)
      Widget   w;
      ArgList  args;
      Cardinal *num_args_ptr;
#else
GetValuesHook(
      Widget   w,
      ArgList  args,
      Cardinal *num_args_ptr )
#endif /* _NO_PROTO */
{
    int i;
    DXmCSTextPosition insertPos;
    DXmCSTextWidget   widget;
    Cardinal         num_args;

    widget   = (DXmCSTextWidget) w;
    num_args = *num_args_ptr;

    for (i = 0; i < num_args; i++)
    {
	if ( strcmp( args[i].name, XmNvalue ) == 0 )
        {
    	    if( CSTextValue( widget ) != (XmString)NULL )
    	    {
		XmStringFree( CSTextValue( widget ) );
    	    }

	    CSTextValue (widget) = _DXmCSTextSourceGetValue( widget );
	    break;
	}
    }

    XtGetSubvalues( (XtPointer) widget,
		    resources,
		    XtNumber( resources ),
		    args,
		    num_args );


    ( *OutputMethod (widget, GetValues)) ((DXmCSTextWidget) widget, args, num_args );
    ( *InputMethod  (widget, GetValues)) ((Widget) widget, args, num_args );
}    


#ifdef _NO_PROTO
Widget DXmCreateCSText(parent, name, args, num_args)
Widget   parent;
char     *name;
Arg      *args;
Cardinal num_args;
#else
Widget DXmCreateCSText(Widget   parent,
		       char     *name,
		       Arg      *args,
		       Cardinal num_args)
#endif /* _NO_PROTO */
{

    extern void DXmCSTextOutputCreate();
    extern void DXmCSTextInputCreate();

    ArgList  newarg;
    Cardinal new_num;

    Widget   out_widget;

    void (*outputptr) ();
    void (*inputptr)  ();

    static Arg argio[] = {
	{XmNoutputCreate, (XtArgVal) NULL},
	{XmNinputCreate,  (XtArgVal) NULL},
    };

#ifdef VMS

  struct dsc$descriptor_s file_name;
  char	 file_name_string[100];
  char   *lang;
/*  
  $DESCRIPTOR(file_name,   "DECW$DXM_I18NIOSHR");
  $DESCRIPTOR(image_name,  "SYS$LIBRARY:DECW$DXM_I18NIOSHR.EXE");
*/
  $DESCRIPTOR(inputcreate_symbol, "_DXMCSTEXTINPUTCREATE");
  $DESCRIPTOR(outputcreate_symbol,"_DXMCSTEXTOUTPUTCREATE");

  strcpy ( file_name_string, "DECW$DXM_I18NIOSHR" );
  lang = xnl_getlanguage();
  if ( lang && *lang ){
  char *dot;
  strcat ( file_name_string, "_" );
  strcat ( file_name_string, lang );
  if ( dot = strchr ( file_name_string, '.') )
    *dot = '\0';
  }
  file_name.dsc$w_length  = strlen(file_name_string);
  file_name.dsc$b_dtype   = DSC$K_DTYPE_T;
  file_name.dsc$b_class   = DSC$K_CLASS_S;
  file_name.dsc$a_pointer = file_name_string;

  /*DIA register exception handler
   */
  VAXC$ESTABLISH ( exception_handler );

  if ( SS$_NORMAL != LIB$FIND_IMAGE_SYMBOL ( &file_name,
					     &outputcreate_symbol,
					     &outputptr,
					/*   &image_name ) )	*/
					     NULL ) )
  {
       outputptr = DXmCSTextOutputCreate;
  }

  if ( SS$_NORMAL != LIB$FIND_IMAGE_SYMBOL ( &file_name,
					     &inputcreate_symbol,
					     &inputptr,
				      /*     &image_name ) )	*/
					     NULL ) )
  {
       inputptr = DXmCSTextInputCreate;
  }

  /* reset the exception handler
   */

  LIB$REVERT();

#else

    inputptr  = DXmCSTextInputCreate;
    outputptr = DXmCSTextOutputCreate;

#endif /* VMS */

    argio[0].value = (XtArgVal) outputptr;
    argio[1].value = (XtArgVal) inputptr;

    newarg = XtMergeArgLists( args, num_args, argio, XtNumber( argio ) );

    new_num = num_args + XtNumber( argio );

    out_widget = 
         XtCreateWidget( name, dxmCSTextWidgetClass, parent, newarg, new_num );

    XtFree ((char *)newarg);

    return (out_widget);
}



#ifdef _NO_PROTO
Widget DXmCreateScrolledCSText( parent, name, arglist, argcount )
Widget   parent;
char     *name;
ArgList  arglist;
Cardinal argcount;
#else
Widget DXmCreateScrolledCSText( Widget    parent,
				char      *name,
				ArgList   arglist,
				Cardinal  argcount )
#endif /* _NO_PROTO */
{
    Widget swindow;
    Widget cstext;
    Arg args[4];
    ArgList m;
    int n;
    char *s;

    /* name + NULL + "SW" 
    */
    s = XtMalloc( strlen( name ) + 3 );  
    strcpy( s, name );
    strcat( s, "SW" );

    /* create scrolled window.
    */
    m = (ArgList) XtCalloc( argcount+4, sizeof( Arg ) );

    for (n=0; n < argcount; n++) 
    {
	m[n].name  = arglist[n].name;
	m[n].value = arglist[n].value;
    }

    XtSetArg (m[n], XmNscrollingPolicy, (XtArgVal) XmAPPLICATION_DEFINED); n++;
    XtSetArg (m[n], XmNvisualPolicy,    (XtArgVal) XmVARIABLE); n++;
    XtSetArg (m[n], XmNscrollBarDisplayPolicy, (XtArgVal) XmSTATIC); n++;
    XtSetArg (m[n], XmNshadowThickness, (XtArgVal) 0); n++;

    swindow = XtCreateManagedWidget( s, xmScrolledWindowWidgetClass, 
				     parent, m, n );

    XtFree (s);
    XtFree ((char *)m);

    /* create I text.
    */
    cstext = DXmCreateCSText (swindow, name, arglist, argcount );

    /* add callback to destroy ScrolledWindow parent.
    */
    XtAddCallback( cstext, XmNdestroyCallback, 
		  _XmDestroyParentCallback, NULL );

    return( cstext );
}




/*--------------------------------------------------------------------*/
#ifdef _NO_PROTO
XmString DXmCSTextGetString(widget)
Widget widget;
#else
XmString DXmCSTextGetString(Widget widget)
#endif /* _NO_PROTO */
{

    return ( _DXmCSTextSourceGetValue((DXmCSTextWidget) widget ) );

}

#ifdef _NO_PROTO
XmString DXmCSTextGetStringWrapped(widget, start, end)
Widget widget;
DXmCSTextPosition start;
DXmCSTextPosition end;
#else
XmString DXmCSTextGetStringWrapped( Widget widget,
				    DXmCSTextPosition start,
				    DXmCSTextPosition end)
#endif /* _NO_PROTO */
{
    DXmCSTextWidget w = (DXmCSTextWidget) widget;
    DXmCSTextStatus status;
    XmString string;

    status = (*OutputMethod (w, ReadString)) (w, start, end, &string);

    if ( status == I_STR_EditDone )
	return (string);
    else
	return ((XmString) NULL );
}



/*--------------------------------------------------------------------*/
#ifdef _NO_PROTO
void DXmCSTextSetString(widget, value)
DXmCSTextWidget   widget;
XmString          value;
#else
void DXmCSTextSetString(DXmCSTextWidget widget, XmString value)
#endif /* _NO_PROTO */
{
    DXmCSTextPosition lastpos;
    DXmCSTextPosition insertpos;
    XmStringCharSet  set;

    DXmCSTextDisableRedisplay ( widget, True );

    insertpos = DXmCSTextGetCursorPosition( widget );

    _DXmCSTextSourceSetValue ( widget, value);

    lastpos = CSTextLength ( widget );

    if ( lastpos < insertpos )
    {
	insertpos = lastpos;
    }

    DXmCSTextSetCursorPosition( widget, insertpos );

    DXmCSTextEnableRedisplay ( widget );

    return;
}



/*--------------------------------------------------------------------*/
#ifdef _NO_PROTO
DXmCSTextStatus DXmCSTextReplace(widget, frompos, topos, value)
Widget   widget;
DXmCSTextPosition frompos, topos;
XmString value;
#else
DXmCSTextStatus DXmCSTextReplace(Widget		   widget,
				 DXmCSTextPosition frompos,
				 DXmCSTextPosition topos,
				 XmString	   value)
#endif /* _NO_PROTO */
{
    DXmCSTextStatus ret_status;
    Boolean editable;

    DXmCSTextDisableRedisplay ( (DXmCSTextWidget)widget, True );

    editable = CSTextEditable (widget);
    CSTextEditable (widget) = TRUE;
    
    ret_status = 
	_DXmCSTextSourceReplaceString( (DXmCSTextWidget)widget,
					frompos, topos, value );
    CSTextEditable (widget) = editable;

    DXmCSTextEnableRedisplay ( (DXmCSTextWidget)widget );

    return ret_status;
}

/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
void DXmCSTextRead(widget, frompos, topos, value)
Widget   widget;
DXmCSTextPosition frompos, topos;
XmString *value;
#else
void DXmCSTextRead(Widget 	     widget,
		   DXmCSTextPosition frompos,
		   DXmCSTextPosition topos,
		   XmString	     *value)
#endif /* _NO_PROTO */
{

    _DXmCSTextSourceReadString( (DXmCSTextWidget)widget,
				frompos, topos, value );
}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
DXmCSTextStatus DXmCSTextInsert(widget, pos, value)
Widget   widget;
DXmCSTextPosition pos;
XmString value;
#else
DXmCSTextStatus DXmCSTextInsert(Widget            widget,
				DXmCSTextPosition pos,
				XmString          value)
#endif /* _NO_PROTO */
{
    DXmCSTextStatus ret_status;
#ifdef DEC_MOTIF_BUG_FIX
    Boolean editable;
#endif

    DXmCSTextDisableRedisplay ( (DXmCSTextWidget)widget, True );
    
#ifdef DEC_MOTIF_BUG_FIX
    /* Allow operation even if CSTextEditable=false, 5/14/93 sp */
    editable = CSTextEditable (widget);
    CSTextEditable (widget) = TRUE;
#endif

    ret_status = 
	_DXmCSTextSourceReplaceString( (DXmCSTextWidget)widget,
					pos, pos, value );
#ifdef DEC_MOTIF_BUG_FIX
    CSTextEditable (widget) = editable;
#endif
    DXmCSTextEnableRedisplay ( (DXmCSTextWidget)widget );

    return ret_status;
}

/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
DXmCSTextStatus DXmCSTextInsertChar(widget, pos, value)
Widget   widget;
DXmCSTextPosition pos;
XmString value;
#else
DXmCSTextStatus DXmCSTextInsertChar(Widget	      widget,
				    DXmCSTextPosition pos,
				    XmString	      value)
#endif /* _NO_PROTO */
{
    DXmCSTextStatus ret_status;

    DXmCSTextDisableRedisplay ( (DXmCSTextWidget)widget, True );
    
    ret_status = 
        _DXmCSTextSourceInsertChar( (DXmCSTextWidget)widget, pos, value );

    DXmCSTextEnableRedisplay ( (DXmCSTextWidget)widget );

    return ret_status;
}

/*--------------------------------------------------------------------*/
#ifdef _NO_PROTO
Boolean DXmCSTextHasSelection( widget )
DXmCSTextWidget widget;
#else
Boolean DXmCSTextHasSelection(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
   return ( CSTextHasSelection (widget));
}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
Boolean DXmCSTextGetEditable( widget )
DXmCSTextWidget widget;
#else
Boolean DXmCSTextGetEditable(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
  return (CSTextEditable (widget));
}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
void DXmCSTextSetEditable( widget, editable )
DXmCSTextWidget widget;
Boolean editable;
#else
void DXmCSTextSetEditable(DXmCSTextWidget widget,
			  Boolean         editable)
#endif /* _NO_PROTO */
{
    XPoint xmim_point;
    Arg args[6];
    Cardinal n = 0;

    if (!CSTextEditable(widget) && editable) {

	XmImRegister((Widget) widget, (unsigned int) NULL);

        DXmCSTextPosToXY ((Widget) widget, DXmCSTextGetCursorPosition(widget),
                                      &xmim_point.x, &xmim_point.y);
	n = 0;
        XtSetArg(args[n], XmNfontList, FontList(widget)); n++;
	XtSetArg(args[n], XmNbackground, 
		 widget->core.background_pixel); n++;
	XtSetArg(args[n], XmNforeground, widget->primitive.foreground); n++;
	XtSetArg(args[n], XmNbackgroundPixmap,
                   widget->core.background_pixmap);n++;
	XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
	XtSetArg(args[n], XmNlineSpace, TypicalHeight(widget)); n++;
	XmImSetValues((Widget)widget, args, n);
    } else if (CSTextEditable(widget) && !editable){
           XmImUnregister((Widget) widget);
    }

    CSTextEditable (widget) = editable;

    n = 0;

    if (editable)
    {
	XtSetArg(args[n], XmNdropSiteActivity, XmDROP_SITE_ACTIVE); n++;
    } 
    else 
    {
	XtSetArg(args[n], XmNdropSiteActivity, XmDROP_SITE_INACTIVE); n++;
    }

    XmDropSiteUpdate((Widget)widget, args, n);

}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
int DXmCSTextGetMaxLength( widget )
DXmCSTextWidget widget;
#else
int DXmCSTextGetMaxLength(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
    return CSTextMaxLength (widget);
}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
void DXmCSTextSetMaxLength( widget, max )
DXmCSTextWidget widget;
int max;
#else
void DXmCSTextSetMaxLength(DXmCSTextWidget widget,
			   int 		   max)
#endif /* _NO_PROTO */
{
    CSTextMaxLength (widget) = max;
}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
XmStringDirection DXmCSTextGetTextPath( widget )
DXmCSTextWidget widget;
#else
XmStringDirection DXmCSTextGetTextPath(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
    return (CSTextPath (widget));
}

/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
void DXmCSTextSetTextPath( widget, path )
DXmCSTextWidget widget;
XmStringDirection path;
#else
void DXmCSTextSetTextPath(DXmCSTextWidget   widget,
			  XmStringDirection path)
#endif /* _NO_PROTO */
{

    TextLine    line     = _DXmCSTextGetFirstLine (widget);
    TextSegment segment ;

    /* Assume widget is empty
    */
    if ( CSTextLength( widget) > 0 ) return ;

    /* Set EditingPath */
    CSTextEditingPath (widget) = path ;

    /* Now toggle the directions of any empty segments.
    */
    if (line != (TextLine)NULL)
    {
    	segment = _DXmCSTextGetFirstSegment(line);
    	while (segment != (TextSegment)NULL)
    	{
                   segment->direction =
                       ( segment->direction == 0 ) ? 1 : 0;
                   segment = _DXmCSTextGetNextSegment( segment ) ;
    	}
    }

    CSTextPath (widget) = path;

}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
XmString DXmCSTextGetSelection( widget )
Widget widget;
#else
XmString DXmCSTextGetSelection(Widget widget)
#endif /* _NO_PROTO */
{
    XmString	     out_string;
    DXmCSTextPosition left, right;

    if ( !_DXmCSTextGetSelection( (DXmCSTextWidget) widget, &left, &right ) )
    {
	return NULL;
    }
    else {
        _DXmCSTextSourceReadString( (DXmCSTextWidget) widget, left, right, &out_string );
	return ( (XmString) out_string );
    }
}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
Boolean DXmCSTextGetSelectionInfo( widget, left, right )
Widget widget;
DXmCSTextPosition *left, *right;
#else
Boolean DXmCSTextGetSelectionInfo(Widget 	    widget,
				  DXmCSTextPosition *left,
				  DXmCSTextPosition *right)
#endif /* _NO_PROTO */
{
    return ( _DXmCSTextGetSelection( (DXmCSTextWidget) widget, left, right ) );
}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
void DXmCSTextSetSelection(widget, first, last, time)
Widget               widget;
DXmCSTextPosition    first, last;
Time                 time;
#else
void DXmCSTextSetSelection(Widget               widget,
			   DXmCSTextPosition    first,
			   DXmCSTextPosition    last,
			   Time   time)
#endif /* _NO_PROTO */
{
    DXmCSTextDisableRedisplay ( (DXmCSTextWidget) widget, True );
    
    _DXmCSTextSetSelection( (DXmCSTextWidget) widget, first, last, time );

    DXmCSTextEnableRedisplay ( (DXmCSTextWidget) widget );

    return;
}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
void DXmCSTextClearSelection(widget, time)
Widget widget;
Time   time;
#else
void DXmCSTextClearSelection(Widget widget,
			     Time   time)
#endif /* _NO_PROTO */
{
    DXmCSTextDisableRedisplay ( (DXmCSTextWidget) widget, True );
    
    /* pass (0, 0) for (left, right) to clear the selection
    */
    _DXmCSTextSetSelection( (DXmCSTextWidget) widget, 0, 0, time );

    DXmCSTextEnableRedisplay ( (DXmCSTextWidget) widget );

    return;
}

/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
DXmCSTextPosition DXmCSTextXYToPos( widget, x, y )
Widget widget;
Position x;
Position y;
#else
DXmCSTextPosition DXmCSTextXYToPos( Widget   widget,
				    Position x,
				    Position y )
#endif /* _NO_PROTO */
{
    DXmCSTextWidget w = (DXmCSTextWidget) widget;
    DXmCSTextPosition pos;

    pos = (*OutputMethod (w, XYToPos)) (w, x, y);

    return (pos);
}


/*--------------------------------------------------------------------*/

#ifdef _NO_PROTO
Boolean DXmCSTextPosToXY ( widget, position, x, y)
Widget widget;
DXmCSTextPosition position;
Position *x;
Position *y;
#else
Boolean DXmCSTextPosToXY ( Widget widget,
			   DXmCSTextPosition position,
			   Position *x,
			   Position *y )
#endif /* _NO_PROTO */
{
    DXmCSTextWidget w = (DXmCSTextWidget) widget;
    Boolean ret_val;

    ret_val = (*OutputMethod (w, PosToXY)) (w, position, x, y);

    return (ret_val);
}


/*--------------------------------------------------------------------*/

static void ClassInitialize()
{
  char * ev_bindings;
#ifdef VMS

  long       cond_value;
  char       **action_table;
  char       **event_bindings;
  long       *action_table_size;

  struct dsc$descriptor_s file_name;
  char	 file_name_string[100];
  char   *lang;
/*
  $DESCRIPTOR(file_name,   "DECW$DXM_I18NIOSHR");
  $DESCRIPTOR(image_name,  "SYS$LIBRARY:DECW$DXM_I18NIOSHR.EXE");
*/
  $DESCRIPTOR(action_table_symbol,       "_DEFAULTCSTEXTACTIONSTABLE");
  $DESCRIPTOR(action_size_symbol,        "_DEFAULTCSTEXTACTIONSTABLESIZE");
  $DESCRIPTOR(event_bindings_symbol,     "_DEFAULTCSTEXTEVENTBINDINGS");

  strcpy ( file_name_string, "DECW$DXM_I18NIOSHR" );
  lang = xnl_getlanguage();
  if ( lang && *lang ){
  char *dot;
  strcat ( file_name_string, "_" );
  strcat ( file_name_string, lang );
  if ( dot = strchr ( file_name_string, '.') )
    *dot = '\0';
  }
  file_name.dsc$w_length  = strlen(file_name_string);
  file_name.dsc$b_dtype   = DSC$K_DTYPE_T;
  file_name.dsc$b_class   = DSC$K_CLASS_S;
  file_name.dsc$a_pointer = file_name_string;

  /*DIA register exception handler
   */
  VAXC$ESTABLISH ( exception_handler );

  /*DIA dynamically load action table to core
   */
  cond_value = LIB$FIND_IMAGE_SYMBOL ( &file_name,
				       &action_table_symbol,
				       &action_table,
				  /*   &image_name );	*/
					NULL );

  dxmCSTextClassRec.core_class.actions = (cond_value == SS$_NORMAL) ?
        (struct _XtActionRec *) *action_table :
        (struct _XtActionRec *) defaultCSTextActionsTable;


  /*DIA dynamically load actions table size to core
   */

  cond_value = LIB$FIND_IMAGE_SYMBOL ( &file_name,
				       &action_size_symbol,
				       &action_table_size,
				  /*   &image_name ) 	*/
				       NULL ); 

  dxmCSTextClassRec.core_class.num_actions = (cond_value == SS$_NORMAL) ?
         *action_table_size : defaultCSTextActionsTableSize;


  /*IO  dynamically load event bindings to core
   */
  cond_value = LIB$FIND_IMAGE_SYMBOL ( &file_name,
				       &event_bindings_symbol,
				       &event_bindings,
				  /*   &image_name ) 	*/
				       NULL ); 

  dxmCSTextClassRec.core_class.tm_table = (cond_value == SS$_NORMAL) ?
        (char *) event_bindings : defaultCSTextEventBindings;

  /* reset the exception handler
   */

  LIB$REVERT();

#else

    dxmCSTextClassRec.core_class.actions =
	(struct _XtActionsRec *)defaultCSTextActionsTable;

    dxmCSTextClassRec.core_class.num_actions = defaultCSTextActionsTableSize;

#ifndef WIN32
    dxmCSTextClassRec.core_class.tm_table    = defaultCSTextEventBindings;
#else
    /*
     * WIN32 compiler can't handle long string constants, need to piece the
     * translation table string together here.
     */ 
    ev_bindings = (char *) XtMalloc(strlen(defaultCSTextEventBindings1) +
				       strlen(defaultCSTextEventBindings2) + 1);
    strcpy(ev_bindings, defaultCSTextEventBindings1);
    strcat(ev_bindings, defaultCSTextEventBindings2);
    dxmCSTextClassRec.core_class.tm_table    = ev_bindings;
#endif
#endif /* VMS */
}




/*
 * Force the given position to be displayed.  Output module will tell us
 * if a redisplay is needed
 */

#ifdef _NO_PROTO
void DXmCSTextShowPosition (widget, position)
DXmCSTextWidget widget;
DXmCSTextPosition position;
#else
void DXmCSTextShowPosition (DXmCSTextWidget   widget,
			    DXmCSTextPosition position)
#endif /* _NO_PROTO */
{
  (*OutputMethod (widget, MakePositionVisible)) (widget, position);

  Redisplay( widget );
}

/* 
 * scroll text by n lines.  Positive n means scroll upward; negative, downward
 */

#ifdef _NO_PROTO
void DXmCSTextVerticalScroll (widget, n)
DXmCSTextWidget widget;
int n;
#else
void DXmCSTextVerticalScroll (DXmCSTextWidget widget,
			      int 	      n)
#endif /* _NO_PROTO */
{
  PendingVScroll (widget) = PendingVScroll (widget) + n;

  DXmCSTextDisableRedisplay ( widget, True );

  if (PendingVScroll (widget) != 0)
    {
      (*OutputMethod (widget, VerticalScroll)) (widget);
    }

  DXmCSTextEnableRedisplay( widget );

}



/* scroll text by n pixels.  Positive n means scroll to right; negative 
 * to left
 */

#ifdef _NO_PROTO
void DXmCSTextHorizontalScroll (widget, n)
DXmCSTextWidget widget;
int n;
#else
void DXmCSTextHorizontalScroll (DXmCSTextWidget widget,
				int 		n)
#endif /* _NO_PROTO */
{
  PendingHScroll (widget) += n;

  DXmCSTextDisableRedisplay ( widget, True );

  if (PendingHScroll (widget) != 0)
    {
      (*OutputMethod (widget, HorizontalScroll)) (widget);
    }

  DXmCSTextEnableRedisplay( widget );

}




#ifdef _NO_PROTO
void DXmCSTextDisableRedisplay(widget, losesbackingstore)
DXmCSTextWidget widget;
Boolean losesbackingstore;
#else
void DXmCSTextDisableRedisplay(DXmCSTextWidget widget,
			       Boolean 	       losesbackingstore)
#endif /* _NO_PROTO */
{
    CSTextDisableDepth (widget)++;

/*    if ( losesbackingstore )
        (*OutputMethod (widget, SetInsertionPoint))
			     ( widget, CursorPos (widget) );
*/
}



#ifdef _NO_PROTO
void DXmCSTextEnableRedisplay(widget)
DXmCSTextWidget widget;
#else
void DXmCSTextEnableRedisplay(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
    if ( CSTextDisableDepth (widget) > 0 )
    { 
       CSTextDisableDepth (widget)--;
    }

    if ( CSTextDisableDepth (widget) == 0 )
    {
        Redisplay( widget );
    }
}




/*
 * Mark the given range of text to be redrawn.  Note one degenerate case:
 * If left is widget->cstext.last_position, we treat it as being
 * widget->cstext.last_position - 1.  That way, we are always sure
 * to redraw things at the end.
 */

static void 
#ifdef _NO_PROTO
AddRedraw (widget, left, right)
    DXmCSTextWidget widget; 
    DXmCSTextPosition left, right;
#else
AddRedraw(
    DXmCSTextWidget widget,
    DXmCSTextPosition left,
    DXmCSTextPosition right )
#endif /* _NO_PROTO */
{
  TextLocationRec left_location, right_location;
  int           i, j;

    if ( left < right )
    {
      _DXmCSTextSourceLocate ( widget, left,    &left_location );
      _DXmCSTextSourceLocate ( widget, right+1, &right_location );

      j = right_location.position - left_location.position;

      for (i = 0; 
	   i < j;
	   i++, _DXmCSTextNavNextChar (&left_location))
	{
	  (*OutputMethod (widget, SetCharRedraw)) (widget, &left_location);
	}
    }
}

#ifdef _NO_PROTO
void DXmCSTextMarkRedraw (widget, left, right)
DXmCSTextWidget   widget;
DXmCSTextPosition left, right;
#else
void DXmCSTextMarkRedraw (DXmCSTextWidget   widget,
			  DXmCSTextPosition left,
			  DXmCSTextPosition right)
#endif /* _NO_PROTO */
{
    /*  The semantics here should be start and end instead of left and 
        right
    */
    if ( left < right )
    {
 	AddRedraw ( widget, left, right );

	Redisplay( widget );
    }
}

/*
 * very similar to add redraw, simply loop across the range asking
 * the output guy to highlighh each character
 */
#ifdef _NO_PROTO
void DXmCSTextSetHighlight(widget, left, right, mode)
DXmCSTextWidget   widget;
DXmCSTextPosition left, right;
XmHighlightMode    mode;
#else
void DXmCSTextSetHighlight(DXmCSTextWidget   widget,
			   DXmCSTextPosition left,
			   DXmCSTextPosition right,
			   XmHighlightMode   mode)
#endif /* _NO_PROTO */
{
  DXmCSTextPosition temp;
  TextLocationRec left_location, right_location;

  if (left > right)
  {
    temp  = left;
    left  = right;
    right = temp;
  }

  _DXmCSTextSourceLocate (widget, left,  &left_location);
  (*OutputMethod(widget, SetCharDrawMode))(widget,
					   &left_location,
					   (unsigned int) right - left,
					   mode);
  
  Redisplay( widget );
}

#ifdef _NO_PROTO
DXmCSTextPosition DXmCSTextGetTopPosition(widget)
DXmCSTextWidget widget;
#else
DXmCSTextPosition DXmCSTextGetTopPosition(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
  return (TopPos (widget));
}
    
#ifdef _NO_PROTO
void DXmCSTextSetTopPosition(widget, top_position)
DXmCSTextWidget   widget;
DXmCSTextPosition top_position;
#else
void DXmCSTextSetTopPosition(DXmCSTextWidget   widget,
			     DXmCSTextPosition top_position)
#endif /* _NO_PROTO */
{
  if ( top_position != TopPos(widget) )
    {
      int new_top_line, old_top_line;

      new_top_line = DXmCSTextPosToLine( widget, top_position   );
      old_top_line = DXmCSTextPosToLine( widget, TopPos(widget) );

      DXmCSTextVerticalScroll( widget, old_top_line - new_top_line );
    }
}

#ifdef _NO_PROTO
DXmCSTextPosition DXmCSTextGetLastPosition(widget)
DXmCSTextWidget widget;
#else
DXmCSTextPosition DXmCSTextGetLastPosition(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
    return (DXmCSTextPosition)widget->cstext.length;
}

#ifdef _NO_PROTO
DXmCSTextPosition DXmCSTextGetCursorPosition(widget)
DXmCSTextWidget widget;
#else
DXmCSTextPosition DXmCSTextGetCursorPosition(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
    return (CursorPos (widget));
}

#ifdef _NO_PROTO
DXmCSTextPosition DXmCSTextGetInsertionPosition(widget)
DXmCSTextWidget widget;
#else
DXmCSTextPosition DXmCSTextGetInsertionPosition(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
    return (CursorPos (widget));
}



#ifdef _NO_PROTO
void DXmCSTextSetInsertionPosition(widget, position)
DXmCSTextWidget   widget;
DXmCSTextPosition position;
#else
void DXmCSTextSetInsertionPosition(DXmCSTextWidget   widget,
				   DXmCSTextPosition position)
#endif /* _NO_PROTO */
{
    DXmCSTextSetCursorPosition( widget, position );
}

#ifdef _NO_PROTO
void DXmCSTextSetCursorPosition(widget, position)
DXmCSTextWidget   widget;
DXmCSTextPosition position;
#else
void DXmCSTextSetCursorPosition(DXmCSTextWidget   widget,
				DXmCSTextPosition position)
#endif /* _NO_PROTO */
{
    DXmCSTextVerifyCallbackStruct cb;

    if (CursorPos (widget) != position )
    {
        /* Call Motion Verify Callback before Cursor Changes Positon
        */
        cb.reason     = XmCR_MOVING_INSERT_CURSOR;
        cb.event      = NULL;
        cb.currInsert = CursorPos (widget);
        cb.newInsert  = position;
        cb.doit       = True;
        cb.text       = (XmString) NULL;

        XtCallCallbackList( (Widget)widget, (XtCallbackList)MotionCB (widget), &cb );

	/* Cancel action upon application request */
        if ( !cb.doit ) {
	    if (CSTextVerifyBell(widget)) XBell(XtDisplay(widget), 0);
	    return;
	}

        CursorPos (widget) = position;
    }
	
    (*OutputMethod (widget, SetInsertionPoint)) ( widget, CursorPos (widget) );
}


/*
 * Return the line number containing the given position.
 */

#ifdef _NO_PROTO
int DXmCSTextPosToLine (widget, position)
DXmCSTextWidget   widget;
DXmCSTextPosition position;
#else
int DXmCSTextPosToLine (DXmCSTextWidget   widget,
			DXmCSTextPosition position)
#endif /* _NO_PROTO */
{
  TextLocationRec location;
  int i;

  _DXmCSTextSourceLocate (widget, position, &location);

  /* chage api to handle logical line.  Needed in word wrap
   */
  (*OutputMethod (widget, ComputeLineIndex)) (widget, &location, &i);

  return (i);
}



/*
 * Return the number of lines in the widget
 */

#ifdef _NO_PROTO
int DXmCSTextNumLines(widget)
DXmCSTextWidget widget;
#else
int DXmCSTextNumLines(DXmCSTextWidget widget)
#endif /* _NO_PROTO */
{
    int num_lines;

      (*OutputMethod (widget, NumLinesOnText)) (widget, &num_lines);

    return (num_lines);
}




#ifdef _NO_PROTO
void DXmCSTextInvalidate (widget, left, right)
DXmCSTextWidget   widget;
DXmCSTextPosition left, right;
#else
void DXmCSTextInvalidate (DXmCSTextWidget   widget,
			  DXmCSTextPosition left,
			  DXmCSTextPosition right)
#endif /* _NO_PROTO */
{
  (*OutputMethod (widget, Invalidate)) (widget, left, right);
  (*InputMethod  (widget, Invalidate)) (widget, left, right);

  DXmCSTextMarkRedraw (widget, left, right);
}




static void
#ifdef _NO_PROTO
Destroy(w)
      Widget w;
#else
Destroy(
      Widget w )
#endif /* _NO_PROTO */
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    _DXmCSTextSourceDestroy ( widget );

    if (InputMethod (widget, destroy))
       (*InputMethod  (widget, destroy)) ((Widget)widget);

    if (OutputMethod (widget, Destroy))
       (*OutputMethod (widget, Destroy)) (widget);

    if( CSTextValue( widget ) != (XmString)NULL )
    {
	XmStringFree( CSTextValue( widget ) );
	CSTextValue(widget) = (XmString) NULL;
    }

    if( CSTextDefaultCharSet( widget ) != (XmStringCharSet) NULL )
    {
	XtFree (CSTextDefaultCharSet(widget));
	CSTextDefaultCharSet(widget) = (XmStringCharSet) NULL;
    }

    XtRemoveAllCallbacks ( w, XmNactivateCallback );
    XtRemoveAllCallbacks ( w, XmNfocusCallback );
    XtRemoveAllCallbacks ( w, XmNlosingFocusCallback );
    XtRemoveAllCallbacks ( w, XmNvalueChangedCallback );
    XtRemoveAllCallbacks ( w, XmNmodifyVerifyCallback );
    XtRemoveAllCallbacks ( w, XmNmotionVerifyCallback );
}

static void
#ifdef _NO_PROTO
Resize(w)
      Widget w;
#else
Resize(
      Widget w )
#endif /* _NO_PROTO */
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;

    (*OutputMethod (widget, Resize)) ( widget );
}

static void
#ifdef _NO_PROTO
DoExpose(w, event)
      Widget w;
      XEvent *event;
#else
DoExpose(
      Widget w,
      XEvent *event )
#endif /* _NO_PROTO */
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;

    (*OutputMethod (widget, Redisplay)) ( widget, event );
}

#ifdef _NO_PROTO
DXmCSTextSource DXmCSTextGetSource(widget)
	Widget   widget;
#else
DXmCSTextSource DXmCSTextGetSource(
	Widget   widget)
#endif /* _NO_PROTO */
{
   /* NYI */
}



#ifdef _NO_PROTO
void DXmCSTextSetSource(widget, source, top_character, cursor_position)
	Widget   widget;
        DXmCSTextSource source;
        DXmCSTextPosition top_character;
        DXmCSTextPosition cursor_position;
#else
void DXmCSTextSetSource(
	Widget   widget,
        DXmCSTextSource source ,
        DXmCSTextPosition top_character,
        DXmCSTextPosition cursor_position)
#endif  /* _NO_PROTO */
{
    /* NYI */
}

#ifdef _NO_PROTO
Boolean DXmCSTextFindString(w, start, search_string, direction, position)
        Widget w;
        DXmCSTextPosition start;
        XmString search_string;
        XmTextDirection direction;
        XmTextPosition *position;
#else
Boolean DXmCSTextFindString(
        Widget w,
        DXmCSTextPosition start,
        XmString search_string,
        XmTextDirection direction,
        XmTextPosition *position )
#endif  /* _NO_PROTO */
{
    /* NYI */
}


#ifdef _NO_PROTO
int DXmCSTextGetSubstring(widget, start, num_chars, buf_size, buffer)
	Widget   widget;
	DXmCSTextPosition start;
	int num_chars;
	int buf_size;
	XmString buffer;
#else
int DXmCSTextGetSubstring(
	Widget   widget,
	DXmCSTextPosition start,
	int num_chars,
	int buf_size,
	XmString buffer )
#endif /* _NO_PROTO */
{
   /* NYI */
}



/*
 * Check each output line in the widget and return it's baseline in an 
 * array.
 */

Boolean 
#ifdef _NO_PROTO
_DXmCSTextGetBaselines( widget, baselines, line_count )
        Widget widget ;
	Dimension **baselines;
	int *line_count;
#else
_DXmCSTextGetBaselines(
        Widget widget,
	Dimension **baselines,
	int *line_count )
#endif /* _NO_PROTO */
{
   DXmCSTextWidget w = (DXmCSTextWidget) widget;
   DXmCSTextOutputData data = w->cstext.output->data;
   Dimension *base_array;
   Dimension marginHeight, lineHeight;
   int i;

   marginHeight = w->cstext.margin_height;
   lineHeight = data->max_ascent + data->max_descent;
   *line_count = data->line_count;	/* Number of output lines */

   base_array = (Dimension *)XtMalloc((sizeof(Dimension) * (*line_count)));

   for (i = 0; i < *line_count; i++) {
       base_array[i] = marginHeight + i * lineHeight + data->max_ascent;
   }

   *baselines = base_array;

   return (TRUE);
}


int 
#ifdef _NO_PROTO
DXmCSTextGetBaseline( widget )
        Widget widget ;
#else
DXmCSTextGetBaseline(
        Widget widget )
#endif /* _NO_PROTO */
{
    Dimension *baselines;
    int temp_bl;
    int line_count;

    (void) _DXmCSTextGetBaselines(widget, &baselines, &line_count);

    if (line_count)
      temp_bl = (int) baselines[0];
    else
      temp_bl = 0;

    XtFree((char *) baselines);
    return (temp_bl);
}	
