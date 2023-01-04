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
 *	CUPDPS
 *
 *ABSTRACT:
 *	This module provides routines to read and display PostScript drawings.
 *	via Display PostScript.
 *
 *	The basic program flow to display a PostScript figure is:
 *	0). Check that DPS is present on the server
 *	1). Use the cupdps_figurexxx_info routine to tell how big the figure is
 *	2). If necessary, create a window of the appropriate size
 *	3). Allocate a PostScript context, giving info on drawing area size
 * 	and clipping parameters.
 *	4). Draw the PostScript with cupdps_draw_ps_file (or cupdps_draw_ps_buf)
 *	5). Wait until the drawing is finished
 *	6). Free the context and it's resources.
 *
 *	It is recommended that you use a new context for every drawing and
 *	for every time you have to redraw a drawing.
 *
 *	These routines are currently designed to display only single page
 *	PostScript drawings.  If the PostScript data actually contains more
 *	than one page, only the first page will be used.
 *
 *ENVIRONMENT:
 *	VAX/VMS
 *
 *FUNCTIONS:
 *	cupdps_check_dps()
 *	cupdps_error_proc()
 *	cupdps_figurefile_info()
 *	cupdps_figurebuf_info()
 *	cupdps_alloc_ctx()
 *	cupdps_get_status()
 *	cupdps_draw_ps_file()
 *	cupdps_draw_ps_buf()
 *	cupdps_wait_finish_drawing()
 *	cupdps_free_ctx()
 *	(also includes a sample main program which will read and display
 *	 a PostScript file in a window)
 *
 *HISTORY:
 *	May 90, Created.  Mike Swatko (MAS).
 *	15 June 90, MAS.  Modified routines to read PS from a buffer instead
 *		of from a file.  Did some reorganization of routines.
 *	29 April 91, MAS.  Renamed cupdps_draw_postscript() to
 *		cupdps_draw_ps_buf() and added routine cupdps_draw_ps_file().
 *		Modified cupdps_figurefile_info() to not alter file pointer
 *		after it returns.
 *	19 Sept 91, MAS.  Added 2 params to cupdps_draw_ps_*() routines.
 *--
 */



#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <DPS/dpsXclient.h>
#include <DPS/dpsexcept.h>
#include "cupdps.h"
#include "cupdps_msg.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define PSBITESIZE	1024
#define BBOXLLX_DEF	0
#define BBOXLLY_DEF	0
#define BBOXURX_DEF	612
#define BBOXURY_DEF	792

#define DEBUG_FLAG	FALSE


static char		*initialCommand = "resyncstart /cupdps_showpage /showpage load def \n/showpage {} def\n";
/* static char		*initialCommand = "{(%stdin) (r) file cvx exec} stopped {/handleerror load stopped pop systemdict /quit get exec} if resyncstart /cupdps_showpage /showpage load def \n/showpage {clientsync} def\n"; */
static char		EPScommand[256] = "";
static char		*bboxstr = "%%BoundingBox:";





int  cupdps_check_dps(dpy)

/* This routine returns TRUE if the Display PostScript extension is present */
/* on the named server, FALSE otherwise. */

    Display	*dpy;

{
    int major_opcode_ret;
    int first_event_ret;
    int first_error_ret;
    int	dpspresent;

    return (XQueryExtension(dpy, "DPSExtension", &major_opcode_ret,
	&first_event_ret, &first_error_ret));
}





void cupdps_error_proc(ctx, errorCode, arg1, arg2)

/* Based heavily on the example implementation error handler in Appendix B.1 */
/* of DPS Client Library Reference Manual. The only main difference between */
/* this procedure and DPSDefaultErrorProc is that this procedure does not */
/* raise exceptions on PostScript language errors and that the error */
/* presentation can be customized. */

    DPSContext		ctx;
    DPSErrorCode	errorCode;
    unsigned long	arg1, arg2;

