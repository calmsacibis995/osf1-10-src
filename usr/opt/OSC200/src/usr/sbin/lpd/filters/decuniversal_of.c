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
#ifndef lint
static char *sccsid = "@(#)$RCSfile: decuniversal_of.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/12/21 20:37:46 $";
#endif

 
/*
 * OSF/1 Release 1.0
 */
/*
 * File:	ln03_lg31_lg02_la75.c
 * Author:	Adrian Thoms (thoms@wessex)
 * Description:
 *	This is the print filter for the following printers:
 *	LN03, LG31, LG02, LG06 and LA75
 *
 *	This file was created by merging the code in lg31of.c, lg02of.c
 *	and la75.c back into ln03of.c from which they appear to have been
 *	derived.
 *
 * Modification history:
 *
 */


#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <a.out.h>
#include <imghdr.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "guesser.h"

struct filter_info;		/* forward declaration */
/*
 * Functions in this file
 */
static struct filter_info *
set_which_filter();		/* Determine which printer we are driving */

static int
filter();			/* Process the data */


#define MAXWIDTH		225		/* maximum char. width	*/
#define BUFWIDTH		300		/* maximum buf.  width	*/
#define MAXREP			10		/* buffer depth		*/
#define DEFWIDTH		80		/* width char. count	*/
#define DEFHEIGHT		66		/* length char. count	*/
#define MAXP_PIX_WIDTH		800		/* portrait pixel width */
#define MAXP_PIX_HEIGHT		1040		/* portrait pix. height */
#define MAXL_PIX_WIDTH		1040		/* landscape p. width	*/
#define MAXL_PIX_HEIGHT		800		/* landscape p. length	*/
#define MAXCOPIES		1		/* default # of copies	*/
#define ITS			77
#define SOFF			'\077'		/* sixel element offset */

#define RESET_SLEEP_TIME	5		/* Seconds to sleep after reset*/

#define MAX(a,b)		(a < b ? b : a) /* useful macros	*/
#define MAXIMUM(a,b,c)		(MAX(MAX(a,b),c))
#define MIN(a,b)		(a > b ? b : a)
#define MINIMUM(a,b,c)		(MIN(MIN(a,b),c))
#define mask(s)			(1 << ((s) - 1))/* Used by sigblock call */

/* Added a few macros for 8bit support */
#define is_7bit_cntrl(ch)	((unsigned char)ch < 040 || (unsigned char)ch == 0177)
#define is_8bit_cntrl(ch)	(is_7bit_cntrl(ch) || (((unsigned char)ch >= 0200) && ((unsigned char)ch <=0240)))

enum job_status_e {
	js_ok, js_retry, js_failed
};

FILE	*input = stdin, *output = stdout;	/* input and output	*/
char	buf[MAXREP][BUFWIDTH];			/* buffer for output	*/
						/* If nonzero return from */
int	maxcol[MAXREP] = {-1};			/* maximum columns	*/
int	lineno;					/* line number		*/
int	width = DEFWIDTH;			/* default line length	*/
int	length = DEFHEIGHT;			/* page length		*/
int	indent;					/* indentation length	*/
int	npages = 0;				/* number of pages printed*/
int	literal;				/* print control chars. */
int	error;					/* error return status	*/
int     esclen;                                 /* num escape chars     */
int     maxrep;                                 /* current depth in buf */
int     col;                                    /* column position      */
char	*name;					/* user's login name	*/
char	*host;					/* user's machine name	*/
char	*acctfile;				/* accounting info. file*/
int	kindofile = EMPTY_FILE;			/* initial kind of file */
int	tmppagecount = 0, tmplinecount = 0;	/* tmp counters		*/

char	*imgptr;				/* image data pointers	*/
short	*cmpptr;				/* color map pointer	*/
struct	imghdr im;				/* image file header	*/
char	*malloc();				/* malloc pointer	*/

unsigned  size;					/* the usual		*/
int	escflg = 0 ;		/* escape sequence flag, 1 = in progress */
int	esctrm = 0 ;		/* Escape sequence terminator type */
int	escpflg= 0 ;		/* ESC P in progress	 */
int	rotated = 0;		/* image rotated	*/

extern void exit(), bcopy();
extern int syslog();
extern unsigned	sleep();

