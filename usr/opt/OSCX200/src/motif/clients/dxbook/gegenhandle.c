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
static char SccsId[] = "@(#)gegenhandle.c	1.2\t3/20/89";
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
**	GEGENHANDLE	             	       Determine handler given handle
**
**  ABSTRACT:
**
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 02/11/87 Created
**
**--
**/

#include "geGks.h"


extern geArc(), geBox(), geBxr(), geCir(), geElp(), geImg(), geLin(),
       gePie(), gePol(), geTxt(), geGrp(), geGrf(), gePs();

geGenHandle(ASegP)
struct GE_SEG	*ASegP;
{                      
char	*error_string; 

if (!strcmp(ASegP->Handle, "ARC")) {ASegP->Handler = geArc; return;}
if (!strcmp(ASegP->Handle, "BOX")) {ASegP->Handler = geBox; return;}
if (!strcmp(ASegP->Handle, "BXR")) {ASegP->Handler = geBxr; return;}
if (!strcmp(ASegP->Handle, "CIR")) {ASegP->Handler = geCir; return;}
if (!strcmp(ASegP->Handle, "ELP")) {ASegP->Handler = geElp; return;}
if (!strcmp(ASegP->Handle, "IMG")) {ASegP->Handler = geImg; return;}
if (!strcmp(ASegP->Handle, "LIN")) {ASegP->Handler = geLin; return;}
if (!strcmp(ASegP->Handle, "PIE")) {ASegP->Handler = gePie; return;}
if (!strcmp(ASegP->Handle, "POL")) {ASegP->Handler = gePol; return;}
if (!strcmp(ASegP->Handle, "EPS")) {ASegP->Handler = gePs ; return;}
/*
 * The "SQR" handler is archaic - at some point trash it?
 */
if (!strcmp(ASegP->Handle, "SQR")) {ASegP->Handler = geBox; return;}
if (!strcmp(ASegP->Handle, "TXT")) {ASegP->Handler = geTxt; return;}
if (!strcmp(ASegP->Handle, "GRP")) {ASegP->Handler = geGrp; return;}
if (!strcmp(ASegP->Handle, "GRF")) {ASegP->Handler = geGrf; return;}
ASegP->Handler = NULL;

error_string = (char *) geFetchLiteral("GE_ERR_BADMETA", MrmRtypeChar8);
if (error_string != NULL) 
  {geError(error_string, FALSE);
   XtFree(error_string);
  }
}
                                                 
