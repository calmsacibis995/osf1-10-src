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
 * Purpose:		contain data structures for alpha calling standard
 *
 * Prefix:		pdsc and PDSC
 *
 * Date started:	5/13/92
 *
 *			6/3/92
 *				massive changes per calling standard meeting
 *
 *			6/4/92
 *				- reverse bit on prologue
 *				- bytes go to 32 bit words
 *				- entry_ra in short form register frame
 *				- fix macros
 *				- fmask in short form to $f2..$f9 instead of $f7
 *				- add, rearrange, remove FLAGS fields
 *				- fix typeos
 *
 *			6/5/92
 *				- rearrange fields so all reg/stack fieldds 
 *					are in first word.
 *				- change sense of REINVOKEABLE flag
 *				- moved up EXTENDER flag
 *
 *			6/22
 *				- rsa_offset & frame_size to quads
 *				- entry_ra to move in short form
 *
 *			9/29
 *				- changed PDSC_FLAGS_HANDLER_VALID to 0x8
 *				- added math library caller id flags
 *
 */

/*

		Code Range Descriptors (crd)

	+---------------------------------------------------------------+
	+31                                                         |1 0+
	+                        begin_address                      |RES+
	+---------------------------------------------------------------+
	+31                                                        2|1|0+
	+                        rpd_offset                         |R|P+
	+---------------------------------------------------------------+


		begin_address	code range start address - _fpdata
		_fpdata		start of .pdata section
		rpd_offset      rpd address - my address
		rpd		runtime procedure descriptor
		my address	address of 32 bit word
		RES,R		must be zero, reserved for future use
		P		if 0, code range starts with contains prolog
				(this was reversed to favor 95% case)

	Notes

	-   both 32 bit words will be treated as 32 bit words. The low two 
	    bits in the second word masked out when the second word is used as 
	    an offset.
	-   rpd_offset may equal zero if the procedure is a null frame whose
	    whose entry_ra is $26 and contains no handler and
	    PDSC_FLAGS_REGISTER_FRAME is set.
	    This implies no separate procedure descriptor.
	-   It was suggested that null frame remap the second word to
	    remove the need for an rpd. We already have the case above where
	    rpd_offset is 0 to take care of most situations. In order to 
	    minimize complexity, I have left it out and suggest it stay
	    left out until someone comes up with the numbers which say it
	    really saves space.
	

		Runtime Procedure Descriptors (rpd)

			Short Form (short_[reg|stack]_rpd):


        +---------------------------------------------------------------+
        +31           24|23           16|15            8|7             0+
        +     imask     |      fmask    |    rsa_offset |     flags     +
        +---------------------------------------------------------------+
        +31           24|23           16|15                            0+
        + entry_length  |     sp_set    |            frame_size         +
        +---------------------------------------------------------------+
	+31                                                            0+
	+                        handler_address                        +
	+- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -+
	+63                                                           32+
	+                        (optional quad)                        +
	+---------------------------------------------------------------+
	+31                                                            0+
	+                        handler_data_address                   +
	+- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -+
	+63                                                           32+
	+                        (optional quad)                        +
	+---------------------------------------------------------------+


		alternate first 32 bit word for register frames
        +---------------------------------------------------------------+
        +31                21|20      16|15    11|     8|7             0+
        +     reserved       |  save_ra |entry_ra|reserv|     flags     +
        +---------------------------------------------------------------+


	stack_frame_size	size in 64 bit words of frame (max 65535)
	rsa_offset		offset in 64 bit words from sp or fp to 
				register save area (max 255). 
	flags			see below (flags fitting in 8 bits)
	entry_length		number of instructions in the prologue (max 255)
	sp_set			instruction offset to instruction which sets sp
				in the prologue (max 255)
	imask			bit set for regs $8..$15 which are saved
				This will be save_ra when
				the PDSC_FLAGS_REGISTER_FRAME  flag bit is set.
	entry_ra		ra on entry to the routine.
	save_ra			where we move entry_ra in prolog. If save_ra
				equals entry_ra, then it's a null frame.
	fmask			bit set for regs $f2..$f9 which are saved
				This will be entry_ra when
				the PDSC_FLAGS_REGISTER_FRAME  flag bit is set.
	handler_address		if present, provides absolute address of 
				exception handler
	handler_data_address	if present, provides absolute address of 
				exception handler data

	Notes

	-   entry_ra must be $26.
	-   if any field is exceeded, you must use long form.
		it is always ok to use long form.
	-   if a register frame has save_ra == entry_ra, then it is a null
		frame. This removes the need for yet another frame structure.


			Long Form (long_[reg|stack]_rpd):

	+---------------------------------------------------------------+
	+31                           16|15    11|10                   0+
	+        rsa_offset             |entry_ra|        flags         +
	+---------------------------------------------------------------+
	+31                           16|15                            0+
	+        entry_length           |           sp_set              +
	+---------------------------------------------------------------+
	+31                                                            0+
	+                        stack_frame_size                       +
	+---------------------------------------------------------------+
	+31                                                            0+
	+                            reserved                           +
	+---------------------------------------------------------------+
	+31                                                            0+
	+                              imask                            +
	+---------------------------------------------------------------+
	+31                                                            0+
	+                              fmask                            +
	+---------------------------------------------------------------+
	+31                                                            0+
	+                        handler_address                        +
	+- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -+
	+63                                                           32+
	+                        (optional quad)                        +
	+---------------------------------------------------------------+
	+31                                                            0+
	+                        handler_data_address                   +
	+- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -+
	+63                                                           32+
	+                        (optional quad)                        +
	+---------------------------------------------------------------+


		alternate first 32 bit word for register frames
	+---------------------------------------------------------------+
	+31                  21|20    16|15    11|10                   0+
	+        reserved      | save_ra|entry_ra|        flags         +
	+---------------------------------------------------------------+


	see above for field definitions (these are in same units except
	larger fields; masks represent all 32 registers).


*/

