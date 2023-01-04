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
 *	@(#)$RCSfile: gl_defs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:11 $
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
#ifdef SEC_BASE



/*
	gl_defs.h - global defs, initialization, & other macros
*/

#ifdef GL_ALLOCATE

# undef GLOBAL
# undef INIT1
# undef INIT2
# undef INIT3
# undef INIT4
# undef INIT5
# undef INIT6
# undef INIT7
# undef INIT8
# undef INIT9
# undef INIT10

# define GLOBAL
# define INIT1(A) = A
# define INIT2(A, B) = { A, B }
# define INIT3(A, B, C) = { A, B, C }
# define INIT4(A, B, C, D) = { A, B, C, D }
# define INIT5(A, B, C, D, E) = { A, B, C, D, E }
# define INIT6(A, B, C, D, E, F) = { A, B, C, D, E, F }
# define INIT7(A, B, C, D, E, F, G) = { A, B, C, D, E, F, G }
# define INIT8(A, B, C, D, E, F, G, H) = { A, B, C, D, E, F, G, H }
# define INIT9(A, B, C, D, E, F, G, H, I) = { A, B, C, D, E, F, G, H, I }
# define INIT10(A, B, C, D, E, F, G, H, I, J) = { A, B, C, D, E, F, G, H, I, J }

#else /* GL_ALLOCATE */

# undef GLOBAL
# undef INIT1
# undef INIT2
# undef INIT3
# undef INIT4
# undef INIT5
# undef INIT6
# undef INIT7
# undef INIT8
# undef INIT9
# undef INIT10

# define GLOBAL extern
# define INIT1(Q)
# define INIT2(A, B)
# define INIT3(A, B, C)
# define INIT4(A, B, C, D)
# define INIT5(A, B, C, D, E)
# define INIT6(A, B, C, D, E, F)
# define INIT7(A, B, C, D, E, F, G)
# define INIT8(A, B, C, D, E, F, G, H)
# define INIT9(A, B, C, D, E, F, G, H, I)
# define INIT10(A, B, C, D, E, F, G, H, I, J)

#endif /* GL_ALLOCATE */

#endif /* SEC_BASE */
