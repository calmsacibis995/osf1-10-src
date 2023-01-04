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

/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_DPS.C*/
/* *8    24-FEB-1993 17:47:19 BALLENGER "Fixes for large topic and Region memory leak."*/
/* *7    19-JUN-1992 20:19:25 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *6     8-JUN-1992 19:06:21 BALLENGER "UCX$CONVERT"*/
/* *5     8-JUN-1992 12:47:38 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *4     3-MAR-1992 16:58:20 KARDON "UCXed"*/
/* *3    13-NOV-1991 14:50:28 GOSSELIN "alpha checkins"*/
/* *2    17-SEP-1991 19:42:14 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:10 PARMENTER "Display PostScript"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_DPS.C*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_DPS.C*/
/* *7    19-JUN-1992 20:19:25 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *6     8-JUN-1992 19:06:21 BALLENGER "UCX$CONVERT"*/
/* *5     8-JUN-1992 12:47:38 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *4     3-MAR-1992 16:58:20 KARDON "UCXed"*/
/* *3    13-NOV-1991 14:50:28 GOSSELIN "alpha checkins"*/
/* *2    17-SEP-1991 19:42:14 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:10 PARMENTER "Display PostScript"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_DPS.C*/
#ifndef VMS
   /*
#else
#  module BKR_DPS "V03-0000"
#endif
#ifndef VMS
   */
#endif

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
**++
**  FACILITY:
**
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Display routines for X Display PostScript.
**
**      These routines display only single page PostScript drawings.  
**	If the PostScript data actually contains more
**      than one page, only the first page will be displayed.
**
**  AUTHORS:
**
**      Joseph M. Kraetsch
**
**  CREATION DATE:     19-Jul-1990
**
**  MODIFICATION HISTORY:
**
**      V03-0001    DLB0001     David L Ballenger       07-Feb-1991
**                  Pass correct number of arguments to bkr_error_modal()
**                  and get error message text from uid file
**
**	V03-0000    JMK0000	Joseph M. Kraetsch	19-Jul-1990
**  	    	    New module.
**
**--
**/


/*
 * INCLUDE FILES
 */

#include   <X11/Xlib.h>
#include   <string.h>
#include   <ctype.h>
#include   <assert.h>
#ifdef VMS
#include   "dpsXclient.h"
#else
#include   <DPS/dpsXclient.h>
#endif

#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_dps.h"         /* function prototypes */
#include "bkr_error.h"       /* Error routines */
#include "bkr_fetch.h"       /* Resource fetching routines */


#define    bkr_screen	    XDefaultScreen( bkr_display )
#define    root_window	    XDefaultRootWindow( bkr_display )


#define    BBOXLLX_DEF     0
#define    BBOXLLY_DEF     0
#define    BBOXURX_DEF     612
#define    BBOXURY_DEF     792

#define    DEBUG_FLAG	    FALSE

#define    PS_INITIAL_CMD  "resyncstart /cupdps_showpage /showpage load def \n\
			    /showpage {clientsync} def\n"
#define    PS_BBOX_STR	    "%%BoundingBox:"

/* Flags: */
/* DPS_PS_FILE is set if the file identifies itself as a PostScript file. */
/*	(ie. starts with "%!")
/* DPS_PS_STRUCT is set if the file claims to follow the Adobe Structuring */
/*	Conventions.  (ie. starts with "%!PS-Adobe-")   If this flag is set, */
/*	the value of *info_structvers_ret is set to the verion number which */
/*	the file claims to adhere to. */
/* DPS_PS_EPSF is set is the file claims to be an Encapsulated PostScript *.
/*	file. (ie. starts with "PS-Adobe-n.n EPSF-n.n")  If this flag is set, */
/*	the value of *info_epsfvers_ret is set to the EPSF conventions verion */
/*	number which the file claims to adhere to. */
/* DPS_PS_BBOX is set if a %%BoundingBox: comment is actually found in the */
/*	file. */
/* DPS_PS_BBOXVALS is set if the numerical parameters to the */
/*	%%BoundingBox: comment are found. */

#define	    DPS_PS_FILE	    0x01
#define	    DPS_PS_STRUCT   0x02
#define	    DPS_PS_EPSF	    0x04
#define	    DPS_PS_BBOX	    0x08
#define	    DPS_PS_BBOXVALS 0x10

/*********** put error messages into UIL **********/

/* Errors:  [NOTE--these should go into UIL  */
#define dps_err_unknownError_text "Unrecognized error code %d\n"
/* Arguments:  errorcode (integer ) */

#define dps_err_ps_text "PostScript language error in figure\n\tError: %s\n\tOffending Command: %s\n"
/* Arguments: errorName, erroneousCommand */

