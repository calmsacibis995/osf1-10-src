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
static char *rcsid = "@(#)$RCSfile: autopush.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/10/08 16:14:07 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1990  Mentat Inc.
 ** autopush.c 2.3, last change 5/1/91
 **/

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <ctype.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <sys/stream.h>
#define	staticf	static
#include <stropts.h>
#include <sys/sad.h>
#include <sys/types.h>

#include <nl_types.h>
#include <locale.h>
#include "autopush_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_AUTOPUSH,Num,Str)
nl_catd catd;

#ifndef ADMINDEV
#define ADMINDEV	"/dev/sad/admin"
#define USERDEV		"/dev/sad/admin"
#endif


staticf int	get_ap_info(   int dmajor, int dminor, struct strapush * infop   );
staticf char	* get_cp(   char * buf, char * name   );
staticf int	get_mm(   int narg, char ** arglist, int * majp, int * minp   );
staticf int	get_next_arg(   char * string1, char * string2, char ** result   );
staticf int	load_ap_modules(   struct strapush * infop   );
staticf void	open_sad_driver(   void   );
staticf void	read_ap_modlist(   FILE * apf   );
staticf int	remove_ap_modules(   int dmajor, int dminor   );
staticf void	show_ap_info(   struct strapush * infop   );
staticf void	show_ap_modules(   int dmajor,  int dminor   );
staticf int	split_line(   char * buf, struct strapush * infop   );
staticf	int	str_to_major(   char * string   );

	int	Fds;			/* file descriptor for stream SAD driver */
	int	Line_count = 0;		/* line count for -f filename input error msgs */

char	*usage_text;

/*------------------------------------------------------------------------------------*/


/* Mentat library routines pulled inline */

staticf	char	* program_name;

staticf void
set_program_name (name)
	char	* name;
{
	program_name = name;
}

#define get_program_name()	program_name

staticf void
usage (str)
	char	* str;
{
	if (!str  ||  !*str)
		str = "\n";
	else if (strncmp("usage:", str, 6) == 0)
		(void)fprintf(stderr, "%s", str);
	else
		(void)fprintf(stderr, MSGSTR(APUSH_USE1, "usage: %s %s"),
		    get_program_name(), str);
	while (*str)
		str++;
	if (str[-1] != '\n')
		(void)fprintf(stderr, "\n");
	exit(1);
	/*NOTREACHED*/
}

#include <stdarg.h>

staticf void
warn (char *fmt, ... )
{
	va_list	ap;

	if (fmt  &&  *fmt) {
		va_start(ap, fmt);
		(void)fprintf(stderr, "%s: ", get_program_name());
		(void)vfprintf(stderr, fmt, ap);
		va_end(ap);
		while (*fmt)
			fmt++;
		if (fmt[-1] != '\n')
			(void)fprintf(stderr, "\n");
	}
}

staticf void
fatal (char *fmt, ... )
{
	va_list	ap;

	if (fmt  &&  *fmt) {
		va_start(ap, fmt);
		(void)fprintf(stderr, "%s: ",
		    get_program_name());
		(void)vfprintf(stderr, fmt, ap);
		va_end(ap);
		while (*fmt)
			fmt++;
		if (fmt[-1] != '\n')
			(void)fprintf(stderr, "\n");
	}
	exit(1);
	/*NOTREACHED*/
}

staticf char *
errmsg (err)
	int	err;
{
	static	char	buf[40];

	if (err  ||  (err = errno)) {
		if (err > 0  &&  err < sys_nerr  &&  sys_errlist[err])
			return sys_errlist[err];
	}
	if (err)
		(void)sprintf(buf, MSGSTR(APUSH_ENUMB, "error number %d"), err);
	else
		(void)sprintf(buf, MSGSTR(APUSH_USPERR, "unspecified error"));
	return buf;
}

/* End of Mentat library routines */

staticf void
open_sad_driver ()
{
	/* TODO: need os-dependent privileged user check */
	if (getuid() == 0) {
		if ((Fds = open(ADMINDEV, O_RDWR)) == -1) {
			fatal(MSGSTR(APUSH_SADOP, "SAD open failed for %s\n"),
			    ADMINDEV);
		}
	} else {
		if ((Fds = open(USERDEV, O_RDWR)) == -1) {
			fatal(MSGSTR(APUSH_SADOP, "SAD open failed for %s\n"),
			    USERDEV);
		}
	}
}


