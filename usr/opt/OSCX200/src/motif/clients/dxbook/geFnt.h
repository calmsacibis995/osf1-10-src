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
/* DEC/CMS REPLACEMENT HISTORY, Element GEFNT.H*/
/* *3    19-MAR-1992 13:40:49 GOSSELIN "added new RAGS support"*/
/* *2     3-MAR-1992 17:21:04 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:49:11 PARMENTER "Rags"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEFNT.H*/
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
**	GEFNT.H        		Default font list
**
**  ABSTRACT:
**
** This file contains the list of fonts compiled into RAGS and presented to the
** the user in the font selection window.  This list must be in a one-to-one
** correspondence with the list of fonts contained in geGlobals.c
**
** The currently supported font list (GenFnts) are those fonts which
** we deem to be always available on all DECwindows supporting
** platforms.  Thus, any user on any platform creating text using this set of
** fonts will be guaranteed that the text will display in the same font on
** any other platform.
**
** geFntPtr      contains the font names as they are presented to the user in the
**               Font Menu.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 03/08/88 Created
**
**--
**/

/* static char GEFntSid[] = "@(#)geFnt.h	1.1 11/22/88";              */

static char *GenFnts[] =
{"AvantGarde-Book8",
 "AvantGarde-Book10",
 "AvantGarde-Book12",
 "AvantGarde-Book14",
 "AvantGarde-Book18",
 "AvantGarde-Book24",
            
 "AvantGarde-BookOblique8",
 "AvantGarde-BookOblique10",
 "AvantGarde-BookOblique12",
 "AvantGarde-BookOblique14",
 "AvantGarde-BookOblique18",
 "AvantGarde-BookOblique24",

 "AvantGarde-Demi8",
 "AvantGarde-Demi10",
 "AvantGarde-Demi12",
 "AvantGarde-Demi14",
 "AvantGarde-Demi18",
 "AvantGarde-Demi24",

 "AvantGarde-DemiOblique8",
 "AvantGarde-DemiOblique10",
 "AvantGarde-DemiOblique12",
 "AvantGarde-DemiOblique14",
 "AvantGarde-DemiOblique18",
 "AvantGarde-DemiOblique24",
             
 "Courier8",
 "Courier10",
 "Courier12",
 "Courier14",
 "Courier18",
 "Courier24",

 "Courier-Bold8",
 "Courier-Bold10",
 "Courier-Bold12",
 "Courier-Bold14",
 "Courier-Bold18",
 "Courier-Bold24",

 "Courier-BoldOblique8",
 "Courier-BoldOblique10",
 "Courier-BoldOblique12",
 "Courier-BoldOblique14",
 "Courier-BoldOblique18",
 "Courier-BoldOblique24",

 "Courier-Oblique8",
 "Courier-Oblique10",
 "Courier-Oblique12",
 "Courier-Oblique14",
 "Courier-Oblique18",
 "Courier-Oblique24",

 "Helvetica8",
 "Helvetica10",
 "Helvetica12",
 "Helvetica14",
 "Helvetica18",
 "Helvetica24",

 "Helvetica-Bold8",
 "Helvetica-Bold10",
 "Helvetica-Bold12",
 "Helvetica-Bold14",
 "Helvetica-Bold18",
 "Helvetica-Bold24",

 "Helvetica-BoldOblique8",
 "Helvetica-BoldOblique10",
 "Helvetica-BoldOblique12",
 "Helvetica-BoldOblique14",
 "Helvetica-BoldOblique18",
 "Helvetica-BoldOblique24",

 "Helvetica-Oblique8",
 "Helvetica-Oblique10",
 "Helvetica-Oblique12",
 "Helvetica-Oblique14",
 "Helvetica-Oblique18",
 "Helvetica-Oblique24",

 "LubalinGraph-Book8",
 "LubalinGraph-Book10",
 "LubalinGraph-Book12",
 "LubalinGraph-Book14",
 "LubalinGraph-Book18",
 "LubalinGraph-Book24",

 "LubalinGraph-BookOblique8",
 "LubalinGraph-BookOblique10",
 "LubalinGraph-BookOblique12",
 "LubalinGraph-BookOblique14",
 "LubalinGraph-BookOblique18",
 "LubalinGraph-BookOblique24",

 "LubalinGraph-Demi8",
 "LubalinGraph-Demi10",
 "LubalinGraph-Demi12",
 "LubalinGraph-Demi14",
 "LubalinGraph-Demi18",
 "LubalinGraph-Demi24",

 "LubalinGraph-DemiOblique8",
 "LubalinGraph-DemiOblique10",
 "LubalinGraph-DemiOblique12",
 "LubalinGraph-DemiOblique14",
 "LubalinGraph-DemiOblique18",
 "LubalinGraph-DemiOblique24",
                   
 "NewCenturySchlbk-Bold8",
 "NewCenturySchlbk-Bold10",
 "NewCenturySchlbk-Bold12",
 "NewCenturySchlbk-Bold14",
 "NewCenturySchlbk-Bold18",
 "NewCenturySchlbk-Bold24",

 "NewCenturySchlbk-BoldItalic8",
 "NewCenturySchlbk-BoldItalic10",
 "NewCenturySchlbk-BoldItalic12",
 "NewCenturySchlbk-BoldItalic14",
 "NewCenturySchlbk-BoldItalic18",
 "NewCenturySchlbk-BoldItalic24",

 "NewCenturySchlbk-Italic8",
 "NewCenturySchlbk-Italic10",
 "NewCenturySchlbk-Italic12",
 "NewCenturySchlbk-Italic14",
 "NewCenturySchlbk-Italic18",
 "NewCenturySchlbk-Italic24",

 "NewCenturySchlbk-Roman8",
 "NewCenturySchlbk-Roman10",
 "NewCenturySchlbk-Roman12",
 "NewCenturySchlbk-Roman14",
 "NewCenturySchlbk-Roman18",
 "NewCenturySchlbk-Roman24",

 "Souvenir-Demi8",
 "Souvenir-Demi10",
 "Souvenir-Demi12",
 "Souvenir-Demi14",
 "Souvenir-Demi18",
 "Souvenir-Demi24",

 "Souvenir-DemiItalic8",
 "Souvenir-DemiItalic10",
 "Souvenir-DemiItalic12",
 "Souvenir-DemiItalic14",
 "Souvenir-DemiItalic18",
 "Souvenir-DemiItalic24",

 "Souvenir-Light8",
 "Souvenir-Light10",
 "Souvenir-Light12",
 "Souvenir-Light14",
 "Souvenir-Light18",
 "Souvenir-Light24",

 "Souvenir-LightItalic8",
 "Souvenir-LightItalic10",
 "Souvenir-LightItalic12",
 "Souvenir-LightItalic14",
 "Souvenir-LightItalic18",
 "Souvenir-LightItalic24",

 "Symbol8",
 "Symbol10",
 "Symbol12",
 "Symbol14",
 "Symbol18",
 "Symbol24",

 "Times-Bold8",
 "Times-Bold10",
 "Times-Bold12",
 "Times-Bold14",
 "Times-Bold18",
 "Times-Bold24",

 "Times-BoldItalic8",
 "Times-BoldItalic10",
 "Times-BoldItalic12",
 "Times-BoldItalic14",
 "Times-BoldItalic18",
 "Times-BoldItalic24",

 "Times-Italic8",
 "Times-Italic10",
 "Times-Italic12",
 "Times-Italic14",
 "Times-Italic18",
 "Times-Italic24",

 "Times-Roman8",
 "Times-Roman10",
 "Times-Roman12",
 "Times-Roman14",
 "Times-Roman18",
 "Times-Roman24",

 NULL
};
