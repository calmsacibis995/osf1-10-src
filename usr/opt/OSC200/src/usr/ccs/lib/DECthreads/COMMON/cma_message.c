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
 * @(#)$RCSfile: cma_message.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/13 21:32:26 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads services
 *
 *  ABSTRACT:
 *
 *	Provide access to the message catalog functions (and fallback
 * 	messages).
 *
 *  AUTHORS:
 *
 *	Jerry Harrow
 *	Dave Butenhof
 *
 *	(Adapted from OSF DCE source module dce_error.c)
 *
 *  CREATION DATE:
 *
 *	19 November 1991
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	20 November 1991
 *		Add extra level of fallback for cat file.
 *	002	Dave Butenhof	 9 January 1992
 *		Remove "Exception: " text from builtin messages
 *	003	Dave Butenhof	29 October 1992
 *		Clean up the fallback code to use a list; make sure it'll
 *		work if NLSPATH is defined with or withing the ".cat" suffix.
 *	004	Dave Butenhof	11 December 1992
 *		Check use of fallbacks if catopen() returns spurious success
 *		(it no longer attempts to open catalog, deferring that to
 *		catgets()).
 *	005	Dave Butenhof	22 March 1993
 *		Add argument to fallback for address exception message.
 *	006	Dave Butenhof	30 March 1993
 *		Replace "%08x" formatting with "0x%lx" for 64-bit safety.
 */

/*
 * Copyright (c) 1991 by
 *     Hewlett-Packard Company, Palo Alto, Ca. & 
 *     Digital Equipment Corporation, Maynard, Mass.
 *
 * %a%copyright(,**  )
 *
 * NAME
 *
 *     cma_dce_error.c
 *
 * FACILITY:
 *
 *     Distributed Computing Environment (DCE)
 *
 * ABSTRACT:
 *
 * Error status management routines.
 *
 * %a%private_begin
 *
 * MODIFICATION HISTORY:
 *
 * 19-Nov-91 harrow    Create cma version, include default values for thd
 *		       messages.
 * 22-oct-91 mishkin   map rad-50 0 to "a" instead of "_".
 * 13-jun-91 martin    Preface rpcsts.h with dce/;
 *                     remove inclusion of dce_error.h.
 * 09-apr-91 janicki   change status to an int
 * 27-mar-91 mishin    remove version number stuff; add support for DFS
 * 08-mar-91 janicki   Change order. Full status code now
 *                     <facility><component><code>
 *                     Update dce_error_inq_text() signature.
 * 25-jan-91 janicki   Add .cat to file name.
 *                     Add second try to open message file. 
 * 07-jan-91 sudama    Fix message not found to return -1
 *                     Fix logic in message file version mismatch
 * 29-nov-90 woodbury  Change #include paths for IBM
 * 13-nov-90 sudama    Original creation.
 *
 * %a%private_end  
 *
 */

#include <exc_handling.h>
#include <stdio.h>			/* standard i/o */
#include <strings.h>
#include <nl_types.h>			/* types for NLS (I18N) routines */

#define FACILITY_CODE_MASK          0xF0000000
#define FACILITY_CODE_SHIFT         28

#define COMPONENT_CODE_MASK         0x0FFFF000
#define COMPONENT_CODE_SHIFT        12

#define STATUS_CODE_MASK            0x00000FFF
#define STATUS_CODE_SHIFT           0

#define NO_MESSAGE                  "THIS IS NOT A MESSAGE"

/*
 * cma__error_inq_text
 *
 * FUNCTIONAL DESCRIPTION:
 *     
 * 	Returns a text string in a user provided buffer associated with a given 
 * 	error status code. In the case of errors a text string will also be 
 * 	returned indicating the nature of the error.
 *
 * FORMAL PARAMETERS:
 *
 *     status_to_convert   A DCE error status code to be converted to 
 *                         text form.
 *
 *     error_text          A user provided buffer to hold the text
 *                         equivalent of status_to_convert or
 *                         a message indicating what error occurred.
 *                         
 *
 *     status              The result of the operation. One of:
 *                          0  -  success
 *                         -1  -  failure
 *
 * IMPLICIT INPUTS:
 *	
 *	none
 *
 * IMPLICIT OUTPUTS:
 *	
 *	none
 *
 * FUNCTION VALUE:
 *	
 *	none
 *
 * SIDE EFFECTS:
 *	
 *	none
 *
 */
void
cma__error_inq_text
#if _CMA_PROTO_
	(unsigned long	status_to_convert,
	unsigned char	*error_text,
	int		*status)
#else
	(status_to_convert, error_text, status)
	unsigned long	status_to_convert;
	unsigned char	*error_text;
	int		*status;
#endif
    {
    unsigned short  facility_code;
    unsigned short  component_code;
    unsigned short  status_code;
    unsigned short  i;
    nl_catd     catd;
    int		ok = 0;
    char        component_name[4];
    char        *facility_name;
    char        catname[7];
    char	**nls_format;
    char        nls_filename[128];
    char        *message;
    static char *facility_names[] = {
        "dce",
        "dfs"
	};
    static char *path_names[] = {
	"%s.cat",
	"dce/%s",
	"%s",
	"dce/%s.cat",
	"/usr/lib/nls/msg/en_US.88591/%s.cat",
	(char *)0
	};

    /*
     * set up output status for future error returns
     */
    if (status != NULL)
        *status = -1;
    
    /*
     * check for ok input status
     */
    if (status_to_convert == 0) {

        if (status != NULL)
            *status = 0;

        strcpy ((char *)error_text, "successful completion");
        return;
	}

    /*
     * extract the component, facility and status codes
     */
    facility_code =
	(status_to_convert & FACILITY_CODE_MASK) >> FACILITY_CODE_SHIFT;
        
    component_code =
	(status_to_convert & COMPONENT_CODE_MASK) >> COMPONENT_CODE_SHIFT;

    status_code =
	(status_to_convert & STATUS_CODE_MASK) >> STATUS_CODE_SHIFT;

    /*
     * see if this is a recognized facility
     */
    if (facility_code == 0
	    || facility_code > sizeof (facility_names) / sizeof (char *)) {
        sprintf (
		(char *)error_text,
		"status 0x%lx (unknown facility)",
		status_to_convert);
        return; 
	}

    facility_name = facility_names[facility_code - 1];

    /*
     * Convert component name from RAD-50 component code.  (Mapping is:
     * 0 => 'a', ..., 25 => 'z', 26 => '{', 27 => '0', ..., 36 => '9'.)
     */

    component_name[3] = 0;
    component_name[2] = component_code % 40;
    component_code /= 40;
    component_name[1] = component_code % 40;
    component_name[0] = component_code / 40;
    
    for (i = 0; i < 3; i++)
        component_name[i] += (component_name[i] <= 26) ? 'a' : ('0' - 27);

    sprintf (catname, "%3s%3s", facility_name, component_name);

    /*
     * While SYSV convention states that NLSPATH should include the ".cat"
     * suffix, this isn't so standard in other UNIX variants. We want to try
     * to catch the catalog anyway. So loop through a list of format strings,
     * and try each one until something works. Of course, if none works,
     * we'll fall back to the literal text message (English) compiled into
     * the file.
     */
    for (nls_format = &path_names[0]; *nls_format != (char *)0; nls_format++) {
	sprintf (nls_filename, *nls_format, catname);
	catd = (nl_catd)catopen (nls_filename, 0);

	if (catd != (nl_catd)-1) {
	    ok = 1;
	    break;
	    }

	}
	
    /*
     * try to get the specified message from the file
     */
    if (ok) {
	message = (char *)catgets (catd, 1, status_code, NO_MESSAGE);

	/*
	 * if everything went well, return the resulting message
	 */
	if (strcmp (message, NO_MESSAGE) != 0) {
	    sprintf (
		    (char *)error_text,
		    "%s (%s / %s)",
		    message,
		    facility_name,
		    component_name);

	    if (status != NULL)
		*status = 0;

	    }
	else
	    ok = 0;

	}

    if (!ok) {
	char *thd_message = NULL;

	switch (status_to_convert) {
	    case exc_s_exception :
		thd_message = "Address exception raised, 0x%1$lx";
		break;
	    case exc_s_exccop :
		thd_message = "Status exception raised";
		break;
	    case exc_s_uninitexc :
		thd_message = "Uninitialized exception raised";
		break;
	    case exc_s_illaddr :
		thd_message = "Invalid memory address";
		break;
	    case exc_s_exquota :
		thd_message = "Operation failed due to insufficient quota";
		break;
	    case exc_s_insfmem :
		thd_message = "Insufficient virtual memory for requested operation";
		break;
	    case exc_s_nopriv :
		thd_message = "Insufficent privilege for requested operation";
		break;
	    case exc_s_normal :
		thd_message = "Normal successful completion";
		break;
	    case exc_s_illinstr :
		thd_message = "Illegal instruction";
		break;
	    case exc_s_resaddr :
		thd_message = "Reserved addressing error";
		break;
	    case exc_s_privinst :
		thd_message = "Privileged instruction error";
		break;
	    case exc_s_resoper :
		thd_message = "Reserved operand error";
		break;
	    case exc_s_SIGTRAP :
		thd_message = "Trace or breakpoint trap";
		break;
	    case exc_s_SIGIOT :
		thd_message = "IOT trap";
		break;
	    case exc_s_SIGEMT :
		thd_message = "EMT trap";
		break;
	    case exc_s_aritherr :
		thd_message = "Arithmetic error";
		break;
	    case exc_s_SIGSYS :
		thd_message = "Bad system call";
		break;
	    case exc_s_SIGPIPE :
		thd_message = "Broken pipe";
		break;
	    case exc_s_excpu :
		thd_message = "CPU-time limit exceeded";
		break;
	    case exc_s_exfilsiz :
		thd_message = "File-size limit exceeded";
		break;
	    case exc_s_intovf :
		thd_message = "Integer overflow";
		break;
	    case exc_s_intdiv :
		thd_message = "Integer divide by zero";
		break;
	    case exc_s_fltovf :
		thd_message = "Floating point overflow";
		break;
	    case exc_s_fltdiv :
		thd_message = "Floating point/decimal divide by zero";
		break;
	    case exc_s_fltund :
		thd_message = "Floating point underflow";
		break;
	    case exc_s_decovf :
		thd_message = "Decimal overflow";
		break;
	    case exc_s_subrng :
		thd_message = "Subscript out of range";
		break;
	    case cma_s_alerted :
		thd_message = "Thread has been canceled";
		break;
	    case cma_s_assertion :
		thd_message = "A thread assertion check has failed";
		break;
	    case cma_s_badparam :
		thd_message = "Invalid parameter to operation";
		break;
	    case cma_s_bugcheck :
		thd_message = "Internal error detected in thread library";
		break;
	    case cma_s_exit_thread :
		thd_message = "Current thread has been requested to exit";
		break;
	    case cma_s_existence :
		thd_message = "Object referenced does not currently exist";
		break;
	    case cma_s_in_use :
		thd_message = "Object referenced is already in use";
		break;
	    case cma_s_use_error :
		thd_message = "Operation inappropriate for specified object";
		break;
	    case cma_s_wrongmutex :
		thd_message = "Wrong mutex specified in condition wait";
		break;
	    case cma_s_stackovf :
		thd_message = "Attempted stack overflow was detected";
		break;
	    case cma_s_nostackmem :
		thd_message = "No space is currently available to create a new stack";
		break;
	    case cma_s_notcmastack :
		thd_message = "The current stack was not allocated by thread library";
		break;
	    case cma_s_timed_out :
		thd_message = "Timed condition wait expired";
		break;
	    case cma_s_unimp :
		thd_message = "Unimplemented feature";
		break;
	    case cma_s_inialrpro :
		thd_message = "Thread library initialization is already in progress";
		break;
	    case cma_s_defer_q_full :
		thd_message = "No space currently available to process interrupt request";
		break;
	    case cma_s_signal_q_full :
		thd_message = "Unable to process condition signal from interrupt level";
		break;
	    case cma_s_alert_nesting :
		thd_message = "Improper nesting of alert scope";
		break;
	    case exc_s_unkstatus :
		thd_message = "Unknown status code 0x%1$lx";
		break;
	    default: {
		sprintf (
			(char *)error_text,
			"status 0x%lx (%s / %s)",
			status_to_convert,
			facility_name,
			component_name);

		if (status != NULL)
		    *status = 0;

		}

	    }

	if (thd_message != NULL) {
	    sprintf (
		    (char *)error_text,
		    "%s (%s / %s)",
		    thd_message,
		    facility_name,
		    component_name);

	    if (status != NULL)
		*status = 0;

	    }

	}

    catclose (catd);
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_MESSAGE.C */
/*  *10   31-MAR-1993 16:41:36 BUTENHOF "Remove use of %08x formatting" */
/*  *9    23-MAR-1993 08:52:49 BUTENHOF "Fix address exception text for AXP" */
/*  *8    11-DEC-1992 07:32:44 BUTENHOF "Fix use of fallback messages" */
/*  *7    30-OCT-1992 05:59:45 BUTENHOF "Support for NLSPATH=%N.cat" */
/*  *6     9-JAN-1992 10:27:22 BUTENHOF "Remove ""Exception: "" from builtin messages" */
/*  *5    22-NOV-1991 11:56:28 BUTENHOF "Integrate dce message formatting" */
/*  *4    24-SEP-1991 16:27:35 BUTENHOF "Merge CMA5 reverse IBM/HP/Apollo drops" */
/*  *3     2-JUL-1991 16:47:17 BUTENHOF "Use normal printf on Tin" */
/*  *2    13-JUN-1991 19:33:12 BUTENHOF "fix history" */
/*  *1    13-JUN-1991 19:29:11 BUTENHOF "Message catalog support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_MESSAGE.C */
