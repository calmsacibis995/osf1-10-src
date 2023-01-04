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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
static char rcsid[] = "@(#)$RCSfile: tdef.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/10/14 04:18:07 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*	tdef.h	4.8	87/04/30	*/

#include <sys/param.h>
#undef CMASK			/* XXX */
#undef BIG			/* XXX */
#define MAXPTR (-1)		/* max value of any pointer variable */
#ifdef NROFF	/*NROFF*/
#define EM t.Em
#define HOR t.Hor
#define VERT t.Vert
#define INCH 240	/*increments per inch*/
#define SPS INCH/10	/*space size*/
#define SS INCH/10	/* " */
#define TRAILER 0
#define UNPAD 0227
#define PO 0 /*page offset*/
#define ASCII 1
#define PTID 1
#define LG 0
#define DTAB 0	/*set at 8 Ems at init time*/
#define ICS 2*SPS
#define TEMP 256	/*65K*/
#endif
#ifndef NROFF	/*TROFF*/
#define INCH 432	/*troff resolution*/
#define SPS 20	/*space size at 10pt; 1/3 Em*/
#define SS 12	/*space size in 36ths of an em*/
#define TRAILER 6048	/*144*14*3 = 14 inches*/
#define UNPAD 027
#define PO 416 /*page offset 26/27ths inch*/
#define HOR 1
#define VERT 3
#define EM (6*(pts&077))
#define ASCII 0
#define PTID 0
#define LG 1
#define DTAB (INCH/2)
#define ICS 3*SPS
#define TEMP 512	/*128K*/
#endif

#include <signal.h>
#define NARSP 0177	/*narrow space*/
#define HNSP 0226	/*half narrow space*/
#define PS 10	/*default point size*/
#define FT 0	/*default font position*/
#define LL 65*INCH/10	/*line length; 39picas=6.5in*/
#define VS INCH/6	/*vert space; 12points*/
#ifdef VMUNIX
#define NN 528	/*number registers*/
#else
#define NN 132	/*number registers*/
#endif
/* #define NN 200	*/
#define NNAMES 14 /*predefined reg names*/
#define NIF 15	/*if-else nesting*/
#define NS 256	/*name buffer*/
#define NTM 256	/*tm buffer*/
#define NEV 3	/*environments*/
#define EVLSZ 10	/*size of ev stack*/
/* #define EVS 4*256	*/
#ifdef VMUNIX
#define NM 600
#define EVS 6*256	/*environment size in words*/
#else
#define NM 300	/*requests + macros*/
#define EVS 3*256	/*environment size in words*/
#endif
#define DELTA 512	/*delta core bytes*/
#define NHYP 10	/*max hyphens per word*/
#define NHEX 128	/*byte size of exception word list*/
#define NTAB 35	/*tab stops*/
#define NSO 5	/*"so" depth*/
#ifdef VMUNIX
#define WDSIZE 340	/*word buffer size*/
#define LNSIZE 960	/*line buffer size*/
#else
#define WDSIZE 170	/*word buffer size*/
#define LNSIZE 480	/*line buffer size*/
#endif
/* #define LNSIZE 680	*/
#define NDI 5	/*number of diversions*/
#define DBL 0100000	/*double size indicator*/
#define MOT 0100000	/*motion character indicator*/
#define MOTV 0160000	/*clear for motion part*/
#define VMOT 0040000	/*vert motion bit*/
#define NMOT 0020000	/* negative motion indicator*/
#define MMASK 0100000	/*macro mask indicator*/
#define CMASK 0100377
#define ZBIT 0400	/*zero width char*/
#define BMASK 0377
#define BYTE 8
#define IMP 004	/*impossible char*/
#define FILLER 037
#define PRESC 026
#define HX 0376	/*High-order part of xlss*/
#define LX 0375	/*low-order part of xlss*/
#define CONT 025
#define COLON 013
#define XPAR 030
#define ESC 033
#define FLSS 031
#define RPT 014
#define JREG 0374
#define NTRAP 20	/*number of traps*/
#define NPN 20	/*numbers in "-o"*/
#define T_PAD 0101	/*cat padding*/
#define T_INIT 0100
#define T_IESC 16 /*initial offset*/
#define T_STOP 0111
#define NPP 10	/*pads per field*/
#ifdef VMUNIX
#define FBUFSZ 1024
#else
#define FBUFSZ 256	/*field buf size words*/
#endif
#define OBUFSZ 8192	/*bytes*/
#define IBUFSZ 8192	/*bytes*/
#define NC 256	/*cbuf size words*/
#define NOV 10	/*number of overstrike chars*/
#define TDELIM 032
#define LEFT 035
#define RIGHT 036
#define LEADER 001
#define TAB 011
#define TMASK  037777
#define RTAB 0100000
#define CTAB 0040000
#define OHC 024

