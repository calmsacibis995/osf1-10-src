/*
**++
**  COPYRIGHT (c) 1987, 1988, 1991, 1992 BY
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
**	11-Aug-1992	Edward P Luwish
**		Add constants for capture_type - so that client message no
**		longer assumes that sizeof(int) == sizeof(void *).
**
**	21-Feb-1992	Edward P Luwish
**		Add support for command line entry, debug tools, batch render
**
**	12-Apr-1991	Edward P Luwish
**		Added many new constants relating to Motif UI
**
**	20-Dec-1993	Dhiren M Patel
**		Removed the definition of dxPrscForm as it is obsolete.
**
**/


#define NULLWINDOW	-1

/*
 * the default 
 */
#define dxPrscDefault		0

/*
 * pixel aspect ratio
 */
#define dxPrscPixAsp1		1
#define dxPrscPixAsp2		2

/*
 * printer color
 */
#define dxPrscPrinterBW		1
#define dxPrscPrinterGrey	2
#define dxPrscPrinterColor	3

/*
 * print widget should or should not be managed
 */
#define dxPrscPrintWidget	2
#define dxPrscQuickPrint	1

/*
 * reverse image
 */
#define dxPrscPositive		1
#define dxPrscNegative		2

/*
 * storage format
 */
#define dxPrscPostscript	1
#define dxPrscSixel		2
#define dxPrscDDIF		3

/*
 * form feed info
 */
#define dxPrscNoForm		2

/*
 * fit options
 */
#define dxPrscScale		1		/* grow or shrink to fit */
#define dxPrscReduce		2		/* reduce only */
#define dxPrscCrop		3		/* crop excess */
#define dxPrscGrow		4		/* grow 2x */
#define dxPrscShrink		5		/* shrink 2x */

/*
 * orientation
 */
#define dxPrscPortrait		1
#define dxPrscLandscape		2
#define dxPrscBestFit		3

/*
** Capture method
*/
#define dxPrscEntire		1
#define dxPrscPartial		2

/*
** Destination
*/
#define dxPrscPrint		1
#define dxPrscFile		2
#define dxPrscBoth		3

/*
 * page sizes
 *
	Use following values from dxmprint.h

	DXmSIZE_LETTER
	DXmSIZE_LEDGER
	DXmSIZE_LEGAL
	DXmSIZE_EXECUTIVE
	DXmSIZE_A5
	DXmSIZE_A4
	DXmSIZE_A3
	DXmSIZE_B5
	DXmSIZE_B4

 *
 */

/*
 * sixel printers
 *
	Use following values from ids$image.h:

	IDS$C_TMPLT_VT240
	IDS$C_TMPLT_LA50
	IDS$C_TMPLT_LA75
	IDS$C_TMPLT_LA100
	IDS$C_TMPLT_LA210
	IDS$C_TMPLT_LN03S
	IDS$C_TMPLT_LJ250
	IDS$C_TMPLT_LJ250_LR
	IDS$C_TMPLT_LCG01

 *
 */

/*
  partial capture
*/

#define dxPrscFull		1
#define dxPrscPartial		2

/*
  command mode
*/

#define dxPrscXmode		1
#define dxPrscShellmode		2

/*
  run mode
*/

#define dxPrscNormalCapture	0
#define dxPrscXimageOnly	1
#define dxPrscRenderOnly	2
#define dxPrscFastCapture	4
#define dxPrscFromCallback	8

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
#define dxPrscInvFit		34	/* invalid fit-to-paper option	*/
#define dxPrscInvOrient		36	/* invalid print orientation	*/
#define dxPrscInvPageSize	38	/* invalid page size		*/
#define dxPrscInvSixelDev	40	/* invalid sixel device		*/
#define dxPrscInvDelay		42	/* invalid delay time		*/

#ifndef SS$_NORMAL
#define SS$_NORMAL		1
#endif

#define dxPrscEndOfList		0	
#define dxPrscAspect		1
#define	dxPrscPrintColor	2
#define dxPrscReverseImage	3
#define dxPrscStorageFormat	4
#define dxPrscFormControl	5
#define dxPrscPrintQueue	6
#define dxPrscPrintDest		7
#define dxPrscOptionsSize	8
#define dxPrscFitOptions	9
#define dxPrscPrintOrient	10
#define dxPrscPageSize		11
#define dxPrscSixelDevice	12
#define dxPrscDelayTime		13
#define dxPrscCapture		14
#define dxPrscXCoord		15
#define dxPrscYCoord		16
#define dxPrscHCoord		17
#define dxPrscWCoord		18
#define dxPrscDelay		19

/*
 * Constants for capture_type (see MESSAGES.C and PRINTCB.C)
 *
 */

#define PRINT_ES		1
#define PRINT_POS		2
#define CAPTURE_ES		3
#define CAPTURE_POS		4

/*
**
** From here on down are 
**
*/