/* sixmap is the dither pattern used to create sixel output bytes */
struct sixmap {
	char s0, s1, s2;
} base_sixmap[] = {
	{ 0+SOFF, 0+SOFF, 0+SOFF },
	{ 4+SOFF, 0+SOFF, 0+SOFF },
	{ 4+SOFF, 0+SOFF, 1+SOFF },
	{ 4+SOFF, 4+SOFF, 1+SOFF },
	{ 6+SOFF, 4+SOFF, 1+SOFF },
	{ 6+SOFF, 6+SOFF, 1+SOFF },
	{ 6+SOFF, 6+SOFF, 6+SOFF },
	{ 7+SOFF, 6+SOFF, 6+SOFF },
	{ 7+SOFF, 6+SOFF, 7+SOFF },
	{ 7+SOFF, 7+SOFF, 7+SOFF }
};

struct sixmap offset_sixmap[] = {
	{  0+SOFF,  0+SOFF,  0+SOFF },
	{ 32+SOFF,  0+SOFF,  0+SOFF },
	{ 32+SOFF,  0+SOFF,  8+SOFF },
	{ 32+SOFF, 32+SOFF,  8+SOFF },
	{ 48+SOFF, 32+SOFF,  8+SOFF },
	{ 48+SOFF, 48+SOFF,  8+SOFF },
	{ 48+SOFF, 48+SOFF, 48+SOFF },
	{ 56+SOFF, 48+SOFF, 48+SOFF },
	{ 56+SOFF, 48+SOFF, 56+SOFF },
	{ 56+SOFF, 56+SOFF, 56+SOFF }
};

/*
 * In order to merge the functionality of the ln03, lg31, lg02 and lg06
 * filters we need a table of structure to contain the difference information.
 * we index into this with an enum.
 *
 * We pass the known names by which the filter can be called from the
 * via pre-processor definitions
 */

static enum which_filter_e {		/* Enumerate different printers 
supported */
	ln03,
	lg31,
	lg02,
	lg06,
	la75,
	ln05,
	ln06,
	ln07,
	ln08,
	ansi_3of,
	ansi_2of
    } which_filter;

static struct filter_info {		/* Printer specific info */
	char *fi_name;
	char *fi_sixel_intro;
	char *fi_reset;
} filter_info[] = {
	{ "ln03of", "\033P9;0;1q\"1;1-", "\033c"},
	{ "lg31of", "\033P9;0;10q\"1;1-","\033[?7h\033[20h\033[1n\033[?40l"},
	{ "lg02of", "\033P9;0;0q\"1;1-", "\033c"},
	{ "lg06of", "\033P9;0;0q\"1;1-", "\033c"},
	{ "la75of", "\033P9;;5;q\"1;1-", "\033c"},
	{ "ln05of", "\033P9;0;1q\"1;1-", "\033[0!p"},
	{ "ln06of", "\033P9;0;1q\"1;1-", "\033[0!p"},
	{ "ln07of", "\033P9;0;1q\"1;1-", "\033[0!p"},
	{ "ln08of", "\033P9;0;1q\"1;1-", "\033[0!p"},
	{ "ansi_3of", NULL,		 "\033[0!p"},
	{ "ansi_2of", NULL,		 "\033[0!p"},
	{ NULL }
};

static char *pname;		/* Name program called with */
struct filter_info *fip;

static struct filter_info *
set_which_filter(progname)
	char *progname;
{
	register struct filter_info *fip;

	if ((pname=strrchr(progname, '/')) == NULL) {
		pname = progname;
	} else {	
		pname++;
	}
	for (fip=filter_info; fip->fi_name != NULL; fip++) {
		if (!strcmp(fip->fi_name, pname)) {
			which_filter =
			    (enum which_filter_e) (fip - filter_info);
			return(fip);
		}
	}
	fprintf(stderr, "%s: Must be called one of following:\n\t", pname);
	for (fip=filter_info; fip->fi_name != NULL; fip++) {
		fprintf(stderr, " %s", fip->fi_name);
	}
	putc('\n', stderr);
	exit(js_failed);
	/* not reached */
	return NULL;
}

void do_accounting()
{
 sigblock(mask(SIGINT) | mask(SIGHUP)) ;
 if (name && acctfile && access(acctfile, 02) >= 0 &&
	    freopen(acctfile, "a", output) != NULL) {
	printf("%7.2f\t%s:%s\n", (float)npages, host, name);
	fflush(stdout) ;
	}
 return ;
}

