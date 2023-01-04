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
/* DEC/CMS REPLACEMENT HISTORY, Element GEMSG.C*/
/* *4    23-MAY-1991 10:40:44 ACKERMAN "cleaning up RAGS error messages, calls bkr_error_modal()"*/
/* *3    25-JAN-1991 17:00:21 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 12:40:02 FITZELL "V3 IFT update"*/
/* *1     8-NOV-1990 11:24:11 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEMSG.C*/
/*  DEC/CMS REPLACEMENT HISTORY, Element GEMSG.C */
/*  *10   22-MAY-1991 16:39:16 PARMENTER "using mrmfetchbitmapliteral, using XUIisWMrunning, removing printf's, fixing huge shelf */
/* accvio, fixing kb traversal" */
/*  *9     9-OCT-1990 13:23:08 FERGUSON "Updated RAGS with Ultrix ISL support/Common source pool between VMS & Ultrix again" */
/*  *8     6-APR-1990 17:17:39 KRAETSCH "replace NULL with 0 for null char" */
/*  *7    12-APR-1989 09:43:43 FERGUSON "V2 update to RAGS modules" */
/*  *6     4-OCT-1988 16:35:06 FERGUSON "FT2 update fixes, qar, etc..." */
/*  *5    11-MAY-1988 17:18:58 FERGUSON "Update graphics display routines" */
/*  *4    11-APR-1988 15:52:23 FERGUSON "Update graphics display routines" */
/*  *3    27-FEB-1988 19:57:05 GEORGE "Add copyright" */
/*  *2    12-JAN-1988 11:56:59 FERGUSON "Change the logical name VOILA$LIBRARY to DECW$BOOK" */
/*  *1     8-JAN-1988 12:02:02 FERGUSON "Initial Entry" */
/*  DEC/CMS REPLACEMENT HISTORY, Element GEMSG.C */
static char SccsId[] = "%W%\t%G%";
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
**	GEMSG			Loads ERROR HELP and MESSAGE files
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
**	GNE 11/03/86 Created
**
**--
**/

#include "geGks.h"
#include "geErr.h"
#include <stdio.h>
         

extern char *geMalloc();

geMsg()
{                       
static FILE	*fd;
static int      i;
static long	msgnum;
/*
 * Read the ERROR file messages
 */

#ifdef GERAGS

geGenFileName("geErr.dat");
/*
 * If there is no error file, then use the messages in the header file.
 */
if (!(fd = fopen(geUtilBuf, "r")))
  {GEMSGALLOC(geErrPtr[0], " ");                /* Leaving the first slot    */
   i = 1;                                       /*open allows the ptr index  */
   while (GenErrs[i])                           /*to line up with the message*/
     {geErrPtr[i] = GenErrs[i - 1]; i++;}       /*numbers, which start at 1  */

   geErrPtr[i] = NULL;
  }
else
  {while ((fscanf(fd, "%d", &msgnum) != EOF) && fgetc(fd)
	  && fgets(geUtilBuf, GE_MAX_MSG_CHAR - 1, fd))
     {if (msgnum == 0) continue;                /* This is just a comment    */
      if (msgnum == -1) break;                  /* This is just a comment    */
      if (msgnum > GE_MAX_ERRS)	{geError(GERRTOOMANYERRS); break;}
      i = strlen(geUtilBuf);
      geUtilBuf[strlen(geUtilBuf) - 1] = '\0';
      GEMSGALLOC(geErrPtr[msgnum], geUtilBuf);
     }
   fclose(fd);
  }


/*
 * Read the HELP file messages
 */
geGenFileName("geHlp.dat");

if ((fd = fopen(geUtilBuf, "r")) == NULL) geError(GERRNOHLPFILE);

while ((fscanf(fd, "%d", &msgnum) != EOF) && fgetc(fd)
       && fgets(geUtilBuf, GE_MAX_MSG_CHAR - 1, fd))
   {if (msgnum == 0) continue;                  /* This is just a comment    */
    if (msgnum == -1) break;                    /* This is just a comment    */
    if (msgnum > GE_MAX_HLPS)	{geError(GERRTOOMANYHLPS); break;}
    geUtilBuf[strlen(geUtilBuf) - 1] = NULL;
    GEMSGALLOC(geHlpPtr[msgnum], geUtilBuf);
   }
fclose(fd);
/*
 * Read the MESSAGE file messages
 */
geGenFileName("geMsg.dat");

if ((fd = fopen(geUtilBuf, "r")) == NULL) {geError(GERRNOMSGFILE); return;}

while ((fscanf(fd, "%d", &msgnum) != EOF) && fgetc(fd)
       && fgets(geUtilBuf, GE_MAX_MSG_CHAR - 1, fd))
   {if (msgnum == 0) continue;                  /* This is just a comment    */
    if (msgnum == -1) break;                    /* This is just a comment    */
    if (msgnum > GE_MAX_MSGS)	{geError(GERRTOOMANYMSGS); break;}
    geUtilBuf[strlen(geUtilBuf) - 1] = NULL;
    GEMSGALLOC(geMsgPtr[msgnum], geUtilBuf);
   }
fclose(fd);

#endif

}