/*------------------------------------------------------------------------------------*/

staticf int
get_ap_info (dmajor, dminor, infop)
	int		dmajor, dminor;
	struct strapush	* infop;
{
	int	ioerr;
	
	infop->sap_major = dmajor;
	infop->sap_minor = dminor;
	if ((ioerr = ioctl(Fds, SAD_GAP, infop)) < 0) {
		switch (errno) {
		case EFAULT:
			warn(MSGSTR(APUSH_PRBUG,
			    "Program bug - bad arg/infop (%p)\n"), infop);
			break;

		case EINVAL:
			warn(MSGSTR(APUSH_INMAJ, "Invalid major (%d)\n"),
			    infop->sap_major);
			break;

		case ENOSTR:
			warn(MSGSTR(APUSH_MAJNST,
			    "Major (%d) is not a STREAMS device\n"),
			    infop->sap_major);
			break;

		case ENODEV:
			warn(MSGSTR(APUSH_DNOTAT,
			    "Device (%d,%d) not configured for autopush\n"),
			    infop->sap_major, infop->sap_minor);
			break;

		default:
			warn(MSGSTR(APUSH_UNXIOCT,
			    "Unexpected IOCTL(SAD_GAP) error: %d\n"), errno);
			break;
		}
	}
	return ioerr;
}

/*------------------------------------------------------------------------------------*/

staticf void
show_ap_info (infop)
	struct strapush	* infop;

{
	int	k;
	
	switch (infop->sap_cmd) {
	case SAP_ONE:
	case SAP_RANGE:
	case SAP_ALL:
		printf(MSGSTR(APUSH_HDR1,
		    "Major Minor Last Minor  %2d module(s)\n"),
		    infop->sap_npush);
		printf(MSGSTR(APUSH_LINE1,
		    "----- ----- ----------  ------------\n"));
		printf("%3d   ", infop->sap_major);
		if (infop->sap_cmd == SAP_ALL) {
			printf(MSGSTR(APUSH_SPALL, " ALL   "));
		} else {
			printf("%3d    ", infop->sap_minor);
		}
		if (infop->sap_cmd == SAP_RANGE) {
			printf("  %3d    ", infop->sap_lastminor);
		} else {
			printf(MSGSTR(APUSH_LITLIN, "  ---    "));
		}
		if (infop->sap_npush <= MAXAPUSH) {
			for (k = 0; k < infop->sap_npush; k++) {
	     			printf("  %s", infop->sap_list[k]);
	     		}
	     	}
		printf("\n");
	     	break;
	
	default:
		warn(MSGSTR(APUSH_INFOP, "infop->sap_cmd = %d\n"),
		    infop->sap_cmd);
		break;
	}
	return;
}

/*------------------------------------------------------------------------------------*/

staticf void
show_ap_modules (dmajor, dminor)
	int	dmajor, dminor;
{
	struct strapush	ap_info;
	
	if (get_ap_info(dmajor, dminor, &ap_info) == 0) {
		show_ap_info(&ap_info);
	}
	return;
}

/*------------------------------------------------------------------------------------*/

staticf int
split_line (buf, infop)
	char		* buf;
	struct strapush	* infop;

	/*
	 * buf is a line of the form: maj_ min_ last_min_ mod1 mod2 ... # comment
	 * Returns # of items before #, or -1 if error.  If #>0, *infop is filled with
	 * values found.
	 */
{
	char	* next;
	int	nfield = 0;		/* number of fields found */
	
	if ((next = strchr(buf, '#')) == 0) {
		next = strchr(buf, '\0');
	}
	*next = '\0';
	
	next = strtok(buf, " \n\t,");
	while (next != 0) {
		if (++nfield == 1) {
			infop->sap_major = str_to_major(next);
		} else if (nfield == 2) {
			infop->sap_minor = atol(next);
		} else if (nfield == 3) {
			infop->sap_lastminor = atol(next);
		} else if (nfield <= 3 + MAXAPUSH) {
			(void)strncpy(infop->sap_list[nfield-4], next, FMNAMESZ+1);
		} else {
			warn(MSGSTR(APUSH_MORMOD,
			    "More than %d modules in line %d.  Entry ignored\n"),
					MAXAPUSH, Line_count);
			fprintf(stderr, MSGSTR(APUSH_LINERR,
			    "\tLine in error: %.40s ...\n"), buf);
			next = 0;
			nfield = -1;
		}
		next = strtok((char *)0, " \n\t,");
	}
	if (!(nfield))
		return (0);
	infop->sap_npush = ( (nfield > 3) ? nfield - 3 : 0 );

	if (infop->sap_minor == -1) {
		infop->sap_cmd = SAP_ALL;
	} else if (infop->sap_lastminor == 0) {
		infop->sap_cmd = SAP_ONE;
	} else 	if (infop->sap_minor < infop->sap_lastminor) {
		infop->sap_cmd = SAP_RANGE;
	} else {
		warn(MSGSTR(APUSH_INMAJMIN,
	"Inconsistent Major (%d), Minor (%d), Last_minor (%d) at line %d\n"), 
			infop->sap_major, infop->sap_minor, infop->sap_lastminor, Line_count);
		nfield = -1;
	}
	return nfield;
}

