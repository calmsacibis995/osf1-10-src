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
/* DEC/CMS REPLACEMENT HISTORY, Element GEERR.H*/
/* *2     3-MAR-1992 17:20:53 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:49:05 PARMENTER "Rags"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEERR.H*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEERR.H*/
/* *3    25-JAN-1991 16:56:18 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 12:36:16 FITZELL "V3 IFT update"*/
/* *1     8-NOV-1990 11:21:49 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEERR.H*/
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
**	GEERR.H        		Default error list
**
**  ABSTRACT:
**
**	This list is compiled in.  It is used if the external file geErr.dat
**      does not exist at run time, in either the current dir. or the one
**      pointed to by RAGS_SUPPORT_DIR.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 03/10/88 Created
**
**--
**/

/* static char GEErrSid[] = "%W% %G%";                                       */

static char *GenErrs[] = {
"RAGS-F-GKSLEVEL, Implementation level of GKS (",
")is incorrect for proper functioning of RAGS.  See your",
"system manager about installing required GKS Level = (",
"RAGS-F-NOMSGFILE, The required Msg file geMsg.dat is corrupted.",
"Please repair it or copy over a new version from the RAGS-Release area.",
"RAGS-F-NOMSGFILE, The required Msg file geMsg.dat is non-existant.",
"Please copy over a new version from the RAGS-Release area.",
"RAGS-F-WSCLASS, Your video device is of the wrong class for RAGS to",
"function properly.  It should be a RASTER or VECTOR class device",
"RAGS-W-UNCLASSIFIED, Unclassified internal error detected.",
"Please report circumstances to RAGS-Maintenance.",
"RAGS CONTINUING....reluctantly...",
"RAGS ABORTING !!!",
"RAGS-I-NOHLPFILE, The required HELP file geHlp.dat is non-existent.",
"The help facility will be unavailable.",
"RAGS-W-ERRRANGE, Numeric message label in geErr.dat exceeds Max = ",
"RAGS-W-HLPRANGE, Numeric message label in geHlp.dat exceeds Max = ",
"RAGS-W-MSGRANGE, Numeric message label in geMsg.dat exceeds Max = [23 ",
"Message ignored.",
"RAGS-F-SEGCHAIN, Segment chain has been corrupted -",
"contact RAGS maintenance.",
"RAGS-F-MEMERROR, System unable to allocate additional requested memory ",
"- contact your system manager.",
"RAGS-E-METAOERROR, Unable to open OUTPUT meta file",
"- check free disk space. WRITE request IGNORED.",
"RAGS-E-METAIERROR, Unable to open INPUT meta file",
"- READ request IGNORED.",
"RAGS-E-VERSION, The input file is not recognized as a legitimate RAGS metafile - Please check the filename and/or its contents.",
"- READ request ignored.",
"RAGS-F-XOPENDISP, X could not open the display",
"RAGS-E-BADMETA, All or part of  INPUT meta file is corrupt - ",
"RAGS CONTINUING....",
"RAGS-W-NOFNTFILE, Cannot open font list geFnt.dat - No text processing",
"RAGS-W-TOOMANYFONTS, Too many fonts in geFnt.dat -  Overflow ignored.",
"RAGS-W-BADFNTFILE, The following font is causing an error - ",
"All subsequent entries are IGNORED.",
"RAGS-W-NOCOLFILE, Cannot open color list geCol.dat - No Color list selection from color palette.",
"RAGS-W-TOOMANYCOLS, Too many colors in geCol.dat -   Overflow ignored.",
"RAGS-W-BADCOLFILE, The following entry in geCol.dat is causing an error - ",
"RAGS-E-HARDCOL, XAllocColorCells has returned an error, no more color settings are available.",
"RAGS-E-NOSUPFILE, Cannot open set-up file geSup.dat.",
"RAGS-E-BADSUPFILE, Set-up file (geSup.dat) is corrupt.",
"RAGS-E-TOOMANYSUPS, Too many entries in set-up file geSup.dat.",
"RAGS-F-PAGCHAIN, Page chain has been corrupted -",
"RAGS-E-IMAGEIERROR, Unable to open INPUT IMAGE file",
"RAGS-E-BADIMGFILE, Image file is corrupt or does not match the specified type.",
"RAGS-E-DDIF, DDIF has returned an error - Error # = ",
"- I/O request ignored.",
"  has incorrect crop settings (crop width and/or height <= 0); processing for this file is terminated.",
"RAGS-E-UNAVAILABLE, The selected function is not yet available.",
"RAGS-E-IOERROR, Unable to open file ...",
"RAGS-E-WRONG-WINDOW, You must select a point in the drawing window which belongs to the File pulldown originating this operation.",
"RAGS-E-NOWIDGET, Cannot find widget",
"RAGS-E-NOHIERARCHY, Cannot open widget hierarchy -",
"RAGS-E-POL-MEMBER, Only ARCs and LINEs may be POLYGON constiuents",
NULL
};

