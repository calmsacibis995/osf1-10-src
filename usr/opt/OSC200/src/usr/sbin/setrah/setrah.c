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
static char *rcsid = "@(#)$RCSfile: setrah.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 16:12:32 $";
#endif

/*
 * setrah.c
 *
 * Modification history
 *
 * 04-Nov-91 - R. Craig Peterson
 *
 *	- Original submission
 *
 */

/* This program allows the super user to set the function of
 * CTRL-ALT-DEL (Reset), CTRL-ALT-RET (HALT), CTRL-SHIFT-ALT-DEL (Attn).
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <io/dec/dti/dti_hdr.h>

#undef DEBUG

#define DTI_NAME "/dev/dti0"

/* The I2C controller allows us to intercept various "signals".  These
   signals indicate some sort of user intervention, such as <CTRL>-<ALT>-<DEL>
   combinations on a keyboard to reset the machine.  There are three
   levels of these signals, and various options can be set by the software.

   16. Host Mode Register (CI_HSTM).

	The CI_HSTM control sets 1-byte of flags specifying how the I2Ctlr
	handles the Host's RESET and HALT lines.

	I2C Mode Flag		If Set
	-------------		------
	bit 0 HisHalt		Sig(Halt) asserts DebugIntr
	bit 1 RisReset		Sig(Reset) initiates HostReset  *1
	bit 2 RisAttn		Sig(Attn) initiates HostReset   *1
	bit 3 RDoesNMI		HostReset is asserted via P1.1  *2
	bit 4 RDly0		LSB, select reset-pending delay
	bit 5 RDly1		MSB, select reset-pending delay
	bit 6 *RFU
	bit 7 *RFU

	RDly1	RDly0	Delay Before Hard Reset
	bit 5	bit 4	(seconds)
	-----	-----	-----------------------
	0	0	none (Hard Reset is immediate)
	0	1	1
	1	0	5
	1	1	10

	*RFU = Reserved for Future Use, these bits MUST be 0.

	*1 HostReset may be initiated by either or both Sig(ATTN) or
	   Sig(RESET).  If both RisReset and RisAttn are 0, HostReset is
	   disabled.

	*2 For debugging purposes, HostReset may be set to assert the 
	   DebugIntr instead of a HardReset by setting RDoesNMI.  If set,
	   this "redirection" would apply in all cases where a HostReset was
	   generated (ie immediate or delayed).

	Default value is 0x03:
		Sig(Halt) asserts DebugIntr (HALT),
		Sig(Reset) asserts HardReset (RESET) immediately.

	Examples:

	To allow keyboard Halt (C-A-RET), allow keyboard Reset (C-A-DEL),
	and allow software to handle Reset with 5 second grace period:
		0x23

	Same as above, but with Halt (C-A-RET) disabled:
		0x22

   */

#define HisHalt	(1<<0)		/* Sig(Halt) asserts DebugIntr */
#define RisReset (1<<1)		/* Sig(Reset) initiates HostReset */
#define RisAttn (1<<2)		/* Sig(Attn) initiates HostReset */
#define RDoesNMI (1<<3)		/* HostReset is asserted via P1.1 */
#define RDly0 (1<<4)		/* LSB, select reset-pending delay */
#define RDly1 (1<<5)		/* MSB, select reset-pending delay */