/*
 * If the filter has been killed reset the printer.
 */

void signal_detected()
{
 fprintf(output,fip->fi_reset); /* reset printer defaults */
 fflush(output);		/* make sure reset goes out */
 sleep(RESET_SLEEP_TIME);/* some printers eat lines during reset so wait */
 do_accounting() ;		/* MUST BE DONE LAST, uses freopen command */
 exit(js_failed) ;
}

/* The general strategy here is to reset the printer to initial state,
   sleep five seconds for stability, process the command line arguments,
   open the syslog file for log information, determine the input
   stream "file type", call the filter code, and then optionally
   process accounting information upon completion. Informational
   and failure conditions are logged to syslog.
*/
main(argc, argv)
	int argc;
	char *argv[];
{
	register int i;
	register char *cp;
	int landscape = 0; /* landscape determines the orientation on page prntrs */
	int tumble = FALSE;
	int duplex = FALSE;
	int two_sided = FALSE;
	signal(SIGHUP, signal_detected) ;
	signal(SIGINT, signal_detected) ;

	fip = set_which_filter(argv[0]);
	/* 
	 * The general strategy here is to reset the printer to initial state,
	 * sleep five seconds for stability, process the command line arguments,
   	 * determine the input stream "file type", call the filter code, and then 
	 * optionally process accounting information.
	 */

	fprintf(output, fip->fi_reset);		/* reset to initial state */
	sleep(RESET_SLEEP_TIME);
	while (--argc) {
		if (*(cp = *++argv) == '-') {
			switch (cp[1]) {
			case 'n':		/* collect login name	*/
				argc--;
				name = *++argv;
				break;

			case 'h':		/* collect host name	*/
				argc--;
				host = *++argv;
				break;

			case 'w':		/* collect page width	*/
				if ((i = atoi(&cp[2])) > 0)
					{
					if (i <= MAXWIDTH)
						width = i;
					else
						width = MAXWIDTH ;
					}
				break;

			case 'l':		/* collect page length	*/
				length = atoi(&cp[2]);
				if(length > 66)
					length=66 ;
				break;

			case 'i':		/* collect indent	*/
				indent = atoi(&cp[2]);
				break;

			case 'c':		/* print control chars	*/
				literal++;
				break;
			case 'O':		/* orientation for page printers */
				if(!strcmp("landscape", &cp[2])) landscape = 1;
				else if(!strcmp("portrait", &cp[2]))  landscape = -1;
				break;

			case 'K':		/* sides/duplex mode for pg printers */
				switch(which_filter) {
				    case ln05:
				    case ln06:
				    case ln07:
				    case ln08:
					if(cp[2] != NULL){
					    if(strstr(&cp[2],"tumble")){ 
						tumble = two_sided = TRUE;
					    }
					    if(strstr(&cp[2],"duplex")){ 
						duplex = TRUE;
					    }
					    if(strstr(&cp[2],"one")){ 
						two_sided = FALSE;
					    }
					    else if (strstr(&cp[2],"two")) {
						two_sided = duplex = TRUE;
					    }
					}
					/* now that we know what they want ... do it */
					if(two_sided){
					    if(tumble){
						fprintf(output,"\033[4 x");
					    } 
					    else {
						fprintf(output,"\033[3 x");
					    }
					}
					else { /* one sided */
					    if(tumble) {
						if(duplex){
						    fprintf(output,"\033[6 x");
						}
						else {
						    fprintf(output,"\033[2 x");
						}
					    } else {
						if(duplex){
						    fprintf(output,"\033[5 x");
						}
						else {
						    fprintf(output,"\033[1 x");
						}
					    }
					}
					break;

				    default:
					break;
				}
				break;

			case 'I':		/* input tray select for pg printers */
				switch(which_filter) {
				    case ln05:
					if(!strcmp("cassette",&cp[2]) ||
					   !strcmp("upper",&cp[2]))
						fprintf(output,"\033[1!v");
					else if(!strcmp("manual",&cp[2]))
						fprintf(output,"\033[99!v");

					break;
				    case ln06:
					if(!strcmp("upper",&cp[2]))
						fprintf(output,"\033[1!v");
					else if(!strcmp("envelope_feeder",&cp[2])) 
						fprintf(output,"\033[3!v");
					else if(!strcmp("lower",&cp[2]))
						fprintf(output,"\033[2!v");
					else if(!strcmp("manual",&cp[2]))
						fprintf(output,"\033[99!v");

					break;
				    case ln07:
					if(!strcmp("upper",&cp[2]) ||
					   !strcmp("manual",&cp[2]))
						fprintf(output,"\033[1!v");
					else if(!strcmp("lower",&cp[2]) || 
					     !strcmp("cassette", &cp[2]))
						fprintf(output,"\033[2!v");

					break;
				    case ln08:
					if(!strcmp("manual",&cp[2]))
					    fprintf(output,"\033[99!v");
					else if(!strcmp("mmf",&cp[2]))
					    fprintf(output,"\033[4!v");
					else if(!strcmp("upper",&cp[2]))
					    fprintf(output,"\033[1!v");
					else if(!strcmp("lower",&cp[2]))
					    fprintf(output,"\033[2!v");
					else if(!strcmp("lcit",&cp[2]))
					    fprintf(output,"\033[3!v");

				    default:
					break;
				}
			    default:
				break;
			}
		} else
			acctfile = cp;
	}
	openlog(name, LOG_PID, LOG_LPR);

	switch(which_filter) {
	case ln03:
	case ln05:
	case ln06:
	case ln08:
		if (((width > 80) && (landscape != -1)) || (landscape == 1)){ 
			/* switch to landscape mode */
			fprintf(output,"\033[15m"); /* change font */
			fprintf(output,"\033[7 J"); /* A4 page format */
			fprintf(output,"\033[66t"); /* 66 lines/page */
		}
		break;

	case lg31:
	case lg02:
	case lg06:
		break;

	case la75:
		if ( width > 80 ) /* change HOR pitch to 16.5 cpi */
		    fprintf(output,"\033[4w");
		break;

	default:
		break;
	}

	kindofile = determinefile(fileno(input));

	error = filter();

	if(error) {
		syslog(LOG_INFO,"%s: Failed to output data", pname);
		exit(js_failed);
	}
	do_accounting() ;
	exit(js_ok);
}

