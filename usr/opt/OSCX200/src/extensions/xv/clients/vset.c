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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/xv/clients/vset.c,v 1.1.2.2 92/02/07 16:44:07 Dave_Hill Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

/***********************************************************
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
  
                        All Rights Reserved
  
Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  
  
DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
  
******************************************************************/
/*
 * * File: *
 *
 *   vset.c: user preference utility for live video extension to X *
 *
 *   
 *
 * Author: *
 *
 *   Susan Angebranndt *
 *
 *       
 *
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xvlib.h>

typedef struct _Options {
    Bool            brightness;
    Bool            contrast;
    Bool            hue;
    Bool            saturation;
    Bool            query;
    Bool            encoding;
    int		    b;
    int		    c;
    int		    h;
    int		    s;
    char *	    e;
    char *	    displayName;
} Options;

static void Usage()
{
    printf("vset [-d display] [-b integer] [-c integer] [-h integer]\n");
    printf("[-s integer] [-e encoding] [-q]\n");
    printf("\nEncoding is one of:\n");
    printf("	ntsc-composite\n");
    printf("	pal-composite\n");
    printf("	secam-composite\n");
    printf("	ntsc-svideo\n");
    printf("	pal-svideo\n");
    printf("	secam-svideo\n");
    printf("	ntsc-rgb\n");
    printf("	pal-rgb\n");
    printf("	secam-rgb\n");

    exit(1);
}

Bool InternXvAtoms(dpy, encoding, saturation, hue, contrast, brightness)
    Display        *dpy;
    Atom           *encoding, *saturation, *hue, *contrast, *brightness;
{
    *encoding = XInternAtom(dpy, "XV_ENCODING", True);
    if (*encoding == None)
	return False;
    *saturation = XInternAtom(dpy, "XV_SATURATION", True);
    if (*saturation == None)
	return False;
    *hue = XInternAtom(dpy, "XV_HUE", True);
    if (*hue == None)
	return False;
    *contrast = XInternAtom(dpy, "XV_CONTRAST", True);
    if (*contrast == None)
	return False;
    *brightness = XInternAtom(dpy, "XV_BRIGHTNESS", True);
    if (*brightness == None)
	return False;
    return True;
}


static void CheckEncodingName(str)
    char *str;
{
    if (strcmp("ntsc-composite", str) == 0) {
	return ;
    } else if (strcmp("pal-composite", str) == 0) {
	return ;
    } else if (strcmp("secam-composite", str) == 0) {
	return ;
    } else if (strcmp("ntsc-svideo", str) == 0) {
	return ;
    } else if (strcmp("pal-svideo", str) == 0) {
	return ;
    } else if (strcmp("secam-svideo", str) == 0) {
	return ;
    } else if (strcmp("ntsc-rgb", str) == 0) {
	return ;
    } else if (strcmp("pal-rgb", str) == 0) {
	return ;
    } else if (strcmp("secam-rgb", str) == 0) {
	return ;
    } else {		/* no match */
	Usage();
    }
}

static int ConvertToInteger(str)
    char *str;
{
    int i;
    int start;
    if (str[0] == '-') 
        start = 1;
    else
	start = 0;
    for (i=start; i<strlen(str); i++) {
	if (! isdigit(str[i])) Usage();
    }
    return atoi(str);
}

static void ParseCommandLine(argv, argc, opts)
    int	argc;
    char *argv[];
    Options *opts;
{
    int i;
    int c;
    extern char *optarg;
    Bool foundOne = False;

    opts->brightness = False;
    opts->contrast = False;
    opts->hue = False;
    opts->saturation = False;
    opts->query = False;
    opts->encoding = False;
    opts->displayName = NULL;
    
    while ((c = getopt(argc, argv, "qd:b:c:h:s:e:")) != EOF) {
	foundOne = True;
	switch (c) {
	    case 'd':
    	        opts->displayName = optarg;
		break;
	    case 'b':
	        opts->brightness = True;
	        opts->b = ConvertToInteger(optarg);
		break;
	    case 'c':
	        opts->contrast = True;
	        opts->c = ConvertToInteger(optarg);
		break;
	    case 'h':
	        opts->hue = True;
	        opts->h = ConvertToInteger(optarg);
		break;
	    case 's':
	        opts->saturation = True;
	        opts->s = ConvertToInteger(optarg);
		break;
	    case 'e':
	        opts->encoding = True;
	        opts->e = optarg;
	        CheckEncodingName(opts->e);
		break;
	    case 'q':
	        opts->query = True;
		break;
	    default:
	        Usage();
	}
    }
    if (!foundOne) Usage();
}

