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
#ifndef __EXCPT_H
#define __EXCPT_H


#if defined(_LANGUAGE_C) || defined(__LANGUAGE_C__) || defined(__cplusplus)

#ifdef _INCLUDE_ID
static char *excpt_h_id="$Header: /usr/sde/osf1/rcs/os/src/usr/include/alpha/excpt.h,v 1.1.7.3 1993/12/15 22:12:41 Thomas_Peterson Exp $";
#endif

/* 
** Structured Exception Handling.
*/



#if defined(__unix) || defined(__osf__)

#if defined(__mips64) || defined(__alpha)
#include <pdsc.h>

/*

	Exisiting condition codes:

			LIBEXC		NT		VMS

	Code/NUMBER	32 bits		0:16		3:13
	Facility	--		16:13		16:11
	Customer	--		29:1		27:1
	Severity	--		30:2		0:3
	Control		--		--		28:4

	Libexc segments the address space into signals and other constants.
	We will not try to be compatible with old LIBEXC constants. Instead
	what we'll try to do is case everything from the facility field.

	We will not try to make NT interoperate with VMS.


*/


typedef union exception_code {
    unsigned long		code_quad;
    struct {
	pdsc_uint_16		facility_dependent_1:16;
	pdsc_uint_16		facility:12;
	pdsc_uint_16		facility_dependent_2:4;
	pdsc_uint_32		facility_dependent_3;
    } exception_code_base;
    struct {
	pdsc_uint_32		osf_facility;/* osf marker+signal, lang, etc */
	pdsc_uint_32		code;		/* subcode */
    } exception_code_osf;
    struct {
	pdsc_uint_16		code:16;	/* subcode */
	pdsc_uint_16		facility:13;	/* base distinguisher */
	pdsc_uint_16		customer:1;	/* nt versus customer */
	pdsc_uint_16		severity:2;	/* as it says */
	pdsc_uint_32            reserved;       /* sign extension of bit 31 */
    } exception_code_nt;
    struct {
	pdsc_uint_16            severity:3;     /* as it says */
	pdsc_uint_16            message_number:13;      /* subcode */
	pdsc_uint_16            facility:11;    /* base distinguisher */
	pdsc_uint_16            customer:1;     /* vms versus customer */
	pdsc_uint_16            control:4;      /* 1=>prnt,rest resrv */
	pdsc_uint_32            reserved;       /* sign extension of bit 31 */
    } exception_code_vms;
} exception_code; /* exception_code */

/* osf sub facility field */
#define EXC_OSF				(0xffeUL<<16UL)	/* osf marker */
#define EXC_INTERNAL			(EXC_OSF|0UL)	/* exception system */
#define EXC_FACILITY_END		(EXC_OSF|1UL)	/* like old end */
#define EXC_ALL				(EXC_OSF|2UL)	/* wild card */
#define EXC_SIGNAL			(EXC_OSF|3UL)	/* signal numbers */
#define EXC_ADA_USER			(EXC_OSF|4UL)	/* ada user */
#define EXC_PL1_USER			(EXC_OSF|5UL)	/* pl1 user */
#define EXC_CXX_USER			(EXC_OSF|6UL)	/* c++ user */
#define EXC_CXX_EXIT			(EXC_OSF|7UL)	/* c++ exit path */
#define EXC_CXX_OTHER			(EXC_OSF|8UL)	/* c++ user */
#define EXC_C_USER			(EXC_OSF|9UL)	/* c user */

#define EXC_VALUE(osf_facility, code)	((((unsigned long)(code)) << 32UL) | ((unsigned long)(osf_facility)))

/* internal exception codes (or called status) */
#define EXC_STATUS_UNWIND			EXC_VALUE(EXC_INTERNAL, 0)
#define EXC_STATUS_NONCONTINUABLE_EXCEPTION	EXC_VALUE(EXC_INTERNAL, 1)
#define EXC_STATUS_INVALID_DISPOSITION		EXC_VALUE(EXC_INTERNAL, 2)
#define EXC_SIGNAL_EXPECTED			EXC_VALUE(EXC_INTERNAL, 3)
#define EXC_RUNTIME_FUNCTION_NOT_FOUND		EXC_VALUE(EXC_INTERNAL, 4)
#define EXC_INFINITE_LOOP_UNWIND		EXC_VALUE(EXC_INTERNAL, 5)
#define EXC_INVALID_DISPATCHER_CONTEXT		EXC_VALUE(EXC_INTERNAL, 6)
#define EXC_LOCK_ERROR				EXC_VALUE(EXC_INTERNAL, 7)
#define EXC_INVALID_EXCEPTION_RECORD		EXC_VALUE(EXC_INTERNAL, 8)
#define EXC_UNSUPPORTED				EXC_VALUE(EXC_INTERNAL, 9)
#define EXC_STACK_OVERFLOW			EXC_VALUE(EXC_INTERNAL, 10)
#define EXC_OUT_OF_MEMORY			EXC_VALUE(EXC_INTERNAL, 11)
#define EXC_INVALID_ARGUMENT			EXC_VALUE(EXC_INTERNAL, 12)


