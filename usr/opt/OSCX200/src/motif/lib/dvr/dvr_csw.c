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
#define Module DVR_CSW
#define Ident  "V1-025"
/*                         
**++
**
**  COPYRIGHT (c) 1991, 1992 BY
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
**           
**  FACILITY:                                             
**	Compound Document Architecture Toolkit
**                                                                 
**  ABSTRACT:
**	This module is called from an application which has already created
**	a top level widget and wants to obtain CDA converter file and 
*	format information prior to initiating a call to CDA_CONVERT.
**             
**  ENVIRONMENT:
**	Transportable.                             
**                    
**  AUTHOR:                                                
**      Roy F. Stone, March 1990
**
**                                                       
**  MODIFICATION HISTORY:                                           
**
**	V1.025	RDH006		Don Haney		24-Aug-1992
**		Resequence includes to eliminate MAX redefined warning
**
**	V1.024	RDH003		Don Haney		15-Jul-1992
**		Specify same-directory includes with "", not <>
**
**	V1.023	RDH001		Don Haney		08-Jul-1992
**		Add casts required for strict checking in OSF1/ALPHA
**
**	V1.022	DAM000		Dennis McEvoy		29-Apr-1992
**		make sure get_path_dir returns status properly
**
**	V1.021	ECR000		Elizabeth C. Rust	06-Apr-1992
**		Fix memory leaks on StringContext.
**
**	V1.020	CJR000		Chris Ralto	19-Feb-1992
**		Move V1 entry point routines from this module
**		into new module dvr_olde.
**
**	V1.012A3	DAM000		Dennis McEvoy	20-nov-1991
**		do not free memory from getenv
**
**	V1.018	RFS009		Roy F. Stone	  	20-Nov-1991
**		(1) Added "#ifdef osf1" for directory structure definition.
**		(2) Added test for a null pointer for callback list.
**		(3) Removed erroneous test for only lower case alpha char.
**		    in search for NEW_STYLE converter files.
**
**	V1.017	DAM000		Dennis McEvoy		18-oct-1991
**		more cleanups for ANSI C compiles
**
**	V1.016	DAM000		Dennis McEvoy		03-oct-1991
**		cleanup X calls to match protos
**
**	V1.015	RFS009		Roy F. Stone		29-Aug-1991
**		Moved deallocation of format list to main entry point and
**		to the destroy-widget callback function.
**
**	V1.014	DAM000		Dennis McEvoy		12-aug-1991
**		rename static vars for shared libs on osf/1
**
**	V1.013	DAM000		Dennis McEvoy		05-aug-1991
**		rename headers, remove dollar signs
**
**	V1.012	RFS008		Roy. F. Stone		15-Jul-1991
**		Fixed bug in override format list function.
**                 
**	V1.011	RFS007		Roy. F. Stone		11-Jul-1991
**		Significant changes to program to provide the ability to
**		search various Ultrix directories for available converter 
**		modules in order to construct the format search list.
**		Change to allow dynamic allocation and reallocation of 
**		array of pointers to format strings.
**
**	V1.010	SJM000		Stephen Munyan		 1-Jul-1991
**		DEC C Cleanups
**
**	V1.009  DAM000		Dennis McEvoy		10-Jun-1991
**		mods for osf/1
**
**	V1.008  RFS006		Roy F. Stone		28-May-1991
**		Temporarily disabled help callback function.
**
**	V1.007  RFS006		Roy F. Stone		10-Apr-1991
**		Modify code for portablility.
**
**	V1.006  RFS005		Roy F. Stone		13-Mar-1991
**		Added code to support help button and produce a help widget.
**
**	V1.005	SJM000		Stephen Munyan		30-Dec-1990
**		Fixed a 10 byte memory leak that was caused due to 
**		XmStringGetNextSegment converting from an int to a
**		char* as the return value for character sets.
**
**	V1.004	RFS004		Roy F. Stone		28-Sep-1990
**		Move key word definitions to DVR_DECW_DEF.H.
**
**	V1.003	SJM000		Stephen Munyan		10-Sep-1990
**		Fix ordering of #include files for PREIFT baselevel
**		to work around Toolkit fixups.
**                                        
**	V1.002	RFS003		Roy F. Stone		21-Aug-1990
**		Miscellaneous cleanup during testing.
**
**	V1.001	RFS002		Roy F. Stone		18-July-1990
**		Converted to Motif.
**
**	X1.001  RFS001		Roy F. Stone		13-March-1990
**		Initial creation.
*/                                                                    
#include <cdatrans.h>

#ifdef __vms__

#include <cdaityp.h>
#include <cdadef.h>                            
#include <cdamsg.h>
#include <rmsdef.h>
#include <descrip.h>

#endif

#ifdef __unix__

#include <cdaityp.h>
#include <cdadef.h>                                                
#include <cdamsg.h>

#include <sys/types.h>
#include <sys/dir.h>
#endif                               

#ifdef __vms__
#pragma nostandard                             
#endif

#include <X11/Xlib.h>                       
#include <X11/Intrinsic.h>                 
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <Xm/Text.h>
#include <Xm/XmP.h>
#include <Xm/SelectioB.h>
#include <Mrm/MrmPublic.h>
#include <stdio.h>                         
#include <DXm/DECspecific.h>
#include <DXm/DXmHelpB.h>
#include <X11/ShellP.h>
#include <DXm/DXmHelpSP.h>
                                   
#ifdef __vms__
#pragma standard
#endif                               

#include "dvrwdef.h"
#include <dvrmsg.h>

/*
 * The following prototypes have been added to get rid of compilation
 * warnings in DEC C when /PORTABLE is used.  Eventually new versions of
 * starlet.h will be made available that have correct proto support.
 * When that time comes these proto statements should be removed.
*/

#ifdef __vms__

PROTO (CDAstatus LIB$FIND_FILE, ());
PROTO (CDAstatus LIB$FIND_FILE_END, ());

#endif

/* Define id codes to match those in UIL file. */           
                                  
#define	FILE_SEL_ID		 2
#define FORMAT_LABEL_ID		 3          
#define	FORMAT_TEXT_ID		 4
#define	LIST_BOX_ID		 5
#define	OPTIONS_BUTTON_ID	 6
#define	OPTIONS_FILE_SEL_ID    	 7
#define OPT_LABEL_BUTTON_ID 	 8
#define OPT_ICON_BUTTON_ID	 9
#define MAX_WIDGETS		10

#define HELP_BUTTON_ID	       101
#define HELP_EXIT              102
	                                                                 
#define TOPLEVEL_WIDGET		csw->widget_table[0]			/* 0 */
#define CSW_MAIN_WINDOW		csw->widget_table[1]                    /* 1 */
#define FILE_SEL_BOX		csw->widget_table[FILE_SEL_ID]          /* 2 */
#define FORMAT_LABEL		csw->widget_table[FORMAT_LABEL_ID]      /* 3 */
#define FORMAT_TEXT    		csw->widget_table[FORMAT_TEXT_ID]       /* 4 */
#define LIST_BOX		csw->widget_table[LIST_BOX_ID]          /* 5 */
#define OPTIONS_FILE_BUTTON	csw->widget_table[OPTIONS_BUTTON_ID]    /* 6 */
#define OPTIONS_FILE_SEL_BOX	csw->widget_table[OPTIONS_FILE_SEL_ID]  /* 7 */
#define OPTIONAL_LABEL_BUTTON	csw->widget_table[OPT_LABEL_BUTTON_ID]  /* 8 */
#define OPTIONAL_ICON_BUTTON	csw->widget_table[OPT_ICON_BUTTON_ID]   /* 9 */
                                                          
#define CHECK_VALUE		55555                
#define FORMAT_ARRAY_INCREMENT	50
                                
typedef struct item_list	CDA_ITEMLIST;             
                                                   
/* Context structure. */        
                                
