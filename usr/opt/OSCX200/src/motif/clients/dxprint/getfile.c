/*
*****************************************************************************
**                                                                          *
**                      COPYRIGHT (c) 1988, 1992 BY                         *
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

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /b5/aguws3.0/aguws3.0_rcs/src/dec/clients/print/getfile.c,v 1.2 91/12/30 12:48:20 devbld Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

#include "iprdw.h"

#ifdef VMS
#include <desc.h>	/* should this be <descrip.h> ? */
#include <ssdef.h>
#include <fscndef.h>

typedef struct
{
    short	len;
    short	code;
    void	*addr;
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
#endif	/* VMS */

FILE *getfile
#if _PRDW_PROTO_
(
    dxPrscOptions	*options
)
#else
(options)
    dxPrscOptions	*options;
#endif
#if !defined(VMS)
{
    FILE	*file;		/* pointer to the opened file(or NULL) 	*/

    /*
    ** if a file is specified, we write to that file, otherwise
    ** to the standard output it goes
    */
    if( command.print_dest[0] == 0 )
    {
	file = stdout;
    }
    else
    {
	if (command.print_dest[0] == '|')
	    file = (FILE *) popen( &(command.print_dest[1]), "w" );
	else
	    file = fopen( command.print_dest, "w" );
    }
    
    /* 
    ** If DDIF, close the file, but we're not going to bother to delete the file
    ** Ultirix has no version numbers, the second open will overwrite the first.
    */
    if (command.options.storage_format == dxPrscDDIF)
    {
	fclose( file);
    }

    return (file);

}
#else
{
    FILE		*file;		/* pointer to the opened file(or NULL)*/
    char		ftype[10];	/* three letter file type	      */
    char		fname[32];	/* default file name		      */
    char		*fspec;		/* the whole filespec		      */
    char		lognam_b[1024];	/* buffer for translated log name     */
    char		srcstr_b[1024];	/* buffer for input source string     */
    int			speclen;	/* length of whole filespec	      */
    int			count;		/* bytes used in whole filespec	      */
    long		status;		/* return status code		      */
    static scan_d	scanlist =	/* filescan item list		      */
    {
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

    long		fldflags;
    int			namlen, typlen;

#pragma nostandard
    DCLDESC( srcstr_d );	/* descriptor for input source string	*/
    DCLDESC( lognam_d );	/* descriptor for translated log name	*/
    DCLDESC( tabnam_d );	/* descriptor for log name tab		*/
    DCLDESC( temp );
#pragma standard

    INITDESC_W( lognam_d, lognam_b );
    INITDESC_W( srcstr_d, srcstr_b );
    if( command.print_dest != 0 )
    {
	FILLDESC_R( srcstr_d, command.print_dest );
    }
    else
    {
	FILLDESC_R( srcstr_d, "DECW$PRINTSCREEN" );
    }

    if( BAD( status = sys$filescan( &srcstr_d, &scanlist, &fldflags)) )
	    return(NULL);

    switch (command.options.storage_format)
    {
    case dxPrscPostscript:
	typlen = strlen(".ps");
	memcpy (ftype, ".ps", typlen);
	break;
    case dxPrscSixel:
	typlen = strlen(".sxl");
	memcpy (ftype, ".sxl", typlen);
	break;
    case dxPrscDDIF:
	typlen = strlen(".ddif");
	memcpy (ftype, ".ddif", typlen);
	break;
    default:
	return( NULL );
	break;

    }

    namlen = strlen("decw$printscreen");
    memcpy (fname, "decw$printscreen", namlen);

    if( scanlist.name.len == 0 )
    {
	scanlist.name.addr = fname;
	scanlist.name.len = namlen;
    }

    if( scanlist.type.len == 0 )
    {
	scanlist.type.addr = ftype;
	scanlist.type.len = typlen;
    }

    /*
    ** if the file spec is more than 1024, die
    */
    speclen = scanlist.node.len + scanlist.devi.len +
	scanlist.root.len + scanlist.dire.len + scanlist.name.len +
	scanlist.type.len + scanlist.vers.len + 1;
    if (speclen > 1024)
    {
	return( NULL );
    }

    /*
    ** now copy the whole mess and get a file spec
    */
    fspec = command.print_dest;

    memcpy(fspec, scanlist.node.addr, scanlist.node.len);
    count = scanlist.node.len;
    memcpy(&fspec[count], scanlist.devi.addr, scanlist.devi.len);
    count += scanlist.devi.len;
    memcpy(&fspec[count], scanlist.root.addr, scanlist.root.len);
    count += scanlist.root.len;
    memcpy(&fspec[count], scanlist.dire.addr, scanlist.dire.len);
    count += scanlist.dire.len;
    memcpy(&fspec[count], scanlist.name.addr, scanlist.name.len);
    count += scanlist.name.len;
    memcpy(&fspec[count], scanlist.type.addr, scanlist.type.len);
    count += scanlist.type.len;
    memcpy(&fspec[count], scanlist.vers.addr, scanlist.vers.len);
    count += scanlist.vers.len;
    fspec[count] = '\0';

    /*
    ** lets finally open the file
    */
    file = fopen (fspec, "w");

    if (!fgetname(file, fspec, 1)) return(NULL);

	
/*
 * If DDIF, close the file because ISL wants to open it.  The reason we open
 * it at all is to return a file io error.  If we let the ISL call blow up, 
 * we'll get a fatal lib error, which is not real helpful 
 */

    if (command.options.storage_format == dxPrscDDIF)
    {
	fclose (file);
	delete (fspec);
    }


    return (file);
}
#endif
