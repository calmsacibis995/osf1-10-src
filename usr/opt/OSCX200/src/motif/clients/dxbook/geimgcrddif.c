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
static char SccsId[] = "@(#)geimgcrddif.c	1.6\t8/29/89";
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
**	GEIMGCRDDIF			Read DDIF bitmap, returns image ptr
**
**  ABSTRACT:
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 10/12/88 Created
**
**--
**/

#include "geGks.h"
#include <X11/Xutil.h>

#ifdef GEISL

#ifdef VMS
#include <img$def.h>
#include <descrip.h>

extern void cupeng_condition_handler();

#else
#include <img/img_def.h>
#endif

#endif

extern char *geMalloc();

static  char 	 *error_string;

XImage *
geImgCrDdif(FileName)
char         *FileName;
{                                                                         

#ifdef GEDDIF
#ifdef GEISL

  int ctx, fid, dummy;
  FILE                    *f;
  XImage *timage, *ximg;


#ifdef VMS
  struct dsc$descriptor_s filenamedsc;
#endif

/*
 * This routine currently can only work using an external file as input, it
 * cannot operate on an in-memory buffer.  As such it will kick out if it is
 * either not provided with a filename or it is provided a bogus filename.
 */

if (!FileName || !strlen(FileName)) return(NULL);

  if (access(FileName, 0))
    {error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
     if (error_string != NULL) 
       {sprintf(geErrBuf, error_string, FileName);
	geError(geErrBuf, FALSE);
	XtFree(error_string);
       }
     return(NULL);
    }

  if (geCdaOpenFile(FileName))
    {error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
     if (error_string != NULL) 
       {sprintf(geErrBuf, error_string, FileName);
	geError(geErrBuf, FALSE);
	XtFree(error_string);
       }
     return(NULL);
    }

  geCdaCloseFile();
	 
#ifdef VMS
  LIB$ESTABLISH(cupeng_condition_handler);
  filenamedsc.dsc$w_length  = strlen(FileName);
  filenamedsc.dsc$a_pointer = FileName;
  filenamedsc.dsc$b_class   = DSC$K_CLASS_S;
  filenamedsc.dsc$b_dtype   = DSC$K_DTYPE_T;
  ctx = ImgOpenDDIFFile(IMG$K_MODE_IMPORT,&filenamedsc,0,0);
#else
  ctx = ImgOpenDDIFFile(IMG$K_MODE_IMPORT,strlen(FileName),FileName,0,0,0);
#endif

  fid = ImgImportDDIFFrame(ctx,0,0,0,0,0);
  geFidToXimage(&ximg, fid);
  ImgDeleteFrame(fid);
  ImgCloseDDIFFile(ctx, 0);

  return(ximg);

#else

return(NULL);

#endif
#endif

}

#ifdef VMS

/* ************************************************************************
 *                                                                        *
 *  COPYRIGHT (c) 1990 BY						  *
 *  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.		  *
 *  ALL RIGHTS RESERVED.                                                  *
 * 									  *
 *  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED *
 *  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE *
 *  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER *
 *  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY *
 *  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY *
 *  TRANSFERRED.                                                          *
 * 									  *
 *  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE *
 *  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT *
 *  CORPORATION.                                                          *
 * 									  *
 *  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS *
 *  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.		  *
 *									  *
 **************************************************************************
 *
 *++
 *FACILITY:
 *	CUPENG_CONDITION_HANDLER
 *
 *ABSTRACT:
 *	This module contains a simple condition handler.
 *
 *ENVIRONMENT:
 *	VAX/VMS only.  These routines use the VMS LIB$ routines,
 *
 *FUNCTIONS:
 *	cupeng_condition_handler()
 *
 *HISTORY:
 *	25 June 90, Mike Swatko.  Created.
 *--
 */

#include <chfdef.h>


void  cupeng_condition_handler(signal, mechanism)

/* To use this condition handler, use the command */
/*	LIB$ESTABLISH(cupeng_condition_handler); */
/* This will establish the condition handler for that procedure and it will */
/* remain in effect for that procedure and all subprocedures until control */
/* is passed back to the parent procedure. */
/* */
/* If an exception is raised while this condition handler is installed */
/* (established), this routine will cause the control to be passed back */
/* to the parent of the routine that installed (established) the */
/* condition handler and give a return status of FALSE. */

    struct chf$signal_array	*signal;
    struct chf$mech_array	*mechanism;
{

    /* Change the error name to be simply FALSE and have that be converted */
    /* and passed back as a return status. */
    signal->chf$l_sig_name = FALSE;

    /* This command will cause control to be passed back to the parent of */
    /* the routine that established the condition handler with the return */
    /* status defined in the line above */
    error_string = (char *) geFetchLiteral("GE_ERR_DDIFNOTBITONAL", MrmRtypeChar8);
    if (error_string != NULL) 
      {geError(error_string, FALSE);
       XtFree(error_string);
      }
    LIB$SIG_TO_RET(signal, mechanism);
/*    ChfSigToRet(signal, mechanism);*/

}

#endif