#define EXC_FALSE 0		/* return value from RtlDispatchException */

#endif	/* alpha */

typedef struct system_exrec *exrec_ptr;

/* UNIX Exception Record */
typedef struct system_exrec {
  long			ExceptionCode;		/* reason for exception */
  unsigned long		ExceptionFlags;		/* in progress, e.g. unwind */
  exrec_ptr		ExceptionRecord;	/* rec chain, e.g.nested info */
  void			*ExceptionAddress;	/* where error occurred */
  unsigned long		NumberParameters;	/* # of ExceptionInformation's*/
  unsigned long		ExceptionInformation[1];/* additional info */ 
} system_exrec_type;

/*
** Langauge specific handlers
*/

#if defined(__mips64) || defined(__alpha)

#include <signal.h>
typedef struct sigcontext CONTEXT, *PCONTEXT;
typedef unsigned long	exc_address;

/*
 * the following hold the addresses currently used to retrieve the registers
 *	for a stack frame (see RtlVirtualUnwind)
 */
typedef exc_address		CONTEXT_POINTERS[64];
typedef CONTEXT_POINTERS	*PCONTEXT_POINTERS;

#else
#if defined(__mips)
typedef unsigned int	exc_address;
typedef struct {
    long	sc_regmask;		/* regs to restore in sigcleanup */
    long	sc_mask;		/* signal mask to restore */
    long	sc_pc;			/* pc at time of signal */
    /*
     * General purpose registers
     */
    long	sc_regs[32];	/* processor regs 0 to 31 */
    /*
     * Floating point coprocessor state
     */
    long	sc_ownedfp;	/* fp has been used */
    long	sc_fpregs[32];	/* fp regs 0 to 31 */
    long	sc_fpc_csr;	/* floating point control and status reg */
    long	sc_fpc_eir;	/* floating point exception instruction reg */
    long	sc_mdhi;	/* Multiplier hi and low regs */
    long	sc_mdlo;
    /*
     * System coprocessor registers at time of signal
     */
    long	sc_cause;	/* cp0 cause register */
    long	sc_badvaddr;	/* cp0 bad virtual address */
    long	sc_badpaddr;	/* cpu bd bad physical address */
    long	is_sigset;
} CONTEXT, *PCONTEXT;
#endif /* mips */
#endif /* alpha */
#endif /* unix & osf */


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

#define EXCEPTION_NONCONTINUABLE 0x1UL    /* Noncontinuable exception */
#define EXCEPTION_UNWINDING      0x2UL    /* Unwind is in progress */
#define EXCEPTION_EXIT_UNWIND    0x4UL    /* Exit unwind is in progress */
#define EXCEPTION_STACK_INVALID  0x08UL   /* Stack out of limits or unaligned */
#define EXCEPTION_NESTED_CALL    0x10UL   /* Nested exception handler call */
#define EXCEPTION_TARGET_UNWIND  0x20UL   /* Execute termination handler for it*/
#define EXCEPTION_COLLIDED_UNWIND  0x40UL /* unwind through unwind dispatcher */
#define EXCEPTION_NOSIGMASK	 0x80UL /* do not restore sigmask on continue */

#define EXCEPTION_UNWIND       (EXCEPTION_UNWINDING | \
				EXCEPTION_EXIT_UNWIND | \
				EXCEPTION_TARGET_UNWIND | \
				EXCEPTION_COLLIDED_UNWIND)

#define IS_UNWINDING(flag)     (((flag) & EXCEPTION_UNWIND) != 0)
#define IS_DISPATCHING(flag)   (((flag) & EXCEPTION_UNWIND) == 0)
#define IS_TARGET_UNWIND(flag) ((flag) & EXCEPTION_TARGET_UNWIND)

/*
 * Function table definition
*/

#if defined(__mips64) || defined(__alpha)


typedef union pdsc_crd RUNTIME_FUNCTION, *pRUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

/* define macros to make this work with existing code */
#define EXCPT_PD(crd)		PDSC_CRD_PRPD(crd)
/* excpects no code ranges yet other than that for the procedure */
#define EXCPT_BEGIN_ADDRESS(p)	PDSC_CRD_BEGIN_ADDRESS_FIELD(p)
#define EXCPT_END_ADDRESS(p)	PDSC_CRD_BEGIN_ADDRESS((p)+1)
#define EXCPT_LANG_HANDLER(p)	PDSC_RPD_HANDLER(EXCPT_PD(p))
#define EXCPT_LANG_HANDLER_DATA(p) PDSC_RPD_HANDLER_DATA(EXCPT_PD(p))
#define EXCPT_PROLOG_END_ADDRESS(p) 		/* address+1????? */ \
	(EXCPT_BEGIN_ADDRESS(p) + PDSC_RPD_ENTRY_LENGTH(EXCPT_PD(p)))