#ifndef __PDSC_H_
#define __PDSC_H_

#if defined(_LANGUAGE_C) || defined(__LANGUAGE_C__) || defined(__cplusplus) || defined(__LANGUAGE_ASSEMBLY__) || defined(_LANGUAGE_ASSEMBLY)

/* math library routine behavior is determined by two flag bits in the math
 *	library routine's callers procedure descriptor. The following
 *	macros help define those values.
 */
#define PDSC_EXC_MASK		0x30		/* mask flags field */
#define PDSC_EXC_SILENT		0x00		/* under->0, no signal */
#define PDSC_EXC_SIGNAL		0x10		/* under->0, signal rest */
#define PDSC_EXC_SIGNAL_ALL	0x20		/* signal all */
#define PDSC_EXC_IEEE		0x30		/* respect ieee control reg */

/* 
 * specific flags for the flags field in pdsc_rpd
 */
#define	PDSC_FLAGS_SHORT		0x001	/* short form */
#define	PDSC_FLAGS_REGISTER_FRAME	0x002	/* register versus stack */
#define	PDSC_FLAGS_BASE_REG_IS_FP	0x004	/* FP is frame ptr */
#define	PDSC_FLAGS_HANDLER_VALID	0x008	/* => stack_handler */
#define	PDSC_FLAGS_EXC_TYPE_1		0x010	/* used to determine math lib */
#define	PDSC_FLAGS_EXC_TYPE_2		0x020	/*  exception behavior	*/

/* The following flag allows us to traceback over signal frames.
 *	NT doesn't have this flag and they disassemble a funky fake
 *	prologue which does two saves of ra which the back execution
 *	mechanism
 */
#define	PDSC_FLAGS_EXCEPTION_FRAME	0x040	/* exception frame, for epc */
#define	PDSC_FLAGS_EXTENDER		0x080	/* reserved for escaping to larger flag area in optional fields */
#define	PDSC_FLAGS_RESERVED1		0x100	/* reserved */
#define	PDSC_FLAGS_RESERVED2		0x200	/* reserved */
#define	PDSC_FLAGS_RESERVED3		0x400	/* reserved */
#endif


#if defined(_LANGUAGE_C) || defined(__LANGUAGE_C__) || defined(__cplusplus)
typedef const unsigned long	pdsc_uconst;	/* unsigned constant */
typedef const long		pdsc_sconst;	/* signed constant */
typedef unsigned int		pdsc_mask;	/* or'ed mask bits */
typedef unsigned char		pdsc_uchar_mask;/* 8 bit or'ed mask bits */
typedef unsigned int		pdsc_space;	/* unused or mbz space */
typedef unsigned int		pdsc_count;	/* 32bit count of something */
typedef int			pdsc_offset;	/* 32 bit offset to something */
typedef unsigned char		pdsc_uchar_offset;/* 8bit offset to something */
typedef unsigned short		pdsc_ushort_offset;/* 16bit offset to somthng */
typedef unsigned short		pdsc_register;	/* use regs.h constants */
typedef unsigned long		pdsc_register_value;/* reg contents */
typedef unsigned long		pdsc_address;	/* address of something */
typedef short			pdsc_sint_16;	/* signed 16 bit int */
typedef unsigned short		pdsc_uint_16;	/* unsigned 16 bit int */
typedef int			pdsc_sint_32;	/* signed 32 bit int */
typedef unsigned int		pdsc_uint_32;	/* unsigned 32 bit int */


