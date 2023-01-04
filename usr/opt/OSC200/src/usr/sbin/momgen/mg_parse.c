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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: mg_parse.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/07/13 22:32:32 $";
#endif
/*
**+
**  Copyright (c) Digital Equipment Corporation, 1992
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
**++
**  FACILITY:  Ultrix/OSF Common Agent MOM Generator
**
**  MODULE DESCRIPTION:
**
**	This module contains the routines to parse the MOM Generator command 
**	line and fill in the MOM and CLASS structures where appropriate.
**	This file was created from the VMS MOM Generator CLI Module.
**
**  AUTHOR:
**
**      Mike Densmore
**
**  CREATION DATE:  18-Mar-1992
**
**  MODIFICATION HISTORY:
**
**	- NOTE: the following temporary fixes need to corrected!!!!
**	    1. error statuses are mucked up (see mom_defs.h)
**	    2. entity (an array for oid) is declared static because MIPS
**	       can't handle the way it is currently declared otherwise
**
**	Mike Densmore	3-Jun-1992  Removed routines that were moved to
**				    mg_common.c:
**					- add_oid_class() (now add_class and
**					    add_oid)
**					- read_info_file()
**
**      Adam Peller     13-Aug-1992  Redesigned for getopt()
**
**	Mike Densmore	25-Sep-1992  Modified for ANSI Standard C
**
**      M. Ashraf       10-Dec-1992  If mom->log_file not specified, send to
**                                   stderr
**
**      M. Ashraf       19-May-1993  Set mom name in mom->mom_target based on param
**                                   file name + "_mom"
**
**  ASSUMPTIONS:
**
**	- The parsing is hardcoded.  This is based on the need to turn
**	  around the V1.0 product on a short schedule.  The assumption
**	  is that the user interface will not change much until V2.
**--
*/

#include "mom_defs.h"
#include <string.h>
#include "iso_defs.h"
#include "mg_prototypes.h"

#define TRUE 1
#define FALSE 0

#define MIR_STRING "mir"
#define MCC_STRING "mcc"

#define MAX_QUALIFIER_SIZE 32
				/* Max filename size */
#define MAX_FILEN_SIZE 200
#if defined(sun) || defined(sparc)
int getopt ();
#endif

char *construct_mom_name();


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	get_qual
**
**	This routine parses the arguments passed from the invocation of
**	MOM Generator in argv.  It puts the values of the parameters
**	specified by the arguments in the MOM structure.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	argv		Array of pointers to input line arguments.
**	argc		Number of input line arguments.
**
**  RETURN VALUE:
**
**      SS$_NORMAL		- Normal successful completion.
**	MOMGEN_C_INSVIRMEM 	- Insufficient Virtual Memory
**	MOMGEN_C_ABSENT 	- Qualifier absent
**	MOMGEN_C_VALUE_REQUIRED - Qualifier value required
**--
*/
int get_qual(argc, argv, mom)                                              

int argc;
char *argv[];
MOM_BUILD *mom;

