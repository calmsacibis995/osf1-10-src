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
 * @(#)$RCSfile: cma_init.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/05/11 22:01:32 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for CMA initialization
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	11 September 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave butenhof	1 June 1990
 *		Add structure and global for environment definition code.
 *	002	Dave Butenhof	28 August 1990
 *		Change cma_shr_init to cma__shr_init, since it's really an
 *		internal interface (even though it's called from the client
 *		image and must be in the transfer vector, no client should
 *		call it directly).
 *	003	Dave Butenhof	22 January 1991
 *		Merge cma_init back into this module (should have been done a
 *		while ago when we removed the requirement that client link
 *		against cma_client.obj).
 *	004	Dave Butenhof	29 March 1991
 *		Change environment initialization stuff
 *	005	Paul Curtin	 7 May 1991
 *		Created cma__int_init macro.
 *	006	Dave Butenhof	19 September 1991
 *		Integrate HPUX CMA5 reverse drop: HP has apparently
 *		integrated DECthreads initialization into crt0.o, and
 *		therefore makes the optimization of defining cma__int_init()
 *		to expand to a null string.
 *	007	Dave Butenhof	14 November 1991
 *		Improve performance of cma__int_init() by doing an
 *		uninterlocked test first; then repeat with interlock if it
 *		doesn't appear to have been set to gain cache coherency.
 *	008	Dave Butenhof	22 November 1991
 *		Remove include of cma_kernel.h, which isn't needed: all it
 *		needs is cma_host.h (which should also avoid a circularity
 *		that seems to be confusing c89). Also, include cma_errors.h
 *		to pick up cma__error().
 *	009	Dave Butenhof	17 April 1992
 *		Define the extern version string.
 *	010	Dave Butenhof	25 August 1992
 *		Add conditional to evaporate cma__int_init() on systems where
 *		DECthreads can initialize automatically -- it's silly to
 *		waste the time checking (and penalizes our pthread interface
 *		unnecessarily versus the OSF/1 pthreads implementation).
 *	011	Dave Butenhof	25 November 1992
 *		Remove some of the environment variables that we don't need
 *		any more.
 *	012	Dave Butenhof	14 April 1993
 *		Remove more silly environment variables. High and low water
 *		marks on clusters?
 */

#ifndef CMA_INIT
#define CMA_INIT

/*
 *  INCLUDE FILES
 */
#include <cma_host.h>
#include <cma_errors.h>

/*
 * CONSTANTS AND MACROS
 */

#define cma__c_env_maxattr	0
#define cma__c_env_minattr	1
#define cma__c_env_maxthread	2
#define cma__c_env_minthread	3
#define cma__c_env_trace	4

#define cma__c_env_count	5


/*
 * cma__int_init
 *
 * Initialize the main body of CMA exactly once.
 *
 * We raise an exception if, for some odd reason, there are already threads
 * in the environment (e.g. kernel threads), and one of them is trying to
 * initialize CMA before the  first thread got all the way through the actual
 * initialization. This code maintains the invariants: "after successfully
 * calling CMA_INIT, you can call any CMA function", and  "CMA is actually
 * initialized at most once".
 *
 * On systems where DECthreads can initialize automatically at startup (HPUX,
 * VMS, and OSF/1, currently), this macro evaporates to save execution time.
 */
#if _CMA_AUTO_INIT_
# define cma__int_init()
#else
# define cma__int_init() { \
    if (!cma__tac_isset(&cma__g_init_started)) { \
	if (!cma__test_and_set (&cma__g_init_started)) { \
	    cma__init_static (); \
	    cma__test_and_set (&cma__g_init_done); \
	    } \
	else if (!cma__tac_isset (&cma__g_init_done)) { \
	    cma__error (cma_s_inialrpro); \
    }}}
#endif

/*
 * TYPEDEFS
 */

typedef enum CMA__T_ENV_TYPE {
    cma__c_env_type_int,
    cma__c_env_type_file
    } cma__t_env_type;

typedef struct CMA__T_ENV {
    char		*name;		/* Name of environment variable */
    cma__t_env_type	type;		/* Type of variable */
    cma_t_integer	value;		/* Numeric value of the variable */
    } cma__t_env;

/*
 *  GLOBAL DATA
 */

extern cma__t_env		cma__g_env[cma__c_env_count];
extern cma__t_atomic_bit	cma__g_init_started;
extern cma__t_atomic_bit	cma__g_init_done;
extern char			*cma__g_version;

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
extern cma_t_address		cma__g_base_frame;
#endif

/*
 * INTERNAL INTERFACES
 */

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
extern void
cma__find_persistent_frame _CMA_PROTOTYPE_ ((void));
#endif

extern void
cma__init_static _CMA_PROTOTYPE_ ((void));	/* Initialize static data */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_INIT.H */
/*  *18   16-APR-1993 13:04:02 BUTENHOF "Remove some silly init logicals" */
/*  *17    1-DEC-1992 14:05:27 BUTENHOF "OSF/1 scheduling" */
/*  *16   25-AUG-1992 11:48:25 BUTENHOF "Don't compile cma__int_init() for auto-init systems!" */
/*  *15   21-MAY-1992 13:46:15 CURTIN "" */
/*  *14   21-MAY-1992 11:43:51 CURTIN "" */
/*  *13   21-MAY-1992 09:39:14 CURTIN "Added extern def for cma__g_base_frame: OpenVMS Alpha specific" */
/*  *12   17-APR-1992 11:11:33 BUTENHOF "Add version number string" */
/*  *11   22-NOV-1991 11:56:19 BUTENHOF "Don't include cma_kernel.h" */
/*  *10   18-NOV-1991 11:23:51 BUTENHOF "Add non-interlocked test to cma__int_init()" */
/*  *9    24-SEP-1991 16:27:26 BUTENHOF "Merge CMA5 reverse IBM/HP/Apollo drops" */
/*  *8    10-JUN-1991 19:53:36 SCALES "Convert to stream format for ULTRIX build" */
/*  *7    10-JUN-1991 19:20:56 BUTENHOF "Fix the sccs headers" */
/*  *6    10-JUN-1991 18:22:05 SCALES "Add sccs headers for Ultrix" */
/*  *5     7-MAY-1991 10:10:52 CURTIN "created cma__int_init macro" */
/*  *4    12-APR-1991 23:35:56 BUTENHOF "Change type of internal locks" */
/*  *3     1-APR-1991 18:09:00 BUTENHOF "Change environment setup" */
/*  *2    24-JAN-1991 00:34:59 BUTENHOF "Get rid of cma_client module" */
/*  *1    12-DEC-1990 21:46:25 BUTENHOF "CMA initializer" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_INIT.H */