#else

/* mips */

typedef struct {
    unsigned long  begin_address;
    unsigned long  end_address;
    unsigned long  lang_handler;
    unsigned long  handler_data;
    unsigned long  prolog_end_address;
} RUNTIME_FUNCTION, *pRUNTIME_FUNCTION;

#define EXCPT_PD(p)			(p)
#define EXCPT_BEGIN_ADDRESS(p)		((p)->begin_address)
#define EXCPT_END_ADDRESS(p)		((p)->end_address)
#define EXCPT_LANG_HANDLER(p)		((p)->lang_handler)
#define EXCPT_HANDLER_DATA(p)		((p)->handler_data)
#define EXCPT_PROLOG_END_ADDRESS(p)	((p)->prolog_end_address)


#endif /* __alpha */

/* two linker defined variables */
extern RUNTIME_FUNCTION		_fpdata[];		/* base of .pdata */
extern char			_fpdata_size[];		/* number of entries */

#define function_table   	_fpdata
#define function_table_size 	((int) _fpdata_size)


/*
 * The dispatcher context structure.
 *
 * WARNING: this will be changed to make libexcpt reentrant.
*/
typedef struct {
    unsigned long     	pc;			/* current pc in backup */
    pRUNTIME_FUNCTION	functionTable;		/* entry matching pc */
    CONTEXT		originating_context;	/* disp was called with this */
    exc_address		establisher_frame;	/* of handler or highest */
    unsigned long	collide_info;		/* lang specific,helps handle */
    PCONTEXT		current_context;	/* of current handler */
    unsigned long	magic;			/* DISPATCHER_MAGIC */
    unsigned long	reserved5;		/* must be zero */
} DISPATCHER_CONTEXT;
#define DISPATCHER_MAGIC	0xabadabad00beed00

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


/* constants used to identify start and end of gp table */
#define GPINFO_MAGIC	0x0badbadbadf00f00
#define GPINFO_LAST	0xffffffffffffffff

/*
 * function prototypes
*/

#ifdef __cplusplus
extern "C" {
#endif

extern void
exception_dispatcher (
    unsigned long	exception,
    unsigned long	code,
    PCONTEXT		scp)
;
extern void
exc_raise_signal_exception (
    unsigned long	exception,
    unsigned long	code,
    PCONTEXT		scp)
;
extern void
set_unhandled_exception(exception_handler_type handler)
;
extern PRUNTIME_FUNCTION
find_rpd(exc_address	pc)
;
extern void
unwind(
    PCONTEXT		pscp,
    PRUNTIME_FUNCTION	prpd)
;
extern unsigned long
exc_virtual_unwind( PRUNTIME_FUNCTION pcrd, PCONTEXT pcontext )
;
extern unsigned long
exc_find_frame_ptr( PRUNTIME_FUNCTION pcrd, PCONTEXT pcontext, PCONTEXT pnext_context )
;
extern void 
exc_continue (PCONTEXT context_record)
;
extern void 
exc_set_last_chance_handler (exception_handler_type	last_chance)
;
extern void 
exc_raise_status_exception(unsigned long sts)
;
extern void 
exc_raise_exception(system_exrec_type * exception_record)
;
extern void
exc_longjmp(PCONTEXT target_context, long val)
;
extern unsigned long
exc_dispatch_exception (system_exrec_type * exception_record,
		   PCONTEXT            context_record)
;
extern void
exc_unwind_rfp (void		*target_real_frame,
	   void			*target_ip,	/* optional handler addr */
	   system_exrec_type	*exception_record,
	   unsigned long	return_value)
;
extern void
exc_unwind (void			*target_frame,
	   void			*target_ip,	/* optional handler addr */
	   system_exrec_type	*exception_record,
	   unsigned long	return_value)
;
extern unsigned long
exc_capture_context (PCONTEXT	pcontext)
;
extern void
exc_add_pc_range_table(
	PRUNTIME_FUNCTION	pbase,		/* base of function table */
	pdsc_count		count)		/* how many */
;
extern PRUNTIME_FUNCTION
exc_lookup_function_table(exc_address pc)
;
extern void
exc_remove_pc_range_table(
	PRUNTIME_FUNCTION	pbase)		/* base of function table */
;
extern PRUNTIME_FUNCTION
exc_lookup_function_entry(exc_address pc)
;
extern exc_address
exc_lookup_gp(exc_address	pc)
;
extern void
exc_remove_gp_range(
	exc_address		first_addr)	/* base of function table */
;
extern void
exc_add_gp_range(
	exc_address		first_addr,	/* base of function table */
	pdsc_count		length,		/* in bytes */
	exc_address		gp)		/* gp for this range */
;
#ifdef __cplusplus
}
#endif

#include <c_excpt.h>

/* backwards compatability with libexc definitions */

#define EXCEPTION_SUFFIX "_exception_info"

#endif /* __LANGUAGE_C__ */
#endif /* __EXCPT_H */

