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
/*
**++
**  COPYRIGHT (c) 1987, 1992 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**  ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**--
**/


/*
**++
**  MODULE NAME:
**	dvr_wmn.c
**
**  FACILITY:
**      CDA Viewer
**
**  ABSTRACT:
**	This is the driver module for the ddif viewer application: dxvdoc
**	on ultrix and OS/2, it parses the command line and flags any errors.
**	If all is correct, it calls Dvr__DECW_Application which does all the
**	work.
**
**  AUTHORS:
**      Dennis McEvoy
**
**  RELEASER:
**
**  CREATION DATE:     15-FEB-1987
**
**  MODIFICATION HISTORY:
**	01 3-aug-88	dam	accept "-" for stdin
**	02 10-aug-88	dam	incorparate changes made by UEG to combine
**				dxvdoc with other images.
**	03 20-feb-89	dam	update for v2
**	04 05-mar-90	dam	changes for OS/2 port
**	05 12-jun-90	dam	make options case insensitive for os/2
**	06 06-sep-90	dam	default format to all caps for os/2
**	07 15-may-91	dam	change to motif include
**	08 11-jun-91	dam	default to -f ps if invoked as dxpsview
**	09 05-aug-91	dam	rename headers, remove dollar signs
**	10 16-sep-91	jjt	remove definition of NULL, not used in module.
**      18-Mar-1992	PBD	Add CDA ident string.
**      29-apr-1992	dam	fix command line parsing for options file
**	13 15-jul-92	rdh	Specify same-directory includes with ""
**	  7-apr-92	PBD	Use CdaGetDataType to determine type of file 
** 				specified on command line.
**
**--
**/

/*
**
**  INCLUDE FILES
**
**/
#include <cdaident.h>
static char *cda_ident_string = CDA_VERSION;

#include <cdatrans.h>

#ifdef OS2
#include <os2.h>

/*  multithreading is a switch for the application, not the widget, so
 *  this define does not go in dvr_decw_def.h
 */
#define DvrUseMultithread (1<<10)

#endif

#ifdef __unix__
#include <Mrm/MrmAppl.h>
#endif

#include <stdio.h>
#include <string.h>
#include <cdaptp.h>
#include <dvrwdef.h>
#include <dvrmsg.h>  /* status codes */
#include <cdadef.h>
#include <cdamsg.h>

/* external routine prototypes */

PROTO( unsigned long Dvr__DECW_Application,
		(char *,
		 char *,
		 char *,
		 long *,
		 int,
		 char * *,
		 unsigned long,
		 unsigned long) );


/* local routine prototypes */

#ifdef COMBINE
PROTO( int doc_main,
		(int,
		 char * *) );

#else
PROTO( int main,
		(int,
		 char * *) );
#endif

#ifndef __osf__
PROTO( int getopt,
		(int,
		 char * *,
		 char *) );
#endif

/* usage strings */

#ifdef OS2
#define dvr_dxvdoc_usage1_str	"\nusage: dxvdoc [/f format] [/O options file]\n"
#define dvr_dxvdoc_usage2_str	"              [/w paper width] [/h paper height]\n"
#define dvr_dxvdoc_usage3_str	"              [/r] file\n"
#else /* ultrix */
#define dvr_dxvdoc_usage1_str	"\nusage: dxvdoc [-f format] [-O options file]\n"
#define dvr_dxvdoc_usage2_str	"              [-w paper width] [-h paper height]\n"
#define dvr_dxvdoc_usage3_str	"              [-r] file\n"
#endif


#ifdef OS2
/* default format string */
#define dvr_default_format_str	"DDIF"

int optind = 1;				/* Receives argv index of the next */
					/*    argument to be processed	   */
unsigned char *optarg;			/* Points to start of option arg   */
					/*    on return from getopt	   */
#endif

#ifdef __unix__
/* default format string */
#define dvr_default_format_str	"ddif"

    extern 	int optind;		/* globals used by getopt	      */
    extern 	char *optarg;
#endif



/*
**++
**  ROUTINE NAME:
**	main(argc, argv)
**
**  FUNCTIONAL DESCRIPTION:
**      This is the driver routine for the ultrix DEC windows ddif viewer
**	application; It parses the command line and then calls
**	Decw$Viewer which does all the work.
**
**  FORMAL PARAMETERS:
**	int		argc;
**	char		**argv;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

/*  constant COMBINE is used to determine if application is being hard
 *  linked to other applications to save space on ultrix.  If it is, then
 *  this is not the main entry point. If we are not being linked to other
 *  applications, then this is the main entry point.
 */

