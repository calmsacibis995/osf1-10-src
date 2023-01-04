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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: NLSsetup.c,v $ $Revision: 1.1.5.6 $ (DEC) $Date: 1993/12/10 18:53:33 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: NLSsetup.c
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.12  com/lib/c/loc/NLSsetup.c, libcloc, bos320, 9132320m 8/11/91 17:42:26
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <locale.h>
#include <sys/localedef.h>
#include <stdlib.h>
#include <langinfo.h>
#include <sys/method.h>
  

/*
  Initialization methods
*/
_LC_charmap_t	 *__charmap_init(_LC_locale_t *);
_LC_ctype_t	 *__ctype_init(_LC_locale_t *);
_LC_collate_t	 *__collate_init(_LC_locale_t *);
_LC_monetary_t	 *__monetary_init(_LC_locale_t *);
_LC_numeric_t	 *__numeric_init(_LC_locale_t *);
_LC_resp_t	 *__resp_init(_LC_locale_t *);
_LC_time_t	 *__time_init(_LC_locale_t *);
_LC_locale_t	 *__locale_init(_LC_locale_t *);


_LC_charmap_t _C_charmap={
  /*
  ** Object header info
  */
  _LC_CHARMAP,               /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  sizeof(_LC_charmap_t),	/* size    */

  /* 
  ** charmap methods for C_locale
  */
  NULL,			/* NL_CSINFO */
  
  NULL,			/* MBTOWC_SB */
  NULL,			/* MBSTOWCS_SB */
  NULL,			/* WCTOMB_SB */
  NULL,			/* WCSTOMBS_SB */

  NULL,			/* MBLEN_SB */

  NULL,			/* WCSWIDTH_LATIN */
  NULL,			/* WCWIDTH_LATIN */

  NULL,			/* __MBTOPC_SB */
  NULL,			/* __MBSTOPCS_SB */
  NULL,			/* __PCTOMB_SB */
  NULL,			/* __PCSTOMBS_SB */
  
  __charmap_init,	     /* init method */
  0,			     /* void * data */

  /*
  ** extension data
  */
  "ISO8859-1",		     /* codeset name*/
  1,			     /* cm_mb_cur_max */
  1,			     /* cm_mb_cur_min */
};


#include <ctype.h>

_LC_classnm_t _C_classnms[_NUM_CLASSES]={
  "alnum", _ISALNUM,
  "alpha", _ISALPHA,
  "blank", _ISBLANK,
  "cntrl", _ISCNTRL,
  "digit", _ISDIGIT,
  "graph", _ISGRAPH,
  "lower", _ISLOWER,
  "print", _ISPRINT,
  "punct", _ISPUNCT,
  "space", _ISSPACE,
  "upper", _ISUPPER,
  "xdigit", _ISXDIGIT
};

wchar_t _C_upper[] = {
	(wchar_t) EOF,
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 
	0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
	0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f, 
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 
	0x0060, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
	0x0058, 0x0059, 0x005a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f, 
	0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 
	0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f, 
	0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 
	0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f, 
	0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 
	0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af, 
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 
	0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf, 
	0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 
	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 
	0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 
	0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df, 
	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 
	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef, 
	0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 
	0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff};
	      	
wchar_t _C_lower[] = {
	(wchar_t) EOF,
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 
	0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
	0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f, 
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
	0x0040, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
	0x0078, 0x0079, 0x007a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,  
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f, 
	0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 
	0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f, 
	0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 
	0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f, 
	0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 
	0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af, 
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 
	0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf, 
	0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 
	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 
	0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 
	0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df, 
	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 
	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef, 
	0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 
	0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff};

