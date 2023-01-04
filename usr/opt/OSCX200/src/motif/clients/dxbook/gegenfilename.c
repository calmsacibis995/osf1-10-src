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
static char SccsId[] = "@(#)gegenfilename.c	1.1\t11/22/88";
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
**	GEGENFILENAME		Generates full file path from given name
**
**  ABSTRACT:
**				This routine prepends the path to either the
**				user's HOME directory or the system directory
**				whichever is defined and contains the file
**				in question.  If neither of these paths is
**				defined (as is the case for external users of
**				the RAGS callable interface - geDispArt), then
**				this routine returns FALSE.  If at least the
**				system path is defined (geRagLoc), then this
**				routine returns TRUE.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 03/02/88 Created
**
**--
**/

#include "geGks.h"
         

geGenFileName(Name)
char *Name;
{ 
FILE	*fd;


if (!Name || !strlen(Name))
   {*geUtilBuf = '\0';
    return(FALSE);
   }

if (geUsrLoc && strlen(geUsrLoc))		/* Try finding it in the     */
   {strcpy(geUtilBuf, geUsrLoc);		/* HOME directory.	     */
    strcat(geUtilBuf, Name);

    if ((fd = fopen(geUtilBuf, "r")))
    	{fclose(fd);				/* Found it in HOME dir.     */
	 return(TRUE);
	}
   }

if (geRagLoc && strlen(geRagLoc))		/* Not in HOME dir.  If path */
   {strcpy(geUtilBuf, geRagLoc);		/* to system dir. is defined */
    strcat(geUtilBuf, Name);			/* assume it's going to be   */
    return(TRUE);
   }						/* there.		     */

return(FALSE);					/* COULDN'T FIND IT anywhere */
}


