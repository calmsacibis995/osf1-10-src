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
static char rcsid[] = "$RCSfile: FileSB.c,v $ $Revision: 1.1.6.5 $ $Date: 1993/12/17 21:19:24 $"
#endif
#endif
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include "XmI.h"

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif

#include "RepTypeI.h"
#include <Xm/FileSBP.h>
#include <Xm/GadgetP.h>
#include <Xm/XmosP.h>
#include <Xm/AtomMgr.h>

#include <Xm/List.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumnP.h>
#include <Xm/ArrowB.h>
#ifndef USE_TEXT_IN_DIALOGS
#include <Xm/TextF.h>
#else
#include <Xm/Text.h>
#endif
#include <Xm/DialogS.h>
#include <Xm/VendorSEP.h>
#include <Xm/DragC.h>
#include <Xm/DropSMgr.h>
#include <Xm/Protocols.h>
#ifdef I18N_EXTENSION
#include <DXm/DXmCSText.h>
#include <DXm/DECspecific.h>
#endif /* I18N_EXTENSION */

#ifdef I18N_MULTIBYTE
#include <X11/DECwI18n.h>
#include "I18nConverter.h"
XmString XmGetLocaleString();
#endif

#ifdef DEC_MOTIF_EXTENSION
#ifdef VMS
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif
#endif


#define IsButton(w) ( \
      XmIsPushButton(w)   || XmIsPushButtonGadget(w)   || \
      XmIsToggleButton(w) || XmIsToggleButtonGadget(w) || \
      XmIsArrowButton(w)  || XmIsArrowButtonGadget(w)  || \
      XmIsDrawnButton(w))

#define IsAutoButton(fsb, w) (                \
      w == SB_OkButton(fsb) ||                \
      w == SB_ApplyButton(fsb) ||     \
      w == SB_CancelButton(fsb) ||    \
      w == SB_HelpButton(fsb))

#define SetupWorkArea(fsb) \
    if (_XmGeoSetupKid (boxPtr, SB_WorkArea(fsb)))    \
    {                                                 \
        layoutPtr->space_above = vspace;              \
        vspace = BB_MarginHeight(fsb);                \
        boxPtr += 2 ;                                 \
        ++layoutPtr ;                                 \
    }
 
typedef struct
    {   XmKidGeometry dir_list_label ;
        XmKidGeometry file_list_label ;
        Dimension   prefer_width ;
        Dimension   delta_width ;
        } FS_GeoExtensionRec, *FS_GeoExtension ;


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void ClassPartInitialize() ;
static void Initialize() ;
static void Destroy() ;
static void DeleteChild() ;
static void FSBCreateFilterLabel() ;
static void FSBCreateDirListLabel() ;
static void FSBCreateDirList() ;
static void FSBCreateFilterText() ;
static XmGeoMatrix FileSBGeoMatrixCreate() ;
static Boolean FileSelectionBoxNoGeoRequest() ;
static void ListLabelFix() ;
static void ListFix() ;
static void FileSearchProc() ;
static void QualifySearchDataProc() ;
static void FileSelectionBoxUpdate() ;
static void DirSearchProc() ;
static void ListCallback() ;
static Boolean SetValues() ;
static void FSBGetDirectory() ;
static void FSBGetNoMatchString() ;
static void FSBGetPattern() ;
static void FSBGetFilterLabelString() ;
static void FSBGetDirListLabelString() ;
static void FSBGetDirListItems() ;
static void FSBGetDirListItemCount() ;
static void FSBGetListItems() ;
static void FSBGetListItemCount() ;
static void FSBGetDirMask() ;
static Widget GetActiveText() ;
static void FileSelectionBoxUpOrDown() ;
static void FileSelectionBoxRestore() ;
static void FileSelectionBoxFocusMoved() ;
static void FileSelectionPB() ;
#ifdef DEC_MOTIF_EXTENSION
static Cursor           CreateWaitCursor();
#endif
#else

static void ClassPartInitialize( 
                        WidgetClass fsc) ;
static void Initialize( 
                        Widget rw,
                        Widget nw,
                        ArgList args_in,
                        Cardinal *num_args) ;
static void Destroy( 
                        Widget fsb) ;
static void DeleteChild( 
                        Widget w) ;
static void FSBCreateFilterLabel( 
                        XmFileSelectionBoxWidget fsb) ;
static void FSBCreateDirListLabel( 
                        XmFileSelectionBoxWidget fsb) ;
static void FSBCreateDirList( 
                        XmFileSelectionBoxWidget fsb) ;
static void FSBCreateFilterText( 
                        XmFileSelectionBoxWidget fs) ;
static XmGeoMatrix FileSBGeoMatrixCreate( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *desired) ;
static Boolean FileSelectionBoxNoGeoRequest( 
                        XmGeoMatrix geoSpec) ;
static void ListLabelFix( 
                        XmGeoMatrix geoSpec,
                        int action,
                        XmGeoMajorLayout layoutPtr,
                        XmKidGeometry rowPtr) ;
static void ListFix( 
                        XmGeoMatrix geoSpec,
                        int action,
                        XmGeoMajorLayout layoutPtr,
                        XmKidGeometry rowPtr) ;
static void FileSearchProc( 
                        Widget w,
                        XtPointer sd) ;
static void QualifySearchDataProc( 
                        Widget w,
                        XtPointer sd,
                        XtPointer qsd) ;
static void FileSelectionBoxUpdate( 
                        XmFileSelectionBoxWidget fs,
                        XmFileSelectionBoxCallbackStruct *searchData) ;
static void DirSearchProc( 
                        Widget w,
                        XtPointer sd) ;
static void ListCallback( 
                        Widget wid,
                        XtPointer client_data,
                        XtPointer call_data) ;
static Boolean SetValues( 
                        Widget cw,
                        Widget rw,
                        Widget nw,
                        ArgList args_in,
                        Cardinal *num_args) ;
static void FSBGetDirectory( 
                        Widget fs,
                        int resource,
                        XtArgVal *value) ;
static void FSBGetNoMatchString( 
                        Widget fs,
                        int resource,
                        XtArgVal *value) ;
static void FSBGetPattern( 
                        Widget fs,
                        int resource,
                        XtArgVal *value) ;
static void FSBGetFilterLabelString( 
                        Widget fs,
                        int resource_offset,
                        XtArgVal *value) ;
static void FSBGetDirListLabelString( 
                        Widget fs,
                        int resource_offset,
                        XtArgVal *value) ;
static void FSBGetDirListItems( 
                        Widget fs,
                        int resource_offset,
                        XtArgVal *value) ;
static void FSBGetDirListItemCount( 
                        Widget fs,
                        int resource_offset,
                        XtArgVal *value) ;
static void FSBGetListItems( 
                        Widget fs,
                        int resource_offset,
                        XtArgVal *value) ;
static void FSBGetListItemCount( 
                        Widget fs,
                        int resource_offset,
                        XtArgVal *value) ;
static void FSBGetDirMask( 
                        Widget fs,
                        int resource_offset,
                        XtArgVal *value) ;
static Widget GetActiveText( 
                        XmFileSelectionBoxWidget fsb,
                        XEvent *event) ;
static void FileSelectionBoxUpOrDown( 
                        Widget wid,
                        XEvent *event,
                        String *argv,
                        Cardinal *argc) ;
static void FileSelectionBoxRestore( 
                        Widget wid,
                        XEvent *event,
                        String *argv,
                        Cardinal *argc) ;
static void FileSelectionBoxFocusMoved( 
                        Widget wid,
                        XtPointer client_data,
                        XtPointer data) ;
static void FileSelectionPB( 
                        Widget wid,
                        XtPointer which_button,
                        XtPointer call_data) ;
#ifdef DEC_MOTIF_EXTENSION
static Cursor           CreateWaitCursor(Widget wid);
#endif

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/*
 * transfer vector from translation manager action names to
 * address of routines 
 */
 
static XtActionsRec ActionsTable[] =
{
    { "UpOrDown", FileSelectionBoxUpOrDown }, /* Motif 1.0 */
    { "SelectionBoxUpOrDown", FileSelectionBoxUpOrDown },
    { "SelectionBoxRestore", FileSelectionBoxRestore },
    };
 

/*---------------------------------------------------*/
/* widget resources                                  */
/*---------------------------------------------------*/
static XtResource resources[] = 
{
    /* fileselection specific resources */
 
	{	XmNdirectory,
		XmCDirectory,
		XmRXmString,
		sizeof( XmString),
		XtOffsetOf( struct _XmFileSelectionBoxRec, 
                                                 file_selection_box.directory),
		XmRXmString,
		(XtPointer) NULL    /* This will initialize to the current   */
	},                          /*   directory, because of XmNdirMask.   */
	{	XmNpattern,
		XmCPattern,
		XmRXmString,
		sizeof( XmString), 
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                                   file_selection_box.pattern),
                XmRImmediate,
                (XtPointer) NULL  /* This really initializes to "*", because */
	},                        /*   of interaction with "XmNdirMask".     */
	{	XmNdirListLabelString, 
		XmCDirListLabelString, 
		XmRXmString, 
		sizeof (XmString), 
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                     file_selection_box.dir_list_label_string),
		XmRImmediate,
                (XtPointer) XmUNSPECIFIED
	},
        {       XmNdirListItems,
                XmCDirListItems,
                XmRXmStringTable,
                sizeof( XmStringTable),
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                            file_selection_box.dir_list_items),
                XmRImmediate,
                (XtPointer) NULL
        },
        {       XmNdirListItemCount,
                XmCDirListItemCount,
                XmRInt,
                sizeof( int),
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                       file_selection_box.dir_list_item_count),
                XmRImmediate,
                (XtPointer) XmUNSPECIFIED
        },
	{	XmNfilterLabelString, 
		XmCFilterLabelString, 
		XmRXmString, 
		sizeof (XmString), 
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                       file_selection_box.filter_label_string),
		XmRImmediate,
                (XtPointer) XmUNSPECIFIED
	},
	{	XmNdirMask, 
		XmCDirMask, 
		XmRXmString, 
		sizeof (XmString), 
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                                  file_selection_box.dir_mask),
		XmRImmediate,
                (XtPointer) XmUNSPECIFIED
	},
	{	XmNnoMatchString, 
		XmCNoMatchString, 
		XmRXmString, 
		sizeof (XmString), 
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                           file_selection_box.no_match_string),
		XmRImmediate,
                (XtPointer) XmUNSPECIFIED
	},
	{	XmNqualifySearchDataProc,
		XmCQualifySearchDataProc,
		XmRProc, 
		sizeof(XtProc),
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                  file_selection_box.qualify_search_data_proc),
		XmRImmediate,
		(XtPointer) QualifySearchDataProc
	},
	{	XmNdirSearchProc,
		XmCDirSearchProc,
		XmRProc, 
		sizeof(XtProc),
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                           file_selection_box.dir_search_proc),
		XmRImmediate,
		(XtPointer) DirSearchProc
	},
	{	XmNfileSearchProc, 
		XmCFileSearchProc,
		XmRProc,
		sizeof(XtProc),
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                          file_selection_box.file_search_proc),
		XmRImmediate,
		(XtPointer) FileSearchProc
	},
	{	XmNfileTypeMask,
		XmCFileTypeMask,
		XmRFileTypeMask,
		sizeof( unsigned char),
		XtOffsetOf( struct _XmFileSelectionBoxRec, 
                                            file_selection_box.file_type_mask),
		XmRImmediate,
		(XtPointer) XmFILE_REGULAR
	}, 
	{	XmNlistUpdated,
		XmCListUpdated,
		XmRBoolean,
		sizeof(Boolean),
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                              file_selection_box.list_updated),
		XmRImmediate,
		(XtPointer) TRUE
	},
	{	XmNdirectoryValid,
		XmCDirectoryValid,
		XmRBoolean,
		sizeof(Boolean),
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                           file_selection_box.directory_valid),
		XmRImmediate,
		(XtPointer) TRUE
	},
	/* superclass resource default overrides */

	{	XmNdirSpec,
		XmCDirSpec,
		XmRXmString,
		sizeof( XmString),
		XtOffsetOf( struct _XmFileSelectionBoxRec, selection_box.text_string),
		XmRImmediate,
		(XtPointer) XmUNSPECIFIED
	},                                        
	{	XmNautoUnmanage,
		XmCAutoUnmanage,
		XmRBoolean,
		sizeof(Boolean),
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                                 bulletin_board.auto_unmanage),
		XmRImmediate,
		(XtPointer) FALSE
	},
	{	XmNfileListLabelString,
		XmCFileListLabelString,
		XmRXmString,
		sizeof(XmString),
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                              selection_box.list_label_string),
		XmRImmediate,
		(XtPointer) XmUNSPECIFIED
	},
	{	XmNapplyLabelString,
		XmCApplyLabelString,
		XmRXmString,
		sizeof(XmString),
		XtOffsetOf( struct _XmFileSelectionBoxRec,
                                             selection_box.apply_label_string),
		XmRImmediate,
		(XtPointer) XmUNSPECIFIED
	},
	{	XmNdialogType,
		XmCDialogType,
		XmRSelectionType,
		sizeof(unsigned char),
		XtOffsetOf( struct _XmFileSelectionBoxRec, selection_box.dialog_type),
		XmRImmediate,
		(XtPointer) XmDIALOG_FILE_SELECTION
	},
	{	XmNfileListItems, 
		XmCItems, XmRXmStringTable, sizeof (XmString *), 
		XtOffsetOf( struct _XmSelectionBoxRec, selection_box.list_items), 
		XmRImmediate, NULL
	},                                        
	{	XmNfileListItemCount, 
		XmCItemCount, XmRInt, sizeof(int), 
		XtOffsetOf( struct _XmSelectionBoxRec, selection_box.list_item_count), 
		XmRImmediate, (XtPointer) XmUNSPECIFIED
	}, 

};

static XmSyntheticResource syn_resources[] =
{
  {	XmNdirectory,
	sizeof (XmString),
	XtOffsetOf( struct _XmFileSelectionBoxRec, file_selection_box.directory),
	FSBGetDirectory,
	(XmImportProc)NULL
  },
  {	XmNdirListLabelString,
	sizeof (XmString), 
	XtOffsetOf( struct _XmFileSelectionBoxRec,
		 file_selection_box.dir_list_label_string),
	FSBGetDirListLabelString,
	(XmImportProc)NULL
  },
  {     XmNdirListItems,
        sizeof( XmString *),
	XtOffsetOf( struct _XmFileSelectionBoxRec, file_selection_box.dir_list_items),
        FSBGetDirListItems,
        (XmImportProc)NULL
  },
  {    XmNdirListItemCount,
        sizeof( int),
	XtOffsetOf( struct _XmFileSelectionBoxRec,
		 file_selection_box.dir_list_item_count),
        FSBGetDirListItemCount,
        (XmImportProc)NULL
  },
  {	XmNfilterLabelString,
	sizeof (XmString), 
	XtOffsetOf( struct _XmFileSelectionBoxRec,
		 file_selection_box.filter_label_string),
	FSBGetFilterLabelString,
	(XmImportProc)NULL
  },
  {	XmNdirMask,
	sizeof( XmString), 
	XtOffsetOf( struct _XmFileSelectionBoxRec, file_selection_box.dir_mask),
	FSBGetDirMask,
	(XmImportProc)NULL
  },
  {	XmNdirSpec,
	sizeof (XmString), 
	XtOffsetOf( struct _XmFileSelectionBoxRec, selection_box.text_string),
	_XmSelectionBoxGetTextString,
	(XmImportProc)NULL
  },
  {	XmNfileListLabelString,
	sizeof (XmString), 
	XtOffsetOf( struct _XmFileSelectionBoxRec, selection_box.list_label_string),
	_XmSelectionBoxGetListLabelString,
	(XmImportProc)NULL
  },
  {	XmNfileListItems, 
	sizeof (XmString *), 
	XtOffsetOf( struct _XmSelectionBoxRec, selection_box.list_items), 
	FSBGetListItems,
	(XmImportProc)NULL
  },                                        
  {	XmNfileListItemCount, 
	sizeof(int), 
	XtOffsetOf( struct _XmSelectionBoxRec, selection_box.list_item_count),
	FSBGetListItemCount,
	(XmImportProc)NULL
  }, 
  {	XmNnoMatchString, 
	sizeof (XmString), 
	XtOffsetOf( struct _XmFileSelectionBoxRec,
		 file_selection_box.no_match_string),
	FSBGetNoMatchString,
	(XmImportProc)NULL
  },
  {	XmNpattern,
	sizeof( XmString), 
	XtOffsetOf( struct _XmFileSelectionBoxRec,
		 file_selection_box.pattern),
	FSBGetPattern,
	(XmImportProc)NULL
  },  
};
 