#define PAIR(A,B) (A|(B<<BYTE))

#define BLK  128	/*alloc block words*/

#ifdef VMUNIX
#define	BIG 1024
#endif /* VMUNIX */

#ifdef BIG
typedef long filep;
#define NBLIST BIG	/*allocation , BIG = 256 per 65k*/
#define BLKBITS 7	/*for BLK=128*/
#endif
#ifndef BIG
typedef unsigned filep;
#define NBLIST TEMP	/*allocation list, TEMP<=512*/
/* BLK*NBLIST<=65536 words, if filep=unsigned */
#define BLKBITS 0
#endif

/* +++ WW001 +++ */

/* CMASK & BMASK have to be redefined again */
#undef	CMASK
#undef	BMASK
#undef	PAIR

#define TRUE	1
#define FALSE	0
#define	MB_MAX	4	/* Maximum number of bytes in a multi-byte char */

/* Redefine BMASK & CMASK for the new encoding scheme */
#define	BMASK		0x7fff00ff
#define CMASK		0x7fff80ff
#define AMASK		0x7fff0000	/* Mask for Asian char info */
#define HI_WORD_MASK	0xffff0000
#define	LO_WORD_MASK	0x0000ffff
#define	LO_CHAR_MASK	0x00007fff	/* Bit 15 is turned off */
#define	LO_BYTE_MASK	0x000000ff
#define	LO_WORD_MSB	0x00008000
#define	WORD_SHIFT	16
#define ASIAN_NL_PAD	BMASK		/* Newline pad character */

/* These are masks to check if the lower & higher 16-bit word	*/
/* of a character pair contains Asian character.		*/
#define LO_PAIR_MASK	0x00007f00
#define HI_PAIR_MASK	0x7f000000

#define	IS_ENCODED_ACHAR(ch)	(((ch) &  HI_WORD_MASK) != 0)
#define	IS_ACHAR(ch)		(((ch) & ~LO_BYTE_MASK) != 0)

#define	PAIR(first,second)	Pair(first, second) /* To function call */
#define	FORM_PAIR(first,second)	(((first ) & LO_CHAR_MASK) |\
				(((second) & LO_CHAR_MASK) << WORD_SHIFT))
/* First character of a character pair */
#define	FIRST_OF_PAIR(ch)	(((ch) & LO_CHAR_MASK) |\
				(((ch) & LO_PAIR_MASK) != 0 ? LO_WORD_MSB : 0)) 
/* Second character of a character pair */
#define	SECOND_OF_PAIR(ch)	((((ch) >> WORD_SHIFT) & LO_CHAR_MASK) |\
				(((ch) & HI_PAIR_MASK) != 0 ? LO_WORD_MSB : 0))

/****************[ MACRO DEFINITION ]*******************/
/*
 * This macro pushes the given character onto the peek stack.
 * It was once a routine but redefined as macro for efficiency reason.
 * The corresponding pop routine is found in na.c
 */
#define Push_To_Stack(ch)                       \
{                                               \
    if (peek.cur_sptr >= &peek.stack[LNSIZE])   \
        prstr("Peek stack overflow.\n") ;       \
    else                                        \
        *peek.cur_sptr++ = ch ;                 \
}


/****************[ TYPE DEFINITION ]********************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define	MAX_SLEVEL	10	/* Maximum stack level */

typedef struct
{
    wchar_t   stack[LNSIZE    ] ; /* Stack for saving char from getword */
    wchar_t  *sbase[MAX_SLEVEL] ; /* Stack bases for each stack level	*/
    wchar_t  *sptr [MAX_SLEVEL] ; /* Stack pointer for each stack level	*/
    struct s *frame[MAX_SLEVEL] ; /* Pointer to the current frame	*/
    wchar_t  *cur_sbase         ; /* Current stack base			*/
    wchar_t  *cur_sptr          ; /* Current stack pointer		*/
    int	      cur_slevel        ; /* Current stack level		*/
} PeekStack ;

#include <locale.h>
#include <nl_types.h>
/* --- WW001 --- */