{
    char		errstr[1024];
    char		arg1str[256];
    char		arg2str[256];
    DPSTextProc		textProc	= DPSGetCurrentTextBackstop();

    errstr[0] = '\0';
    switch(errorCode) {
	case dps_err_ps: {
	    char  *buf = (char *) arg1;
	    DPSBinObj  ary = (DPSBinObj)(buf+DPS_HEADER_SIZE);
	    DPSBinObj  elements;
	    char *error, *errorName;
	    int  errorCount, errorNameCount;
	    int  resyncFlg;

	    assert((ary->attributedType & 0x7f) == DPS_ARRAY);
	    assert(ary->length == 4);

	    elements = (DPSBinObj) (((char *) ary) + ary->val.arrayVal);
	    errorName = (char *) (((char *) ary) + elements[1].val.nameVal);
	    errorNameCount = elements[1].length;

	    error = (char *) (((char *) ary) + elements[2].val.nameVal);
	    errorCount = elements[2].length;

	    resyncFlg = elements[3].val.booleanVal;

	    strncpy(arg1str,errorName,errorNameCount);
	    arg1str[errorNameCount] = '\0';
	    strncpy(arg2str,error,errorCount);
	    arg2str[errorCount] = '\0';
	    sprintf(errstr, dps_err_ps_text, arg1str, arg2str);

	    if (resyncFlg) { /* && (ctx != dummyCtx)) { */
/*		RAISE(dps_err_ps, ctx); */
/*		CantHappen(); */
	    }
	    break;
	}
	case dps_err_nameTooLong:
	    sprintf(errstr, dps_err_nameTooLong_text, (char *) arg1);
	    break;
	case dps_err_invalidContext:
	    sprintf(errstr, dps_err_invalidContext_text, arg1);
	    break;
	case dps_err_resultTagCheck:
	    sprintf(errstr, dps_err_resultTagCheck_text,
		*((unsigned char *) arg1 + 1));
	    break;
	case dps_err_resultTypeCheck:
	    sprintf(errstr, dps_err_resultTypeCheck_text,
		*((unsigned char *) arg1 + 1));
	    break;
	case dps_err_invalidAccess:
	    sprintf(errstr, dps_err_invalidAccess_text);
	    break;
	case dps_err_encodingCheck:
	    sprintf(errstr, dps_err_encodingCheck_text,(int)arg1,(int)arg2);
	    break;
	case dps_err_closedDisplay:
	    sprintf(errstr, dps_err_closedDisplay_text, (int)arg1);
	    break;
	case dps_err_deadContext:
	    sprintf(errstr, dps_err_deadContext_text, (int)arg1);
	    break;
	default:
	    sprintf(errstr, dps_err_unknownError_text, errorCode);
	    break;
    }

    if ((textProc != NULL) && (errstr[0] != '\0'))
	(*textProc)(ctx, errstr, strlen(errstr));

    DPSResetContext(ctx);
    DPSPrintf(ctx, "clientsync\n");
}





int cupdps_figurefile_info(f, dpy, scrnum, 
	llx_pts_ret, lly_pts_ret, urx_pts_ret, ury_pts_ret,
	width_pix_ret, height_pix_ret,
	info_flags_ret, info_structvers_ret, info_epsfvers_ret)

/* This routine is used to gather important information about the */
/* PostScript figure in a file.  It returns the bounding box of the figure */
/* if there is one (in points).  If not, it returns a bounding box the */
/* size of an 8.5 x 11 inch page.  It also returns width and height (in */
/* pixels) of the figure.  The info_flag has certain bits set to convey */
/* information about the PostScript file. */
/* */
/* CUPDPS_PS_FILE is set if the file identifies itself as a PostScript file. */
/*	(ie. starts with "%!")
/* CUPDPS_PS_STRUCT is set if the file claims to follow the Adobe Structuring */
/*	Conventions.  (ie. starts with "%!PS-Adobe-")   If this flag is set, */
/*	the value of *info_structvers_ret is set to the verion number which */
/*	the file claims to adhere to. */
/* CUPDPS_PS_EPSF is set is the file claims to be an Encapsulated PostScript *.
/*	file. (ie. starts with "PS-Adobe-n.n EPSF-n.n")  If this flag is set, */
/*	the value of *info_epsfvers_ret is set to the EPSF conventions verion */
/*	number which the file claims to adhere to. */
/* CUPDPS_PS_BBOX is set if a %%BoundingBox: comment is actually found in the */
/*	file. */
/* CUPDPS_PS_BBOXVALS is set if the numerical parameters to the */
/*	%%BoundingBox: comment are found. */
/* */
/* If there are problems this routine returns FALSE.  If successful it */
/* returns TRUE. */

    FILE	*f;
    Display	*dpy;
    int		scrnum;
    int		*llx_pts_ret, *lly_pts_ret, *urx_pts_ret, *ury_pts_ret;
    int		*width_pix_ret, *height_pix_ret;
    unsigned	*info_flags_ret;
    float	*info_structvers_ret, *info_epsfvers_ret;