static void QueryAllInformation(dpy, port, 
	batom, catom, hatom, satom, eatom, version, revision)
    Display	    *dpy;
    XvPortID	    port;
    Atom            batom, catom, hatom, satom, eatom;    
    int		    version, revision;    
{    
    int value;
    int             nEncodings;
    XvEncodingInfo *encodingInfo;
    XvEncodingInfo *a;
    int i;

    printf("Xv V%01d.%d\n\n", version, revision);

    XvGetPortAttribute(dpy, port, eatom, &value);    
    XvQueryEncodings(dpy, port, &nEncodings, &encodingInfo);
    for (i = 0; i < nEncodings; i++) {
	a = &encodingInfo[i];
	if (a->encoding_id == value) {
	    printf("Encoding: %s\n", a->name);
	}
    }
    XvGetPortAttribute(dpy, port, catom, &value);    
    printf("Contast: %d\n", value);

    XvGetPortAttribute(dpy, port, hatom, &value);    
    printf("Hue: %d\n", value);

    XvGetPortAttribute(dpy, port, satom, &value);    
    printf("Saturation: %d\n", value);

    XvGetPortAttribute(dpy, port, batom, &value);    
    printf("Brightness: %d\n", value);
    XvFreeEncodingInfo(encodingInfo);
}

int main(argc, argv)
    int             argc;
    char           *argv[];
{
    Display        *dpy;
    int             version, revision, major_opcode;
    int		     event_base, error_base, status;
    int             width, height;
    XvRational      rate;
    unsigned long   nAdaptors;
    XvAdaptorInfo  *pAdaptors;
    XvPortID        port;
    Atom            batom, catom, hatom, satom, eatom;
    Options	    opts;
    int		    nEncodings;
    XvEncodingInfo  *encodingInfo;
    XvEncodingInfo  *a;
    int		    i;

    ParseCommandLine(argv, argc, &opts);

    dpy = XOpenDisplay(opts.displayName);
    if (!dpy) {
	printf("Couldn't open display %s\n", opts.displayName);
	exit(-1);
    }
    status = XvQueryExtension(dpy, &version, &revision,
			      &major_opcode, &event_base, &error_base);

    if (status != Success) {
	printf("Xv video extension not available\n");
	exit(-1);
    }
    if (! InternXvAtoms(dpy, &eatom, &satom, &hatom, &catom, &batom)) {
	printf("Encoding atoms not set up properly\n");
	exit(-1);
    }

    XvQueryAdaptors(dpy, XDefaultRootWindow(dpy), &nAdaptors, &pAdaptors);
    if (!nAdaptors) {
	printf("Your display has no video adaptors\n");
	exit(-1);
    }
    port = pAdaptors->base_id;

    if (opts.query) {
	QueryAllInformation(dpy, port, 
			    batom, catom, hatom, satom, eatom, 
			    version, revision);
	exit(0);
    }
    if (opts.brightness) {
        XvSetPortAttribute(dpy, port, batom, opts.b);
    }
    if (opts.contrast) {
        XvSetPortAttribute(dpy, port, catom, opts.c);
    }
    if (opts.hue) {
        XvSetPortAttribute(dpy, port, hatom, opts.h);
    }
    if (opts.saturation) {
        XvSetPortAttribute(dpy, port, satom, opts.s);
    }
    if (opts.encoding) {
	XvQueryEncodings(dpy, port, &nEncodings, &encodingInfo);
	for (i=0; i<nEncodings; i++) {
	    a = &encodingInfo[i];
	    if (strcmp(a->name, opts.e) == 0) {
	        XvSetPortAttribute(dpy, port, eatom, a->encoding_id);
		break;		
	    }
	}
	XvFreeEncodingInfo(encodingInfo);
    }
    XSync(dpy, 0);
}