typedef struct csw_context {                           
	CDAconstant	context_check_value;
	MrmHierarchy   	mrmhierarchy;
	Widget		*context_parent;
	CDAflags	option_flags;
	XmString	**format_list;
	CDAindex	format_limit;
	CDAindex	format_count;
	XmString	*override_format_list;                
  	XmString	*initial_format[1];
	XmString	*options_file_spec;               
   	CDAindex	options_file_spec_len;         
   	XtCallbackList 	callback_list;                      
	Widget		widget_table[MAX_WIDGETS];           

#ifdef __unix__
	CDAenvirontext  **ultrix_list;
	CDAindex	ultrix_limit;
	CDAindex	ultrix_count;
#endif
                        
} CswContext, *CswContextPtr;
                
/* Declaration of functions. */    
                                            
PROTO (void CDA_APIENTRY csw_get_something, (Widget, char *, XtArgVal));
PROTO (void CDA_APIENTRY csw_set_something, (Widget, char *, XtArgVal)); 
PROTO (void CDA_APIENTRY csw_store_format, (CswContextPtr, XmString *));
PROTO (void CDA_APIENTRY csw_store_ultrix_format, (CswContextPtr, CDAenvirontext *));
PROTO (CswContextPtr CDA_APIENTRY csw_get_context, (Widget, CDAindex));
PROTO (void CDA_APIENTRY csw_create_new_file_filter, (CswContextPtr,
	CDAenvirontext *));
PROTO (void CDA_CALLBACK csw_create_proc, (Widget, Opaque, 
	XmAnyCallbackStruct *));
PROTO (void CDA_CALLBACK csw_selection_proc, (Widget, Opaque, 
	XmFileSelectionBoxCallbackStruct *));                           
PROTO (void CDA_CALLBACK csw_list_sel_proc, (Widget, Opaque, 
   	XmListCallbackStruct *));
PROTO (void CDA_CALLBACK csw_options_activate_proc, (Widget, Opaque, 
 	XmAnyCallbackStruct *));                               
PROTO (void CDA_CALLBACK csw_options_ok_proc, (Widget, Opaque, 
	XmFileSelectionBoxCallbackStruct *));                   
PROTO (void CDA_CALLBACK csw_help_proc, (Widget, Opaque, 
	XmAnyCallbackStruct *));
PROTO (void CDA_CALLBACK csw_options_cancel_proc, (Widget, Opaque, 
	XmFileSelectionBoxCallbackStruct *));
PROTO (void CDA_CALLBACK csw_destroy_proc, (Widget, Opaque, 
	XmAnyCallbackStruct *));
PROTO (CDAstatus CDA_APIENTRY DvrConverterFileSelection, (Widget *, Widget *,
	CDAflags, CDAindex, CDAindex, CDAenvirontext *, CDAboolean, 
	XtCallbackList));                 
PROTO (CDAstatus CDA_APIENTRY DvrConvFileSelectionCreate, (Widget *,
	Widget *, CDAitemlist *, XtCallbackList));

#ifdef __unix__
PROTO (int get_path_dir, (CDAenvirontext **, CDAenvirontext **));
PROTO (int search_directory, (CswContextPtr, DIR *, CDAindex));
#endif
                                                 
static MrmRegisterArg reglist[] = {                                       
        {"csw_create_proc", (caddr_t) csw_create_proc},
	{"csw_selection_proc", (caddr_t) csw_selection_proc},
	{"csw_list_sel_proc", (caddr_t) csw_list_sel_proc},
        {"csw_options_activate_proc", (caddr_t) csw_options_activate_proc},
	{"csw_options_ok_proc", (caddr_t) csw_options_ok_proc},
	{"csw_help_proc", (caddr_t) csw_help_proc},
	{"csw_options_cancel_proc", (caddr_t) csw_options_cancel_proc},
	{"csw_destroy_proc", (caddr_t) csw_destroy_proc}
};                  
                                                                     
static CDAindex reglist_num = (sizeof reglist / sizeof reglist[0]);

static	CDAenvirontext	null_string[] = "";

#ifdef __vms__
static 	CDAenvirontext  *file_name_vec[] = {"ddif$csw.uid"};
static	CDAenvirontext	ddif_format_string[] = "DDIF";
static	CDAenvirontext	dtif_format_string[] = "DTIF";
#endif

#ifdef __unix__

#define OLD_STYLE 0
#define NEW_STYLE 1
          
static 	CDAenvirontext  *file_name_vec[] = {"ddif_csw_1_2.uid"};
static	CDAenvirontext	ddif_format_string[] = "ddif";
static	CDAenvirontext	dtif_format_string[] = "dtif";
static  CDAenvirontext  *old_new_format_table[5][2] = {
	{"tab","ascii_tabular"},                               
	{"calc","calcgrd"},
	{"ghc","graphics"},
	{"macp","macpaint"},
	{"anls","analysis"}
};
#endif

struct  DvrConverterSelectionCallback {
	CDAindex	reason;
	XEvent		*event;      
	XmString	*file_spec;
	CDAindex	file_spec_len; 
	XmString	*file_format;
	CDAindex	file_format_len;                           
	XmString	*options_file_spec;                         
	CDAindex	options_file_len;
};                                                    
                    
#ifdef __vms__

static CDAenvirontext *input_spec_list[] = { 
       	"sys$share:cda$read_*.exe",
	"sys$share:ddif$read_*.exe",          
	"sys$share:dtif$read_*.exe"};            

static CDAenvirontext *output_spec_list[] = {
	"sys$share:cda$write_*.exe",
	"sys$share:ddif$write_*.exe",
	"sys$share:dtif$write_*.exe"};                              
          
static CDAenvirontext *file_type_table[5][2] = {
	{"ANALYSIS","CDA$ANALYSIS"},
	{"ASCII_TABULAR","TAB"},
	{"CALCGRD","CALC$GRD"},
	{"DCA","RFT"},   
	{"TEXT","TXT"}                                              
	};
#endif              

#ifdef __unix__

static CDAenvirontext ultrix_directory[] = "/usr/bin";
  
static CDAenvirontext *ultrix_input_spec_list[] = {
	"cda_read_",
	"ddif_read_",
	"dtif_read_"};

static CDAenvirontext *ultrix_output_spec_list[] = {
	"cda_write_",
	"ddif_write_",
	"dtif_write_"};

static CDAenvirontext *file_type_table[5][2] = {
	{"analysis","cda_analysis"},
	{"ascii_tabular","tab"},
	{"calcgrd","calc_grd"},
	{"dca","rft"},   
	{"text","txt"}
	};
#endif
                  
/*                             
**++                                                           
**  FUNCTIONAL DESCRIPTION:                                  
**
**	MIT C high-level entry point for the Converter Selection Widget.
**                
**  FORMAL PARAMETERS:                   
**             
**	None.                        
**             
**  IMPLICIT INPUTS:
**	None.                                                               
**                                
**  IMPLICIT OUTPUTS:           
**	None.                            
**                                                         
**  FUNCTION VALUE:                                    
**	As returned by CDA converter routine
**	As returned by DDIF file routine
**                              
**  SIDE EFFECTS:
**                                    
**
**--     
**/ 
CDAstatus CDA_APIENTRY DvrConverterFileSelection (parent_widget,
			   sel_box_widget,                         
			   option_flags,
			   sel_box_x,
			   sel_box_y,                          
			   filter_mask,
			   default_position,
			   callback_ptr)
