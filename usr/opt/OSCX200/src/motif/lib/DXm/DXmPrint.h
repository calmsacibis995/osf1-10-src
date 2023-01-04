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
/************************************************************************/
/*									*/
/*	Copyright (c) Digital Equipment Corporation, 1990  		*/
/*	All Rights Reserved.  Unpublished rights reserved		*/
/*	under the copyright laws of the United States.			*/
/*									*/
/*	The software contained on this media is proprietary		*/
/*	to and embodies the confidential technology of 			*/
/*	Digital Equipment Corporation.  Possession, use,		*/
/*	duplication or dissemination of the software and		*/
/*	media is authorized only pursuant to a valid written		*/
/*	license from Digital Equipment Corporation.			*/
/*									*/
/*	RESTRICTED RIGHTS LEGEND   Use, duplication, or 		*/
/*	disclosure by the U.S. Government is subject to			*/
/*	restrictions as set forth in Subparagraph (c)(1)(ii)		*/
/*	of DFARS 252.227-7013, or in FAR 52.227-19, as			*/
/*	applicable.							*/
/*									*/
/************************************************************************/
/************************************************************************/
/*								 	*/
/*   FACILITY:								*/
/*									*/
/*        Print Widget 							*/
/*									*/
/*   ABSTRACT:								*/
/*									*/
/*	This module contains public definitions for the print widget.   */
/*									*/
/*   AUTHORS:								*/
/*									*/
/*	Print Wgt Team	Winter-1990                                     */
/*									*/
/*   CREATION DATE:     Winter-1990					*/
/*									*/
/*   MODIFICATION HISTORY:						*/
/*									*/
/*	018	WDW			11-Apr-1991			*/
/*		Remove obsolete resources.				*/
/*	017	WDW			29-Mar-1991			*/
/*		Work on function prototypes.				*/
/*	016	WDW			14-Mar-1991			*/
/*		Add non-popup version of print widget.			*/
/*	015	WDW			06-Feb-1991			*/
/*		Add "Default" print format.				*/
/*	014	WDW			05-Feb-1991			*/
/*		Fix up problem with message log objects on ULTRIX.	*/
/*	013	WDW			29-Jan-1991			*/
/*		Fix up problem with message log objects on ULTRIX.	*/
/*	012	WDW			11-Jan-1991			*/
/*		Make DXmN{ok|cancel}selectedCallbackList map to		*/
/*		XmN{ok|cancel}Callback for consistency issues.		*/
/*		Also make DXmNunmapOn{Ok|Cancel}Selected be		*/
/*		DXmNunmanageOn{Ok|Cancel} for more consistency.		*/
/*	011	WDW			07-Jan-1991			*/
/*		Add public resource for name of 2ndary box.		*/
/*	010	WDW			19-Nov-1990			*/
/*		More Style Guide Compliancy.				*/
/*	009	WDW			20-Sep-1990			*/
/*		Add augment list capability.				*/
/*	008	WDW			12-Sep-1990			*/
/*		Remove DXmInitializeForMrm, users should be using	*/
/*		DXmInitialize instead.					*/
/*	007	WDW			17-Aug-1990			*/
/*		Add ability to suppress certain options.		*/
/*	006	WDW			13-Jul-1990			*/
/*		I18N work.						*/
/*	005	WDW			25-Jun-1990			*/
/*		Add DXmSUPPRESS_NONE.					*/
/*	004	WDW			21-Jun-1990			*/
/*		Make printer list resources public.			*/
/*	003	WDW			18-Jun-1990			*/
/*		Add "DXm" prefix to all of the resources.		*/
/*	002	WDW			27-Mar-1990			*/
/*		Reorganize.						*/
/*	001	WDW			19-Mar-1990			*/
/*		Add modification history.  Change names of resources to	*/
/* 		match Xm naming conventions.				*/
/*									*/
/************************************************************************/
#ifndef _DXmPrint_h
#define _DXmPrint_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#define		DXmSClassPrintWgt	"DXmPrintWgt"
externalref	WidgetClass		dxmPrintWgtWidgetClass;