{
    int	 still_cmts, goodparams;
    float  llxf,llyf,urxf,uryf;
    int    xdpi,ydpi;
    char   buf[PSBITESIZE];
    char   *i;
    char   keyword[256];
    long   fpos;

    still_cmts = TRUE;
    *llx_pts_ret = BBOXLLX_DEF;
    *lly_pts_ret = BBOXLLY_DEF;
    *urx_pts_ret = BBOXURX_DEF;
    *ury_pts_ret = BBOXURY_DEF;
    *info_flags_ret = 0;
    *info_structvers_ret = 0.0;
    *info_epsfvers_ret = 0.0;

    /* Get the first line of the PostScript file and examine it against */
    /* the PostScript structuring conventions */

    fpos = ftell(f);

    fgets(buf, PSBITESIZE, f);

    if (strncmp(buf,"%!",2) == 0) {
	*info_flags_ret |= CUPDPS_PS_FILE;
    }

    if (strncmp(buf,"%!PS-Adobe-",11) == 0) {
	*info_flags_ret |= CUPDPS_PS_STRUCT;
	sscanf(buf+11, "%f", info_structvers_ret);

	sscanf(buf,"%*s %s",keyword);
	if (strncmp(keyword,"EPSF",4) == 0) {
	    *info_flags_ret |= CUPDPS_PS_EPSF;
	    sscanf(keyword+5,"%f",info_epsfvers_ret);
	}
    }


    /* While in comment section, look for %%BoundingBox comment */
    do {
	fgets(buf, PSBITESIZE, f);
	if (buf[0] != '%') still_cmts = FALSE;
    } while ((strncmp(buf, bboxstr, strlen(bboxstr)) != 0) && still_cmts);

    /* If found %%BoundingBox comment in comment section */
    /* then calculate appropriate window size recommendation */
    if (still_cmts) {
	*info_flags_ret |= CUPDPS_PS_BBOX;
	goodparams = TRUE;
	for (i = buf+strlen(bboxstr); i < buf+strlen(buf); i++) {
	    if (!isdigit(*i) && !isspace(*i) && (*i != '.') && (*i != '-')) {
		goodparams = FALSE;
		break;
	    }
	}
	if (goodparams) {
	    *info_flags_ret |= CUPDPS_PS_BBOXVALS;
	    sscanf(buf+strlen(bboxstr), "%f%f%f%f", &llxf,&llyf,&urxf,&uryf);
	    *llx_pts_ret = (int) llxf;
	    *lly_pts_ret = (int) llyf;
	    *urx_pts_ret = (int)(urxf + 0.5);
	    *ury_pts_ret = (int)(uryf + 0.5);
	}
    }

    xdpi = (int) (((DisplayWidth(dpy,scrnum) * 25.4)
			/ DisplayWidthMM(dpy,scrnum)) + 0.50);
    ydpi = (int) (((DisplayHeight(dpy,scrnum) * 25.4)
			/ DisplayHeightMM(dpy,scrnum)) + 0.50);

    *width_pix_ret = (*urx_pts_ret - *llx_pts_ret) * xdpi / 72;
    *height_pix_ret = (*ury_pts_ret - *lly_pts_ret) * ydpi / 72;

    /* Restore file ptr to original position */
    fseek(f, fpos, 0);

    return(TRUE);

}




int cupdps_figurebuf_info(buf, buflen, dpy, scrnum, 
	llx_pts_ret, lly_pts_ret, urx_pts_ret, ury_pts_ret,
	width_pix_ret, height_pix_ret,
	info_flags_ret, info_structvers_ret, info_epsfvers_ret)