Widget		*parent_widget;           
Widget		*sel_box_widget;                                        
CDAflags	option_flags;          
CDAindex	sel_box_x;
CDAindex	sel_box_y;                                         
CDAenvirontext	*filter_mask;
CDAboolean	default_position;
XtCallbackList 	callback_ptr;
{
	CDAitemlist 	item_list[3];            
	CDAitemlist	*itm;                         
	Arg    		arglist[4];	
	CDAindex	arg_count = 0;
	CDAstatus	status;
                                                     
	itm = item_list;                                   
                                                       
	itm->item_code = DvrOptionFlags;
	itm->item_length = 0;                
	itm->CDAitemparam.item_value = option_flags;
	itm += 1;

	XtSetArg(arglist[arg_count], XmNx, sel_box_x);
	++arg_count;
	XtSetArg(arglist[arg_count], XmNy, sel_box_y);
	++arg_count;
	XtSetArg(arglist[arg_count], XmNdefaultPosition, default_position);
	++arg_count;       
	if (filter_mask != '\0')
	{
		XtSetArg(arglist[arg_count], XmNdirMask, filter_mask); 
		++arg_count;
	}
	itm->item_code = DvrFileSelectionOverride;
	itm->item_length = arg_count;	   
	itm->CDAitemparam.item_address = arglist;
	itm += 1;

	itm->item_code = 0;
	itm->item_length = 0;
    
	status = DvrConvFileSelectionCreate(parent_widget,
						 sel_box_widget,
	       					 item_list,
						 callback_ptr);
	return status;
}
                  
/*                                                         
**++                                                           
**  FUNCTIONAL DESCRIPTION:                                  
**                                                            
**	MIT C low-level entry point for the Converter Selection Widget.
**                
**  FORMAL PARAMETERS:                   
**                                     
**	None.
**             
**  IMPLICIT INPUTS:
**	None.                                                               
**                                
**  IMPLICIT OUTPUTS:
**	None.                            
**               
**  FUNCTION VALUE:
**	As returned by CDA converter routine
**	As returned by DDIF file routine
**                              
**  SIDE EFFECTS:
**
**
**--                                   
**/                                      
CDAstatus	DvrConvFileSelectionCreate (parent_widget,
		 	   	sel_box_widget,        
		   		itm,  
				callback_ptr)
