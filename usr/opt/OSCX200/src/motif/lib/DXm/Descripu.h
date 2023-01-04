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
 *	DESCRIP - VMS Descriptor Definitions
 *	(Based on the VAX-11 Procedure Calling and Condition Handling Standard, Revision 9.0 [7-Dec-81])
 */


/*
 *	General descriptor format - each class of descriptor consists of at least the following fields:
 */
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

struct	dsc$descriptor
	{
	unsigned short	dsc$w_length;	/* specific to descriptor class;  typically a 16-bit (unsigned) length */
	unsigned char	dsc$b_dtype;	/* data type code */
	unsigned char	dsc$b_class;	/* descriptor class code */
	char		*dsc$a_pointer;	/* address of first byte of data element */
	};


/*
 *	Scalar or string descriptor:
 */
struct	dsc$descriptor_s
	{
	unsigned short	dsc$w_length;	/* length of data item in bytes,
					     or if dsc$b_dtype is DSC$K_DTYPE_V, bits,
					     or if dsc$b_dtype is DSC$K_DTYPE_P, digits (4 bits each) */
	unsigned char	dsc$b_dtype;	/* data type code */
	unsigned char	dsc$b_class;	/* descriptor class code = DSC$K_CLASS_S */
	char		*dsc$a_pointer;	/* address of first byte of data storage */
	};


/*
 *	Codes for dsc$b_dtype:
 */

/*
 *	Atomic data types:
 */
#define DSC$K_DTYPE_Z	0		/* unspecified */
#define DSC$K_DTYPE_BU	2		/* byte logical;  8-bit unsigned quantity */
#define DSC$K_DTYPE_WU	3		/* word logical;  16-bit unsigned quantity */
#define DSC$K_DTYPE_LU	4		/* longword logical;  32-bit unsigned quantity */
#define DSC$K_DTYPE_QU	5		/* quadword logical;  64-bit unsigned quantity */
#define DSC$K_DTYPE_OU	25		/* octaword logical;  128-bit unsigned quantity */
#define DSC$K_DTYPE_B	6		/* byte integer;  8-bit signed 2's-complement integer */
#define DSC$K_DTYPE_W	7		/* word integer;  16-bit signed 2's-complement integer */
#define DSC$K_DTYPE_L	8		/* longword integer;  32-bit signed 2's-complement integer */
#define DSC$K_DTYPE_Q	9		/* quadword integer;  64-bit signed 2's-complement integer */
#define DSC$K_DTYPE_O	26		/* octaword integer;  128-bit signed 2's-complement integer */
#define DSC$K_DTYPE_F	10		/* F_floating;  32-bit single-precision floating point */
#define DSC$K_DTYPE_D	11		/* D_floating;  64-bit double-precision floating point */
#define DSC$K_DTYPE_G	27		/* G_floating;  64-bit double-precision floating point */
#define DSC$K_DTYPE_H	28		/* H_floating;  128-bit quadruple-precision floating point */
#define DSC$K_DTYPE_FC	12		/* F_floating complex */
#define DSC$K_DTYPE_DC	13		/* D_floating complex */
#define DSC$K_DTYPE_GC	29		/* G_floating complex */
#define DSC$K_DTYPE_HC	30		/* H_floating complex */
#define DSC$K_DTYPE_CIT	31		/* COBOL Intermediate Temporary */
/*
 *	String data types:
 */
#define DSC$K_DTYPE_T	14		/* character-coded text;  a single character or a string */
#define DSC$K_DTYPE_VT	37		/* varying character-coded text;  16-bit count, followed by a string */
#define DSC$K_DTYPE_NU	15		/* numeric string, unsigned */
#define DSC$K_DTYPE_NL	16		/* numeric string, left separate sign */
#define DSC$K_DTYPE_NLO	17		/* numeric string, left overpunched sign */
#define DSC$K_DTYPE_NR	18		/* numeric string, right separate sign */
#define DSC$K_DTYPE_NRO	19		/* numeric string, right overpunched sign */
#define DSC$K_DTYPE_NZ	20		/* numeric string, zoned sign */
#define DSC$K_DTYPE_P	21		/* packed decimal string */
#define DSC$K_DTYPE_V	1		/* bit;  aligned bit string */
#define DSC$K_DTYPE_VU	34		/* bit unaligned;  arbitrary bit string */
/*
 *	Miscellaneous data types:
 */
#define DSC$K_DTYPE_ZI	22		/* sequence of instructions */
#define DSC$K_DTYPE_ZEM	23		/* procedure entry mask */
#define DSC$K_DTYPE_DSC	24		/* descriptor */
#define DSC$K_DTYPE_BPV	32		/* bound procedure value */
#define DSC$K_DTYPE_BLV	33		/* bound label value */
#define DSC$K_DTYPE_ADT	35		/* absolute date and time */
/*
 *	Reserved data type codes:
 *	codes 38-191 are reserved to DIGITAL;
 *	codes 160-191 are reserved to DIGITAL facilities for facility-specific purposes;
 *	codes 192-255 are reserved for DIGITAL's Computer Special Systems Group
 *	and for customers for their own use.
 */


/*
 *	Codes for dsc$b_class:
 */
#define DSC$K_CLASS_S	1		/* scalar or string descriptor */
#define DSC$K_CLASS_D	2		/* dynamic string descriptor */
/*	DSC$K_CLASS_V	3		** variable buffer descriptor;  reserved for use by DIGITAL */
#define DSC$K_CLASS_A	4		/* array descriptor */
#define DSC$K_CLASS_P	5		/* procedure descriptor */
/*	DSC$K_CLASS_PI	6		** procedure incarnation descriptor;  obsolete */
/*	DSC$K_CLASS_J	7		** label descriptor;  reserved for use by the VAX-11 Debugger */
/*	DSC$K_CLASS_JI	8		** label incarnation descriptor;  obsolete */
#define DSC$K_CLASS_SD	9		/* decimal scalar string descriptor */
#define DSC$K_CLASS_NCA	10		/* noncontiguous array descriptor */
#define DSC$K_CLASS_VS	11		/* varying string descriptor */
#define DSC$K_CLASS_VSA	12		/* varying string array descriptor */
#define DSC$K_CLASS_UBS	13		/* unaligned bit string descriptor */
#define DSC$K_CLASS_UBA	14		/* unaligned bit array descriptor */
/*
 *	Reserved descriptor class codes:
 *	codes 15-191 are reserved to DIGITAL;
 *	codes 160-191 are reserved to DIGITAL facilities for facility-specific purposes;
 *	codes 192-255 are reserved for DIGITAL's Computer Special Systems Group
 *	and for customers for their own use.
 */


/*
 *	A simple macro to construct a string descriptor:
 */
#define $DESCRIPTOR(name,string)	struct dsc$descriptor_s name = { sizeof(string)-1, DSC$K_DTYPE_T, DSC$K_CLASS_S, string }

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