/*------------------------------------------------------------------------------------*/

staticf int
load_ap_modules (infop)
	struct strapush	* infop;
	
{
	struct str_list namelist;
	int		k;
	int		ioerr, ioerr2;
	int		saved_errno;
	char		* sap_cmd_text;
		
	if (ioerr = ioctl(Fds, SAD_SAP, infop) < 0) {
		warn(MSGSTR(APUSH_CNPSH,
		    "Can't push requested modules on STREAM for entry %d\n"),
		    Line_count);
		switch (saved_errno = errno) {

		case EINVAL:
			/*
			 * Validate module names, since otherwise user must re-run
			 * autopush for each device to get list of ok names.  Since
			 * error code is ambiguous, must check all drivers in list first.
			 * If all are ok, then error is major device number; otherwise
			 * if one of the module is bad, we show entire table.
			 */
			 
			if (infop->sap_npush <= 0 || infop->sap_npush > MAXAPUSH) {
				warn(MSGSTR(APUSH_ONETO,
				"1 to %d module names required at line %d\n"),
				MAXAPUSH, Line_count);
			} else {
				namelist.sl_nmods = infop->sap_npush;
				namelist.sl_modlist = (struct str_mlist *)infop->sap_list[0];
				if ((ioerr2 = ioctl(Fds, SAD_VML, &namelist)) == 0) {
					warn(MSGSTR(APUSH_INMN,
					    "Invalid major (%d)\n"),
					    infop->sap_major);
				} else if (ioerr2 = 1) {
					fprintf(stderr, MSGSTR(APUSH_MODAVAL,
					    "\nModule          Available\n"));
					fprintf(stderr, MSGSTR(APUSH_LN1,
					    "------          ---------\n"));
					for (k = 0; k < infop->sap_npush; k++) {
						namelist.sl_nmods = 1;
						namelist.sl_modlist = (struct str_mlist *)infop->sap_list[k];
						fprintf(stderr, " %-16s",
						    infop->sap_list[k]);
						if(ioctl(Fds, SAD_VML, &namelist) == 0) {
							fprintf(stderr,
							    MSGSTR(APUSH_SYES,
							    "   Yes\n"));
						} else {
							fprintf(stderr,
							    MSGSTR(APUSH_SNO,
							    "   No\n"));
						}
					}
				} else {
					warn(MSGSTR(APUSH_UXIOC,
	"Unexpected IOCTL(SAD_VML) error (%d) checking module list.\n"), errno);
				}
			}
			break;
			
		case EEXIST:
			warn(MSGSTR(APUSH_DEVCON,
			    "Device (%d,%d) already configured\n"),
			    infop->sap_major, infop->sap_minor);
			break;
			
		case EFAULT:
			warn(MSGSTR(APUSH_PRGMB,
			    "Program bug - bad arg/infop (%p)\n"), infop);
			break;

		case ENOSTR:
			warn(MSGSTR(APUSH_MAJNSTR,
			    "Major (%d) is not a STREAMS device\n"),
			    infop->sap_major);
			break;

		case ENODEV:
			warn(MSGSTR(APUSH_DNOAUTO,
			    "Device (%d,%d) not configured for autopush\n"),
			    infop->sap_major, infop->sap_minor);
			break;

		case ERANGE:
			warn(MSGSTR(APUSH_MINNOT,
			    "Minor (%d) not first number in configured range\n"),
			    infop->sap_minor);
			break;

		case ENOSR:
			warn(MSGSTR(APUSH_NOMAUTO,
			    "No memory for autopush data structure\n"));
			break;

		default:
			switch (infop->sap_cmd) {
			case SAP_ONE:
				sap_cmd_text = MSGSTR(APUSH_ONE, "ONE");
				break;
			case SAP_RANGE:
				sap_cmd_text = MSGSTR(APUSH_RANGE, "RANGE");
				break;
			case SAP_ALL:
				sap_cmd_text = MSGSTR(APUSH_ALL, "ALL");
				break;
			default:
				sap_cmd_text = MSGSTR(APUSH_QUES, "???");
				break;
			}
			warn(MSGSTR(APUSH_UNXSAD,
			    "Unexpected IOCTL(SAD_SAP/%s) error: %d\n"),
			    sap_cmd_text, saved_errno);
			break;
		}
	}
	return ioerr;
}

