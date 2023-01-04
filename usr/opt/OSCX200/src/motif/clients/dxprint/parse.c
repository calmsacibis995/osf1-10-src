/*
*****************************************************************************
**                                                                          *
**                   COPYRIGHT (c) 1988, 1991, 1992 BY                      *
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
**
**  MODULE:
**
**	parse.c
**
**  FACILITY:
**	Printscreen
**
**  ABSTRACT:
**
**	parse input parameters
**
**  ENVIRONMENT:
**
**	VMS V5, Ultrix V2.2  DW FT1
**
**  AUTHORS:
**      Kathy Robinson, Mark Antonelli
**
**  RELEASER:
**	Kathy Robinson
**
**  CREATION DATE:
**	April 26, 1987
**
**  MODIFICATION HISTORY:
**
**	11-Oct-1993	Peter Wolfe
**		Fix ootb_bug 66. command.options.sixel_device is not
**		properly initialized when print screen is invoked from 
**		the command line. Fixed/implemented commands.options.template
**		misuse (should have been sixel_device).
**		Fixed conflict with default tookit -g[eometry] flag
**		and out use of that for greyscale. 
**	26-Feb-1992	Edward P Luwish
**		Add new splitter routine that compensates for the fact
**		that on VMS, command line must be quoted.
**
**	21-Feb-1992	Edward P Luwish
**		Add support for debug, Motif interface and batch render.
**
**	17-Aug-1991	Edward P Luwish
**		Major rewrite for OSF-1 to use getopt().  This is now a
**		supported interface to Print Screen for customers, so
**		numerous improvements have been made.
**	07-Dec-1993	Dhiren M Patel
**		Fix ootb_bug 447. XmProcessTraversal was called with
**		XmTRAVERSE_NEXT_TAB_GROUP as value for direction 
**		parameter. The correct parameter is XmTRAVERSE_CURRENT.
**		In End_Text() free the font list after destroying the
**		text widget (changed the order of calls).
**
**--
**/
/*
 *
 *	parse(argc, argv)
 *		argc	- (RO) (int)
 *		argv	- (RO) (**char)
 *
 *
 */

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: [parse.c,v 1.2 91/07/01 17:52:25 mmeyer Exp ]$";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

#include <stdio.h>
#include <stdlib.h>
#include <img/IdsImage.h>
#include "iprdw.h"
#include "smdata.h"

#ifdef VMS
#include "desc.h"
#include <climsgdef>
#endif


/* For OSF/1, there's an include file with these externs */
#ifdef __osf__		
#include <getopt.h>
#else

#ifndef VMS		/* else, any other UNIX */
extern char *optarg;
extern int  optind;
extern int  opterr;
extern char optopt;
#else                   /* VMS */
char *optarg;
int	optind = 1;
int	opterr = 0;
char	optopt;
#endif /* ifndef VMS */
#endif /* ifdef __osf__ */

static void usage PROTOTYPE((void));

#if defined(VMS)
static int getopt PROTOTYPE((int argc, char **argv, char *opts));
#endif

int parse
#if _PRDW_PROTO_
(
    int		argc,
    char	**argv
)
#else
(argc, argv)
    int		argc;
    char	**argv;
