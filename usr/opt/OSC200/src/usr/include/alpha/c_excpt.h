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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
#ifndef __C_EXCPT_H
#define __C_EXCPT_H

#if defined(__LANGUAGE_C) || defined(__LANGUAGE_C__) || defined(__cplusplus)

#ifdef _INCLUDE_ID
static char *excpt_h_id="$Header: /usr/sde/osf1/rcs/os/src/usr/include/alpha/c_excpt.h,v 1.1.4.3 1993/12/15 22:12:39 Thomas_Peterson Exp $";
#endif

/* 
** C Structured Exception Handling.
*/


/* definitions of keywords and intrinsic function names */
#define try                    __builtin_try
#define except                 __builtin_except
#define leave                  __builtin_leave
#define finally                __builtin_finally
#define exception_code()       __exception_code
#define exception_info()       __exception_info
#define abnormal_termination() __abnormal_termination
 
/*
 * Scope table definition
*/

typedef struct {
    unsigned long count;   /* number of scope records follows */
    struct 
    {
	unsigned long  begin_address;	/* begin guarded block address */
	unsigned long  end_address;	/* end guarded block address */
	unsigned long  handle_address;	/* finally handler/filter for block */
	unsigned long  jmp_target;	/* jump target if exc is handled */
    } scope_record[1];
} SCOPE_TABLE;

#ifdef __cplusplus
extern "C" {
#endif
extern void
__jump_unwind(void* Frame_ptr, void* target_ip)
;
extern EXCEPTION_DISPOSITION
__C_specific_handler(system_exrec_type *   exception_record,
		     void *                establisher_frame,
		     PCONTEXT              context_record,
		     DISPATCHER_CONTEXT *  dispatcher_context)
;
#ifdef __cplusplus
}
#endif

/* Typedef for pointer returned by exception_info() */ 

typedef struct _EXCEPTION_POINTERS
{
    system_exrec_type*		ExceptionRecord;
    PCONTEXT			ContextRecord;
    EXCEPTION_DISPOSITION	Disposition;
    DISPATCHER_CONTEXT		*DispatcherContext;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS, *Exception_info_ptr;

typedef
int
(* EXCEPTION_FILTER) ( EXCEPTION_POINTERS * ExceptionPtr );

typedef
void
(* TERMINATION_HANDLER) (unsigned long is_abnormal );

#endif /* __LANGUAGE_C__ */
#endif /* __C_EXCPT_H */
