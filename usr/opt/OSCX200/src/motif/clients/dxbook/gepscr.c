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
static char SccsId[] = "@(#)gepscr.c	1.2\t4/17/89";
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
**	GEPSCR			
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
**	MAS 08/27/91 Created     
**
**--
**/

#include "geGks.h"
#include <X11/Xutil.h>
#ifdef VMS
#include <unixio.h>
#endif VMS
#ifdef __osf__
#include <sys/stat.h>
#else
#include <stat.h>
#endif
#include "cupdps.h"


#ifndef __osf__
extern int  cupdps_figurebuf_info();
#endif

unsigned char   *gePsCr (filename, widthp, heightp,
			llxp, llyp, urxp, uryp, buflenp)
    char   *filename;
    int    *widthp,
           *heightp,
	   *llxp, *llyp, *urxp, *uryp,
	   *buflenp;
{
    FILE    *f;

    char    *error_string,
            *psbuf;
    struct stat  stat_buf;
    unsigned info_flags;
    float   struct_vers, epsf_vers;


    if (geMemIO.InMem) {
	*buflenp = geMemIO.NumBytes;
	psbuf	 = geMemIO.PtrS;
    } else {

	geGenFileDir (filename, geDefDirInPs);
	geGenFileExt (filename, geDefProofPs, FALSE);

	if (!(f = fopen(filename, "r"))) {
	    error_string = (char *) geFetchLiteral
					("GE_ERR_XOPENEPS", MrmRtypeChar8);
	    if (error_string != NULL) {
		sprintf (geErrBuf, error_string, geUtilBuf);
		geError (geErrBuf, FALSE);
		XtFree (error_string);
		if (f) fclose(f);
	    }
	    return (NULL);
	}

	/* Determine size of buffer - "stat" funct won't work across net */
	if (stat(filename, &stat_buf)  != -1) {
	    *buflenp = (int) stat_buf.st_size;
	} else {
	    *buflenp = 0;
	    while (fgetc(f) != EOF) (*buflenp)++;
	    rewind(f);
	}

	/* Allocate the memory for the buffer */
	psbuf = (char *) geMalloc((*buflenp)+2); /* Leave space for \n and \0 */

	/* Read the file into the buffer */
	*buflenp = fread(psbuf, sizeof(char), *buflenp, f);
	fclose(f);

        /* If there's no CR at end of file, add one */
        if (psbuf[*buflenp - 1] != '\n') {
	    psbuf[*buflenp] = '\n';
	    *buflenp++;
	}

	/* If it's not null terminated, add one */
	if (psbuf[*buflenp - 1] != '\0') {
	    psbuf[*buflenp] = '\0';
	    *buflenp++;
	}
    }

    /* First get sizing info on the figure */
#ifndef __osf__
    cupdps_figurebuf_info(psbuf, *buflenp, geDispDev, geScreen,
	llxp, llyp, urxp, uryp, widthp, heightp,
	&info_flags, &struct_vers, &epsf_vers);
#endif

    if (!(CUPDPS_PS_FILE & info_flags) ||
	!(CUPDPS_PS_STRUCT & info_flags) ||
	!(CUPDPS_PS_EPSF & info_flags)) {
	error_string = (char *) geFetchLiteral ("GE_ERR_NOTEPSFILE", MrmRtypeChar8);
	if (error_string != NULL) {
	    sprintf (geErrBuf, error_string, geUtilBuf);
	    geError (geErrBuf, FALSE);
	    XtFree (error_string);
	    geFree(&psbuf, *buflenp);
	}
	return (NULL);
    }

    if (!(CUPDPS_PS_BBOX & info_flags) ||
	!(CUPDPS_PS_BBOXVALS & info_flags)) {
	error_string = (char *) geFetchLiteral ("GE_ERR_BADBBOX", MrmRtypeChar8);
	if (error_string != NULL) {
	    sprintf (geErrBuf, error_string, geUtilBuf);
	    geError (geErrBuf, FALSE);
	    XtFree (error_string);
	    geFree(&psbuf, *buflenp);
	}
	return (NULL);
    }

    return((unsigned char *) psbuf);
}