Widget		*parent_widget;           
Widget		*sel_box_widget;                    
CDAitemlist	*itm;
XtCallbackList	callback_ptr;
{                                                    
	CswContextPtr 	csw;
	XtCallbackList 	activate_callback_list;
	Widget		file_format_box;                              
	MrmType 	dummy_class;      
	CDAindex	byte_count;
	CDAsize		c_status;
	CDAboolean	optional_button_sw = 0;
	Arg 		arglist[4];                             
        CDAindex	x;
	CDAindex	n, n1, n2;
	CDAstatus	status;
	CDAindex	displ;           
 	CDAindex	find_file_context;
	CDAenvirontext	format_string[256];
	CDAenvirontext	*y, *z;     
#ifdef __vms__                                          
	struct dsc$descriptor_s search, found;                 
#endif
#ifdef __unix__
	CDAindex	x1;
	CDAenvirontext	*path;
	CDAenvirontext	*path_ptr;
	CDAenvirontext	*dir_name;
	CDAenvirontext	*ref_ptr;          
	DIR		*dir_ptr;
#endif

	/* Check for valid widget handle and obtain context pointer. */
                                                                    
	if (*sel_box_widget != 0)               
	{
            csw = csw_get_context(*sel_box_widget, 0);         
	    if (csw->context_check_value != CHECK_VALUE)       
		return DVR_BADPARAM;
	    csw->override_format_list = NULL;
	    csw->initial_format[0] = NULL;

	    /* Free memory allocated to previous format list. */
                                                          
	    for (x = 0; x < csw->format_count; ++x)
	        XtFree((char *)csw->format_list[x]);
	    XtFree((char *)csw->format_list);
	    csw->format_list = NULL;
	    csw->format_limit = csw->format_count = 0;
	}                                           
	else                             
	{                                             
	    /* Allocate new context block. */                 

    	    csw = (CswContextPtr) XtMalloc(sizeof(CswContext));
	    csw->context_check_value = CHECK_VALUE;                
	                                             
	    /* Initialize context block. */
                                                                            
	    csw->context_parent = parent_widget;
	    csw->callback_list = callback_ptr;               
	    csw->option_flags = 0;
	    csw->format_list = NULL;
	    csw->format_limit = 0;
	    csw->format_count = 0;
	    csw->override_format_list = 0;
	    csw->initial_format[0] = NULL;              
	    csw->options_file_spec = NULL;
	    csw->options_file_spec_len = 0;
#ifdef __unix__
	    csw->ultrix_list = NULL;
	    csw->ultrix_limit = 0;
	    csw->ultrix_count = 0;
#endif                                                    
	    /*  Initialize widget handles. */
                                  
	    for (x = 2; x < 8; ++x) csw->widget_table[x] = NULL;
                                         
	    /*  Initialize the DRM. */         
                                                                     
	    MrmInitialize ();		/* Initialize Motif. */
                                                                            
	    DXmInitialize ();		/* Initialize DEC extentions. */

	    /* Define the MRM hierarchy. */  
                                                               
	    status = MrmOpenHierarchy (1,           
	       	    		   file_name_vec,             
				   NULL,
	       		  	   &csw->mrmhierarchy);

    	    if (!(status & 1)) printf("Can't open hierarchy.");
                                                                  
	    /* Register callback routines. */
                                                   
	    MrmRegisterNames (reglist, reglist_num);            

	    /* Call DRM to fetch and create the file selection widget. */

	    if (FILE_SEL_BOX == NULL)                  
    	    {                               
		status = MrmFetchWidget(csw->mrmhierarchy,   
		   	  		"file_sel_box",
	       	    			*parent_widget,
	       	     		     	&FILE_SEL_BOX,
				  	&dummy_class);
                                                              
	       	if (!(status & 1)) printf("Can't fetch interface.");
                                                      
		*sel_box_widget = FILE_SEL_BOX;
        
	       	/* Store context pointer in File Selection widget user data. */

		csw_set_something(FILE_SEL_BOX, XmNuserData, (XtArgVal)csw);

		/* Fetch file format box as child of file selection box.
		 * This fetch must be done separately in order to give
	       	 * its children access to the context pointer just stored
		 * in the User Data resource for the file selection box.
		 */ 
                    
		status = MrmFetchWidget(csw->mrmhierarchy,
		   	  		"file_format_box",
		  		 	FILE_SEL_BOX,
					&file_format_box,
				  	&dummy_class);
                                                              
		if (!(status & 1)) printf("Can't fetch interface.");
                                                                 
		XtManageChild(file_format_box);
		XtUnmanageChild(OPTIONAL_LABEL_BUTTON);
		XtUnmanageChild(OPTIONAL_ICON_BUTTON);
    	    }                                                
	}
                        
	/* Examine contents of item list. */

	while (itm->item_code != 0)
	{
		switch (itm->item_code)
		{                   
		    case DvrOptionFlags:

			csw->option_flags = 
				(CDAflags)itm->CDAitemparam.item_value;
			break;

		    case DvrFileSelectionOverride:
			XtSetValues(FILE_SEL_BOX, 
				    (ArgList) itm->CDAitemparam.item_value,
	 	   		    itm->item_length);
			break;  

		    case DvrFormatSelectionList:
                                                        
			csw->override_format_list = 
				(XmString *)itm->CDAitemparam.item_address;
			for (x = 0; x < itm->item_length; ++x)
			{
			    csw_store_format(csw, 
				*(XmString **)(csw->override_format_list + x));
			}    
			break;
                                        
		    case DvrInitialFormatSelection:
                                                       
			byte_count = itm->item_length;
			csw->initial_format[0] = (XmString *)DXmCvtFCtoCS (
			       		(Opaque)itm->CDAitemparam.item_address,
					(long *) &byte_count, &c_status);
			XmTextSetString(FORMAT_TEXT,
				itm->CDAitemparam.item_address);
			csw_create_new_file_filter(csw,              
				itm->CDAitemparam.item_address);
			break;                                        
	                                                                
		    case DvrOptionalLabelButton:    

			if (optional_button_sw) break;
 			optional_button_sw = 1;               
			XtSetValues(OPTIONAL_LABEL_BUTTON, 
    				    itm->CDAitemparam.item_address,
				    itm->item_length);
			XtManageChild(OPTIONAL_LABEL_BUTTON);
			XtUnmanageChild(OPTIONAL_ICON_BUTTON);
			break;

		    case DvrOptionalIconButton:
                                
			if (optional_button_sw) break;
			optional_button_sw = 1;
			XtSetValues(OPTIONAL_ICON_BUTTON, 
				    itm->CDAitemparam.item_address,
				    itm->item_length);
			XtManageChild(OPTIONAL_ICON_BUTTON);       
			XtUnmanageChild(OPTIONAL_LABEL_BUTTON);
			break;          
		}                   
		++itm;          
	}                          
            
	/* Set label values for input or output. */
                                                               
	if ((csw->option_flags & 3) == DvrMinputFile)             
	{                       
        	XtSetArg(arglist[0], XmNdialogTitle, "csw_input_sel_title");
		MrmFetchSetValues(csw->mrmhierarchy, FILE_SEL_BOX, arglist, 1);
    		XtSetArg(arglist[0], XmNlabelString, "csw_inp_format_label");
		MrmFetchSetValues(csw->mrmhierarchy, FORMAT_LABEL, arglist, 1);
	}                                                             
	else
	{                                                  
	    if ((csw->option_flags & 3) == DvrMoutputFile)
	    {                   
        	XtSetArg(arglist[0], XmNdialogTitle, "csw_output_sel_title");
		MrmFetchSetValues(csw->mrmhierarchy, FILE_SEL_BOX, arglist, 1);
		XtSetArg(arglist[0], XmNlabelString, "csw_out_format_label");
		MrmFetchSetValues(csw->mrmhierarchy, FORMAT_LABEL, arglist, 1);
	    }                                                          
	    else
		return DVR_BADPARAM;
	}  
                               
	/* Check to see if necessary to load format selection list. */
                                        
	if (!csw->override_format_list)                
	{
		csw->format_count = 0;                                      
                                                    
		/* Pre-load list with DDIF and DTIF formats. */          
 
		byte_count = strlen(ddif_format_string);
	    	if (csw->option_flags & DvrMlistDDIFformats)
			csw_store_format(csw, 
			 	(XmString *)DXmCvtFCtoCS(ddif_format_string,
							 (long *) &byte_count, 
							 &c_status));
		if (csw->option_flags & DvrMlistDTIFformats)
			csw_store_format(csw, 
				(XmString *)DXmCvtFCtoCS(dtif_format_string, 
							 (long *) &byte_count, 
							 &c_status));
#ifdef __vms__ 	                                         
		search.dsc$b_class = found.dsc$b_class = DSC$K_CLASS_S;
		search.dsc$b_dtype = found.dsc$b_dtype = DSC$K_DTYPE_T;
                                        
		for (x = 0; x < 3; ++x)       
	     	{                                                    
		    if (((csw->option_flags & DvrMlistCDAformats) && x == 0) ||
	 		((csw->option_flags & DvrMlistDDIFformats) && x == 1) ||
 			((csw->option_flags & DvrMlistDTIFformats) && x == 2))
		    {    
			find_file_context = 0;
		                                              
			if (csw->option_flags & DvrMinputFile)
			{   
	   		    search.dsc$w_length = strlen(input_spec_list[x]); 
			    search.dsc$a_pointer = input_spec_list[x];
	                }                                
		        else                                      
			{                                                   
	     	    	    if (csw->option_flags & DvrMoutputFile)
			    {
		                search.dsc$w_length = strlen(output_spec_list[x]); 
			        search.dsc$a_pointer = output_spec_list[x];
			    }                
			    else                       
	    		 	break;               
			}
		    	found.dsc$w_length = 255;              
			found.dsc$a_pointer = format_string;
		                                            
			do                                     
			{                                  
			    status = LIB$FIND_FILE(&search, 
						   &found, 
		 				   &find_file_context, 
	     	    				   0,0,0,0);
			    if (status & 1 == 1)                   
			    {         
			     	y = z = format_string;         
		                                        
				while (*y && *y++ != ']');
 			 	while (*y && *y++ != '_');               
				while ((*z++ = *y++) != '.');
		    	  	*(--z) = '\0';
			    
				if (format_string[0] != '*')
			    	{
			       	    byte_count = strlen(format_string);
			    	    csw_store_format(csw, 
				   	(XmString *)DXmCvtFCtoCS(format_string, 
						&byte_count, &c_status));
			  	}                         
			    }                                   
	     	    	}
	   		while (status & 1 == 1);              
	                              
			status = LIB$FIND_FILE_END(&find_file_context);
		    }
	     	}           
#endif
                                                               
#ifdef __unix__

		csw->ultrix_count = 0;
                                                 
		/* Search for old style converter files in CDAPATH. */
  		path = getenv("CDAPATH");
                                                        
		if (path)
       		{
		    path_ptr = path;
		    while (get_path_dir(&path_ptr, &dir_name))
		    {                                
			if ((dir_ptr = opendir(dir_name)) != NULL)
			{
			    search_directory(csw, dir_ptr, OLD_STYLE);
			    XtFree(dir_name);                     
			}
		    }                                          
  		}

		/* Search for old style converter files in /usr/bin.
		   Attempt to open ultrix directory.  If unsuccessful, there
		   is nothing else to add to format list. */
                                   
		if ((dir_ptr = opendir(ultrix_directory)) != NULL) 
		    search_directory(csw, dir_ptr, OLD_STYLE);

		/* Search for new style converter files in PATH and CDAPATH. */

		path = getenv("PATH");

		if (path)
       		{                     
		    path_ptr = path;
		    while (get_path_dir(&path_ptr, &dir_name))
		    {
			if ((dir_ptr = opendir(dir_name)) != NULL)
			{
			    search_directory(csw, dir_ptr, NEW_STYLE);
			    XtFree(dir_name);                     
			}                                   
		    }
		}

		path = getenv("CDAPATH");
  		                               
		if (path)                                       
	        {              
		    path_ptr = path;
		    while (get_path_dir(&path_ptr, &dir_name))
		    {
			if ((dir_ptr = opendir(dir_name)) != NULL)  
			{
		     	    search_directory(csw, dir_ptr, NEW_STYLE);
			    XtFree(dir_name);
			}
		    }
		}		
  
		/* Sort ultrix format list to alphabetic sequence. */

		for (x = 0; x < csw->ultrix_count - 1; ++x)      
		{
		    n1 = strlen(csw->ultrix_list[x]);
		    for (x1 = x + 1; x1 < csw->ultrix_count; ++x1)
		    {
			n2 = strlen(csw->ultrix_list[x1]);
			n = n1 <= n2 ? n1 : n2;
			if (strncmp(csw->ultrix_list[x], 
			            csw->ultrix_list[x1],n) > 0)
			{
			    ref_ptr = csw->ultrix_list[x];
			    csw->ultrix_list[x] = csw->ultrix_list[x1];
			    csw->ultrix_list[x1] = ref_ptr;
  		     	}                         
		    }                 
		}                              
                               
		/* Convert format strings to compound strings, skipping 
		   duplicates, and store in format list. */
                                                                
		ref_ptr = "";
		for (x = 0; x < csw->ultrix_count; ++x)
		{                           
		    /* Skip formats cda, ddif, dtif and oldc. */

		    if (!strcmp(csw->ultrix_list[x], "cda") ||
                        !strcmp(csw->ultrix_list[x], "ddif") ||
			!strcmp(csw->ultrix_list[x], "dtif") ||
  	 		!strcmp(csw->ultrix_list[x], "oldc"))
       		    {
			XtFree(csw->ultrix_list[x]);
			continue;
		    }

                    if (strcmp(csw->ultrix_list[x], ref_ptr))              
	  	    {
		        byte_count = strlen(csw->ultrix_list[x]);
			csw_store_format(csw, 
			    (XmString *)DXmCvtFCtoCS(csw->ultrix_list[x], 
					       	     (long *) &byte_count, &c_status));
			XtFree(ref_ptr);                    
			ref_ptr = csw->ultrix_list[x];
		    }
  		    else                
		        XtFree(csw->ultrix_list[x]);       
		}
		if (*ref_ptr != '\0')                            
		    XtFree(ref_ptr);     
		XtFree((char *) csw->ultrix_list);
		csw->ultrix_list = NULL;
		csw->ultrix_limit = csw->ultrix_count = 0;
#endif
        }

	XtSetArg(arglist[0], XmNselectedItems, 0);       
	XtSetArg(arglist[1], XmNselectedItemCount, 0);
	XtSetArg(arglist[2], XmNitems, csw->format_list);
	XtSetArg(arglist[3], XmNitemCount, csw->format_count);
	XtSetValues(LIST_BOX, arglist, 4);     

	/* Check to see if there is an initial default format. */
                                                             
	if (csw->initial_format[0])                                           
	{    
		XtSetArg(arglist[0], XmNselectedItems, csw->initial_format);
		XtSetArg(arglist[1], XmNselectedItemCount, 1);
		XtSetValues(LIST_BOX, arglist, 2);             
	}
                            
	/* Check to see if options file button should be mapped. */ 
                                          
	if (csw->option_flags & DvrMomitOptionsFile)           
	{                 
	    if (OPTIONAL_LABEL_BUTTON)
	        csw_set_something(OPTIONAL_LABEL_BUTTON, XmNtopWidget,
				  (XtArgVal)FORMAT_TEXT);
	    if (OPTIONAL_ICON_BUTTON)
	        csw_set_something(OPTIONAL_ICON_BUTTON, XmNtopWidget,
				  (XtArgVal)FORMAT_TEXT);
	    XtUnmanageChild(OPTIONS_FILE_BUTTON);
	}                                        
	else                                                                
	{        
	    if (OPTIONAL_LABEL_BUTTON)
	        csw_set_something(OPTIONAL_LABEL_BUTTON, XmNtopWidget,
				  (XtArgVal)OPTIONS_FILE_BUTTON);
	    if (OPTIONAL_ICON_BUTTON)
	        csw_set_something(OPTIONAL_ICON_BUTTON, XmNtopWidget,
				  (XtArgVal)OPTIONS_FILE_BUTTON);
	    XtManageChild(OPTIONS_FILE_BUTTON);
	}
      	return DVR_NORMAL;             
}                                                    