/* Here is where all the real output work begins. We switch
   to the appropriate code for the determined file type stream.
*/
static int
filter()
{
	register int i = 0;
	register char *cp;
	register int ch;
	register short *tmpptr, *tmp;
	unsigned int temp;
	int done, linedone;
	char *limit;
	register struct filter_info *fip= &filter_info[(int)which_filter];

	switch(kindofile) {
	case EMPTY_FILE:
		break;
	case EXECUTABLE_FILE:
	case ARCHIVE_FILE:
	case DATA_FILE:
	case CAT_FILE:
		syslog(LOG_INFO,"%s: Unprintable data", pname);
		fprintf (output, "\n\n\tFile contains non-printable data.\n\f");
		fflush (output);
		return(1);
		break;

	case POSTSCRIPT_FILE:
		syslog(LOG_INFO,"%s: PostScript - Unprintable data", pname);
		fprintf (output, "\n\n\tA PostScript file is not printable.\n\f");
		fflush (output);
		return(1);
		break;

	case ANSI_FILE:
		print_ansi_file();
		break;

	case XIMAGE_FILE:
		error = readXimghdr();
		if(error) {
			syslog(LOG_INFO,"%s: Failed to use image header", pname);
			fprintf (output, "\n\n\tFailed to use image header\n\f");
			fflush (output);
			return(1);
		}
		error = readXimgcmp();

		if(error) {
			syslog(LOG_INFO,"%s: Failed to use image colormap", pname);
			fprintf (output, "\n\n\tFailed to use image colormap\n\f");
			fflush (output);
			return(1);
		}
		if(im.format != ITS) {
			/* do RGB to YIQ conversion */
			tmpptr=cmpptr;
			for(i=0;i!=256;i++) {
				tmp=tmpptr;
				temp= *tmpptr * .30;
				tmpptr++;
				temp+= *tmpptr * .59;
				tmpptr++;
				temp+= *tmpptr * .11;
				tmpptr++;
				*tmp=(255-temp);
			}
		}
		error = readXimgdat();
		if(error) {
			syslog(LOG_INFO,"%s: Failed to use image data", pname);
			fprintf (output, "\n\n\tFailed to use image data\n\f");
			fflush (output);
			return(1);
		}
		switch(which_filter) {
		case ln03:
		case ln05:
		case ln06:
		case ln07:
		case ln08:
		case lg31:
		case lg02:
		case lg06:
		default:
			if(im.spbxnm > MAXP_PIX_WIDTH) {
				fprintf(output,"\033[11h\033[7 I");
				fprintf(output,"\033[?21 J");
				fprintf(output,"\033[?52l");
				fprintf(output,"\033[1;3150s");
				rotated++;
				if(im.spbxnm > MAXL_PIX_WIDTH) {
					im.spbxnm = MAXL_PIX_WIDTH;
				}
				if(im.spbynm > MAXL_PIX_WIDTH) {
					tmppagecount = im.spbynm/MAXP_PIX_WIDTH;
					tmplinecount = MAXP_PIX_WIDTH;
				}
			} else { /* im.spbxnm <= MAXP_PIX_WIDTH */
				fprintf(output,"\033[11h\033[7 I");
				fprintf(output,"\033[?20 J");
				fprintf(output,"\033[?52l");
				fprintf(output,"\033[1;2400s");
				if(im.spbynm > MAXP_PIX_HEIGHT) {
					tmppagecount = im.spbynm/MAXP_PIX_HEIGHT;
					tmplinecount = MAXP_PIX_HEIGHT;
				}
			}
			break;
		case la75:
			/* Now LA75 has only one mode...so go for that */
			if ( im.spbxnm > MAXP_PIX_WIDTH )
			    im.spbxnm =  MAXP_PIX_WIDTH;
			if ( im.spbynm > MAXP_PIX_HEIGHT ){
				tmppagecount = im.spbynm/MAXP_PIX_HEIGHT;
				tmplinecount = MAXP_PIX_HEIGHT;
			}
			break;
		case ansi_2of:
		case ansi_3of:
			break;
		}
		fprintf(output, fip->fi_sixel_intro);

		error = dosixel();
		if(error) {
			syslog(LOG_INFO,"%s: Failed to 'sixelize' data", pname);
			fprintf(output, "\033\\");
			fprintf(output, fip->fi_reset);
			fprintf (output, "\n\n\tFailed to 'sixelize' data\n\f");
			sleep(RESET_SLEEP_TIME) ;
			fprintf(output, "\014");
			fflush(output);
			return(1);
		}
		break;
	case TEXT_FILE:
	case CTEXT_FILE:
	case ATEXT_FILE:
	case RTEXT_FILE:
	case FTEXT_FILE:
	default:
		memset(buf, ' ', (size_t) sizeof(buf));
		done = 0;

		escflg = 0;	/* is escape/control sequence in progress? */
		globi = 0;
		while (!done) {
			col = indent;
			esclen = 0;
			maxrep = -1;
			linedone = 0;
			while (!linedone) {
				ch = globi < in ? filestorage[globi++] : getc(input);
				if (eschdl(output,ch)) /* deal with escape character */
				    if ( literal && is_8bit_cntrl(ch) &&
					(ch != '\n') && (ch != EOF)) {
					    cp = &buf[0][col]; /* Since literal mode..everything is the first row itself.*/
					    maxrep = 0;
					    if (is_7bit_cntrl(ch)){
						    if (ch == 0177){
							    *cp++ = '^';
							    *cp   = '?';
							    col ++;
						    } else{ /* It is < 040 */
							    *cp++ = '^';
							    *cp   = ch + '@';
							    col ++;
						    }
					    } else{ /* It is 8bit cntrl */
						    if ((unsigned char)ch < 0240){
							    *cp++ = 'M';
							    *cp++ = '-';
							    ch   &= 0177;
							    *cp   = ch + '@';
							    col  += 2;
						    } else {
							    *cp++ = 'M';
							    *cp++ = '-';
							    *cp   = '?';
							    col  += 2;
						    }
					    }
					    if ( col > maxcol[0] )
						maxcol[0] = col;
					    col++; /* col points to next blank entry in buf */
				    }
				    else { /* regular characters */
					    switch (ch) {
					    case EOF:
						    linedone = done = 1;
						    ch = '\n';
						    break;
  
					    case '\f': /* new page on form feed */
						    lineno = length;
					    case '\n': /* new line */
						    if (maxrep < 0)
							maxrep = 0;
						    linedone = 1;
						    break;
  
					    case '\b': /* backspace */
						    if (--col < indent)
							col = indent;
						    break;
  
					    case '\r': /* carriage return */
						    col = indent;
						    break;
  
					    case '\t': /* tab */
						    col = ((col - indent - esclen) | 07) ;
						    col = col + indent + esclen + 1;
						    break;
  
					    case '\031': /* end media */
						    /*
						     * lpd needs to use a different filter to
						     * print data so stop what we are doing and
						     * wait for lpd to restart us.
						     */
						    ch = globi < in ? filestorage[globi++] : getc(input);
						    if (ch == '\1') {
							    fflush(output);
							    kill(getpid(), SIGSTOP);
							    break;
						    } else {
							    if(globi <= in) {
								    globi--;
							    } else {
								    ungetc(ch, input);
							    }
							    ch = '\031';
						    }
  
					    default: /* everything else */
						    addtobuf(ch);
						    break;
					    } /* end switch */
				    } /* end else */

			}	/* end while not linedone */

			/* print out lines */
			for (i = 0; i <= maxrep; i++) {
				switch(which_filter) {
				case la75:
				case ansi_2of:
					if (i == 0) {
						break;
					} else {
						/* drop through */
					}
				case ln03:
				case ln05:
				case ln06:
				case ln07:
				case ln08:
				case lg31:
				case lg02:
				case lg06:
				case ansi_3of:
				default:
					putc('\r', output);
				}
				for (cp = buf[i], limit = cp+maxcol[i]; cp <= limit;) {
					putc(*cp, output);
					*cp++ = ' ';
				}
				/* On page boundary don't outupt new line char */
				if ((i == maxrep) && (lineno+1 < length))
				    putc(ch, output);
				maxcol[i] = -1;
			}
			if (++lineno >= length) {
				npages++;
				lineno = 0;
				if (length <= 66)
				    putc('\f',output); /* FF for length < 66 */
			}
		}
		if (lineno) {	/* If linecount > 0 count current page */
				/* Terminating ESC c will eject this page */
			npages++;
		}  
		fprintf(output,fip->fi_reset); /* reset printer defaults */
		fflush(output);	/* make sure reset goes out */
		sleep(RESET_SLEEP_TIME);/* some printers eat lines during reset so wait */
		break;
	}
	return(0);
}


