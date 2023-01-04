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
static char rcsid[] = "$RCSfile: UilCmd.c,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/07 00:32:16 $"
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
**      Command line interpreter for the
**
**--
**/

/*
**
**  INCLUDE FILES
**
**/


#include "UilDefI.h"
#include "UilCmdDef.h"


/*
**
**  GLOBAL DECLARATIONS
**
**/

externaldef(uil_comp_glbl) cmd_command_line_type Uil_cmd_z_command;



#ifdef VMS
/*
**
**  VMS SPECIFIC INCLUDE FILES
**
*/

#include <climsgdef.h>
#include <descrip.h>

/*
**
**  VMS SPECIFIC EXTERNAL PROCEDURES
**
*/


status  cli$get_value
            ( struct dsc$descriptor *,      /* label to get */
              struct dsc$descriptor *,      /* value returned */
              unsigned short *);            /* length of value returned */

status  cli$get_present
            ( struct dsc$descriptor * );    /* label to see if present */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure parses the VMS command line and places the
**      results of the parse in the global structure "Uil_cmd_z_command".
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      Uil_cmd_z_command:      repository for command line info
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      Uil_cmd_z_command is set
**
**--
**/

void    cmd_decode_command_line( void )
{

    /* define static descriptors for entities to be fetched */

    static $DESCRIPTOR(dsc_source_file,    "SOURCE_FILE" );
    static $DESCRIPTOR(dsc_resource_file,  "RESOURCE_FILE" );
    static $DESCRIPTOR(dsc_listing_file,   "LISTING_FILE" );

#if debug_version
    static $DESCRIPTOR(dsc_trace_qual,     "TRACE_QUAL" );
    static $DESCRIPTOR(dsc_tokens,         "TOKENS" );
    static $DESCRIPTOR(dsc_symbols,        "SYMBOLS" );
#endif

    static $DESCRIPTOR(dsc_machine_qual,   "MACHINE_QUAL" );
    static $DESCRIPTOR(dsc_warnings_qual,  "WARNINGS_QUAL" );
    static $DESCRIPTOR(dsc_noinfo,         "NOINFORMATIONALS" );
    static $DESCRIPTOR(dsc_nowarn,         "NOWARNINGS" );
    static $DESCRIPTOR(dsc_v1,         "V1" );
    static $DESCRIPTOR(dsc_v2,         "V2" );
    static $DESCRIPTOR(dsc_wmd_file,       "WIDGET_QUAL" );
    static char     *include_list[Uil_k_max_include_dir_count] = 
                    {"uil$include:"};

    struct dsc$descriptor   dsc_value;      /* descriptor for fetched values */
    char                    c_buffer[256];  /* buffer to hold fetched values */
    unsigned short          w_buffer_len;   /* buffer length */
    status                  l_status;       /* status returned by CLI */

    /* initialize the descriptor for fetched values */

    dsc_value.dsc$b_class = DSC$K_CLASS_S;
    dsc_value.dsc$b_dtype = DSC$K_DTYPE_T;
    dsc_value.dsc$w_length = 255;
    dsc_value.dsc$a_pointer = &c_buffer;

    /* initialize the command line structure */

    Uil_cmd_z_command.ac_resource_file = NULL;
    Uil_cmd_z_command.ac_listing_file = NULL;
    Uil_cmd_z_command.include_dir_count = 1;
    Uil_cmd_z_command.ac_include_dir = include_list;
    Uil_cmd_z_command.v_resource_file = TRUE;
    Uil_cmd_z_command.v_listing_file = FALSE;
    Uil_cmd_z_command.v_show_machine_code = FALSE;
    Uil_cmd_z_command.v_parse_tree = FALSE;
    Uil_cmd_z_command.status_update_delay = 0;
    Uil_cmd_z_command.message_cb = NULL;
    Uil_cmd_z_command.status_cb = NULL;
    Uil_cmd_z_command.ac_database = NULL;
    Uil_cmd_z_command.v_database = FALSE;

#if debug_version
    uil_v_dump_tokens = FALSE;
    uil_v_dump_symbols = FALSE;
#endif

    /* get the source file */
    /* ac_source_file always holds the filename */

    cli$get_value( &dsc_source_file, &dsc_value, &w_buffer_len );

    /* make the string null terminated and make a permanent copy */

    c_buffer[ w_buffer_len ] = 0;
    Uil_cmd_z_command.ac_source_file = _get_memory( w_buffer_len + 1 );
    _move( Uil_cmd_z_command.ac_source_file, &c_buffer, w_buffer_len + 1);

    /* find out about the resource file */

    l_status = cli$present( &dsc_resource_file );

    if (l_status == CLI$_PRESENT)
    {
        /* get the resource file */

        l_status =
            cli$get_value( &dsc_resource_file, &dsc_value, &w_buffer_len );

        if (w_buffer_len > 0)
        {
            /* make the string null terminated and make a permanent copy */

            c_buffer[ w_buffer_len ] = 0;
            Uil_cmd_z_command.ac_resource_file =
                    _get_memory( w_buffer_len + 1 );
            _move( Uil_cmd_z_command.ac_resource_file,
                   &c_buffer, w_buffer_len + 1);
        }

    }

    else if (l_status == CLI$_NEGATED)
        Uil_cmd_z_command.v_resource_file = FALSE;

    /* if the resource file is Null make it "" */
    if (Uil_cmd_z_command.ac_resource_file == NULL)
        {
        Uil_cmd_z_command.ac_resource_file = XtCalloc (1, 1);
        }

    /* get the listing file */

    l_status = cli$present( &dsc_listing_file );

    if (l_status == CLI$_PRESENT)
    {
        /* get the listing file */

        Uil_cmd_z_command.v_listing_file = TRUE;
        l_status =
            cli$get_value( &dsc_listing_file, &dsc_value, &w_buffer_len );

        if (w_buffer_len > 0)
        {
            /* make the string null terminated and make a permanent copy */

            c_buffer[ w_buffer_len ] = 0;
            Uil_cmd_z_command.ac_listing_file =
                    _get_memory( w_buffer_len + 1 );
            _move( Uil_cmd_z_command.ac_listing_file,
                   &c_buffer, w_buffer_len + 1);
        }
    }
    else if (l_status == CLI$_DEFAULTED)
        Uil_cmd_z_command.v_listing_file = TRUE;

    /* get the WMD file */

    l_status = cli$present( &dsc_wmd_file );

    if (l_status == CLI$_PRESENT)
    {
        /* get the listing file */

        Uil_cmd_z_command.v_database = TRUE;
        l_status =
            cli$get_value( &dsc_wmd_file, &dsc_value, &w_buffer_len );

        if (w_buffer_len > 0)
        {
            /* make the string null terminated and make a permanent copy */

            c_buffer[ w_buffer_len ] = 0;
            Uil_cmd_z_command.ac_database =
                    _get_memory( w_buffer_len + 1 );
            _move( Uil_cmd_z_command.ac_database,
                   &c_buffer, w_buffer_len + 1);
        }
    }
    else if (l_status == CLI$_DEFAULTED)
        Uil_cmd_z_command.v_database = FALSE;

    /* look for machine qualifier */

    Uil_cmd_z_command.v_show_machine_code =
        (cli$present( &dsc_machine_qual ) == CLI$_PRESENT) &&
        Uil_cmd_z_command.v_listing_file;

#if debug_version

    /* look for symbols keyword */

    uil_v_dump_symbols =
        (cli$present( &dsc_symbols ) == CLI$_PRESENT);

    /* look for tokens keyword */

    uil_v_dump_tokens =
        (cli$present( &dsc_tokens ) == CLI$_PRESENT);
#endif

    /* look for warnings qualifier */

    switch (cli$present( &dsc_warnings_qual ))
    {
    case CLI$_PRESENT:
        Uil_cmd_z_command.v_report_info_msg =
                (cli$present( &dsc_noinfo ) == CLI$_ABSENT);
        Uil_cmd_z_command.v_report_warn_msg =
                (cli$present( &dsc_nowarn ) == CLI$_ABSENT);
        break;

    case CLI$_NEGATED:
        Uil_cmd_z_command.v_report_info_msg = FALSE;
        Uil_cmd_z_command.v_report_warn_msg = FALSE;

        if ((cli$present( &dsc_noinfo ) == CLI$_PRESENT) ||
            (cli$present( &dsc_nowarn ) == CLI$_PRESENT) )
            diag_issue_diagnostic
                ( d_subnotall, diag_k_no_source, diag_k_no_column );

        break;

    case CLI$_ABSENT:
        Uil_cmd_z_command.v_report_info_msg = TRUE;
        Uil_cmd_z_command.v_report_warn_msg = TRUE;

        break;

    default:
        _assert( FALSE, "unexpect qualifier status" );
    }


}