/* This routine is used to gather important information about the */
/* PostScript figure in a buffer.  It returns the bounding box of the figure */
/* if there is one.  If not, it returns a bounding box the size of an */
/* 8.5 x 11 inch page.  It also returns width and height */
/* recommendations (in pixels) to the caller as a hint about how large a */
/* window is necessary to display the figure in it's entirety.  The info_flag
/* has certain bits set to convey information about the PostScript file. */
/* */
/* CUPDPS_PS_FILE is set if the file identifies itself as a PostScript file. */
/*	(ie. starts with "%!")
/* CUPDPS_PS_STRUCT is set if the file claims to follow the Adobe Structuring */
/*	Conventions.  (ie. starts with "%!PS-Adobe-")   If this flag is set, */
/*	the value of *info_structvers_ret is set to the verion number which */
/*	the file claims to adhere to. */
/* CUPDPS_PS_EPSF is set is the file claims to be an Encapsulated PostScript *.
/*	file. (ie. starts with "PS-Adobe-n.n EPSF-n.n")  If this flag is set, */
/*	the value of *info_epsfvers_ret is set to the EPSF conventions verion */
/*	number which the file claims to adhere to. */
/* CUPDPS_PS_BBOX is set if a %%BoundingBox: comment is actually found in the */
/*	file. */
/* CUPDPS_PS_BBOXVALS is set if the numerical parameters to the */
/*	%%BoundingBox: comment are found. */
/* */
/* If there are problems this routine returns FALSE.  If successful it */
/* returns TRUE. */

    char	*buf;
    int		buflen;
    Display	*dpy;
    int		scrnum;
    int		*llx_pts_ret, *lly_pts_ret, *urx_pts_ret, *ury_pts_ret;
    int		*width_pix_ret, *height_pix_ret;
    unsigned	*info_flags_ret;
    float	*info_structvers_ret, *info_epsfvers_ret;

{
    int	 still_cmts, goodparams;
    float  llxf,llyf,urxf,uryf;
    int    xdpi,ydpi;
    char   *i, *bufptr;
    char   keyword[256];

    still_cmts = TRUE;
    *llx_pts_ret = BBOXLLX_DEF;
    *lly_pts_ret = BBOXLLY_DEF;
    *urx_pts_ret = BBOXURX_DEF;
    *ury_pts_ret = BBOXURY_DEF;
    *info_flags_ret = 0;
    *info_structvers_ret = 0.0;
    *info_epsfvers_ret = 0.0;

    /* Get the first line of the PostScript file and examine it against */
    /* the PostScript structuring conventions */

    if (strncmp(buf,"%!",2) == 0) {
	*info_flags_ret |= CUPDPS_PS_FILE;
    }

    if (strncmp(buf,"%!PS-Adobe-",11) == 0) {
	*info_flags_ret |= CUPDPS_PS_STRUCT;
	sscanf(buf+11, "%f", info_structvers_ret);

	sscanf(buf,"%*s %s",keyword);
	if (strncmp(keyword,"EPSF",4) == 0) {
	    *info_flags_ret |= CUPDPS_PS_EPSF;
	    sscanf(keyword+5,"%f",info_epsfvers_ret);
	}
    }


    /* While in comment section, look for %%BoundingBox comment */
    bufptr = buf;
    do {
	bufptr = strchr(bufptr, '\n');	/* Find the end of a line */
	bufptr++;			/* Move to first char of next line */
	if (*bufptr != '%') still_cmts = FALSE;
    } while ((strncmp(bufptr, bboxstr, strlen(bboxstr)) != 0) && still_cmts);

    /* If found %%BoundingBox comment in comment section */
    /* then calculate appropriate window size recommendation */
    if (still_cmts) {
	*info_flags_ret |= CUPDPS_PS_BBOX;
	goodparams = TRUE;
	for (i = bufptr+strlen(bboxstr); (*i != '\n')&&(i < buf+buflen); i++) {
	    if (!isdigit(*i) && !isspace(*i) && (*i != '.')  && (*i != '-')) {
		goodparams = FALSE;
		break;
	    }
	}
	if (goodparams) {
	    *info_flags_ret |= CUPDPS_PS_BBOXVALS;
	    sscanf(bufptr+strlen(bboxstr), "%f%f%f%f", &llxf,&llyf,&urxf,&uryf);
	    *llx_pts_ret = (int) llxf;
	    *lly_pts_ret = (int) llyf;
	    *urx_pts_ret = (int)(urxf + 0.5);
	    *ury_pts_ret = (int)(uryf + 0.5);
	}
    }

    xdpi = (int) (((DisplayWidth(dpy,scrnum) * 25.4)
			/ DisplayWidthMM(dpy,scrnum)) + 0.50);
    ydpi = (int) (((DisplayHeight(dpy,scrnum) * 25.4)
			/ DisplayHeightMM(dpy,scrnum)) + 0.50);

    *width_pix_ret = (*urx_pts_ret - *llx_pts_ret) * xdpi / 72;
    *height_pix_ret = (*ury_pts_ret - *lly_pts_ret) * ydpi / 72;

    return(TRUE);

}