/******************************************************************
* Prints a file already formatted with ansi control sequences.    *
*  The lf and cr options on the lg02 and lg06 should be disabled. *
*  Fixes a problem in lg02 when printing block characters in      *
*  landscape mode. This procedure sends file directly to printer, *
*  however, it adds a carraige return character for linefeed      *
*  character seen.                                                *
******************************************************************/
print_ansi_file ()
{
	int     done;
	register char ch;
	
	globi = 0;
	done = 0;
	while (!done) {
		ch = globi < in ? filestorage[globi++] : getc(input);
		switch (ch) {
			case EOF: 
				done = 1;
				break;
			case '\n':                   /* new line */
				putc('\r',output);   /* insert CR character */
				putc(ch, output);
				break;
			default:
				putc(ch, output);
				break;
		}
	}
	fprintf(output,fip->fi_reset);  /* reset printer defaults */
	fflush(output);		     /* make sure reset goes out  */
	sleep(RESET_SLEEP_TIME);     /* make sure the reset sets  */
}
					
/****************************************************************
  Adds the character specified to buffer - if it will fit on line
*****************************************************************/

addtobuf (ch)
register char      ch;
{
	register char  *cp;
        int  i;

        if ((col >= (width + esclen) || is_7bit_cntrl(ch)) && (ch != '\033')) {
		col++;
		return(0);
        }
        cp = &buf[0][col];
        for (i = 0; i < MAXREP; i++) {
		if (i > maxrep)
		    maxrep = i;
		if (*cp == ' ') {
			*cp = ch;
			if (col > maxcol[i])
			    maxcol[i] = col;
			break;
		}
		cp += BUFWIDTH;
        }
        col++;
        return(1);
}

