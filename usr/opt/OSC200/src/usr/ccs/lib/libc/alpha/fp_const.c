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
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: LIBCCNV floating point constants
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak DEPSILON = __DEPSILON
#pragma weak DFPMAX = __DFPMAX
#pragma weak DFPMIN = __DFPMIN
#pragma weak DINFINITY = __DINFINITY
#pragma weak DQNAN = __DQNAN
#pragma weak DSNAN = __DSNAN
#pragma weak SEPSILON = __SEPSILON
#pragma weak SFPMAX = __SFPMAX
#pragma weak SFPMIN = __SFPMIN
#pragma weak SINFINITY = __SINFINITY
#pragma weak SQNAN = __SQNAN
#pragma weak SSNAN = __SSNAN
#endif
#include <ctype.h>
#include <fp.h>

/*
 * NAME: fp constants
 *                                                                    
 * FUNCTION: define several floating point constants
 *                                                                    
 * NOTES:
 *
 *      This file contains the external definitions for various
 *      values declared in float.h and math.h. This sort of definition
 *      is allowed in ANSI C.
 *
 *      For FORTRAN programs these external variables can be
 *      referenced directly as external variables. Thus the names
 *      below must be FORTRAN acceptable labels.
 *
 *
 */

/*
 *      Doubles
 */


	unsigned int DEPSILON[2]    = { INTS2DBL(0x3cb00000, 0x00000000) };
	unsigned int DFPMIN[2]      = { INTS2DBL(0x00100000, 0x00000000) };
	unsigned int DFPMAX[2]      = { INTS2DBL(0x7fefffff, 0xffffffff) };
	unsigned int DINFINITY[2]   = { INTS2DBL(0x7ff00000, 0x00000000) };
	unsigned int DQNAN[2]       = { INTS2DBL(0x7ff80000, 0x00000000) };
	unsigned int DSNAN[2]       = { INTS2DBL(0x7ff55555, 0x55555555) };

/*
 *      Floats (SINGLES)
 */

	unsigned int SEPSILON  = 0x34000000;
	unsigned int SFPMIN    = 0x00800000;
	unsigned int SFPMAX    = 0x7f7fffff;
	unsigned int SINFINITY = 0x7f800000;
	unsigned int SQNAN     = 0x7fc00000;
	unsigned int SSNAN     = 0x7f855555;