#ifdef __unix__                           
 
/*
**  Sub-functions for searching Ultrix directories.
*/
get_path_dir(path_ptr, dir_ptr)                     
CDAenvirontext	**path_ptr;
CDAenvirontext	**dir_ptr;
{
	CDAenvirontext *y;

	/* Check to see if last directory has already been returned.
	   If so, set directory pointer to NULL and return/ */
                                       
	if (**path_ptr == '\0')
      	{
		*dir_ptr = NULL;
		return(FALSE);
	}

	/* Search path string for next occurrence of a colon or NULL char. */

	y = *path_ptr;
	while (*y != ':' && *y != '\0')
		++y;                    

	/* Allocate memory for directory string and copy from path to 
	   new string. */

	*dir_ptr = (CDAenvirontext *)XtMalloc(y - *path_ptr + 1);
	strncpy(*dir_ptr, *path_ptr, y - *path_ptr);

	*(*dir_ptr + (y - *path_ptr)) = '\0';

	/* Move path pointer to next string, if any. */

	if (*y == ':') ++y;
	*path_ptr = y;

	return(TRUE);
	
}

search_directory(csw, dir_ptr, format_style)
CswContextPtr	csw;
DIR		*dir_ptr;                                    
CDAindex	format_style;
{
	CDAindex	x, x1;
	CDAenvirontext	*y, *z;          
	CDAenvirontext	*ref_ptr;                    
 	CDAindex	find_file_context;
	CDAenvirontext	format_string[256];
	CDAindex	old_new_sw;
	CDAindex	valid_sw;            

#ifdef osf1
	struct dirent	*file_ptr;
#else
	struct direct	*file_ptr;
#endif

	CDAenvirontext	*string_ptr;
	
	switch (format_style)
	{
	    case OLD_STYLE:
              
	    /* Search for converter files within each of the desired
	       domains, CDA, DDIF, and DTIF. */
                                                                  
	    for (x = 0; x < 3; ++x)                     
 	    {                                                    
	     	if (((csw->option_flags & DvrMlistCDAformats) && x == 0) ||
	 	    ((csw->option_flags & DvrMlistDDIFformats) && x == 1) ||
 		    ((csw->option_flags & DvrMlistDTIFformats) && x == 2))
		{                                    
		    find_file_context = 0;
		                                                   
		    if (csw->option_flags & DvrMinputFile)
		    {                             
		        /* Set pointer to encoded reference string
      			   from the ultrix file spec input list. */
                                                             
			ref_ptr = ultrix_input_spec_list[x];
	            }                                  
		    else                                      
		    {                                     
	     	    	if (csw->option_flags & DvrMoutputFile)
	     		{    
			    /* Set pointer to encoded reference string
		    	       from the ultrix file spec output list. */
                                              
		 	    ref_ptr = ultrix_output_spec_list[x];
			}                
	   		else            
	    		    break;                               
		    }                           
                                   
                    /* Begin reading contents of directory and look for
		       files which match the encoded reference string. */
                                   
	    	    while (file_ptr = readdir(dir_ptr))   
		    {
		        if (strncmp(file_ptr->d_name, ref_ptr, strlen(ref_ptr)))
				continue;
                                                                              
			/* Converter file found, extract format portion. */
                                     
			y = file_ptr->d_name;
			z = format_string;           
                        valid_sw = TRUE;

 			while (*y && *y++ != '_');               
      			while (*y && *y++ != '_');               
			while (*z = *y++)
			{
			    if (*z++ == '.')
			    {
				valid_sw = FALSE;
				break;
			    }
			}         

			/* Add new format to format list. */
                                                                 
			if (valid_sw)
			{                                
			    string_ptr = (CDAenvirontext *)XtMalloc(
						strlen(format_string) + 1);	
		 	    strcpy(string_ptr, format_string);             
			    csw_store_ultrix_format(csw, string_ptr);
			}
		    }   				          
	            rewinddir(dir_ptr);
		}                                                         
	    }
	    break;                     

	    case NEW_STYLE:

	    while (file_ptr = readdir(dir_ptr))
	    {
	        /* Skip any file names less than 5 characters. */       
                                             
		x1 = strlen(file_ptr->d_name);
	        if (x1 < 5 || x1 > 9)
  		    continue;
                                     
	        if (csw->option_flags & DvrMinputFile)
	        {               
		    /* Set pointer to 4th character from the end of the
		       file name. ( ****ddif or ****dtif    
                                        ^           ^     */
		    y = file_ptr->d_name + x1 - 4;
		    if (((csw->option_flags & DvrMlistDDIFformats) &&
                         strncmp(y, "ddif", 4) == 0) ||
			((csw->option_flags & DvrMlistDTIFformats) &&
                         strncmp(y, "dtif", 4) == 0))
		    {                                              
			/* Move format portion of file name to buffer. */

			y = file_ptr->d_name;
			z = format_string;
	      		valid_sw = TRUE;           
			for (x = 0; x < (x1-4); ++x)
	                {
			    *z++ = *y++;
			}

			*z = '\0';			

	       		/* Check for format in old_new_format_table and 
			   use old name if format is in the table. */

	    	 	old_new_sw = FALSE;                               
			for (x = 0; x < 5; ++x)
	   		{
		     	    if (strcmp(format_string,
	 			    old_new_format_table[x][0]) == 0)
			    {                                     
	      			/* Add format from conversion table to
				   to format list. */
                  
				old_new_sw = TRUE;                 
				string_ptr = (CDAenvirontext *)XtMalloc(
					strlen(old_new_format_table[x][1]) + 1);
	      			strcpy(string_ptr, old_new_format_table[x][1]);	
			        csw_store_ultrix_format(csw, string_ptr);
		    	    	break;
			    }                   
			}

                        if (old_new_sw == FALSE)       
			{
	       		    /* Add format from file spec to format list. */
                            
	  		    string_ptr = (CDAenvirontext *)XtMalloc(x1 - 3);
			    strcpy(string_ptr, format_string);
	      		    *(string_ptr + x1 - 3) = '\0';
			    csw_store_ultrix_format(csw, string_ptr);
			}
		    }
		    continue;                
   	        }                                                   
                                            
	        if (csw->option_flags & DvrMoutputFile)
	        {                             
	    	    if (((csw->option_flags & DvrMlistDDIFformats) && 
		         strncmp(file_ptr->d_name, "ddif", 4) == 0) ||
		        ((csw->option_flags & DvrMlistDTIFformats) &&
		         strncmp(file_ptr->d_name, "dtif", 4) == 0))		
		    {                        
			/* Move format portion of file name to buffer. */

			y = file_ptr->d_name + 4;
			z = format_string;           
			valid_sw = TRUE;
			for (x = 4; x < x1; ++x)
	                {            
			    *z++ = *y++;
			}
			*z = '\0';			

			/* Check for format in old_new_format_table and 
			   use old name if format is in the table. */

			old_new_sw = FALSE;
			for (x = 0; x < 5; ++x)                    
			{
			    if (strcmp(format_string,
	    			old_new_format_table[x][0]) == 0)
			    {
	      			/* Add format from conversion table to
				   to format list. */

	      			old_new_sw = TRUE;
				string_ptr = (CDAenvirontext *)XtMalloc(
					strlen(old_new_format_table[x][1]) + 1);
				strcpy(string_ptr, old_new_format_table[x][1]);	
			        csw_store_ultrix_format(csw, string_ptr);
				break;
			    }
			}

                        if (old_new_sw == FALSE)
			{
			    /* Add format from file spec to format list. */

			    string_ptr = (CDAenvirontext *)XtMalloc(x1 - 3);
		       	    strcpy(string_ptr, format_string);
			    csw_store_ultrix_format(csw, string_ptr);
			}                    
		    }
		}
	    }
	    break;
	}                                                                  
	closedir(dir_ptr);
}

