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
 * @(#)$RCSfile: c_asm.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/12/15 22:13:07 $
 */
#ifndef __C_ASM_H
#define __C_ASM_H

/*** #include <c_asm.h>
 *
 *   Synopsis:  This header file is used to control the functionality
 *              of the asm in-line assembly language generator 
 *              functions.  Users include this header file to get
 *              the special compiler-defined behavior.  Not defining
 *              it causes asm and friends to default to normal function
 *              call behavior.
 *
 *** Usage information:
 *
 *   asm, dasm, and fasm all have function semantics in terms of how
 *   they are viewed by the compiler.  The following restrictions apply:
 *
 *   1.  The first parameter must be a constant character string
 *       (that is, it may not be a complex reference into a table,
 *       and it may not be something which requires runtime evaluation.
 *
 *   2.  asm returns integral values, fasm returns floating values, 
 *       and dasm returns double values.  It is the responsibility
 *       of the user to insure that the result value is placed into
 *       the proper return register (that is, v0 for asm, f0 for 
 *       fasm/dasm).  
 *
 *   3.  A return value is optional; that is, backward compatibility 
 *       with the "old" asm is maintained.asm, dasm, or fasm can be seen as
 *       void functions at the user's option.
 *
 *   
 *
 *** Usage Example:
 *
 *   #include <c_asm.h>
 *
 *   main()
 *       {
 *        long op1, op2, r, results[2],op[2];
 *
 *
 *        op1  = 0xffffffffffffffff;
 *        op2  = 0xfff;
 *
 *        results[0] = asm("umulh %a0, %a1, %v0",op1, op2 );
 *        results[1] = asm("mulq  %a0, %a1, %v0");
 *
 *        op[0] = results[0];
 *        op[1] = results[1];
 *        printf("%d %d\n",op[0],op[1] );
 *       }
 *
 */

#ifdef __cplusplus
#error <c_asm.h> is not a valid C++ header
#endif
 
	float  fasm( const char *,... );
	long   asm( const char *,...);
	double dasm( const char *,... );

#pragma intrinsic( fasm )
#pragma intrinsic( asm )
#pragma intrinsic( dasm )

#endif