DPSContext cupdps_alloc_ctx(dpy, scrnum, drawable, gc,
	xoff, yoff, xclip, yclip, wclip, hclip, dpsTextProc, dpsErrorProc,
	cmap, blackPixel, whitePixel, drawMono)

/* This routine initializes a Display PostScript "context" and allocates */
/* virtual space for the context to work in.  A context can be thought of */
/* as a single independent PostScript interpreter.  You can have more than */
/* one context and they can be in use simultaneously.  You must allocate */
/* a context with this routine before attempting to use it. */
/* This routine returns a DPS context if successful, NULL otherwise. */

    Display	*dpy;		/* ptr to DEcw display structure */
    int		scrnum;		/* screen number */
    Drawable	drawable;	/* id of window or pixmap */
    GC		gc;		
    int		xoff, yoff;	/* x,y offset from drawable origin */
    int		xclip, yclip, wclip, hclip; /* clip rect from drawable origin */
    DPSTextProc	dpsTextProc;	/* Routine to handle text output from DPS */
				/* Use "DPSDefaultTextBackstop" to simply */
				/* write output text to stdout */
    DPSErrorProc dpsErrorProc;  /* Routine to handle DPS errors */
				/* Use "cupdps_error_proc" unless you have */
				/* your own error handler */
    Colormap	cmap;		/* Colormap of the drawable */
    unsigned long  blackPixel, whitePixel; /* Pixels must already be properly */
					   /* allocated by caller */
    int		drawMono;	/* if TRUE then renders PS in monochrome */

{

    DPSContext		ctx;
    XRectangle		rect[1];
    XColor		*color;
    XStandardColormap	grayRamp;

    /* Set up clipping rectangle */

    rect[0].x = 0;
    rect[0].y = 0;
    rect[0].width = wclip;
    rect[0].height = hclip;

/* This is messing up the expose clipping rectangle.  Clipping will have to */
/* be handled in PostScript for this reason and also because X can't clip */
/* to a non-orthogonal rectangle. */
/*    XSetClipRectangles(dpy, gc, xclip, yclip, rect, 1, Unsorted); */

    /* Create the context. */
    /* There seems to be a design flaw in X windows, particularly */
    /* with the XStandardColorMap structure which requires that the */
    /* darkest color (the base_pixel) have the lowest pixel value in */
    /* the ramp.  There's no way to describe a ramp where the */
    /* base_pixel is not the lowest because all the _max and _mult */
    /* values in the XStandardColorMap structure are UNSIGNED. */

    ctx = NULL;
    if (drawMono && (blackPixel < whitePixel)) {
	grayRamp.colormap = cmap;
	grayRamp.red_max = 1;
	grayRamp.green_max = 0;
	grayRamp.blue_max = 0;

	grayRamp.base_pixel = blackPixel;

	grayRamp.red_mult = whitePixel - blackPixel;
	grayRamp.green_mult = grayRamp.blue_mult = 0;

	ctx = XDPSCreateContext(dpy, drawable, gc, xoff, yoff,
		0, &grayRamp, NULL, 0,
 		dpsTextProc, dpsErrorProc, NULL);
    } else {
	ctx = XDPSCreateSimpleContext(dpy, drawable, gc, xoff, yoff,
		dpsTextProc, dpsErrorProc, NULL);
    }

    if (ctx == NULL) {
	/* Could not create context */
	return(NULL);
    }

    return(ctx);
}