#endif

                  
/*                                      
**++                                    
**  FUNCTIONAL DESCRIPTION:                                  
**
**	Following are several short utility routines.
**
**  FORMAL PARAMETERS:                                   
**
**	Varies by function.
**
**  IMPLICIT INPUTS:                                
**	None.                                      
**             
**  IMPLICIT OUTPUTS:
**	None.
**                                                      
**  FUNCTION VALUE:                     
**	None                                
**                                   
**  SIDE EFFECTS:
**                                                        
**                                                      
**--
**/
    
/* Simplified SET VALUE routine for changing a single widget attribute. */
                                               
void csw_set_something(widget, resource, value)
    Widget	widget;             
    char	*resource;
    XtArgVal	value;
{
    Arg arg_list[1];
                                        
    XtSetArg(arg_list[0], resource, value);
    XtSetValues(widget, arg_list, 1);
}
                                    
                                                                           
/* Simplified GET VALUE routine for retrieving a single widget attribute. */

void csw_get_something(widget, resource, value)
    Widget	widget;                                                       
    char	*resource;
    XtArgVal	value;
{                                             
    Arg arg_list[1];

    XtSetArg(arg_list[0], resource, value);                  
    XtGetValues(widget, arg_list, 1);
}                                                       
                            
/* Routine to store a pointer to a format string into an allocated array. */

void csw_store_format (csw, format_string)
CswContextPtr	csw;
XmString	*format_string;
{
      	CDAindex	new_limit;                       
	XmString	**new_array;

	/* Check to see if existing allocated array (if any) is full. */

	if (csw->format_count >= csw->format_limit)
	{
      		/* Allocate memory for an enlarged array. */

		new_limit = csw->format_limit + FORMAT_ARRAY_INCREMENT;
		new_array = (XmString **)XtMalloc(new_limit * sizeof(XmString *));

		/* Copy contents of existing array (if any) to new array. */

		if (csw->format_count > 0)
		{ 
			memcpy(new_array, csw->format_list, 
				csw->format_limit * sizeof(XmString *));
			XtFree((char *) csw->format_list);
		}
		csw->format_list = new_array;
		csw->format_limit = new_limit;
	}

	/* Store pointer to compound string into the format array. */

	csw->format_list[csw->format_count++] = format_string;
}

#ifdef __unix__
/* Routine to store a pointer to a format string into an allocated array. */
                                                     
void csw_store_ultrix_format (csw, ultrix_string)
CswContextPtr	csw;
CDAenvirontext	*ultrix_string;
{
      	CDAindex	new_limit;                       
	CDAenvirontext	**new_array;

	/* Check to see if existing allocated array (if any) is full. */
      
	if (csw->ultrix_count >= csw->ultrix_limit)
	{
      		/* Allocate memory for an enlarged array. */

		new_limit = csw->ultrix_limit + FORMAT_ARRAY_INCREMENT;
		new_array = (CDAenvirontext **)XtMalloc(
				new_limit * sizeof(CDAenvirontext *));

		/* Copy contents of existing array (if any) to new array. */

		if (csw->ultrix_count > 0)
		{ 
      			memcpy(new_array, csw->ultrix_list, 
	     			csw->ultrix_limit * sizeof(XmString *));
			XtFree((char *) csw->ultrix_list);
		}
		csw->ultrix_list = new_array;
		csw->ultrix_limit = new_limit;
	}

	/* Store pointer to compound string into the format array. */

	csw->ultrix_list[csw->ultrix_count++] = ultrix_string;
}
#endif

/* Get context pointer from parent file selection box user data. */

CswContextPtr csw_get_context(widget,generation)                  
Widget		widget;
CDAindex	generation;
{
	CswContextPtr	context_pointer;
	Widget parent_widget, grandparent_widget, great_grandparent_widget;	

	if (generation < 0 || generation > 3)
		printf("Invalid generation value to get context pointer.");

	if (generation == 0)                                              
	{                                    
		csw_get_something(widget, XmNuserData,
			  		  (XtArgVal) &context_pointer);
	}
	else                    
	{                                                                
	     	parent_widget = XtParent(widget);
		if (generation == 1)      
		{
			csw_get_something(parent_widget, XmNuserData,
			  		  (XtArgVal) &context_pointer);              
		}
		else
		{
		    grandparent_widget = XtParent(parent_widget);
		    if (generation == 2)
		    {
			csw_get_something(grandparent_widget, XmNuserData,
			  		  (XtArgVal) &context_pointer);
		    }
		    else
		    {                                                    
                    	great_grandparent_widget = XtParent(grandparent_widget);
			csw_get_something(great_grandparent_widget,
					  XmNuserData, (XtArgVal) &context_pointer);
		    }
		}                        
	}

 	return (context_pointer);
}

/* Check option flag and create new file filter for selected format. */
                                        