#endif
{
    int		aflag = 0; /* -a aspect ratio - only valid with -s format */
    int		bflag = 0; /* -b black and white printing */
    int		cflag = 0; /* -c color printing */
    int		fflag = 0; /* -f fit-to-paper - only valid with -p format */
    int		hflag = 0; /* -h height - requires argument */
    int		oflag = 0; /* -o Ximage output, run auxiliary code or exit */
    int		pflag = 0; /* -p postscript format */
    int		rflag = 0; /* -r reverse contrast. not valid with -c printing */
    int		sflag = 0; /* -s sixel format */
    int		tflag = 0; /* -t time-delay - requires argument */
    int		wflag = 0; /* -w width - requires argument */
    int		xflag = 0; /* -x x-coordinate - requires argument */
    int		yflag = 0; /* -y y-coordinate - requires argument */
    int		Dflag = 0; /* -D DDIF output */
    int		Fflag = 0; /* -F Fast - capture, then render in background */
    int		Gflag = 0; /* -G greyscale printing */
    int		Iflag = 0; /* -I Internal call from client message */
    int		Pflag = 0; /* -P Printer - requires argument */
    int		Rflag = 0; /* -R Render only - requires argument */
    int		Tflag = 0; /* -T DAS Template number - requires argument */
    int		Xflag = 0; /* -X Create X user interface */

    char	c;

#ifdef VMS
    /*
    ** dxprint without arguments is equivalent to dxprint -X (VMS only)
    */
    if(argc == 1)
    {
	command.options.command_mode = dxPrscXmode;
	return;
    }
#endif

    opterr	= 0;

/* [pjw] From the osf manpage. May not macth exact implementation here since
 * I'm not sure where the code was taken from.
 * The getopt command is used to parse a list of tokens using a format that
 * specifies expected flags and arguments.  A flag is a single ASCII letter
 * and, when followed by a : (colon), is expected to take a modifying argument
 * that may or may not be separated from it by one or more tabs or spaces.
 */ 

    while ((c = getopt (argc, argv, "abcdfh:oprst:w:x:y:DGIFP:R:S:T:X")) != EOF)
    {
	switch(c)
	{
	case 'a':
	    aflag++;
	    Dprintf(("a option\n"), DbgMsgOnly );
	    if(pflag) usage();
	    if(Dflag) usage();
	    command.options.aspect = dxPrscPixAsp2;
	    break;
	case 'b':
	    bflag++;
	    Dprintf(("b option\n"), DbgMsgOnly );
	    if(cflag) usage();
	    if(Gflag) usage();
	    command.options.print_color = dxPrscPrinterBW;
	    break;
	case 'c':				/* Color capture */
	    cflag++;
	    Dprintf(("c option\n"), DbgMsgOnly );
	    if(bflag) usage();			/* B&W, */
	    if(Gflag) usage();			/* Greyscale, */
	    if(rflag) usage();	/* and reverse and mutually exclusive */
	    command.options.print_color = dxPrscPrinterColor;
	    break;
	case 'D':
/* 
 * DDIF formatted output. Note that the D is uppercase. The toolkit's
 * -d[isplay] flag is lowercase
 */
	    Dflag++;
	    Dprintf(("d option\n"), DbgMsgOnly );
	    if(pflag) usage();
	    if(sflag) usage();
	    if(aflag) usage();
	    if(fflag) usage();
	    command.options.storage_format = dxPrscDDIF;
	    break;
	case 'f':			/* Scale to Fit */
	    fflag++;
	    Dprintf(("f option\n"), DbgMsgOnly );
	    if(Dflag) usage();
	    if(sflag) usage();
/* dp:	The default for postscript is dxPrscCrop.
 *	The user can get to fit on a page whatever he/she 
 *	selects by using this flag. The fit option would default 
 *	to dxPrscReduce if b&w postscript is implicitly specified 
 *	i.e. no -p or [-G OR -C] is specified.
 *	Also, this flag CANNOT be specified with sixels or ddif
 *	captures.
 */
	    command.options.fit = dxPrscReduce;
	    break;
	case 'h':
	    hflag++;
	    Dprintf(("h option\n"), DbgMsgOnly );
	    command.options.h_coord = atoi(optarg);
	    Dprintf(("h coord is %d\n",command.options.h_coord), DbgMsgOnly );
	    break;
	case 'o':
	    oflag++;
	    Dprintf(("o option\n"), DbgMsgOnly );
	    if(Fflag) usage();
	    if(Rflag) usage();
	    command.options.run_mode =
		command.options.run_mode + dxPrscXimageOnly;
	    break;
	case 'p':
	    pflag++;
	    Dprintf(("p option\n"), DbgMsgOnly );
	    if(Dflag) usage();
	    if(sflag) usage();
	    if(aflag) usage();
	    command.options.storage_format = dxPrscPostscript;
	    break;
	case 'r':
	    rflag++;
	    Dprintf(("r option\n"), DbgMsgOnly );
	    if(cflag) usage();
	    command.options.reverse_image = dxPrscNegative;
	    break;
	case 's': 				/* Sixel format output */
	    sflag++;
	    Dprintf(("s option\n"), DbgMsgOnly );
	    if(Dflag) usage();			/* DDIF format, */
	    if(pflag) usage();			/* PS format,   */
	    if(fflag) usage();	/* and form feed are mutually exclusive */
	    command.options.storage_format = dxPrscSixel;
	    break;
	case 't':
	    tflag++;
	    Dprintf(("t option\n"), DbgMsgOnly );
	    command.options.time_delay = atoi(optarg);
	    Dprintf(("time delay is %d\n",command.options.time_delay), DbgMsgOnly );
	    break;
	case 'w':
	    wflag++;
	    Dprintf(("w option\n"), DbgMsgOnly );
	    command.options.w_coord = atoi(optarg);
	    Dprintf(("w coord is %d\n",command.options.w_coord), DbgMsgOnly );
	    break;
	case 'x':
	    xflag++;
	    Dprintf(("x option\n"), DbgMsgOnly );
	    command.options.x_coord = atoi(optarg);
	    Dprintf(("x coord is %d\n",command.options.x_coord), DbgMsgOnly );
	    break;
	case 'y':
	    yflag++;
	    Dprintf(("y option\n"), DbgMsgOnly );
	    command.options.y_coord = atoi(optarg);
	    Dprintf(("y coord is %d\n",command.options.y_coord), DbgMsgOnly );
	    break;
	case 'F':
	    Fflag++;
	    Dprintf(("F option\n"), DbgMsgOnly );
	    if(oflag) usage();
	    if(Rflag) usage();
	    command.options.run_mode =
		command.options.run_mode + dxPrscFastCapture;
	    break;
	case 'G':			/* Greyscale */
	    Gflag++;
	    Dprintf(("g option\n"), DbgMsgOnly );
	    if(bflag) usage();
	    if(cflag) usage();
	    command.options.print_color = dxPrscPrinterGrey;
	    break;
	case 'I':
	    Iflag++;
	    Dprintf(("I option\n"), DbgMsgOnly );
	    command.options.run_mode = 
		command.options.run_mode + dxPrscFromCallback;
	    break;
	case 'P':
	    Pflag++;
	    Dprintf(("P option\n"), DbgMsgOnly );
	    break;
	case 'R':
	    Rflag++;
	    Dprintf(("R option\n"), DbgMsgOnly );
	    if(oflag) usage();
	    if(Fflag) usage();
	    if(Xflag) usage();
	    command.options.run_mode = 
		command.options.run_mode + dxPrscRenderOnly;
	    strcpy(tempFilename,optarg);
	    break;
		   		/* IdsCreatePresentSurface template, i.e. 
				   sixel device type */
	case 'T':  
	    Tflag++;
	    Dprintf(("T option\n"), DbgMsgOnly );
	    command.options.sixel_device = atoi(optarg);
	    Dprintf(("tmpl is %d\n",command.options.sixel_device), DbgMsgOnly );
	    break;
	case 'X':
	    Xflag++;
	    Dprintf(("X option\n"), DbgMsgOnly );
	    if(Rflag) usage();
	    command.options.command_mode = dxPrscXmode;
	    break;
	case '?':
	    if(	(optopt == 'h')||(optopt == 't')||(optopt == 'w')||
		    (optopt == 'x')||(optopt == 'y')||(optopt == 'P')||
		    (optopt == 'R')||(optopt == 'S')||(optopt == 'T')    )
	    Dprintf(("Option %c missing required parameter\n",optopt),
		    DbgMsgOnly );
	    usage();
	    break;
	}	/* switch and single-statment while */
    }

    /* Check if defaults used, and if therefore any inconsistent options */

    if((pflag == 0) && (sflag + Dflag == 0)) /* PostScript by default */
    {
	command.options.storage_format = dxPrscPostscript;
	command.options.fit = dxPrscReduce;
	if (aflag) usage();
	Dprintf(("PostScript and fit-to-page by default\n"), DbgMsgOnly );
    }

    /* Check for proper partial capture syntax and set partial capture flag */
    if (xflag + yflag + hflag + wflag)
    {
	if (!(xflag * yflag * hflag * wflag))
	    usage();
	else
	    command.options.partial_capture = dxPrscPartial;
    }

    /* I'm not sure if these consistency checks belong here or closer
     * to the routines in which they are relevant. I'm leaving them here
     * for now. 
     * For Sixel screen captures, command.options.storage_format is
     * set to sixels above but sixel_device also needs to be initialized. 
     * The actual device corresponds to the device types supported by
     * the IdsCreatePresentSurface call increatoutput/creatsx. 
     * The -T (capital t) allows the user to specify the IDS device
     * template for the desired sixel output device. So much for
     * a polished UI. However, at least it's simple and minimizes
     * then I18N support work required for allowing string names. 
     * If -T is not specified, we need to to choose an intelligent 
     * default. The valid command lines will default as follows:
     *	Color Option:		Sixel device default:
     *		B&W			Ids_TmpltLn03s  = LN03s
     *		Color                   Ids_TmpltLj250Lr = LJ250LR
     *		Greyscale		Ids_TmpltLn03s  = LN03s
     *
     * dp:
     * LJ250 is 180x180 dpi with 8 colors
     * LJ250LR is 90x90 dpi with 256 colors (called "low resolution"
     *                          due to 90x90 dpi)
     * The default is LJ250LR to get more colors !!!!
     *
     */

     if (sflag == 1)				/* Sixel format */
     {
	/* User didn't specify any color options*/	
	if ( (bflag == 0) && (cflag == 0) && (Gflag == 0) )
	{
	    bflag = 1;				/* Default to black and white */
 	    command.options.print_color = dxPrscPrinterBW;
	}						

	if (!Tflag)	/* If the user did not specify a IDS device template */
	{
	    if (bflag == 1)	/* and chose black$white */
		command.options.sixel_device = Ids_TmpltLn03s;
	    else if (cflag == 1)	/* choose color */
		command.options.sixel_device = Ids_TmpltLj250Lr;
	    else			/* greyscale */ 
		command.options.sixel_device = Ids_TmpltLn03s;
	}

     }	/* End sixels consistency checks */

    /*
     * Set up output filename. Must first check for null argument
     */
    if ((argv[optind] == NULL) || (*argv[optind] == 0))
    {
	Dprintf(("no file name supplied\n"), DbgMsgOnly );
    }
    else
    {
	strcpy(command.print_dest,argv[optind]);
	Dprintf(("file name should be %s\n",command.print_dest), DbgMsgOnly );
    }
    return;
} /* parse( */