#ifdef COMBINE
doc_main(argc, argv)
#else
main(argc, argv)
#endif

    int  argc;
    char **argv;
{

    char	file_name_str[256];	/* name of file to view 	      */
    char	format_str[256];	/* format of file to pass to cda      */
    char	op_file_str[256];	/* options file to pass to cda	      */
    char	proc_op_str[20];	/* processing options for dxvdoc	      */
    long	select_options = 0;	/* selection options mask	      */
    int		i, c, j;		/* counters			      */
    int 	errflg = 0;		/* tells if an error has been seen    */
    int		proc_op_flag = 0;	/* tells if proc options have been set*/
    unsigned long paper_width = 0;
    unsigned long paper_height = 0;

#ifdef OS2
    DVR_BOOLEAN multithread = FALSE;
#endif

    op_file_str[0] = '\0';		/* these arguments are null strings   */
    file_name_str[0] = '\0';		/* by default			      */
    format_str[0] = '\0';

    /* parse the command line for valid args */

    if (argc > 1)
      while( (!errflg)  &&
#ifdef OS2
             ((c = getopt (argc, argv, "f:F:o:O:w:W:h:H:rRmM")) != EOF) )
#else
             ((c = getopt (argc, argv, "f:O:P:d:w:h:r")) != EOF) )
#endif
	switch(c) {


	  /* 'f' is for the file format, store the string */
#ifdef OS2
	  case 'F':
#endif
	  case 'f': strcpy(format_str, optarg);
		    break;

	  /* 'O' is for the options file string, store the string */
#ifdef OS2
	  case 'o':
#endif
	  case 'O': strcpy(op_file_str, optarg);
		    break;

	  /* w is for paper width */
#ifdef OS2
	  case 'W':
#endif
	  case 'w':
		    sscanf(optarg, "%d", &paper_width);
		    break;

	  /* h is for paper height */
#ifdef OS2
	  case 'H':
#endif
	  case 'h':
		    sscanf(optarg, "%d", &paper_height);
		    break;

	  /* r is for reformat (false if not specified) */
#ifdef OS2
	  case 'R':
#endif
	  case 'r':
		    proc_op_flag = 1;
		    select_options =  DvrWordWrap;
		    break;
#ifdef OS2
	  /* 'm' is for the multithreading option */
	  case 'M':
	  case 'm': multithread = TRUE;
		    break;
#endif

#ifdef __unix__
	  /* 'd' is used by Xlib for display */
	  case 'd': break;

	  /*  'P' is for the processing options string,
           *  parse the string and set the processing options
           *  mask accordingly
	   */
	  case 'P': strcpy(proc_op_str, optarg);
		    proc_op_flag = 1;	/* set flag, do not use defaults */

		    /*  the string cannot be longer than 2 characters or
		     *  less than 1 character; if so, set error flag
		     */
		    if ((strlen(proc_op_str) <1) || (strlen(proc_op_str)>2))
			errflg = 1;

		    /*  if this character is an 'n', then make sure it
		     *  is the only character; 'n' means none, and is not
		     *  valid when used with the other options.
		     */
		    else if(proc_op_str[0] == 'n')
		      {
			if (strlen(proc_op_str) != 1)
			    errflg = 1;
		      }

		    /* else, parse the string */
		    else for (j=0;j<strlen(proc_op_str);j++)
		      {
			/* 's' means soft directives */
			if (proc_op_str[j] == 's')
			  {
			    select_options = select_options | DvrSoftDirectives;
			  }

			/* 'w' means word wrap */
			else if (proc_op_str[j] == 'w')
			  {
			    select_options = select_options | DvrWordWrap;
			  }

			/* anything else is invalid */
			else errflg = 1;
		      }
		    break;
#endif

	  /* '?' is returned by getopt if the char is invalid */
	  case '?':
#ifdef OS2
                    errflg = 1;
                    break;
#else
		    /*  for decwindows, XtInitialize() may care about
		     *	some command line parameters that dxvdoc does not
		     *  (like -d node::0), so in this case, do nothing
		     */
		    ;
#endif


	  }

	/*  pass any other arguments to XtInitialize() in
	 *  Dvr__DECW_Application
  	 */

#ifdef __unix__
	if (errflg)
	  /* print a usage message */
	  {
	     fprintf(stderr, dvr_dxvdoc_usage1_str);
	     fprintf(stderr, dvr_dxvdoc_usage2_str);
	     fprintf(stderr, dvr_dxvdoc_usage3_str);
	  }

	else /* no errors, start application */