#define dps_err_nameTooLong_text "PostScript language user name too long\n\tName: %s\n"
/* Arguments: badName */

#define dps_err_invalidContext_text "Invalid Display PostScript context\n\tContext Id: %d\n"
/* Arguments: badContextId (integer) */

#define dps_err_resultTagCheck_text "Erroneous Display PostScript wrap result tag\n\tTag: %s\n"
/* Arguments: badTag */

#define dps_err_resultTypeCheck_text "Incompatable Display PostScript wrap result type\n\tType: %s\n"
/* Arguments: badType */

#define dps_err_invalidAccess_text "Invalid Display PostScript context access\n"
/* Arguments:  <none> */

#define dps_err_encodingCheck_text "Invalid Display PostScript name/program encoding\n\t%d/%d\n"
/* Arguments:  arg1, arg2 (both integers) */

#define dps_err_closedDisplay_text "Broken display connection %d\n"
/* Arguments:  arg1 (integer) */

#define dps_err_deadContext_text "Dead context 0x0%x\n"
/* Arguments:  arg1 (hex integer ) */


/*
 * FORWARD ROUTINES
 */
static void		    dps_error_proc();
static int		    dps_figure_info();
static DPSContext	    dps_alloc_context();
static int		    dps_get_status();
static void		    dps_draw_postscript();
static int		    dps_wait_finish_drawing();



/*
 * FORWARD DEFINITIONS 
 */

static BR_HANDLE 	    dps_init = NULL;
static GC                   dps_gc = NULL;    /*  Private GC for dps  */

static char		    *initialCommand = PS_INITIAL_CMD;
static char		    EPScommand[256] = "";
static char		    *bboxstr = PS_BBOX_STR;


void
bkr_dps_init( VOID_PARAM )
{
    int             major_opcode_ret;
    int             first_event_ret;
    int             first_error_ret;
    unsigned int    value_mask;
    XGCValues       xgcvalues;

    if ( dps_init != NULL )
	return;

    if ( ! XQueryExtension( bkr_display, "DPSExtension", &major_opcode_ret,
	&first_event_ret, &first_error_ret ) )
    {
	dps_init = NODISPLAY;
	return;
    }
	
    /*  create a private gc for dps use  */	

    xgcvalues.foreground = bkr_window_foreground;
    xgcvalues.background = bkr_window_background;
    value_mask = GCForeground | GCBackground;
    dps_gc = (GC) XCreateGC( bkr_display, root_window, value_mask, &xgcvalues );

    dps_init = (BR_HANDLE)1;
    return;
};   /*  end of bkr_dps_init  */


void
bkr_dps_exit(VOID_PARAM)
{
    if ( ( dps_init == NULL ) || ( dps_init == NODISPLAY ) )
	return;

    /*  free any dps resources  */

    XFreeGC ( bkr_display, dps_gc );
    dps_gc = NULL;
    dps_init = NULL;
    return;

};   /*  end of bkr_dps_exit  */


BR_HANDLE
bkr_dps_display PARAM_NAMES((window,chunk,xoff,yoff,xclip,yclip,wclip,hclip))
    Window		window PARAM_SEP
    BMD_CHUNK		*chunk PARAM_SEP
    int 		xoff PARAM_SEP	/*  x offset of origin in window    */
    int 		yoff PARAM_SEP	/*  y offset of origin in window    */
    int 		xclip PARAM_SEP	/*  x value of clip rectangle	    */
    int 		yclip PARAM_SEP	/*  y value of clip rectangle	    */
    int 		wclip PARAM_SEP	/*  width of clip rectangle	    */
    int 		hclip PARAM_END	/*  height of clip rectangle	    */
					/*  NOTE: clip rect. is wrt window  */
					/*  origin, NOT wrt (xoff, yoff)    */
{
    unsigned		handle;		/*  dps display handle	    */
    DPSContext		context;	/*  PostScript context   */

    if ( dps_init == 0 )		/*  first call for dps  */
	bkr_dps_init();

    if ( dps_init == NODISPLAY		/*  dps not available  */
	|| chunk->handle == NODISPLAY )	/*  couldn't display chunk last time */
	return NODISPLAY;

    if ( (DPSContext) chunk->handle != NULL )
	context = (DPSContext) chunk->handle;
    else
	context = dps_alloc_context( bkr_display, bkr_screen, window, dps_gc,
	    xoff, yoff + chunk->rect.height, /*  PS origin is lower left  */
	    xclip, yclip, wclip, hclip, NULL, dps_error_proc );

    if ( context == NULL )
    {
        /*  put up a message and flag chunk as failed  */
        
        char	*error_string;
        error_string = (char *)bkr_fetch_literal("POSTSCRIPT_DISPLAY_ERROR",ASCIZ );
        if ( error_string != NULL )
        {
            bkr_error_modal( errmsg, NULL );
            XtFree( error_string );
        }
        return NODISPLAY;
    }

    if ( context == NULL )
	return NODISPLAY;

    /*  display the PostScript  */

    dps_draw_postscript(
	bkr_display, bkr_screen, context, chunk->data_addr, chunk->data_len);

    /*  wait until the server is done drawing  */

    dps_wait_finish_drawing( context );

    return NULL;

};   /*  end of bkr_dps_display  */