#define _C _ISCNTRL
#define _B (_ISBLANK|_ISPRINT|_ISSPACE)
#define _T (_ISBLANK|_ISSPACE)
#define _V _ISSPACE
#define _P (_ISPUNCT|_ISPRINT|_ISGRAPH)
#define _X _ISXDIGIT
#define _U (_ISUPPER|_ISALPHA|_ISALNUM|_ISPRINT|_ISGRAPH)
#define _L (_ISLOWER|_ISALPHA|_ISALNUM|_ISPRINT|_ISGRAPH)
#define _G (_ISGRAPH|_ISPRINT)
#define _N (_ISDIGIT|_ISALNUM|_ISPRINT|_ISGRAPH)
unsigned int _C_masks[]={

/* -1*/  0,
/*	 0	 1	 2	 3	 4	 5	 6	 7  */
/* 0*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 10*/	_C,	_T|_C,	_V|_C,	_V|_C,	_V|_C,	_V|_C,	_C,	_C,
/* 20*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 30*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 40*/	_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
/* 50*/	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
/* 60*/	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,
/* 70*/	_N|_X,	_N|_X,	_P,	_P,	_P,	_P,	_P,	_P,
/*100*/	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U,
/*110*/	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
/*120*/	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
/*130*/	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P,
/*140*/	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L,
/*150*/	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
/*160*/	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
/*170*/	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C,
/*200*/   0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0,
          0,     0,      0,      0,      0,      0,      0,      0
};

_LC_ctype_t _C_ctype={
  /*
  ** Object header info
  */
  _LC_CTYPE,                 /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  sizeof(_LC_ctype_t),		/* size    */

  /*
  ** Character Attributes Methods
  */
  NULL,			/* TOWUPPER_STD */
  NULL,			/* TOWLOWER_STD */

  NULL,			/* GET_WCTYPE_STD */
  NULL,			/* IS_WCTYPE_SB */

  __ctype_init,	     	    /* init method */
  0,			     /* void * data */

  /* 
  ** class extension data.
  */
  0,			    /* min process code */
  255,			    /* max process code */
  255,			    /* Last valid upper entry */
  255,			    /* Last valid lower entry */
  &_C_upper[1],		    /* upper */
  &_C_lower[1],		    /* lower */

  &_C_masks[1],             /* array of classification masks */
  (unsigned int *)0,	    /* qmask - code-points > 255 */
  (unsigned char *)0,       /* qidx - indices into qmask */
  0,			    /* qidx_hbound - last code point with qidx */
  sizeof(_C_classnms) / sizeof(_LC_classnm_t),
  _C_classnms,
};