externaldef( xmfileselectionboxclassrec) XmFileSelectionBoxClassRec
                                                   xmFileSelectionBoxClassRec =
{
    {   /* core class record        */
	/* superclass	            */	(WidgetClass) &xmSelectionBoxClassRec,
	/* class_name		    */	"XmFileSelectionBox",
	/* widget_size		    */	sizeof(XmFileSelectionBoxRec),
	/* class_initialize	    */	(XtProc)NULL,
	/* class part init          */	ClassPartInitialize,
	/* class_inited		    */	FALSE,
	/* initialize		    */	Initialize,
	/* initialize hook	    */	(XtArgsProc)NULL,
	/* realize		    */	XtInheritRealize,
	/* actions		    */	ActionsTable,
	/* num_actions		    */	XtNumber(ActionsTable),
	/* resources		    */	resources,
	/* num_resources	    */	XtNumber(resources),
	/* xrm_class		    */	NULLQUARK,
	/* compress_motion	    */	TRUE,
	/* compress_exposure        */	XtExposeCompressMaximal,
	/* compress crossing        */	FALSE,
	/* visible_interest	    */	FALSE,
	/* destroy		    */	Destroy,
	/* resize		    */	XtInheritResize,
	/* expose		    */	XtInheritExpose,
	/* set_values		    */	SetValues,
	/* set_values_hook	    */	(XtArgsFunc)NULL,                    
	/* set_values_almost        */	XtInheritSetValuesAlmost,
	/* get_values_hook	    */	(XtArgsProc)NULL,                    
	/* accept_focus		    */	(XtAcceptFocusProc)NULL,
	/* version		    */	XtVersion,
	/* callback_private         */	(XtPointer)NULL,
	/* tm_table                 */	XtInheritTranslations,
	/* query_geometry	    */	XtInheritQueryGeometry,
	/* display_accelerator	    */	(XtStringProc)NULL,
	/* extension		    */	(XtPointer)NULL,
	},
    {   /* composite class record   */    
	/* geometry manager         */	XtInheritGeometryManager,
	/* set changed proc	    */	XtInheritChangeManaged,
	/* insert_child		    */	XtInheritInsertChild,
	/* delete_child 	    */	DeleteChild,
	/* extension		    */	(XtPointer)NULL,
	},
    {   /* constraint class record  */
	/* no additional resources  */	(XtResourceList)NULL,
	/* num additional resources */	0,
	/* size of constraint rec   */	0,
	/* constraint_initialize    */	(XtInitProc)NULL,
	/* constraint_destroy	    */  (XtWidgetProc)NULL,
	/* constraint_setvalue      */	(XtSetValuesFunc)NULL,
	/* extension                */	(XtPointer)NULL,
	},
    {   /* manager class record     */
	/* translations             */	XtInheritTranslations,
	/* get_resources            */	syn_resources,
	/* num_syn_resources        */	XtNumber(syn_resources),
	/* constraint_syn_resources */	(XmSyntheticResource *)NULL,
	/* num_constraint_syn_resources*/ 0,
        /* parent_process<           */  XmInheritParentProcess,
	/* extension		    */	(XtPointer)NULL,
	},
    {	/* bulletinBoard class record*/
	/* always_install_accelerators*/TRUE,
	/* geo_matrix_create        */	FileSBGeoMatrixCreate,
	/* focus_moved_proc         */	FileSelectionBoxFocusMoved,
	/* extension                */	(XtPointer)NULL,
	},
    {	/*selectionbox class record */
        /* list_callback            */  ListCallback,
	/* extension		    */	(XtPointer)NULL,
	},
    {	/* fileselection class record*/
	/* extension		    */	(XtPointer)NULL,
	}
};

externaldef( xmfileselectionboxwidgetclass) WidgetClass
     xmFileSelectionBoxWidgetClass = (WidgetClass)&xmFileSelectionBoxClassRec ;


/****************************************************************
 * Class Initialization.  Sets up accelerators and fast subclassing.
 ****************/
static void 
#ifdef _NO_PROTO
ClassPartInitialize( fsc )
        WidgetClass fsc ;
#else
ClassPartInitialize(
        WidgetClass fsc )
#endif /* _NO_PROTO */
{
/****************/

    _XmFastSubclassInit( fsc, XmFILE_SELECTION_BOX_BIT) ;

    return ;
    }
#ifdef DEC_MOTIF_RTOL

static void UpdateDirection( sw )
    XmFileSelectionBoxWidget sw;
{

    Arg  arglist[1];
    Widget *kid ;
    int i;

    XtSetArg(arglist[0], DXmNlayoutDirection, LayoutM(sw));
    for (i = 0, kid = sw->composite.children;
         i < sw->composite.num_children;
         i++, kid++)

            if (!(*kid)->core.being_destroyed)
               XtSetValues(*kid, arglist, 1);  /*  Propagate direction */
}
#endif /* DEC_MOTIF_RTOL */



/****************************************************************
 * This routine initializes an instance of the file selection widget.
 * Instance record fields which are shadow resources for child widgets and
 *   which are of an allocated type are set to NULL after they are used, since
 *   the memory identified by them is not owned by the File Selection Box.
 ****************/
static void 
#ifdef _NO_PROTO
Initialize( rw, nw, args_in, num_args )
        Widget rw ;
        Widget nw ;
        ArgList args_in ;
        Cardinal *num_args ;
#else
Initialize(
        Widget rw,
        Widget nw,
        ArgList args_in,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmFileSelectionBoxWidget new_w = (XmFileSelectionBoxWidget) nw ;
    Arg             args[16] ;
    int             numArgs ;
    XmFileSelectionBoxCallbackStruct searchData ;
    XmString local_xmstring ;
/****************/

    FS_StateFlags( new_w) = 0 ;

    /*	Here we have now to take care of XmUNSPECIFIED (CR 4856).
     */  
    if (new_w->selection_box.list_label_string == 
	(XmString) XmUNSPECIFIED) {
	
#ifdef I18N_MULTIBYTE
	local_xmstring = XmGetLocaleString( (I18nContext)NULL,
					"Files", I18NLABEL );
#else
	local_xmstring = XmStringLtoRCreate("Files", 
					    XmFONTLIST_DEFAULT_TAG);
#endif
	numArgs = 0 ;
	XtSetArg( args[numArgs], XmNlabelString, local_xmstring) ; ++numArgs ;
	XtSetValues( SB_ListLabel( new_w), args, numArgs) ;
	XmStringFree(local_xmstring);

	new_w->selection_box.list_label_string = NULL ;
    }
	   
    if (new_w->selection_box.apply_label_string == 
	(XmString) XmUNSPECIFIED) {
	
#ifdef I18N_MULTIBYTE
	local_xmstring = XmGetLocaleString( (I18nContext)NULL,
					"Filter", I18NLABEL );
#else
	local_xmstring = XmStringLtoRCreate("Filter", 
					    XmFONTLIST_DEFAULT_TAG);
#endif
	numArgs = 0 ;
	XtSetArg( args[numArgs], XmNlabelString, local_xmstring) ; ++numArgs ;
	XtSetValues( SB_ApplyButton( new_w), args, numArgs) ;
	XmStringFree(local_xmstring);

	new_w->selection_box.list_label_string = NULL ;
    }


    /* must set adding_sel_widgets to avoid adding these widgets to 
     * selection work area
     */
    SB_AddingSelWidgets( new_w) = TRUE ;

    if(    !(SB_ListLabel( new_w))    )
    {   _XmSelectionBoxCreateListLabel( (XmSelectionBoxWidget) new_w) ;
        } 
    if(    !(SB_List( new_w))    )
    {   _XmSelectionBoxCreateList( (XmSelectionBoxWidget) new_w) ;
        } 
    if(    !(SB_SelectionLabel( new_w))    )
    {   _XmSelectionBoxCreateSelectionLabel( (XmSelectionBoxWidget) new_w) ;
        } 
    if(    !(SB_Text( new_w))    )
    {   _XmSelectionBoxCreateText( (XmSelectionBoxWidget) new_w) ;
        } 
    if(    !(SB_ApplyButton( new_w))    )
    {   _XmSelectionBoxCreateApplyButton( (XmSelectionBoxWidget) new_w) ;
        } 
    if(    !(SB_OkButton( new_w))    )
    {   _XmSelectionBoxCreateOkButton( (XmSelectionBoxWidget) new_w) ;
        } 
    if(    !(SB_CancelButton( new_w))    )
    {   _XmSelectionBoxCreateCancelButton( (XmSelectionBoxWidget) new_w) ;
        } 
    if(    !(SB_HelpButton( new_w))    )
    {   _XmSelectionBoxCreateHelpButton( (XmSelectionBoxWidget) new_w) ;
        } 

    numArgs = 0 ;
    XtSetArg( args[numArgs], XmNscrollBarDisplayPolicy, XmSTATIC) ; ++numArgs ;
    XtSetValues( SB_List( new_w), args, numArgs) ;


    if (FS_FilterLabelString( new_w) == (XmString) XmUNSPECIFIED) {
	
#ifdef I18N_MULTIBYTE
	FS_FilterLabelString( new_w) = XmGetLocaleString
				( (I18nContext)NULL, "Filter", I18NLABEL);
#else
	FS_FilterLabelString( new_w) = XmStringLtoRCreate("Filter", 
					    XmFONTLIST_DEFAULT_TAG);
#endif
	FSBCreateFilterLabel( new_w) ;
	XmStringFree(FS_FilterLabelString( new_w));

    } else 
	FSBCreateFilterLabel( new_w) ;
    FS_FilterLabelString( new_w) = NULL ;
	
    
    if (FS_DirListLabelString( new_w) == (XmString) XmUNSPECIFIED) {
	
#ifdef I18N_MULTIBYTE
	FS_DirListLabelString( new_w) = XmGetLocaleString
				( (I18nContext)NULL, "Directories", I18NLABEL);
#else
	FS_DirListLabelString( new_w) = XmStringLtoRCreate("Directories", 
					    XmFONTLIST_DEFAULT_TAG);
#endif
	FSBCreateDirListLabel( new_w) ;
	XmStringFree(FS_DirListLabelString( new_w));

    } else 
	FSBCreateDirListLabel( new_w) ;
    FS_DirListLabelString( new_w) = NULL ;

    

    FSBCreateFilterText( new_w);

    FSBCreateDirList( new_w) ;

    /* Since the DirSearchProc is going to be run during initialize,
    *   and since it has the responsibility to manage the directory list and
    *   the filter text, any initial values of the following resources can
    *   be ignored, since they will be immediately over-written.
    */
    FS_DirListItems( new_w) = NULL ;  /* Set/Get Values only.*/
    FS_DirListItemCount( new_w) = XmUNSPECIFIED ; /* Set/Get Values only.*/

    SB_AddingSelWidgets( new_w) = FALSE;

    /* Remove the activate callbacks that our superclass
    *   may have attached to these buttons
    */
    XtRemoveAllCallbacks( SB_ApplyButton( new_w), XmNactivateCallback) ;
    XtRemoveAllCallbacks( SB_OkButton( new_w), XmNactivateCallback) ;
    XtRemoveAllCallbacks( SB_CancelButton( new_w), XmNactivateCallback) ;
    XtRemoveAllCallbacks( SB_HelpButton( new_w), XmNactivateCallback) ;

    XtAddCallback( SB_ApplyButton( new_w), XmNactivateCallback,
                          FileSelectionPB, (XtPointer) XmDIALOG_APPLY_BUTTON) ;
    XtAddCallback( SB_OkButton( new_w), XmNactivateCallback,
                             FileSelectionPB, (XtPointer) XmDIALOG_OK_BUTTON) ;
    XtAddCallback( SB_CancelButton( new_w), XmNactivateCallback,
                         FileSelectionPB, (XtPointer) XmDIALOG_CANCEL_BUTTON) ;
    XtAddCallback( SB_HelpButton( new_w), XmNactivateCallback,
                           FileSelectionPB, (XtPointer) XmDIALOG_HELP_BUTTON) ;


    if( FS_NoMatchString( new_w) == (XmString) XmUNSPECIFIED) {
	FS_NoMatchString( new_w) = XmStringLtoRCreate(" [    ] ", 
						      XmFONTLIST_DEFAULT_TAG);
    }
    else {   
	FS_NoMatchString( new_w) = XmStringCopy( FS_NoMatchString( new_w)) ;
    } 

    searchData.reason = XmCR_NONE ;
    searchData.event = NULL ;
    searchData.value = NULL ;
    searchData.length = 0 ;
    searchData.mask = NULL ;
    searchData.mask_length = 0 ;
    searchData.dir = NULL ;
    searchData.dir_length = 0 ;
    searchData.pattern = NULL ;
    searchData.pattern_length = 0 ;

    /* The XmNdirSpec resource will be loaded into the Text widget by
    *   the Selection Box (superclass) Initialize routine.  It will be 
    *   picked-up there by the XmNqualifySearchDataProc routine to fill
    *   in the value field of the search data.
    */

    if(FS_DirMask( new_w) != (XmString) XmUNSPECIFIED    )
    {   
        searchData.mask = XmStringCopy(FS_DirMask( new_w)) ;
    } else {
#ifdef VMS
	searchData.mask = XmStringLtoRCreate("*.*", XmFONTLIST_DEFAULT_TAG);
#else
	searchData.mask = XmStringLtoRCreate("*", XmFONTLIST_DEFAULT_TAG);
#endif
    }

    searchData.mask_length = XmStringLength( searchData.mask) ;

        /* The DirMask field will be set after subsequent call to
        *   the DirSearchProc.  Set field to NULL to prevent freeing of
        *   memory owned by request.
        */
    FS_DirMask( new_w) = (XmString) XmUNSPECIFIED ;

    if(    FS_Directory( new_w)    )
    {
        searchData.dir = XmStringCopy( FS_Directory( new_w)) ;
        searchData.dir_length = XmStringLength( searchData.dir) ;

        /* The Directory field will be set after subsequent call to
        *   the DirSearchProc.  Set field to NULL to prevent freeing of
        *   memory owned by request.
        */
        FS_Directory( new_w) = NULL ;
        }
    if(    FS_Pattern( new_w)    )
    {
        searchData.pattern = XmStringCopy( FS_Pattern( new_w)) ;
        searchData.pattern_length = XmStringLength( searchData.pattern) ;

        /* The Pattern field will be set after subsequent call to
        *   the DirSearchProc.  Set field to NULL to prevent freeing of
        *   memory owned by request.
        */
        FS_Pattern( new_w) = NULL ;
        }

    if(    !FS_QualifySearchDataProc( new_w)    )
    {   FS_QualifySearchDataProc( new_w) = QualifySearchDataProc ;
        } 
    if(    !FS_DirSearchProc( new_w)    )
    {   FS_DirSearchProc( new_w) = DirSearchProc ;
        } 
    if(    !FS_FileSearchProc( new_w)    )
    {   FS_FileSearchProc( new_w) = FileSearchProc ;
        } 

#ifdef DEC_MOTIF_RTOL
    if (LayoutIsRtoLM(new_w))
        UpdateDirection(new_w);
#endif /* DEC_MOTIF_RTOL */

    FileSelectionBoxUpdate( new_w, &searchData) ;

    XmStringFree( searchData.mask) ;
    XmStringFree( searchData.pattern) ;
    XmStringFree( searchData.dir) ;

    /* Mark everybody as managed because no one else will.
    *   Only need to do this if we are the instantiated class.
    */
    if(    XtClass( new_w) == xmFileSelectionBoxWidgetClass    )
    {   XtManageChildren( new_w->composite.children, 
                                                 new_w->composite.num_children) ;
        } 
    return ;
    }

