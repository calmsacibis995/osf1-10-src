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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/valopt.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988, 1989, 1991 BY                *
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
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**
**  MODULE:
**
**	valopt.c
**
**  FACILITY:
**	Printscreen
**
**  ABSTRACT:
**
**	Validates DECwindows options, setting defaults if necessary.
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
**  CREATION DATE:
**	May	14,	1987, MFA
**
**  MODIFICATION HISTORY:
**
**	10-MAR-1991 Edward P Luwish
**		Added innerValopt interface which permits the use of
**		global string smdata.print_destination for filename
**		rather than allocating buffer.  This behavior is selected
**		by a third parameter and is useful for passing options
**		to forked process.  Old interface to valopt is preserved.
**	June	10,	1987, MFA
**		some options come, some options go
**
**	June 26, 1989,		B. Bazemore
**		Init filenm immediately after allocation.  Fixes
**		File I/O error when Print_Dest option not specified.
**
**--
**/
/*
 *
 *	valopt( optionslist,  options )
 *		optionslist - (RO) (*dxPrscOptions) options
 *		options - (WO) (*dxPrscOptions) options
 *      Goes from a list of items codes and valuse to an options structure
 *
 *	requires:
 * 		none
 *	returns:
 *		status code (in prdw.h)
 *	notes:
 *		none
 *
 *
 *	convertoptions( options )
 *		options - (RW) (*dxPrscOptions) options
 *      Goes to a list of items codes and valuse from an options structure
 *
 *	requires:
 * 		none
 *	returns:
 *		A list of item codes and valuse
 *	notes:
 *		remember to return alloced options list
 *
 */

#include "smdata.h"
#include "iprdw.h"

long innerValopt( optionslist, options, ignoreFlag )
long		*optionslist;
dxPrscOptions	*options;
int		ignoreFlag;	/* If 1, use global filename */

{
long	status;		/* return status code 			*/
long	*ptr;		/* pointer into item list 		*/
char    *filenm;

options->aspect = dxPrscPixAsp1;
options->print_color = dxPrscPrinterBW;
options->print_queue = dxPrscImmediate;
options->reverse_image = dxPrscPositive;
options->storage_format = dxPrscPostscript;
options->form_feed = dxPrscForm;

/* Allocate and initialize the destination filename	*/
if (ignoreFlag == 0)
	{
	filenm = malloc(1024);
	if (!filenm)
		return(status = dxPrscNoMemory);
	bcopy (DEFAULT_FILENAME, filenm, strlen(DEFAULT_FILENAME)+1 );
	options->print_dest = filenm;
	}
else
	{
	options->print_dest = smdata.print_destination;
	}

/* Now process all specified options	*/

ptr = optionslist;
while (*ptr != dxPrscEndOfList)
	{
	switch (*ptr)
		{
		case dxPrscAspect:
			ptr++;
			switch( *ptr )
				{
				case dxPrscPixAsp1:
				case dxPrscPixAsp2:
					options->aspect = *ptr;
				case dxPrscDefault:
					ptr++;
					break;
				default:
			       		return(status = dxPrscInvAspect);
					break;
				}
			break;
		case dxPrscPrintColor:
			ptr++;
			switch( *ptr )
				{
				case dxPrscPrinterBW:
				case dxPrscPrinterGrey:
				case dxPrscPrinterColor:
					options->print_color = *ptr;
				case dxPrscDefault:
					ptr++;
					break;
				default:
					return(status = dxPrscInvPrColor);
					break;
				}
			break;
		case dxPrscPrintQueue:
			ptr++;
			switch( *ptr )
				{
				case dxPrscImmediate:
				case dxPrscAccumulate:
					options->print_queue = *ptr;
				case dxPrscDefault:
					ptr++;
				break;
				default:
					return(status = dxPrscInvPrQ);
					break;
				}
			break;
		case dxPrscReverseImage:
			ptr++;
			switch( *ptr )
				{
				case dxPrscPositive:
				case dxPrscNegative:
					options->reverse_image = *ptr;
				case dxPrscDefault:
					ptr++;
					break;
				default:
					return(status = dxPrscInvRevImg);
					break;
				}
			break;
		case dxPrscStorageFormat:
			ptr++;
			switch( *ptr )
				{
				case dxPrscPostscript:
				case dxPrscSixel:
				case dxPrscDDIF:
					options->storage_format = *ptr;
				case dxPrscDefault:
					ptr++;
					break;
				default:
					return(status = dxPrscInvStorage);
					break;
				}
			break;

		case dxPrscFormControl:
			ptr++;
			switch( *ptr )
				{
				case dxPrscForm:
				case dxPrscNoForm:
					options->form_feed = *ptr;
				case dxPrscDefault:
					ptr++;
					break;
				default:
					return(status = dxPrscInvForm);
					break;
				}
			break;

		case dxPrscPrintDest:
			ptr++;
			if (*ptr != NULL) {   /* double de-reference ptr! */
/*
 				bcopy (*ptr, filenm, strlen(*ptr) );
				filenm[strlen(*ptr)] = 0;
*/
				if (ignoreFlag == 0)
				    {
				    bcopy (smdata.print_destination, filenm,
					strlen(smdata.print_destination));
				    filenm[strlen(smdata.print_destination)]=0;
				    }
				}
			ptr++;
			break;

		default:
			if (*ptr != dxPrscEndOfList)
				return (dxPrscInvItem);
		} /* switch */
	} /* while */


	/*
	 * everything is just perfect (within this subroutine)
	 */
	return(status = Normal);
}