int  cupdps_get_status(ctx, printit)

/* This routine returns the current execution status of a context */
/* and to optionally prints out the status to the stdout. */

    DPSContext	ctx;
    int		printit;
{
    int		status;
    static int	laststatus = -1;

    switch (status = XDPSGetContextStatus(ctx)) {
	case PSRUNNING:
	    if (status != laststatus)
		if (printit)
			printf("CUPDPS Status:\t===== Running\n");
	    break;
	case PSNEEDSINPUT:
	    if (status != laststatus)
		if (printit)
			printf("CUPDPS Status:\t===== Needs Input\n");
	    break;
	case PSFROZEN:
	    if (status != laststatus)
		if (printit)
			printf("CUPDPS Status:\t===== FROZEN\n");
	    break;
	case PSZOMBIE:
	    if (status != laststatus)
		if (printit)
			printf("CUPDPS Status:\t===== ZOMBIE\n");
	    break;
	case PSSTATUSERROR:
	    if (status != laststatus)
		if (printit)
			printf("CUPDPS Status:\t===== STATUSERROR\n");
	    break;
    }
    laststatus = status;

    return(status);
}




void  cupdps_draw_ps_buf(dpy, scrnum, ctx, appinitbuf, appinitbuflen,
			 psbuf, psbuflen)

/* Use this routine to draw PostScript residing in a buffer to a context. */
/* This routine will image the first page of PostScript residing in the */
/* buffer. */

    Display	*dpy;	
    int		scrnum;
    DPSContext	ctx;
    char	*appinitbuf;	/* Buf containing appl. specific init string */
    int		appinitbuflen;
    char	*psbuf;		/* Buffer containing PS data */
    int		psbuflen;
{
    int    llx,lly,urx,ury;
    int    width, height;
    unsigned  info_flags;
    float  struct_vers, epsf_vers;


    /* Feed the context the set-up commands */
    DPSWritePostScript(ctx,initialCommand, strlen(initialCommand));

    /* Feed the application-specific initialization PostScript string to */
    /* the context. */
    if (appinitbuflen > 0)
	DPSWritePostScript(ctx, appinitbuf, appinitbuflen);

    /* Feed the PostScript to the context. Only one page drawings will be */
    /* displayed.  If the PostScript file contains multiple pages, only */
    /* the first will be displayed. */
    if (psbuflen > 0)
	DPSWritePostScript(ctx, psbuf, psbuflen);

    /* Tell the context that we are done drawing */
    DPSPrintf(ctx, "clientsync\n");

}



void  cupdps_draw_ps_file(dpy, scrnum, ctx, appinitbuf, appinitbuflen, f)

/* Use this routine to draw a PostScript file to a context.  This routine */
/* will image the first page of PostScript file. */

    Display	*dpy;	
    int		scrnum;
    DPSContext	ctx;
    char	*appinitbuf;	/* Buf containing appl. specific init string */
    int		appinitbuflen;
    FILE	*f;
{
    int    llx,lly,urx,ury;
    int    width, height;
    unsigned  info_flags;
    float  struct_vers, epsf_vers;
    char   psbuf[PSBITESIZE];
    long   fpos;

    fpos = ftell(f);

    /* Get bounding box info on the figure */
    cupdps_figurefile_info(f, dpy, scrnum, &llx, &lly, &urx, &ury,
	&width, &height, &info_flags, &struct_vers, &epsf_vers);

    /* Feed the context the set-up commands */
    DPSWritePostScript(ctx,initialCommand, strlen(initialCommand));

    /* Translate the lower left of the Bounding Box to the PostScript origin */
    sprintf(EPScommand,"%d %d translate\n", -llx,-lly);
    DPSWritePostScript(ctx,EPScommand, strlen(EPScommand));


    /* Feed the application-specific initialization PostScript string to the */
    /* the context. */
    if (appinitbuflen > 0)
	DPSWritePostScript(ctx, appinitbuf, appinitbuflen);

    /* Feed the PostScript to the context. Only one page drawings will be */
    /* displayed.  If the PostScript file contains multiple pages, only */
    /* the first will be displayed. */
    while (!feof(f)) {
	fgets(psbuf, PSBITESIZE, f);
	DPSWritePostScript(ctx, psbuf, strlen(psbuf));
    }

    /* Restore file ptr to original position */
    fseek(f, fpos, 0);

    /* Tell the context that we are done drawing */
    DPSPrintf(ctx, "clientsync\n");

}