/*------------------------------------------------------------------------------------*/

staticf void
read_ap_modlist (apf)
	FILE	* apf;
	
{
	char		buf[256];
	int		nitem;
	struct strapush	info;
	
	while (fgets(buf, sizeof buf, apf) != 0) {
		Line_count++;
		nitem = split_line(buf, &info);
		if (nitem > 0 ) {
			if (load_ap_modules(&info) < 0) {
				fprintf(stderr, MSGSTR(APUSH_CONTI,
				    "Continuing...\n"));
			}
		} else if (nitem < 0) {
			break;
		}
	}
	return;
}

/*------------------------------------------------------------------------------------*/

staticf int
remove_ap_modules (dmajor, dminor)
	int	dmajor, dminor;
{
	struct strapush	ap_info;
	int		ioerr;
	
	ap_info.sap_major = dmajor;
	ap_info.sap_minor = dminor;
	ap_info.sap_cmd = SAP_CLEAR;
	if (ioerr = ioctl(Fds, SAD_SAP, &ap_info) < 0) {
		switch (errno) {

		case EFAULT:
			warn(MSGSTR(APUSH_PBGARG,
			    "Program bug - bad arg/infop (%p)\n"), &ap_info);
			break;

		case EINVAL:
			warn(MSGSTR(APUSH_CONALL,
	 "Major (%d) configured for ALL minors. Use \"-m0\" for minor.\n"),
			    ap_info.sap_major);
			break;

		case ENOSTR:
			warn(MSGSTR(APUSH_NSTRDEV,
			    "Major (%d) is not a STREAMS device\n"),
			    ap_info.sap_major);
			break;

		case ENODEV:
			warn(MSGSTR(APUSH_DNOCNF,
			    "Device (%d,%d) not configured for autopush\n"),
			    ap_info.sap_major, ap_info.sap_minor);
			break;

		case ERANGE:
			warn(MSGSTR(APUSH_CONFRAN,
			   "Minor (%d) not first number in configured range\n"),
			    ap_info.sap_minor);
			break;

		case ENOSR:
			warn(MSGSTR(APUSH_NOSTR,
			    "No memory for autopush data structure\n"));
			break;

		default:
			warn(MSGSTR(APUSH_SADN,
			    "Unexpected IOCTL(SAD_SAP/CLEAR) error: %d\n"),
			    errno);
			break;
		}
	}
	return ioerr;
}

/*------------------------------------------------------------------------------------*/

staticf int
get_next_arg (string1, string2, result)
	char	* string1, * string2, ** result;
	
	/* string1 & string2 are consecutive elements of argv[]. We expect string1 to
	 * be of the form "-x...", a 1-character switch.  If string1 is longer than
	 * 2 characters, remainder of string is returned as result; otherwise string2 is
	 * returned as result (no checks).  Function returns:
	 *		-1   cmd error
	 *		 1   only string1 used
	 *		 2   both string1 & string2 used
	 */
{
	int	ret_code = -1;
	int	nch;
	
	if (*string1 == '-') {
		if ((nch = (int)strlen(string1)) > 2) {
			*result = string1 + 2;
			ret_code = 1;
		} else if (nch == 2) {
			*result = string2;
			ret_code = 2;
		}
	}
	return ret_code;
}

/*------------------------------------------------------------------------------------*/