void CDA_APIENTRY csw_create_new_file_filter (csw, format)
CswContextPtr	csw;
CDAenvirontext	format[];
{                                                              
	Widget		file_spec_widget;        
	CDAenvirontext  *file_spec;
	XmStringContext context;        
	CDAenvirontext	*char_set;
	int  		dont_care;                       
	CDAenvirontext	*selected_item_line, *dir_mask, *existing_mask;
	CDAenvirontext	mask_work_area[256];
	CDAenvirontext	*file_type;                         
	CDAindex	byte_count;
	CDAsize		c_status;
	CDAindex	x, dir_sw;
	CDAenvirontext	*y, *z;         
                                         
	if ((csw->option_flags & DvrMnoMaskModification) == 0)
	{

	    /* Because of design defect in the File Selection Widget, it
	       it necessary to capture any existing file specification
	       from the widget and save it until after the file filter
	       has been modified.  The saved file specification must then
	       be restored to the File Selection Widget. */

	    file_spec_widget = (Widget)XmSelectionBoxGetChild(FILE_SEL_BOX, 
						      (CDAoctet)XmDIALOG_TEXT);
	    file_spec = (CDAenvirontext *)XmTextGetString(file_spec_widget);

	    /* Compare format to file-type table to pick up associated
	       default file type for that format.  If format name is not 
	       found in the table, use the format name itself as the
	       file type. */
                                                             
	    for (x = 0, file_type = 0; x < 5; ++x)                 
	    {                         
		    if (!strcmp(format, file_type_table[x][0]))
		    {       
			    file_type = file_type_table[x][1];
			    break;
		    }
	    }         
	    if (file_type == 0) file_type = format;
                                          
    	    csw_get_something(FILE_SEL_BOX, XmNdirMask, (XtArgVal) &dir_mask);
	    if (dir_mask)
	    {
		XmStringInitContext(&context, (XmString) dir_mask); 
		XmStringGetNextSegment(context, &existing_mask, 
				&char_set, 
				(XmStringDirection *) &dont_care, 
				(Boolean *) &dont_care);
		strcpy(mask_work_area, existing_mask);
		XtFree(existing_mask);
		XtFree(char_set);
		XtFree((char *) context);
	    }
	    else
	    {
		strcpy(mask_work_area, "*.*");
	    }         
	    y = mask_work_area + strlen(mask_work_area); 
	    z = file_type;
	    while (y >= mask_work_area)
	    {
		if (*y == ']' || *y == '/')
		    break;
	        if (*y == '.')
	        {                                            
		    ++y;                          
        	    strcpy(y, z);
		    byte_count = strlen(mask_work_area);     
		    csw_set_something(FILE_SEL_BOX, XmNdirMask, 
	    	    	(XtArgVal) DXmCvtFCtoCS(mask_work_area, (long *) &byte_count, 
						 &c_status));
		    break;
	        }
		--y;
	    }         

	    XmTextSetString(file_spec_widget, file_spec);
            XtFree(file_spec);
	}
}

                                          
/*                                                    
**++                                
**  FUNCTIONAL DESCRIPTION:                                  
**                                                            
**	This routine handles all create callbacks from the various
**	widgets.
**
**  FORMAL PARAMETERS:                                         
**                                                  
**	widget  [pointer to a widget structure]
**
**	tag	[an ID value to identify the callback reason and widget]
**
**  	callback_data [a structure containing parameter information]
**
**  IMPLICIT INPUTS:
**	None.                                      
**             
**  IMPLICIT OUTPUTS:
**	None.                                                
**                                      
**  FUNCTION VALUE:
**	None.
**
**  SIDE EFFECTS:                                            
**                                                            
**
**--                  
**/                                   
                                                                    
void csw_create_proc ( widget, tag, callback_data )  
	Widget	widget;          
  	Opaque	tag;                                                      
	XmAnyCallbackStruct *callback_data;
{                                                              
        CswContextPtr	csw; 
        CDAindex	*widget_id = (CDAindex *) tag;		
	                                                 
	if (*widget_id == LIST_BOX_ID)
		csw = csw_get_context(widget, 3);
	else
		csw = csw_get_context(widget, 2);

	csw->widget_table[*widget_id] = widget;
}

                                          
/*
**++
**  FUNCTIONAL DESCRIPTION:                                  
**
**	This routine handles the callback when the options file button has
**	been pressed. 
**
**  FORMAL PARAMETERS:
**                                                        
**	widget  [pointer to a widget structure]
**
**	tag	[an ID value to identify the callback reason and widget]
**
**  	callback_data [a structure containing parameter information]
**              
**  IMPLICIT INPUTS:
**	None.                                      
**             
**  IMPLICIT OUTPUTS:
**	None.
**                      
**  FUNCTION VALUE:
**	None.
**
**  SIDE EFFECTS:                                            
**                                                 
**                              
**--
**/                                                

void csw_options_activate_proc (widget, tag, callback_data)
	Widget	widget;
	Opaque	tag;                             
	XmAnyCallbackStruct *callback_data;
                                                        
{                                                  
        CswContextPtr	csw;
	MrmType 	dummy_class;      
	CDAstatus	status;
	CDAindex	x_coordinate, y_coordinate;
	Arg arglist[4];
	CDAindex	byte_count;
	CDAsize		c_status;
                                        
	csw = csw_get_context(widget, 2);
     	if (OPTIONS_FILE_SEL_BOX == NULL)
	{   
		status = MrmFetchWidget(csw->mrmhierarchy,
					"options_file_sel_box",
					FILE_SEL_BOX,
					&OPTIONS_FILE_SEL_BOX,
					&dummy_class);
                                                              
		if (!status & 1) printf("Can't fetch interface.");

		csw_set_something(OPTIONS_FILE_SEL_BOX, XmNuserData, 
			     		(XtArgVal)csw);

	}   
                                                   
	OPTIONS_FILE_BUTTON = widget;

#ifdef __unix__

	XtSetArg(arglist[0], XmNdirMask, "csw_sel_opt_ultrix_mask");
	MrmFetchSetValues(csw->mrmhierarchy, OPTIONS_FILE_SEL_BOX, arglist, 1);

#endif

     	/* Set options file selection default values. */

	byte_count = 0;
	XtSetArg(arglist[0], XmNx, &x_coordinate);
	XtSetArg(arglist[1], XmNy, &y_coordinate);
	XtGetValues(FILE_SEL_BOX, arglist, 2);
	x_coordinate += 40;
	y_coordinate += 40;
	XtSetArg(arglist[0], XmNx, x_coordinate);
	XtSetArg(arglist[1], XmNy, y_coordinate);
	XtSetArg(arglist[2], XmNvalue, 
		(XmString *)DXmCvtFCtoCS(null_string, (long *) &byte_count, &c_status));
	XtSetValues(OPTIONS_FILE_SEL_BOX, arglist, 3);
	XtManageChild(OPTIONS_FILE_SEL_BOX);
}                                                                   
                                                      
                                                            
/*             
**++
**  FUNCTIONAL DESCRIPTION:                                  
**
**	This routine handles the call back from the OK button (or 
**	equivalent) in the file selection box.  (Double clicking
**	on a file specification or pressing the return key on the
**	file specification line will also generate this callback.)
**
**  FORMAL PARAMETERS:
**
**	widget  [pointer to a widget structure]
**
**	tag	[an ID value to identify the callback reason and widget]
**
**  	callback_data [a structure containing parameter information 
**		       specific to the file selection widget]
**                                           
**  IMPLICIT INPUTS:
**	None.                                      
**             
**  IMPLICIT OUTPUTS:      
**	None.                   
**
**  FUNCTION VALUE:
**	None.  
**                                                                
**  SIDE EFFECTS:                                            
**                                                 
**
**--                                                        
**/                     
                                                              
void csw_selection_proc ( widget, tag, callback_data)
	Widget	widget;
	Opaque	tag;                               
	XmFileSelectionBoxCallbackStruct *callback_data;
{                                                   
        CswContextPtr	csw;
	CDAindex	x;
	CDAenvirontext	*format;                    
	struct DvrConverterSelectionCallback return_data;
	CDAindex	byte_count;
	CDAsize		c_status;
	                                                              
	csw = csw_get_context(widget, 0);

	return_data.reason = callback_data->reason;
	return_data.event  = callback_data->event;;        
	return_data.file_spec = (XmString *)callback_data->value;      

	return_data.file_spec_len = callback_data->length; 
	format = XmTextGetString(FORMAT_TEXT);
	byte_count = return_data.file_spec_len;
	return_data.file_format = (XmString *)DXmCvtFCtoCS(format, (long *) &byte_count,
							   &c_status);
	return_data.file_format_len = strlen(format);
	return_data.options_file_spec = csw->options_file_spec;       
	return_data.options_file_len = csw->options_file_spec_len;;
	XtFree(format);

	x = 0;
	if (csw->callback_list != NULL)
	{               
		while (csw->callback_list[x].callback != 0)
		{                           
			csw->callback_list[x].callback(widget, 
				csw->callback_list[x].closure,
				&return_data);
	  		++x;
		}                                                    
	}
}                                                      
                                                                     
