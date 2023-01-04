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
 *       @(#)$RCSfile: excpt.h,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/08/02 22:44:29 $
 */
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
#ifndef __EXCPT_H
#define __EXCPT_H

#ifdef _INCLUDE_ID
static char *excpt_h_id="@(#)$RCSfile: excpt.h,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/08/02 22:44:29 $";
#endif

/* 
** C Structured Exception Handling.
*/



/* definitions of keywords and intrinsic function names */
#define try                    __builtin_try
#define except                 __builtin_except
#define finally                __builtin_finally
#define leave                  __builtin_leave
#define exception_code()       __exception_code
#define exception_info()       __exception_info
#define abnormal_termination() __abnormal_termination
 
typedef struct system_exrec *exrec_ptr;


#ifdef __unix__
/* UNIX Exception Record */
typedef struct system_exrec {
  long ExceptionCode;
  unsigned long ExceptionFlags;
  exrec_ptr ExceptionRecord;
  void *ExceptionAddress;
  unsigned long NumberParameters;
  unsigned long ExceptionInformation[1]; 
} system_exrec_type;
#endif



/*
** Langauge specific handlers
*/



#ifdef __unix__
typedef struct {
    int	sc_regmask;		/* regs to restore in sigcleanup */
    int	sc_mask;		/* signal mask to restore */
    int	sc_pc;			/* pc at time of signal */
    /*
     * General purpose registers
     */
    int	sc_regs[32];	/* processor regs 0 to 31 */
    /*
     * Floating point coprocessor state
     */
    int	sc_ownedfp;	/* fp has been used */
    int	sc_fpregs[32];	/* fp regs 0 to 31 */
    int	sc_fpc_csr;	/* floating point control and status reg */
    int	sc_fpc_eir;	/* floating point exception instruction reg */
    int	sc_mdhi;	/* Multiplier hi and low regs */
    int	sc_mdlo;
    /*
     * System coprocessor registers at time of signal
     */
    int	sc_cause;	/* cp0 cause register */
    int	sc_badvaddr;	/* cp0 bad virtual address */
    int	sc_badpaddr;	/* cpu bd bad physical address */
    int	is_sigset;
} CONTEXT, *PCONTEXT;

#endif

/*
 * Exception disposition return values.
*/

typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution,
    ExceptionContinueSearch,
    ExceptionNestedException,
    ExceptionCollidedUnwind
    } EXCEPTION_DISPOSITION;

/*
 * Exception flag definitions.
*/

#define EXCEPTION_NONCONTINUABLE 0x1    /* Noncontinuable exception */
#define EXCEPTION_UNWINDING      0x2    /* Unwind is in progress */
#define EXCEPTION_EXIT_UNWIND    0x4    /* Exit unwind is in progress */
#define TARGET_UNWIND            0x8    /* Execute termination handler for it*/
#define EXCEPTION_STACK_INVALID  0x10   /* Stack out of limits or unaligned */
#define EXCEPTION_NESTED_CALL    0x20   /* Nested exception handler call */

#define EXCEPTION_UNWIND       (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND |\
				TARGET_UNWIND)
#define IS_UNWINDING(flag)     ((flag & EXCEPTION_UNWIND) != 0)
#define IS_DISPATCHING(flag)   ((flag & EXCEPTION_UNWIND) == 0)
#define IS_TARGET_UNWIND(flag) (flag & TARGET_UNWIND)

/*
 * Function table definition
*/

typedef struct {
    unsigned long  begin_address;
    unsigned long  end_address;
    unsigned long  lang_handler;
    unsigned long  handler_data;
    unsigned long  prolog_end_address;
} RUNTIME_FUNCTION, *pRUNTIME_FUNCTION;

/*
 * Scope table definition
*/

typedef struct {
    unsigned long count;   /* number of scope records follows */
    struct 
    {
	unsigned long  begin_address;
	unsigned long  end_address;
	unsigned long  handle_address;
	unsigned long  jmp_target;
    } scope_record[1];
} SCOPE_TABLE;


/*
 * Exception dispatcher routine definition.
*/

int
DispatchException (system_exrec_type *exceptionRecord,
		   CONTEXT *          contextRecord
		   );

/*
 * The dispatcher context structure.
*/
typedef struct {
    unsigned long     pc;
    pRUNTIME_FUNCTION functionTable;
} DISPATCHER_CONTEXT;

/*
 * Exception handler routine definition.
*/

typedef
EXCEPTION_DISPOSITION
(*exception_handler_type) (system_exrec_type *   exceptionRecord,
			   void *                EstablisherFrame,
			   CONTEXT *             contextRecord,
			   DISPATCHER_CONTEXT *  dispatcherContext
			   );

/*
 * Utility functions
 */
extern
pRUNTIME_FUNCTION
find_rtfunc(unsigned long pc);

extern
unsigned long
find_framePtr( pRUNTIME_FUNCTION pfunc, PCONTEXT context );

extern
void
virtual_unwind( pRUNTIME_FUNCTION pfunc, PCONTEXT context );

/* some temporary definitions, just to get things going */
#define STATUS_UNWIND          99999
#ifndef __boolean
#define __boolean
#define boolean int
#endif

/* function interfaces for MIPS implementation */
#define longjmp __excpt_longjmp

extern
EXCEPTION_DISPOSITION
__C_specific_handler(system_exrec_type *   exceptionRecord,
		     void *                EstablisherFrame,
		     PCONTEXT              contextRecord,
		     DISPATCHER_CONTEXT *  dispatcherContext);
/* Typedef for pointer returned by exception_info() */ 

typedef struct _EXCEPTION_POINTERS
{
    system_exrec_type* ExceptionRecord;
    PCONTEXT           ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS, *Exception_info_ptr;

typedef
int
(* EXCEPTION_FILTER) ( EXCEPTION_POINTERS * ExceptionPtr );

typedef
void
(* TERMINATION_HANDLER) (boolean is_abnormal );

/* two linker defined variable */
extern RUNTIME_FUNCTION _fpdata[];
extern char _fpdata_size[];
#define function_table   _fpdata
#define function_table_size ((int) _fpdata_size)
extern void __jump_unwind(void *fp, void* target_pc);

#endif

/* 
 * 
*/
