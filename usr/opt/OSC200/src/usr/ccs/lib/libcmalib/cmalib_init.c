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
static char *rcsid = "@(#)$RCSfile: cmalib_init.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:59:03 $";
#endif
/*
 *  Copyright (c) 1990, 1993 by
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
 *	Concert Multithread Architecture (CMA) Library services
 *
 *  ABSTRACT:
 *
 *	Initialize the CMA Library
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	10 August 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	29 August 1990
 *		Change calls to CMA to pass structures by reference.
 *	002	Dave Butenhof	12 October 1990
 *		Fix definition of cma_lib__init for new cma_once.
 *	003	Paul Curtin	22 March 1991
 *		Added sig_block_mask global
 *	004	Paul Curtin	25 March 1991
 *		Made sig_block_mask condition on unix
 *	005	Paul Curtin	27 March 1991
 *		Unblocked a couple of signals in sig_block_mask
 *	006	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 */


/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cmalib_init.h>
#include <cmalib_attr.h>
#include <cmalib_aq_cre.h>

char *getenv _CMA_PROTOTYPE_ ((char *string));
long atoi    _CMA_PROTOTYPE_ ((char *string));

/*
 *  GLOBAL DATA
 */
cma_t_once		cma_lib__g_init_block = cma_once_init;
cma_lib__t_known_object	cma_lib__g_known_obj[cma_lib__c_obj_num];
cma_lib__t_env		cma_lib__g_env[cma_lib__c_env_count] = {
    /*
     * The following highwater marks determine the maximum number of objects
     * which can be cached (of each type) on a particular attributes object;
     * if the number of cached objects exceeds the "max" value, objects will
     * be destroyed until the number is reduced to the "min" value.
     */
    {"CMA_MAXATTR",	20},		/* If more attributes than this, */
    {"CMA_MINATTR",	5},		/* . purge back to this */
    {"CMA_MAXQUEUE",	20},		/* If more queues than this */
    {"CMA_MINQUEUE",	5},		/* . purge back to this */
    };

#ifdef unix
sigset_t        cma_lib__g_sig_block_mask;
#endif

/*
 *  LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */
static void
cma_lib___init_static _CMA_PROTOTYPE_ ((void));	/* Initialize static data */

static void
cma_lib___init_env _CMA_PROTOTYPE_ ((void));	/* Get environment variables */


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize the CMA Library 
 *
 *  FORMAL PARAMETERS:
 *
 *	arg	Argument value passed to cma_once.  The value is ignored,
 *		and may be cma_c_null_ptr.
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma_lib__init
#ifdef _CMA_PROTO_
	(cma_t_address	arg)
#else	/* no prototypes */
	(arg)
	cma_t_address	arg;
#endif	/* prototype */
    {
    cma_t_integer   i;


    /*
     * WARNING:
     *
     * Do not EVER change the order of these calls unless you are ABSOLUTELY
     * certain that you know exactly what every one does!  There are many
     * interdependencies, and some (particularly the first few) use special
     * "bootstrap" versions of some calls to get their job done.  Changing
     * the order could be MUCH more complicated than it might seem!
     *
     * In general, new initializations should be added at the END; if this
     * won't do, be VERY, VERY, VERY CAREFUL!  
     */
    cma_init ();
    for (i = 1; i < cma_lib__c_obj_num; i++) {
	cma_lib__queue_init (&cma_lib__g_known_obj[i].queue);
	cma_mutex_create    (&cma_lib__g_known_obj[i].mutex, &cma_c_null);
	}
    cma_lib___init_env ();
    cma_lib__init_sequence();
    cma_lib__init_attr ();
#ifdef unix
    if (sigfillset (&cma_lib__g_sig_block_mask) == -1)  RAISE (cma_e_assertion);
    if (sigdelset (&cma_lib__g_sig_block_mask, SIGKILL) == -1)  RAISE (cma_e_assertion);
    if (sigdelset (&cma_lib__g_sig_block_mask, SIGSTOP) == -1)  RAISE (cma_e_assertion);
    if (sigdelset (&cma_lib__g_sig_block_mask, SIGCONT) == -1)  RAISE (cma_e_assertion);
    if (sigdelset (&cma_lib__g_sig_block_mask, SIGTRAP) == -1)  RAISE (cma_e_assertion);
# ifndef NDEBUG
    if (sigdelset (&cma_lib__g_sig_block_mask, SIGQUIT) == -1)  RAISE (cma_e_assertion);
# endif
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This function is used by the transfer vector to report unimplemented
 *	entry points.
 *
 *  FORMAL PARAMETERS:
 *
 * 	none
 *
 *  IMPLICIT INPUTS:
 *
 * 	none
 *
 *  IMPLICIT OUTPUTS:
 *
 * 	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	Raises an exception.
 */
extern void
cma_lib__unimplemented
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    RAISE (cma_e_unimp);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get the value of all CMA environment variables to customize caching
 *	characteristics (and other things).
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	cma_lib__g_env array (names of variables)
 *
 *  IMPLICIT OUTPUTS:
 *
 *	cma_lib__g_env array (value of variables)
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
static void
cma_lib___init_env
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_integer	i;
    char		*stringvalue;

    for (i = 0; i < cma_lib__c_env_count; i++) {
	stringvalue = getenv (cma_lib__g_env[i].name);

	if (stringvalue != cma_c_null_ptr)
	    cma_lib__g_env[i].value = atoi (stringvalue);

	}

    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_INIT.C */
/*  *12    1-JUN-1992 14:39:57 BUTENHOF "Modify for new build environment" */
/*  *11   28-MAR-1991 12:14:08 CURTIN "removed sigquit from mask" */
/*  *10   27-MAR-1991 15:36:35 CURTIN "name chg" */
/*  *9    27-MAR-1991 15:32:16 CURTIN "unblocked a coupple of signals" */
/*  *8    25-MAR-1991 11:28:59 CURTIN "made a mask conditional on unix" */
/*  *7    22-MAR-1991 13:30:13 CURTIN "name chg" */
/*  *6    22-MAR-1991 13:28:13 CURTIN "name chg" */
/*  *5    22-MAR-1991 12:14:52 CURTIN "changed an exception name" */
/*  *4    22-MAR-1991 12:05:39 CURTIN "added sig_block_mask global" */
/*  *3    12-OCT-1990 07:11:17 BUTENHOF "Fix definition of cma_lib_init" */
/*  *2    29-AUG-1990 17:03:52 SCALES "Change CMA services to by-reference" */
/*  *1    27-AUG-1990 02:15:42 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_INIT.C */
