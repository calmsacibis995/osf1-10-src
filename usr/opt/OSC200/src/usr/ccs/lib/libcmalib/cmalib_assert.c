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
static char *rcsid = "@(#)$RCSfile: cmalib_assert.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:58:22 $";
#endif
/*
 *  Copyright (c) 1990, 1992 by
 *  Digital Equipment Corporation, Maynard Massachusetts.
 *  All rights reserved.
 *
 *  This software is furnished under a license and may be used and  copied
 *  only  in  accordance  with  the  terms  of  such  license and with the
 *  inclusion of the above copyright notice.  This software or  any  other
 *  copies  thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of  the  software  is  hereby
 *  transferred.
 *
 *  The information in this software is subject to change  without  notice
 *  and  should  not  be  construed  as  a commitment by DIGITAL Equipment
 *  Corporation.
 *
 *  DIGITAL assumes no responsibility for the use or  reliability  of  its
 *  software on equipment which is not supplied by DIGITAL.
 */

/*
 *  FACILITY:
 *
 *	CMA Library services
 *
 *  ABSTRACT:
 *
 *	CMA Library Assertions package  (Copied directly from CMA_ASSERTIONS.C)
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	8 August 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 */


/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cmalib_defs.h>
#include <cmalib_assert.h>
#include <stdio.h>

/*
 * LOCAL DATA
 */

static int 
    failure_count = 0,  /* Initially there are no failures */
    failure_limit = 1,  /* Limit on failure messages before program is 
			   terminated */
    warning_count = 0,  /* Initially there are no warnings */
    warning_limit = 50; /* Limit on warning messages before program is
			   terminated */

/*
 * LOCAL FUNCTIONS
 */



/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine is called when a warning assertion or failure
 * 	assertion triggers.
 *
 * 	For either kind, a count is incremented and if the count
 * 	does not exceed, or meets, a limit for that kind of assertion, an
 * 	error message is printed.  Then, if the limit on the number
 * 	of messages is met, or exceeded, a hard error is reported that
 * 	should result in program termination.
 *
 *  FORMAL PARAMETERS:
 *
 *	is_failure,	
 * 		TRUE if the assertion that triggered is a failure assertion
 * 		FALSE if the assertion that triggered is a warning assertion
 *
 *      description
 * 		Address of a string describing the problem in english.
 *
 *      file
 *		Name of the file where the assertion is located.
 *
 *      line	
 *		Line number where the assertion is located.
 *
 *  IMPLICIT INPUTS:
 *
 *	NONE
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Output is generated that describes the assertion that failed.
 *
 *  FUNCTION VALUE:
 *
 *	NONE
 *
 *  SIDE EFFECTS:
 *
 *	The program is terminated if the assertion limit is reached.
 */

cma_t_integer
cma_lib__assert_message
#ifdef _CMA_PROTO_
	(
	cma_t_boolean	    is_failure,	    /* Was it a fail or a warn? */
	cma_lib__t_string   description,    /* English description of problem */
	cma_lib__t_string   file,	    /* File name */
	cma_t_integer	    line)	    /* Line number */
#else	/* no prototypes */
	(is_failure, description, file, line)
	cma_t_boolean	    is_failure;	    /* Was it a fail or a warn? */
	cma_lib__t_string   description;    /* English description of problem */
	cma_lib__t_string   file;	    /* File name */
	cma_t_integer	    line;	    /* Line number */
#endif	/* prototype */
    {

    if (is_failure) {

	/* 
	 * Process a failure assertion.
	 *
	 * Increment the failure count.  Print out the error message
	 * up to the limit on the number of messages to be displayed.  
	 * When the limit is met or exceeded, then attempt to blow away 
	 * the program by reporting a hard error to the current
	 * thread.  
	 *
	 * FIX-ME: We may want to lock a mutex when incrementing this count.
	 */
	failure_count++;

	if (failure_count <= failure_limit) {
	    printf ("**** Assertion failure. %s\n",description);
	    printf ("     at line %d in %s \n", line, file);
	    };
	if (failure_count >= failure_limit)
	    RAISE (cma_e_assertion);

	}
    else {

	/* 
	 * Process a warning assertion.
	 *
	 * Increment the warning count.  Print out the error message
	 * up to the limit on the number of messages to be displayed.  
	 * When the limit is met or exceeded, then simply stop 
	 * displaying warning messages.
	 *
	 * FIX-ME: We may want to lock a mutex when incrementing this count.
	 */
	warning_count++;

	if (warning_count <= warning_limit) {
	    printf ("**** Assertion warning. %s\n",description);
	    printf ("     at line %d in %s \n", line, file);
	    };
	if (warning_count >= warning_limit)
	    RAISE (cma_e_assertion);
	}    
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_ASSERT.C */
/*  *2     1-JUN-1992 14:39:31 BUTENHOF "Modify for new build environment" */
/*  *1    27-AUG-1990 02:14:46 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_ASSERT.C */