int  cupdps_wait_finish_drawing(ctx)

/* This is a synchronization routine which will return when the context */
/* has finished drawing or an error occurs.  It returns the last status of */
/* the context */


    DPSContext	ctx;
{
    int    ctxstatus;
    int    done;

    done = FALSE;
    do {
	ctxstatus = cupdps_get_status(ctx, DEBUG_FLAG);
	switch (ctxstatus) {
	    case PSSTATUSERROR:
		/* Error has occurred and there's nothing we can do about it */
		done = TRUE;
		break;
	    case PSNEEDSINPUT:
		/* We've reached end of input file but it still wants more */
		/* Something must be wrong */
		DPSResetContext(ctx);
		DPSPrintf(ctx, "clientsync\n");
		cupdps_wait_finish_drawing(ctx);
		done = TRUE;
		break;
	    case PSZOMBIE:
		/* The context is dead.  There's nothing we can do about it */
		done = TRUE;
		break;
	    case PSFROZEN:
		/* It's done drawing the page */
		done = TRUE;
		break;
	    case PSRUNNING:
		/* Let it keep going until it's done */
		break;
	}
    } while (!done);
    return (ctxstatus);
}




void  cupdps_free_ctx(ctx)

    DPSContext	ctx;

/* Use this routine to destroy a context.  It will destroy the context's */
/* space as well which will also destroy all contexts associated with that */
/* space with that space */

{
    DPSSpace	space;

    space = DPSSpaceFromContext(ctx);
    DPSDestroySpace(space);
}




int  sample_main() /* (argc,argv) */

/* Here is a sample main program.  */