/****************************************************************/
static void 
#ifdef _NO_PROTO
Destroy( fsb )
        Widget fsb ;
#else
Destroy(
        Widget fsb )
#endif /* _NO_PROTO */
{
/****************/

    XmStringFree( FS_NoMatchString( fsb)) ;
    XmStringFree( FS_Pattern( fsb)) ;
    XmStringFree( FS_Directory( fsb)) ;

    return ;
    }

/****************************************************************
 * This procedure is called to remove the child from
 *   the child list, and to allow the parent to do any
 *   neccessary clean up.
 ****************/
static void 
#ifdef _NO_PROTO
DeleteChild( w )
        Widget w ;
#else
DeleteChild(
        Widget w )
#endif /* _NO_PROTO */
{   
            XmFileSelectionBoxWidget fs ;
/****************/

    if(    XtIsRectObj( w)    )
    {   
        fs = (XmFileSelectionBoxWidget) XtParent( w) ;

        if(    w == FS_FilterLabel( fs)    )
        {   FS_FilterLabel( fs) = NULL ;
            } 
        else
        {   if(    w == FS_FilterText( fs)    )
            {   FS_FilterText( fs) = NULL ;
                } 
            else
            {   if(   FS_DirList( fs)  &&  (w == XtParent( FS_DirList( fs)))  )
                {   FS_DirList( fs) = NULL ;
                    } 
                else
                {   if(    w == FS_DirListLabel( fs)    )
                    {   FS_DirListLabel( fs) = NULL ;
                        } 
                    } 
                } 
            }
        }
    (*((XmSelectionBoxWidgetClass) xmSelectionBoxWidgetClass)
                                          ->composite_class.delete_child)( w) ;
    return ;
    }

/****************************************************************/
static void 
#ifdef _NO_PROTO
FSBCreateFilterLabel( fsb )
        XmFileSelectionBoxWidget fsb ;
#else
FSBCreateFilterLabel(
        XmFileSelectionBoxWidget fsb )
#endif /* _NO_PROTO */
{
/****************/

    FS_FilterLabel( fsb) = _XmBB_CreateLabelG( (Widget) fsb, 
					      FS_FilterLabelString( fsb),
					      "FilterLabel") ;
    return ;
    }
/****************************************************************/
static void 
#ifdef _NO_PROTO
FSBCreateDirListLabel( fsb )
        XmFileSelectionBoxWidget fsb ;
#else
FSBCreateDirListLabel(
        XmFileSelectionBoxWidget fsb )
#endif /* _NO_PROTO */
{
/****************/

    FS_DirListLabel( fsb) = _XmBB_CreateLabelG( (Widget) fsb,
					       FS_DirListLabelString( fsb),
					       "Dir") ;
    return ;
    }

/****************************************************************
 * Create the directory List widget.
 ****************/
static void 
#ifdef _NO_PROTO
FSBCreateDirList( fsb )
        XmFileSelectionBoxWidget fsb ;
#else
FSBCreateDirList(
        XmFileSelectionBoxWidget fsb )
#endif /* _NO_PROTO */
{
	Arg		al[20];
	register int	ac = 0;
            XtCallbackProc callbackProc ;
/****************/

    FS_DirListSelectedItemPosition( fsb) = 0 ;

    XtSetArg( al[ac], XmNvisibleItemCount,
                                        SB_ListVisibleItemCount( fsb)) ; ac++ ;
    XtSetArg( al[ac], XmNstringDirection, SB_StringDirection( fsb));  ac++;
    XtSetArg( al[ac], XmNselectionPolicy, XmBROWSE_SELECT);  ac++;
    XtSetArg( al[ac], XmNlistSizePolicy, XmCONSTANT);  ac++;
    XtSetArg( al[ac], XmNscrollBarDisplayPolicy, XmSTATIC);  ac++;
    XtSetArg( al[ac], XmNnavigationType, XmSTICKY_TAB_GROUP) ; ++ac ;
#ifdef DEC_MOTIF_RTOL
    XtSetArg( al[ac], DXmNlayoutDirection, LayoutM(fsb)); ++ac ;
#endif /* DEC_MOTIF_RTOL */

    FS_DirList( fsb) = XmCreateScrolledList( (Widget) fsb, "DirList", al, ac);

    callbackProc = ((XmSelectionBoxWidgetClass) fsb->core.widget_class)
                                          ->selection_box_class.list_callback ;
    if(    callbackProc    )
    {   
        XtAddCallback( FS_DirList( fsb), XmNsingleSelectionCallback,
                                               callbackProc, (XtPointer) fsb) ;
        XtAddCallback( FS_DirList( fsb), XmNbrowseSelectionCallback,
                                               callbackProc, (XtPointer) fsb) ;
        XtAddCallback( FS_DirList( fsb), XmNdefaultActionCallback,
                                               callbackProc, (XtPointer) fsb) ;
        } 
    XtManageChild( FS_DirList( fsb)) ;

    return ;
    }

/****************************************************************
 * Creates fs dir search filter text entry field.
 ****************/
static void 
#ifdef _NO_PROTO
FSBCreateFilterText( fs )
        XmFileSelectionBoxWidget fs ;
#else
FSBCreateFilterText(
        XmFileSelectionBoxWidget fs )
#endif /* _NO_PROTO */
{
            Arg             arglist[10] ;
            int             argCount ;
#ifdef I18N_EXTENSION
            XmString        stext_value_cs;
#else
            char *          stext_value ;
#endif
            XtAccelerators  temp_accelerators ;
/****************/

    /* Get text portion from Compound String, and set
    *   fs_stext_charset and fs_stext_direction bits...
    */
    /* Should do this stuff entirely with XmStrings when the text
    *   widget supports it.
    */
#ifdef I18N_EXTENSION
    stext_value_cs = FS_Pattern(fs);
#else

    if(    !(stext_value = _XmStringGetTextConcat( FS_Pattern( fs)))    )
    {   stext_value = (char *) XtMalloc( 1) ;
        stext_value[0] = '\0' ;
        }
#endif
    argCount = 0 ;
    XtSetArg( arglist[argCount], XmNcolumns, 
                                            SB_TextColumns( fs)) ; argCount++ ;
    XtSetArg( arglist[argCount], XmNresizeWidth, FALSE) ; argCount++ ;
#ifdef I18N_EXTENSION
    XtSetArg( arglist[argCount], XmNvalue, stext_value_cs) ; argCount++ ;
#else
    XtSetArg( arglist[argCount], XmNvalue, stext_value) ; argCount++ ;
#endif
    XtSetArg( arglist[argCount], XmNnavigationType, 
                                             XmSTICKY_TAB_GROUP) ; argCount++ ;
#ifndef USE_TEXT_IN_DIALOGS
    FS_FilterText( fs) = XmCreateTextField( (Widget) fs, "FilterText",
                                                           arglist, argCount) ;
#else
    XtSetArg( arglist[argCount], XmNeditMode, XmSINGLE_LINE) ; argCount++ ;
    XtSetArg( arglist[argCount], XmNrows, 1) ; argCount++ ;
#ifdef I18N_EXTENSION
    FS_FilterText( fs) = DXmCreateCSText( (Widget)fs, "fsb_filter_text",
                                                           arglist, argCount) ;
#else
    FS_FilterText( fs) = XmCreateText( fs, "FilterText",
                                                           arglist, argCount) ;
#endif
#endif
    /*	Install text accelerators.
    */
    temp_accelerators = fs->core.accelerators ;
    fs->core.accelerators = SB_TextAccelerators( fs) ;
    XtInstallAccelerators( FS_FilterText( fs), (Widget) fs) ;
    fs->core.accelerators = temp_accelerators ;

#ifdef I18N_EXTENSION
#else
    XtFree( stext_value) ;
#endif
    return ;
    }

/****************************************************************
 * Get Geo matrix filled with kid widgets.
 ****************/
static XmGeoMatrix 
#ifdef _NO_PROTO
FileSBGeoMatrixCreate( wid, instigator, desired )
        Widget wid ;
        Widget instigator ;
        XtWidgetGeometry *desired ;
#else
FileSBGeoMatrixCreate(
        Widget wid,
        Widget instigator,
        XtWidgetGeometry *desired )
#endif /* _NO_PROTO */
{
    XmFileSelectionBoxWidget fsb = (XmFileSelectionBoxWidget) wid ;
    XmGeoMatrix     geoSpec ;
    register XmGeoRowLayout  layoutPtr ;
    register XmKidGeometry   boxPtr ;
    XmKidGeometry   firstButtonBox ; 
    Boolean         dirListLabelBox ;
    Boolean         listLabelBox ;
    Boolean         dirListBox ;
    Boolean         listBox ;
    Boolean         selLabelBox ;
    Boolean         filterLabelBox ;
    Dimension       vspace = BB_MarginHeight(fsb);
    int             i;

/*
 * Layout FileSelectionBox XmGeoMatrix.
 * Each row is terminated by leaving an empty XmKidGeometry and
 * moving to the next XmGeoRowLayout.
 */

    geoSpec = _XmGeoMatrixAlloc( XmFSB_MAX_WIDGETS_VERT,
                              fsb->composite.num_children,
                              sizeof( FS_GeoExtensionRec)) ;
    geoSpec->composite = (Widget) fsb ;
    geoSpec->instigator = (Widget) instigator ;
    if(    desired    )
    {   geoSpec->instig_request = *desired ;
        } 
    geoSpec->margin_w = BB_MarginWidth( fsb) + fsb->manager.shadow_thickness ;
    geoSpec->margin_h = BB_MarginHeight( fsb) + fsb->manager.shadow_thickness ;
    geoSpec->no_geo_request = FileSelectionBoxNoGeoRequest ;

    layoutPtr = &(geoSpec->layouts->row) ;
    boxPtr = geoSpec->boxes ;

    /* menu bar */
 
    for (i = 0; i < fsb->composite.num_children; i++)
    {   Widget w = fsb->composite.children[i];

        if(    XmIsRowColumn(w)
            && ((XmRowColumnWidget)w)->row_column.type == XmMENU_BAR
            && w != SB_WorkArea(fsb)
            && _XmGeoSetupKid( boxPtr, w)    )
        {   layoutPtr->fix_up = _XmMenuBarFix ;
            boxPtr += 2;
            ++layoutPtr;
            vspace = 0;		/* fixup space_above of next row. */
            break;
            }
        }

    /* work area, XmPLACE_TOP */

    if (fsb->selection_box.child_placement == XmPLACE_TOP)
      SetupWorkArea(fsb);

    /* filter label */

    filterLabelBox = FALSE ;
    if(    _XmGeoSetupKid( boxPtr, FS_FilterLabel( fsb))    )
    {   
        filterLabelBox = TRUE ;
        layoutPtr->space_above = vspace;
        vspace = BB_MarginHeight(fsb);
        boxPtr += 2 ;
        ++layoutPtr ;
        } 

    /* filter text */

    if(    _XmGeoSetupKid( boxPtr, FS_FilterText( fsb))    )
    {   
        if(    !filterLabelBox    )
        {   layoutPtr->space_above = vspace;
            vspace = BB_MarginHeight(fsb);
            } 
        boxPtr += 2 ;
        ++layoutPtr ;
        } 

#ifdef DEC_MOTIF_RTOL
    if (LayoutIsRtoLM(fsb))
    {
        listLabelBox = FALSE ;
        if(    _XmGeoSetupKid( boxPtr, SB_ListLabel( fsb))    )
        {
           listLabelBox = TRUE ;
           ++boxPtr ;
           }
        dirListLabelBox = FALSE ;
        if(    _XmGeoSetupKid( boxPtr, FS_DirListLabel( fsb))    )
        {
           dirListLabelBox = TRUE ;
           ++boxPtr ;
           }
    }
    else
    {
#endif /* DEC_MOTIF_RTOL */


    /* dir list and file list labels */

    dirListLabelBox = FALSE ;
    if(    _XmGeoSetupKid( boxPtr, FS_DirListLabel( fsb))    )
    {   
        dirListLabelBox = TRUE ;
        ++boxPtr ;
        } 
    listLabelBox = FALSE ;
    if(    _XmGeoSetupKid( boxPtr, SB_ListLabel( fsb))    )
    {   
        listLabelBox = TRUE ;
        ++boxPtr ;
        } 

#ifdef DEC_MOTIF_RTOL
    }
#endif /* DEC_MOTIF_RTOL */

    if(    dirListLabelBox  ||  listLabelBox    )
    {   layoutPtr->fix_up = ListLabelFix ;
        layoutPtr->space_above = vspace;
        vspace = BB_MarginHeight(fsb);
        layoutPtr->space_between = BB_MarginWidth( fsb) ;

        if(    dirListLabelBox && listLabelBox    )
        {   layoutPtr->sticky_end = TRUE ;
            } 
        layoutPtr->fill_mode = XmGEO_PACK ;
        ++boxPtr ;
        ++layoutPtr ;
        } 

#ifdef DEC_MOTIF_RTOL
    if (LayoutIsRtoLM(fsb))
    {
        listBox = FALSE ;
        if(    SB_List( fsb) &&  XtIsManaged( SB_List( fsb))
           && _XmGeoSetupKid( boxPtr, XtParent( SB_List( fsb)))    )
        {
           listBox = TRUE ;
           ++boxPtr ;
           }
        dirListBox = FALSE ;
        if(     FS_DirList( fsb) && XtIsManaged( FS_DirList(fsb))
           &&  _XmGeoSetupKid( boxPtr, XtParent( FS_DirList( fsb)))    )
        {
           dirListBox = TRUE ;
           ++boxPtr ;
           }
    }
    else
    {
#endif /* DEC_MOTIF_RTOL */
    /* dir list and file list */

    dirListBox = FALSE ;
    if(     FS_DirList( fsb)  &&  XtIsManaged( FS_DirList( fsb))
        &&  _XmGeoSetupKid( boxPtr, XtParent( FS_DirList( fsb)))    )
    {   
        dirListBox = TRUE ;
        ++boxPtr ;
        } 
    listBox = FALSE ;
    if(    SB_List( fsb)  &&  XtIsManaged( SB_List( fsb))
        && _XmGeoSetupKid( boxPtr, XtParent( SB_List( fsb)))    )
    {   
        listBox = TRUE ;
        ++boxPtr ;
        } 
#ifdef DEC_MOTIF_RTOL
    }
#endif /* DEC_MOTIF_RTOL */

    if(    dirListBox  || listBox    )
    {   layoutPtr->fix_up = ListFix ;
        layoutPtr->fit_mode = XmGEO_AVERAGING ;
        layoutPtr->space_between = BB_MarginWidth( fsb) ;
        layoutPtr->stretch_height = TRUE ;
        layoutPtr->min_height = 70 ;
        layoutPtr->even_height = 1 ;
        if(    !listLabelBox  &&  !dirListLabelBox    )
        {   layoutPtr->space_above = vspace;
            vspace = BB_MarginHeight(fsb);
            } 
        ++boxPtr ;
        ++layoutPtr ;
        } 

    /* work area, XmPLACE_ABOVE_SELECTION */

    if (fsb->selection_box.child_placement == XmPLACE_ABOVE_SELECTION)
      SetupWorkArea(fsb)

    /* selection label */

    selLabelBox = FALSE ;
    if(    _XmGeoSetupKid( boxPtr, SB_SelectionLabel( fsb))    )
    {   selLabelBox = TRUE ;
        layoutPtr->space_above = vspace;
        vspace = BB_MarginHeight(fsb);
        boxPtr += 2 ;
        ++layoutPtr ;
        } 

    /* selection text */

    if(    _XmGeoSetupKid( boxPtr, SB_Text( fsb))    )
    {   
        if(    !selLabelBox    )
        {   layoutPtr->space_above = vspace;
            vspace = BB_MarginHeight(fsb);
            } 
        boxPtr += 2 ;
        ++layoutPtr ;
        } 

    /* work area, XmPLACE_BELOW_SELECTION */

    if (fsb->selection_box.child_placement == XmPLACE_BELOW_SELECTION)
      SetupWorkArea(fsb)

    /* separator */

    if(    _XmGeoSetupKid( boxPtr, SB_Separator( fsb))    )
    {   layoutPtr->fix_up = _XmSeparatorFix ;
        layoutPtr->space_above = vspace;
        vspace = BB_MarginHeight(fsb);
        boxPtr += 2 ;
        ++layoutPtr ;
        } 

    /* button row */

    firstButtonBox = boxPtr ;

#ifdef DEC_MOTIF_RTOL
    if (LayoutIsRtoLM(fsb))
    {
        if(    _XmGeoSetupKid( boxPtr, SB_HelpButton( fsb))    )
        {   ++boxPtr ;
            }
        if(    _XmGeoSetupKid( boxPtr, SB_CancelButton( fsb))    )
        {   ++boxPtr ;
            }
        if(    _XmGeoSetupKid( boxPtr, SB_ApplyButton( fsb))    )
        {   ++boxPtr ;
            }

	for (i = 0; i < fsb->composite.num_children; i++)
	{
	    Widget w = fsb->composite.children[fsb->composite.num_children-i-1];
	    if (IsButton(w) && !IsAutoButton(fsb,w) && w != SB_WorkArea(fsb))
	    {
		if (_XmGeoSetupKid( boxPtr, w))
		{   ++boxPtr ;
		    } 
	    }
	}

        if(    _XmGeoSetupKid( boxPtr, SB_OkButton( fsb))    )
        {   ++boxPtr ;
            }
    }
    else
    {
#endif /* DEC_MOTIF_RTOL */

    if(    _XmGeoSetupKid( boxPtr, SB_OkButton( fsb))    )
    {   ++boxPtr ;
        } 

    for (i = 0; i < fsb->composite.num_children; i++)
    {
      Widget w = fsb->composite.children[i];
      if (IsButton(w) && !IsAutoButton(fsb,w) && w != SB_WorkArea(fsb))
      {
          if (_XmGeoSetupKid( boxPtr, w))
          {   ++boxPtr ;
              } 
          }
      }

    if(    _XmGeoSetupKid( boxPtr, SB_ApplyButton( fsb))    )
    {   ++boxPtr ;
        } 
    if(    _XmGeoSetupKid( boxPtr, SB_CancelButton( fsb))    )
    {   ++boxPtr ;
        } 
    if(    _XmGeoSetupKid( boxPtr, SB_HelpButton( fsb))    )
    {   ++boxPtr ;
        } 
#ifdef DEC_MOTIF_RTOL
    }
#endif /* DEC_MOTIF_RTOL */

    if(    boxPtr != firstButtonBox    )
    {   
        layoutPtr->fill_mode = XmGEO_CENTER ;
        layoutPtr->fit_mode = XmGEO_WRAP ;
        if( !(SB_MinimizeButtons( fsb))    )
        {   layoutPtr->even_width = 1 ;
            } 
        layoutPtr->space_above = vspace ;
        vspace = BB_MarginHeight(fsb) ;
        layoutPtr->even_height = 1 ;
	++layoutPtr ;
        } 

    /* the end. */

    layoutPtr->space_above = vspace ;
    layoutPtr->end = TRUE ;
    return( geoSpec) ;
    }