/*                                                           
**++
**  FUNCTIONAL DESCRIPTION:                                  
**
**	This routine handles the callback when the help button has
**	been pressed.                                      
**
**  FORMAL PARAMETERS:
**                                                        
**	widget  [pointer to a widget structure]
**
**	tag	[an ID value to identify the callback reason and widget]
**
**  	callback_data [a structure containing parameter information]
**              
**  IMPLICIT INPUTS:
**	None.                                      
**             
**  IMPLICIT OUTPUTS:
**	None.
**                                                 
**  FUNCTION VALUE:
**	None.
**
**  SIDE EFFECTS:                                            
**                                                 
**                              
**--
**/                             
                                                           
void csw_help_proc (widget, tag, callback_data)
	Widget	widget;
	Opaque	tag;                             
	XmAnyCallbackStruct *callback_data;       
                                                        
{                                                  
	CDAstatus	status;                    
	static Widget	dvr_csw_help_box;
	CswContextPtr 	csw;
	MrmType 	*dummy_class;      
	Arg		arglist[2];               
                                           
#if 0

	if (*tag == HELP_EXIT)
	{
	    if (dvr_csw_help_box != NULL)                            
	    	XtUnmanageChild(dvr_csw_help_box);
	}
	else
	{                
	    csw_get_something(widget, XmNuserData, &csw);
                                                   
	    if (dvr_csw_help_box == NULL)
	    {          
	    	status = MrmFetchWidget(csw->mrmhierarchy,
					"help_box",
					*csw->context_parent,
					&dvr_csw_help_box,
					&dummy_class);
                                                              
		if (!(status & 1)) 
		{
		    printf("Can't fetch interface.");
		    return;                         
		}
	    }   
                                                   
     	    switch (*tag)
            {
	    	case HELP_BUTTON_ID:
			XtSetArg (arglist[0], DXmNhelptitleLabel,
					    "csw_main_help_title");
			XtSetArg (arglist[1], DXmNfirstTopic,
					    "csw_main_help_topic");
     			break;

		case OPTIONS_FILE_SEL_ID:
			XtSetArg (arglist[0], DXmNhelptitleLabel,
					    "csw_options_help_title");
			XtSetArg (arglist[1], DXmNfirstTopic,
					    "csw_options_help_topic");
			break;
            }	    

	    MrmFetchSetValues (csw->mrmhierarchy,dvr_csw_help_box, arglist, 2);
	    XtManageChild(dvr_csw_help_box);
	}

#endif

	return;
}                                                                   
                                                      
                                          
/*
**++                                         
**  FUNCTIONAL DESCRIPTION:                                  
**                                         
**	This routine handles the callbacks from the format list box
**	widget.  A callback is generated by either single or double
**	clicking on a item in the list.
**
**  FORMAL PARAMETERS:
**
**	widget  [pointer to a widget structure]
**
**	tag	[an ID value to identify the callback reason and widget]
**
**  	callback_data [a structure containing parameter information
**		       specific to the list box widget]    
**
**  IMPLICIT INPUTS:                         
**	None.                                      
**                                         
**  IMPLICIT OUTPUTS:
**	None.
**
**  FUNCTION VALUE:
**	None.
**
**  SIDE EFFECTS:                                            
**                                                 
**
**--
**/

void csw_list_sel_proc (widget, tag, callback_data)
	Widget	widget;                      
	Opaque	tag; 
	XmListCallbackStruct *callback_data;     
{                    
        CswContextPtr	csw;
	CDAenvirontext	*char_set;
	CDAindex	index, dont_care;                       
	CDAenvirontext	*selected_item_line;
                                                   
	XmStringContext 	context;        
	                                                        
	csw = csw_get_context(widget, 3);
	index = callback_data->item_position - 1;  

	/* Get selected format from callback structure. */
                                                                      
	XmStringInitContext(&context, (XmString) csw->format_list[index]); 
	XmStringGetNextSegment(context, &selected_item_line, 
				&char_set, (XmStringDirection *) &dont_care, 
				(Boolean *) &dont_care);

	/* Set selected format into simple text widget. */

	csw_set_something(FORMAT_TEXT, XmNvalue, 
				(XtArgVal) selected_item_line);

	csw_create_new_file_filter (csw, selected_item_line);

	XtFree(selected_item_line);
	XtFree(char_set);
	XtFree((char *) context);
}                                                
                                                            
/*                     
**++
**  FUNCTIONAL DESCRIPTION:                                  
**
**	This routine handles the call back from the OK button (or 
**	equivalent) in the options file selection box.  (Double clicking
**	on a file specification or pressing the return key on the
**	file specification line will also generate this callback.)
**                      
**  FORMAL PARAMETERS:
**
**	widget  [pointer to a widget structure]
**
**	tag	[an ID value to identify the callback reason and widget]
**
**  	callback_data [a structure containing parameter information 
**		       specific to the file selection widget]
**                                           
**  IMPLICIT INPUTS:
**	None.                                      
**             
**  IMPLICIT OUTPUTS:             
**	None.                   
**
**  FUNCTION VALUE:     
**	None.
**                                                                
**  SIDE EFFECTS:                                            
**                                                 
**
**--
**/
              
void csw_options_ok_proc ( widget, tag, callback_data)
	Widget	widget;
	Opaque	tag;
	XmFileSelectionBoxCallbackStruct *callback_data;
{                                                   
        CswContextPtr	csw;

	csw = csw_get_context(widget, 2);
	csw->options_file_spec = (XmString *)callback_data->value;
	csw->options_file_spec_len = callback_data->length;

	XtUnmanageChild(OPTIONS_FILE_SEL_BOX);
}                                                      
                                                            
/*                     
**++            
**  FUNCTIONAL DESCRIPTION:                                  
**                                        
**	This routine handles the call back from the CANCEL button (or 
**	equivalent) in the options file selection box.  (Double clicking
**	on a file specification or pressing the return key on the
**	file specification line will also generate this callback.)
**                                                      
**  FORMAL PARAMETERS:                         
**
**	widget  [pointer to a widget structure]
**
**	tag	[an ID value to identify the callback reason and widget]
**
**  	callback_data [a structure containing parameter information 
**		       specific to the file selection widget]
**                                     
**  IMPLICIT INPUTS:
**	None.                                                
**             
**  IMPLICIT OUTPUTS:             
**	None.
**             
**  FUNCTION VALUE:
**	None.                                  
**                                                                
**  SIDE EFFECTS:                                            
**                                                   
**
**--
**/             

void csw_options_cancel_proc ( widget, tag, callback_data)
	Widget	widget;
	Opaque	tag;
	XmFileSelectionBoxCallbackStruct *callback_data;
{                                                   
        CswContextPtr	csw;

	csw = csw_get_context(widget, 2);
	csw_set_something(OPTIONS_FILE_BUTTON, XmNsensitive, TRUE);
	XtUnmanageChild(OPTIONS_FILE_SEL_BOX);
}                                                      

void csw_destroy_proc ( widget, tag, callback_data)
	Widget	widget;
	Opaque	tag;
	XmAnyCallbackStruct *callback_data;
{                                                                        
        CswContextPtr	csw;
	CDAstatus	status;
	CDAindex	x;

	csw = csw_get_context(widget, 0);

	/* Free memory allocated to format list strings. */
                                                          
	for (x = 0; x < csw->format_count; ++x)
	    XtFree((char *) csw->format_list[x]);
	XtFree((char *) csw->format_list);
	csw->format_list = NULL;
	csw->format_limit = csw->format_count = 0;

	XtFree((char *) csw);
}
