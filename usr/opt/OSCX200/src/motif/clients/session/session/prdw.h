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
#define NULLWINDOW	-1


/*
 * the default 
 */
#define dxPrscDefault		0
#define DECWC_PRSC_DEFAULT	dxPrscDefault

/*
 * pixel aspect ratio
 */
#define dxPrscPixAsp1		1
#define dxPrscPixAsp2		2
#define DECWC_PRSC_ASPECT_1	dxPrscPixAsp1
#define DECWC_PRSC_ASPECT_2	dxPrscPixAsp2

/*
 * printer color
 */
#define dxPrscPrinterBW			1
#define dxPrscPrinterGrey		2
#define dxPrscPrinterColor		3
#define DECWC_PRSC_PRINTER_BW		dxPrscPrinterBW
#define DECWC_PRSC_PRINTER_GREY	dxPrscPrinterGrey
#define DECWC_PRSC_PRINTER_COLOR	dxPrscPrinterColor

/*
 * print queue
 */
#define dxPrscAccumulate	2
#define dxPrscImmediate		1
#define DECWC_PRSC_IMMEDIATE	dxPrscImmediate
#define DECWC_PRSC_ACCUMULATE	dxPrscAccumulate

/*
 * reverse image
 */
#define dxPrscPositive		1
#define dxPrscNegative		2
#define DECWC_PRSC_POSITIVE	dxPrscPositive
#define DECWC_PRSC_NEGATIVE	dxPrscNegative

/*
 * storage format
 */
#define dxPrscPostscript	1
#define dxPrscSixel		2
#define dxPrscDDIF		3
#define DECWC_PRSC_POSTSCRIPT	dxPrscPostscript
#define DECWC_PRSC_SIXEL	dxPrscSixel
#define DECWC_PRSC_DDIF	dxPrscDDIF

/*
 * form feed info
 */
#define dxPrscForm		1
#define dxPrscNoForm		2
#define DECWC_PRSC_FORM	dxPrscForm
#define DECWC_PRSC_NOFORM	dxPrscNoForm

/*
 * return codes
 */
#define Normal			1	/* normal completion 		*/
#define dxPrscInvDevId		2	/* invalid device id		*/
#define dxPrscInvWinId		4	/* invalid window id		*/
#define dxPrscInvAspect		6	/* invalid aspect ratio		*/
#define dxPrscInvPrColor	8	/* invalid printer color use	*/
#define dxPrscInvPrDest		10	/* invalid print destination	*/
#define dxPrscInvPrQ		12	/* invalid queue management	*/
#define dxPrscInvRevImg		14	/* invalid reverse image	*/
#define dxPrscInvStorage	16	/* invalid storage format	*/
#define dxPrscInvForm		18	/* invalid form control 	*/
#define dxPrscInvItem		20	/* invalid item code	 	*/
#define dxPrscFatErrLib		22	/* fatal internal library error	*/
#define dxPrscNoImage		24	/* cannot get an X image	*/
#define	dxPrscBadFileIO		26	/* cannot open or read a file	*/
#define dxPrscIntChkFail	28	/* internal consist check error */
#define dxPrscNoMemory		30	/* cannot get memory		*/
#define dxPrscFunError		31	/* functionality error(limited) */
#define dxPrscXError		32	/* internal X error		*/

#ifndef SS_NORMAL
#define SS_NORMAL		1
#endif

#define DECW_PRSC_INVDEV_ID		dxPrscInvDevId
#define DECW_PRSC_INVWIN_ID		dxPrscInvWinId
#define DECW_PRSC_INVASPECT		dxPrscInvAspect
#define DECW_PRSC_INVPR_COLOR		dxPrscInvPrColor
#define DECW_PRSC_INVPR_DEST		dxPrscInvPrDest
#define DECW_PRSC_INVPR_Q		dxPrscInvPrQ
#define DECW_PRSC_INVREVIMG		dxPrscInvRevImg
#define DECW_PRSC_INVSTORAGE		dxPrscInvStorage
#define DECW_PRSC_INVFORM		dxPrscInvForm
#define DECW_PRSC_INVITEM		dxPrscInvItem
#define DECW_PRSC_FATERRLIB		dxPrscFatErrLib
#define DECW_PRSC_NOIMAGE		dxPrscNoImage
#define	DECW_PRSC_BADFILEIO		dxPrscBadFileIO
#define DECW_PRSC_INTCHKFAIL		dxPrscIntChkFail
#define DECW_PRSC_NOMEMORY		dxPrscNoMemory
#define DECW_PRSC_FUN_ERROR dxPrscFunError	
#define DECW_PRSC_X_ERROR		 dxPrscXError

/*
 * I believe we are replacing the options structure with an array of
 * longwords (one being a pointer to a string or descriptor)
 */

#define DECWC_PRSC_END_OF_LIST		dxPrscEndOfList	
#define DECWC_PRSC_ASPECT		dxPrscAspect	
#define	DECWC_PRSC_PRINT_COLOR		dxPrscPrintColor
#define DECWC_PRSC_REVERSE_IMAGE	dxPrscReverseImage
#define DECWC_PRSC_STORAGE_FORMAT	dxPrscStorageFormat
#define DECWC_PRSC_FORM_CONTROL	dxPrscFormControl	
#define DECWC_PRSC_PRINT_QUEUE		dxPrscPrintQueue
#define DECWC_PRSC_PRINT_DEST		dxPrscPrintDest	
#define DECWC_PRSC_OPTIONS_SIZE	dxPrscOptionsSize


#define dxPrscEndOfList		0	
#define dxPrscAspect		1
#define	dxPrscPrintColor	2
#define dxPrscReverseImage	3
#define dxPrscStorageFormat	4
#define dxPrscFormControl	5
#define dxPrscPrintQueue	6
#define dxPrscPrintDest		7
#define dxPrscOptionsSize	8

