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
 * @(#)$RCSfile: cma_assert.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/08/18 14:43:19 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for CMA Assertions package
 *
 *  AUTHORS:
 *
 *	R. Conti
 *
 *  CREATION DATE:
 *
 *	3 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	3 November 1989
 *		Changed type of cma__assert_message to cma_t_integer so it
 *		could be used in macros which expand to expressions instead of
 *		blocks.
 *	002	Paul Curtin	6 August 1990
 *		Replace printf w/ internal cma__put* functions.
 *		Add include for cma_util.h
 *	003	Dave Butenhof	14 December 1990
 *		Change cma_assertions.h to cma_assert.h (shorten length)
 *	004	Dave Butenhof	11 June 1991
 *		Change cma__assert_message() to call formatting functions for
 *		kernel and semaphore tracing if the appropriate conditionals
 *		are set.
 *	005	Dave Butenhof	30 July 1991
 *		Restore *printf functions, now that we emulate them on VMS to
 *		keep the VAX C rtl safely out of the picture.
 *	006	Paul Curtin	20 August 1991
 *		Conditionalized out the include of stdio.h on VMS.
 *	007	Dave Butenhof	24 March 1992
 *		Report assertion failure by a bugcheck, rather than by
 *		raising an exception that just obscures the error state.
 *	008	Webb Scales	20 May 1992
 *		Put assertion message text in the bugcheck message so that
 *		it is available in the log file as well.
 *	009	Dave Butenhof	14 October 1992
 *		Flush output after assert message so it'll come out before a
 *		later bugcheck.
 *	010	Brian Keane	1 July 1993
 *		Minor touch-ups to eliminate warnings with DEC C on OpenVMS AXP.
 */

/*
 *  INCLUDE FILES
 */

#include <cma_assert.h>
#include <cma_errors.h>
#include <cma_util.h>
#if _CMA_OS_ == _CMA__UNIX
# include <stdio.h>
#endif

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
cma__assert_message
#ifdef _CMA_PROTO_
	(
	cma_t_boolean	is_failure,	/* Was it a fail or a warn? */
	cma__t_string	description,	/* English description of problem */
	cma__t_string	file,		/* File name */
	cma_t_integer	line)		/* Line number */
#else	/* no prototypes */
	(is_failure, description, file, line)
	cma_t_boolean	is_failure;	/* Was it a fail or a warn? */
	cma__t_string	description;	/* English description of problem */
	cma__t_string	file;		/* File name */
	cma_t_integer	line;		/* Line number */
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
	 */
	failure_count++;

	if (failure_count <= failure_limit) {
	    cma__int_printf ("**** Assertion failure. %s\n",description);
	    cma__int_printf ("     at line %d in %s \n", line, file);
#if _CMA_OS_ == _CMA__UNIX
	    fflush (stdout);
#endif
	    };

	if (failure_count >= failure_limit)
	    cma__bugcheck (
		    "assertion failure:  %s\n\tat line %d in %s",
		    description,
		    line, 
		    file);

	}
    else {

	/* 
	 * Process a warning assertion.
	 *
	 * Increment the warning count.  Print out the error message
	 * up to the limit on the number of messages to be displayed.  
	 * When the limit is met or exceeded, then simply stop 
	 * displaying warning messages.
	 */
	warning_count++;

	if (warning_count <= warning_limit) {
	    cma__int_printf ("**** Assertion warning. %s\n",description);
	    cma__int_printf ("     at line %d in %s \n", line, file);
#if _CMA_OS_ == _CMA__UNIX
	    fflush (stdout);
#endif
	    };

	if (warning_count >= warning_limit)
	    cma__bugcheck ("assertion warning overflow (%d)", warning_limit);

	}    

    return 0;

    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ASSERT.C */
/*  *10    2-JUL-1993 09:42:51 KEANE "Fix DEC C warnings: add return statements to non-void routines" */
/*  *9    14-OCT-1992 12:10:10 BUTENHOF "flush assert messages" */
/*  *8    20-MAY-1992 16:52:08 SCALES "Put assertion info in the bugcheck message" */
/*  *7    24-MAR-1992 14:47:05 BUTENHOF "Report assertion failure with bugcheck" */
/*  *6    21-AUG-1991 16:42:12 CURTIN "Removed VMS include of stdio.h" */
/*  *5    31-JUL-1991 18:40:06 BUTENHOF "Drop cma__put* functions" */
/*  *4    11-JUN-1991 17:16:30 BUTENHOF "Add & use functions to dump kernel/sem trace arrays" */
/*  *3    10-JUN-1991 18:16:51 SCALES "Add sccs headers for Ultrix" */
/*  *2    14-DEC-1990 00:54:55 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:41:11 BUTENHOF "Assertions" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ASSERT.C */