/****************************************************************/
/*								*/
/*	eschdl - escape sequence handler			*/
/*								*/
/*	This routine intercepts escape sequences for the purpose*/
/*	of pass through.					*/
/*								*/
/* RETURNS 0 if in an escape sequence				*/
/* RETURNS 1 if the character is not in an escape sequence	*/
/****************************************************************/

/* 		Pass the following escape sequences			*/
/* ---------------------------------------------------------------------*/
/* Escape sequence introducer			Sequence terminator	*/
/*									*/
/* $[ or 233					chars @ -> ~		*/
/* $P or 220					$\ or 234		*/
/* $] or 235					$\ or 234		*/
/* $^ or 236					$\ or 234		*/
/* $_ or 237					$\ or 234		*/
/* $ 						any next character 	*/

/* escflg values are:							*/
/*	0 no escape sequence in process					*/
/*	1 for ESC character or 8bit escape sequence introducer		*/
/*	2 find terminator string					*/
/*									*/
/* esctrm values are:							*/
/*	1 find chars @ -> ~						*/
/*	2 $\ or 234							*/
/*	3 any character							*/


eschdl(o, ch)
FILE *o ;
char ch ;
{

 /* Look for escape character, for escape introducer */
 switch(escflg)
	{
	case 0:
		{
		switch(ch)
			{
			case '\033':	/* ESC */
				ch = (char)0 ;
				escflg=1 ;
				break ;
			case '\233':	/* ESC [ */
				escflg=2 ;
				esctrm=1 ;
				break ;
			case '\220':	/* ESC P */
				escpflg++;
			case '\235':	/* ESC ] */
			case '\236':	/* ESC ^ */
			case '\237':	/* ESC _ */
				escflg=2 ;
				esctrm=2 ;
				break ;
			case '\213':	/* plu */
			case '\214':	/* pld */
				break ;
			default:
				return(1) ;
			}
		break ;
		}
	/* Found an escape, test second character to find out what type */
	/* of escape sequence it is and set esctrm type */
	case 1:
		{
		if (ch == 'P')
			putc('\033', o) ;
		else
			{
			addtobuf('\033') ;
			esclen++;	
			}

		switch(ch)
			{
			case '[':	/* ESC [ */
				escflg=2 ;
				esctrm=1 ;
				break ;
			case 'P':	/* ESC P */
				escpflg++;
			case ']':	/* ESC ] */
			case '^':	/* ESC ^ */
			case '_':	/* ESC _ */
				escflg=2 ;
				esctrm=2 ;
				break ;
			default:
				escflg=0 ;
				break ;
			}
		break ;
		}
	case 2:/* Find terminator string */
		{
		switch(esctrm)
			{
			case(1):
				if((ch >= '@') && (ch <= '~'))
					escflg=0 ;
				break ;
			case(2): 	/* Look for ESC \ */
				if (ch == '\033')
					esctrm=3;/* Assume next char is '\' */
				else if (ch=='\234')
					escflg=0 ;
				break ;
			case(3):
				escflg=0;
				break;
			}
		}
	}

 if(escpflg && ch)
	{
	putc(ch, output) ;
	if(escflg==0)
		escpflg=0;
	}
 else
	{
	if(ch)
		{
		addtobuf(ch) ;
		esclen++ ;
		}
	}

 return(0);
}