/************************************************************************/
/*									*/
/* FORWARD DECLARATIONS							*/
/*									*/
/************************************************************************/
#ifdef _NO_PROTO
extern Widget DXmCreatePrintBox ( );
extern Widget DXmCreatePrintDialog ( );
extern unsigned long int DXmPrintWgtPrintJob ( );
extern unsigned long int DXmPrintWgtAugmentList ( );
#else
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
extern Widget DXmCreatePrintBox ( Widget ar_parent , char *at_name , ArgList ar_args , int l_arg_count );
extern Widget DXmCreatePrintDialog ( Widget ar_parent , char *at_name , ArgList ar_args , int l_arg_count );
extern unsigned long int DXmPrintWgtPrintJob ( Widget ar_w , XmString ar_cs_filenames [], int l_filename_count );
extern unsigned long int DXmPrintWgtAugmentList ( Widget ar_w , int l_list , XtPointer ar_data );
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* _NO_PROTO */

/************************************************************************/
/*									*/
/* Print widget resource names						*/
/*									*/
/************************************************************************/
#define DXmNnumberCopies		"DXmnumberCopies"		/* Number Copies	*/
#define DXmCNumberCopies		"DXmNumberCopies"
#define DXmNpageRangeFrom		"DXmpageRangeFrom"		/* Page Range From	*/
#define DXmCPageRangeFrom		"DXmPageRangeFrom"
#define DXmNpageRangeTo	       		"DXmpageRangeTo"		/* Page Range To	*/
#define DXmCPageRangeTo			"DXmPageRangeTo"
#define DXmNprintFormatList		"DXmprintFormatList"		/* Printer Format List	*/
#define DXmCPrintFormatList		"DXmPrintFormatList"
#define DXmNprintFormatCount		"DXmprintFormatCount"		/* Printer Format Count */
#define DXmCPrintFormatCount		"DXmPrintFormatCount"
#define	DXmNprintFormatChoice		"DXmprintFormatChoice"		/* Print Format Choice	*/
#define	DXmCPrintFormatChoice		"DXmPrintFormatChoice"
#define DXmNorientation			"DXmorientation"		/* Orientation		*/
#define DXmCOrientation			"DXmOrientation"
#define DXmNprinterList			"DXmprinterList"		/* Printer List		*/
#define DXmCPrinterList			"DXmPrinterList"
#define DXmNprinterCount		"DXmprinterCount"		/* Printer Count	*/
#define DXmCPrinterCount		"DXmPrinterCount"
#define	DXmNprinterChoice		"DXmprinterChoice"    		/* Printer Choice    	*/
#define	DXmCPrinterChoice		"DXmPrinterChoice"
#define DXmNprintAfter			"DXmprintAfter"			/* Print After		*/
#define DXmCPrintAfter			"DXmPrintAfter"
#define DXmNdeleteFile			"DXmdeleteFile"			/* Delete File		*/
#define DXmCDeleteFile			"DXmDeleteFile"