/****************************************************************/
static Boolean 
#ifdef _NO_PROTO
FileSelectionBoxNoGeoRequest( geoSpec )
        XmGeoMatrix geoSpec ;
#else
FileSelectionBoxNoGeoRequest(
        XmGeoMatrix geoSpec )
#endif /* _NO_PROTO */
{
/****************/

    if(    BB_InSetValues( geoSpec->composite)
        && (XtClass( geoSpec->composite) == xmFileSelectionBoxWidgetClass)    )
    {   
        return( TRUE) ;
        } 
    return( FALSE) ;
    }

/****************************************************************
 * This routine saves the geometry pointers of the list labels so that they
 *   can be altered as appropriate by the ListFix routine.
 ****************/
static void 
#ifdef _NO_PROTO
ListLabelFix( geoSpec, action, layoutPtr, rowPtr )
        XmGeoMatrix geoSpec ;
        int action ;
        XmGeoMajorLayout layoutPtr ;
        XmKidGeometry rowPtr ;
#else
ListLabelFix(
        XmGeoMatrix geoSpec,
        int action,
        XmGeoMajorLayout layoutPtr,
        XmKidGeometry rowPtr )
#endif /* _NO_PROTO */
{
            FS_GeoExtension extension ;
/****************/

    extension = (FS_GeoExtension) geoSpec->extension ;
#ifdef DEC_MOTIF_RTOL
    if (LayoutIsRtoLM(geoSpec->composite))
    {
	extension->file_list_label = rowPtr++ ;
	extension->dir_list_label = rowPtr ;
    }
    else
    {
#endif
    extension->dir_list_label = rowPtr++ ;
    extension->file_list_label = rowPtr ;
#ifdef DEC_MOTIF_RTOL
    }
#endif

    return ;
    }

/****************************************************************
 * Geometry layout fixup routine for the directory and file lists.  This
 *   routine reduces the preferred width of the file list widget according 
 *   to the length of the directory  path.
 * This algorithm assumes that each row has at least one box.
 ****************/
static void 
#ifdef _NO_PROTO
ListFix( geoSpec, action, layoutPtr, rowPtr )
        XmGeoMatrix geoSpec ;
        int action ;
        XmGeoMajorLayout layoutPtr ;
        XmKidGeometry rowPtr ;
#else
ListFix(
        XmGeoMatrix geoSpec,
        int action,
        XmGeoMajorLayout layoutPtr,
        XmKidGeometry rowPtr )
#endif /* _NO_PROTO */
{
            Dimension       listPathWidth ;
            XmListWidget    dirList ;
            XmListWidget    fileList ;
            XmKidGeometry   fileListGeo ;
            XmKidGeometry   dirListGeo ;
            XmString        dirString ;
            Arg             argv[2] ;
            Cardinal        argc ;
            XmFontList      listFonts ;
            FS_GeoExtension extension ;
            int             listLabelsOffset ;
/****************/

#ifdef DEC_MOTIF_RTOL
    if (LayoutIsRtoLM(geoSpec->composite))
    {
	fileListGeo = rowPtr++;
	dirListGeo = rowPtr ;
    }
    else
    {
#endif
    dirListGeo = rowPtr++ ;
    fileListGeo = rowPtr ;
#ifdef DEC_MOTIF_RTOL
    }
#endif

    if(    !fileListGeo->kid    )
    {   /* Only one list widget in this row, so do nothing.
        */
        return ;
        }
    extension = (FS_GeoExtension) geoSpec->extension ;
    fileList = (XmListWidget) SB_List( geoSpec->composite) ;
    dirString = FS_Directory( geoSpec->composite) ;

    switch(    action    )
    {   
        case XmGET_PREFERRED_SIZE:
        {   
            argc = 0 ;
            XtSetArg( argv[argc], XmNfontList, &listFonts) ; ++argc ;
            XtGetValues( (Widget) fileList, argv, argc) ;

            listPathWidth = XmStringWidth( listFonts, dirString) ;

            if(    !(FS_StateFlags( geoSpec->composite) & XmFS_NO_MATCH)    )
            {   
                if(    listPathWidth < fileListGeo->box.width    )
                {   fileListGeo->box.width -= listPathWidth ;
                    } 
                } 
            if(    listPathWidth < dirListGeo->box.width    )
            {   dirListGeo->box.width -= listPathWidth ;
                } 
            if(    extension->dir_list_label
                && (extension->dir_list_label->box.width
                                                  < dirListGeo->box.width)    )
            {   extension->dir_list_label->box.width = dirListGeo->box.width ;
                } 
            /* Drop through to pick up extension record field for either
            *   type of geometry request.
            */
            }
        case XmGET_ACTUAL_SIZE:
        {   extension->prefer_width = fileListGeo->box.width ;
            break ;
            } 
        case XmGEO_PRE_SET:
        {   
            if(    fileListGeo->box.width > extension->prefer_width    )
            {   
                /* Add extra space designated for file list to dir list
                *   instead, assuring that file list only shows the file name
                *   and not a segment of the path.
                */
                extension->delta_width = fileListGeo->box.width
                                                    - extension->prefer_width ;
                fileListGeo->box.width -= extension->delta_width ;
#ifdef DEC_MOTIF_RTOL
		if (LayoutIsRtoLM(geoSpec->composite))
		    dirListGeo->box.x -= extension->delta_width ;
		else
#endif
		    fileListGeo->box.x += extension->delta_width ;
                dirListGeo->box.width += extension->delta_width ;
                } 
            else
            {   extension->delta_width = 0 ;
                } 
            /* Set label boxes to be the same width and x dimension as the 
            *   lists below them.
            */
#ifdef DEC_MOTIF_RTOL
	if (LayoutIsRtoLM(geoSpec->composite))
	{
            if(    extension->file_list_label    )
            {   
                if(    extension->file_list_label->box.width 
                                                  < fileListGeo->box.width    )
                {   extension->file_list_label->box.width = fileListGeo->box.width ;
                    extension->file_list_label->box.x = fileListGeo->box.x ;
                } 

                if(    extension->dir_list_label    )
                {   
                    extension->dir_list_label->box.x = dirListGeo->box.x ;
                    extension->dir_list_label->box.width = dirListGeo->box.width ;
                } 
	    }
	}
	else
	{
#endif
            if(    extension->file_list_label    )
            {   
                if(    extension->file_list_label->box.width 
                                                  < fileListGeo->box.width    )
                {   extension->file_list_label->box.width
                                                     = fileListGeo->box.width ;
                    extension->file_list_label->box.x = fileListGeo->box.x ;
                    } 
                if(    extension->dir_list_label    )
                {   
                    listLabelsOffset = extension->file_list_label->box.x
                                           - extension->dir_list_label->box.x ;
                    if(    listLabelsOffset
                                      > (int) layoutPtr->row.space_between    )
                    {   extension->dir_list_label->box.width =
                                            (Dimension) listLabelsOffset
                                               - layoutPtr->row.space_between ;
                        } 
                    }
                } 
#ifdef DEC_MOTIF_RTOL
	}
#endif    
            break ;
            } 
        case XmGEO_POST_SET:
        {   
            if(    extension->delta_width    )
            {   /* Undo the changes of PRE_SET, so subsequent re-layout
                *   attempts will yield correct results.
                */
                fileListGeo->box.width += extension->delta_width ;
#ifdef DEC_MOTIF_RTOL
		if (LayoutIsRtoLM(geoSpec->composite))
		    dirListGeo->box.x += extension->delta_width ;
		else
#endif
		    fileListGeo->box.x -= extension->delta_width ;
                dirListGeo->box.width -= extension->delta_width ;
                } 
            if(    !(FS_StateFlags( geoSpec->composite) & XmFS_NO_MATCH)    )
            {   
                /* Move horizontal position so path does not show in file list.
                */
                argc = 0 ;
                XtSetArg( argv[argc], XmNfontList, &listFonts) ; ++argc ;
                XtGetValues( (Widget) fileList, argv, argc) ;
                listPathWidth = XmStringWidth( listFonts, dirString) ;
                XmListSetHorizPos( (Widget) fileList, listPathWidth) ;
                } 
            /* Move horizontal scroll position of directory list as far to the
            *   right as it will go, so that the right end of the list is 
            *   never hidden.
            */
            dirList = (XmListWidget) FS_DirList( geoSpec->composite) ;
            argc = 0 ;
            XtSetArg( argv[argc], XmNfontList, &listFonts) ; ++argc ;
            XtGetValues( (Widget) dirList, argv, argc) ;

            listPathWidth = XmStringWidth( listFonts, dirString) ;
            XmListSetHorizPos( (Widget) dirList, listPathWidth) ;
            
            break ;
            } 
        } 
    return ;
    }

/****************************************************************/
static void 
#ifdef _NO_PROTO
FileSearchProc( w, sd )
        Widget w ;
        XtPointer sd ;
#else
FileSearchProc(
        Widget w,
        XtPointer sd )
