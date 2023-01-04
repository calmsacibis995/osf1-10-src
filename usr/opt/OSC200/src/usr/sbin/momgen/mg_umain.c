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
static char *rcsid = "@(#)$RCSfile: mg_umain.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/10/08 16:13:56 $";
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
**
**  FACILITY: Common Agent MOM Generator
**
**  MODULE DESCRIPTION:
**
**      Main routine for Common Agent MOM Generator.  Sets up defaults
**	for some variables.  Parses any arguments entered by user when
**	MOM Generator started.  Calls the build process to build the
**	skeleton and MAKE files.
**
**	NOTE: This module is specific to UNIX and cannot be ported to
**	other platforms directly.
**
**  AUTHORS:
**
**      Mike Densmore 
**
**  CREATION DATE:  10-Mar-1992
**
**  MODIFICATION HISTORY:
**
**	1-Jun-1992  Mike Densmore   added define to conditionally compile
**				    MCC support
**
**	3-Jun-1992  Mike Densmore   added defaults for mom_in, mom_out_src
**				    and mom_out_build
**
**	25-Sep-1992 Mike Densmore   modifications to support ANSI C standard
**
**       2-Dec-1992 M. Ashraf       currdate malloc'ed memory, overwrote ptr,
**                                  and free'd it, causing fault.  Fixed. 
**
**  CONTEXT:
**
**	Main is called on start up.  Arguments entered by user are
**	passed in argv.
**
**	Legal arguments:
**		info file
**		-d	specifies dictionary
**		-l	produce log file
**		-f	specifies function file
**		-...	specific build options (refer to manual)
**	Refer to user manual for details on arguments.
**
**	Routines Supplied:
**		main
**		get_defaults
**
**	Routines Required:
**		get_qual	[mg_parse.c]
**		get_MIR_defs	[mg_mir.c]
**		get_MCC_defs	[mg_mcc.c]
**		build_mom	[mg_function.c]
**--
*/

#ifdef NL
#include <langinfo.h>
#include <locale.h>
#endif

#include <time.h>
#include <ctype.h>
#include <string.h>
#include "mom_defs.h"
#include "mg_prototypes.h"


