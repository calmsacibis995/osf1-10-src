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
 * @(#)$RCSfile: cmalib_assert.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:58:38 $
 */
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
 *	Header file for CMA Assertions package 
 *	(Copied directly from CMA_ASSERTIONS.H)
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	7 August 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 *
 */

/* 
 * Overview of Assertions
 * ----------------------
 *
 * An assertion is a claim that some relationship among program
 * variables will always be true. Put another way, an assertion is
 * a claim that an "invariant" holds true.
 * 
 * This module defines macros that allow you to insert assertions into
 * your code.  (The assertions inserted using this module are only present
 * in a 'debugging version' of CMA.  In a 'production version' of CMA,
 * while assertions remain present in the source code, no object code
 * is generated -- the assertions "evaporate".
 *
 * The purpose of assertions is to facilitate early detection of unexpected
 * situations, internal inconsistencies, and bugs in the CMA implementation.  
 * Since they do not generate any code in a production version of CMA,
 * and do not use messages that can be easily internationalized, they should
 * not be used for reporting client-generated errors (a separate 
 * error-message-reporting package is provided for that).
 *
 * As a general rule, when your code makes important assumptions
 * about the state of variables, it is a good idea to code an assertion
 * for each such assumption.  When you document such assumptions in
 * assertions, you will be immediately notified when the assertion is
 * violated at run-time (during testing or development). Without assertions, 
 * such assumptions would not get checked and might not even get documented.  
 * Assertions help to ensure that future maintenance will not accidentally 
 * violate some important built-in assumption.
 * 
 * Appropriate usage of this package saves many hours spent tracking
 * down the obscure errors that typically occur during development.
 * (It is a lot easier to locate a failing assertion than to work back
 * from an "access violation" to the offending source line.)
 *
 * Kinds of Assertions
 * -------------------
 * This module recognizes two kinds of assertions -- warnings and failures.  
 * 
 * You should insert a warning assertion when some assumption is violated
 * that is not necessarily a 'fatal' error.  When a warning assertion
 * is violated, the warning message is displayed and program execution
 * resumes.  You can also use warning assumptions to document things that 
 * you think *ought* to be true, even if you aren't absolutely sure -- 
 * if you are wrong, the consequence is a visible warning message and
 * you will will be able to investigate the inconsistency. An example of a 
 * warning assertion, stated in English, is "The foo list shouldn't have more 
 * than 100 elements". (This assumption might be the basis of your deciding 
 * to use sequential search rather than a hash algorithm, and you'd like to 
 * know when your assumption is wrong.) This assertion would be coded as:
 *  
 *   cma_lib__assert_warn (foo_list_size <= 100, 
 *       "Size of foo list exceeds 100 elements");
 *
 * You should insert a failure assertion when some assumption is violated
 * that *must* be true for the subsequent code to execute correctly.  An
 * example of a failure assertion, is that a particular mutex *must* be locked.
 * This assertion would be coded as:
 *
 *   cma_lib__assert_fail (cma_is_locked(att_mutex), 
 *       "Attributes object mutex is not locked");
 *
 * When Are Assertions Present?
 * ----------------------------
 * Assertions can be enabled in any build of CMA.  Typically, we do so
 * only when building a 'debugging version'.  Currently assertions are
 * activated by default whenever you compile a module, and can be made
 * to evaporate by compiling all modules with symbol NDEBUG defined.
 * (There might be some reason in the future to use a different scheme
 * since NDEBUG has widespread effects on various C macros.  But, this
 * seems reasonable for now.)
 * 
 * The test system will be run against a debugging version.
 * Also, while we are developing code, we will link against debugging
 * versions of the object modules.  So, in general, we'll all know
 * immediately when any assertion is violated.
 */

#ifndef CMALIB_ASSERT
#define CMALIB_ASSERT

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cmalib_defs.h>

/*
 * CONSTANTS AND MACROS
 */

/* 
 * Macro for warning assertion.  Generates no code in productions builds.
 * 
 *      cma_lib__assert_warn (<boolean_expression>, <description of violation>);
 *      NOTE: a C_boolean_expression is an integer-valued expression that
 *            is interpreted as false if 0 and is true otherwise.
 *     
 */

#ifndef NDEBUG
#define cma_lib__assert_warn(condition, description)	\
    ( (!(condition)) ?					\
	cma_lib__assert_message (			\
		cma_c_false,				\
		description, 				\
		__FILE__, 				\
		__LINE__) :				\
	0 )
#else
#define cma_lib__assert_warn(condition, description)    0
#endif

/* 
 * Macro for failure assertions.  Generates no code in production builds.
 * 
 *      cma_lib__assert_fail (<boolean_expression>, <description of violation>);
 *      NOTE: a C_boolean_expression is an integer valued expression that
 *            is interpreted as false if 0 and is true otherwise.
 */
#ifndef NDEBUG
#define cma_lib__assert_fail(condition, description)	\
    ( (!(condition)) ?					\
	cma_lib__assert_message ( 			\
		cma_c_true, 			    	\
		description, 			    	\
		__FILE__,				\
		__LINE__) :				\
	0 )
#else
#define cma_lib__assert_fail(condition, description)    0
#endif

/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

/* 
 * Worker routine needed by above macros.  
 * Reports the error when an assertion is violated.
 */
cma_t_integer
cma_lib__assert_message _CMA_PROTOTYPE_ ((
	cma_t_boolean	    is_failure,	    /* Was it a fail or a warn? */
	cma_lib__t_string   description,    /* English description of problem */
	cma_lib__t_string   file,	    /* File name */
	cma_t_integer	    line));	    /* Line number */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_ASSERT.H */
/*  *2     1-JUN-1992 14:39:34 BUTENHOF "Modify for new build environment" */
/*  *1    27-AUG-1990 02:14:51 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_ASSERT.H */