void
bkr_dps_close PARAM_NAMES((handle))
    BR_HANDLE		handle PARAM_END	/*  dps display handle	*/

{
    DPSSpace		space;

    if ( ( dps_init == NULL ) || ( dps_init == NODISPLAY )
	|| ( handle == NULL ) || ( handle == NODISPLAY ) )
	return;

    /*  Destroy the context's space which will also destroy all contexts 
     *  associated with that space  */

    space = DPSSpaceFromContext( (DPSContext) handle );
    DPSDestroySpace( space );

    return;

};   /*  end of bkr_dps_close  */


static void 
dps_error_proc( context, errorCode, arg1, arg2 )

/* Based heavily on the example implementation error handler in Appendix B.1 */
/* of DPS Client Library Reference Manual. The only main difference between */
/* this procedure and DPSDefaultErrorProc is that this procedure does not */
/* raise exceptions on PostScript language errors and that the error */
/* presentation can be customized. */

    DPSContext		context;
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

            /* don't really need these and the ALpha compiler pukes on it */
	    /* assert((ary->attributedType & 0x7f) == DPS_ARRAY); */
	    /* assert(ary->length == 4); */

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

	    if (resyncFlg) { /* && (context != dummycontext)) {*/
/*		RAISE(dps_err_ps, context);*/
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
	(*textProc)(context, errstr, strlen(errstr));

    DPSResetContext(context);
    DPSPrintf(context, "clientsync\n");

};  /* end of dps_error_proc */


static int
dps_figure_info( buf, buflen, dpy, scrnum, 
	llx_pts_ret, lly_pts_ret, urx_pts_ret, ury_pts_ret,
	width_pix_ret, height_pix_ret,
	info_flags_ret, info_structvers_ret, info_epsfvers_ret )

