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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: UilSrcSrc.c,v $ $Revision: 1.1.4.3 $ $Date: 1993/10/18 16:51:17 $"
#endif
#endif

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      This module contains the procedure for managing the UIL source
**	files.  
**
**--
**/


/*
**
**  INCLUDE FILES
**
**/

#include "UilDefI.h"

/* %COMPLETE */
#include <sys/stat.h>
#ifdef WIN32
#include <sys\types.h>
#include <X11\xlib_nt.h>
#endif


/*
**
**  DEFINE and MACRO DEFINITIONS
**
**/



/*
**
**  EXTERNAL VARIABLE DECLARATIONS
**
**/


/*
**
**  GLOBAL VARIABLE DECLARATIONS
**
**/

#ifdef VMS
#if defined(__DECC)
/*
 *  Hack alert!  The DEC C RTL changed these from external symbols to macros,
 *  at least that's what they say they are going to do "real soon now"...
 */
#define SHELL$TO_VMS            decc$to_vms
#define SHELL$K_FOREIGN         0
#define SHELL$K_FILE            1
#define SHELL$K_DIRECTORY       2
#else
globalvalue int SHELL$K_FILE;
globalvalue int SHELL$K_DIRECTORY;
#endif
#endif

/*
**  define the source buffer data structures
*/

externaldef(uil_comp_glbl) src_source_buffer_type	*src_az_current_source_buffer;
externaldef(uil_comp_glbl) src_source_buffer_type	*src_az_avail_source_buffer;
externaldef(uil_comp_glbl) src_message_item_type	*src_az_orphan_messages;
/* %COMPLETE */
externaldef(uil_comp_glbl) long Uil_file_size;
#ifndef WIN32
struct stat stbuf;
#else
struct _stat stbuf;
#endif


/*
**  define the source record data structures
*/

externaldef(uil_comp_glbl) src_source_record_type
	*src_az_current_source_record;
externaldef(uil_comp_glbl) src_source_record_type
	*src_az_first_source_record;

externaldef(uil_comp_glbl) uil_fcb_type
 	*src_az_source_file_table[src_k_max_source_files];
externaldef(uil_comp_glbl) int
	src_l_last_source_file_number;

/*
**
**  OWN VARIABLE DECLARATIONS
**
**/
    static uil_fcb_type	    *main_fcb;
    static char		    *include_dir;
    static unsigned short   main_dir_len;
#ifdef VMS
    static char             shell_file_name[ NAM$C_MAXRSS+1 ];
#endif




/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure initializes the reading of the UIL source program.
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**      Uil_cmd_z_command
**
**  IMPLICIT OUTPUTS:
**
**      src_az_first_source_buffer
**      src_az_current_source_buffer
**	src_l_last_source_file_number
**	src_az_source_file_table
**      src_az_current_source_record
**	main_fcb
**	include_dir 
**	main_dir_len
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      source file is opened
**	source buffer structure is setup
**	source record structure is setup
**
**--
**/

