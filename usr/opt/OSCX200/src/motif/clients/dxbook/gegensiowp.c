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
static char SccsId[] = "@(#)gegensiowp.c	1.1\t11/22/88";
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
**	GEGENSIOWP	             	       Read-Write Line Weight, Pattern
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
**	GNE 03/19/87 Created
**
**--
**/

#include "geGks.h"


geGenSIOWP(cmd, LinW, Pat)
long             cmd, *LinW;
struct GE_PAT	 *Pat;
{                       
int             pattern, DashLen, DashList0, DashList1, DashList2, DashList3,
                Lw, i;
char		*error_string;

switch(cmd)
   {case GEWRITE:
     /*
      * Check the integrity of the line pattern and adjust it if necessary
      */
     if (Pat->LinP != LineSolid)
	{for (i = 3; i >= 0; i--)
	    if (Pat->DashList[i] <= 0) Pat->DashLen = i;

    	 if (Pat->DashLen < 2 || Pat->DashLen > 4)
		Pat->LinP = LineSolid;
    	}

     fprintf(geFdO, " %d %d",
	     *LinW,
	      Pat->LinP);
     if (Pat->LinP != LineSolid)
     	fprintf(geFdO, " %d %d %d %d %d",
		Pat->DashLen,
	      	Pat->DashList[0],
	      	Pat->DashList[1],
	      	Pat->DashList[2],
	      	Pat->DashList[3]);
     break;

    case GEREAD:
     geFscanf2(geFdI, " %d %d",
	       &Lw,
	       &pattern);
     if (geFeof(geFdI)) 
       {error_string = (char *) geFetchLiteral("GE_ERR_BADMETA", MrmRtypeChar8);
        if (error_string != NULL) 
	   {geError(error_string, FALSE);
	    XtFree(error_string);
	   }
	return(GERRBADMETA);
       }	
     *LinW      = Lw;
     Pat->LinP = pattern;
     if (Pat->LinP != LineSolid)
     	{geFscanf5(geFdI, " %d %d %d %d %d",
		   &DashLen,
		   &DashList0,
		   &DashList1,
		   &DashList2,
		   &DashList3);
     	if (geFeof(geFdI)) 
          {error_string = (char *) geFetchLiteral("GE_ERR_BADMETA", MrmRtypeChar8);
           if (error_string != NULL) 
	     {geError(error_string, FALSE);
	      XtFree(error_string);
	     }
	   return(GERRBADMETA);
          }	
	Pat->DashList[0] = DashList0;
	Pat->DashList[1] = DashList1;
	Pat->DashList[2] = DashList2;
	Pat->DashList[3] = DashList3;
	Pat->DashLen 	 = DashLen;
  	/*
	 * Figure out the index
	 */
	Pat->DashIndx = 3;
	for (i = 0; i < 3; i++)
	   {if (Pat->DashList[0] == geDashList[i][0] &&
	        Pat->DashList[1] == geDashList[i][1] &&
	        Pat->DashList[2] == geDashList[i][2] &&
	        Pat->DashList[3] == geDashList[i][3])
		{Pat->DashIndx = i;
	   	 break;
		}
	   }
       }
     else
      {Pat->DashList[0] = Pat->DashList[1] = 4; Pat->DashList[2] =
       Pat->DashList[3] = 0; Pat->DashLen = 2;
       Pat->DashIndx    = 0;
      }
     /*
      * Check the integrity of the line pattern and adjust it if necessary
      */
     if (Pat->LinP != LineSolid)
   	{for (i = 3; i >= 0; i--)
	    if (Pat->DashList[i] <= 0) Pat->DashLen = i;

         if (Pat->DashLen < 2 || Pat->DashLen > 4)
	    {Pat->LinP = LineSolid;
	     Pat->DashIndx = 0;
	    }
        }

     break;

   default:
     break;
   }
return(0);
}