{

char	    buf[ MAX_QUALIFIER_SIZE ];	/* buffer for option strings */
char        *optstring;  
extern char *optarg;
extern int  optind, opterr;
int         c;
int         i;
int         stat;
int         usage_error = FALSE;
int         bool;
int	    function_default = TRUE;	/* flag to indicate if function argument found */

    opterr = 0;

    while ((c = getopt(argc, argv, "aAcCd:Df:ghi:Il:mn:s:St:uv")) != EOF)
      switch (c) 
      {

	    case 'i': /* info file */
	      mom->info_file = (char *) malloc (strlen (optarg) + 1);
	      strcpy( mom->info_file, optarg );
	      break;

	    case 'f': /* function file */  /*******NOT IMPLEMENTED*******/
	      function_default = FALSE;
	      break;

	    case 'd': /* dictionary location */
	      mom->mir_file = (char *) malloc (strlen (optarg) + 1);
	      strcpy( mom->mir_file, optarg );
	      break;

	    case 'l': /* log file */
	      mom->log_file_str = (char *) malloc ( strlen(optarg) + 1);
	      strcpy ( mom->log_file_str, optarg );
	      mom->log = TRUE;
	      mom->log_file = fopen( mom->log_file_str, "w" );
	      if ( mom->log_file == NULL )
	      {
		fprintf( stderr, "%s: Can't write to output file: %s\n",
			argv[0], mom->log_file_str);
		return MOMGEN_C_OPENWRITEERROR;
	      }
	      break;
	      
	    case 't': /* template directory */
	      i = strlen(optarg);
	      mom->mom_in = (char *) malloc ( i + 2 );
	      strcpy ( mom->mom_in, optarg );
	      if (mom->mom_in[i - 1] != '/' ) /* if there isn't a slash at */
	      {                               /* the end, append one */
		mom->mom_in[i]='/';
		mom->mom_in[i + 1]='\0';
	      }
	      break;
	      
	    case 'a':    /*  moi_invoke_action entry point */
	      mom->build_action = TRUE;
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    case 'A':    /* access routine */
	      mom->build_access = TRUE;
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    case 'c':    /* moi_create */
	      mom->build_create = TRUE;
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    case 'C':    /* moi_cancel_get entry point */
	      mom->build_cancel_get = TRUE;
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    case 'D':    /* moi_directive */
	      mom->build_directive = TRUE;
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    case 'g':    /* moi_get_instance entry point */
	      mom->build_get = TRUE;
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    case 'h':    /* common.h */
	      mom->build_common = TRUE;
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    case 'I':    /* moi_init entry point */
	      mom->build_init = TRUE;
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    case 'm':    /* make file */
	      mom->build_makefile = TRUE;
/*	      mom->build_descrip = TRUE;  what do we do about
	                                  .mms functionality?  Took it out
					  for now. */
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    case 'v':	/* VMS MMS, Options and COM files */
	      mom->build_descrip = TRUE;
	      mom->build_command_file = TRUE;
	      mom->build_options = TRUE;
	      function_default = FALSE;
	      break;

	    case 'n':    /* no support code */
	    case 's':    /* support code */
	      bool = (c == 's') ? TRUE : FALSE ;
	      function_default = FALSE;  /* function set...don't use default */
	      i = 0;
	      while (optarg[i] != '\0')
	      {
	       switch (optarg[i++]) {
	       case 'a':	/* moi_invoke_action */
		 mom->support_action = bool;
		 break;

	       case 'c':	/* moi_create_instance */
		 mom->support_create = bool;
		 break;

	       case 'C':	/* moi_cancel_get */
		 mom->support_cancel_get = bool;
		 break;

	       case 'd':	/* moi_delete_instance */
		 mom->support_delete = bool;
		 break;

	       case 'g':	/* moi_get_attributes */
		 mom->support_get = bool;
		 break;

	       case 's':	/* moi_set_attributes */
		 mom->support_set = bool;
		 break;

	       default:		/* illegal option */
		usage_error = TRUE;
		break;

	       } /* end switch */

	      } /* end while */
	      break;

	    case 'S':    /* moi_set_instance */
	      mom->build_set = TRUE;
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    case 'u':    /* utility file */
	      mom->build_util = TRUE;
	      function_default = FALSE;  /* function set...don't use default */
	      break;

	    default:
	    case '?':
	      fprintf( stderr, "%s: Illegal option.", argv[0] );
	      usage_error = TRUE;
	      break;
	      
      }

/* Look for destination of generated mom source code.  MANDATORY PARAMETER!! */
    if (optind == argc - 1)
    {
      i = strlen(argv[optind]);
      mom->mom_out_src = (char *) malloc( strlen (argv[optind]) + 2 );
      strcpy( mom->mom_out_src, argv[optind]);
      if (mom->mom_out_src[i - 1] != '/' ) /* if there isn't a slash at */
      {                                    /* the end, append one */
	mom->mom_out_src[i]='/';
        mom->mom_out_src[i + 1]='\0';
      }
      mom->mom_out_build = (char *) malloc( strlen( mom->mom_out_src ) +1);
      strcpy( mom->mom_out_build, mom->mom_out_src );
    }

/* someday, hopefully, the info_file will become optional. */
/* right now, make sure info file and output directory was specified.
   If not, print usage guide and return error status. */
    if ((mom->info_file == NULL) || (mom->mom_out_src == NULL) || usage_error)
    {
      if (!mom->info_file)
	fprintf( stderr, "%s: infofile not specified!\n\n", argv[0] );
      else if (!mom->mom_out_src)
	fprintf( stderr, "%s: output directory not specified!\n\n", argv[0] );
      fprintf( stderr,
	      "Usage: %s -i paramfile [options] output-directory\n", argv[0] );
      fprintf( stderr,
	      "  options: -t template-directory -l logfile -d dictionary\n");
      fprintf( stderr, "-a\tgenerate moi_invoke_action entry point\n");
      fprintf( stderr, "-A\tgenerate access routine entry point\n");
      fprintf( stderr, "-c\tgenerate moi_create_instance entry point\n");
      fprintf( stderr, "-C\tgenerate moi_cancel_get entry point\n");
      fprintf( stderr, "-D\tgenerate moi_directive entry point\n");
      fprintf( stderr, "-g\tgenerate moi_get_attributes entry point\n");
      fprintf( stderr, "-h\tgenerate common.h file\n");
      fprintf( stderr, "-I\tgenerate moi_init entry point\n");
      fprintf( stderr, "-m\tgenerate make file\n");
      fprintf( stderr, "-v\tgenerate VMS MMS related files\n");
      fprintf( stderr, "-n\tgenerate code to return MAN_C_DIRECTIVE_NOT_SUPPORTED for\n");
      fprintf( stderr, "\tthe specified directives(s):\n");
      fprintf( stderr, "\t\ta - moi_invoke_action\n");
      fprintf( stderr, "\t\tc - moi_create_instance\n");
      fprintf( stderr, "\t\tC - moi_cancel_get\n");
      fprintf( stderr, "\t\td - moi_delete_instance\n");
      fprintf( stderr, "\t\tg - moi_get_attributes\n");
      fprintf( stderr, "\t\ts - moi_set_attributes\n");
      fprintf( stderr, "-s\tgenerate code to perform the specified directive(s) (see -n)\n");
      fprintf( stderr, "-S\tgenerate moi_set_attributes entry point\n");
      fprintf( stderr, "-u\tgenerate utility file\n");
      return MOMGEN_C_ABSENT;
    }

    mom->multiclass = TRUE;

    stat = read_info_file( mom );
    if (error_condition( stat ))
	return stat;

    /* Build target mom name from parameter file name */
    mom->mom_target = construct_mom_name( mom->info_file , mom->mom_target );

#ifdef UNIXPLATFORM
 /*  If log file not specified, then use stderr to send error messages */
     if (mom->log_file == NULL)
         mom->log_file = stderr;
#endif /* UNIXPLATFORM */

    if (function_default == TRUE) {
    	mom->support_get = TRUE;
    	mom->support_set = TRUE;
	mom->build_create = TRUE;
	mom->build_delete = TRUE;
	mom->build_access = TRUE;
    	mom->build_directive = TRUE;
    	mom->build_get = TRUE;
    	mom->build_init = TRUE;
    	mom->build_oids = TRUE;
    	mom->build_set = TRUE;
    	mom->build_util = TRUE;
	mom->build_get_candidate = TRUE;
	mom->build_perform_init = TRUE;
	mom->build_find = TRUE;
	mom->build_prototypes = TRUE;
    	mom->build_common = TRUE;
    	mom->build_makefile = TRUE;

	if (!mom->OID_SNMP) {
	    mom->build_cancel_get = TRUE;
	    mom->support_action = TRUE;
	    mom->support_create = TRUE;
	    mom->support_delete = TRUE;
	    mom->build_action = TRUE;
	    mom->build_cancel_get = TRUE;
	    }
	else {
	    mom->build_getnext = TRUE;
	    mom->support_getnext = TRUE;
	}
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      construct_mom_name
**
**      This routine parses the file specification in the general form,
**      "<device>:<directory>/<file-name>.<ext>", and returns the 
**      <file-name> portion of the specification.
**
**  FORMAL PARAMETERS:
**
**      file_spec		File specification to be parsed
**      concat_str		String to be concatenated to file name
**
**  RETURN VALUE:
**
**      file-name		Pointer to string containing file name
**
**  SIDE EFFECTS:
**
**	It assumes that the file specification string parameter is
**	syntactically correct as this routine is called only if the file has
**	been previously opened successfully.
**--
*/
char *construct_mom_name (char *file_spec, char *concat_str)
{
  static  char filename[MAX_FILEN_SIZE];
  char *ptr, *base_ptr;

            /*  Point to ".<ext>" */
            if ((ptr = (char*)strrchr(file_spec, '.')) == NULL)
              ptr = file_spec + strlen(file_spec);

            /*  Point to "<file-name>" */
            if (base_ptr = (char*)strrchr(file_spec, '/'))
              base_ptr++;
            else
              base_ptr = file_spec;

            strncpy (filename, base_ptr, (ptr - base_ptr));
            filename[(ptr - base_ptr)] = 0;
            strcat( filename , "_");
            strcat( filename , concat_str);
            return (filename);              /* Return newly created filename */

} /* End of construct_mom_name() */

/* End of file mg_parse.c */