#endif
	  {
  	    /* if processing options have not been set, use the defaults */
	    if (!proc_op_flag)
		select_options = DvrWordWrap | DvrSoftDirectives |
				 DvrLayout | DvrSpecificLayout;

#ifdef OS2
            if (multithread)
                select_options |= DvrUseMultithread;
#endif

	    if (argc > 1)
              {
#ifdef OS2
                /* if this is a switch, do not use for file name */
                if (!strchr(argv[argc-1], '/'))
#endif
		    strcpy(file_name_str, argv[argc-1]);
              }

	    /* if the file format has not been set, use the default (ddif) */
	    if (strlen(format_str) == 0)
	      {
		if (strstr(argv[0], "dxpsview"))
		    /* invoked as dxpsview, use ps as default format */
		    strcpy(format_str, "ps");
		else
		    /* default, not invoked as dxpsivew, use ddif as default */
		    if ((file_name_str[0] == '\0') 
		       || (file_name_str[0] == '-' && file_name_str[1] == '\0'))
			strcpy(format_str, dvr_default_format_str);
		    else
			{
			CDAitemlist	itmlst[3];
			CDAconstant	file_type;
			
			itmlst[0].item_length = strlen(file_name_str);
			itmlst[0].item_code = CDA_FILE_NAME;
			itmlst[0].CDAitemparam.item_address = file_name_str;

			itmlst[1].item_length = sizeof(file_type);
			itmlst[1].item_code = CDA_FILE_TYPE_VALUE;
			itmlst[1].CDAitemparam.item_address = &file_type;

			itmlst[2].item_length = itmlst[2].item_code = 0;

			if (CDA_NORMAL == CdaGetDataType(itmlst))
			    switch (file_type)
				{
				case CDA_K_FILE_CONTENT_DDIF:
				    strcpy(format_str, "ddif");
				    break;
				case CDA_K_FILE_CONTENT_DOTS:
				    strcpy(format_str, "dots");
				    break;
				case CDA_K_FILE_CONTENT_DTIF:
				    strcpy(format_str, "dtif");
				    break;
				case CDA_K_FILE_CONTENT_PS:
				    strcpy(format_str, "ps");
				    break;
				case CDA_K_FILE_CONTENT_TEXT:
				    strcpy(format_str, "text");
				    break;
				default:
				    strcpy(format_str, dvr_default_format_str);
				    break;
				}
			else
			    strcpy(format_str, dvr_default_format_str);

			}
	      }

#ifdef OS2
            /* for OS/2, the usage string is printed in a window */
            if (errflg)
                /* pass application bad format to indicate usage error */
                strcpy(format_str, "***");
#endif

	    /* start the application */
    	    return( (int)
                Dvr__DECW_Application(file_name_str, format_str, op_file_str,
			&select_options, argc, argv, paper_height,
			paper_width)
                  );
	  }

    return(0);
}


#ifdef OS2
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Routine getopt(argv,**argc,*option_list).
**	This routine parses the argv input list in pairs.
**
**  FORMAL PARAMETERS:
**
**	argv		# of items in argc array.
**	argc		array of pointers to input strings, treated in pairs,
**			depending upon the arg string. If a ':' occurs after
**			the parameter, then the next item is picked off to be
**			the argurments arguement.
**			The 1st of the pair (even indexed items) are options.
**			The 2nd of the pair (odd  indexed items) are arguments
**			    to the options.
**	option_list	string containing valid options.
**	The option list is a string with valid options separated by a ':'.
**
**  IMPLICIT INPUTS:
**	optind  - index # into argv array of next input to be processed.
**		  this must be initialized to 0.
**
**  IMPLICIT OUTPUTS:
**	optind	- contains index # of argv element for next call.
**	optarg  - contains argv[] of matched option.
**		  (address of argument to matching option).
**
**  FUNCTION VALUE:
**	The returned value is the option character, or the '?' character
**	if an invalid option was detected. The returned value is an int.
**
**	EOF	returned if fewer than 2 args left.
**
**  SIDE EFFECTS:
**	None.
**
**--
**/

int getopt(int argc, char * *argv, char *option_list)
{	char *o, *l;
	if (argc <= 1) return (int) '?';
	if (optind >= argc) return EOF;
	o = argv[optind];
	if ((*o != '/') && (optind == argc-1)) return EOF;
	if (*o++ != '/') return (int) '?';
	l = option_list;
	while (*l != '\0') {
	    if (*o == *l++) {
		optind++;
		if (*l == ':') {
		   if (optind >= argc) return '?';
		   optarg = argv[optind++];
		   }
		return (int) *o;
		}
	    if (*l == ':') l++;
	    }
	return (int) '?';
	}

#endif