_LC_coltbl_t _C_coltbl[]={
    {{0x101}, 0}, {{0x102}, 0}, {{0x103}, 0}, 
    {{0x104}, 0}, {{0x105}, 0}, {{0x106}, 0}, {{0x107}, 0}, 
    {{0x108}, 0}, {{0x109}, 0}, {{0x10A}, 0}, {{0x10B}, 0}, 
    {{0x10C}, 0}, {{0x10D}, 0}, {{0x10E}, 0}, {{0x10F}, 0},
    {{0x110}, 0}, {{0x111}, 0}, {{0x112}, 0}, {{0x113}, 0}, 
    {{0x114}, 0}, {{0x115}, 0}, {{0x116}, 0}, {{0x117}, 0},
    {{0x118}, 0}, {{0x119}, 0}, {{0x11A}, 0}, {{0x11B}, 0}, 
    {{0x11C}, 0}, {{0x11D}, 0}, {{0x11E}, 0}, {{0x11F}, 0},
    {{0x120}, 0}, {{0x121}, 0}, {{0x122}, 0}, {{0x123}, 0}, 
    {{0x124}, 0}, {{0x125}, 0}, {{0x126}, 0}, {{0x127}, 0},
    {{0x128}, 0}, {{0x129}, 0}, {{0x12A}, 0}, {{0x12B}, 0}, 
    {{0x12C}, 0}, {{0x12D}, 0}, {{0x12E}, 0}, {{0x12F}, 0},
    {{0x130}, 0}, {{0x131}, 0}, {{0x132}, 0}, {{0x133}, 0}, 
    {{0x134}, 0}, {{0x135}, 0}, {{0x136}, 0}, {{0x137}, 0},
    {{0x138}, 0}, {{0x139}, 0}, {{0x13A}, 0}, {{0x13B}, 0}, 
    {{0x13C}, 0}, {{0x13D}, 0}, {{0x13E}, 0}, {{0x13F}, 0},
    {{0x140}, 0}, {{0x141}, 0}, {{0x142}, 0}, {{0x143}, 0}, 
    {{0x144}, 0}, {{0x145}, 0}, {{0x146}, 0}, {{0x147}, 0},
    {{0x148}, 0}, {{0x149}, 0}, {{0x14A}, 0}, {{0x14B}, 0}, 
    {{0x14C}, 0}, {{0x14D}, 0}, {{0x14E}, 0}, {{0x14F}, 0},
    {{0x150}, 0}, {{0x151}, 0}, {{0x152}, 0}, {{0x153}, 0}, 
    {{0x154}, 0}, {{0x155}, 0}, {{0x156}, 0}, {{0x157}, 0},
    {{0x158}, 0}, {{0x159}, 0}, {{0x15A}, 0}, {{0x15B}, 0}, 
    {{0x15C}, 0}, {{0x15D}, 0}, {{0x15E}, 0}, {{0x15F}, 0},
    {{0x160}, 0}, {{0x161}, 0}, {{0x162}, 0}, {{0x163}, 0}, 
    {{0x164}, 0}, {{0x165}, 0}, {{0x166}, 0}, {{0x167}, 0},
    {{0x168}, 0}, {{0x169}, 0}, {{0x16A}, 0}, {{0x16B}, 0}, 
    {{0x16C}, 0}, {{0x16D}, 0}, {{0x16E}, 0}, {{0x16F}, 0},
    {{0x170}, 0}, {{0x171}, 0}, {{0x172}, 0}, {{0x173}, 0}, 
    {{0x174}, 0}, {{0x175}, 0}, {{0x176}, 0}, {{0x177}, 0},
    {{0x178}, 0}, {{0x179}, 0}, {{0x17A}, 0}, {{0x17B}, 0}, 
    {{0x17C}, 0}, {{0x17D}, 0}, {{0x17E}, 0}, {{0x17F}, 0},
    {{0x180}, 0}, {{0x181}, 0}, {{0x182}, 0}, {{0x183}, 0}, 
    {{0x184}, 0}, {{0x185}, 0}, {{0x186}, 0}, {{0x187}, 0},
    {{0x188}, 0}, {{0x189}, 0}, {{0x18A}, 0}, {{0x18B}, 0}, 
    {{0x18C}, 0}, {{0x18D}, 0}, {{0x18E}, 0}, {{0x18F}, 0},
    {{0x190}, 0}, {{0x191}, 0}, {{0x192}, 0}, {{0x193}, 0}, 
    {{0x194}, 0}, {{0x195}, 0}, {{0x196}, 0}, {{0x197}, 0},
    {{0x198}, 0}, {{0x199}, 0}, {{0x19A}, 0}, {{0x19B}, 0}, 
    {{0x19C}, 0}, {{0x19D}, 0}, {{0x19E}, 0}, {{0x19F}, 0},
    {{0x1A0}, 0}, {{0x1A1}, 0}, {{0x1A2}, 0}, {{0x1A3}, 0}, 
    {{0x1A4}, 0}, {{0x1A5}, 0}, {{0x1A6}, 0}, {{0x1A7}, 0},
    {{0x1A8}, 0}, {{0x1A9}, 0}, {{0x1AA}, 0}, {{0x1AB}, 0}, 
    {{0x1AC}, 0}, {{0x1AD}, 0}, {{0x1AE}, 0}, {{0x1AF}, 0},
    {{0x1B0}, 0}, {{0x1B1}, 0}, {{0x1B2}, 0}, {{0x1B3}, 0}, 
    {{0x1B4}, 0}, {{0x1B5}, 0}, {{0x1B6}, 0}, {{0x1B7}, 0},
    {{0x1B8}, 0}, {{0x1B9}, 0}, {{0x1BA}, 0}, {{0x1BB}, 0}, 
    {{0x1BC}, 0}, {{0x1BD}, 0}, {{0x1BE}, 0}, {{0x1BF}, 0},
    {{0x1C0}, 0}, {{0x1C1}, 0}, {{0x1C2}, 0}, {{0x1C3}, 0}, 
    {{0x1C4}, 0}, {{0x1C5}, 0}, {{0x1C6}, 0}, {{0x1C7}, 0},
    {{0x1C8}, 0}, {{0x1C9}, 0}, {{0x1CA}, 0}, {{0x1CB}, 0}, 
    {{0x1CC}, 0}, {{0x1CD}, 0}, {{0x1CE}, 0}, {{0x1CF}, 0},
    {{0x1D0}, 0}, {{0x1D1}, 0}, {{0x1D2}, 0}, {{0x1D3}, 0}, 
    {{0x1D4}, 0}, {{0x1D5}, 0}, {{0x1D6}, 0}, {{0x1D7}, 0},
    {{0x1D8}, 0}, {{0x1D9}, 0}, {{0x1DA}, 0}, {{0x1DB}, 0}, 
    {{0x1DC}, 0}, {{0x1DD}, 0}, {{0x1DE}, 0}, {{0x1DF}, 0},
    {{0x1E0}, 0}, {{0x1E1}, 0}, {{0x1E2}, 0}, {{0x1E3}, 0}, 
    {{0x1E4}, 0}, {{0x1E5}, 0}, {{0x1E6}, 0}, {{0x1E7}, 0},
    {{0x1E8}, 0}, {{0x1E9}, 0}, {{0x1EA}, 0}, {{0x1EB}, 0}, 
    {{0x1EC}, 0}, {{0x1ED}, 0}, {{0x1EE}, 0}, {{0x1EF}, 0},
    {{0x1F0}, 0}, {{0x1F1}, 0}, {{0x1F2}, 0}, {{0x1F3}, 0}, 
    {{0x1F4}, 0}, {{0x1F5}, 0}, {{0x1F6}, 0}, {{0x1F7}, 0},
    {{0x1F8}, 0}, {{0x1F9}, 0}, {{0x1FA}, 0}, {{0x1FB}, 0}, 
    {{0x1FC}, 0}, {{0x1FD}, 0}, {{0x1FE}, 0}, {{0x1FF}, 0},
    {{0x201}, 0},
};

