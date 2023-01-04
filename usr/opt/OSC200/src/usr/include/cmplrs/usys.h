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
/* $Header: /usr/sde/osf1/rcs/os/src/usr/include/cmplrs/usys.h,v 4.2.8.2 1993/11/19 14:58:04 Ken_Lesniak Exp $ */

#if defined(__LANGUAGE_PASCAL__) || defined(_PASTEL)
#if defined __mips__ || defined __mips64 || defined __alpha

(* system-wide constants and types *)

const 
  Machine  = 2000;                      (* Machine ID.                       *)

  Identlength = 32;                     (* size of identifiers               *)
  Blankid  = '                                ';
#ifdef  _BSD
  Filenamelen = 1024;             	(* maximum length file name in       *)
#endif /* _BSD */
#ifdef  _SYSV
  Filenamelen = 1024;               	(* maximum length file name in       *)
#endif /* _SYSV */
#ifdef  _SYSTYPE_SVR4
  Filenamelen = 1024;               	(* maximum length file name in       *)
#endif /* _SYSTYPE_SYS4 */
					(* target operating system           *)
#if 0	/* Dec-1-87 */
  BlankFilename = '                                                                                    ';
#endif
  Strglgth = Filenamelen;               (* maximum size of a string constant *)
  UseBcode = true;                     (* use binary form of Ucode flag     *)
  HostCharsPerWord = 4;
  HostCharsPerDWord = 8;
  Maxsetsize = 512;                     (* maximum size of a set constant    *)

  (***************************************************************************)
  (* Uscan interface                                                         *)
  (***************************************************************************)
  Maxswitches = 15;                     (* maximum number of switches user   *)
					(* can set in command line           *)
  Maxfiles = 10;                        (* maximum number of files user can  *)
					(* specify in command line           *)

  Charlen  = 32;                        (* Length of component of text file  *)
  Tabsetting = 8;                       (* Spaces per tab                    *)

  (***************************************************************************)
  (* Character set. Must be same on host and target.                         *)
  (***************************************************************************)
  Eolchars = 1;                         (* number of end of line chars       *)
  Eopchar  = 12;                        (* end of page char, usually form    *)
					(* feed                              *)
  Tab      = 9;                         (* ord (tab)                         *)
  Ordlastchar = 127;                    (* Highest ORD of character in a     *)
					(* Text file                         *)


  Wordsize = 32;
  Doublewordsize = 64;
  Wordalign = 32;
  Doublewordalign = 64;
  Bytesize = 8;                         (* size of fixed-length byte         *)
  Setunitsize = 32;
  Setunitmax = 31;
  Defsetsize = 128;                     (* default size of a set             *)

#if 0
  Maxreg   = 63;                        (* >= number of registers on         *)
					(* machine; <= maximum set element   *)
					(* on host compiler.                 *)

  Labchars = 16;                        (* length of external names          *)
  Modchars = 12;                        (* number of significant characters  *)
					(* in a module name                  *)
#endif
  Localsbackwards = true;		(* assign locals in negative         *)
					(* direction?                        *)
#if 0
  Pmem     = true;                      (* use parameter memory?             *)
#endif

  Addrunit = 8;  	                (* size of addressable unit (e.g.    *)
					(* byte size on a byte-addressable   *)
					(* machine)                          *)

  Salign   = 32;                        (* simple types are guaranteed never *)
					(* to cross a boundary of this many  *)
					(* bits                              *)
#if 0
  Regsize  = Wordsize;                  (* size of a register variable       *)
#endif
#if defined(__mips64) || defined(__alpha)
  VarAlign = Doublewordsize;            (* alignment of variables (in bits)  *)
#else
  VarAlign = Wordsize;                  (* alignment of variables (in bits)  *)
#endif
  RecAlign = Addrunit;			(* alignment of fields of unpacked   *)
					(* records                           *)
  ArrAlign = Addrunit;                  (* alignment of elements of unpacked *)
					(* arrays                            *)
  Fpackunit = Addrunit;                 (* alignment of fields of packed     *)
					(* files                             *)
  Rpackunit = 1;			(* alignment of fields of packed     *)
					(* records                           *)
  Apackunit = 1;			(* alignment of elements of packed   *)
					(* arrays                            *)
  Apackeven = true;                     (* pack arrays evenly?               *)
#if defined(__mips64) || defined(__alpha)
  SpAlign  = Doublewordsize;            (* DEFs will always be multiples of  *)
					(* this                              *)
  HeapAlign = Doublewordsize;           (* Sizes of heap variables will      *)
					(* always be multiples of this       *)
#else
  SpAlign  = Wordsize;                  (* DEFs will always be multiples of  *)
					(* this                              *)
  HeapAlign = Wordsize;                 (* Sizes of heap variables will      *)
					(* always be multiples of this       *)
#endif

  Realintsep = false;  (* whether separate static areas for reals and integers,
			   as in FOM *)

  (***************************************************************************)
  (* sizes of unpacked data types, in bits -- must always be a multiple of   *)
  (* Addrunit                                                                *)
  (***************************************************************************)
  Intsize  = Wordsize;                  (* size of integer                   *)
  Intalign = Wordalign;
  Realsize = Wordsize;                  (* size of real                      *)
  Realalign = Wordalign;
  Doublesize = Doublewordsize;		(* size of double                    *)
  Doublealign = Doublewordalign;
  Extendedsize = 3*Wordsize;		(* size of extended		     *)
  Extendedalign = Doublewordalign;