#define TRUE 1
#define FALSE 0

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	get_defaults 
**
**	This routine gets the current date/time and sets up other defaults.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
int get_defaults( mom )
MOM_BUILD *mom;
{
int status;

time_t	    *currtime;
struct	tm  *currdate;
int	    datesize;

/* 
 * Set up some defaults
 */

/*
 *  preset all functions to FALSE; preset output to FALSE; preset dictionary and file pointers
 */
    mom->info_file = NULL;
    mom->mir_file = NULL;
    mom->MCC_dictionary = FALSE;
    mom->log = FALSE;
    mom->build_all = FALSE;
    mom->build_shut = FALSE;
    mom->build_cancel_get = FALSE;
    mom->support_action = FALSE;
    mom->support_create = FALSE;
    mom->support_delete = FALSE;
    mom->support_get = FALSE;
    mom->support_set = FALSE;
    mom->build_access = FALSE;
    mom->build_action = FALSE;
    mom->build_cancel_get = FALSE;
    mom->build_command_file = FALSE;
    mom->build_common = FALSE;
    mom->build_create = FALSE;
    mom->build_delete = FALSE;
    mom->build_descrip = FALSE;
    mom->build_makefile = FALSE;
    mom->build_directive = FALSE;
    mom->build_get = FALSE;
    mom->build_getnext  = FALSE;
    mom->build_init = FALSE;
    mom->build_oids = FALSE;
    mom->build_options = FALSE;
    mom->build_set = FALSE;
    mom->build_shut = FALSE;
    mom->build_util = FALSE;
    mom->build_prototypes = FALSE;
    mom->support_instances = TRUE;
    mom->extern_common = "#include \"extern_common.h\"";
    mom->mom_in = MOM_IN_DEFAULT;
    mom->mom_out_src = NULL;
    mom->system	= "";
    mom->organization = "EMF";
    mom->copyright_owner = "Digital Equipment Corporation" ;
    mom->facility = "Common Agent";
    mom->prefix = "";
    mom->spaces = " ";
    mom->num_threads = 1;
    mom->mom_target = "mom";

    /* get date and set up strings for creation date and copyright date */

    currtime = (time_t *)malloc(sizeof(time_t));    /* allocate memory for time data */

    mom->creation_date = (char *)malloc(20);	/* string for creation date (dd-mmm-yyyy) */
    mom->copyright_date = (char *)malloc(6);	/* string for copyright date (19xx) */

    time(currtime);		    /* get the current time */
    currdate = localtime(currtime); /* convert to tm structure */
		    /* format date into proper forms for mom */
    datesize = 15;
    strftime (mom->creation_date, datesize, "%d-%b-%Y", currdate);
    datesize = 5;
    strftime (mom->copyright_date, datesize, "%Y", currdate);

    free (currtime);	/* free up memory */

if (getenv("EDA_MOMGEN_USESYM") != 0)
    mom->use_symbols = 1;

if (getenv("EDA_MOMGEN_DEBUG") != 0)
    mom->debug = 1;

if (getenv("EDA_MOMGEN_DEBUGATTR") != 0)
    mom->attr = 1;

return SS$_NORMAL;
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	main
**
**	This routine is the main program of the MOM Generator.
**
**  FORMAL PARAMETERS:
**
**      argc - number of arguments from user
**	argv - array of pointers to argument strings 
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**      or any error condition generated by the subroutines.
**--
*/
main(argc, argv)

int argc;
char *argv[];

{
    int stat,i;
    MOM_BUILD	mom;
    CLASS_DEF	*classes,*temp;

#ifdef NL
/* set up for internationlized text */

    _m_catd = catopen("ca_momgen.cat",NL_CAT_LOCALE);
    setlocale(LC_ALL,"");
#endif

/*
 * Check for CA Developers license
 */
    if ((ca_check_developer_license()) != 0) {
      printf ("\nLicense for Common Agent Developer's Toolkit (COM-AGNT-DEV) is not loaded.\n");
      exit (1);
    }

    memset(&mom,'\0',sizeof(MOM_BUILD));

    /*
     * Get the current time and set up some MOM prefix default
     */

    stat = get_defaults( &mom );
    if (error_condition( stat ))
	return stat;

    /*
     * Pass the user input args to the parser routine. 
     */

    stat = get_qual(argc, argv, &mom );
    if (error_condition( stat ))
	return stat;

    /* 
     * Read the attributes from the appropriate dictionary for each class.
     */

    temp = mom.class;
    for (i=0; i < mom.num_classes; i++)
        {
	if (mom.MCC_dictionary)
#ifdef MCCDICT
	  stat = get_MCC_defs( temp, &mom );
#else
	  {
	    fprintf (stderr, 
	     "%s: Use of the DECmcc Dictionary not supported.\n", argv[0]);
	    exit (-1);
	  }
#endif
	else
	    stat = get_MIR_defs( temp, &mom );
	
        if ((stat & 1) == 0)
	{
	    fprintf (stderr,
		     "%s: Error opening MIR file\n", argv[0]);
	    exit(stat);
        }
	temp = temp->next;
        }

    /* 
     * Generate the code.
     */

    stat = build_mom(&mom);
    return (stat == SS$_NORMAL) ? 0 : stat; /* return 0 on success to make
					       Ultrix shells happy */
}

/*********************
 * the following stub must be added when the mom generator is not being
 * compiled for use with the mcc dictionary...
 *********************/

#ifndef MCCDICT
int add_aes( newclass, mom )

CLASS_DEF *newclass;
MOM_BUILD *mom;

{ return 1;}
#endif