_LC_collate_t _C_collate={
  /*
  ** Object header info
  */
  _LC_COLLATE,               /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  sizeof(_LC_collate_t),	/* size    */

  /*
  ** Collation Methods
  */
  /* character collation methods */
  NULL,			/* STRCOLL_C */
  NULL,			/* STRXFRM_C */

  /* process code collation methods */
  NULL,			/* WCSCOLL_C */
  NULL,			/* WCSXFRM_C */

  /* filename matching methods */
  NULL,			/* FNMATCH_C */

  /* regular expression methods */
  NULL,			/* REGCOMP_C */
  NULL,			/* REGERROR_STD */
  NULL,			/* REGEXEC_C */
  NULL,			/* REGFREE_STD */

  __collate_init,	      /* init method */
  0,			     /* void * data */

  /*
  ** Class Extension Data
  */
  
  0,			     /* co_nord   */
  { 0 },		     /* co_sort */

  0,                         /* co_wc_min */
  255,			     /* co_wc_max */
  255,			     /* Last valid collation entry */
  257,			     /* co_col_min*/
  257+256,		     /* co_col_max*/

  (_LC_coltbl_t *)_C_coltbl,		     /* co_coltbl */
};

_LC_numeric_t _C_numeric={
    /* Object header info */
    _LC_NUMERIC,       /* type_id */
    _LC_MAGIC,	       /* magic   */
    _LC_VERSION,       /* version */
    sizeof(_LC_numeric_t),	/* size    */

    NULL,	/* NL_NUMINFO */
  
    __numeric_init,      /* init method */
    0,		       /* void * data */

    /* Class Extension Data */
    ".",	       /* decimal_point */
    "",		       /* thousands_sep */
    (unsigned char *) "",
};

_LC_monetary_t _C_monetary={
  /*
  ** Object header info
  */
  _LC_MONETARY,              /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  sizeof(_LC_monetary_t),	/* size    */

  /* Methods for monetary class */
  NULL,			/* NL_MONINFO */
  NULL,			/* STRFMON_STD */
  
  __monetary_init,	     /* init method */
  0,			     /* void * data */

  /* Class Extension Data */
  "",			     /* int_curr_symbol */
  "",			     /* currency_symbol */
  "",			     /* mon_decimal_point */
  "",			     /* mon_thousands_sep */
  "",			     /* mon_grouping */

  "",			     /* positive_sign   */
  "",			     /* negative_sign   */
  CHAR_MAX,			     /* int_frac_digits */
  CHAR_MAX,			     /* frac_digits     */
  CHAR_MAX,			     /* p_cs_precedes   */
  CHAR_MAX,			     /* p_sep_by_space  */
  CHAR_MAX,			     /* n_cs_precedes   */
  CHAR_MAX,			     /* n_sep_by_space  */
  CHAR_MAX,			     /* p_sign_posn     */
  CHAR_MAX,			     /* n_sign_posn     */
  "",			     /* debit_sign 	*/
  "",			     /* credit_sign    	*/
  "",			     /* left_parenthesis */
  ""			     /* right_parenthesis */
};

