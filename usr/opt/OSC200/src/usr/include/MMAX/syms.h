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
 *	@(#)$RCSfile: syms.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:06:09 $
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
/*	@(#)syms.h	3.1	*/
/*		*/
/*	2/26/91	*/
/*	5.2 SID syms.h	2.4 2/28/83	*/

/*		Storage Classes are defined in storclass.h  */
#include "storclass.h"

/*		Number of characters in a symbol name */
#define  SYMNMLEN	8
/*		Number of characters in a file name */
#define  FILNMLEN	14
/*		Number of array dimensions in auxiliary entry */
#define  DIMNUM		4


struct syment
{
	union
	{
		char		_n_name[SYMNMLEN];	/* old COFF version */
		struct
		{
			long	_n_zeroes;	/* new == 0 */
			long	_n_offset;	/* offset into string table */
		} _n_n;
		char		*_n_nptr[2];	/* allows for overlaying */
	} _n;
	long			n_value;	/* value of symbol */
	short			n_scnum;	/* section number */
	unsigned short		n_type;		/* type and derived type */
	char			n_sclass;	/* storage class */
	char			n_numaux;	/* number of aux. entries */
#ifdef ns32000
	/* The compilers, as, and ld use the next 16 bits as a rev indicator */
	/* 5 for compiler 1=C, 2=F77, 3=Pascal; 6,5 for major, minor rev */
	char			n_env;		/* symbol interp environ */
	char			n_dummy;	/* currently unused */
#else
	short			n_dummy;	/* currently unused */
#endif
};

#define n_name		_n._n_name
#define n_nptr		_n._n_nptr[1]
#define n_zeroes	_n._n_n._n_zeroes
#define n_offset	_n._n_n._n_offset

/*
   Relocatable symbols have a section number of the
   section in which they are defined.  Otherwise, section
   numbers have the following meanings:
*/
        /* undefined symbol */
#define  N_UNDEF	0
        /* value of symbol is absolute */
#define  N_ABS		-1
        /* special debugging symbol -- value of symbol is meaningless */
#define  N_DEBUG	-2
	/* indicates symbol needs transfer vector (preload) */
#define  N_TV		(unsigned short)-3

	/* indicates symbol needs transfer vector (postload) */

#define  P_TV		(unsigned short)-4

/*
   The fundamental type of a symbol packed into the low 
   4 bits of the word.
*/

#define  _EF	".ef"

#define  T_NULL     0
#define  T_ARG      1          /* function argument (only used by compiler) */
#define  T_CHAR     2          /* character */
#define  T_SHORT    3          /* short integer */
#define  T_INT      4          /* integer */
#define  T_LONG     5          /* long integer */
#define  T_FLOAT    6          /* floating point */
#define  T_DOUBLE   7          /* double word */
#define  T_STRUCT   8          /* structure  */
#define  T_UNION    9          /* union  */
#define  T_ENUM     10         /* enumeration  */
#define  T_MOE      11         /* member of enumeration */
#define  T_UCHAR    12         /* unsigned character */
#define  T_USHORT   13         /* unsigned short */
#define  T_UINT     14         /* unsigned integer */
#define  T_ULONG    15         /* unsigned long */

/*
 * derived types are:
 */

#define  DT_NON      0          /* no derived type */
#define  DT_PTR      1          /* pointer */
#define  DT_FCN      2          /* function */
#define  DT_ARY      3          /* array */

/*
 *   type packing constants
 */

#define  N_BTMASK     017
#define  N_TMASK      060
#define  N_TMASK1     0300
#define  N_TMASK2     0360
#define  N_BTSHFT     4
#define  N_TSHIFT     2

/*
 *   MACROS
 */

	/*   Basic Type of  x   */

#define  BTYPE(x)  ((x) & N_BTMASK)

	/*   Is  x  a  pointer ?   */

#define  ISPTR(x)  (((x) & N_TMASK) == (DT_PTR << N_BTSHFT))

	/*   Is  x  a  function ?  */

#define  ISFCN(x)  (((x) & N_TMASK) == (DT_FCN << N_BTSHFT))

	/*   Is  x  an  array ?   */

#define  ISARY(x)  (((x) & N_TMASK) == (DT_ARY << N_BTSHFT))

	/* Is x a structure, union, or enumeration TAG? */

#define ISTAG(x)  ((x)==C_STRTAG || (x)==C_UNTAG || (x)==C_ENTAG)

#define  INCREF(x) ((((x)&~N_BTMASK)<<N_TSHIFT)|(DT_PTR<<N_BTSHFT)|(x&N_BTMASK))

#define  DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))

/*
 *	AUXILIARY ENTRY FORMAT
 */

union auxent
{
	struct
	{
		long		x_tagndx;	/* str, un, or enum tag indx */
		union
		{
			struct
			{
				unsigned short	x_lnno;	/* declaration line number */
				unsigned short	x_size;	/* str, union, array size */
			} x_lnsz;
			long	x_fsize;	/* size of function */
		} x_misc;
		union
		{
			struct			/* if ISFCN, tag, or .bb */
			{
				long	x_lnnoptr;	/* ptr to fcn line # */
				long	x_endndx;	/* entry ndx past block end */
			} 	x_fcn;
			struct			/* if ISARY, up to 4 dimen. */
			{
				unsigned short	x_dimen[DIMNUM];
			} 	x_ary;
		}		x_fcnary;
		union
		{
			struct
			{
				unsigned short x_callseq;/* calling sequence information */
				unsigned short x_level;/* function nesting level */
			} x_fend;
			struct
			{
				unsigned short x_tvindx;	/* actually a dummy */
				unsigned short x_plude;
			} 	x_befn;
		} x_fnend;
	} 	x_sym;
	struct
	{
		char	x_fname[FILNMLEN];
		unsigned short	x_fdummy;
		long	x_foff;			/* string table offset of file name */
	} 	x_file;
        struct
        {
                long    x_scnlen;          /* section length */
                unsigned short  x_nreloc;  /* number of relocation entries */
                unsigned short  x_nlinno;  /* number of line numbers */
		long	x_linoptr;	   /* pointer to line number entries */
        }       x_scn;

};
#define x_tvndx x_fnend.x_befn.x_tvindx

#define	SYMENT	struct syment
#define	SYMESZ	20	/* sizeof(SYMENT) */

#define	AUXENT	union auxent
#define	AUXESZ	20	/* sizeof(AUXENT) */

/*	Defines for "special" symbols   */

#if VAX || n16 || m68
#define _ETEXT	"_etext"
#define	_EDATA	"_edata"
#define	_END	"_end"
#else
#define _ETEXT	"etext"
#define _EDATA	"edata"
#define _END	"end"
#endif

#define _START	"_start"

#define _TVORIG	"_tvorig"
#define _TORIGIN	"_torigin"
#define _DORIGIN	"_dorigin"

#define _SORIGIN	"_sorigin"