/*    int		argc; */
/*    char	**argv; */
{

    int		argc;
    char	**argv;

    char *displayname = "";
    Display *dpy;
    int i;
    DPSContext	ctx, txtctx;
    long mask;
    Window window;
    GC	gc;
    XSetWindowAttributes xswa;
    int scrnum;
    char junk[255];
    FILE *f;
    int    llx,lly,urx,ury;
    int    width, height;
    int    sync = 0;
    int    showstatus = 0;
    int    ctxstatus;
    int    echo = 0;
    static char   *filename = NULL;
    int    done;
    unsigned  info_flags;
    float  struct_vers, epsf_vers;
    int    statuschkcnt;
    char   *psbuf;
    int    psbuflen;


    /* Parse the command line */
    for (i=1 ; i<argc ; i++) {
	if (strncmp(argv[i], "-display", strlen(argv[i])) == 0) {
	    i++;
	    displayname = argv[i];
	} else if (strncmp(argv[i], "-sync", strlen(argv[i])) == 0) {
	    sync = 1;
	} else if (strncmp(argv[i], "-status", strlen(argv[i])) == 0) {
	    showstatus = 1;
	} else if (strncmp(argv[i], "-echo", strlen(argv[i])) == 0) {
	    echo = 1;
	} else {
	    filename = argv[i];
	}
    }

    if (filename == NULL) {
	fprintf(stderr, "Usage: %s [-display] [-sync] [-status] [-echo]  file\n", argv[0]);
	exit(1);
    }


    /* Open the display */
    dpy = XOpenDisplay(displayname);
    if (dpy == NULL) {
	fprintf(stderr, "%s: Can't open display \"%s\"!\n", argv[0], displayname);
	exit(1);
    }
    scrnum = XDefaultScreen(dpy);

    if (sync) XSynchronize(dpy, True);


    if (!(f = fopen(filename, "r"))) {
	fprintf(stderr, "%s: Can't open input file \"%s\"!\n",
		argv[0], filename);
	exit(1);
    }

#if 0
    /* Read the file into a buffer to duplicate the situation of the */
    /* bookreader */
    /* Read the file once to see how many chars are in it, alloc the */
    /* required memory, then read it for real */


    /* This was put in a separate module because it is painfully slow in */
    /* the debugger */
    psbuflen = charsinfile(f); 

    rewind(f);


    /* Allocate the memory for the buffer */
    psbuf = (char *) malloc(psbuflen);
    if (psbuf == NULL) {
	fprintf(stderr, "Error: Could not allocate %d byte buffer to hold PostScript file!\n");
	exit(1);
    }

    /* Read the file into the buffer */
    fread(psbuf, sizeof(char), psbuflen, f);
    fclose(f);


    /* First get sizing info on the figure */
    cupdps_figurebuf_info(psbuf, psbuflen, dpy, scrnum, &llx, &lly, &urx, &ury,
	&width, &height, &info_flags, &struct_vers, &epsf_vers);

#else

    cupdps_figurefile_info(f, dpy, scrnum, &llx, &lly, &urx, &ury,
	&width, &height, &info_flags, &struct_vers, &epsf_vers);
#endif



    /* If desired, print out info from the PS file header */
    if (showstatus) {
	printf("PostScript file =");
	if (CUPDPS_PS_FILE & info_flags) printf("T\n"); else printf("F\n");

	printf("Structured file =");
	if (CUPDPS_PS_STRUCT & info_flags)
	    printf("T, version = %1.1f\n", struct_vers);
	else
	    printf("F\n");

	printf("EPS file =");
	if (CUPDPS_PS_EPSF & info_flags)
	    printf("T, version = %1.1f\n", epsf_vers);
	else
	    printf("F\n");
    }


    /* Set up the display window */
    xswa.background_pixel = XWhitePixel(dpy,scrnum);
    xswa.border_pixel = XWhitePixel(dpy,scrnum);

    xswa.colormap = XDefaultColormap(dpy,scrnum);
    window = XCreateWindow(dpy, DefaultRootWindow(dpy),
	100,100,width,height, 0, 
	XDefaultDepth(dpy,scrnum), InputOutput, XDefaultVisual(dpy,scrnum),
	CWBorderPixel|CWColormap|CWBackPixel,&xswa);

    xswa.bit_gravity = SouthWestGravity;
    xswa.backing_store = WhenMapped;
    mask = CWBitGravity | CWBackingStore;
    XChangeWindowAttributes(dpy, window, mask, &xswa);
	
    XMapWindow(dpy, window);

    gc = XCreateGC(dpy, XRootWindow(dpy,scrnum), 0, NULL);
    XSetForeground(dpy, gc, XBlackPixel(dpy,scrnum));
    XSetBackground(dpy, gc, XWhitePixel(dpy,scrnum));

    /* Check to see if DPS is on the server */
    if (!cupdps_check_dps(dpy)) {
	fprintf(stderr, "XDPS not present on server %s\n",displayname);
	exit(1);
    }


    /* Allocate a DPS context */
    if (!(ctx = cupdps_alloc_ctx(dpy, scrnum, window, gc, 0, 0,
	0, 0, width, height, DPSDefaultTextBackstop, cupdps_error_proc))) {
	printf("Error initializing XDPS\n");
	exit(1);
    }

    /* Have the PostScript text echoed to the screen if the user wanted it */
    if (echo) {
	txtctx = DPSCreateTextContext(DPSDefaultTextBackstop,
		cupdps_error_proc);
/*		DPSDefaultErrorProc);*/

	DPSChainContext(ctx,txtctx);
    }

	
    /* Feed the PostScript to the context. */
/*    cupdps_draw_ps_buf(dpy, scrnum, ctx, psbuf, psbuflen);*/
    cupdps_draw_ps_file(dpy, scrnum, ctx, f);


    if (showstatus)  printf("---EOF---\n");

    /* Wait until the server is done drawing the picture */
    cupdps_wait_finish_drawing(ctx);

    printf("Press RETURN to continue\n");
    gets(junk);

    /* Free the context and its resources */
    cupdps_free_ctx(ctx);

    XCloseDisplay(dpy);

    if (showstatus)  printf("---Exit---\n");

}