static char *null_era[1] = { NULL };

_LC_time_t _C_time={
  /*
  ** Object header info
  */
  _LC_TIME,                  /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  sizeof(_LC_time_t),		/* size    */
 
  NULL,		/* NL_TIMINFO */
  
  NULL, 	/* STRFTIME_STD */
  NULL,		/* STRPTIME_STD */
  NULL, 	/* WCSFTIME_STD */

  __time_init,		     /* init method */
  0,			     /* void * data */

  /* Class Extension Data */
  "%m/%d/%y",		     /* d_fmt */
  "%H:%M:%S",		     /* t_fmt */
  "%a %b %d %H:%M:%S %Y",    /* d_t_fmt */
  "%I:%M:%S %p",             /* t_fmt_ampm */
  { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
  { "Sunday", "Monday", "Tuesday", "Wednesday", 
    "Thursday", "Friday", "Saturday" },
  { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
    "Oct", "Nov", "Dec" },
  { "January", "February", "March", "April", "May", "June", "July", 
    "August", "September", "October", "November", "December" },
  { "AM", "PM" },
  null_era,                        /* era */
  "",                        /* era_year */
  "",                        /* era_d_fmt */
  "",                        /* alt_digits */
  "",                        /* m_d_recent  OSF extension */
  "",                         /* m_d_old OSF extension */
  "",				/* era_d_t_fmt */
  "",				/* era_t_fmt */
};

_LC_resp_t _C_resp={
  /*
  ** Object header info
  */
  _LC_RESP,                  /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  sizeof(_LC_resp_t),		/* size    */

  NULL,	/* NL_RESPINFO */
  NULL,	/* RPMATCH_C */
  
  __resp_init,		     /* init method */
  0,			     /* void * data */

  /*
  ** Class Extension Data 
  */
  "^[yY].*",		     /* yesexpr - unused */
  "^[nN].*",		     /* noexpr - unused */
  "yes:y:Y",		     /* yesstr */
  "no:n:N",		     /* nostr */
};

struct lconv __C_lconv={
  ".",				/* decimal_point     */
  "",				/* thousands_sep     */
  "",				/* grouping 	     */
  "",				/* int_curr_symbol   */
  "",				/* currency_symbol   */
  "",				/* mon_decimal_point */
  "",				/* mon_thousands_sep */
  "",				/* mon_grouping      */
  "",				/* positive_sign     */
  "", 				/* negative_sign     */
  CHAR_MAX,				/* int_frac_digits   */
  CHAR_MAX,				/* frac_digits       */
  CHAR_MAX,				/* p_cs_precedes     */
  CHAR_MAX,				/* p_sep_by_space    */
  CHAR_MAX,				/* n_cs_precedes     */
  CHAR_MAX,				/* n_sep_by_space    */
  CHAR_MAX,				/* p_sign_posn       */
  CHAR_MAX				/* n_sign_posn       */
};

_LC_locale_t __C_locale={

    {	{_LC_LOCALE, _LC_MAGIC, _LC_VERSION, sizeof(_LC_locale_t)},

	  NULL,	/* NL_LANGINFO_STD */
	  NULL,	/* LOCALECONV_STD */
  
	  __locale_init,		/* Init method */
	  0			        /* data pointer */
    },					/* Core object */

  /* info strings */
  {
      "",						/* NOT USED */
      "%a %b %d %H:%M:%S %Y",				/* d_t_fmt */
      "%m/%d/%y",					/* d_fmt */
      "%H:%M:%S",					/* t_fmt */
      "AM",						/* AM_STR  */
      "PM",						/* PM_STR  */
      "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", 	/* ABDAY_x */
      "Sunday", "Monday", "Tuesday", "Wednesday",     
      "Thursday", "Friday", "Saturday",			/* DAY_x   */
      "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
      "Aug", "Sep", "Oct", "Nov", "Dec",		/* ABMON_x */
      "January", "February", "March", "April", "May",
      "June", "July", "August", "September", "October",
      "November", "December",				/* MON_x   */
      ".",						/* RADIXCHAR */
      "",						/* THOUSEP */
      "yes:y:Y",					/* YESSTR  */
      "no:n:N",					   	/* NOSTR   */
      "",						/* CRNCYSTR*/
      "ISO8859-1",   	                                /* CODESET */
      "",						/* m_d_recent */
      "",						/* m_d_old */
      "",						/* era */
      "",						/* era_d_fmt */
      "",						/* era_d_t_fmt */
  },

  &__C_lconv,
  
  &_C_charmap, 
  &_C_collate, 
  &_C_ctype, 
  &_C_monetary, 
  &_C_numeric, 
  &_C_resp, 
  &_C_time, 
    /* nl_info2[] */
  {	"",			/* era_t_fmt */
	"",			/* T_FMT_AMPM */
	"",			/* Alt digits */
	"",			/* NOEXPR */
	"",			/* YESEXPR */
	"",			/* Unused */
  }
}; 

_LC_locale_t   * _C_locale = &__C_locale;

_LC_charmap_t  * __lc_charmap  = &_C_charmap;
_LC_ctype_t    * __lc_ctype    = &_C_ctype;
_LC_collate_t  * __lc_collate  = &_C_collate;
_LC_numeric_t  * __lc_numeric  = &_C_numeric;
_LC_monetary_t * __lc_monetary = &_C_monetary;
_LC_time_t     * __lc_time     = &_C_time;
_LC_resp_t     * __lc_resp     = &_C_resp;
_LC_locale_t   * __lc_locale   = &__C_locale;


/*
*  FUNCTION: collate_init
*
*  DESCRIPTION:
*  Initialization method for collate locale object.  
*/
_LC_collate_t *
__collate_init(_LC_locale_t *lp)
{
    return lp->lc_collate;
}


/*
*  FUNCTION: ctype_init
*
*  DESCRIPTION:
*  Initialization method for ctype locale object. 
*/
_LC_ctype_t *
__ctype_init(_LC_locale_t *lp)
{
    return lp->lc_ctype;
}


/*
*  FUNCTION: monetary_init
*
*  DESCRIPTION:
*  Initialization method for monetary locale object.
*/
_LC_monetary_t *
__monetary_init(_LC_locale_t *lp)
{

    char *csym = lp->lc_monetary->currency_symbol;
    static char	*crncy = 0;
    static int incr = 64;
    int len;

    if (csym) {
	if (*csym == '\0')
	    lp->nl_info[CRNCYSTR] = "";
	else {
	    len = strlen(csym);
	    if ((!crncy) || (len+2 > incr)) {
		if (len+2 > incr)
		    incr = len + 2;
		if (crncy)
		  free(crncy);
		crncy = (char *)malloc(incr);
	    }
	    if (lp->lc_monetary->mon_decimal_point && 
		!strcmp(csym, lp->lc_monetary->mon_decimal_point))
		crncy[0] = '.';
	    else if (lp->lc_monetary->p_cs_precedes)
		/*
		 * You'd think that XPG4 would want to look at BOTH the
		 * p_cs_precedes and the n_cs_precedes values, but the tests
		 * seem to presume that you only examine the positive sign case
		 */
		crncy[0] = '-';
	    else
		crncy[0] = '+';
	    strcpy( crncy+1, csym );
	    lp->nl_info[CRNCYSTR] = crncy;
	}
    } else
	lp->nl_info[CRNCYSTR] = NULL; /* Should never happen */

    /* setup localeconv() structure */
   lp->nl_lconv->int_curr_symbol    = lp->lc_monetary->int_curr_symbol;
   lp->nl_lconv->currency_symbol    = lp->lc_monetary->currency_symbol;
   lp->nl_lconv->mon_decimal_point  = lp->lc_monetary->mon_decimal_point;
   lp->nl_lconv->mon_thousands_sep  = lp->lc_monetary->mon_thousands_sep;
   lp->nl_lconv->mon_grouping       = lp->lc_monetary->mon_grouping;
   lp->nl_lconv->positive_sign      = lp->lc_monetary->positive_sign;
   lp->nl_lconv->negative_sign      = lp->lc_monetary->negative_sign;
   lp->nl_lconv->int_frac_digits    = lp->lc_monetary->int_frac_digits;
   lp->nl_lconv->frac_digits        = lp->lc_monetary->frac_digits;
   lp->nl_lconv->p_cs_precedes      = lp->lc_monetary->p_cs_precedes;
   lp->nl_lconv->p_sep_by_space     = lp->lc_monetary->p_sep_by_space;
   lp->nl_lconv->n_cs_precedes      = lp->lc_monetary->n_cs_precedes;
   lp->nl_lconv->n_sep_by_space     = lp->lc_monetary->n_sep_by_space;
   lp->nl_lconv->p_sign_posn        = lp->lc_monetary->p_sign_posn;
   lp->nl_lconv->n_sign_posn        = lp->lc_monetary->n_sign_posn;
   lp->nl_lconv->left_parenthesis   = lp->lc_monetary->left_parenthesis;
   lp->nl_lconv->right_parenthesis  = lp->lc_monetary->right_parenthesis;

    return lp->lc_monetary;
}


/*
*  FUNCTION: charmap_init
*
*  DESCRIPTION:
*  Locale charmap class object initialization method.
*/
_LC_charmap_t *
__charmap_init(_LC_locale_t *lp)
{
    /* set nl_langinfo() information */
    lp->nl_info[CODESET] = lp->lc_charmap->cm_csname;
    return lp->lc_charmap;
}


/*
*  FUNCTION: resp_init
*
*  DESCRIPTION:
*  Initialization method for the response class.
*/
_LC_resp_t *
__resp_init(_LC_locale_t *lp)
{
    lp->nl_info[YESSTR] = lp->lc_resp->yesstr;
    lp->nl_info[NOSTR] = lp->lc_resp->nostr;
    return lp->lc_resp;
}


/*
*  FUNCTION: numeric_init
*
*  DESCRIPTION:
*  Numeric category object initialization method.
*/
_LC_numeric_t *
__numeric_init(_LC_locale_t *lp)
{
    /* set nl_langinfo() information */
    lp->nl_info[RADIXCHAR] = lp->lc_numeric->decimal_point;
    lp->nl_info[THOUSEP]   = lp->lc_numeric->thousands_sep;

    /* setup localeconv() lconv structure */
    lp->nl_lconv->decimal_point = lp->lc_numeric->decimal_point;
    lp->nl_lconv->thousands_sep = lp->lc_numeric->thousands_sep;
    lp->nl_lconv->grouping      = (char *)lp->lc_numeric->grouping;
    return lp->lc_numeric;
}


/*
*  FUNCTION: time_init
*
*  DESCRIPTION:
*  This is the initialization method for the time category
*/
_LC_time_t *
__time_init(_LC_locale_t *lp)
{
    /* set nl_langinfo() information */
    lp->nl_info[D_FMT]    = lp->lc_time->d_fmt;
    lp->nl_info[T_FMT]    = lp->lc_time->t_fmt;
    lp->nl_info[D_T_FMT]  = lp->lc_time->d_t_fmt;
    lp->nl_info[_M_D_RECENT]  = lp->lc_time->m_d_recent;
    lp->nl_info[_M_D_OLD] = lp->lc_time->m_d_old;
    lp->nl_info[ABDAY_1]  = lp->lc_time->abday[0];
    lp->nl_info[ABDAY_2]  = lp->lc_time->abday[1];
    lp->nl_info[ABDAY_3]  = lp->lc_time->abday[2];
    lp->nl_info[ABDAY_4]  = lp->lc_time->abday[3];
    lp->nl_info[ABDAY_5]  = lp->lc_time->abday[4];
    lp->nl_info[ABDAY_6]  = lp->lc_time->abday[5];
    lp->nl_info[ABDAY_7]  = lp->lc_time->abday[6];
    lp->nl_info[DAY_1]    = lp->lc_time->day[0];
    lp->nl_info[DAY_2]    = lp->lc_time->day[1];
    lp->nl_info[DAY_3]    = lp->lc_time->day[2];
    lp->nl_info[DAY_4]    = lp->lc_time->day[3];
    lp->nl_info[DAY_5]    = lp->lc_time->day[4];
    lp->nl_info[DAY_6]    = lp->lc_time->day[5];
    lp->nl_info[DAY_7]    = lp->lc_time->day[6];
    lp->nl_info[ABMON_1]  = lp->lc_time->abmon[0];
    lp->nl_info[ABMON_2]  = lp->lc_time->abmon[1];
    lp->nl_info[ABMON_3]  = lp->lc_time->abmon[2];
    lp->nl_info[ABMON_4]  = lp->lc_time->abmon[3];
    lp->nl_info[ABMON_5]  = lp->lc_time->abmon[4];
    lp->nl_info[ABMON_6]  = lp->lc_time->abmon[5];
    lp->nl_info[ABMON_7]  = lp->lc_time->abmon[6];
    lp->nl_info[ABMON_8]  = lp->lc_time->abmon[7];
    lp->nl_info[ABMON_9]  = lp->lc_time->abmon[8];
    lp->nl_info[ABMON_10] = lp->lc_time->abmon[9];
    lp->nl_info[ABMON_11] = lp->lc_time->abmon[10];
    lp->nl_info[ABMON_12] = lp->lc_time->abmon[11];
    lp->nl_info[MON_1]    = lp->lc_time->mon[0];
    lp->nl_info[MON_2]    = lp->lc_time->mon[1];
    lp->nl_info[MON_3]    = lp->lc_time->mon[2];
    lp->nl_info[MON_4]    = lp->lc_time->mon[3];
    lp->nl_info[MON_5]    = lp->lc_time->mon[4];
    lp->nl_info[MON_6]    = lp->lc_time->mon[5];
    lp->nl_info[MON_7]    = lp->lc_time->mon[6];
    lp->nl_info[MON_8]    = lp->lc_time->mon[7];
    lp->nl_info[MON_9]    = lp->lc_time->mon[8];
    lp->nl_info[MON_10]   = lp->lc_time->mon[9];
    lp->nl_info[MON_11]   = lp->lc_time->mon[10];
    lp->nl_info[MON_12]   = lp->lc_time->mon[11];
    lp->nl_info[AM_STR]   = lp->lc_time->am_pm[0];
    lp->nl_info[PM_STR]   = lp->lc_time->am_pm[1];

    /*
     * For now, we have to ignore the era fields.  In OSF/1, lc_time->era is
     * a NULL terminated array of pointers to era strings.  This was needed to
     * allow an era to have a semicolon in it.
     */

    lp->nl_info[ERA]	  = "";

    lp->nl_info[ERA_D_FMT]	  = lp->lc_time->era_d_fmt;

    if (lp->lc_time->core.hdr.size >= sizeof(_LC_time_t) &&
	lp->core.hdr.size >= sizeof(_LC_locale_t)) {
	/*
	 * This object is at least as big as one with extra fields
	 * so we can copy them.  These didn't used to be in locales
	 * before.
	 */
	lp->nl_info[ERA_D_T_FMT]  = lp->lc_time->era_d_t_fmt;
	lp->nl_info2[ERA_T_FMT-_NL_OLD_SIZE] = lp->lc_time->era_t_fmt;
	lp->nl_info2[T_FMT_AMPM-_NL_OLD_SIZE] = lp->lc_time->t_fmt_ampm;
	lp->nl_info2[ALT_DIGITS-_NL_OLD_SIZE] = lp->lc_time->alt_digits;
	lp->nl_info2[NOEXPR-_NL_OLD_SIZE] = lp->lc_resp->noexpr;
	lp->nl_info2[YESEXPR-_NL_OLD_SIZE] = lp->lc_resp->yesexpr;
	lp->nl_info2[__UNUSED_1-_NL_OLD_SIZE] = "";

    } else if (lp->core.hdr.size >= sizeof(_LC_locale_t)){
	/*
	 * This is an 'old _LC_time_t object', but a NEW _LC_locale_t object.
	 * Put null strings in those fields that didn't exist before.
	 */
	int i;

	lp->nl_info[ERA_D_T_FMT] = "";
	for(i=0; i< (_NL_NUM_ITEMS - _NL_OLD_SIZE); i++)
	    lp->nl_info2[i] = "";
    }

    return lp->lc_time;
}


/*
*  FUNCTION: locale_init
*
*  DESCRIPTION:
*  Initialization method for the locale handle.
*/
_LC_locale_t *
__locale_init(_LC_locale_t *lp)
{
#define TINIT(o)	if (lp->o->core.init) METHOD(lp->o, init)(lp)

    TINIT(lc_charmap);
    TINIT(lc_collate);
    TINIT(lc_ctype);
    TINIT(lc_monetary);
    TINIT(lc_numeric);
    TINIT(lc_resp);
    TINIT(lc_time);

  return lp;
}

