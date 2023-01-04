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
/* DEC/CMS REPLACEMENT HISTORY, Element GEFNTCONV.H*/
/* *3    19-MAR-1992 13:41:07 GOSSELIN "added new RAGS support"*/
/* *2     3-MAR-1992 17:21:10 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:49:15 PARMENTER "Rags"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEFNTCONV.H*/
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	GEFNTCONV.H        		Font converters
**
**  ABSTRACT:
**
** This file contains the font converter vectors (FntConv"x").
**
** It is inevitable that the font list will change from time to time.  For example,
** new fonts may be added and others may be deleted, or the order of the fonts
** may have to change.  Since it is the index into the font list which is saved
** within the meta file, it is essential to provide a mechanism whereby the reorder-
** ing of the font list is transparent to the user.  This is the function of the
** converter vectors.  Each converter vector is a mapping from the old font list
** to the next newer font list.  The vector index = the old font list offset;
** the "contents" = the newer font list offset.  See the example below, illustrating
** the tracking of a font "fnt" from its original position in FontListA to FontListB,
** C and finally to D (wherein it no longer exists, in which case it is mapped to the
** default font = entry # 0).
**
** FontList A         FontList B         FontList C         FontList D
**       |                  |                  |                  |
**Index  |  Vec1     Index  |  Vec2     Index  |  Vec3     Index  |
** |     |   |        |     |   |        |     |   |        |     |
** V    _V___V_       V    _V___V_       V    _V___V_       V    _V_
** 0   |   |   |      0   |   |   |      0   |   |   |   -->0   |   |
**      -------            -------            -------   |        ---
** 1   |   |   |   -->1   |fnt|n-1|--    1   |   |   |  |   1   |   |
**      -------   |        -------   |        -------   |        ---
** 2   |fnt| 1 |--    2   |   |   |  |   2   |   |   |  |   2   |fnt|
**      -------            -------   |        -------   |        ---
**       . . .              . . .    |         . . .    |       . . .
**       . . .              . . .    |         . . .    |       . . .
**       . . .              . . .    |         . . .    |       . . .
**      -------            -------   |        -------   |        ---
** n-1 |   |   |      n-1 |   |   |   -->n-1 |fnt| 0 |--    n-1 |   |
**      -------            -------            -------            ---
**  n  |   |   |                          n  |   |   |       n  |   |
**      -------                               -------            ---
**                                                          n+1 |   |
**                                                               ___
**
** The above diagram shows the following history of the font named "fnt".  In the
** first font list (A) it occupied slot # 2.  In the next revision of the font
** list (B) it was moved to slot # 1 (thus FntConv1[2] = 1).  In the following
** revision of the font list (C) it was moved to slot # n-1
** (thus FntConv2[1] = n-1).  In the latest revision of the font list, "fnt" was
** elliminated altogether (thus FntConv3[n-1]=0, indicating the default font).
** Note that the latest (current) font list does not have a converter vector
** associated with it.  When and if FontList "E" should ever be generated, then
** it would be necessary to create converter vector number "4",
** to map from D to E.
**
** To find the current font index of "fnt" (Di) given that it is coming from an old
** meta file corresponding to font list C, B or A, wherein the index encoded within
** the meta file is Ci, Bi or Ai, respectively, the following conversions
** would be performed:
**
** Version MetaIndex  Mapping                          Fnt example (result = 0)
**    D        Di       None                                -
**    C        Ci Di=FntConv3[Ci]                     FntConv3[n-1]
**    B        Bi Di=FntConv3[FntConv2[Bi]]           FntConv3[FntConv2[1]]
**    A        Ai Di=FntConv3[FntConv2[FntConv1[Ai]]] FntConv3[FntConv2[FntConv1[2]]]
**
**  Note that "FntConv"x"Max" is the total number of entries in the vector.
**  FntConv2Custom is the adjustment that must be made to accomodate the
**  user specific custom fonts that each user may have in geFnt.dat.  The first
**  font in this file is tacked onto the static list, so that for example, if
**  the "last" font in the "previous" set was 123 and the "last" font in the
**  "new" set is 158, then the custom fonts "used" to run from 124 on, and
**  now they must run from 159 on.  So 158 - 123 = 35, must be added to each
**  custom font index from the "previous" font set.  Therefore, FntConv_Custom
**  would be +35.
**   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,   ,
**  ,  ,  ,  ,  ,  ,  ,  ,  ,  ,  ,  ,  ,  ,  ,
** 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 03/23/88 Created
**
**--
**/

/* static char GEFntConvSid[] = "@(#)geFntConv.h	1.1 11/22/88";                                   */

/* 
 * FntConv1 is for converting from version (1) -> 2
 *
 * The new font set indeces run from 0 to 146.
 */
static short FntConv1Max = 62, FntConv1[] = {
  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,
  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60, 126, 127, 128, 129,
 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
 145, 146
};
/* 
 * FntConv2 is for converting from versions (2, 3, 4) -> 5
 *
 * The new font set indeces run from 0 to 173 (27 new fonts)
 *
 * New Default = 25
 */
static short FntConv2Max = 147, FntConv2Custom = 27, FntConv2[] = {
   1,   2,   3,   4,   5,   7,   8,   9,  10,  11,  13,  14,  15,  16,  17,
  19,  20,  21,  22,  23,  25,  26,  27,  28,  29,  31,  32,  33,  34,  35,
  37,  38,  39,  40,  41,  43,  44,  45,  46,  47,  25,  49,  50,  51,  52,
  53,  55,  56,  57,  58,  59,  61,  62,  63,  64,  65,  67,  68,  69,  70,
  71,  73,  74,  75,  76,  77,  79,  80,  81,  82,  83,  85,  86,  87,  88,
  89,  91,  92,  93,  94,  95,  97,  98,  99, 100, 101, 103, 104, 105, 106,
 107, 109, 110, 111, 112, 113, 115, 116, 117, 118, 119, 121, 122, 123, 124,
 125, 127, 128, 129, 130, 131, 133, 134, 135, 136, 137, 139, 140, 141, 142,
 143, 145, 146, 147, 148, 149, 151, 152, 153, 154, 155, 157, 158, 159, 160,
 161, 163, 164, 165, 166, 167, 169, 170, 171, 172, 173, 25
};