/*
 * Runtime Procedure Descriptor
 */

typedef union pdsc_rpd {

    struct pdsc_short_stack_rpd {
	pdsc_uchar_mask	flags:8;	/* info about frame */
	pdsc_uchar_offset rsa_offset;	/* savregs offst in quadwords */
	pdsc_uchar_mask	fmask:8;	/* floating point reg mask */
	pdsc_uchar_mask	imask:8;	/* integer register mask */
	pdsc_count	frame_size:16;	/* frame size in 64 bit words */
	pdsc_offset	sp_set:8;	/* instofset to inst which sets sp */
	pdsc_count	entry_length:8;	/* # of insts in prologue */
    } short_stack_rpd;

    struct pdsc_short_reg_rpd {
	pdsc_uchar_mask	flags:8;	/* info about frame */
	pdsc_space	reserved1:3;	/* must be zero */
	pdsc_register	entry_ra:5;	/* what contains ra on entry */
	pdsc_register	save_ra:5;	/* entry_ra here after prolog */
	pdsc_space	reserved2:11;	/* must be zero */
	pdsc_count	frame_size:16;	/* frame size in 64 bit words */
	pdsc_offset	sp_set:8;	/* instofset to inst which sets sp */
	pdsc_count	entry_length:8;	/* # of insts in prologue */
    } short_reg_rpd;

    struct pdsc_long_stack_rpd {
	pdsc_mask	flags:11;	/* info about frame */
        pdsc_register	entry_ra:5;	/* where ret pc is on entry */
	pdsc_ushort_offset rsa_offset;	/* saveregs offst in quad words */
	pdsc_offset	sp_set:16;	/* instofset to inst which sets sp */
	pdsc_count	entry_length:16;/* # of insts in prologue */
	pdsc_count	frame_size;	/* frame size in quad words */
        pdsc_space	reserved;	/* must be zero */
	pdsc_mask	imask;		/* integer register mask */
	pdsc_mask	fmask;		/* floating point register mask */
    } long_stack_rpd;

    struct pdsc_long_reg_rpd {
	pdsc_mask	flags:11;	/* info about frame */
        pdsc_register	entry_ra:5;	/* where ret pc is on entry */
	pdsc_register	save_ra:5;	/* we moved entry_ra in prolog */
	pdsc_space	reserved1:11;	/* must be zero */
	pdsc_offset	sp_set:16;	/* instofset to inst which sets sp */
	pdsc_count	entry_length:16;/* # of insts in prologue */
	pdsc_count	frame_size;	/* frame size in quad words */
        pdsc_space	reserved2;	/* must be zero */
	pdsc_mask	imask;		/* integer register mask */
	pdsc_mask	fmask;		/* floating point register mask */
    } long_reg_rpd;

    struct pdsc_short_with_handler {
	union {
	    struct pdsc_short_stack_rpd	short_stack_rpd;  /* base stack rpd */
	    struct pdsc_short_reg_rpd	short_reg_rpd;  /* base stack rpd */
	} stack_or_reg;
	pdsc_address	handler;       		/* optional handler address */
	pdsc_address	handler_data;  		/* optional handler data */
    } short_with_handler;


    struct pdsc_long_with_handler {
	union {
	    struct pdsc_long_stack_rpd	long_stack_rpd;  /* base stack rpd */
	    struct pdsc_long_reg_rpd	long_reg_rpd;  /* base stack rpd */
	} stack_or_reg;
	pdsc_address	handler;       		/* optional handler address */
	pdsc_address	handler_data;  		/* optional handler data */
    } long_with_handler;

} pdsc_rpd;		/* runtime procedure descriptor */


/* size defines mentioned in spec */
#define PDSC_SHORT_RPD_SIZE		sizeof(struct pdsc_short_stack_rpd)
#define PDSC_LONG_RPD_SIZE		sizeof(struct pdsc_long_stack_rpd)
#define PDSC_DEFAULT_ENTRY_RA		26

