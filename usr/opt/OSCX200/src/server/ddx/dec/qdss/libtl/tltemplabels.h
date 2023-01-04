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
/* jumps to template ram public labels */
#define JMPT_INITGET 0x46bf
#define JMPT_SET_MASKED_ALU 0x46a8
#define JMPT_RESET_FORE_BACK 0x469d
#define JMPT_RESET_FAST_DY_SLOW_DX 0x4697
#define JMPT_TILEBITMAP 0x468a
#define JMPT_INITTILEBITMAP 0x467f
#define JMPT_BITBLTDONENOCLIP 0x467e
#define JMPT_BITBLTNOCLIP 0x4671
#define JMPT_INITBITBLTNOCLIP 0x4665
#define JMPT_RAWBTOP 0x4659
#define JMPT_INITRAWBTOP 0x4654
#define JMPT_RAWPTOB 0x4649
#define JMPT_INITRAWPTOB 0x4647
#define JMPT_RESETRASTERMODE 0x4642
#define JMPT_SETRASTERMODE 0x463d
#define JMPT_SETDSTOCRB 0x4637
#define JMPT_SETSRC2OCRB 0x4631
#define JMPT_SETSRC1OCRB 0x462b
#define JMPT_SETDSTOCRA 0x4625
#define JMPT_SETSRC2OCRA 0x461f
#define JMPT_SETSRC1OCRA 0x4619
#define JMPT_SETRGBBACKCOLOR 0x4607
#define JMPT_SETRGBFORECOLOR 0x45f5
#define JMPT_SETMASK 0x45ef
#define JMPT_SETALU 0x45e7
#define JMPT_PATTERNSPANLOOP 0x45de
#define JMPT_PATTERNSPAN 0x45d7
#define JMPT_PATTERNRECTLOOP 0x45cc
#define JMPT_PATTERNRECT 0x45c7
#define JMPT_SOLIDSPANLOOP 0x45be
#define JMPT_SOLIDSPAN 0x45ba
#define JMPT_SOLIDRECT 0x45af
#define JMPT_TILESPANLOOP 0x45a6
#define JMPT_TILESPAN 0x45a2
#define JMPT_TILERECT 0x4597
#define JMPT_INIT1STIPPLE 0x458d
#define JMPT_INIT1TILE 0x4583
#define JMPT_INIT2STIPPLE 0x457c
#define JMPT_INIT2TILE 0x4575
#define JMPT_INITSOLID 0x4574
#define JMPT_ROTILEDONE 0x4573
#define JMPT_ROTILE 0x456b
#define JMPT_INITROTILE 0x4556
#define JMPT_FILLTILERECT 0x454b
#define JMPT_INITFILLTILERECT 0x4540
#define JMPT_INITFGLINE 0x4517
#define JMPT_INITFGBGLINE 0x44ee
#define JMPT_INITFGMASK 0x44c5
#define JMPT_INITFGBGMASK 0x449c
#define JMPT_TEXTAA 0x4493
#define JMPT_INITTEXTAA 0x447d
#define JMPT_TEXTTERM 0x4473
#define JMPT_INITTEXTTERM 0x4452
#define JMPT_TEXTSOLID 0x4448
#define JMPT_INITTEXTSOLID 0x4427
#define JMPT_TEXTMASK 0x441d
#define JMPT_INITTEXTMASK 0x43fc
#define JMPT_PTOBXY24MASK 0x43d9
#define JMPT_PTOBXY24CLEAN 0x43c5
#define JMPT_PTOBXY24 0x43a2
#define JMPT_PTOBXYMASK 0x4389
#define JMPT_PTOBXY_PLAIN 0x4370
#define JMPT_PTOBXYCLEAN 0x4366
#define JMPT_PTOBXY 0x434d
#define JMPT_SETTRANSLATE 0x4343
#define JMPT_SETRGBSCROLLENABLE 0x4309
#define JMPT_SETRGBPLANEMASK 0x42f7
#define JMPT_SETBLUEPLANEMASK 0x42ef
#define JMPT_SETGREENPLANEMASK 0x42e7
#define JMPT_SETREDPLANEMASK 0x42df
#define JMPT_SETPLANEMASK 0x42d7
#define JMPT_RESETCLIP 0x42cf
#define JMPT_SETCLIP 0x42c7
#define JMPT_SETVIPER24 0x42ba
#define JMPT_SETVIPER 0x42ad
#define JMPT_SETRGBFOREBACKCOLOR 0x428c
#define JMPT_SETFOREBACKCOLOR 0x427f
#define JMPT_SETRGBSCROLLFILL 0x426d
#define JMPT_SETRGBCOLOR 0x425b
#define JMPT_SETCOLOR 0x4253
#define JMPT_SWAPBUFFER1 0x423a
#define JMPT_SWAPBUFFER0 0x4221
#define JMPT_SWAPDONE 0x420c
#define JMPT_INITSWAPDONE 0x41fe
#define JMPT_SWAPINIT 0x41d6
#define JMPT_CLEANSHADELINE 0x41d1
#define JMPT_SHADELINE 0x41bf
#define JMPT_INITSHADELINE 0x41bb
#define JMPT_CLEANSHADESPAN 0x41b6
#define JMPT_SHADESPAN 0x41a4
#define JMPT_INITSHADESPAN 0x41a0
#define JMPT_CLEAR 0x4191
#define JMPT_SCROLLCLEAR 0x4183
#define JMPT_CONPOLYPATTERN 0x4180
#define JMPT_B_LE_A_PATTERN 0x417b
#define JMPT_A_LE_B_PATTERN 0x4176
#define JMPT_INITCONPOLYPATTERN 0x416b
#define JMPT_CONPOLY 0x4168
#define JMPT_B_LE_A 0x4163
#define JMPT_A_LE_B 0x415e
#define JMPT_INITCONPOLY 0x4157
#define JMPT_PATTERNPOLYLINE 0x414d
#define JMPT_INITPATTERNPOLYLINE 0x413c
#define JMPT_FATPOLYLINE 0x4130
#define JMPT_INITFATPOLYLINE 0x412e
#define JMPT_POLYLINE 0x4124
#define JMPT_INITPOLYLINE 0x411c
#define JMPT_BITBLTDONE 0x411b
#define JMPT_BITBLT 0x4111
#define JMPT_INITBITBLT 0x4103
#define JMPT_PLANE_BITMAP 0x40fa
#define JMPT_INIT_PLANE_BITMAP 0x40ee
#define JMPT_COLORBITMAP 0x40e1
#define JMPT_INIT2COLORBITMAP 0x40b3
#define JMPT_INIT1COLORBITMAP 0x4085
#define JMPT_FILLPATTERNRECT 0x407a
#define JMPT_INITFILLPATTERNRECT 0x4074
#define JMPT_ZBLOCKPTOB 0x4068
#define JMPT_INITZBLOCKPTOB 0x4066
#define JMPT_PTOB 0x405a
#define JMPT_INITPTOB 0x405a
#define JMPT_FILLSPANS 0x4051
#define JMPT_INITFILLSPANS 0x404f
#define JMPT_PIXELS 0x4047
#define JMPT_INITPIXELS 0x4040

/* return from template */
#define TEMPLATE_DONE 0x4000
