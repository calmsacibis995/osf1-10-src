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
static char    *sccsid = "@(#)$RCSfile: fddi_config.c,v $ $Revision: 1.1.3.7 $ $Date: 1992/10/28 11:15:03 $";
#endif

/*
 *  This program is used to set various FDDI ANSI timers that make up
 *  the timed token protocol of FDDI. The FDDI timers are Request TRT (T_Req), 
 *  Valid Transmission Time (Tvx) and  Restricted Token Timeout. The
 *  LEM threshold also can be changed. The Ring purger mode can be turn
 *  on or off by this utility.
 *
 *  %fddi_config -iinterface  -t[vaule] -v[value] -r[value] -l[value]
 *   -p[1/0]   
 *
 */
#include <stdio.h>			/* standard I/O */
#include <errno.h>			/* error numbers */
#include <time.h>			/* time definitions */
#include <sys/types.h>			/* system types */
#include <sys/socket.h>			/* socket stuff */
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>			/* internet stuff */
#include <netinet/if_ether.h>		/* Ethernet interface structs */

extern	char	*optarg;
extern	int	optind;
extern	char	*sys_errlist[];

char	Usage[] = "\
Usage: %s -i[interface] [-option] \n\
	-t[value]  Token Request Time (range from 4 ms to 167.77208 ms) \n\
	-v[value]  Valid Transmit Time (range from 2.35 ms to 5.2224 ms) \n\
	-l[value]  Link Error Monitor Threshold (range from 5 to 8)\n\
	-r[value]  Restricted Token Timeout Time (range from 0 to 10000 ms) \n\
	-p[1/0]    Enable(1)/Disable(0) Ring Purger State\n\
	-x[1/0]    Enable(1)/Disable(0) Full Duplex Mode ('fta' interface only)\n\
	-c[value]  Counter Update Interval (value >= 0s ; 'fta' interface only)\n\
	-d	   Display the settable parameters value \n\
";

static char *ring_states[] = {
     "Purger off",
     "Candidate",
     "Non Purger",
     "Purger",
     NULL
};

/*
 * States if full duplex is enabled.
 */
static char *fdx_states[] = {
     "Unknown",
     "Idle",
     "Request",
     "Confirm",
     "Operational",
     NULL
};