/* defines to make field access easier, do not use incrementing pointers 
 *	as arguments since we use the argument twice.
 */
#define PDSC_RPD_SHORT(p)	((p)->short_reg_rpd.flags&PDSC_FLAGS_SHORT)
#define PDSC_RPD_REGISTER(p)	((p)->short_reg_rpd.flags&PDSC_FLAGS_REGISTER_FRAME)
#define PDSC_RPD_HAS_HANDLER(p)	((p)->short_reg_rpd.flags&PDSC_FLAGS_HANDLER_VALID)
#define PDSC_RPD_FLAGS(p)	(!(p) ? PDSC_FLAGS_REGISTER_FRAME : \
	PDSC_RPD_SHORT(p) ? (p)->short_reg_rpd.flags : (p)->long_reg_rpd.flags)
#define PDSC_RPD_RSA_OFFSET_FIELD(p)	(!(p) ? 0 : PDSC_RPD_SHORT(p) ? \
	(p)->short_stack_rpd.rsa_offset :  \
	(p)->long_stack_rpd.rsa_offset)
#define PDSC_RPD_RSA_OFFSET(p) \
	(PDSC_RPD_RSA_OFFSET_FIELD(p)<<PDSC_FRAME_SIZE_SHIFT)
#define PDSC_RPD_SAVE_RA(p)	\
	((!(p) || !PDSC_RPD_REGISTER(p)) ? PDSC_DEFAULT_ENTRY_RA : \
	 PDSC_RPD_SHORT(p) ? \
	 (p)->short_reg_rpd.save_ra : \
	 (p)->long_reg_rpd.save_ra)
#define PDSC_RPD_ENTRY_RA(p)	\
	(!(p) ? PDSC_DEFAULT_ENTRY_RA : \
	 PDSC_RPD_SHORT(p) ? \
	 (!PDSC_RPD_REGISTER(p) ? \
	  PDSC_DEFAULT_ENTRY_RA : (p)->short_reg_rpd.entry_ra) : \
	 (p)->long_reg_rpd.entry_ra)
#define PDSC_RPD_SIZE_FIELD(p)	(!(p) ? 0 : PDSC_RPD_SHORT(p) ? \
	(p)->short_stack_rpd.frame_size : (p)->long_stack_rpd.frame_size)
#define PDSC_RPD_SIZE(p)	(PDSC_RPD_SIZE_FIELD(p)<<PDSC_FRAME_SIZE_SHIFT)
#define PDSC_RPD_SP_SET_FIELD(p)(!(p) ? 0 : PDSC_RPD_SHORT(p) ? \
	(p)->short_stack_rpd.sp_set : (p)->long_stack_rpd.sp_set)
#define PDSC_RPD_SP_SET(p) (PDSC_RPD_SP_SET_FIELD(p)<<PDSC_INST_OFFSET_SHIFT)
#define PDSC_RPD_ENTRY_LENGTH_FIELD(p) (!(p) ? 0 : PDSC_RPD_SHORT(p) ? \
	(p)->short_stack_rpd.entry_length : (p)->long_stack_rpd.entry_length)
#define PDSC_RPD_ENTRY_LENGTH(p) \
	(PDSC_RPD_ENTRY_LENGTH_FIELD(p)<<PDSC_INST_OFFSET_SHIFT)
#define PDSC_RPD_IMASK(p)	(!(p) ? 0 : PDSC_RPD_SHORT(p) ? \
	(p)->short_stack_rpd.imask << PDSC_SHORT_RPD_IMASK_SHIFT :\
	(p)->long_stack_rpd.imask)
#define PDSC_RPD_FMASK(p)	(!(p) ? 0 : PDSC_RPD_SHORT(p) ? \
	(p)->short_stack_rpd.fmask << PDSC_SHORT_RPD_FMASK_SHIFT :\
	(p)->long_stack_rpd.fmask)
#define PDSC_RPD_HANDLER(p)	((!(p) || !PDSC_RPD_HAS_HANDLER(p)) ? 0 : \
	PDSC_RPD_SHORT(p) ? \
	(p)->short_with_handler.handler :(p)->long_with_handler.handler)
#define PDSC_RPD_HANDLER_DATA(p)	(!(p) ? 0 : PDSC_RPD_SHORT(p) ? \
	(p)->short_with_handler.handler_data : \
	(p)->long_with_handler.handler_data)

/*
 * Runtime Code Range Descriptor
 */