#endif /* _NO_PROTO */
{   
            XmFileSelectionBoxWidget fs = (XmFileSelectionBoxWidget) w ;
            XmFileSelectionBoxCallbackStruct * searchData
                                    = (XmFileSelectionBoxCallbackStruct *) sd ;
            String          dir ;
            String          pattern ;
            Arg             args[3] ;
            int             Index ;
            String *        fileList ;
            unsigned int    numFiles ;
            unsigned int    numAlloc ;
            XmString *      XmStringFileList ;
/****************/

#ifdef I18N_EXTENSION
    {
        long    byte_count, status;
        dir = DXmCvtCStoOS ( searchData->dir, &byte_count, &status );
        if ( status == DXmCvtStatusFail ) return;
        pattern = DXmCvtCStoOS ( searchData->pattern, &byte_count, &status );
        if ( status == DXmCvtStatusFail ){
            XtFree ( dir );
            return;
        }
    }
#else
    if(   !(dir = _XmStringGetTextConcat( searchData->dir))    )
    {   return ;
        } 
    if(    !(pattern = _XmStringGetTextConcat( searchData->pattern))    )
    {   XtFree( dir) ;
        return ;
        } 
#endif
    fileList = NULL ;
    _XmOSBuildFileList( dir, pattern, FS_FileTypeMask( fs), 
                                            &fileList,  &numFiles, &numAlloc) ;
    if(    fileList  &&  numFiles    )
    {   if(    numFiles > 1    )
        {   qsort( (void *)fileList, numFiles, sizeof( char *), _XmOSFileCompare) ;
            } 
        XmStringFileList = (XmString *) XtMalloc( 
                                                numFiles * sizeof( XmString)) ;
        Index = 0 ;
        while(    Index < numFiles    )
#ifdef I18N_EXTENSION
        {
            long        byte_count, status;
            XmStringFileList[Index] = DXmCvtOStoCS( fileList[Index],
                                                        &byte_count, &status );
            if ( status == DXmCvtStatusFail )
            XmStringFileList[Index] =
                        XmStringCreateLtoR ("", XmSTRING_DEFAULT_CHARSET);
#else
        {   XmStringFileList[Index] = XmStringLtoRCreate( fileList[Index], 
                                                    XmFONTLIST_DEFAULT_TAG) ;
#endif
            ++Index ;
            } 
        /* Update the list.
        */
        Index = 0 ;
        XtSetArg( args[Index], XmNitems, XmStringFileList) ; Index++ ;
        XtSetArg( args[Index], XmNitemCount, numFiles) ; Index++ ;
        XtSetValues( SB_List( fs), args, Index) ;

        Index = numFiles ;
        while(    Index--    )
        {   XtFree( fileList[Index]) ;
            } 
        while(    numFiles--    )
        {   XmStringFree( XmStringFileList[numFiles]) ;
            }
        XtFree( (char *) XmStringFileList) ;
        }
    else
    {   XtSetArg( args[0], XmNitemCount, 0) ;
        XtSetValues( SB_List( fs), args, 1) ;
        } 
    FS_ListUpdated( fs) = TRUE ;

    XtFree( (char *) fileList) ;
    XtFree( pattern) ;
    XtFree( dir) ;
    return ;
    }

/****************************************************************
 * This routine validates and allocates new copies of all searchData
 *   fields that are required by the DirSearchProc and the FileSearchProc
 *   routines.  The default routines require only the "dir" and "pattern" 
 *   fields to be filled with appropriate qualified non-null XmStrings.
 * Any of the fields of the searchData passed into this routine may be NULL.
 *   Generally, only those fields which signify changes due to a user action
 *   will be passed into this routine.  This data should always override
 *   data derived from other sources.
 * The caller is responsible to free the XmStrings of all (non-null) fields
 *   of the qualifiedSearchData record.
 ****************/
static void 
#ifdef _NO_PROTO
QualifySearchDataProc( w, sd, qsd )
        Widget w ;
        XtPointer sd ;
        XtPointer qsd ;
#else
QualifySearchDataProc(
        Widget w,
        XtPointer sd,
        XtPointer qsd )
#endif /* _NO_PROTO */
{
            XmFileSelectionBoxWidget fs = (XmFileSelectionBoxWidget) w ;
            XmFileSelectionBoxCallbackStruct * searchData 
                                    = (XmFileSelectionBoxCallbackStruct *) sd ;
            XmFileSelectionBoxCallbackStruct * qualifiedSearchData 
                                   = (XmFileSelectionBoxCallbackStruct *) qsd ;
            String          valueString ;
            String          patternString ;
            String          dirString ;
            String          maskString ;
            String          qualifiedDir ;
            String          qualifiedPattern ;
            String          qualifiedMask ;
            char *          dirPartPtr ;
#ifdef VMS
	    char *	    vmsDirPartPtr = NULL;
#endif
            char *          patternPartPtr ;
            unsigned int    qDirLen ;
#ifdef I18N_EXTENSION
            long        byte_count, status;
#endif /* I18N_EXTENSION */
/****************/
#ifdef I18N_EXTENSION
    if ( searchData->mask ){
        maskString = DXmCvtCStoOS ( searchData->mask, &byte_count, &status );
        if ( status == DXmCvtStatusFail )
            maskString = NULL;
    } else
        maskString = NULL;
    if ( searchData->dir ){
        dirString = DXmCvtCStoOS ( searchData->dir, &byte_count, &status );
        if ( status == DXmCvtStatusFail )
            dirString = NULL;
    } else
        dirString = NULL;
    if ( searchData->pattern ){
        patternString = DXmCvtCStoOS
                        ( searchData->pattern, &byte_count, &status );
        if ( status == DXmCvtStatusFail )
            patternString = NULL;
    } else
        patternString = NULL;
#else
    maskString = _XmStringGetTextConcat( searchData->mask) ;
    dirString = _XmStringGetTextConcat( searchData->dir) ;
    patternString = _XmStringGetTextConcat( searchData->pattern) ;
#endif

    if(    !maskString  ||  (dirString  &&  patternString)    )
    {   
        if(    !dirString    )
#ifdef I18N_EXTENSION
        {
            dirString = DXmCvtCStoOS ( FS_Directory( fs), &byte_count, &status 
            if ( status == DXmCvtStatusFail )
                dirString = NULL;
            }
#else
        {   dirString = _XmStringGetTextConcat( FS_Directory( fs)) ;
            } 
#endif
        if(    !patternString    )
#ifdef I18N_EXTENSION
        {
            patternString = DXmCvtCStoOS
                        ( FS_Pattern( fs), &byte_count, &status );
            if ( status == DXmCvtStatusFail )
                patternString = NULL;
            }
#else
        {   patternString = _XmStringGetTextConcat( FS_Pattern( fs)) ;
            } 
#endif
        _XmOSQualifyFileSpec( dirString, patternString,
                                            &qualifiedDir, &qualifiedPattern) ;
        } 
    else
    {   patternPartPtr = _XmOSFindPatternPart( maskString) ;

        if(    patternPartPtr != maskString    )
        {   
	    /*** This need to be re-think with Xmos.c in mind. dd */

#ifdef VMS
	    qDirLen = (patternPartPtr == NULL) ?
			    qDirLen = strlen(maskString) :  /* no pattern part */
			    patternPartPtr - maskString;    /* calc length as diff of addresses */
	    vmsDirPartPtr = XtMalloc(qDirLen+1);
	    strncpy(vmsDirPartPtr, maskString, qDirLen);
	    vmsDirPartPtr[qDirLen] = '\0';
	    dirPartPtr = vmsDirPartPtr;
#else
            /* To avoid allocating memory and copying part of the mask string,
            *   just stuff '\0' at the '/' which is between the directory part
            *   and the pattern part.  The QualifyFileSpec below does not
            *   require the trailing '/', and it will assure that the resulting
            *   qualifiedDir will have the required trailing '/'.
            * Must check to see if the directory part of the mask
            *   string is "//", so that this information is not lost when
            *   deleting the '/' before the pattern part.  Embedded "//"
            *   sequences are not protected, but root specifications are.
            */
            *(patternPartPtr - 1) = '\0' ;

            if(    !*maskString
#ifdef WIN32
                || ((*maskString == '\\')  &&  !maskString[1])    )
#else
                || ((*maskString == '/')  &&  !maskString[1])    )
#endif
            {   
                if(    !*maskString    )
                {   /* The '/' that was replaced with '\0' above was the only 
                    *    character in the directory specification (root
                    *    directory "/"), so simply restore it.
                    */
#ifdef WIN32
                    dirPartPtr = "\\" ;
#else
                    dirPartPtr = "/" ;
#endif
                    } 
                else
                {   /* The directory specification was "//" before the
                    *   trailing '/' was deleted, so restore original.
                    */
#ifndef WIN32 /* But in Windows NT, it's illegal, so leave it as "\" */
                    dirPartPtr = "//" ;
#endif
                    } 
                } 
            else
            {   /* Is non-root directory specification, so its ok to have
                *   deleted the '/', since we are not protecting embedded
                *   "//" path specifications from reduction to a single slash.
                */
                dirPartPtr = maskString ;
                } 
#endif	/* VMS */
            } 
        else
        {   dirPartPtr = NULL ;
            } 
        if(    dirString    )
        {   dirPartPtr = dirString ;
            } 
        if(    patternString    )
        {   patternPartPtr = patternString ;
            } 
        _XmOSQualifyFileSpec( dirPartPtr, patternPartPtr,
                                            &qualifiedDir, &qualifiedPattern) ;
        }
    qDirLen = strlen( qualifiedDir) ;
    qualifiedMask = XtMalloc( 1 + qDirLen + strlen( qualifiedPattern)) ;
    strcpy( qualifiedMask, qualifiedDir) ;
    strcpy( &qualifiedMask[qDirLen], qualifiedPattern) ;

    qualifiedSearchData->reason = searchData->reason ;
    qualifiedSearchData->event = searchData->event ;

    if(    searchData->value    )
    {   qualifiedSearchData->value = XmStringCopy( searchData->value) ;
        valueString = NULL ;
        } 
    else
    {   
#ifndef USE_TEXT_IN_DIALOGS
        valueString = XmTextFieldGetString( SB_Text( fs)) ;
#else
#ifdef I18N_EXTENSION
    {
        XmString cs_value;

        cs_value = DXmCSTextGetString (SB_Text(fs));

       /* XmString is converted to char * and then converted back to
        * XmString again.
        * This redundant code is necessary for the cases
        * (1) When filename longer than 50 bytes  and it makes plural
        * ISO Latin1 segments, because CSText MAX_SEGMENT_SIZE is 50.
        * (2) When directory name is SetString'ed by system and file name is
        * typed by a user, CSText will give two ISO Latin1 segments.
        * (3) When CSText doesn't have ISO Latin1 font, but JIS Roman font,
        * it gives JIS Roman segment.
        * Local applications should be OK because they must be using
        * DXmCvtCStoOS() but this conversion here is done for running
        * US applications.
        */
        valueString = DXmCvtCStoOS ( cs_value, &byte_count, &status );
        if ( status != DXmCvtStatusFail ){
          XmStringFree ( cs_value );
          cs_value = DXmCvtOStoCS ( valueString, &byte_count, &status );
          XtFree( valueString );
        } else
          cs_value = NULL;

        if ( cs_value )
          qualifiedSearchData->value = cs_value;
        else
          qualifiedSearchData->value =
                XmStringCreateLtoR ("", XmSTRING_DEFAULT_CHARSET);

        valueString = NULL;
        }
#else

        valueString = XmTextGetString( SB_Text( fs)) ;
#endif
#endif
      
        qualifiedSearchData->value = XmStringLtoRCreate( valueString,
                                                    XmFONTLIST_DEFAULT_TAG) ;
        } 
    qualifiedSearchData->length = XmStringLength( qualifiedSearchData->value) ;

#ifdef I18N_EXTENSION
    qualifiedSearchData->mask = DXmCvtOStoCS( qualifiedMask,
                                                &byte_count, &status);
    if ( status == DXmCvtStatusFail )
      qualifiedSearchData->mask =
                        XmStringCreateLtoR ("", XmSTRING_DEFAULT_CHARSET);
#else
    qualifiedSearchData->mask = XmStringLtoRCreate( qualifiedMask,
                                                    XmFONTLIST_DEFAULT_TAG) ;
#endif
    qualifiedSearchData->mask_length = XmStringLength(
                                                   qualifiedSearchData->mask) ;

#ifdef I18N_EXTENSION
    qualifiedSearchData->dir = DXmCvtOStoCS( qualifiedDir,
                                                &byte_count, &status);
    if ( status == DXmCvtStatusFail )
      qualifiedSearchData->dir =
                        XmStringCreateLtoR ("", XmSTRING_DEFAULT_CHARSET);
#else
    qualifiedSearchData->dir = XmStringLtoRCreate( qualifiedDir,
                                                    XmFONTLIST_DEFAULT_TAG) ;
#endif
    qualifiedSearchData->dir_length = XmStringLength(
                                                    qualifiedSearchData->dir) ;

#ifdef I18N_EXTENSION
    qualifiedSearchData->pattern = DXmCvtOStoCS( qualifiedPattern,
                                                &byte_count, &status);
    if ( status == DXmCvtStatusFail )
      qualifiedSearchData->pattern =
                        XmStringCreateLtoR ("", XmSTRING_DEFAULT_CHARSET);
#else
    qualifiedSearchData->pattern = XmStringLtoRCreate( qualifiedPattern,
                                                    XmFONTLIST_DEFAULT_TAG) ;
#endif
    qualifiedSearchData->pattern_length = XmStringLength(
                                                qualifiedSearchData->pattern) ;
#ifdef VMS
    if (dirPartPtr) XtFree (vmsDirPartPtr);
#endif
    XtFree( valueString) ;
    XtFree( qualifiedMask) ;
    XtFree( qualifiedPattern) ;
    XtFree( qualifiedDir) ;
    XtFree( patternString) ;
    XtFree( dirString) ;
    XtFree( maskString) ;
    return ;
    }

/****************************************************************/
static void 
#ifdef _NO_PROTO
FileSelectionBoxUpdate( fs, searchData )
        XmFileSelectionBoxWidget fs ;
        XmFileSelectionBoxCallbackStruct *searchData ;
#else
FileSelectionBoxUpdate(
        XmFileSelectionBoxWidget fs,
        XmFileSelectionBoxCallbackStruct *searchData )
#endif /* _NO_PROTO */
{
            Arg             ac[5] ;
            Cardinal        al ;
            int             itemCount ;
            XmString        item ;
            String          textValue ;
            String          dirString ;
            String          maskString ;
            String          patternString ;
            int             len ;
            XmFileSelectionBoxCallbackStruct qualifiedSearchData ;
/****************/

#ifdef DEC_MOTIF_EXTENSION
            Cursor          wait_cursor;

            wait_cursor = CreateWaitCursor((Widget) fs);
            if (XtIsRealized(fs) && XtIsManaged(fs))
                {
                XDefineCursor(  XtDisplay(fs),
                                XtWindow(fs),
                                wait_cursor);
                }
#endif /* DEC_MOTIF_EXTENSION */


    /* Unmap file list, so if it takes a long time to generate the
    *   list items, the user doesn't wonder what is going on.
    */
    XtSetMappedWhenManaged( SB_List( fs), FALSE) ;
    XFlush( XtDisplay( fs)) ;

    if(    FS_StateFlags( fs) & XmFS_NO_MATCH    )
    {   XmListDeleteAllItems( SB_List( fs)) ;
        } 
    FS_StateFlags( fs) |= XmFS_IN_FILE_SEARCH ;

    (*FS_QualifySearchDataProc( fs))( (Widget) fs, (XtPointer) searchData,
                                            (XtPointer) &qualifiedSearchData) ;
    FS_ListUpdated( fs) = FALSE ;
    FS_DirectoryValid( fs) = FALSE ;

    (*FS_DirSearchProc( fs))( (Widget) fs, (XtPointer) &qualifiedSearchData) ;

    if(    FS_DirectoryValid( fs)    )
    {   
        (*FS_FileSearchProc( fs))( (Widget) fs,
                                            (XtPointer) &qualifiedSearchData) ;
        /* Now update the Directory and Pattern resources.
        */
        if(    !XmStringCompare( qualifiedSearchData.dir, FS_Directory( fs))  )
        {   if(    FS_Directory( fs)    )
            {   XmStringFree( FS_Directory( fs)) ;
                } 
            FS_Directory( fs) = XmStringCopy( qualifiedSearchData.dir) ;
            } 

        if(   !XmStringCompare( qualifiedSearchData.pattern, FS_Pattern( fs)) )
        {   if(    FS_Pattern( fs)    )
            {   XmStringFree( FS_Pattern( fs)) ;
                } 
            FS_Pattern( fs) = XmStringCopy( qualifiedSearchData.pattern) ;
            } 
        /* Also update the filter text.
        */
#ifdef I18N_EXTENSION
        {
            XmString maskStringCS;
            maskStringCS = XmStringConcat ( FS_Directory(fs), FS_Pattern(fs) );
            DXmCSTextSetString ( FS_FilterText(fs), maskStringCS );
            XmStringFree ( maskStringCS );
        }
#else
        if(    dirString = _XmStringGetTextConcat( FS_Directory( fs))    )
        {   
            if(   patternString = _XmStringGetTextConcat( FS_Pattern( fs))   )
            {   
                len = strlen( dirString) ;
                maskString = XtMalloc( len + strlen( patternString) + 1) ;
                strcpy( maskString, dirString) ;
                strcpy( &maskString[len], patternString) ;

#ifndef USE_TEXT_IN_DIALOGS
                XmTextFieldSetString( FS_FilterText( fs), maskString) ;
                XmTextFieldSetCursorPosition( FS_FilterText( fs),
			     XmTextFieldGetLastPosition( FS_FilterText( fs))) ;
#else
                XmTextSetString( FS_FilterText( fs), maskString) ;
                XmTextSetCursorPosition( FS_FilterText( fs),
			     XmTextGetLastPosition( FS_FilterText( fs))) ;
#endif
                XtFree( maskString) ;
                XtFree( patternString) ;
                } 
            XtFree( dirString) ;
            } 
#endif
        } 
    FS_StateFlags( fs) &= ~XmFS_IN_FILE_SEARCH ;

    al = 0 ;
    XtSetArg( ac[al], XmNitemCount, &itemCount) ; ++al ;
    XtGetValues( SB_List( fs), ac, al) ;

    if(    itemCount    )
    {   FS_StateFlags( fs) &= ~XmFS_NO_MATCH ;
        } 
    else
    {   FS_StateFlags( fs) |= XmFS_NO_MATCH ;

        if(    item = FS_NoMatchString( fs)    )
        {   al = 0 ;
            XtSetArg( ac[al], XmNitems, &item) ; ++al ;
            XtSetArg( ac[al], XmNitemCount, 1) ; ++al ;
            XtSetValues( SB_List( fs), ac, al) ;
            } 
        } 
    if(    FS_ListUpdated( fs)    )
    {   
        if(    textValue = _XmStringGetTextConcat( FS_Directory( fs))    )
        {   
#ifndef USE_TEXT_IN_DIALOGS
            XmTextFieldSetString( SB_Text( fs), textValue) ;
            XmTextFieldSetCursorPosition( SB_Text( fs),
			     XmTextFieldGetLastPosition( SB_Text( fs))) ;
#else
#ifdef I18N_EXTENSION
            DXmCSTextSetString ( SB_Text(fs), NULL );
        else
            DXmCSTextSetString ( SB_Text(fs), FS_Directory(fs) );
#else
            XmTextSetString( SB_Text( fs), textValue) ;
            XmTextSetCursorPosition( SB_Text( fs),
			     XmTextGetLastPosition( SB_Text( fs))) ;
#endif
            XtFree( textValue) ;
#endif 
            }
        _XmBulletinBoardSizeUpdate( (Widget) fs) ;
        } 
    XtSetMappedWhenManaged( SB_List( fs), TRUE) ;

    XmStringFree( qualifiedSearchData.value) ;
    XmStringFree( qualifiedSearchData.mask) ;
    XmStringFree( qualifiedSearchData.dir) ;
    XmStringFree( qualifiedSearchData.pattern) ;
#ifdef DEC_MOTIF_EXTENSION
    if (XtIsRealized(fs) && XtIsManaged(fs))
        {
        XUndefineCursor(XtDisplay(fs),
                        XtWindow(fs));
        }
#endif /* DEC_MOTIF_EXTENSION */
    return ;
    }

/****************************************************************
 * This loads the list widget with a directory list based
 *   on the directory specification.
 ****************/
static void 
#ifdef _NO_PROTO
DirSearchProc( w, sd )
        Widget w ;
        XtPointer sd ;
#else
DirSearchProc(
        Widget w,
        XtPointer sd )
#endif /* _NO_PROTO */
{   
            XmFileSelectionBoxWidget fs = (XmFileSelectionBoxWidget) w ;
            XmFileSelectionBoxCallbackStruct * searchData
                                    = (XmFileSelectionBoxCallbackStruct *) sd ;
            String          qualifiedDir ;
            Arg             args[10] ;
            int             Index ;
            String *        dirList ;
            unsigned int    numDirs ;
            unsigned int    numAlloc ;
            XmString *      XmStringDirList ;
/****************/

    if((!XmStringCompare( searchData->dir, FS_Directory( fs))) ||
       (FS_StateFlags( fs) & XmFS_DIR_SEARCH_PROC))
    {   
#ifdef I18N_EXTENSION
        long    byte, status;
#endif /* I18N_EXTENSION */

      FS_StateFlags( fs) &= ~XmFS_DIR_SEARCH_PROC ;

        /* Directory is different than current, so update dir list.
        */
#ifdef I18N_EXTENSION
        qualifiedDir = DXmCvtCStoOS ( searchData->dir, &byte, &status );
        if ( status == DXmCvtStatusFail )
#else
        if(    !(qualifiedDir = _XmStringGetTextConcat( searchData->dir))    )
#endif
        {   
            if(    _XmGetAudibleWarning((Widget) fs) == XmBELL    )
            {   XBell( XtDisplay( fs), 0) ;
                } 
        	XtFree( (char *) qualifiedDir) ;
            return ;
            } 

        dirList = NULL ;
#ifdef VMS
        _XmOSGetDirEntries( qualifiedDir, "*.DIR", XmFILE_DIRECTORY, FALSE, TRUE,
                                               &dirList, &numDirs, &numAlloc) ;
#else
        _XmOSGetDirEntries( qualifiedDir, "*", XmFILE_DIRECTORY, FALSE, TRUE,
                                               &dirList, &numDirs, &numAlloc) ;
#endif
        if(    !numDirs    )
        {   /* Directory list is empty, so have attempted to go 
            *   into a directory without permissions.  Don't do it!
            */
            if(    _XmGetAudibleWarning((Widget) fs) == XmBELL    )
            {   XBell( XtDisplay( fs), 0) ;
                } 
	    XtFree((char *) dirList) ;
            return ;
            } 
        if(    numDirs > 1    )
        {
#ifndef VMS
   qsort( (void *)dirList, numDirs, sizeof( char *), _XmOSFileCompare) ;
#endif
            } 
        XmStringDirList = (XmString *) XtMalloc( numDirs * sizeof( XmString)) ;

        Index = 0 ;
        while(    Index < numDirs    )
#ifdef I18N_EXTENSION
        {   XmStringDirList[Index] = DXmCvtOStoCS( dirList[Index],
                                                        &byte, &status );
            if ( status == DXmCvtStatusFail )
                XmStringDirList[Index] =
                        XmStringCreateLtoR ("", XmSTRING_DEFAULT_CHARSET);
#else
        {   XmStringDirList[Index] = XmStringLtoRCreate( dirList[Index],
                                                    XmFONTLIST_DEFAULT_TAG) ;
#endif
            ++Index ;
            } 
        /* Update the list.  */
        Index = 0;
        XtSetArg( args[Index], XmNitems, XmStringDirList) ; Index++ ;
        XtSetArg( args[Index], XmNitemCount, numDirs) ; Index++ ;
        XtSetArg( args[Index], XmNtopItemPosition, 1) ; Index++ ;
        XtSetValues( FS_DirList( fs), args, Index);

        XmListSelectPos( FS_DirList( fs), 1, FALSE) ;
        FS_DirListSelectedItemPosition( fs) = 1 ;

        Index = numDirs ;
        while(    Index--    )
        {   XtFree( dirList[Index]) ;
            } 
        XtFree( (char *) dirList) ;
    
        while(    numDirs--    )
        {
            XmStringFree( XmStringDirList[numDirs]) ;
            }
        XtFree( (char *) XmStringDirList) ;
        XtFree( (char *) qualifiedDir) ;
        FS_ListUpdated( fs) = TRUE ;
        }
    FS_DirectoryValid( fs) = TRUE ;
    return ;
    }
   
/****************************************************************
 * Process callback from either List of the File Selection Box.
 ****************/
static void 
#ifdef _NO_PROTO
ListCallback( wid, client_data, call_data )
        Widget wid ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
ListCallback(
        Widget wid,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{   
            XmListCallbackStruct * callback ;
            XmFileSelectionBoxWidget fsb ;
            XmGadgetClass   gadget_class ;
            XmGadget        dbutton ;
            XmFileSelectionBoxCallbackStruct change_data ;
            XmFileSelectionBoxCallbackStruct qualified_change_data ;
            String          textValue ;
            String          dirString ;
            String          maskString ;
            String          patternString ;
            int             len ;
/****************/

    callback = (XmListCallbackStruct *) call_data ;
    fsb = (XmFileSelectionBoxWidget) client_data ;

    switch(    callback->reason    )
    {   
        case XmCR_BROWSE_SELECT:
        case XmCR_SINGLE_SELECT:
        {   
            if(    wid == FS_DirList( fsb)    )
            {   
                FS_DirListSelectedItemPosition( fsb)
                                                    = callback->item_position ;
                change_data.event  = NULL ;
                change_data.reason = XmCR_NONE ;
                change_data.value = NULL ;
                change_data.length = 0 ;
#ifdef I18N_EXTENSION
                textValue = NULL;
                change_data.mask = DXmCSTextGetString (FS_FilterText(fsb));
#else
#ifndef USE_TEXT_IN_DIALOGS
                textValue = XmTextFieldGetString( FS_FilterText( fsb)) ;
#else
                textValue = XmTextGetString( FS_FilterText( fsb)) ;
#endif
                change_data.mask = XmStringLtoRCreate( textValue,
                                                    XmFONTLIST_DEFAULT_TAG) ;
#endif
                change_data.mask_length = XmStringLength( change_data.mask) ;
                change_data.dir = XmStringCopy( callback->item) ;
                change_data.dir_length = XmStringLength( change_data.dir) ;
                change_data.pattern = NULL ;
                change_data.pattern_length = 0 ;

                /* Qualify and then update the filter text.
                */
                (*FS_QualifySearchDataProc( fsb))( (Widget) fsb,
                                     (XtPointer) &change_data,
                                          (XtPointer) &qualified_change_data) ;
#ifdef I18N_EXTENSION
                {
                    XmString maskStringCS;
                    maskStringCS =
                        XmStringConcat ( qualified_change_data.dir,
                                           qualified_change_data.pattern );
                    DXmCSTextSetString ( FS_FilterText(fsb), maskStringCS );
                    XmStringFree ( maskStringCS );
                }
#else
                if(    dirString = _XmStringGetTextConcat( 
                                                qualified_change_data.dir)    )
                {   if(    patternString = _XmStringGetTextConcat( 
                                            qualified_change_data.pattern)    )
                    {   len = strlen( dirString) ;
                        maskString = XtMalloc( len
                                                 + strlen( patternString) + 1) ;
                        strcpy( maskString, dirString) ;
                        strcpy( &maskString[len], patternString) ;
#ifndef USE_TEXT_IN_DIALOGS
                        XmTextFieldSetString( FS_FilterText( fsb),
                                                                  maskString) ;
                        XmTextFieldSetCursorPosition( FS_FilterText( fsb),
			    XmTextFieldGetLastPosition( FS_FilterText( fsb))) ;
#else
                        XmTextSetString( FS_FilterText( fsb), maskString) ;
                        XmTextSetCursorPosition( FS_FilterText( fsb),
			    XmTextGetLastPosition( FS_FilterText( fsb))) ;
#endif
                        XtFree( maskString) ;
                        XtFree( patternString) ;
                        } 
                    XtFree( dirString) ;
                    } 
#endif
                XmStringFree( qualified_change_data.pattern) ;
                XmStringFree( qualified_change_data.dir) ;
                XmStringFree( qualified_change_data.mask) ;
                XmStringFree( qualified_change_data.value) ;
                XmStringFree( change_data.mask) ;
                XmStringFree( change_data.dir) ;
                XtFree( textValue) ;
                } 
            else    /* wid is File List. */
            {   
                if(    FS_StateFlags( fsb) & XmFS_NO_MATCH    )
                {   
                    XmListDeselectPos( SB_List( fsb), 1) ;
                    break ;
                    } 
                SB_ListSelectedItemPosition( fsb) = callback->item_position ;
#ifdef I18N_EXTENSION
                DXmCSTextSetString ( SB_Text(fsb), callback->item );
#else
                if(    textValue = _XmStringGetTextConcat( callback->item)    )
                {   
#ifndef USE_TEXT_IN_DIALOGS
                    XmTextFieldSetString( SB_Text( fsb), textValue) ;
                    XmTextFieldSetCursorPosition( SB_Text( fsb),
			     XmTextFieldGetLastPosition( SB_Text( fsb))) ;
#else
                    XmTextSetString( SB_Text( fsb), textValue) ;
                    XmTextSetCursorPosition( SB_Text( fsb),
			     XmTextGetLastPosition( SB_Text( fsb))) ;
#endif
                 XtFree(textValue);
                    } 
#endif
                } 
            break ;
            }
        case XmCR_DEFAULT_ACTION:
        {   
            dbutton = (XmGadget) BB_DynamicDefaultButton( fsb) ;
            /* Catch only double-click default action here.
            *  Key press events are handled through the ParentProcess routine.
            */
            if(    (callback->event->type != KeyPress)
                && dbutton  &&  XtIsManaged( dbutton)
                && XtIsSensitive( dbutton)  &&  XmIsGadget( dbutton)
	        && (    !(FS_StateFlags(fsb) & XmFS_NO_MATCH)
		    || (wid == FS_DirList( fsb)))    )
             {   
                gadget_class = (XmGadgetClass) dbutton->object.widget_class ;
                if (gadget_class->gadget_class.arm_and_activate)
		{   
		/* pass the event so that the button can pass it on to its
		** callbacks, even though the event isn't within the button
		*/
		(*(gadget_class->gadget_class.arm_and_activate))
			  ((Widget) dbutton, callback->event, NULL, NULL) ;
		} 
             }
            break ;
            } 
        default:
        {   break ;
            } 
        }
    return ;
    }

/****************************************************************
 * This routine detects differences in two versions
 *   of a widget, when a difference is found the
 *   appropriate action is taken.
 ****************/
static Boolean 
#ifdef _NO_PROTO
SetValues( cw, rw, nw, args_in, num_args )
        Widget cw ;
        Widget rw ;
        Widget nw ;
        ArgList args_in ;
        Cardinal *num_args ;
#else
SetValues(
        Widget cw,
        Widget rw,
        Widget nw,
        ArgList args_in,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
            XmFileSelectionBoxWidget current = (XmFileSelectionBoxWidget) cw ;
            XmFileSelectionBoxWidget request = (XmFileSelectionBoxWidget) rw ;
            XmFileSelectionBoxWidget new_w = (XmFileSelectionBoxWidget) nw ;
            Arg             args[10] ;
            int             n ;
            String          newString ;
            Boolean         doSearch = FALSE ;
            XmFileSelectionBoxCallbackStruct searchData ;
/****************/

    BB_InSetValues( new_w) = TRUE ;


#ifdef DEC_MOTIF_RTOL
        /*      Update Direction
        */

        if( LayoutM(new_w) != LayoutM(current))
            UpdateDirection(new_w);
#endif /* DEC_MOTIF_RTOL */

    if(    FS_DirListLabelString( current) != FS_DirListLabelString( new_w)    )
    {   
        n = 0 ;
        XtSetArg( args[n], XmNlabelString, FS_DirListLabelString( new_w)) ; n++ ;
        XtSetArg( args[n], XmNlabelType, XmSTRING) ; n++ ;
        XtSetValues( FS_DirListLabel( new_w), args, n) ;
        FS_DirListLabelString( new_w) = NULL ;
        }
    if(    FS_FilterLabelString( current) != FS_FilterLabelString( new_w)    )
    {   
        n = 0 ;
        XtSetArg( args[n], XmNlabelString, FS_FilterLabelString( new_w)) ; n++ ;
        XtSetArg( args[n], XmNlabelType, XmSTRING) ; n++ ;
        XtSetValues( FS_FilterLabel( new_w), args, n) ;
        FS_FilterLabelString( new_w) = NULL ;
        }
    n = 0 ;
    if(    SB_ListVisibleItemCount( current)
                                          != SB_ListVisibleItemCount( new_w)    )
    {   XtSetArg( args[n], XmNvisibleItemCount, 
                                         SB_ListVisibleItemCount( new_w)) ; ++n ;
        } 
    if(    FS_DirListItems( new_w)    )
    {   
        XtSetArg( args[n], XmNitems, FS_DirListItems( new_w)) ; ++n ;
        FS_DirListItems( new_w) = NULL ;
        } 
    if(    FS_DirListItemCount( new_w) != XmUNSPECIFIED    )
    {   
        XtSetArg( args[n], XmNitemCount, FS_DirListItemCount( new_w)) ; ++n ;
        FS_DirListItemCount( new_w) = XmUNSPECIFIED ;
        } 

    if(    n    )
    {   XtSetValues( FS_DirList( new_w), args, n) ;
        } 

    if(    (SB_TextColumns( new_w) != SB_TextColumns( current))
        && FS_FilterText( new_w)    )
    {   
        n = 0 ;
        XtSetArg( args[n], XmNcolumns, SB_TextColumns( new_w)) ; ++n ;
        XtSetValues( FS_FilterText( new_w), args, n) ;
        }
    if(    FS_NoMatchString( new_w) != FS_NoMatchString( current)    )
    {   XmStringFree( FS_NoMatchString( current)) ;
        FS_NoMatchString( new_w) = XmStringCopy( FS_NoMatchString( new_w)) ;
        } 
    if(    !FS_QualifySearchDataProc( new_w)    )
    {   FS_QualifySearchDataProc( new_w) = QualifySearchDataProc ;
        } 
    if(    FS_DirSearchProc( new_w)  != FS_DirSearchProc( current)  )
    {   FS_StateFlags(new_w) |= XmFS_DIR_SEARCH_PROC ;
      /* in order to track the case where the directory does not
         change but the dirsearch proc does so we have to regenerate
         the dir list from scratch */
    } 
    if(    !FS_DirSearchProc( new_w)    )
    {   FS_DirSearchProc( new_w) = DirSearchProc ;
    } 
    if(    !FS_FileSearchProc( new_w)    )
    {   FS_FileSearchProc( new_w) = FileSearchProc ;
        } 
    /* The XmNdirSpec resource will be loaded into the Text widget by
    *   the Selection Box (superclass) SetValues routine.  It will be 
    *   picked-up there by the XmNqualifySearchDataProc routine to fill
    *   in the value field of the search data.
    */
    memset( &searchData, 0, sizeof( XmFileSelectionBoxCallbackStruct)) ;

    if(    FS_DirMask( new_w) != FS_DirMask( current)    )
    {   
        if(    FS_StateFlags( new_w) & XmFS_IN_FILE_SEARCH    )
        {   
            if(    FS_FilterText( new_w)    )
            {   
#ifdef I18N_EXTENSION
                DXmCSTextSetString ( FS_FilterText(new), FS_DirMask(new) );
#else

                newString = _XmStringGetTextConcat( FS_DirMask( new_w)) ;

                /* Should do this stuff entirely with XmStrings when the text
                *   widget supports it.
                */
#ifndef USE_TEXT_IN_DIALOGS
                XmTextFieldSetString( FS_FilterText( new_w), newString) ;
                if(    newString    )
                {   XmTextFieldSetCursorPosition( FS_FilterText( new_w),
			    XmTextFieldGetLastPosition( FS_FilterText( new_w))) ;
                    } 
#else
                XmTextSetString( FS_FilterText( new_w), newString) ;
                if(    newString    )
                {   XmTextSetCursorPosition( FS_FilterText( new_w),
			    XmTextGetLastPosition( FS_FilterText( new_w))) ;
                    } 
#endif
                XtFree( newString) ;
#endif

                }
            } 
        else
        {   doSearch = TRUE ;
            searchData.mask = XmStringCopy( FS_DirMask( request)) ;
            searchData.mask_length = XmStringLength( searchData.mask) ;
            } 
        FS_DirMask( new_w) = (XmString) XmUNSPECIFIED ;
        } 
    if(    FS_Directory( current) != FS_Directory( new_w)    )
    {   
        if(    FS_StateFlags( new_w) & XmFS_IN_FILE_SEARCH    )
        {   
            FS_Directory( new_w) = XmStringCopy( FS_Directory( request)) ;
            XmStringFree( FS_Directory( current)) ;
            } 
        else
        {   doSearch = TRUE ;
            searchData.dir = XmStringCopy( FS_Directory( request)) ;
            searchData.dir_length = XmStringLength( searchData.dir) ;

            /* The resource will be set to the new value after the Search
            *   routines have been called for validation.
            */
            FS_Directory( new_w) = FS_Directory( current) ;
            }
        }
    if(    FS_Pattern( current) != FS_Pattern( new_w)    )
    {   
        if(    FS_StateFlags( new_w) & XmFS_IN_FILE_SEARCH    )
        {   
            FS_Pattern( new_w) = XmStringCopy( FS_Pattern( request)) ;
            XmStringFree( FS_Pattern( current)) ;
            } 
        else
        {   doSearch = TRUE ;
            searchData.pattern = XmStringCopy( FS_Pattern( request)) ;
            searchData.pattern_length = XmStringLength( searchData.pattern) ;

            /* The resource will be set to the new value after the Search
            *   routines have been called for validation.
            */
            FS_Pattern( new_w) = FS_Pattern( current) ;
            }
        }
    if(    FS_FileTypeMask( new_w) != FS_FileTypeMask( current)    )
    {   
        if(    !(FS_StateFlags( new_w) & XmFS_IN_FILE_SEARCH)    )
        {   doSearch = TRUE ;
            } 
        }
    if(    doSearch    )
    {   
        FileSelectionBoxUpdate( new_w, &searchData) ;

        XmStringFree( searchData.value) ;
        XmStringFree( searchData.mask) ;
        XmStringFree( searchData.dir) ;
        XmStringFree( searchData.pattern) ;
        }
    BB_InSetValues( new_w) = FALSE ;

    if(    XtClass( new_w) == xmFileSelectionBoxWidgetClass    )
    {   
        _XmBulletinBoardSizeUpdate( (Widget) new_w) ;
        }
    return( FALSE) ;
    }

/****************************************************************/
static void
#ifdef _NO_PROTO
FSBGetDirectory( fs, resource, value)
            Widget fs ;
            int resource ;
            XtArgVal *value ;
#else
FSBGetDirectory(
            Widget fs,
            int resource,
            XtArgVal *value)
#endif
/****************           ARGSUSED
 * This does get values hook magic to keep the
 * user happy.
 ****************/
{
    XmString        data ;
/****************/
  
    data = XmStringCopy(FS_Directory(fs));
    *value = (XtArgVal) data ;

    return ;
    }
/****************************************************************/
static void
#ifdef _NO_PROTO
FSBGetNoMatchString( fs, resource, value)
            Widget fs ;
            int resource ;
            XtArgVal *value ;
#else
FSBGetNoMatchString(
            Widget fs,
            int resource,
            XtArgVal *value)
#endif
/****************           ARGSUSED
 * This does get values hook magic to keep the
 * user happy.
 ****************/
{
    XmString        data ;
/****************/
  
    data = XmStringCopy(FS_NoMatchString(fs));
    *value = (XtArgVal) data ;

    return ;
    }
/****************************************************************/
static void
#ifdef _NO_PROTO
FSBGetPattern( fs, resource, value)
            Widget fs ;
            int resource ;
            XtArgVal *value ;
#else
FSBGetPattern(
            Widget fs,
            int resource,
            XtArgVal *value)
#endif
/****************           ARGSUSED
 * This does get values hook magic to keep the
 * user happy.
 ****************/
{
    XmString        data ;
/****************/
  
    data = XmStringCopy(FS_Pattern(fs));
    *value = (XtArgVal) data ;

    return ;
    }
/****************************************************************
 * This does get values hook magic to keep the user happy.
 ****************/
static void 
#ifdef _NO_PROTO
FSBGetFilterLabelString( fs, resource_offset, value )
        Widget fs ;
        int resource_offset ;
        XtArgVal *value ;
#else
FSBGetFilterLabelString(
        Widget fs,
        int resource_offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
            XmString        data ;
            Arg             al[1] ;
/****************/

    XtSetArg( al[0], XmNlabelString, &data) ;
    XtGetValues( FS_FilterLabel( fs), al, 1) ;
    *value = (XtArgVal) data ;

    return ;
    }
/****************************************************************
 * This does get values hook magic to keep the user happy.
 ****************/
static void 
#ifdef _NO_PROTO
FSBGetDirListLabelString( fs, resource_offset, value )
        Widget fs ;
        int resource_offset ;
        XtArgVal *value ;
#else
FSBGetDirListLabelString(
        Widget fs,
        int resource_offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
            XmString        data ;
            Arg             al[1] ;
/****************/

    XtSetArg( al[0], XmNlabelString, &data) ;
    XtGetValues( FS_DirListLabel( fs), al, 1) ;
    *value = (XtArgVal) data ;

    return ;
    }
/****************************************************************
 * This does get values hook magic to keep the user happy.
 ****************/
static void 
#ifdef _NO_PROTO
FSBGetDirListItems( fs, resource_offset, value )
        Widget fs ;
        int resource_offset ;
        XtArgVal *value ;
#else
FSBGetDirListItems(
        Widget fs,
        int resource_offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
            XmString        data ;
            Arg             al[1] ;
/****************/

    XtSetArg( al[0], XmNitems, &data) ;
    XtGetValues( FS_DirList( fs), al, 1) ;
    *value = (XtArgVal) data ;

    return ;
    }
/****************************************************************
 * This does get values hook magic to keep the user happy.
 ****************/
static void 
#ifdef _NO_PROTO
FSBGetDirListItemCount( fs, resource_offset, value )
        Widget fs ;
        int resource_offset ;
        XtArgVal *value ;
#else
FSBGetDirListItemCount(
        Widget fs,
        int resource_offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
            XmString        data ;
            Arg             al[1] ;
/****************/

    XtSetArg( al[0], XmNitemCount, &data) ;
    XtGetValues( FS_DirList( fs), al, 1) ;
    *value = (XtArgVal) data ;

    return ;
    }
/****************************************************************
 * This does get values hook magic to keep the user happy.
 ****************/
static void 
#ifdef _NO_PROTO
FSBGetListItems( fs, resource_offset, value )
        Widget fs ;
        int resource_offset ;
        XtArgVal *value ;
#else
FSBGetListItems(
        Widget fs,
        int resource_offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
            XmString        data ;
            Arg             al[1] ;
/****************/

    if(    FS_StateFlags( fs) & XmFS_NO_MATCH    )
    {   
        *value = (XtArgVal) NULL ;
        } 
    else
    {   XtSetArg( al[0], XmNitems, &data) ;
        XtGetValues( SB_List( fs), al, 1) ;
        *value = (XtArgVal) data ;
        } 
    return ;
    }
/****************************************************************
 * This does get values hook magic to keep the user happy.
 ****************/
static void 
#ifdef _NO_PROTO
FSBGetListItemCount( fs, resource_offset, value )
        Widget fs ;
        int resource_offset ;
        XtArgVal *value ;
#else
FSBGetListItemCount(
        Widget fs,
        int resource_offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
            XmString        data ;
            Arg             al[1] ;
/****************/

    if(    FS_StateFlags( fs) & XmFS_NO_MATCH    )
    {   
        *value = (XtArgVal) 0 ;
        } 
    else
    {   XtSetArg( al[0], XmNitemCount, &data) ;
        XtGetValues( SB_List( fs), al, 1) ;
        *value = (XtArgVal) data ;
        } 

    return ;
    }
/****************************************************************
 * This does get values hook magic to keep the
 * user happy.
 ****************/
static void 
#ifdef _NO_PROTO
FSBGetDirMask( fs, resource_offset, value )
        Widget fs ;
        int resource_offset ;
        XtArgVal *value ;
#else
FSBGetDirMask(
        Widget fs,
        int resource_offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{   
            String          filterText ;
            XmString        data ;
/****************/

#ifdef I18N_EXTENSION
    data = DXmCSTextGetString ( FS_FilterText(fs) );
    *value = (XtArgVal) data ;
#else
#ifndef USE_TEXT_IN_DIALOGS
    filterText = XmTextFieldGetString( FS_FilterText(fs)) ;
#else
    filterText = XmTextGetString( FS_FilterText(fs)) ;
#endif
    data = XmStringLtoRCreate( filterText, XmFONTLIST_DEFAULT_TAG) ;
    *value = (XtArgVal) data ;
    XtFree( filterText) ; 
#endif

    return ;
    }

/****************************************************************/
static Widget 
#ifdef _NO_PROTO
GetActiveText( fsb, event )
        XmFileSelectionBoxWidget fsb ;
        XEvent *event ;
#else
GetActiveText(
        XmFileSelectionBoxWidget fsb,
        XEvent *event )
#endif /* _NO_PROTO */
{
            Widget          activeChild = NULL ;
/****************/

    if(    _XmGetFocusPolicy( (Widget) fsb) == XmEXPLICIT    )
    {   
        if(    (fsb->manager.active_child == SB_Text( fsb))
            || (fsb->manager.active_child == FS_FilterText( fsb))    )
        {   
            activeChild = fsb->manager.active_child ;
            } 
        } 
    else
    {   
#ifdef TEXT_IS_GADGET
        activeChild = _XmInputInGadget( (CompositeWidget) fsb, 
                                                          event->x, event->y) ;
        if(    (activeChild != SB_Text( fsb))
            && (activeChild != FS_FilterText( fsb))    )
        {   
            activeChild = NULL ;
            } 
#else /* TEXT_IS_GADGET */
        if(    SB_Text( fsb)
            && (XtWindow( SB_Text( fsb))
                                   == ((XKeyPressedEvent *) event)->window)   )
        {   activeChild = SB_Text( fsb) ;
            } 
        else
        {   if(    FS_FilterText( fsb)
                && (XtWindow( FS_FilterText( fsb)) 
                                  ==  ((XKeyPressedEvent *) event)->window)   )
            {   activeChild = FS_FilterText( fsb) ;
                } 
            } 
#endif /* TEXT_IS_GADGET */
        } 
    return( activeChild) ;
    }


/****************************************************************/
static void 
#ifdef _NO_PROTO
FileSelectionBoxUpOrDown( wid, event, argv, argc )
        Widget wid ;
        XEvent *event ;
        String *argv ;
        Cardinal *argc ;
#else
FileSelectionBoxUpOrDown(
        Widget wid,
        XEvent *event,
        String *argv,
        Cardinal *argc )
#endif /* _NO_PROTO */
{   
            XmFileSelectionBoxWidget fsb = (XmFileSelectionBoxWidget) wid ;
            int	            visible ;
            int	            top ;
            int	            key_pressed ;
            Widget	    list ;
            int	*           position ;
            int	            count ;
            Widget          activeChild ;
            Arg             av[5] ;
            Cardinal        ac ;
/****************/

    if(    !(activeChild = GetActiveText( fsb, event))    )
    {   return ;
        } 
    if(    activeChild == SB_Text( fsb)    )
    {   
        if(    FS_StateFlags( fsb) & XmFS_NO_MATCH    )
        {   return ;
            } 
        list = SB_List( fsb) ;
        position = &SB_ListSelectedItemPosition( fsb) ;
        } 
    else /* activeChild == FS_FilterText( fsb) */
    {   list = fsb->file_selection_box.dir_list ;
        position = &FS_DirListSelectedItemPosition( fsb) ;
        } 
    if(    !list    )
    {   return ;
        } 
    ac = 0 ;
    XtSetArg( av[ac], XmNitemCount, &count) ; ++ac ;
    XtSetArg( av[ac], XmNtopItemPosition, &top) ; ++ac ;
    XtSetArg( av[ac], XmNvisibleItemCount, &visible) ; ++ac ;
    XtGetValues( (Widget) list, av, ac) ;

    if(    !count    )
    {   return ;
        } 
    key_pressed = atoi( *argv) ;

    if(    *position == 0    )
    {   /*  No selection, so select first item.
        */
        XmListSelectPos( list, ++*position, True) ;
        } 
    else
    {   if(    !key_pressed && (*position > 1)    )
        {   /*  up  */
            XmListDeselectPos( list, *position) ;
            XmListSelectPos( list, --*position, True) ;
            }
        else
        {   if(    (key_pressed == 1) && (*position < count)    )
            {   /*  down  */
                XmListDeselectPos( list, *position) ;
                XmListSelectPos( list, ++*position, True) ;
                } 
            else
            {   if(    key_pressed == 2    )
                {   /*  home  */
                    XmListDeselectPos( list, *position) ;
                    *position = 1 ;
                    XmListSelectPos( list, *position, True) ;
                    } 
                else
                {   if(    key_pressed == 3    )
                    {   /*  end  */
                        XmListDeselectPos( list, *position) ;
                        *position = count ;
                        XmListSelectPos( list, *position, True) ;
                        } 
                    } 
                } 
            }
        } 
    if(    top > *position    )
    {   XmListSetPos( list, *position) ;
        } 
    else
    {   if(    (top + visible) <= *position    )
        {   XmListSetBottomPos( list, *position) ;
            } 
        } 
    return ;
    }
/****************************************************************/
static void 
#ifdef _NO_PROTO
FileSelectionBoxRestore( wid, event, argv, argc )
        Widget wid ;
        XEvent *event ;
        String *argv ;
        Cardinal *argc ;
#else
FileSelectionBoxRestore(
        Widget wid,
        XEvent *event,
        String *argv,
        Cardinal *argc )
#endif /* _NO_PROTO */
{   
            XmFileSelectionBoxWidget fsb = (XmFileSelectionBoxWidget) wid ;
            String          itemString ;
            String          dir ;
            String          mask ;
            int             dirLen ;
            int             maskLen ;
            Widget          activeChild ;
/****************/

    if(    !(activeChild = GetActiveText( fsb, event))    )
    {   return ;
        } 
    if(    activeChild == SB_Text( fsb)    )
    {   _XmSelectionBoxRestore( (Widget) fsb, event, argv, argc) ;
        } 
    else /* activeChild == FS_FilterText( fsb) */
    {   /* Should do this stuff entirely with XmStrings when the text
        *   widget supports it.
        */
#ifdef I18N_EXTENSION
        {
            XmString itemStringCS;
            itemStringCS =
                XmStringConcat ( FS_Directory(fsb), FS_Pattern(fsb) );
            DXmCSTextSetString ( FS_FilterText(fsb), itemStringCS );
            XmStringFree ( itemStringCS );
        }
#else
        if(    dir = _XmStringGetTextConcat( FS_Directory( fsb))    )
        {   
            dirLen = strlen( dir) ;

            if(    mask = _XmStringGetTextConcat( FS_Pattern( fsb))    )
            {   
                maskLen = strlen( mask) ;
                itemString = XtMalloc( dirLen + maskLen + 1) ;
                strcpy( itemString, dir) ;
                strcpy( &itemString[dirLen], mask) ;
#ifndef USE_TEXT_IN_DIALOGS
                XmTextFieldSetString( FS_FilterText( fsb), itemString) ;
                XmTextFieldSetCursorPosition( FS_FilterText( fsb),
			    XmTextFieldGetLastPosition( FS_FilterText( fsb))) ;
#else
                XmTextSetString( FS_FilterText( fsb), itemString) ;
                XmTextSetCursorPosition( FS_FilterText( fsb),
			    XmTextGetLastPosition( FS_FilterText( fsb))) ;
#endif
                XtFree( itemString) ;
                XtFree( mask) ;
                } 
            XtFree( dir) ;
            }
#endif
        } 
    return ;
    }
/****************************************************************/
static void 
#ifdef _NO_PROTO
FileSelectionBoxFocusMoved( wid, client_data, data )
        Widget wid ;
        XtPointer client_data ;
        XtPointer data ;
#else
FileSelectionBoxFocusMoved(
        Widget wid,
        XtPointer client_data,
        XtPointer data )
#endif /* _NO_PROTO */
{            
            XmFocusMovedCallbackStruct * call_data
                                        = (XmFocusMovedCallbackStruct *) data ;
            Widget          ancestor ;
/****************/

    if(    !call_data->cont    )
    {   /* Preceding callback routine wants focus-moved processing
        *   to be discontinued.
        */
        return ;
        } 

    if(    call_data->new_focus
        && (   (call_data->new_focus == FS_FilterText( client_data))
            || (call_data->new_focus == FS_DirList( client_data)))
        && XtIsManaged( SB_ApplyButton( client_data))    )
    {   
        BB_DefaultButton( client_data) = SB_ApplyButton( client_data) ;
        }
 
 /*
  * Fix for 4110 - Check to see if the new_focus is NULL.  If it is, check
  *                to see if the default button has been set.  If not, set
  *                it to the OkButton.  Then, check if the new_focus is
  *                either the File list or the File name text field.  If
  *                they are, set the default button to the OkButton.
  *                Otherwise, leave the default button alone.
  */
     else if (!call_data->new_focus && (BB_DefaultButton(client_data)) == NULL)
      {   BB_DefaultButton( client_data) = SB_OkButton( client_data) ;
          }
     else if (call_data->new_focus
              && ((call_data->new_focus == SB_Text(client_data))
              || (call_data->new_focus == SB_List(client_data))))
     {   BB_DefaultButton( client_data) = SB_OkButton( client_data) ;
         }
 /*
  * End Fix 4110
  */
      else
    {   BB_DefaultButton( client_data) = SB_OkButton( client_data) ;
        }

    _XmBulletinBoardFocusMoved( wid, client_data, call_data) ;

    /* Since the focus-moved callback of an ancestor bulletin board may
    *   have already been called, we must make sure that it knows that
    *   we have changed our default button.  So, walk the hierarchy and
    *   synchronize the dynamic default button of all ancestor bulletin 
    *   board widgets.
    */
    if(    call_data->cont    )
    {   
        ancestor = XtParent( (Widget) client_data) ;
        
        while(    ancestor  &&  !XtIsShell( ancestor)    )
        {   
            if(    XmIsBulletinBoard( ancestor)    )
            {   
                if(    BB_DynamicDefaultButton( ancestor)
                    && BB_DynamicDefaultButton( client_data)    )
                {   
                    _XmBulletinBoardSetDynDefaultButton( ancestor, 
                                       BB_DynamicDefaultButton( client_data)) ;
                    } 
                } 
            ancestor = XtParent( ancestor) ;
            } 
        } 
    return ;
    }

/****************************************************************
 * This is the procedure which does all of the button
 *   callback magic.
 ****************/
static void 
#ifdef _NO_PROTO
FileSelectionPB( wid, which_button, call_data )
        Widget wid ;
        XtPointer which_button ;
        XtPointer call_data ;
#else
FileSelectionPB(
        Widget wid,
        XtPointer which_button,
        XtPointer call_data )
#endif /* _NO_PROTO */
{   
            XmAnyCallbackStruct * callback = (XmAnyCallbackStruct *) call_data;
            XmFileSelectionBoxWidget fs ;
            XmFileSelectionBoxCallbackStruct searchData ;
            XmFileSelectionBoxCallbackStruct qualifiedSearchData ;
            Boolean         match = True ;
            String          text_value ;
            Boolean         allowUnmanage = FALSE ;
/****************/

    fs = (XmFileSelectionBoxWidget) XtParent( wid) ;

    searchData.reason = XmCR_NONE ;
    searchData.event = callback->event ;
    searchData.value = NULL ;
    searchData.length = 0 ;
    searchData.mask = NULL ;
    searchData.mask_length = 0 ;
    searchData.dir = NULL ;
    searchData.dir_length = 0 ;
    searchData.pattern = NULL ;
    searchData.pattern_length = 0 ;
                
    if(    ((int) which_button) == XmDIALOG_APPLY_BUTTON    )
    {   
#ifdef I18N_EXTENSION
        XmString text_value_cs;
        if(    FS_FilterText( fs)
            && ( text_value_cs = DXmCSTextGetString( FS_FilterText( fs))) )
        {
            searchData.mask = text_value_cs;
            searchData.mask_length = XmStringLength( searchData.mask) ;
            }
#else
#ifndef USE_TEXT_IN_DIALOGS
        if(    FS_FilterText( fs)
            && (text_value = XmTextFieldGetString( FS_FilterText( fs)))    )
#else
        if(    FS_FilterText( fs)
            && (text_value = XmTextGetString( FS_FilterText( fs)))    )
#endif
        {   
            searchData.mask = XmStringLtoRCreate( text_value, 
                                                    XmFONTLIST_DEFAULT_TAG) ;
            searchData.mask_length = XmStringLength( searchData.mask) ;
            XtFree( text_value) ;
            } 
#endif
        searchData.reason = XmCR_NONE ;

        FileSelectionBoxUpdate( fs, &searchData) ;

        XmStringFree( searchData.mask) ;
        searchData.mask = NULL ;
        searchData.mask_length = 0 ;
        }

    /* Use the XmNqualifySearchDataProc routine to fill in all fields of the
    *   callback data record.
    */
    (*FS_QualifySearchDataProc( fs))( (Widget) fs, (XtPointer) &searchData,
                                            (XtPointer) &qualifiedSearchData) ;
    switch(    (int) which_button    )
    {   
        case XmDIALOG_OK_BUTTON:
        {   
            if(    SB_MustMatch( fs)    )
            {   
                match = XmListItemExists( SB_List( fs),
                                                   qualifiedSearchData.value) ;
                }
            if(    !match    )
            {   
                qualifiedSearchData.reason = XmCR_NO_MATCH ;
                XtCallCallbackList( ((Widget) fs),
                   fs->selection_box.no_match_callback, &qualifiedSearchData) ;
                }
            else
            {   qualifiedSearchData.reason = XmCR_OK ;
                XtCallCallbackList( ((Widget) fs),
                         fs->selection_box.ok_callback, &qualifiedSearchData) ;
                }
            allowUnmanage = TRUE ;
            break ;
            }
        case XmDIALOG_APPLY_BUTTON:
        {   
            qualifiedSearchData.reason = XmCR_APPLY ;
            XtCallCallbackList( ((Widget) fs),
                      fs->selection_box.apply_callback, &qualifiedSearchData) ;
            break ;
            }
        case XmDIALOG_CANCEL_BUTTON:
        {   
            qualifiedSearchData.reason = XmCR_CANCEL ;
            XtCallCallbackList( ((Widget) fs),
                     fs->selection_box.cancel_callback, &qualifiedSearchData) ;
            allowUnmanage = TRUE ;
            break ;
            }
        case XmDIALOG_HELP_BUTTON:
        {   
            if(    fs->manager.help_callback    )
            {   
                qualifiedSearchData.reason = XmCR_HELP ;
                XtCallCallbackList( ((Widget) fs),
                             fs->manager.help_callback, &qualifiedSearchData) ;
                }
            else
            {   _XmManagerHelp((Widget) fs, callback->event, NULL, NULL) ;
                } 
            break ;
            }
        }
    XmStringFree( qualifiedSearchData.pattern) ;
    XmStringFree( qualifiedSearchData.dir) ;
    XmStringFree( qualifiedSearchData.mask) ;
    XmStringFree( qualifiedSearchData.value) ;

    if(    allowUnmanage
        && fs->bulletin_board.shell
        && fs->bulletin_board.auto_unmanage   )
    {   
        XtUnmanageChild( (Widget) fs) ;
        } 
    return ;
    }

/****************************************************************
 * This function returns the widget id of the
 *   specified SelectionBox child widget.
 ****************/
Widget 
#ifdef _NO_PROTO
XmFileSelectionBoxGetChild( fs, which )
        Widget fs ;
        unsigned char which ;
#else
XmFileSelectionBoxGetChild(
        Widget fs,
#if NeedWidePrototypes
        unsigned int which )
#else
        unsigned char which )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{   
            Widget          child ;
/****************/

    switch(    which    )
    {   
        case XmDIALOG_DIR_LIST:
        {   child = FS_DirList( fs) ;
            break ;
            } 
        case XmDIALOG_DIR_LIST_LABEL:
        {   child = FS_DirListLabel( fs) ;
            break ;
            } 
        case XmDIALOG_FILTER_LABEL:
        {   child = FS_FilterLabel( fs) ;
            break ;
            } 
        case XmDIALOG_FILTER_TEXT:
        {   child = FS_FilterText( fs) ;
            break ;
            }
        default:
        {   child = XmSelectionBoxGetChild( fs, which) ;
            break ;
            }
        }
    return( child) ;
    }

/****************************************************************/
void 
#ifdef _NO_PROTO
XmFileSelectionDoSearch( fs, dirmask )
        Widget fs ;
        XmString dirmask ;
#else
XmFileSelectionDoSearch(
        Widget fs,
        XmString dirmask )
#endif /* _NO_PROTO */
{   
            XmFileSelectionBoxCallbackStruct searchData ;
            String          textString ;
/****************/

    searchData.reason = XmCR_NONE ;
    searchData.event = 0 ;
    searchData.value = NULL ;
    searchData.length = 0 ;
    searchData.dir = NULL ;
    searchData.dir_length = 0 ;
    searchData.pattern = NULL ;
    searchData.pattern_length = 0 ;

    if(    dirmask    )
    {   
        searchData.mask = XmStringCopy( dirmask) ;
        searchData.mask_length = XmStringLength( searchData.mask) ;
        }
    else
#ifdef I18N_EXTENSION
    {   if(    FS_FilterText( fs)    )
        {
            XmString textStringCS;
            if (textStringCS = DXmCSTextGetString (FS_FilterText (fs))){
                searchData.mask = textStringCS;
                searchData.mask_length = XmStringLength( searchData.mask);
            } else
              {   searchData.mask = NULL ;
                  searchData.mask_length = 0 ;
                  }
        }
        else
        {   searchData.mask = NULL ;
            searchData.mask_length = 0 ;
            }
        }
#else
    {   if(    FS_FilterText( fs)    )
        {   
#ifndef USE_TEXT_IN_DIALOGS
            textString = XmTextFieldGetString( FS_FilterText( fs)) ;
#else
            textString = XmTextGetString( FS_FilterText( fs)) ;
#endif
            } 
        else
        {   textString = NULL ;
            } 
        if(    textString    )
        {   searchData.mask = XmStringLtoRCreate( textString, 
                                                    XmFONTLIST_DEFAULT_TAG) ;
            searchData.mask_length = XmStringLength( searchData.mask) ;
            XtFree( textString) ;
            } 
        else
        {   searchData.mask = NULL ;
            searchData.mask_length = 0 ;
            } 
        }
#endif 
    FileSelectionBoxUpdate( (XmFileSelectionBoxWidget) fs, &searchData) ;

    XmStringFree( searchData.mask) ;
    return ;
    }

/****************************************************************/
Widget 
#ifdef _NO_PROTO
XmCreateFileSelectionBox( p, name, args, n )
        Widget p ;
        String name ;
        ArgList args ;
        Cardinal n ;
#else
XmCreateFileSelectionBox(
        Widget p,
        String name,
        ArgList args,
        Cardinal n )
#endif /* _NO_PROTO */
{
/****************/

    return( XtCreateWidget( name, xmFileSelectionBoxWidgetClass, p, args, n));
    }
/****************************************************************
 * This convenience function creates a DialogShell
 *   and a FileSelectionBox child of the shell;
 *   returns the FileSelectionBox widget.
 ****************/
Widget 
#ifdef _NO_PROTO
XmCreateFileSelectionDialog( ds_p, name, fsb_args, fsb_n )
        Widget ds_p ;
        String name ;
        ArgList fsb_args ;
        Cardinal fsb_n ;
#else
XmCreateFileSelectionDialog(
        Widget ds_p,
        String name,
        ArgList fsb_args,
        Cardinal fsb_n )
#endif /* _NO_PROTO */
{   
            Widget          fsb ;       /*  new fsb widget      */
            Widget          ds ;        /*  DialogShell         */
            ArgList         ds_args ;   /*  arglist for shell  */
            char *          ds_name ;
/****************/

    /*  Create DialogShell parent.
    */
    ds_name = XtCalloc( strlen( name)+XmDIALOG_SUFFIX_SIZE+1, sizeof( char)) ;
    strcpy( ds_name, name) ;
    strcat( ds_name, XmDIALOG_SUFFIX) ;

    ds_args = (ArgList) XtMalloc( sizeof( Arg) * (fsb_n + 1)) ;
    memcpy( ds_args, fsb_args, (sizeof( Arg) * fsb_n)) ;
    XtSetArg( ds_args[fsb_n], XmNallowShellResize, True) ; 
    ds = XmCreateDialogShell( ds_p, ds_name, ds_args, fsb_n + 1) ;

    XtFree((char *) ds_args) ;
    XtFree(ds_name) ;


    /*  Create FileSelectionBox.
    */
    fsb = XtCreateWidget( name, xmFileSelectionBoxWidgetClass, ds, 
                                                             fsb_args, fsb_n) ;
    XtAddCallback( fsb, XmNdestroyCallback, _XmDestroyParentCallback, NULL) ;

    return( fsb) ;
    }

#ifdef DEC_MOTIF_EXTENSION

#include <X11/StringDefs.h>

/************************************************************************/
/*                                                                      */
/* CreateWaitCursor                                                     */
/*                                                                      */
/* FUNCTIONAL DESCRIPTION:                                              */
/*                                                                      */
/*      Creates a wait cursor for the file selection widget             */
/*                                                                      */
/*      Code copied from _DXmCreateWaitCursor                           */
/*                                                                      */
/*                                                                      */
/* FORMAL PARAMETERS:                                                   */
/*                                                                      */
/*      wid             Widget id to create wait cursor for.            */
/*                                                                      */
/* IMPLICIT INPUTS:                                                     */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* IMPLICIT OUTPUTS:                                                    */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* FUNCTION VALUE:                                                      */
/*                                                                      */
/*      A Cursor which means "wait."                                    */
/*                                                                      */
/* SIDE EFFECTS:                                                        */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
static Cursor CreateWaitCursor(wid)
    Widget wid;
{
    Cursor              wait_cursor = 0;
    char *              cursor_name;
    XrmValue            from_val, to_val;
    int status;

    /********************************************************************/
    /*                                                                  */
    /* Try to get Digital's wait cursor.  If we can't find the wait     */
    /* cursor font, just load the X watch cursor.                       */
    /*                                                                  */
    /********************************************************************/
    /*
     *  Call the string to cursor converter
     */
    from_val.size = strlen("decw_wait_cursor") + 1;
    from_val.addr = (XtPointer)"decw_wait_cursor";
    to_val.addr = (XtPointer)&wait_cursor;
    to_val.size = sizeof(Cursor);
    status = XtConvertAndStore(wid, XtRString, &from_val, XtRCursor, &to_val);

    return (wait_cursor);

} /* CreateWaitCursor */

#endif /* DEC_MOTIF_EXTENSION */