#if 0
#if defined(__mips64) || defined(__alpha)
  Pointersize = Doublewordsize;		(* size of a pointer (address)       *)
  Pointeralign = Doublewordalign;
#else
  Pointersize = Wordsize;		(* size of a pointer (address)       *)
  Pointeralign = Wordalign;
#endif
#endif
  Boolsize = Addrunit;                  (* size of a boolean                 *)
  Boolalign = Addrunit;
  Charsize = Addrunit;                  (* size of a character               *)
  Charalign = Addrunit;
  Pcharsize = 8;			(* minimum packing of characters     *)
#if defined(__mips64) || defined(__alpha)
  Entrysize = 2*Doublewordsize;		(* size of a procedure descriptor    *)
#else
  Entrysize = 2*Wordsize;		(* size of a procedure descriptor    *)
#endif
  Entryalign = Doublewordalign;		(* (type E)                          *)
  Psetsize = 1;                         (* minimum packing of sets           *)
  CharsperSalign = 4;                   (* packed chars per salign unit      *)

  Parthreshold = 2*Wordsize;		(* simple objects larger than this   *)
					(* will be passed indirectly         *)

#if 0
  DefLocalsinregs = 2;                  (* default number of locals in       *)
					(* registers                         *)
  Maxlocalsinregs = 4;                  (* maximum number of locals in       *)
					(* registers                         *)
#endif
  Uoptalloc = true;                     (* whether uopt should perform       *)
					(* register alloc.                   *)
#if 0
  Uoptregs = 8;                        (* number of registers reserved for  *)
					(* uopt                              *)
  Numregclass = 1;                      (* number of classes of registers    *)
					(* (either 1 or 2)                   *)
  Class2reg = 9;                       (* if no 2nd register class, =       *)
					(* uoptregs+1; oe, the first         *)
					(* register which is of the 2nd      *)
					(* class                             *)
  movcost  = 1.2;                       (* the execution cost of a           *)
					(* register-memory transfer          *)
  reglodsave = 1.0;                     (* the saving of loading from a reg  *)
					(* instead of from memory            *)
  regstrsave = 1.0;                     (* the saving of storing to a reg    *)
					(* instead of tomemory               *)

  ParmsinRegs = false;                  (* pass parameters in registers?     *)
  Calleesave = false;                   (* calleesave linkage convention?    *)
#endif
  (***************************************************************************)
  (* for definition of standard types on target machine                      *)
  (***************************************************************************)

  Tgtfirstchar = 0;                     (* lower bound of type char          *)
  Tgtlastchar = 127;                    (* upper bound of type char          *)
  Tgtmaxint = 2147483647;               (* largest integer                   *)
  Tgtminint = -2147483648;		(* smallest integer                  *)
#if 0
  Maxintdiv10 = 214748364;              (* for testing for overflow          *)
  Maxintmod10 = 7;                      (* for testing for overflow          *)
  Tgtmaxexp = 36;                       (* exponent of largest real          *)
  Tgtmaxman = 0.8507059173;             (* mantissa of largest real          *)
  Tgtminexp = -36;                      (* exponent of smallest pos real     *)
  Tgtminman = 0.14693680107;            (* mantissa of smallest pos real     *)
#endif
  Maxsetval = 1151;

  Lngrealsize = DoubleWordsize;
#if 0
  Lngint   = DoubleWordsize;
  Functhreshold = Wordsize;
#endif

  iob_size = 24*addrunit;		(* size of _iobuf struct *)

type
  Identname = packed array[1..Identlength] of char;
  Filename = packed array[1..Filenamelen] of char;

#endif /* defined(__mips__) */
#endif /* defined(__LANGUAGE_PASCAL__) || define(_PASTEL) */

#if defined(__LANGUAGE_C__)
#if defined __mips__ || defined __mips64 || defined __alpha

/* system-wide constants and types */

#define  Machine   2001                 /* Machine ID.                       */
#define  Identlength  32                /* size of identifiers               */
#define  HostCharsPerWord 4
#define  HostCharsPerDWord 8
#define  bytes_per_target_word 4	/* bytes in the target word */
#define  Bytesize  8		        /* size of fixed-length byte         */
#ifdef  _BSD
#define Filenamelen 1024
#endif  /* _BSD */
#ifdef  _SYSV
#define Filenamelen 1024
#endif  /* _SYSV */
#ifdef  _SYSTYPE_SVR4
#define Filenamelen 1024
#endif  /* _SYSTYPE_SVR4 */
#define  Strglgth   Filenamelen         /* maximum size of a string constant */

typedef char Filename[Filenamelen];
typedef char Identname[Identlength];

#else /* !defined(__mips__) */

/* system-wide constants and types */
#define   Machine  11		/* Machine ID. */
#define   Identlength  32	/* size of identifiers */
#define   Strglgth  288		/* maximum size of a string constant */
#define   HostCharsPerWord 4

typedef char Identname[Identlength];

#endif /* defined(__mips__)			   */
#endif /* define(__LANGUAGE_C__) 		   */