void	src_initialize_source()
{
    /* initialize source data structures */

    src_az_current_source_buffer = NULL;
    src_az_avail_source_buffer = NULL;
    src_l_last_source_file_number = -1;
    src_az_first_source_record = NULL;
    src_az_current_source_record =
	(src_source_record_type *) &src_az_first_source_record;

    /*  Initialize Own storage    */

    main_fcb = NULL;
    include_dir = NULL;
    main_dir_len = 0;


    /* open the source file */
    if ( Uil_cmd_z_command.ac_source_file == NULL )
	diag_issue_diagnostic (d_src_open,
			       diag_k_no_source, diag_k_no_column,
			       "<null file name>");

    src_open_file ( Uil_cmd_z_command.ac_source_file, NULL );

    /* fixes initial filename is NULL bug in callable UIL */
    Uil_current_file = Uil_cmd_z_command.ac_source_file;

    return;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This procedure does the cleanup processing of the source files
**	structures.
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**	Uil_cmd_z_command
**      src_az_first_source_buffer
**      src_az_current_source_buffer
**	src_l_last_source_file_number
**	src_az_source_file_table
**      src_az_current_source_record
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**	structures are freed
**
**--
**/

void	Uil_src_cleanup_source()
{

    int				i;		    /* index over fcbs */
    src_source_buffer_type	*buffer_to_free;   
    src_source_record_type	*record_to_free;
    status			l_close_status;

    /*
    **  Loop through all open files freeing their fcbs
    */
    for (i = 0; i <= src_l_last_source_file_number; i++)
	{
	/* it is possible to get an error before the files are open,
	   so check and see if table is NULL before opening */
	if (src_az_source_file_table[i] == NULL)
		continue;
	l_close_status = close_source_file (src_az_source_file_table[i]);
	if ( l_close_status == src_k_close_error )
	    {
	    diag_issue_diagnostic (d_src_close,
				   diag_k_no_source, diag_k_no_column,
				   src_az_source_file_table[i]->expanded_name);
	    }
	_free_memory ((char*)src_az_source_file_table [i]);
	src_az_source_file_table[i] = NULL;
	}

    /*
    **  Loop through list of current source buffers, freeing them
    */
    while (src_az_current_source_buffer != NULL)
	{
    	buffer_to_free = src_az_current_source_buffer;
    	src_az_current_source_buffer = 
	    src_az_current_source_buffer->az_prior_source_buffer;
	_free_memory ((char*)buffer_to_free);
	}

    /*
    **  Loop through list of source records, freeing them
    */
    while (src_az_first_source_record != NULL)
	{
	record_to_free = src_az_first_source_record;
	src_az_first_source_record =
	    src_az_first_source_record->az_next_source_record;
	_free_memory ((char*)record_to_free);
	}

    /*
    **  Free Own storage
    */
/* BEGIN OSF FIX pir 2240 */
    /* Memory pointed to by main_fcb already freed. */
/* END OSF FIX pir 2240 */
    _free_memory (include_dir);

    return;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure opens a file and sets up the static pointers to
**	read from this file.
**
**  FORMAL PARAMETERS:
**
**      c_file_name	    file to open
**
**  IMPLICIT INPUTS:
**
**      src_az_first_source_buffer
**      src_az_current_source_buffer
**	src_l_last_source_file_number
**	src_az_source_file_table
**
**  IMPLICIT OUTPUTS:
**
**      src_az_first_source_buffer
**      src_az_current_source_buffer
**	src_l_last_source_file_number
**	src_az_source_file_table
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      input file is opened
**	input buffer structure is setup
**	input record structure is setup
**
**--
**/

void	    src_open_file
		(c_file_name,full_file_name)

XmConst char	* c_file_name;
char		* full_file_name;

{
    uil_fcb_type		*az_fcb;	    /* file control block ptr */
    status			l_open_status;	    /* status variable */
    src_source_buffer_type	*az_source_buffer;  /* source buffer ptr */

    /* allocate fcb and source buffer */

    az_fcb = (uil_fcb_type *) _get_memory (sizeof (uil_fcb_type));

    if (src_az_avail_source_buffer != NULL) {
	az_source_buffer = src_az_avail_source_buffer;
	src_az_avail_source_buffer = 
		src_az_avail_source_buffer->az_prior_source_buffer;
    } else {
	az_source_buffer =
	    (src_source_buffer_type *)
			_get_memory (sizeof (src_source_buffer_type));
    }

    /* Call the OS-specific open file procedure */

    l_open_status = open_source_file (
			c_file_name,
			az_fcb,
			az_source_buffer );

    /*	If the file is not found, a fatal error is generated.	*/

    if ( l_open_status == src_k_open_error ) {
	diag_issue_diagnostic( d_src_open,
			       diag_k_no_source, diag_k_no_column,
			       c_file_name );
    }

    /* put fcb in the file table */

    src_l_last_source_file_number++;

    if (src_l_last_source_file_number >= src_k_max_source_files) {
	diag_issue_diagnostic (
		d_src_limit,
		src_az_current_source_record,
		src_az_current_source_buffer -> w_current_position - 1,
		az_fcb->expanded_name );
    }

    src_az_source_file_table[ src_l_last_source_file_number ] = az_fcb;

    /* Complete the OS-independent initialization. Get the size of the file
    ** for %complete info then initialize a source
    ** buffer placing a null in the buffer will cause the lexical analyzer
    ** to start by reading the first line of the file
    */

    /* %COMPLETE */
    if (stat(az_fcb->expanded_name, &stbuf) == -1) {
	diag_issue_diagnostic( d_src_open,
			       diag_k_no_source, diag_k_no_column,
			       az_fcb->expanded_name );
    }

    Uil_file_size = stbuf.st_size;

    if (full_file_name != NULL)
	strcpy (full_file_name, az_fcb->expanded_name);

    az_fcb->v_position_before_get = FALSE;

    az_source_buffer->w_current_line_number = 0;
    az_source_buffer->b_file_number = src_l_last_source_file_number;
    az_source_buffer->w_current_position = 0;
    az_source_buffer->c_text[ 0 ] = 0;

    /* make the source buffer current */

    az_source_buffer->az_prior_source_buffer =
				src_az_current_source_buffer;
    src_az_current_source_buffer = az_source_buffer;

    return;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure reads the next source line;
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**      src_az_source_file_table
**
**  IMPLICIT OUTPUTS:
**
**      src_az_current_source_buffer
**      src_az_current_source_record
**
**  FUNCTION VALUE:
**
**      src_k_end_source	no more source lines
**      src_k_read_normal	new line in the source buffer
**
**  SIDE EFFECTS:
**
**      may issue diagnostics if error occurs reading record
**	may restore previous source upon reaching end of current source
**
**--
**/

status	src_get_source_line()
{
    uil_fcb_type	    *az_fcb;
    src_source_record_type  *az_source_record;
    status		    l_read_status;

    /* Return if already at the end of file */

    if (src_az_current_source_buffer == NULL)
	return src_k_end_source;

    /* Find the current fcb */

    az_fcb = src_az_source_file_table
		[ src_az_current_source_buffer->b_file_number ];

    /* Read the next record */

    l_read_status = get_line( az_fcb );

    /* Increment lines processed count, and update current file */
    Uil_lines_processed++;
    Uil_current_file = az_fcb->expanded_name;

    if ( (l_read_status == src_k_read_normal) ||
	 (l_read_status == src_k_read_truncated) )
    {
	/* Read was successful
	 * Set position to the start of the record */

	src_az_current_source_buffer->w_current_position = 0;

	/* Allocate and initialize a source record */
	
	az_source_record = 
	    (src_source_record_type *)
		_get_memory( sizeof( src_source_record_type ) );

	az_source_record->az_next_source_record = NULL;
	az_source_record->w_line_number = 
	    ++src_az_current_source_buffer->w_current_line_number;
	az_source_record->b_file_number = 
	    src_az_current_source_buffer->b_file_number;
	az_source_record->az_message_list = NULL;
	az_source_record->az_machine_code_list = NULL;
	az_source_record->w_machine_code_cnt = 0;
	az_source_record->z_access_key = az_fcb->last_key;

	/* was uninitialized; fixes listing problem on HP  (RAP) */
        az_source_record->b_flags = 0;

	/* Link the source record to the end of source record list */

	src_az_current_source_record->az_next_source_record =
	    az_source_record;
	src_az_current_source_record = az_source_record;

	if (l_read_status == src_k_read_truncated)
	    diag_issue_diagnostic( d_src_truncate,
			      	   src_az_current_source_record,
			      	   diag_k_no_column,
				   src_k_max_source_line_length );
    
	return src_k_read_normal;
    }

    /* Check for end of file */

    if (l_read_status == src_k_end_source)
    {
	src_source_buffer_type	*az_prior_source_buffer;
	
	/* get prior source buffer */

	az_prior_source_buffer = 
	    src_az_current_source_buffer->az_prior_source_buffer;

	/* place current source buffer on the available list */

	src_az_current_source_buffer->az_prior_source_buffer =
	    src_az_avail_source_buffer;
	src_az_avail_source_buffer = src_az_current_source_buffer;

	/* if there is no prior source buffer - return end of source */

	if (az_prior_source_buffer == NULL)
	    return src_k_end_source;

	/* restore the prior buffer as current */

	src_az_current_source_buffer = az_prior_source_buffer;

	return src_k_read_normal;
    }

    /* must have been an error */

    diag_issue_diagnostic( d_src_read,
		      	   src_az_current_source_record,
		      	   diag_k_no_column,
			   az_fcb->expanded_name );

    _assert( FALSE, "read past source error" );
	return(src_k_read_error);
}


#ifdef VMS

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      VMS specific file open of the source file.
**
**  FORMAL PARAMETERS:
**
**      c_file_name         source file to open
**      az_fcb              file control block for the file
**      az_source_buffer    source buffer for the file
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      src_k_open_normal
**      src_k_open_error
**
**  SIDE EFFECTS:
**
**      file is opened and has a source buffer associated with it
**
**--
**/

status  open_source_file( c_file_name, az_fcb, az_source_buffer )
const char                      *c_file_name;
uil_fcb_type                    *az_fcb;
const src_source_buffer_type    *az_source_buffer;

{
    status              l_status;
    char                interim_file_name[ NAM$C_MAXRSS+1 ];
    boolean             main_file;
    int                 i;  /* loop index for searching include directories */

/*    These are obtained from starlet.req.  There is no c header file
      with this information.    */

#define         FSCN$M_NODE             (1 << 0)
#define         FSCN$M_DEVICE           (1 << 1)
#define         FSCN$M_DIRECTORY        (1 << 3)

#define         FSCN$_FILESPEC          1          /* complete filespec */
#define         FSCN$_NODE              2          /* node:: field */
#define         FSCN$_DEVICE            3          /* device: field */
#define         FSCN$_ROOT              4          /* [root.] field */
#define         FSCN$_DIRECTORY         5          /* [directory] field */
#define         FSCN$_NAME              6          /* name field */
#define         FSCN$_TYPE              7          /* .typ field */
#define         FSCN$_VERSION           8          /* ;version field */

    typedef struct {
        unsigned short int      FSCN$W_LENGTH;     /* return length */
        unsigned short int      FSCN$W_ITEM_CODE;  /* item code */
        unsigned char           * FSCN$L_ADDR;     /* return pointer */
    } FSCN$ITEM;

    struct dsc$descriptor       file_desc;
    FSCN$ITEM                   valuelst [4];
    unsigned long               fldflags;

    file_desc . dsc$b_dtype = DSC$K_DTYPE_T;
    file_desc . dsc$b_class = DSC$K_CLASS_S;
    file_desc . dsc$w_length = strlen (c_file_name);
    file_desc . dsc$a_pointer = c_file_name;
    fldflags = 0;

    az_fcb->fab = cc$rms_fab;
    az_fcb->nam = cc$rms_nam;
    az_fcb->rab = cc$rms_rab;

    az_fcb->fab.fab$b_fac = FAB$M_GET;
    az_fcb->fab.fab$l_fna = c_file_name;
    az_fcb->fab.fab$b_fns = strlen(c_file_name);
    az_fcb->fab.fab$l_nam = &(az_fcb->nam);

    /*
    **  If the open of the file fails, interim_file_name give the name
    **  of the file we were trying to open.  If the open succeeds then
    **  interim_file_name is not need.  Expanded name has the full file spec.
    */

    az_fcb->nam.nam$l_rsa = &(az_fcb->expanded_name);
    az_fcb->nam.nam$b_rss = NAM$C_MAXRSS;

    az_fcb->nam.nam$l_esa = interim_file_name;
    az_fcb->nam.nam$b_ess = NAM$C_MAXRSS;

    az_fcb->rab.rab$l_fab = &(az_fcb->fab);
    az_fcb->rab.rab$b_rac = RAB$C_SEQ;
    az_fcb->rab.rab$l_rop = RAB$M_RAH;
    az_fcb->rab.rab$w_usz = src_k_max_source_line_length;

/*    Determine if this is the main file or an include file.  */

    main_file = (main_fcb == NULL);

    if (main_file) {

        static char const       c_main_ext[]= ".uil";
        int             len;
        char            * ptr;

/*    Check if any directory information is specified for the main
 *    file.  If not, then any include files which do not have directory
 *    information specified will use the default directory.  Otherwise,
 *    include files will use the directory specified for the main file.
 */

        valuelst [0].FSCN$W_ITEM_CODE = FSCN$_NODE;
        valuelst [1].FSCN$W_ITEM_CODE = FSCN$_DEVICE;
        valuelst [2].FSCN$W_ITEM_CODE = FSCN$_DIRECTORY;
        valuelst [3].FSCN$W_ITEM_CODE = valuelst [3].FSCN$W_LENGTH = 0;

        l_status = sys$filescan (& file_desc, & valuelst [0], & fldflags);
        _assert( _success (l_status), "filescan error" );

        if ((fldflags & (FSCN$M_NODE | FSCN$M_DEVICE | FSCN$M_DIRECTORY)) != 0)
            {
            len = valuelst [0].FSCN$W_LENGTH +
                  valuelst [1].FSCN$W_LENGTH + valuelst [2].FSCN$W_LENGTH;
            _assert( len != 0, "missing directory spec" );
            ptr = file_desc .dsc$a_pointer;
        } else {
            ptr = getenv ("UIL$INCLUDE");
            len = strlen (ptr);
        }

        include_dir = (char *) _get_memory (len + 1);
        _move (include_dir, ptr, len);
        include_dir [len] = '\0';

        main_fcb = az_fcb;

/*    Just look in the specified directory for the main file.   */

        az_fcb->fab.fab$l_dna = c_main_ext;
        az_fcb->fab.fab$b_dns = (sizeof c_main_ext)-1;

        l_status = sys$open( &(az_fcb->fab) );

    } else {

        static char const       c_include_ext[]= ".uil";
        static char const       c_include_dir[]= "uil$include:";
        char                    buffer [ NAM$C_MAXRSS+1 ], * ptr;
        char                    tmpbuffer [ NAM$C_MAXRSS+1 ];
        int                     len, total_len, number_found;

/*
    Check to see if the file includes any Ultrix file syntax (look for "/")
    If found append a "/" if it isn't the first character
    Convert the file using SHELL$TO_VMS C-RTL call.
*/
        for (len = strlen (c_file_name),
             ptr = & c_file_name [len - 1];
             len > 0; len--, ptr--)
            {
            if ((* ptr) == '/')
                {
                break;
                }
            }

        if (len != 0)
            {
             if (c_file_name[0] == '/')
                {
                number_found = SHELL$TO_VMS( c_file_name, convert_unix_to_vms, 0);
                }
            else
                {
                _move (tmpbuffer, "/", 1);
                _move (& tmpbuffer[1], c_file_name, strlen (c_file_name) + 1);
                number_found = SHELL$TO_VMS( tmpbuffer, convert_unix_to_vms, 0);
                }
            if (number_found != 1)
                {
                l_status = 0;       /* force an error */
                goto open_label;
                }
            file_desc . dsc$w_length = strlen (shell_file_name);
            file_desc . dsc$a_pointer = shell_file_name;
            az_fcb->fab.fab$l_fna = shell_file_name;
            az_fcb->fab.fab$b_fns = strlen(shell_file_name);
            }

/*    Look in the specified directory for the include file.  If the dir
      is not specified, have RMS look in the main file's directory  */

        ptr = buffer; total_len = 0;

        valuelst [0].FSCN$W_ITEM_CODE = valuelst [0].FSCN$W_LENGTH = 0;

        l_status = sys$filescan (& file_desc, & valuelst, & fldflags);

        _assert( _success (l_status), "filescan error" );

        if ((fldflags & (FSCN$M_NODE | FSCN$M_DEVICE | FSCN$M_DIRECTORY)) == 0)
            {
            len = strlen (include_dir);
            _move (ptr, include_dir, len);
            ptr += len; total_len += len;
        }

        len = sizeof c_include_ext - 1;
        _move (ptr, c_include_ext, len);
        ptr += len; total_len += len;

        az_fcb->fab.fab$l_dna = buffer;
        az_fcb->fab.fab$b_dns = total_len;

        l_status = sys$open( &(az_fcb->fab) );

        if ( _success ( l_status )) {
            goto open_label;
        }

/*    Look in the include directory for the include file only if
      no directory spec is in the file name.  */

        if (
            ( fldflags & (FSCN$M_NODE | FSCN$M_DEVICE | FSCN$M_DIRECTORY) )
            != 0 ) {
            l_status = 0;       /* force an error */
            goto open_label;
        }



/*    Look in the command line specified include directories, if any.    */

        for (i = 0; i < Uil_cmd_z_command.include_dir_count; i++) {
            /* respecify the filename to open (for some reason after        */
            /* sys$open it is nolonger correct)                             */
            len = az_fcb->nam.nam$b_name + az_fcb->nam.nam$b_type +
                  az_fcb->nam.nam$b_ver;

            az_fcb->fab.fab$l_fna = az_fcb->nam.nam$l_name;
            az_fcb->fab.fab$b_fns = len;

            /* set the default name to point to the next include directory  */
            ptr = buffer; total_len = 0;
            len = strlen (Uil_cmd_z_command.ac_include_dir[i]);
            _move (ptr, Uil_cmd_z_command.ac_include_dir[i], len);
            ptr += len; total_len += len;

            /* tack on the file extension to the default name               */
            len = sizeof c_include_ext - 1;
            _move (ptr, c_include_ext, len);
            ptr += len; total_len += len;

            /* fill it in to the default name field of the fab              */
            az_fcb->fab.fab$l_dna = buffer;
            az_fcb->fab.fab$b_dns = total_len;

            l_status = sys$open( &(az_fcb->fab) );

            /* If we didn't get an error, then we have found the file,      */
            /* otherwise, look at the next file in the list                 */
            if (!_error(l_status)) break;
            };

    }




    /* check the open status. */

open_label:

    if (_error( l_status )) {
        _move( az_fcb->expanded_name,
                   interim_file_name, az_fcb->nam.nam$b_esl );
        az_fcb->expanded_name[ az_fcb->nam.nam$b_esl ] = 0;
        return src_k_open_error;
    }

    /* make expanded file name a null terminated string */

    az_fcb->expanded_name[ az_fcb->nam.nam$b_rsl ] = 0;

    /* connect to the file */

    l_status = sys$connect( &(az_fcb->rab) );

    if (_error( l_status ))
        return src_k_open_error;

    /* open succeeded - place buffer address in fcb */

    az_fcb->rab.rab$l_ubf = az_source_buffer->c_text;

    return src_k_open_normal;
}
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      VMS specific SHELL$TO_VMS action routine.
**      SHELL$TO_VMS converts UNIX file names to VMS file names
**
**  FORMAL PARAMETERS:
**
**      name - file name in VMS format
**      type - file type
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      True - Not a valid file, continue file translation
**      False - Valid file, cease file translation
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

convert_unix_to_vms ( name, type)
    char *name;
    int type;
{
    if (type == SHELL$K_DIRECTORY)
        {
        return (1);
        }
    else {
        _move (shell_file_name, name, strlen (name) + 1);
        return (0);
        }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      VMS specific close source file.
**
**  FORMAL PARAMETERS:
**
**      az_fcb              file control block for the file
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      src_k_close_error
**      src_k_close_normal
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

status  close_source_file ( az_fcb )
uil_fcb_type    *az_fcb;
{
    status      l_close_status;
    struct FAB  *az_fab;

    az_fab = &(az_fcb->fab);

    /*
    ** close the file
    */

    l_close_status = sys$close( az_fab );

    if ( _success( l_close_status ))
        return src_k_close_normal;
    else
        return src_k_close_error;
}
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      VMS specific file read line of the source file.
**
**  FORMAL PARAMETERS:
**
**      az_fcb              file control block for the file
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      src_k_read_normal
**      src_k_read_error
**      src_k_end_source
**
**  SIDE EFFECTS:
**
**      next record in file is read
**
**--
**/

status  get_line( az_fcb )
uil_fcb_type    *az_fcb;
{
    status      l_read_status;
    struct RAB  *az_rab;

    az_rab = &(az_fcb->rab);

    /*
    **  if v_position_before_get is true, we need to reposition
    **  before the get because another retrieve has altered the
    **  current record.
    */

    if (az_fcb->v_position_before_get)
    {
        az_rab->rab$b_rac = RAB$C_RFA;
        *((z_key *)(az_rab->rab$w_rfa)) = az_fcb->last_key;
        sys$get( az_rab );
        az_rab->rab$b_rac = RAB$C_SEQ;
        az_fcb->v_position_before_get = FALSE;
    }

    /* read the next line */

    l_read_status = sys$get( az_rab );

    if ( _success( l_read_status ) || ( l_read_status == RMS$_RTB ) ) {

        /* Read was successful or truncated
         * Put a null at the end of record read */

        src_az_current_source_buffer->c_text[ az_rab->rab$w_rsz ] = 0;
        az_fcb->last_key = *((z_key *)(az_rab->rab$w_rfa));

        /* Check the record for null characters. */

        if (strlen (src_az_current_source_buffer->c_text) !=
                                                        az_rab->rab$w_rsz) {
            diag_issue_diagnostic(
                        d_src_null_char,
                           src_az_current_source_record,
                           diag_k_no_column,
                           az_fcb->expanded_name );
        }

        if ( l_read_status == RMS$_RTB ) {
            return src_k_read_truncated;
        } else {
            return src_k_read_normal;
        }
    }

    /* Check for end of file */

    if (l_read_status == RMS$_EOF) {
        if (sym_az_current_section_entry->prev_section != NULL)
            {
            sym_include_file_entry_type *include_entry;

            /*
            ** This is the end of an include file.  Set the pointers so that 
	    ** the sections
            ** in the include file hang off the previous list correctly.
            */
            include_entry = (sym_include_file_entry_type *)
                                sym_az_current_section_entry->prev_section->entries;
            include_entry->sections = sym_az_current_section_entry;
               sym_az_current_section_entry = sym_az_current_section_entry->prev_section;
            }
        return src_k_end_source;
    }

    /* must have been an error */

    return src_k_read_error;
}
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      VMS specific file re-read line of the source file.
**
**  FORMAL PARAMETERS:
**
**      az_fcb              file control block for the file
**      c_buffer            pointer to buffer to hold the source line
**      z_access_key        key to retrieve the source line
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      v_position_before_read = TRUE
**
**  FUNCTION VALUE:
**
**      true            if the record can be retrieved
**      false           if the record cannot be retrieved
**
**  SIDE EFFECTS:
**
**      change next record for the file
**
**--
**/

boolean reget_line

#ifndef _NO_PROTO
(uil_fcb_type *az_fcb, char *c_buffer, const z_key *z_access_key )

#else
        (az_fcb, c_buffer, z_access_key)

uil_fcb_type            *az_fcb;
char                    *c_buffer;
const z_key             *z_access_key;

#endif

{
    status      l_read_status;
    struct RAB  *az_rab;
    char        *prior_buffer;

    az_rab = &(az_fcb->rab);

    /*
    **  if v_position_before_get is true, we need to reposition
    **  before the get because another retrieve has altered the
    **  current record.
    */

    az_rab->rab$b_rac = RAB$C_RFA;
    *((z_key *)(az_rab->rab$w_rfa)) = *z_access_key;
    prior_buffer = az_rab->rab$l_ubf;
    az_rab->rab$l_ubf = c_buffer;

    /* read the requested line */

    l_read_status = sys$get( az_rab );

    az_rab->rab$b_rac = RAB$C_SEQ;
    az_rab->rab$l_ubf = prior_buffer;
    az_fcb->v_position_before_get = TRUE;

    if ( _success( l_read_status ) ||
         (l_read_status == RMS$_RTB)
       )
    {
        /* Read was successful
         * Put a null at the end of record read */

        c_buffer[ az_rab->rab$w_rsz ] = 0;

        return TRUE;
    }

    /* must have been an error */

    return FALSE;
}
#else






/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      open the source file.
**
**  FORMAL PARAMETERS:
**
**      c_file_name	    source file to open
**	az_fcb		    file control block for the file
**	az_source_buffer    source buffer for the file
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      src_k_open_normal
**      src_k_open_error
**
**  SIDE EFFECTS:
**
**      file is opened and has a source buffer associated with it
**
**--
**/

status	open_source_file( c_file_name, az_fcb, az_source_buffer )
XmConst char		*c_file_name;
uil_fcb_type		*az_fcb;
src_source_buffer_type	*az_source_buffer;
{

    static unsigned short	main_dir_len = 0;
    boolean			main_file;
    int				i;  /* loop index through include files */
    char			buffer[256];


    /* place the file name in the expanded_name buffer */

    strcpy(buffer, c_file_name);

/*    Determine if this is the main file or an include file.  */

    main_file = (main_fcb == NULL);

    if (main_file) {

	char XmConst		* ptr;
	unsigned short		len;

/*    Save the directory info for the main file.    */

	for (len = strlen (c_file_name),
	     ptr = & c_file_name [len - 1];
	     len > 0; len--, ptr--) {
	    if ((* ptr) == '/') {
		break;
	    }
	}

	main_dir_len = len;
	main_fcb = az_fcb;

/*    Open the main file.    */

	az_fcb->az_file_ptr = fopen(c_file_name, "r");

    } else {
	static char XmConst	c_include_dir[]= "/usr/include/";
	Boolean			search_user_include=True;
	Boolean			specific_directory=False;

/*    See if the file name has a leading slash and set the flag. 
      Look in the specified directory for the include file.  If the dir
      is not specified (leading slash), look in the main file's directory  */

	if (c_file_name[0] == '/') {
	    specific_directory = True;
	    }

	if (!specific_directory) {
	    _move (buffer, main_fcb -> expanded_name, main_dir_len);
	    _move (& buffer [main_dir_len],
		   c_file_name, strlen (c_file_name) + 1);  /* + NULL */
	} else {
	    strcpy (buffer, c_file_name);
	}

/*    Open the include file.    */

	az_fcb->az_file_ptr = fopen (buffer, "r");

/*    If a specific directory was specified, or if the file was found,
      then we are done.	*/

	if ( (specific_directory) || (az_fcb -> az_file_ptr != NULL) ) {
	  goto open_label;
	}

/*    Look in the command line specified include directories, if any.    */

	for (i = 0; i < Uil_cmd_z_command.include_dir_count; i++) {	     
	    int		inc_dir_len;

	    inc_dir_len = strlen (Uil_cmd_z_command.ac_include_dir[i]);
	    if (inc_dir_len == 0) {
		search_user_include = False;
		}
	    _move (buffer, Uil_cmd_z_command.ac_include_dir[i], inc_dir_len);

	/*  Add '/' if not specified at end of directory  */

	    if (Uil_cmd_z_command.ac_include_dir[i][inc_dir_len - 1] != '/') {
		buffer [inc_dir_len] = '/';
		inc_dir_len++;
	    };

	    _move (& buffer [inc_dir_len],
		   c_file_name, strlen (c_file_name) + 1);  /* + NULL */

	/*    Open the include file.  If found, we are done.    */

	    az_fcb->az_file_ptr = fopen (buffer, "r");

	    if (az_fcb -> az_file_ptr != NULL) {
		goto open_label;
	    }
	}

/*    Look in the default include directory.    */
	if (search_user_include) {
	  _move(buffer, c_include_dir, sizeof c_include_dir - 1); /* no NULL */
	  _move(&buffer[sizeof c_include_dir - 1], 
		c_file_name, strlen (c_file_name) + 1);  /* + NULL */

/*    Open the include file.    */
	  az_fcb->az_file_ptr = fopen (buffer, "r");
	}
    }

open_label:

    /* check the open status. */

    if (az_fcb->az_file_ptr == NULL)
	return src_k_open_error;

    /* open succeeded - place buffer address in fcb */

    az_fcb->c_buffer = az_source_buffer->c_text;
    az_fcb->c_buffer[ src_k_max_source_line_length ] = 0;
    strcpy(az_fcb->expanded_name, buffer);

    return src_k_open_normal;
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      close the source file.
**
**  FORMAL PARAMETERS:
**
**	az_fcb		    file control block for the file
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      src_k_close_normal
**      src_k_close_error
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/

status	close_source_file( az_fcb )
uil_fcb_type	*az_fcb;
{
    status	l_close_status;

    /*
    ** Close  the file
    */

    l_close_status = fclose (az_fcb->az_file_ptr);

    if ( l_close_status != EOF )
	return src_k_close_normal;
    else
	return src_k_close_error;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      read line of the source file.
**
**  FORMAL PARAMETERS:
**
**	az_fcb		    file control block for the file
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      src_k_read_normal
**      src_k_read_error
**	src_k_read_truncated
**	src_k_end_source
**
**  SIDE EFFECTS:
**
**      next record in file is read
**
**--
**/

status	get_line( az_fcb )
uil_fcb_type	*az_fcb;
{
    status	l_read_status;
    char	*c_new_line;

    /*	
    **	if v_position_before_get is true, we need to reposition
    **	before the get because another retrieve has altered the
    **	current record.
    */

    if (az_fcb->v_position_before_get)
    {
	fseek( az_fcb->az_file_ptr,
	       az_fcb->last_key.l_key,
	       0 );
	l_read_status = (status) fgets(az_fcb->c_buffer, 
			      src_k_max_source_line_length, 
			      az_fcb->az_file_ptr);
	az_fcb->v_position_before_get = FALSE;
    }

    /* get the current offset */

    az_fcb->last_key.l_key = ftell(az_fcb->az_file_ptr);
    
    /* read the next line */

    l_read_status = (status) fgets(az_fcb->c_buffer, 
			  src_k_max_source_line_length, 
			  az_fcb->az_file_ptr);

    if ( l_read_status != 0 )
    {
	/* Read was successful
	 * Find \n character an replace with a null */

	c_new_line = (char *) strchr( az_fcb->c_buffer, '\n' );

	if (c_new_line == NULL)
	    return src_k_read_truncated;

	*c_new_line = 0;

	return src_k_read_normal;
    }

    /* Check for end of file */

    if (feof(az_fcb->az_file_ptr))
    {
	if (sym_az_current_section_entry->prev_section != NULL)
	    {
	    sym_include_file_entry_type	*include_entry;

	    /*
	    ** This is the end of an include file.  Set the pointers so that the sections
	    ** in the include file hang off the previous list correctly.
	    */
	    include_entry = (sym_include_file_entry_type *)
				sym_az_current_section_entry->prev_section->entries;
	    include_entry->sections = sym_az_current_section_entry;
	    sym_az_current_section_entry = sym_az_current_section_entry->prev_section;
	    }
	return src_k_end_source;
    }

    /* must have been an error */

    return src_k_read_error;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      re-read line of the source file.
**
**  FORMAL PARAMETERS:
**
**	az_fcb		    file control block for the file
**	c_buffer	    pointer to buffer to hold the source line
**	z_access_key	    key to retrieve the source line
**  
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      v_position_before_read = TRUE
**
**  FUNCTION VALUE:
**
**      true		if the record can be retrieved
**      false		if the record cannot be retrieved
**
**  SIDE EFFECTS:
**
**      change next record for the file
**
**--
**/

boolean	reget_line( az_fcb, c_buffer, z_access_key )

uil_fcb_type		*az_fcb;
char			*c_buffer;
XmConst z_key		*z_access_key;

{
    status	l_read_status;
    char	*c_new_line;

    fseek( az_fcb->az_file_ptr,
	   z_access_key->l_key,
	   0 );

    l_read_status = (status) fgets(c_buffer, 
			  src_k_max_source_line_length, 
			  az_fcb->az_file_ptr);

    az_fcb->v_position_before_get = TRUE;

    if ( l_read_status != 0 )
    {
	/* Read was successful
	 * Find \n character an replace with a null */

	c_new_line = (char *) strchr( c_buffer, '\n' );

	if (c_new_line == NULL)
	    return src_k_read_truncated;

	*c_new_line = 0;

	return TRUE;
    }

    /* must have been an error */

    return FALSE;
}
#endif


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Given a source record, this function returns the file name of
**	the file containing that source record.
**
**  FORMAL PARAMETERS:
**
**      az_src_rec	pointer to a source record structure
**
**  IMPLICIT INPUTS:
**
**      src_az_source_file_table
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      pointer to the file name string
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

char	*src_get_file_name(az_src_rec)

XmConst src_source_record_type	*az_src_rec;

{
    uil_fcb_type    *fcb;

    /* Find the correct fcb */

    fcb = src_az_source_file_table[ az_src_rec->b_file_number ];
    
    /* Return a pointer to file name */

    return fcb->expanded_name;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Given a source record, this function retrieves the text of the
**	line corresponding to that source line.
**
**  FORMAL PARAMETERS:
**
**      az_src_rec	pointer to a source record structure
**	c_buffer	pointer to buffer to hold the text
**
**  IMPLICIT INPUTS:
**
**      src_az_source_file_table
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      true		if there is a source line
**      false		if there is no source line
**
**  SIDE EFFECTS:
**
**      buffer is set to the contents of source line
**
**--
**/

static char XmConst no_source[] = "[ source not available ]";



boolean	src_retrieve_source

#ifndef _NO_PROTO
	(XmConst src_source_record_type *az_src_rec,
	char *c_buffer)
#else
	(az_src_rec, c_buffer)

XmConst src_source_record_type	*az_src_rec;
char				*c_buffer;

#endif

{
    uil_fcb_type    *fcb;

    /* check if there is any source */

    if (az_src_rec == diag_k_no_source)
    {
	_move( c_buffer, no_source, sizeof no_source );
	return FALSE;
    }

    /* 
    **	check if we are dealing with the current source record
    **	in which case we don't need to reread the source 
    */

    if ((az_src_rec->b_file_number == 
	 src_az_current_source_buffer->b_file_number)
	 &&
	(az_src_rec->w_line_number ==
	 src_az_current_source_buffer->w_current_line_number)
       )
    {
	strcpy( c_buffer, 
	       src_az_current_source_buffer->c_text);
	return TRUE;
    }

    /*
    **	Will have to reread the data from the file.
    */

    /* Find the correct fcb */

    fcb = src_az_source_file_table[ az_src_rec->b_file_number ];
    
    /* get the line */

    if (reget_line( fcb, c_buffer, (z_key *) &(az_src_rec->z_access_key) ))
	return TRUE;

    _move( c_buffer, no_source, sizeof no_source );
    return FALSE;

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function adds diagnostic information to the source representation.
**	This permit diagnostics to be placed in the listing.
**
**  FORMAL PARAMETERS:
**
**      az_src_rec	    source line diagnostic issued against
**	l_src_pos	    offset of diagnostic in the source line
**	c_msg_text	    text of diagnostic
**	l_msg_number	    message number for diagnostic
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      diagnostic stuff away in the source structure
**
**--
**/

void    src_append_diag_info( az_src_rec, l_src_pos, c_msg_text, l_msg_number )

XmConst src_source_record_type	*az_src_rec;
XmConst int			l_src_pos;
XmConst char			*c_msg_text;
XmConst int			l_msg_number;

{
    src_message_item_type	    *az_msg_item;
    int			    	    l_msg_length;
    src_message_item_type	    *current;
    src_message_item_type   	    **prior;

    /*
    **	create the message item and fill it in.
    */

    l_msg_length = strlen( c_msg_text ) + 1;	/* includes null */

    az_msg_item = (src_message_item_type *)
	_get_memory( sizeof( src_message_item_type ) + l_msg_length );

    az_msg_item->l_message_number = l_msg_number;
    az_msg_item->b_source_pos = l_src_pos;

    _move( (az_msg_item->c_text), c_msg_text, l_msg_length );

    /*
    **  Link the message from its source line
    **	    Messages are in ascending column order for a line with
    **	      messages without column info at the end
    **	    Messages without source are appended to a list of orphans
    */

    if (az_src_rec == diag_k_no_source)
	prior = &src_az_orphan_messages;
    else
	prior = (src_message_item_type **)&(az_src_rec->az_message_list);

    current = *prior;

    for (;  
	 current != NULL;
	 prior = &(current->az_next_message),  
	 current = *prior )
    {
	if (l_src_pos < current->b_source_pos)
	    break;
    }

    az_msg_item->az_next_message = current;
    *prior = az_msg_item;
    
    return;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function adds machine code information to the source
**	representation.  This permits machine code to be placed in the listing.
**
**  FORMAL PARAMETERS:
**
**      az_src_rec	    source line machine code is associated with.
**	l_offset	    offset in the record for this code element
**	l_code_len	    length of the binary machine code buffer
**	c_code		    buffer containing the machine code in binary form
**	c_text_arg	    text of machine code; optional
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      Machine code stuffed away in the source structure
**
**--
**/

void    src_append_machine_code (
		 az_src_rec, l_offset, l_code_len, c_code, c_text_arg )

src_source_record_type	*az_src_rec;
XmConst int			l_offset;
XmConst int			l_code_len;
XmConst char			*c_code;
XmConst char			*c_text_arg;

{
    src_machine_code_type   *az_code_item;
    int			    l_text_len;
    XmConst char	    *c_text;

    if (c_text_arg == NULL) {
	c_text = "";
    } else {
	c_text = c_text_arg;
    }

    /*
    **	create the machine code item and fill it in.
    */

    l_text_len = strlen( c_text ) + 1;	/* includes null */

    az_code_item = (src_machine_code_type *) _get_memory(
		sizeof( src_machine_code_type ) + l_text_len + l_code_len );

    az_code_item -> w_offset = l_offset;
    az_code_item -> w_code_len = l_code_len;
    _move( (az_code_item->c_data), c_code, l_code_len );
    _move( &(az_code_item->c_data [l_code_len]), c_text, l_text_len );

    /*
    **  Link the machine code to its source line, at the head of
    **  the machine code list.
    */

    az_code_item->az_next_machine_code = az_src_rec->az_machine_code_list;
    az_src_rec->az_machine_code_list = az_code_item;
    az_src_rec->w_machine_code_cnt++;
    
    return;
}