main( argc, argv )
	int argc;
	char *argv[];
{
	struct	ifchar	ifchar;
	struct  ctrreq  ctr;	
	int s, i, c,type,display = 0;
	int errflg = 0, purger = -1 , interface = 0, cnt_interval = -1;
	int treq = -1 , tvx = -1 , lem = -1 , rtoken = -1, duplex_mode = -1;  
	double tmp, atof();

	strcpy( &ifchar.ifc_name[0], "fza0" );
	if(argc == 1 )
		goto usage;
	while ((c = getopt( argc, argv, "i:t:v:r:l:p:h:c:x:d" )) != EOF)  {
		switch(c) {
		case 'i':		/* interface */
			strcpy( &ifchar.ifc_name[0], optarg );
			strcpy( &ctr.ctr_name[0], optarg );
			interface = 1 ;
			break;
		case 't':		/* T_req */
			tmp = atof(optarg); 
			if((tmp > 173.015 ) || ( tmp < 4  )) {
				fprintf(stderr, "T_REQ value out of range ( 4.0 ms <= T_req <= 167.77208 ms ) \n");
				exit(1);
			}
			treq = (int) (tmp * 1000000 / 80) ; 
			display = 1;
			break;
		case 'v':		/* vtx */	
			tmp = atof(optarg); 
			if((tmp > 5.2224 ) || ( tmp < 2.35 )) {
				fprintf(stderr, "VTX value out of range ( 2.35 <= tvx <= 5.224 ms ) \n");
				exit(1);
			}
			tvx = (int) ( tmp * 1000000 / 80 ); 
			display = 1;
			break;
		case 'r':	/* Restricted Token */	
			rtoken = atoi(optarg); 
			if((rtoken > 10000 ) || ( rtoken < 0 )) {
				fprintf(stderr, "Restricted Token Timeout value out of range ( 0 <= R_token <= 10000 ms ) \n");
				exit(1);
			}
			rtoken =  rtoken * 1000000 / 80 ; 
			display = 1;
			break;
		case 'l':		/* lem */
			lem = atoi(optarg); 
			if((lem > 8 ) || ( lem  < 5 )) {

				fprintf(stderr, "LEM threshold value out of range ( 5 <= LEM <= 8 ) \n");
				exit(1);
			}
			display = 1;
			break;

		case 'p':		/* purger  */
			purger = atoi(optarg); 
			display = 1;
			break;
		case 'c':		/* Counter interval  */
			cnt_interval = atoi(optarg); 
			display = 1;
			break;
		case 'x':		/* Full duplex mode */
			duplex_mode = atoi(optarg);
			if (duplex_mode == 0)
				duplex_mode = FDX_DIS;
			display = 1;
			break;

		case 'd':
			display = 1;
			break;
		case 'h':
		default:		/* unrecognized option */
			goto usage;
			break;
		}
	}


	/*  we need a socket -- any old socket will do.  */
	s = socket( AF_INET, SOCK_DGRAM, 0 );
	if  (s < 0)  {
		perror( "socket" );
		exit(1);
	}

	/* do we need to change the value ? */
	if(treq > -1 || rtoken > -1 || tvx > -1 || 
		lem > -1 || purger > -1 || 
		cnt_interval > -1 || duplex_mode > -1) {
		ifchar.ifc_treq = treq;
		ifchar.ifc_rtoken = rtoken; 
		ifchar.ifc_tvx = tvx;
		ifchar.ifc_lem = lem;
		ifchar.ifc_ring_purger = purger ;
		ifchar.ifc_cnt_interval = cnt_interval;
		ifchar.ifc_full_duplex_mode = duplex_mode;
		if(ioctl( s, SIOCIFSETCHAR, &ifchar) < 0 ) {
					perror( &ifchar.ifc_name[0] );		
					exit(1);
		}   
	}

	/* if user supply the interface name and want to display */
	if(display && interface ) {
		ctr.ctr_type = FDDI_STATUS ;
		if(ioctl( s, SIOCRDCTRS, &ctr) < 0 ) {
				perror( &ctr.ctr_name[0] );		
				exit(1);
		}
		fprintf(stdout,"\n%s ANSI FDDI settable parameters \n\n",
			&ctr.ctr_name[0]);
		fprintf(stdout,"%-35s%-6.4f ms \n","Token Request Time:",
		  ctr.ctr_ctrs.status_fddi.t_req * 80.0 / 1000000.0 ); 
		fprintf(stdout,"%-35s%-6.4f ms \n","Valid Transmission Time:",
		  ctr.ctr_ctrs.status_fddi.tvx * 80.0 / 1000000.0 );
		fprintf(stdout,"%-35s%d \n","LEM Threshold:",
		  ctr.ctr_ctrs.status_fddi.lem_threshold);
		fprintf(stdout,"%-35s%-6.4f ms \n","Restricted Token Timeout:",
                  ctr.ctr_ctrs.status_fddi.rtoken_timeout * 80.0 / 1000000.0 );
		fprintf(stdout,"%-35s%s\n","Ring Purger State:",
		  ring_states[ctr.ctr_ctrs.status_fddi.ring_purge_state]);
		if (strncmp(ifchar.ifc_name, "fza", 3) != 0) {
			fprintf(stdout,"\n%s Full Duplex Mode: ", 		
				&ctr.ctr_name[0]); 
			if (ctr.ctr_ctrs.status_fddi.full_duplex_mode == 1) {
				fprintf(stdout,"Enabled\n");
				fprintf(stdout,"%-35s%s\n","Full Duplex State:",
		  			fdx_states[ctr.ctr_ctrs.status_fddi.full_duplex_state]);
			} else 
				fprintf(stdout,"Disabled\n");
			fprintf(stdout,"\n%s Counter Update Interval: %d sec\n", 
				&ctr.ctr_name[0], ctr.ctr_ctrs.status_fddi.cnt_interval);
		}
	} else
		goto usage; 

	close(s);
	exit(0);
usage:
	fprintf(stderr, Usage,argv[0]);
	exit(1);
}

