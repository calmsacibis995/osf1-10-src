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
/********************************************************************/
/*                    Compiler Revisions									  */
/*																						  */
/*  init   Date        Revision/Changes                             */
/*	 ----   ----        ----------------									  */
/*  bsb   9/7/90       1.0	/ Initial general customer release       */
/********************************************************************/
/*                    error.h Revisions									  */
/*																						  */
/*  init   Date        Revision/Changes									  */
/*  ----   ----        -----------------									  */
/*  bsb   9/7/90       1.0 / Initial general customer release       */
/********************************************************************/

#ifndef _SSC_ERROR_H_
#define _SSC_ERROR_H_


/*
    error.h

    #defines for error and warning messages

*/

#define CASE(num) case ((num)&0xffff)

#define CRITICAL            0x80000000L     /* Must abort compile */
#define ERROR               0x40000000L     /* Delete output (except for .lis) when done */
#define WARNING             0x00000000L     /* OK to generate output */
#define LEX                 0x20000000L     /* Can point to offending character */
#define FIRST_PASS          0x10000000L     /* Print warning on first pass, don't wait */
#define SHELL               0x08000000L     /* Error is not in input file */

#define NO_INPUT                ( (UINT32) (CRITICAL | SHELL | 01) )
#define CANT_READ               ( (UINT32) (CRITICAL | SHELL | 02) )
#define MALLOC_ERR              ( (UINT32) (CRITICAL | SHELL | 03) )

#define CANT_WRITE              ( (UINT32) (WARNING | SHELL | 04) )

#define MASK_WO_DATA            ( (UINT32) (WARNING | 10) )
#define MULTIPLE_ACK            ( (UINT32) (WARNING | 11) )
#define MULTIPLE_ATN            ( (UINT32) (WARNING | 12) )
#define MULTIPLE_TARGET         ( (UINT32) (WARNING | 13) )
#define MULTIPLE_PHASE          ( (UINT32) (WARNING | 14) )
#define MULTIPLE_DATA           ( (UINT32) (WARNING | 15) )
#define MULTIPLE_MASK           ( (UINT32) (WARNING | 16) )
#define MULTIPLE_DECLARATION    ( (UINT32) (WARNING | 17) )
#define ORIGIN_UNDEFINED        ( (UINT32) (WARNING | 18) )
#define COUNT24_RANGE           ( (UINT32) (WARNING | 19) )
#define ADDR24_RANGE            ( (UINT32) (WARNING | 20) )
#define ID_RANGE                ( (UINT32) (WARNING | 21) )
#define DATA_RANGE              ( (UINT32) (WARNING | 22) )
#define REGISTER_RANGE          ( (UINT32) (WARNING | 23) )
#define ENTRY_NOT_LABEL         ( (UINT32) (WARNING | 24) )

#define IDENT_TOO_LONG          ( (UINT32) (WARNING | FIRST_PASS | 40) )


#define ATN_AND_PHASE           ( (UINT32) (ERROR | 50) )
#define SYNTAX_ERROR            ( (UINT32) (ERROR | 51) )
#define REG_IO_ILLEGAL          ( (UINT32) (ERROR | 52) )
#define UNRESOLVED              ( (UINT32) (ERROR | 53) )
#define RELATIVE_EXTERN         ( (UINT32) (ERROR | 54) )

#define UNEXPECTED_CHAR         ( (UINT32) (ERROR | LEX | 70) )
#define BAD_CONSTANT            ( (UINT32) (ERROR | LEX | 71) )
#define CONSTANT_OVERFLOW       ( (UINT32) (ERROR | LEX | 72) )
#define MISMATCH_BRACKETS       ( (UINT32) (ERROR | LEX | 73) )

#endif 
