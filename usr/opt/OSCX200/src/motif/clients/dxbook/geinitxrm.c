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
static char SccsId[] = "@(#)geinitxrm.c	1.1\t11/22/88";
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
**	GEINITXRM	             	       Initialize Resource Manager
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
**	DAA 10/17/88 Created
**
**--                
**/

#include "geGks.h"

geInitXrm()
{
static   Initialized = FALSE;
char	 *error_string, *b;
long     GraVersionSys, GraVersionUser;

XrmValue v, unit_def;

/* Initialize the Resource Manager structure.  Get the user's database.
 * If it doesn't exist, get the system database.  Merge the two databases.
 */

if (!Initialized)
   {XrmInitialize();
    Initialized = TRUE;
   }

geRM.UserDb = geRM.SysDb = NULL;

strcpy(geUtilBuf, geRagLoc);
strcat(geUtilBuf, "gra.dat");
geRM.SysDb = XrmGetFileDatabase(geUtilBuf);

if (!geRM.SysDb)
  {             		/* must be hard coded text because uid file */
#ifdef GERAGS                   /* has not been read in yet 		    */
    printf ("\n The required RESOURCE file, 'gra.dat', is either non-existent\n");
    printf (" is the wrong version or is corrupted.\n");
    printf (" Please copy over a new version from the Release area.\n");
    printf (" ABORTING !!!\n");
   _exit(0);
#endif
   return;
  }
else
  {if (XrmGetResource (geRM.SysDb, "gra.versionid", NULL, &b, &v))
     GraVersionSys = atoi(v.addr);
   else
     GraVersionSys = 0;	/* Should never happen */
  }

strcpy(geUtilBuf, geUsrLoc);
strcat(geUtilBuf, "gra.dat");
geRM.UserDb = XrmGetFileDatabase(geUtilBuf);

if (geRM.UserDb)
  {if (XrmGetResource (geRM.UserDb, "gra.versionid", NULL, &b, &v))
     GraVersionUser = atoi(v.addr);
   else
     GraVersionUser = 0;

   if (GraVersionUser == GraVersionSys)
     {geState.GraDatVersionMatch = TRUE;
      XrmMergeDatabases(geRM.UserDb, &geRM.SysDb);
     }
   else
      geState.GraDatVersionMatch = FALSE;
  }
else
  {/* User's GRA.DAT couldn't be loaded */
   /* They probably don't have one (new user) so this is not a problem. */
   geState.GraDatVersionMatch = TRUE;
  }
}        