#define	DXmNpageSize			"DXmpageSize"			/* Page Size 		*/
#define	DXmCPageSize			"DXmPageSize"
#define	DXmNsides			"DXmsides"			/* Sides 		*/
#define	DXmCSides			"DXmSides"
#define DXmNnumberUp			"DXmnumberUp"			/* Number Up		*/
#define DXmCNumberUp			"DXmNumberUp"
#define DXmNsheetCount			"DXmsheetCount"			/* Sheet Count		*/
#define DXmCSheetCount			"DXmSheetCount"
#define	DXmNfileStartSheet		"DXmfileStartSheet"		/* File Start Sheet 	*/
#define	DXmCFileStartSheet		"DXmFileStartSheet"
#define	DXmNfileEndSheet		"DXmfileEndSheet"		/* File End Sheet 	*/
#define	DXmCFileEndSheet		"DXmFileEndSheet"
#define	DXmNfileBurstSheet		"DXmfileBurstSheet"		/* File Burst Sheet 	*/
#define	DXmCFileBurstSheet		"DXmFileBurstSheet"
#define	DXmNmessageLog			"DXmmessageLog"			/* Message Log 		*/
#define	DXmCMessageLog			"DXmMessageLog"
#define DXmNholdJob			"DXmholdJob"			/* Hold Job		*/
#define DXmCHoldJob			"DXmHoldJob"
#define DXmNnotify			"DXmnotify"			/* Notify		*/
#define DXmCNotify			"DXmNotify"
#define	DXmNsheetSize			"DXmsheetSize"			/* Sheet Size 		*/
#define	DXmCSheetSize			"DXmSheetSize"
#define	DXmNinputTray			"DXminputTray"			/* Input Tray 		*/
#define	DXmCInputTray			"DXmInputTray"
#define	DXmNoutputTray			"DXmoutputTray"			/* Output Tray 		*/
#define	DXmCOutputTray			"DXmOutputTray"
#define DXmNjobName			"DXmjobName"			/* Job Name		*/
#define DXmCJobName			"DXmJobName"
#define DXmNoperatorMessage		"DXmoperatorMessage"		/* Operator Message	*/
#define DXmCOperatorMessage		"DXmOperatorMessage"
#define DXmNheader			"DXmheader"			/* Header		*/
#define DXmCHeader			"DXmHeader"
#define DXmNautoPagination		"DXmautoPagination"		/* Automatic Pagination	*/
#define DXmCAutoPagination		"DXmAutoPagination"
#define DXmNdoubleSpacing		"DXmdoubleSpacing"		/* Double Spacing	*/
#define DXmCDoubleSpacing		"DXmDoubleSpacing"
#define DXmNlayupDefinition		"DXmlayupDefinition"		/* Layup Definition	*/
#define DXmCLayupDefinition		"DXmLayupDefinition"
#define DXmNstartSheetComment		"DXmstartSheetComment"		/* Start Sheet Comment	*/
#define DXmCStartSheetComment		"DXmStartSheetComment"
#define	DXmNpassAll			"DXmpassAll"			/* Pass All 		*/
#define	DXmCPassAll	       		"DXmPassAll"
#define DXmNprinterFormList		"DXmprinterFormList"		/* Printer Form List	*/
#define DXmCPrinterFormList		"DXmPrinterFormList"	
#define DXmNprinterFormCount		"DXmprinterFormCount"		/* Printer Form Count	*/
#define DXmCPrinterFormCount		"DXmPrinterFormCount"
#define	DXmNprinterFormChoice		"DXmprinterFormChoice"    	/* Printer Form Choice  */
#define	DXmCPrinterFormChoice		"DXmPrinterFormChoice"
#define DXmNpriority			"DXmpriority"			/* Priority		*/
#define DXmCPriority			"DXmPriority"
#define DXmNsetup			"DXmsetup"			/* Setup		*/
#define DXmCSetup			"DXmSetup"

#define	DXmNunmanageOnOk		"DXmunmanageOnOk"		/* Unmanage On OK sel. 	*/
#define	DXmCUnmanageOnOk		"DXmUnmanageOnOk"
#define	DXmNunmanageOnCancel		"DXmunmanageOnCancel"		/* Unmanage on Cancel   */
#define	DXmCUnmanageOnCancel		"DXmUnmanageOnCancel"

#define DXmNdefaultPrinter		"DXmdefaultPrinter"		/* Default Printer */
#define DXmCDefaultPrinter		"DXmDefaultPrinter"

#define DXmNfileNameList		"DXmfileNameList"		/* file name count	*/
#define DXmCFileNameList		"DXmFileNameList"
#define DXmNfileNameCount		"DXmfileNameCount"		/* file name count	*/
#define DXmCFileNameCount		"DXmFileNameCount"

#define DXmNsuppressOptionsMask		"DXmsuppressOptionsMask"	/* supress options mask	*/
#define DXmCSuppressOptionsMask		"DXmSuppressOptionsMask"

#define	DXmNoptionsDialogTitle		"DXmoptionsDialogTitle"		/* Title of 2ndary dialog box 	*/
#define	DXmCOptionsDialogTitle		"DXmOptionsDialogTitle"

/************************************************************************/
/*									*/
/* Print Format Codes							*/
/*									*/
/************************************************************************/
#define DXmPRINT_FORMAT_DEFAULT		0
#define DXmPRINT_FORMAT_TEXT		1
#define DXmPRINT_FORMAT_LINE_PRINTER	2
#define DXmPRINT_FORMAT_TERMINAL	3
#define DXmPRINT_FORMAT_ANSI2		4
#define DXmPRINT_FORMAT_ANSI		5
#define DXmPRINT_FORMAT_POSTSCRIPT	6
#define DXmPRINT_FORMAT_REGIS		7
#define DXmPRINT_FORMAT_TEKTRONIX	8
#define DXmPRINT_FORMAT_DDIF		9

/************************************************************************/
/*									*/
/* Page orientation codes						*/
/*									*/
/************************************************************************/
#define DXmORIENTATION_DEFAULT		0
#define DXmORIENTATION_PORTRAIT		1
#define DXmORIENTATION_LANDSCAPE	2