staticf int
get_mm (narg, arglist, majp, minp)
	int	narg;
	char	* arglist[];
	int	* majp, * minp;

	/*
	 * Looking for segment of command line "-M major -m minor" where order
	 * doesn't matter, and "-m minor" may be missing (returned as -1 ==> all)
	 */
{
	int	maj_found, min_found;
	int	karg1, karg2, knext;
	int	ret_code = 0;
	char	* result;
	
	
	maj_found = 0;
	min_found = 0;
	*minp = -1;
	*majp = -1;
	karg1 = 2;
	while (karg1 < narg) {
		karg2 = (karg1 < narg - 1) ? karg1 + 1 : karg1;
		if ((knext = get_next_arg(arglist[karg1], arglist[karg2], &result)) == -1) {
			ret_code = -1;
			break;
		} else {
			switch (arglist[karg1][1]) {

			case 'm':
				if (min_found) {
					ret_code = -1;
				} else {
					*minp = atol(result);
					min_found = 1;
				}
				break;

			case 'M':
				if (maj_found) {
					ret_code = -1;
				} else {
					*majp = str_to_major(result);
					maj_found = 1;
				}
				break;

			default:
				ret_code = -1;
				break;
			}
		}
		karg1 += knext;
	}
	if (*majp == -1 || *minp == -1) {
		ret_code = -1;
	}
	return (ret_code);
}

staticf int
str_to_major (string)
	char	* string;
{
	char	* cp;
	char	info_buf[128];
	int	fd;
	struct strioctl	stri;

	if ( isdigit(string[0]) )
		return atol(string);

}

staticf char *
get_cp (buf, name)
	char	* buf;
	char	* name;
{
	char	* cp;
	int	name_len;

	name_len = strlen(name);
	for (cp = buf; cp[0]  &&  cp[1]; cp += (strlen(cp)+1)) {
		if (cp[0] == name[0]  &&  strncmp(cp, name, name_len) == 0) {
			if (cp = strchr(cp, '='))
				cp += 3;
			return cp;
		}
	}
	return 0;
}

/********************************** M A I N ******************************************/


main (argc, argv)
	int	argc;
	char	* argv[];
	
	/* extract command line arguments:
	 *
	 *   -f filename                or
	 *   -r -M major -m minor       or
	 *   -g -M major -m minor  
	 *
	 * -f, -r, -g must appear first; -M, -m can appear in either order
	 *
	 */
	 
{
	FILE		* apf;
	int		dmajor, dminor;			/* major,minor from cmd line */
	int		exit_code = 0;
	char		* filename;
	struct strapush	info;
	
	setlocale(LC_ALL,"");
	catd = catopen (MF_AUTOPUSH, NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, MSGSTR(APUSH_AUTH,
    		    "autopush: need sysadmin authorization\n"));
		exit(1);
	}
	if (forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_OWNER,
#if SEC_MAC
	    SEC_ALLOWMACACCESS,
#endif
	    -1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(APUSH_PRIV,
		    "autopush: insufficient privileges\n"));
		exit(1);
	}
#endif

	usage_text = MSGSTR(APUSH_USAGE,
"-f filename\n       autopush -r -M major -m minor\n       autopush -g -M major -m minor");

	set_program_name(argv[0]);
	open_sad_driver();

	if (argc < 2 || argv[1][0] != '-') {
		close(Fds);
		usage(usage_text);
	}
	switch (argv[1][1]) {
	case 'f':
		if (argc > 3 || (argc -= get_next_arg(argv[1], argv[argc-1], &filename)) != 1) {
			usage(usage_text);
			exit_code = 1;
		} else {
	 		if ((apf = fopen(filename, "r")) == 0) {
				close(Fds);
	 			fatal(MSGSTR(APUSH_NOPENF,
				    "Can't open file: %s\n"), filename);
			} else {
				read_ap_modlist(apf);
			}
	 	}
		break;
		
	case 'r':
		if (get_mm(argc, argv, &dmajor, &dminor) == 0) {
			(void)remove_ap_modules(dmajor, dminor);
		} else {
			usage(usage_text);
			exit_code = 1;
		}
		break;

	case 'g':
		if (get_mm(argc, argv, &dmajor, &dminor) == 0) {
			show_ap_modules(dmajor, dminor);
		} else {
			usage(usage_text);
			exit_code = 1;
		}
		break;
		
	default:
		usage(usage_text);
		exit_code = 1;
		break;
	}
	close(Fds);
	exit(exit_code);
}
