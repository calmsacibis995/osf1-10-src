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
static char *BuildSystemHeader = "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/parse.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";
#endif		/* BuildSystemHeader */
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
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
**	parse.c
**
**  FACILITY:
**	Printscreen
**
**  ABSTRACT:
**
**	parse input parameters
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
**	April 26, 1987
**
**  MODIFICATION HISTORY:
**
**
**--
**/
/*
 *
 *	parse(argc, argv, command)
 *		argc	- (RO) (int) guess, only Ultrix uses
 *		argv	- (RO) (**char) guess again, only Ultrix uses
 *		command	- (WO) (*Command) command structure
 *
 *	requires:
 * 		none
 *	returns:
 *		nothing for now
 *	notes:
 *		VMS part is braindamaged
 *		When I have lots of time, I'll write a generic parse procedure.
 *
 */

#include "iprdw.h"

#ifdef VMS
#include "desc.h"
#include <climsgdef>
#endif

parse( argc, argv, command )
int	argc;
char	**argv;
Command *command;
#ifndef VMS
{
	for( argc-- ; argc >= 0; argc-- )
	{
		if( argv[argc][0] == '-' ) switch( argv[argc][1] )
		{
			/*
			 * aspect ratio is 2:1
			 */
			case 'a':
				command->options.aspect = DECW$C_PRSC_ASPECT_2;
				break;
			/*
			 * hold queue requests
			 */
			case 'w':
				command->options.print_queue = DECW$C_PRSC_ACCUMULATE;
				break;
			/*
			 * format is ddif
			 */
			case 'd':
				command->options.storage_format = DECW$C_PRSC_DDIF;
				break;
			/*
			 * format is sixel
			 */
			case 's':
				command->options.storage_format = DECW$C_PRSC_SIXEL;
				break;
			/*
			 * reverse image
			 */
			case 'r':
				command->options.reverse_image = DECW$C_PRSC_NEGATIVE;
				break;
			/*
			 * form feed
			 */
			case 'f':
				command->options.form_feed =  DECW$C_PRSC_FORM;
				break;
			default:
				break;
		}
		else if ( argv[argc][1] == '=' ) switch( argv[argc][0] )
		{
			/*
			 * output file name
			 */
			case 'o':
				strcpy( command->options.print_dest,
					&argv[argc][2] );
				break;
			default:
				break;

		}
		else if ( strchr( argv[argc], ':' ) != NULL )
		{
			strcpy( command->DisplayName, argv[argc] );
		}
	}
}
#else ifdef VMS
{
	/*
	 * descriptors for command line qualifiers
	 */
	$DESCRIPTOR(ForD, "form");
	$DESCRIPTOR(AccD, "accumulate");
	$DESCRIPTOR(RevD, "reverse_image");
	$DESCRIPTOR(AspD1, "aspect_ratio.1to1");
	$DESCRIPTOR(AspD2, "aspect_ratio.2to1");
	$DESCRIPTOR(PriBW, "printer_color.bw");
	$DESCRIPTOR(PriGr, "printer_color.grey");
	$DESCRIPTOR(PriCo, "printer_color.color");
	$DESCRIPTOR(StoDS, "storage_format.sixel");
	$DESCRIPTOR(StoDD, "storage_format.ddif");
	$DESCRIPTOR(StoDP, "storage_format.postscript");
	$DESCRIPTOR(OutD, "P1");
	$DESCRIPTOR(DisD, "P2");
	DCLDESC(RetD);

	char	retinfo[1024];	/* buffer to hold return info		*/
	short	retlen;		/* the length of the return descriptor	*/
	long	status;		/* return status			*/

	INITDESC_W(RetD, retinfo);

	/*
	 * Parse the command line the VMS way (which sucks just like the
	 * Un*x way)
	 */
	if( (status = CLI$PRESENT(&AccD)) == CLI$_PRESENT )
		command->options.print_queue = DECW$C_PRSC_ACCUMULATE;

	if( (status = CLI$PRESENT(&RevD)) == CLI$_PRESENT )
		command->options.reverse_image = DECW$C_PRSC_NEGATIVE;

	if( (status = CLI$PRESENT(&ForD)) == CLI$_PRESENT )
		command->options.form_feed =  DECW$C_PRSC_FORM;

	if( (status = CLI$PRESENT(&AspD1)) == CLI$_PRESENT )
		command->options.aspect =  DECW$C_PRSC_ASPECT_1;

	if( (status = CLI$PRESENT(&AspD2)) == CLI$_PRESENT )
		command->options.aspect =  DECW$C_PRSC_ASPECT_2;

	if( (status = CLI$PRESENT(&StoDP)) == CLI$_PRESENT )
		command->options.storage_format =  DECW$C_PRSC_POSTSCRIPT;

	if( (status = CLI$PRESENT(&StoDS)) == CLI$_PRESENT )
		command->options.storage_format =  DECW$C_PRSC_SIXEL;

	if( (status = CLI$PRESENT(&StoDD)) == CLI$_PRESENT )
		command->options.storage_format =  DECW$C_PRSC_DDIF;


	if( (status = CLI$PRESENT(&PriBW)) == CLI$_PRESENT )
		command->options.print_color =  DECW$C_PRSC_PRINTER_BW;
	if( (status = CLI$PRESENT(&PriGr)) == CLI$_PRESENT )
		command->options.print_color =  DECW$C_PRSC_PRINTER_GREY;
	if( (status = CLI$PRESENT(&PriCo)) == CLI$_PRESENT )
		command->options.print_color =  DECW$C_PRSC_PRINTER_COLOR;



	if( (status = CLI$GET_VALUE(&OutD,&RetD ,&retlen)) == SS$_NORMAL )
	{
		command->options.print_dest = malloc(retlen+1);
		strncpy(command->options.print_dest,retinfo, retlen);
		command->options.print_dest[retlen] = '\0';
	}


	if( (status = CLI$GET_VALUE(&DisD,&RetD ,&retlen)) == SS$_NORMAL )
	{
		strncpy( command->DisplayName, retinfo, retlen);
		command->DisplayName[retlen] = '\0';
	}
}
#endif

