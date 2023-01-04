/* BuildSystemHeader added automatically */
/* $Header: /usr/sde/x11/rcs/x11/src/./clients/emacs/src/m-alpha.h,v 1.2 92/08/25 17:24:06 devrcs Exp $ */

/* The following three symbols give information on
 the size of various data types.  */

#define SHORTBITS 16		/* Number of bits in a short */

#define INTBITS 32L		/* Number of bits in an int */

#define LONGBITS 64L		/* Number of bits in a long */

/* Define BIG_ENDIAN iff lowest-numbered byte in a word
   is the most significant byte.  */

/* #define BIG_ENDIAN */

/* Define NO_ARG_ARRAY if you cannot take the address of the first of a
 * group of arguments and treat it as an array of the arguments.  */

#define NO_ARG_ARRAY

/* Define WORD_MACHINE if addresses and such have
 * to be corrected before they can be used as byte counts.  */

/* #define WORD_MACHINE */

/* Define how to take a char and sign-extend into an int.
   On machines where char is signed, this is a no-op.  */

#define SIGN_EXTEND_CHAR(c) ((signed char)(c))

/* Now define a symbol for the cpu type, if your compiler
   does not define it automatically:
   Ones defined so far include vax, m68000, ns16000, pyramid,
   orion, tahoe, APOLLO and many others */

/* Use type int rather than a union, to represent Lisp_Object */
/* This is desirable for most machines.  */

#define NO_UNION_TYPE

/* Define EXPLICIT_SIGN_EXTEND if XINT must explicitly sign-extend
   the 24-bit bit field into an int.  In other words, if bit fields
   are always unsigned.

   If you use NO_UNION_TYPE, this flag does not matter.  */

#define EXPLICIT_SIGN_EXTEND

/* Data type of load average, as read out of kmem.  */
/* Load average requires special crocks.  Version 19 has them.
   For now, don't define this.  */

/* #define LOAD_AVE_TYPE long */

/* Convert that into an integer that is 100 for a load average of 1.0  */

/* #define LOAD_AVE_CVT(x) (int) (((double) (x)) * 100.0 / 256.0) */

/* Define CANNOT_DUMP on machines where unexec does not work.
   Then the function dump-emacs will not be defined
   and temacs will do (load "loadup") automatically unless told otherwise.  */

/* #define CANNOT_DUMP */

/* Define VIRT_ADDR_VARIES if the virtual addresses of
   pure and impure space as loaded can vary, and even their
   relative order cannot be relied on.

   Otherwise Emacs assumes that text space precedes data space,
   numerically.  */

/* #define VIRT_ADDR_VARIES */

/* Define C_ALLOCA if this machine does not support a true alloca
   and the one written in C should be used instead.
   Define HAVE_ALLOCA to say that the system provides a properly
   working alloca function and it should be used.
   Define neither one if an assembler-language alloca
   in the file alloca.s should be used.  */

#define HAVE_ALLOCA

/* Define NO_REMAP if memory segmentation makes it not work well
   to change the boundary between the text section and data section
   when Emacs is dumped.  If you define this, the preloaded Lisp
   code will not be sharable; but that's better than failing completely.  */

#define NO_REMAP

/* This machine requires completely different unexec code
   which lives in a separate file.  Specify the file name.  */

#define UNEXEC unexmips.o

/* Describe layout of the address space in an executing process.  */

#define TEXT_START    0x120000000     /* user text starts at  9*2^29 */
#define DATA_START    0x140000000     /* user data starts at 10*2^29 */

/*
 * Cast pointers to this value on Alpha machines.
 */
#define PNTR_COMPARISON_TYPE unsigned long

#define NO_UNION_TYPE

#if defined(NO_UNION_TYPE)
/*
 * These definitions do NOT work properly on Alpha, since these bit
 * operations result in strange addresses if we are not longword aligned.
 */
#define VALBITS		56L
/* #define VALMASK	((1L<<VALBITS) - 1) */
#define VALMASK		0x00ffffffffffffffL
#define MARKBIT		0x8000000000000000L
#define ARRAY_MARK_FLAG 0x4000000000000000L

#define XSETTYPE(a, b)	((a)  =  XUINT (a) | ((long)(b) << VALBITS))
#define XINT(a)		(((long)(a) << LONGBITS-VALBITS) >> LONGBITS-VALBITS)
#define XUINT(a)	((long)(a) & VALMASK)
#define XSETINT(a, b)	((a) = (long)((a) & ~VALMASK) | (long)((b) & VALMASK))
#define XSET(var, type, ptr) \
   ((var) = (long)((long)(type) << VALBITS) + ((long) (ptr) & VALMASK))

#define XMARKBIT(a) ((a) & MARKBIT)
#define XSETMARKBIT(a,b) ((a) = ((a) & ~MARKBIT) | (b))
#define XMARK(a) ((a) |= MARKBIT)
#define XUNMARK(a) ((a) &= ~MARKBIT)

#endif /* defined(NO_UNION_TYPE */

#define PURESIZE 	264000