typedef union pdsc_crd {
	struct {
	    pdsc_offset	begin_address;	/* offst to 1st addr in range */
	    pdsc_offset	rpd_offset;	/* offst to rpd includng bits */
	} words;
	struct {
	    pdsc_space	reserved1:2;		/* must be 0 */
	    pdsc_offset	shifted_begin_address:30;/* shifted left 2 */
	    pdsc_mask	no_prolog:1;		/* flag */
	    pdsc_space	reserved2:1;		/* must be 0 */
	    pdsc_offset	shifted_rpd_offset:30;	/* shifted left 2 */
	} fields;
} pdsc_crd;

#define PDSC_CRD_OFFSET_MASK 0xfffffffffffffffc	/* mask off bottom bits */
#define PDSC_CRD_NO_PROLOG_MASK 0x1		/* get bottom bit */

#define PDSC_CRD_PRPD(p)  ((pdsc_rpd *) 	/* pointer to rpd */	\
    ((!(p) || ((p)->words.rpd_offset == 0)) ? 0 : \
     (pdsc_address)(((long)&((p)->words.rpd_offset)) + \
     (long)((p)->words.rpd_offset&PDSC_CRD_OFFSET_MASK))))

#define PDSC_CRD_CONTAINS_PROLOG(p)		/* 0 => range contains prolg */\
	(!(p)->fields.no_prolog)

#define PDSC_CRD_BEGIN_ADDRESS_FIELD(p)	\
	((p)->words.begin_address&PDSC_CRD_OFFSET_MASK)

#define PDSC_CRD_BEGIN_ADDRESS(ftable_base, p)	/* actual start address */ \
	(((pdsc_address)(ftable_base)) + PDSC_CRD_BEGIN_ADDRESS_FIELD(p))

#define PDSC_INST_OFFSET_SHIFT	2UL	/* how much to shift byte size */
#define PDSC_FRAME_SIZE_SHIFT	3UL	/* how much to shift byte size */
#define PDSC_SHORT_RPD_IMASK_SHIFT 8UL	/* $8..$15 */
#define PDSC_SHORT_RPD_FMASK_SHIFT 2UL	/* $f2..$f9 */
#endif /* __LANGUAGE_C__ */

#if defined(_LANGUAGE_PASCAL) || defined(__LANGUAGE_PASCAL__)
#define PDSC_INST_OFFSET_SHIFT	2	/* how much to shift byte size */
#define PDSC_FRAME_SIZE_SHIFT	3	/* how much to shift byte size */
#define PDSC_SHORT_RPD_IMASK_SHIFT 8	/* $8..$15 */
#define PDSC_SHORT_RPD_FMASK_SHIFT 2	/* $f2..$f9 */

type

pdsc_address = integer64;
/* math library routine behavior is determined by two flag bits in the math
 *	library routine's callers procedure descriptor. The following
 *	macros help define those values.
 */
#define PDSC_EXC_MASK		16#30		/* mask flags field */
#define PDSC_EXC_SILENT		16#00		/* under->0, no signal */
#define PDSC_EXC_SIGNAL		16#10		/* under->0, signal rest */
#define PDSC_EXC_SIGNAL_ALL	16#20		/* signal all */
#define PDSC_EXC_IEEE		16#30		/* respect ieee control reg */

/* 
 * specific flags for the flags field in pdsc_rpd
 */
#define	PDSC_FLAGS_SHORT		16#001	/* short form */
#define	PDSC_FLAGS_REGISTER_FRAME	16#002	/* register versus stack */
#define	PDSC_FLAGS_BASE_REG_IS_FP	16#004	/* FP is frame ptr */
#define	PDSC_FLAGS_HANDLER_VALID	16#008	/* => stack_handler */
#define	PDSC_FLAGS_EXC_TYPE_1		16#010	/* used to determine math lib */
#define	PDSC_FLAGS_EXC_TYPE_2		16#020	/*  exception behavior	*/
#define	PDSC_FLAGS_EXCEPTION_FRAME	16#040	/* exception frame, for epc */
#define	PDSC_FLAGS_EXTENDER		16#080	/* reserved for escaping to larger flag area in optional fields */
#define	PDSC_FLAGS_RESERVED1		16#100	/* reserved */
#define	PDSC_FLAGS_RESERVED2		16#200	/* reserved */
#define	PDSC_FLAGS_RESERVED3		16#400	/* reserved */