main(argc, argv)
int argc;
char **argv;
{
    struct dti_msg msg;
    int	hstm_mask = DTI_SIG_MASK, c, fd;
    extern char *optarg;
    register char *p;

    while ((c = getopt_plus(argc, argv, "ahrd:")) != EOF)
	switch(c)
	{
	case 'a':		/* Attn does not cause host reset */
	    hstm_mask &= ~RisAttn;
	    break;

	case 'A':		/* Attn signal causes host reset */
	    hstm_mask |= RisAttn;
	    break;

	case 'h':		/* Halt signal off */
	    hstm_mask &= ~HisHalt;
	    break;

	case 'H':		/* Halt signal on */
	    hstm_mask |= HisHalt;
	    break;

	case 'r':		/* Reset signal off */
	    hstm_mask &= ~RisReset;
	    break;

	case 'R':		/* Reset signal on */
	    hstm_mask |= RisReset;
	    break;

	case 'd':		/* Reset delay */
	    switch(atoi(optarg))
	    {
	    case 0:
		hstm_mask &= ~(RDly0 | RDly1);
		break;

	    case 1:
		hstm_mask &= ~(RDly0 | RDly1);
		hstm_mask |= RDly0;
		break;

	    case 5:
		hstm_mask &= ~(RDly0 | RDly1);
		hstm_mask |= RDly1;
		break;

	    case 10:
		hstm_mask &= ~(RDly0 | RDly1);
		hstm_mask |= RDly0 | RDly1;
		break;

	    default:
		fprintf(stderr, "%s: delay must be 0, 1, 5, or 10 seconds\n",
			argv[0]);
		exit(-2);
	    }
	    break;

	case '?':
	default:
	    fprintf(stderr, "%s: usage: %s [-r|-a|-h|+r|+a|+h|-d[ ]value]\n", argv[0], argv[0]);
	    fprintf(stderr, "\t-r\tRESET signal is ignored\n");
	    fprintf(stderr, "\t+r\tRESET signal causes complete host reset\n");
	    fprintf(stderr, "\t-a\tATTN does not cause host reset\n");
	    fprintf(stderr, "\t+a\tATTN causes host reset\n");
	    fprintf(stderr, "\t-h\tHALT signal ignored\n");
	    fprintf(stderr, "\t+h\tHALT signal causes jump into console ROM\n");
	    fprintf(stderr, "\t-d[ ]value\tindicates how many seconds to wait before hard reset\n");
	    fprintf(stderr, "\t\twhere value is one of 0, 1, 5, or 10 seconds\n");
	    fprintf(stderr, "\n\tDefault is +h +r -d5 (any options modify this default --\n");
	    fprintf(stderr, "\tit is always used as a base.)\n");
	    exit(-3);
	}

#ifdef DEBUG
    printf("%#x\n", hstm_mask);
#endif

    if ((fd = open(DTI_NAME, 0)) == -1)
    {
	perror(DTI_NAME);
	exit(-1);
    }

    if (hstm_mask & RisAttn)
	printf("Attn signal causes host reset.\n");

    if (hstm_mask & HisHalt)
	printf("Halt signal jumps to console ROM.\n");
    else
	printf("Halt ignored.\n");

    if (hstm_mask & RisReset)
    {
	int	time = 0;

	printf("Reset signal resets machine ");
	switch((hstm_mask & (RDly0 | RDly1)) >> 4)
	{
	case 0:
	    printf("immediately.\n");
	    break;
	case 1:
	    time = 1;
	    break;
	case 2:
	    time = 5;
	    break;
	case 3:
	    time = 10;
	    break;
	}
	if (time)
	    printf("in %d seconds if machine is completely hung.\n",
		   time);
    }
    else
	printf("Reset ignored.\n");

    
    if (ioctl(fd, DTI_SET_HSTM_MASK, &hstm_mask) == -1)
    {
	perror(argv[0]);
	exit(-1);
    }

    close(fd);
    exit(0);
}

/*	@(#)getopt.c	1.5	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#define NULL	0
#define EOF	(-1)
#define ERR(s, c)	if(opterr){\
	extern int strlen(), write();\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
	(void) write(2, s, (unsigned)strlen(s));\
	(void) write(2, errbuf, 2);}

extern int strcmp();
extern char *strchr();

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

int
getopt_plus(argc, argv, opts)
int	argc;
char	**argv, *opts;
{
    static int sp = 1;
    register int c;
    register char *cp;

    if(sp == 1)
	if(optind >= argc ||
	   (argv[optind][0] != '-' && argv[optind][0] != '+')
	   || argv[optind][1] == '\0')
	    return(EOF);
	else if(strcmp(argv[optind], "--") == NULL)
	{
	    optind++;
	    return(EOF);
	}
    optopt = c = argv[optind][sp];
    if(c == ':' || (cp=strchr(opts, c)) == NULL)
    {
	ERR(": illegal option -- ", c);
	if(argv[optind][++sp] == '\0')
	{
	    optind++;
	    sp = 1;
	}
	return('?');
    }
    if (argv[optind][0] == '+')
	c = toupper(c);
    if(*++cp == ':') {
	if(argv[optind][sp+1] != '\0')
	    optarg = &argv[optind++][sp+1];
	else if(++optind >= argc)
	{
	    ERR(": option requires an argument -- ", c);
	    sp = 1;
	    return('?');
	} else
	    optarg = argv[optind++];
	sp = 1;
    } else {
	if(argv[optind][++sp] == '\0')
	{
	    sp = 1;
	    optind++;
	}
	optarg = NULL;
    }
    return(c);
}