static void usage ()
/*
 * Function: usage
 *	
 * Inputs:  None
 *	
 * Outputs: Textual explanation of command line options
 *	
 * Notes:
 * 	The -D option for DDIF output must be in upper case else
 * 	it conflicts with the toolkit -display option. The 
 * 	code previously used -D to enable debugging but this 
 * 	currently disabled. The debug option needs to use another
 * 	command like switch. 
 */
{
fprintf(stderr,"Usage:\n");
fprintf(stderr,"  dxprint -X\n");
fprintf(stderr,"  dxprint [-p] [-b|G] [-f] [-r] [geom*] [-t seconds] [-P queue] [file]\n");         
fprintf(stderr,"  dxprint [-p] -c [-f] [geom*] [-t seconds] [-P queue] [file]\n");
fprintf(stderr,"  dxprint -D [-b|G] [-r] [geom*] [-t seconds] [-P queue] [file]\n");
fprintf(stderr,"  dxprint -D -c [geom*] [-t seconds] [-P queue] [file]\n");
fprintf(stderr,"  dxprint -s [-b|G] [-a] [-r] [geom*] [-t seconds] [-T n] [-P queue] [file]\n");
fprintf(stderr,"  dxprint -s -c [geom*] [-t seconds] [-T n] [-P queue] [file]\n");
fprintf(stderr,"    *geom is -x n -y n -h n -w n\n");
exit(Normal);
}		/* end routine usage */

#if defined(VMS)

#ifndef NULL
#define NULL	0
#endif

#ifndef EOF
#define EOF	(-1)
#endif

#ifndef ERR
#define ERR(s, c)	if(opterr){\
	extern int write();\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
	(void) write(2, s, (unsigned)strlen(s));\
	(void) write(2, errbuf, 2);}
#endif

static int getopt
#if _PRDW_PROTO_
(
    int		argc,
    char	**argv,
    char	*opts
)
#else
(argc, argv, opts)
int	argc;
char	**argv, *opts;
#endif
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == 0) {
			optind++;
			return(EOF);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == NULL) {
		ERR(": illegal option -- ", c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			ERR(": option requires an argument -- ", c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

#endif /* VMS */
