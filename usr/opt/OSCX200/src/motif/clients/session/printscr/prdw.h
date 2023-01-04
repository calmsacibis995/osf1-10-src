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
/* BuildSystemHeader added automatically */
/* $Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/prdw.h,v 1.1 90/01/01 00:00:00 devrcs Exp $ */
/*
**++
**  COPYRIGHT (c) 1987 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**  ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**--
**/


/*
**++
**  MODULE NAME:
**	prdw.h
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**	Definitions for PrintScreen DECwindows
**
**  ENVIRONMENT:
**
**	VMS V5, Ultrix V2.2  DW FT1
**
**  AUTHORS:
**      Kathy Robinson, Mark Antonelli
**
**  RELEASER:
**	Kathy Robinson
**
**  CREATION DATE:     April 26, 1987
**
**  MODIFICATION HISTORY:
**
**/


#define NULLWINDOW	-1


/*
 * the default
 */
#define dxPrscDefault		0
#define DECW$C_PRSC_DEFAULT	dxPrscDefault

/*
 * pixel aspect ratio
 */
#define dxPrscPixAsp1		1
#define dxPrscPixAsp2		2
#define DECW$C_PRSC_ASPECT_1	dxPrscPixAsp1
#define DECW$C_PRSC_ASPECT_2	dxPrscPixAsp2

/*
 * printer color
 */
#define dxPrscPrinterBW			1
#define dxPrscPrinterGrey		2
#define dxPrscPrinterColor		3
#define DECW$C_PRSC_PRINTER_BW		dxPrscPrinterBW
#define DECW$C_PRSC_PRINTER_GREY	dxPrscPrinterGrey
#define DECW$C_PRSC_PRINTER_COLOR	dxPrscPrinterColor

/*
 * print queue
 */
#define dxPrscAccumulate	2
#define dxPrscImmediate		1
#define DECW$C_PRSC_IMMEDIATE	dxPrscImmediate
#define DECW$C_PRSC_ACCUMULATE	dxPrscAccumulate

/*
 * reverse image
 */
#define dxPrscPositive		1
#define dxPrscNegative		2
#define DECW$C_PRSC_POSITIVE	dxPrscPositive
#define DECW$C_PRSC_NEGATIVE	dxPrscNegative

/*
 * storage format
 */
#define dxPrscPostscript	1
#define dxPrscSixel		2
#define dxPrscDDIF		3
#define DECW$C_PRSC_POSTSCRIPT	dxPrscPostscript
#define DECW$C_PRSC_SIXEL	dxPrscSixel
#define DECW$C_PRSC_DDIF	dxPrscDDIF

/*
 * form feed info
 */
#define dxPrscForm		1
#define dxPrscNoForm		2
#define DECW$C_PRSC_FORM	dxPrscForm
#define DECW$C_PRSC_NOFORM	dxPrscNoForm

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

#ifndef SS$_NORMAL
#define SS$_NORMAL		1
#endif

#define DECW$_PRSC_INVDEV_ID		dxPrscInvDevId
#define DECW$_PRSC_INVWIN_ID		dxPrscInvWinId
#define DECW$_PRSC_INVASPECT		dxPrscInvAspect
#define DECW$_PRSC_INVPR_COLOR		dxPrscInvPrColor
#define DECW$_PRSC_INVPR_DEST		dxPrscInvPrDest
#define DECW$_PRSC_INVPR_Q		dxPrscInvPrQ
#define DECW$_PRSC_INVREVIMG		dxPrscInvRevImg
#define DECW$_PRSC_INVSTORAGE		dxPrscInvStorage
#define DECW$_PRSC_INVFORM		dxPrscInvForm
#define DECW$_PRSC_INVITEM		dxPrscInvItem
#define DECW$_PRSC_FATERRLIB		dxPrscFatErrLib
#define DECW$_PRSC_NOIMAGE		dxPrscNoImage
#define	DECW$_PRSC_BADFILEIO		dxPrscBadFileIO
#define DECW$_PRSC_INTCHKFAIL		dxPrscIntChkFail
#define DECW$_PRSC_NOMEMORY		dxPrscNoMemory
#define DECW$_PRSC_FUN_ERROR dxPrscFunError
#define DECW$_PRSC_X_ERROR		 dxPrscXError

/*
 * I believe we are replacing the options structure with an array of
 * longwords (one being a pointer to a string or descriptor)
 */

#define DECW$C_PRSC_END_OF_LIST		dxPrscEndOfList
#define DECW$C_PRSC_ASPECT		dxPrscAspect
#define	DECW$C_PRSC_PRINT_COLOR		dxPrscPrintColor
#define DECW$C_PRSC_REVERSE_IMAGE	dxPrscReverseImage
#define DECW$C_PRSC_STORAGE_FORMAT	dxPrscStorageFormat
#define DECW$C_PRSC_FORM_CONTROL	dxPrscFormControl
#define DECW$C_PRSC_PRINT_QUEUE		dxPrscPrintQueue
#define DECW$C_PRSC_PRINT_DEST		dxPrscPrintDest
#define DECW$C_PRSC_OPTIONS_SIZE	dxPrscOptionsSize


#define dxPrscEndOfList		0
#define dxPrscAspect		1
#define	dxPrscPrintColor	2
#define dxPrscReverseImage	3
#define dxPrscStorageFormat	4
#define dxPrscFormControl	5
#define dxPrscPrintQueue	6
#define dxPrscPrintDest		7
#define dxPrscOptionsSize	8