/************************************************************************/
/*									*/
/* Page/Sheet size codes              					*/
/*									*/
/************************************************************************/
#define DXmSIZE_DEFAULT			0
#define	DXmSIZE_LETTER			1
#define	DXmSIZE_LEDGER			2
#define	DXmSIZE_LEGAL			3
#define	DXmSIZE_EXECUTIVE		4
#define	DXmSIZE_A5			5
#define	DXmSIZE_A4			6
#define	DXmSIZE_A3			7
#define	DXmSIZE_B5			8
#define	DXmSIZE_B4			9
#define DXmSIZE_7X9			0  /* unsupported */
#define DXmSIZE_C4_ENVELOPE		10
#define DXmSIZE_C5_ENVELOPE		11
#define DXmSIZE_C56_ENVELOPE		12
#define DXmSIZE_10X13_ENVELOPE		13
#define DXmSIZE_9X12_ENVELOPE		14
#define DXmSIZE_BUSINESS_ENVELOPE	15

/************************************************************************/
/*									*/
/* Sides Codes								*/
/*									*/
/************************************************************************/
#define DXmSIDES_DEFAULT		0
#define DXmSIDES_SIMPLEX_ONE		1
#define DXmSIDES_SIMPLEX_TWO		2
#define DXmSIDES_DUPLEX_ONE		3
#define DXmSIDES_DUPLEX_TWO		4
#define DXmSIDES_TUMBLE_ONE		5
#define DXmSIDES_TUMBLE_TWO		6

/************************************************************************/
/*									*/
/* File Start/End/Burst Sheet Codes					*/
/*									*/
/************************************************************************/
#define DXmFILE_SHEET_DEFAULT		0
#define DXmFILE_SHEET_NONE		1
#define DXmFILE_SHEET_ONE		2
#define DXmFILE_SHEET_ALL		3

/************************************************************************/
/*									*/
/* Message Log Codes							*/
/*									*/
/************************************************************************/
#ifdef VMS
#define DXmMESSAGE_LOG_DEFAULT		0
#define DXmMESSAGE_LOG_IGNORE		1
#define DXmMESSAGE_LOG_PRINT		2
#define DXmMESSAGE_LOG_KEEP		3
#define DXmMESSAGE_LOG_KEEP_AND_PRINT	4
#else
#define DXmMESSAGE_LOG_DEFAULT		0
#define DXmMESSAGE_LOG_IGNORE		1
#define DXmMESSAGE_LOG_KEEP		2
#define DXmMESSAGE_LOG_PRINT		0 /* Not valid on ULTRIX */
#define DXmMESSAGE_LOG_KEEP_AND_PRINT	0 /* Not valid on ULTRIX */
#endif

/************************************************************************/
/*									*/
/* Input Tray Codes							*/
/*									*/
/************************************************************************/
#define DXmINPUT_TRAY_DEFAULT		0
#define DXmINPUT_TRAY_TOP		1
#define DXmINPUT_TRAY_MIDDLE		2
#define DXmINPUT_TRAY_BOTTOM		3

/************************************************************************/
/*									*/
/* Output Tray Codes							*/
/*									*/
/************************************************************************/
#define DXmOUTPUT_TRAY_DEFAULT		0
#define DXmOUTPUT_TRAY_TOP		1
#define DXmOUTPUT_TRAY_SIDE		2
#define DXmOUTPUT_TRAY_FACE_UP		3
#define DXmOUTPUT_TRAY_UPPER	        4
#define DXmOUTPUT_TRAY_LOWER		5
#define DXmOUTPUT_TRAY_LARGE_CAPACITY	6