#define PDSC_1BIT	0..16#1
#define PDSC_2BITS	0..16#3
#define PDSC_3BITS	0..16#7
#define PDSC_4BITS	0..16#f
#define PDSC_5BITS	0..16#1f
#define PDSC_6BITS	0..16#3f
#define PDSC_7BITS	0..16#7f
#define PDSC_8BITS	0..16#ff
#define PDSC_10BITS	0..16#3ff
#define PDSC_11BITS	0..16#7ff
#define PDSC_12BITS	0..16#fff
#define PDSC_16BITS	0..16#ffff
#define PDSC_30BITS	0..16#3fffffff
#define PDSC_32BITS	integer

/*
 * Runtime Procedure Descriptor
 */



pdsc_short_rpd = packed record

    flags:		PDSC_8BITS;	/* info about frame */
    /*stack_or_reg:	pdsc_short_stack_or_reg;/* stack/reg frame specific */
    case integer of
    0: (
        rsa_offset:	PDSC_8BITS;	/* save regs offset in 32 bit words */
	fmask:		PDSC_8BITS;   	/* floating point  register mask */
	imask:		PDSC_8BITS;   	/* integer register mask */
    );
    1: (
        reserved1:	PDSC_3BITS;	/* must be zero */
	entry_ra:	PDSC_5BITS;	/* where ret pc is on entry */
	save_ra:	PDSC_5BITS;	/* where ret pc is on entry */
        reserved2:	PDSC_11BITS;	/* must be zero */
	frame_size:	PDSC_16BITS;	/* frame size in 32 bit words */
	sp_set:		PDSC_8BITS;	/* instofset to inst which sets sp */
	entry_length:	PDSC_8BITS;	/* # of insts in prologue */
    );

end; { short_rpd }

pdsc_long_rpd = packed record

    flags:		PDSC_11BITS;	/* info about frame */
    entry_ra:		PDSC_5BITS;	/* where ret pc is on entry */
    case integer of
    0: (
	rsa_offset:	PDSC_16BITS;	/* save regs offset in 32 bit words */
    );
    1: (
	save_ra:	PDSC_5BITS;	/* where ret pc is after prolog */
        reserved1:	PDSC_11BITS;	/* must be zero */
	sp_set:		PDSC_16BITS;	/* instofset to inst which sets sp */
	entry_length:	PDSC_16BITS;	/* # of insts in prologue */
	frame_size:	PDSC_32BITS;	/* frame size in 32 bit words */
	reserved2:	PDSC_32BITS;	/* must be zero */
	imask:		PDSC_32BITS;   	/* integer register mask */
	fmask:		PDSC_32BITS;   	/* floating point  register mask */
    );

end; { long_rpd }


pdsc_short_rpd_with_handler = packed record
    short_rpd:		pdsc_short_rpd;	/* base stack frame stuff */
    handler:		pdsc_address;  	/* optional handler address */
    handler_data:	pdsc_address;  	/* optional handler data */
end;

pdsc_long_rpd_with_handler = packed record
    long_rpd:		pdsc_long_rpd;	/* base stack frame stuff */
    handler:		pdsc_address;  	/* optional handler address */
    handler_data:	pdsc_address;  	/* optional handler data */
end;

pdsc_rpd = packed record
	case cardinal of
	0: (short_rpd:			pdsc_short_rpd);
	1: (short_rpd_with_handler:	pdsc_short_rpd_with_handler);
	2: (long_rpd:			pdsc_long_rpd);
	3: (long_rpd_with_handler:	pdsc_long_rpd_with_handler);
	5: (quad:		array[0..4] of cardinal64);
	6: (word:		array[0..9] of cardinal);
end;

#define PDSC_SHORT_RPD_FRAME_SIZE_MAX	cardinal(16#ffff)

/*
 * Code Range Descriptor
 */
pdsc_crd = packed record
    case integer of
    0: (quad:	cardinal64);
    1: (
	case integer of
	0: (
	    begin_address:PDSC_32BITS;	/* offst to 1st addr in range */
	    rpd_offset:PDSC_32BITS;	/* offset to rpd including bits */
	);
	1: (
	    reserved1:	PDSC_2BITS;	/* must be 0 */
	    shifted_begin_address:PDSC_30BITS;/* offst to 1st addr in range */
	    no_prolog:	PDSC_1BIT;	/* 1 => no prolog in this range */
	    reserved2:	PDSC_1BIT;	/* must be 0 */
	    shifted_rpd_offset:PDSC_30BITS;	/* shifted left 2 */
	);
    );

end;

#endif /* __LANGUAGE_PASCAL__ */


#endif /* PDSC_H */