#else
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure parses the command line and places the
**	results of the parse in the global structure "Uil_cmd_z_command".
**
**  FORMAL PARAMETERS:
**
**      l_arg_count:	number of command arguments
**	rac_arg_value:	array of pointers to null terminated character strings
**			each of which is one of the command line arguments
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      Uil_cmd_z_command:	respository for command line info
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      Uil_cmd_z_command is set
**
**--
**/
void	cmd_decode_command_line( l_arg_count, rac_arg_value )
int 	l_arg_count;
char 	*rac_arg_value[ ];

{
    static char	    *include_list	[Uil_k_max_include_dir_count];
    int	i;

    Uil_cmd_z_command.ac_source_file = NULL;
    Uil_cmd_z_command.ac_resource_file = NULL;
    Uil_cmd_z_command.ac_listing_file = NULL;
    Uil_cmd_z_command.include_dir_count = 0;
    Uil_cmd_z_command.ac_include_dir = (char **)include_list;
    Uil_cmd_z_command.v_resource_file = TRUE;
    Uil_cmd_z_command.v_listing_file = FALSE;
    Uil_cmd_z_command.v_show_machine_code = FALSE;
    Uil_cmd_z_command.v_report_info_msg = TRUE;
    Uil_cmd_z_command.v_report_warn_msg = TRUE;
    Uil_cmd_z_command.v_parse_tree = FALSE;
    Uil_cmd_z_command.v_use_setlocale = FALSE;
    Uil_cmd_z_command.status_update_delay = 0;
    Uil_cmd_z_command.message_cb = (Uil_continue_type(*)())NULL;
    Uil_cmd_z_command.status_cb = (Uil_continue_type(*)())NULL;
    Uil_cmd_z_command.ac_database = NULL;
    Uil_cmd_z_command.v_database = FALSE;

#if debug_version
    uil_v_dump_tokens = FALSE;
    uil_v_dump_symbols = FALSE;
#endif

    /* traverse the options on the command line */

    for (i = 1;  i < l_arg_count;  i++)
    {
	/* check for an output file  */

	if ( strcmp("-o", rac_arg_value[ i ]) == 0 )
	{
	    /* the next argument is the output file name  */

	    /* check next field is not an option */

            if (((i+1) >= l_arg_count) ||
                ( '-' == rac_arg_value[ i+1 ][ 0 ] ))
	    {
		diag_issue_diagnostic
			( d_miss_opt_arg, 
			  diag_k_no_source, diag_k_no_column,
			  rac_arg_value[ i ],
			  "output file"
			);
		continue;
	    }

	    if (Uil_cmd_z_command.ac_resource_file == NULL)
	        Uil_cmd_z_command.ac_resource_file = rac_arg_value[ i+1 ];
	    else
		diag_issue_diagnostic
		    ( d_dupl_opt, 
		      diag_k_no_source, diag_k_no_column,
		      rac_arg_value[ i ]
		    );
	    i = i + 1;
	}
	    
	/* check for a binary database file */

	else if ( strcmp("-wmd", rac_arg_value[ i ]) == 0 )
	{
	    /* the next argument is the binary database file name  */

	    /* check next field is not an option */

            if (((i+1) >= l_arg_count) ||
                ( '-' == rac_arg_value[ i+1 ][ 0 ] ))
	    {
		diag_issue_diagnostic
			( d_miss_opt_arg, 
			  diag_k_no_source, diag_k_no_column,
			  rac_arg_value[ i ],
			  "binary database file"
			);
		continue;
	    }

	    if (!Uil_cmd_z_command.v_database)
	    {
	        Uil_cmd_z_command.v_database = TRUE;
	        Uil_cmd_z_command.ac_database = rac_arg_value[ i+1 ];
	    }
	    else
		diag_issue_diagnostic
		    ( d_dupl_opt, 
		      diag_k_no_source, diag_k_no_column,
		      rac_arg_value[ i ]
		    );
	    i = i + 1;
	}

	/* check for an listing file */

	else if ( strcmp("-v", rac_arg_value[ i ]) == 0 )
	{
	    /* the next argument is the listing file name  */

	    /* check next field is not an option */

            if (((i+1) >= l_arg_count) ||
                ( '-' == rac_arg_value[ i+1 ][ 0 ] ))
	    {
		diag_issue_diagnostic
			( d_miss_opt_arg, 
			  diag_k_no_source, diag_k_no_column,
			  rac_arg_value[ i ],
			  "listing file"
			);
		continue;
	    }

	    if (!Uil_cmd_z_command.v_listing_file)
	    {
	        Uil_cmd_z_command.v_listing_file = TRUE;
	        Uil_cmd_z_command.ac_listing_file = rac_arg_value[ i+1 ];
	    }
	    else
		diag_issue_diagnostic
		    ( d_dupl_opt, 
		      diag_k_no_source, diag_k_no_column,
		      rac_arg_value[ i ]
		    );
	    i = i + 1;
	}
	    
	/* check for the machine code option */

	else if ( strcmp("-m", rac_arg_value[ i ]) == 0 )
	{
	        Uil_cmd_z_command.v_show_machine_code = TRUE;
	}
	    
	/* check if warnings are to be supressed */

	else if ( strcmp("-w", rac_arg_value[ i ]) == 0 )
	{
	        Uil_cmd_z_command.v_report_info_msg = FALSE;
	        Uil_cmd_z_command.v_report_warn_msg = FALSE;
	}

	/* check if setlocale is to be enabled */

	else if ( strcmp("-s", rac_arg_value[ i ]) == 0 )
	{
	  Uil_cmd_z_command.v_use_setlocale = TRUE;
	}
	
	/* check for an unexpected option */

	else if ( '-' == rac_arg_value[ i ][ 0 ] )
	{

	/* check for an include directory */

	    if ( 'I' == rac_arg_value[ i ][ 1 ] )
	    {
		if (Uil_cmd_z_command.include_dir_count < Uil_k_max_include_dir_count)
		    
		    include_list[Uil_cmd_z_command.include_dir_count++] = 
			& rac_arg_value[i] [2];
		else
		    diag_issue_diagnostic
		        ( d_too_many_dirs, 
		          diag_k_no_source, diag_k_no_column,
		          rac_arg_value[ i ], Uil_k_max_include_dir_count
		        );
	    } else
	    {
		diag_issue_diagnostic
		    ( d_unknown_opt, 
		      diag_k_no_source, diag_k_no_column,
		      rac_arg_value[ i ]
		    );
	    }
	}

	/* assume it is a UIL source file specification
	 * validation of the file spec is done when file is opened */

	else
	{
	    if (Uil_cmd_z_command.ac_source_file == NULL)
	       Uil_cmd_z_command.ac_source_file = rac_arg_value[ i ];
	    else
		diag_issue_diagnostic
		    ( d_add_source, 
		      diag_k_no_source, diag_k_no_column,
		      rac_arg_value[ i ]
		    );
	}
    }

    /*
    **	Check for a source file - otherwise issue a diagnostic.
    */

    if (Uil_cmd_z_command.ac_source_file == NULL)
	diag_issue_diagnostic
	    ( d_no_source, diag_k_no_source, diag_k_no_column );

    if (Uil_cmd_z_command.ac_resource_file == NULL)
	{
        Uil_cmd_z_command.ac_resource_file = XtMalloc (strlen ("a.uid") + 1);
        strcpy (Uil_cmd_z_command.ac_resource_file,"a.uid");
        }

    /*
    **	Machine code listing only makes sense if listing is set.
    */
    
    Uil_cmd_z_command.v_show_machine_code =
        ( Uil_cmd_z_command.v_listing_file & 
	  Uil_cmd_z_command.v_show_machine_code);
}

#endif  /* VMS */