/* This routine is used to gather important information about the */
/* PostScript figure in a buffer.  It returns the bounding box of the figure */
/* if there is one.  If not, it returns a bounding box the size of an */
/* 8.5 x 11 inch page.  It also returns width and height */
/* recommendations (in pixels) to the caller as a hint about how large a */
/* window is necessary to display the figure in it's entirety.  The info_flag
/* has certain bits set to convey information about the PostScript file. */
/* */
/* DPS_PS_FILE is set if the file identifies itself as a PostScript file. */
/*	(ie. starts with "%!")
/* DPS_PS_STRUCT is set if the file claims to follow the Adobe Structuring */
/*	Conventions.  (ie. starts with "%!PS-Adobe-")   If this flag is set, */
/*	the value of *info_structvers_ret is set to the verion number which */
/*	the file claims to adhere to. */
/* DPS_PS_EPSF is set is the file claims to be an Encapsulated PostScript *.
/*	file. (ie. starts with "PS-Adobe-n.n EPSF-n.n")  If this flag is set, */
/*	the value of *info_epsfvers_ret is set to the EPSF conventions verion */
/*	number which the file claims to adhere to. */
/* DPS_PS_BBOX is set if a %%BoundingBox: comment is actually found in the */
/*	file. */
/* DPS_PS_BBOXVALS is set if the numerical parameters to the */
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
	*info_flags_ret |= DPS_PS_FILE;
    }

    if (strncmp(buf,"%!PS-Adobe-",11) == 0) {
	*info_flags_ret |= DPS_PS_STRUCT;
	sscanf(buf+11, "%f", info_structvers_ret);

	sscanf(buf,"%*s %s",keyword);
	if (strncmp(keyword,"EPSF",4) == 0) {
	    *info_flags_ret |= DPS_PS_EPSF;
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
	*info_flags_ret |= DPS_PS_BBOX;
	goodparams = TRUE;
	for (i = bufptr+strlen(bboxstr); (*i != '\n')&&(i < buf+buflen); i++) {
	    if (!isdigit(*i) && !isspace(*i) && (*i != '.')) {
		goodparams = FALSE;
		break;
	    }
	}
	if (goodparams) {
	    *info_flags_ret |= DPS_PS_BBOXVALS;
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

};  /* end of dps_figure_info */


static DPSContext 
dps_alloc_context( dpy, scrnum, drawable, gc,
	xoff, yoff, xclip, yclip, wclip, hclip, dpsTextProc, dpsErrorProc )

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
				/* this tis the PS origin--l.l. corner  */
    int		xclip, yclip, wclip, hclip; /* clip rect from drawable origin */
    DPSTextProc	dpsTextProc;	/* Routine to handle text output from DPS */
				/* Use "DPSDefaultTextBackstop" to simply */
				/* write output text to stdout */
    DPSErrorProc dpsErrorProc;  /* Routine to handle DPS errors */
				/* Use "dps_error_proc" unless you have */
				/* your own error handler */
{

    int		x, y;
    DPSContext	context;
    XRectangle	rect;

    /* Set up clipping rectangle */
    rect.x = xclip;
    rect.y = yclip;
    rect.width = wclip;
    rect.height = hclip;
    XSetClipRectangles(dpy, gc, 0, 0, &rect, 1, Unsorted);    

    /* Create the context with the origin in the lower left of the window */
    /* to simulate the way PostScript works on paper */

    context = XDPSCreateSimpleContext(dpy, drawable, gc, xoff, yoff,
	dpsTextProc, dpsErrorProc, NULL);

    if (context == NULL) {
	/* Could not create context */
	return(NULL);
    }

    return(context);

};  /* end of dps_alloc_context */



static int
dps_get_status( context, printit )

/* This routine returns the current execution status of a context */
/* and to optionally prints out the status to the stdout. */

    DPSContext	context;
    int		printit;
{
    int		status;
    static int	laststatus = -1;

    switch (status = XDPSGetContextStatus(context)) {
	case PSRUNNING:
	    if (status != laststatus)
		if (printit)
			printf("DPS Status:\t===== Running\n");
	    break;
	case PSNEEDSINPUT:
	    if (status != laststatus)
		if (printit)
			printf("DPS Status:\t===== Needs Input\n");
	    break;
	case PSFROZEN:
	    if (status != laststatus)
		if (printit)
			printf("DPS Status:\t===== FROZEN\n");
	    break;
	case PSZOMBIE:
	    if (status != laststatus)
		if (printit)
			printf("DPS Status:\t===== ZOMBIE\n");
	    break;
	case PSSTATUSERROR:
	    if (status != laststatus)
		if (printit)
			printf("DPS Status:\t===== STATUSERROR\n");
	    break;
    }
    laststatus = status;

    return(status);

};  /* end of dps_get_status */


static void  
dps_draw_postscript( dpy, scrnum, context, psbuf, psbuflen )

/* Use this routine to draw PostScript to a context.  This routine will image */
/* the first page of PostScript residing in the buffer */

    Display	*dpy;	
    int		scrnum;
    DPSContext	context;
    char	*psbuf;
    int		psbuflen;
{
    int    llx,lly,urx,ury;
    int    width, height;
    unsigned  info_flags;
    float  struct_vers, epsf_vers;


    /* Get bounding box info on the figure */
    dps_figure_info( psbuf, psbuflen, dpy, scrnum, &llx, &lly, &urx, &ury,
	&width, &height, &info_flags, &struct_vers, &epsf_vers);

    /* Feed the context the set-up commands */
    DPSWritePostScript(context,initialCommand, strlen(initialCommand));

    /* Translate the lower left of the Bounding Box to the PostScript origin */
    sprintf(EPScommand,"%d %d translate\n", -llx,-lly);
    DPSWritePostScript(context,EPScommand, strlen(EPScommand));

    /* Feed the PostScript to the context. Only one page drawings will be */
    /* displayed.  If the PostScript file contains multiple pages, only */
    /* the first will be displayed. */
    DPSWritePostScript(context, psbuf, psbuflen);

    /* Tell the context that we are done drawing */
    DPSPrintf(context, "clientsync\n");

};  /* end of dps_draw_postscript */


static int
dps_wait_finish_drawing( context )

/* This is a synchronization routine which will return when the context */
/* has finished drawing or an error occurs.  It returns the last status of */
/* the context */


    DPSContext	context;
{
    int    contextstatus;
    int    done;

    done = FALSE;
    do {
	contextstatus = dps_get_status(context, DEBUG_FLAG);
	switch (contextstatus) {
	    case PSSTATUSERROR:
		/* Error has occurred and there's nothing we can do about it */
		done = TRUE;
		break;
	    case PSNEEDSINPUT:
		/* We've reached end of input file but it still wants more */
		/* Something must be wrong */
		DPSResetContext(context);
		DPSPrintf(context, "clientsync\n");
		dps_wait_finish_drawing(context);
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

    return (contextstatus);

};  /* end of dps_wait_finish_drawing */