/* The convertoptions routine mashes the user specified item list
 * into a fixed item list format.
 */
long
*convertoptions( options )
dxPrscOptions	*options;


{
long	status;		/* return status code 			*/
long	*ptr;		/* pointer into new item list 		*/
long	*start;		/* top of item list 		*/

/* Allocate the new fixed item list and keep a pointer to the top */

start= ptr = (long *)malloc(20*sizeof(long));

/* Start mashing    */
*ptr = dxPrscAspect;
ptr++;
if (options->aspect  == dxPrscPixAsp1 || options->aspect  == dxPrscPixAsp2)
	*ptr = options->aspect;
else
	*ptr= dxPrscDefault;
ptr++;

*ptr = dxPrscPrintColor;
ptr++;
if (options->print_color == dxPrscPrinterBW ||
		options->print_color  == dxPrscPrinterGrey ||
		options->print_color  == dxPrscPrinterColor)
	*ptr = options->print_color;
else
	*ptr= dxPrscDefault;
ptr++;

*ptr = dxPrscReverseImage;
ptr++;
if ( options->reverse_image  == dxPrscPositive ||
		options->reverse_image  == dxPrscNegative)
	*ptr = options->reverse_image;
else
	*ptr= dxPrscDefault;
ptr++;

*ptr = dxPrscFormControl;
ptr++;
if ( options->form_feed  == dxPrscNoForm || options->form_feed  == dxPrscForm)
	*ptr = options->form_feed;
else
	*ptr= dxPrscDefault;
ptr++;

*ptr = dxPrscPrintQueue;
ptr++;
if ( options->print_queue  == dxPrscAccumulate ||
		options->print_queue == dxPrscImmediate)
	*ptr = options->print_queue;
else
	*ptr= dxPrscDefault;
ptr++;

*ptr = dxPrscStorageFormat;
ptr++;
if ( options->storage_format  == dxPrscPostscript ||
		options->storage_format  == dxPrscDDIF ||
		options->storage_format  == dxPrscSixel)
	*ptr = options->storage_format;
else
	*ptr= dxPrscDefault;
ptr++;

*ptr = dxPrscPrintDest;
ptr++;
*ptr = (long)options->print_dest;
ptr++;

*ptr = dxPrscEndOfList;
ptr++;
*ptr = 10;

return(start);
}

long valopt( optionslist, options )
long		*optionslist;
dxPrscOptions	*options;
{
long	status;
status = innerValopt( optionslist, options, 0);
}
