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
**	getfile.c
**
**  FACILITY:
**	Printscreen
**
**  ABSTRACT:
**
**	return a pointer to an output file or device
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
**	April 	15, 	1987
**
**  MODIFICATION HISTORY:
**
**	May	26,	1987, MFA
**		VMS stuff added
**
**--
**/
/*
 *
 *	getfile( options, openfileflag )
 *		options - (RO) (*dxPrscOptions) options, print_dest is used
 *		OpernFileFlag - (RO) If false, file isn't really opened
 *
 *	requires:
 * 		none
 *	returns:
 *		pointer to file or NULL on error
 *	notes:
 *		VMS stuff needs to do a F_$_TRN_LNM_$
 *
 */

#include <stdio.h>
#include "iprdw.h"

#ifdef VMS
#include <desc.h>
#include <ssdef.h>
#include <fscndef.h>

typedef struct
{
	short	len;
	short	code;
	long	addr;
} item_d;

typedef struct
{
	item_d	file;
	item_d	node;
	item_d	devi;
	item_d	root;
	item_d	dire;
	item_d	name;
	item_d	type;
	item_d	vers;
	long	zero;
} scan_d;
#endif

FILE	*getfile( options)
dxPrscOptions	*options;
#ifndef VMS
{
	FILE	*file;		/* pointer to the opened file(or NULL) 	*/

	/*
	 * if a file is specified, we write to that file, otherwise
	 * to the standard output it goes
	 */
	if( options->print_dest == NULL )
	{
		file = stdout;
	}
	else
	{
	    if ((char *)options->print_dest[0] == "|") 
      		file =  popen( &options->print_dest[1], "w" );
	    else
		file = fopen( options->print_dest, "w" );
	}
	
/* 
 * If DDIF, close the file, but we're not going to bother to delete the file
 * Ultirix has no version numbers, the second open will overwrite the first.
 */
    	if ( (options->storage_format == dxPrscDDIF) && (file != 0) )
    		{
    		fclose( file);
    		}
	return( file);


}
#else ifdef VMS
{
	FILE	*file;		/* pointer to the opened file(or NULL) 	*/
	char	ftype[5];	/* three letter file type		*/
	char	fname[32];	/* default file name			*/
	char	*fspec;		/* the whole filespec			*/
	char	lognam_b[1024];	/* buffer for translated log name	*/
	char	srcstr_b[1024];	/* buffer for input source string	*/
	int	speclen;	/* length of whole filespec		*/
	int	count;		/* bytes used in whole filespec		*/
	long	status;		/* return status code			*/
	scan_d	scanlist;/* = *//* filescan item list			*/
/*	{
		0, FSCN$_FILESPEC, 0,
		0, FSCN$_NODE, 0,
		0, FSCN$_DEVICE, 0,
		0, FSCN$_ROOT, 0,
		0, FSCN$_DIRECTORY, 0,
		0, FSCN$_NAME, 0,
		0, FSCN$_TYPE, 0,
		0, FSCN$_VERSION, 0,
		0
	};
*/
	long	fldflags;

	DCLDESC( srcstr_d );	/* descriptor for input source string	*/
	DCLDESC( lognam_d );	/* descriptor for translated log name	*/
	DCLDESC( tabnam_d );	/* descriptor for log name tab		*/
	DCLDESC( temp );
	/*
	 * make a nicely formed VMS style output file name
	 */
	scanlist.file.code = FSCN$_FILESPEC;
	scanlist.node.code = FSCN$_NODE;
	scanlist.devi.code = FSCN$_DEVICE;
	scanlist.root.code = FSCN$_ROOT;
	scanlist.dire.code = FSCN$_DIRECTORY;
	scanlist.name.code = FSCN$_NAME;
	scanlist.type.code = FSCN$_TYPE;
	scanlist.vers.code = FSCN$_VERSION;
	scanlist.zero = 0;

	INITDESC_W( lognam_d, lognam_b );
	INITDESC_W( srcstr_d, srcstr_b );
	if( options->print_dest != NULL )
	{
		FILLDESC_R( srcstr_d, options->print_dest );
	}
	else
	{
		FILLDESC_R( srcstr_d, "DECW$PRINTSCREEN" );
	}

	if( BAD( status = SYS$FILESCAN( &srcstr_d, &scanlist, &fldflags)) )
		return(NULL);

	switch( options->storage_format )
	{
		case DECW$C_PRSC_POSTSCRIPT:
			strcpy( ftype, ".ps" );
			break;
		case DECW$C_PRSC_SIXEL:
			strcpy( ftype, ".sxl" );
			break;
		case DECW$C_PRSC_DDIF:
			strcpy( ftype, ".ddif" );
			break;
		default:
			return( NULL );
			break;

	}
	strcpy( fname, "decw$printscreen");

	if( scanlist.name.len == 0 )
	{
		scanlist.name.addr = fname;
		scanlist.name.len = strlen(fname);
	}

	if( scanlist.type.len == 0 )
	{
		scanlist.type.addr = ftype;
		scanlist.type.len = strlen(ftype);
	}

	/*
	 * if the file spec is more than 1024, die
	 */
	if( (speclen = scanlist.node.len + scanlist.devi.len +
		scanlist.root.len + scanlist.dire.len + scanlist.name.len +
		scanlist.type.len + scanlist.vers.len + 1) > 1024 )
	{
		return( NULL );
	}

	/*
	 * now copy the whole mess and get a file spec
	 */
	fspec = options->print_dest;

	bcopy(scanlist.node.addr, fspec, scanlist.node.len);
	count = scanlist.node.len;
	bcopy(scanlist.devi.addr, &fspec[count], scanlist.devi.len);
	count += scanlist.devi.len;
	bcopy(scanlist.root.addr, &fspec[count], scanlist.root.len);
	count += scanlist.root.len;
	bcopy(scanlist.dire.addr, &fspec[count], scanlist.dire.len);
	count += scanlist.dire.len;
	bcopy(scanlist.name.addr, &fspec[count], scanlist.name.len);
	count += scanlist.name.len;
	bcopy(scanlist.type.addr, &fspec[count], scanlist.type.len);
	count += scanlist.type.len;
	bcopy(scanlist.vers.addr, &fspec[count], scanlist.vers.len);
	count += scanlist.vers.len;
	fspec[count] = '\0';

	/*
	 * lets finally open the file
	 */
	file = TRUE;

	file = fopen( fspec, "w" );

	if (!fgetname(file, fspec, 1) )
		return(NULL);

	
/* 
 * If DDIF, close the file because ISL wants to open it.  The reason we open
 * it at all is to return a file io error.  If we let the ISL call blow up, 
 * we'll get a fatal lib error, which is not real helpful 
 */

    	if (options->storage_format == dxPrscDDIF)
    		{
    		fclose( file);
    		delete( fspec);
    		}


	return( file);
}
#endif