/************************************************************************/
/*									*/
/* Suppress Option Mask Bits						*/
/*									*/
/************************************************************************/
#define DXmSUPPRESS_NONE			0
#define DXmSUPPRESS_DELETE_FILE			1 << 0
#define DXmSUPPRESS_NUMBER_COPIES		1 << 1
#define DXmSUPPRESS_PAGE_RANGE			1 << 2
#define DXmSUPPRESS_PRINT_FORMAT		1 << 3
#define DXmSUPPRESS_ORIENTATION			1 << 4
#define DXmSUPPRESS_PRINTER			1 << 5
#define DXmSUPPRESS_PRINT_AFTER			1 << 6
#define DXmSUPPRESS_PAGE_SIZE			1 << 7
#define DXmSUPPRESS_SIDES			1 << 8
#define DXmSUPPRESS_NUMBER_UP			1 << 9
#define DXmSUPPRESS_SHEET_COUNT			1 << 10
#define DXmSUPPRESS_FILE_START_SHEET		1 << 11
#define DXmSUPPRESS_FILE_END_SHEET		1 << 12
#define DXmSUPPRESS_FILE_BURST_SHEET		1 << 13
#define DXmSUPPRESS_MESSAGE_LOG			1 << 14
#define DXmSUPPRESS_HOLD_JOB			1 << 15
#define DXmSUPPRESS_NOTIFY			1 << 16
#define DXmSUPPRESS_SHEET_SIZE			1 << 17
#define DXmSUPPRESS_INPUT_TRAY			1 << 18
#define DXmSUPPRESS_OUTPUT_TRAY			1 << 19
#define DXmSUPPRESS_JOB_NAME			1 << 20
#define DXmSUPPRESS_OPERATOR_MESSAGE		1 << 21
#define DXmSUPPRESS_HEADER			1 << 22
#define DXmSUPPRESS_AUTOMATIC_PAGINATION	1 << 23
#define DXmSUPPRESS_DOUBLE_SPACING		1 << 24
#define DXmSUPPRESS_LAYUP_DEFINITION		1 << 25
#define DXmSUPPRESS_START_SHEET_COMMENT		1 << 26
#define DXmSUPPRESS_PASS_ALL			1 << 27
#define DXmSUPPRESS_PRINTER_FORM		1 << 28
#define DXmSUPPRESS_PRIORITY			1 << 29
#define DXmSUPPRESS_SETUP			1 << 30

/************************************************************************/
/*									*/
/* Structures and constants for augmenting the print formats and the	*/
/* option menu lists.							*/
/*									*/
/************************************************************************/
typedef struct _DXmPrintOptionMenuStruct
{
    XmString	ui_string;  	/* What is in interface */
    XmString	os_string;  	/* What goes to operating system */
} DXmPrintOptionMenuStruct;

typedef struct _DXmPrintFormatStruct
{
    XmString	ui_string; 	/* What is in interface	*/
    XmString	os_string;	/* What goes to operating system */
    XmString	var_string;	/* What {VMS logical | ULTRIX environment variable} to use */
} DXmPrintFormatStruct;

#define DXmPAGE_SIZE		0
#define DXmSIDES		1
#define DXmFILE_START_SHEET	2
#define DXmFILE_END_SHEET	3
#define DXmFILE_BURST_SHEET	4
#define DXmMESSAGE_LOG		5
#define DXmSHEET_SIZE		6
#define DXmINPUT_TRAY		7
#define DXmOUTPUT_TRAY		8
#define DXmPRINT_FORMAT		10

/************************************************************************/
/*									*/
/* The following are provided for backwards compatibility only and	*/
/* should *NOT* be used (use the one on the right instead).		*/
/*									*/
/************************************************************************/
#define	DXmNautoPaginationChoice	DXmNautoPagination	/* Backward compat. only */
#define	DXmNdoubleSpaceChoice		DXmNdoubleSpacing	/* Backward compat. only */
#define	DXmNheaderChoice		DXmNheader		/* Backward compat. only */
#define	DXmNnotifyChoice		DXmNnotify		/* Backward compat. only */
#define	DXmNdeleteFileChoice		DXmNdeleteFile		/* Backward compat. only */
#define DXmNpassallChoice		DXmNpassAll		/* Backward compat. only */
#define DXmNjobNameChoice		DXmNjobName		/* Backward compat. only */
#define DXmNorientationChoice		DXmNorientation		/* Backward compat. only */
#define DXmNpageSizeChoice		DXmNpageSize		/* Backward compat. only */
#define DXmNsheetSizeChoice		DXmNsheetSize		/* Backward compat. only */
#define DXmNsheetLimitLowChoice		DXmNpageRangeFrom      	/* Backward compat. only */
#define DXmNsheetLimitHighChoice	DXmNpageRangeTo		/* Backward compat. only */
#define	DXmNnumberCopiesChoice		DXmNnumberCopies	/* Backward compat. only */
#define DXmPRINT_FORMAT_NONE		DXmPRINT_FORMAT_DEFAULT	/* Backward compat. only */

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _DXmPrint_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