/* 
 * If the file stream is of the XIMAGE_FILE type this routine is called
 * from filter().  The routine creates a sixel output stream to send
 * the printer based on the input data from an image file. Currently,
 * only a frame buffer image of raw red, green, blue points or a
 * GPX workstation "savimg"  image are valid data streams.  Sixel
 * output is initialized and then the processing begins.
 * The current design of the algorithm dithers the data to a 3 x 3 matrix
 * of points using the structure sixmap.
 */
dosixel()
{
	register int i;
	int xcnt,ycnt,n = 1,iindex,ij=0;
	unsigned char	*srcptr;
	unsigned char	*nxtptr;
	unsigned short sl;
	unsigned int	temp;
	struct sixmap *Sixmap;
	struct sixmap *Sixmap_offset;
	char *base_band,*pb;
	int base_band_length = 0;
	int count = 1;
	char lastc = 0;
	register char *cp;

	srcptr=(unsigned char *)imgptr;
	ycnt=im.spbynm;
	if(im.format != ITS)
	    nxtptr=srcptr+im.spbxnm * n;
	else
	    nxtptr=srcptr+(im.spbxnm * n * 3);
	pb = base_band = (char *)malloc((unsigned) 6 * im.spbxnm);
	Sixmap = base_sixmap;
	Sixmap_offset = offset_sixmap;
	while((ycnt/n) * n) {
		xcnt=im.spbxnm;
		base_band_length = 0;
		base_band = pb;
		while(xcnt>0) {
			if(MAXL_PIX_WIDTH - xcnt < 0) {
				if(im.format != ITS)
				    srcptr+=n;
				else
				    srcptr+=n*3;
				xcnt-=n;
				continue;
			}
			if(im.format != ITS) {
				sl = *(cmpptr + 3*(*srcptr));
			} else {
				/* need to convert to YIQ */
				temp= *srcptr++ * .30;
				temp+= *srcptr++ * .59;
				temp+= *srcptr * .11;
				temp=(255-temp);
				sl = (unsigned short)temp;
			}
			iindex = (sl * 9) / 255;
			for(i=0;i<3;i++) {
				*(base_band+i) = SOFF;
			}
			if(!ij) {
				*base_band++ = Sixmap[iindex].s0;
				*base_band++ = Sixmap[iindex].s1;
				*base_band++ = Sixmap[iindex].s2;
			} else {
				*base_band++ = Sixmap_offset[iindex].s0;
				*base_band++ = Sixmap_offset[iindex].s1;
				*base_band++ = Sixmap_offset[iindex].s2;
			}
			base_band_length+=3;
			srcptr+=n;
			xcnt-=n;
		}
		if(!ij) {
			cp = pb;
			lastc = *cp++;
			for(i=0;i < base_band_length;i++) {
				if(*cp != lastc) {
					if(count >= 4)
					    fprintf(output,"!%d%c",count,lastc);
					else
					    while(count--)putc(lastc,output);
					count = 1;
					lastc = *cp;
				} else count++;
				cp++;
			}
			fprintf(output,"$");
			count = 1;
			ij++;
		} else {
			cp = pb;
			lastc = *cp++;
			for(i=0;i < base_band_length;i++) {
				if(*cp != lastc) {
					if(count >= 4)
					    fprintf(output,"!%d%c",count,lastc);
					else
					    while(count--)
						putc(lastc,output);
					count = 1;
					lastc = *cp;
				} else
				    count++;
				cp++;
			}
			count = 1;
			ij = 0;
			fprintf(output,"-");
		}
		srcptr=nxtptr;
		if(im.format != ITS)
		    nxtptr=srcptr+im.spbxnm * n;
		else
		    nxtptr=srcptr+(im.spbxnm * n * 3);
		ycnt-= n;
	
		if(tmppagecount) {
			tmplinecount--;
			if(tmplinecount == 0) {
				fprintf(output,"+");
				tmppagecount--;
				if(im.spbxnm >= MAXL_PIX_WIDTH) {
					if(im.spbynm > MAXL_PIX_WIDTH) {
						tmplinecount = MAXP_PIX_WIDTH;
					}
				}
				if(im.spbxnm <= MAXP_PIX_WIDTH) {
					if(im.spbynm > MAXP_PIX_HEIGHT) {
						tmplinecount = MAXP_PIX_HEIGHT;
					}
				}
			}
		}
	}
	fprintf(output,"+");
	fprintf(output,"\033\\");
	return(0);
}

readXimghdr()
{
	register int tmp;

	size=HEDLEN*512;
	bcopy(filestorage+globi, (char *)&im, (int)size);
	globi = globi + size;
	if(im.imgflg != IMGFLG)
		return(1);
	if(im.format != QDSS || im.spbgnm != 1) {
		if(im.format != ITS)
			return(1);
	}
	if(im.format == ITS) {
		tmp = im.spbxnm;
		im.spbxnm = im.spbynm;
		im.spbynm = tmp;
		im.spblen = im.spblen * 3;
	}
	return(0);
}

readXimgcmp()
{
	if(im.format == ITS)
		return(0);
	size=im.cmplen*512;
	if(size==0)
		return(1);
	if((cmpptr=(short *) malloc(size))==NULL)
		return(1);
	bcopy(filestorage+globi, (char *)cmpptr, (int)size);
	globi = globi + size;
	return(0);
}

readXimgdat()
{
	size=im.spblen*512;
	if((imgptr=(char *) malloc(size))==NULL)
		return(1);
	bcopy(&filestorage[globi],imgptr,in-globi);
	fread(imgptr+(in-globi), (char)size-(in-globi), 1, input);
	return(0);
}
