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
static char	*sccsid = "@(#)$RCSfile: cntr.c,v $ $Revision: 4.3.9.3 $ (DEC) $Date: 1993/07/08 23:05:49 $";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*	Modification History
 *
 *	1-23-91 -- Uttam Shikarpur 
 *	Made some minor changes to the output format.
 *	Added copyright notice.
 *
 */

#include <stdio.h>			/* standard I/O */
#include <errno.h>			/* error numbers */
#include <time.h>			/* time definitions */
#include <sys/types.h>			/* system types */
#include <sys/socket.h>			/* socket stuff */
#include <sys/ioctl.h>			/* ioctls */
#include <net/if.h>			/* generic interface structs */
#include <netinet/in.h>			/* internet stuff */
#include <netinet/if_ether.h>		/* Ethernet interface structs */


extern char *interface;
extern int unit;

if_cntrs()
{
	struct ifreq *ifr, ifreq, ifreqs[32];
	struct ifconf ifc;
	int s, i;

	if (interface) {
		char ifname[IFNAMSIZ];
		sprintf(ifname, "%s%d", interface, unit);
		if_do_cntrs(ifname, 1);
		return;
	}

	/*  we need a socket -- any old socket will do.  */
	s = socket( AF_INET, SOCK_DGRAM, 0 );
	if (s < 0) {
		perror( "socket" );
		exit(1);
	}

	ifc.ifc_req = ifreqs;
	ifc.ifc_len = sizeof(ifreqs);
	if (ioctl(s, SIOCGIFCONF, &ifc) < 0) {
		perror("siocgifconf");
		exit(1);
	}
	(void) close(s);

	for (ifr = ifreqs; ifc.ifc_len > 0; ifr++, ifc.ifc_len -= sizeof(*ifr)) {
		if (strcmp(ifreq.ifr_name, ifr->ifr_name)) {
			(void) strcpy(ifreq.ifr_name, ifr->ifr_name);
			if_do_cntrs(ifreq.ifr_name, 0);
		}
	}
}

if_do_cntrs(ifname, print_errors)
	char *ifname;
	int print_errors;
{
	int s, zero = 0;
	struct ctrreq ctrreq;

	strcpy(ctrreq.ctr_name, ifname);
	/*  we need a socket -- any old socket will do.  */
	s = socket( AF_INET, SOCK_DGRAM, 0 );
	if (s < 0) {
		if (print_errors) perror( "socket" );
		return;
	}

	/*
	 *	Read the counter for the interface.  If the user asked to
	 *	zero the counters and we aren't super user, just read the
	 *	counters and the user later.
	 */
	if (ioctl( s, SIOCRDCTRS , &ctrreq ) < 0)  {
		if  (zero && errno == EPERM)  {
			zero = -1;
			if (ioctl( s, SIOCRDCTRS, &ctrreq ) < 0)  {
				if (print_errors) fprintf( stderr, 
					"%s: SIOCRDCTRS: %s\n", 
					&ctrreq.ctr_name[0],
					strerror(errno) );
				return;
			}
		} else {
			if (print_errors) fprintf( stderr, 
				"%s: %s: %s\n", 
				&ctrreq.ctr_name[0],
				"SIOCRDCTRS",
				strerror(errno) );
			return;
		}
	}

	switch (ctrreq.ctr_type) {
	    case CTR_ETHER:
		display_estats( stdout, 	/* display the counters */
			ctrreq.ctr_name,
			&ctrreq.ctr_ctrs.ctrc_ether,
			zero );
		break;
	    case CTR_DDCMP:
		display_dstats( stdout,
			ctrreq.ctr_name,
			&ctrreq.ctr_ctrs.ctrc_ddcmp,
			zero );
		break;
#ifdef CTR_FDDI
	    case CTR_FDDI:
		display_fstats( stdout,
			ctrreq.ctr_name,
			&ctrreq.ctr_ctrs.ctrc_fddi,
			zero );
		ctrreq.ctr_type = FDDI_STATUS ;
		if (ioctl( s, SIOCRDCTRS, &ctrreq ) < 0)  {
			if (print_errors) fprintf( stderr, 
				"%s: SIOCRDCTRS: %s\n", 
				&ctrreq.ctr_name[0],
				strerror(errno) );
			return;
		} else 
			display_fstatus( stdout,ctrreq.ctr_name,
				&ctrreq.ctr_ctrs.status_fddi);
		break;
#endif
	    case CTR_TRN:
		display_trncount( stdout,
			ctrreq.ctr_name,
			&ctrreq.ctr_ctrs.trncount,
			zero );
		ctrreq.ctr_type = TRN_CHAR;
		if (ioctl( s, SIOCRDCTRS, &ctrreq ) < 0)  {
			if (print_errors) fprintf( stderr, 
				"%s: SIOCRDCTRS: %s\n", 
				&ctrreq.ctr_name[0],
				strerror(errno) );
			return;
		} else 
			display_trnchars(stdout,ctrreq.ctr_name,
				&ctrreq.ctr_ctrs.trnchar);
		break;

	}
	close(s);
}

display_address( outfile, devea )
	FILE *outfile;
	struct ifdevea *devea;
{
	fprintf( outfile, "\n%s default address: ", &devea->ifr_name[0] );
	fprintf( outfile, "%02X-%02X-%02X-%02X-%02X-%02X",
		devea->default_pa[0],		devea->default_pa[1],
		devea->default_pa[2],		devea->default_pa[3],
		devea->default_pa[4],		devea->default_pa[5] );
	fprintf( outfile, ", current address: %02X-%02X-%02X-%02X-%02X-%02X",
		devea->current_pa[0],		devea->current_pa[1],
		devea->current_pa[2],		devea->current_pa[3],
		devea->current_pa[4],		devea->current_pa[5] );
	fprintf( outfile, "\n" );
}


display_reasons(outfile, fmt, ctr, bitmask, failures)
    FILE *outfile;
    char *fmt;
    int ctr;
    int bitmask;
    char **failures;
{
    register int i;
    fprintf( outfile, fmt, ctr );
    if (ctr > 0)  {
	fprintf( outfile, ", reasons include:\n" );
	for(i=0; bitmask != 0; i++)  {
	    if (failures && failures[i] == NULL)
		failures = NULL;
	    if  (bitmask & 1)  {
		if (!failures)  {
		    fprintf( outfile, "\t\tUnknown reason #%d\n", i);
		} else {
		    fprintf( outfile, "\t\t%s\n", failures[i]);
		}
	    }
	    bitmask >>= 1;
	}
    } else {
	fprintf( outfile, "\n");
    }
}

static	char	*ether_sendfails[] = {
    "Excessive collisions",
    "Carrier check failed",
    "Short circuit",
    "Open circuit",
    "Frame too Long",
    "Remote failure to defer",
    NULL
};

static	char	*ether_recvfails[] = {
    "Block check error",
    "Framing Error",
    "Frame too long",
    NULL
};


display_estats( outfile, ifname, estats, flag )
	FILE *outfile;
	char ifname[];
	struct estat *estats;
	int flag;
{
	int i;
	time_t now = time(0);

	fprintf( outfile, "\n%s Ethernet counters at %s",
			ifname, ctime(&now) );
	if  (flag == -1)  {
		errno == EPERM;
		perror( "Warning: failed to zero counters" );
	}
	fprintf( outfile, "\n");
	fprintf( outfile, "%12u seconds since last zeroed\n",
						estats->est_seconds );
	fprintf( outfile, "%12u bytes received\n",
						estats->est_bytercvd );
	fprintf( outfile, "%12u bytes sent\n",
						estats->est_bytesent );
	fprintf( outfile, "%12u data blocks received\n",
						estats->est_blokrcvd );
	fprintf( outfile, "%12u data blocks sent\n",
						estats->est_bloksent );
	fprintf( outfile, "%12u multicast bytes received\n",
						estats->est_mbytercvd );
	fprintf( outfile, "%12u multicast blocks received\n",
						estats->est_mblokrcvd );
	fprintf( outfile, "%12u multicast bytes sent\n",
						estats->est_mbytesent );
	fprintf( outfile, "%12u multicast blocks sent\n",
						estats->est_mbloksent );
	fprintf( outfile, "%12u blocks sent, initially deferred\n",
						estats->est_deferred );
	fprintf( outfile, "%12u blocks sent, single collision\n",
						estats->est_single );
	fprintf( outfile, "%12u blocks sent, multiple collisions\n",
						estats->est_multiple );
	display_reasons(outfile, "%12u send failures", estats->est_sendfail,
		estats->est_sendfail_bm, ether_sendfails);
	fprintf( outfile, "%12u collision detect check failure\n",
						estats->est_collis );
	display_reasons(outfile, "%12u receive failures", estats->est_recvfail,
		estats->est_recvfail_bm, ether_recvfails);
	fprintf( outfile, "%12u unrecognized frame destination\n",
						estats->est_unrecog );
	fprintf( outfile, "%12u data overruns\n",
						estats->est_overrun );
	fprintf( outfile, "%12u system buffer unavailable\n",
						estats->est_sysbuf );
	fprintf( outfile, "%12u user buffer unavailable\n",
						estats->est_userbuf );
}

char *ddcmp_data_inbound_errors[] = {
    "NAKs sent, header CRC",
    "NAKs sent, data CRC",
    "NAKs sent, REP response",
    NULL
};

char *ddcmp_data_outbound_errors[] = {
    "NAKs received, header CRC",
    "NAKs received, data CRC",
    "NAKs received, REP response",
    NULL
};

char *ddcmp_remote_buffer_errors[] = {
    "NAKs received, buffer unavailable",
    "NAKs received, buffer too small",
    NULL,
};

char *ddcmp_local_buffer_errors[] = {
    "NAKs sent, buffer unavailable",
    "NAKs sent, buffer too small",
    NULL,
};

char *ddcmp_selection_interval_failures[] = {
    "No reply to select",
    "Incomplete reply to select",
    NULL
};

char *ddcmp_remote_station_errors[] = {
    "NAKs received, receive overrun",
    "NAKs sent, header format",
    "Select address errors",
    "Streaming tributaries",
    NULL
};

char *ddcmp_local_station_errors[] = {
    "NAKs sent, receive overrun",
    "Receive overrun, NAK not sent",
    "Transmit underruns",
    "NAKs rcvd, header format",
    NULL
};

display_dstats( outfile, ifname, dstats, flag )
	FILE *outfile;
	char ifname[];
	struct dstat *dstats;
	int flag;
{
	int i;
	int now = time(0);

	fprintf( outfile, "\n%s DDCMP counters at %s",
			ifname, ctime(&now) );
	if  (flag == -1)  {
		errno == EPERM;
		perror( "Warning: failed to zero counters" );
	}
	fprintf( outfile, "\n");
	fprintf( outfile, "%12u seconds since last zeroed\n",
						dstats->dst_seconds );
	fprintf( outfile, "%12u bytes received\n",
						dstats->dst_bytercvd );
	fprintf( outfile, "%12u bytes sent\n",
						dstats->dst_bytesent );
	fprintf( outfile, "%12u data blocks received\n",
						dstats->dst_blockrcvd );
	fprintf( outfile, "%12u data blocks sent\n",
						dstats->dst_blocksent );
	display_reasons(outfile, "%12u data errors inbound",
		dstats->dst_inbound, dstats->dst_inbound_bm,
		ddcmp_data_inbound_errors);

	display_reasons(outfile, "%12u data errors outbound",
		dstats->dst_outbound, dstats->dst_outbound_bm,
		ddcmp_data_outbound_errors);

	fprintf( outfile, "%12u remote reply timeouts\n",
						dstats->dst_remotetmo );
	fprintf( outfile, "%12u local reply timeouts\n",
						dstats->dst_localtmo );

	display_reasons(outfile, "%12u remote buffer errors",
		dstats->dst_remotebuf, dstats->dst_remotebuf_bm,
		ddcmp_remote_buffer_errors);

	display_reasons(outfile, "%12u local buffer errors",
		dstats->dst_localbuf, dstats->dst_localbuf_bm,
		ddcmp_local_buffer_errors);

	fprintf( outfile, "%12u selection intervals elapsed\n",
						dstats->dst_select );

	display_reasons(outfile, "%12u selection timeouts",
		dstats->dst_selecttmo, dstats->dst_selecttmo_bm,
		ddcmp_selection_interval_failures);

	display_reasons(outfile, "%12u remote station errors",
		dstats->dst_remotesta, dstats->dst_remotesta_bm,
		ddcmp_remote_station_errors);

	display_reasons(outfile, "%12u local station errors",
		dstats->dst_localsta, dstats->dst_localsta_bm,
		ddcmp_local_station_errors);

}

#ifdef CTR_FDDI
display_fstats( outfile, ifname, fstats, flag )
	FILE *outfile;
	char ifname[];
	struct fstat *fstats;
	int flag;
{
	int i;
	time_t now = time(0);

	fprintf( outfile, "\n%s FDDI counters at %s",
			ifname, ctime(&now) );
	if  (flag == -1)  {
		errno == EPERM;
		perror( "Warning: failed to zero counters" );
	}
	fprintf( outfile, "\n");
	fprintf( outfile, "%12u seconds since last zeroed\n",
						fstats->fst_second );
	fprintf( outfile, "%12u ANSI MAC frame count\n",
						fstats->fst_frame );
	fprintf( outfile, "%12u ANSI MAC frame error count\n",
						fstats->fst_error );
	fprintf( outfile, "%12u ANSI MAC frames lost count\n",
						fstats->fst_lost );
	fprintf( outfile, "%12u bytes received\n",
						fstats->fst_bytercvd );
	fprintf( outfile, "%12u bytes sent\n",
						fstats->fst_bytesent );
	fprintf( outfile, "%12u data blocks received\n",
						fstats->fst_pdurcvd );
	fprintf( outfile, "%12u data blocks sent\n",
						fstats->fst_pdusent );
	fprintf( outfile, "%12u multicast bytes received\n",
						fstats->fst_mbytercvd );
	fprintf( outfile, "%12u multicast blocks received\n",
						fstats->fst_mpdurcvd );
	fprintf( outfile, "%12u multicast bytes sent\n",
						fstats->fst_mbytesent );
	fprintf( outfile, "%12u multicast blocks sent\n",
						fstats->fst_mpdusent );
	fprintf( outfile, "%12u transmit underrun errors\n",
						fstats->fst_underrun );
	fprintf( outfile, "%12u send failures\n",
						fstats->fst_sendfail );
	fprintf( outfile, "%12u FCS check failures\n",
						fstats->fst_fcserror );
	fprintf( outfile, "%12u frame status errors\n",
						fstats->fst_fseerror );
	fprintf( outfile, "%12u frame alignment errors\n",
						fstats->fst_pdualig );
	fprintf( outfile, "%12u frame length errors\n",
						fstats->fst_pdulen );
	fprintf( outfile, "%12u unrecognized frames\n",
						fstats->fst_pduunrecog );
	fprintf( outfile, "%12u unrecognized multicast frames\n",
						fstats->fst_mpduunrecog );
	fprintf( outfile, "%12u receive data overruns\n",
						fstats->fst_overrun );
	fprintf( outfile, "%12u system buffers unavailable\n",
						fstats->fst_sysbuf );
	fprintf( outfile, "%12u user buffers unavailable\n",
						fstats->fst_userbuf );
	fprintf( outfile, "%12u ring reinitialization received\n",
						fstats->fst_ringinitrcv );
	fprintf( outfile, "%12u ring reinitialization initiated\n",
						fstats->fst_ringinit );
	fprintf( outfile, "%12u ring beacon process initiated\n",
						fstats->fst_ringbeacon );
	fprintf( outfile, "%12u ring beacon process received\n",
						fstats->fst_ringbeaconrecv );
	fprintf( outfile, "%12u duplicate tokens detected\n",
						fstats->fst_duptoken );
	fprintf( outfile, "%12u duplicate address test failures\n",
						fstats->fst_dupaddfail );
	fprintf( outfile, "%12u ring purger errors\n",
						fstats->fst_ringpurge );
	fprintf( outfile, "%12u bridge strip errors\n",
						fstats->fst_bridgestrip );
	fprintf( outfile, "%12u traces initiated\n",
						fstats->fst_traceinit );
	fprintf( outfile, "%12u traces received\n",
						fstats->fst_tracerecv );
	fprintf( outfile, "%12u LEM reject count\n",
						fstats->fst_lem_rej );
	fprintf( outfile, "%12u LEM events count\n",
						fstats->fst_lem_events );
	fprintf( outfile, "%12u LCT reject count\n",
						fstats->fst_lct_rej );
	fprintf( outfile, "%12u TNE expired reject count\n",
						fstats->fst_tne_exp_rej );
	fprintf( outfile, "%12u Completed Connection count\n",
						fstats->fst_connection );
	fprintf( outfile, "%12u Elasticity Buffer Errors\n",
						fstats->fst_ebf_error);
}

display_state(outfile,fmt,flag,states,size)
   FILE *outfile;
   char *fmt;
   short  flag;
   char **states;
   int size;
{
   /* check to see if flag is an index outside of the states array */
   if(flag != 0 && flag >= size/sizeof(*states)) {
   	flag = size/sizeof(*states) - 1;
   }

   fprintf( outfile,"%-35s",fmt);
   fprintf( outfile,"%s\n",states[flag]);
}

static char *led_states[] = {
    "Null",
    "Off",
    "Red",
    "Red and Blinking",
    "Green",
    "Green and Blinking",
    "Red and Green",
    "Unknown"
};

static char *fta_led_states[] = {
    "Off",
    "Red",
    "Red and Blinking",
    "Green",
    "Green and Blinking",
    "Red and Green",
    "Unknown"
};

static char *mfa_led_states[] = {
    "Off/Off",
    "Off/Green",
    "Yellow/Green Blinking",
    "Yellow/Off",
    "Yellow/Green",
    "Unknown"
};

static char *link_states[] = {
    "Off Maintenance",
    "Off Ready",
    "Off Fault Recovery",
    "On Ring Initializing",
    "On Ring Running",
    "Broken",
    "Unknown"
};

static char *duplicate_add_tests[] = {
     "Unknown",
     "Absent",
     "Present",
     "Unknown"
};

static char *ring_purge_states[] = {
     "Purger off",
     "Candidate",
     "Non Purger",
     "Purger",
     "Unknown"
};

static char *phy_states[] = {
     "Off Maintenance",
     "Broken",
     "Off Ready",
     "Waiting",
     "Starting",
     "Failed",
     "Watching",
     "In Use",
     "Unknown"
};

static char *nbr_phy_types[] = {
     "A", "B", "Slave", "Master", "Unknown", NULL };

static char *rej_reasons[] = {
     "No Reason",
     "LCT Local",
     "LCT Remote",
     "LCT Both",
     "LEM failure",
     "Topology Rules",
     "TNE Expired",
     "Remote Reject",
     "Trace In Process",
     "Trace Received, Trace Off",
     "Stand By",
     "Unknown"
};

static char *rmt_states[] = {
	"Isolated",
	"Non Operation",
	"Ring Operation",
	"Detected",
	"Non Operation Duplicated",
	"Ring Operation Duplicated",
	"Directed",
	"Traced",
     	"Unknown"
};
static char *booleans[] = {
	"False",
	"True",
     	"Unknown"
};

static char *ring_error_reasons[] = {
	"No Reason",
	NULL, NULL, NULL, NULL,
	"Ring Init Initiated",
	"Ring Init Received",
	"Ring Beaconing Initiated",
	"Duplicate Address Detected",
	"Duplicate Token Detected",
	"Ring Purge Error",
	"Bridge Strip Error",
	"Ring Op Oscillation",
	"Directed Beacon Received",
	"PC Trace Initiated",
	"PC Trace Received",
     	"Unknown"
};

static char *pmd_types[] = {
	"ANSI Multimode",
	"ANSI Singlemode",
	"Thinwire",
	"Shielded Twisted Pair",
	"Unshielded Twisted Pair",
     	"Unknown"
};

static char *fta_pmd_types[] = {
	"ANSI Multimode",
	"ANSI Singlemode Type 1",
	"ANSI Singlemode Type 2",
	"ANSI SONET",
     	"Unknown"
};

static char *fta_pmd_types_cont[] = {
	"Low Power",			/* 100 */
	"Thin Wire",			/* 101 */
	"Shielded Twisted Pair",	/* 102 */
	"Unshielded Twisted Pair",	/* 103 */
     	"Unknown"
};

static char *mfa_pmd_types[] = {
	NULL,
	"ANSI Multimode",
	"ANSI Singlemode",
	"Low cost fiber",
	"Thinwire copper",
	"Shielded Twisted Pair",
	"Unshielded Twisted Pair",
     	"Unknown"
};

static char *frame_strip_modes[] = {
	"Source Address Match",
	"Bridge Strip",
     	"Unknown"
};

static char *adapter_states[] = {
	"Reset State",
	"Uninitialized State",
	"Initialized State",
	"Running State",
	"Maintenance State",
	"Halted State",
     	"Unknown"
};

static char *fta_adapter_states[] = {
	"Reset State",
	"Upgrade State",
	"DMA Unavailable State",
	"DMA Available State",
	"Link Available State",
	"Link Unavailable State",
	"Halted State",
	"Ring Member State",
     	"Unknown"
};

static char *fdx[] = { 
	"Unknown",
	"Enabled",
	"Disabled",
     	"Unknown"
};

static char *fdx_states[] = { 
	"Unknown",
	"Idle",
	"Request",
	"Confirm",
	"Operational",
     	"Unknown"
};

display_fstatus( outfile, ifname, fs)
	FILE *outfile;
	char ifname[];
	struct fstatus *fs;
{
	int i;
	int bitmask;
	static char *fddirev = "1.2";

	fprintf(outfile, "\n%s FDDI status \n", ifname);

	fprintf( outfile, "\n");
	if ((strncmp(ifname, "fta", 3) == 0) || (strncmp(ifname, "faa", 3) == 0)) {
	    display_state( outfile, "Adapter State:",fs->state,fta_adapter_states,sizeof(fta_adapter_states));
            display_state( outfile,"LED State:",fs->led_state,fta_led_states,sizeof(fta_led_states));
	} else {
            display_state( outfile, "Adapter State:",fs->state,adapter_states,sizeof(adapter_states));
            if (strncmp(ifname, "mfa", 3) == 0)
              display_state( outfile,"LED State:",fs->led_state,mfa_led_states,sizeof(mfa_led_states));
	    else
              display_state( outfile,"LED State:",fs->led_state,led_states,sizeof(led_states));
	}
/* 	
	for(i=0,bitmask=fs->rmt_state;bitmask > 0;) {
		if(bitmask & 1 )
			break;
		i++;
		bitmask >>= 1;
	}
 	display_state( outfile,"RMT State:",i,rmt_states);  */

        display_state( outfile, "Link State:",fs->link_state,link_states,sizeof(link_states));
        display_state( outfile, "Duplicate Address Condition:",
              fs->dup_add_test,duplicate_add_tests,sizeof(duplicate_add_tests)
);
        display_state( outfile, "Ring Purger State:",fs->ring_purge_state,
              ring_purge_states,sizeof(ring_purge_states));
	fprintf(outfile,"%-35s%-6.3f ms\n","Negotiated TRT:",(fs->neg_trt *
80.0)/1000000.0 );	
	fprintf(outfile,"%-35s","Upstream Neighbor Address:");
	fprintf( outfile, "%02X-%02X-%02X-%02X-%02X-%02X\n",
		fs->upstream[0],		fs->upstream[1],
		fs->upstream[2],		fs->upstream[3],
		fs->upstream[4],		fs->upstream[5] );
        display_state( outfile, "UNA Timed Out:",fs->una_timed_out,
                      booleans,sizeof(booleans));
	if((strcmp(fs->fw_rev,fddirev) >= 0) ||
	   (strncmp(ifname, "mfa", 3) == 0) ||
	   (strncmp(ifname, "fta", 3) == 0) ||
	   (strncmp(ifname, "faa", 3) == 0)
	  ) {
	fprintf(outfile,"%-35s","Downstream Neighbor Address:");
	fprintf( outfile, "%02X-%02X-%02X-%02X-%02X-%02X\n",
		fs->downstream[0],		fs->downstream[1],
		fs->downstream[2],		fs->downstream[3],
		fs->downstream[4],		fs->downstream[5] );
	}
	if (strncmp(ifname, "mfa", 3) == 0) 
		display_state( outfile, "Ring Error Reason:", (fs->ring_error & 0xff),
                      ring_error_reasons,sizeof(ring_error_reasons));
	else {
		display_state( outfile, "Claim Token Yield:",fs->claim_token_mode,
                      booleans,sizeof(booleans));
		display_state( outfile,"Frame Strip Mode:",fs->frame_strip_mode,
                      frame_strip_modes,sizeof(frame_strip_modes));
		display_state( outfile, "Ring Error Reason:",fs->ring_error,
                      ring_error_reasons,sizeof(ring_error_reasons));
		fprintf(outfile,"%-35s","Last Direct Beacon SA:");
		fprintf( outfile, "%02X-%02X-%02X-%02X-%02X-%02X\n",
			fs->dir_beacon[0],		fs->dir_beacon[1],
			fs->dir_beacon[2],		fs->dir_beacon[3],
			fs->dir_beacon[4],		fs->dir_beacon[5] );
	}
	display_state( outfile, "Physical Port State:",fs->phy_state,
                      phy_states,sizeof(phy_states));
	display_state( outfile, "Neighbor Physical Port Type:",
                      fs->neighbor_phy_type,nbr_phy_types,sizeof(nbr_phy_types
));
	display_state( outfile, "Reject Reason:",fs->rej_reason,
                      rej_reasons,sizeof(rej_reasons));
	fprintf(outfile,"%-35s%d\n","Physical Link Error Estimate:",
			fs->phy_link_error);
	if ((strncmp(ifname, "fta", 3) == 0) ||
	    (strncmp(ifname, "faa", 3) == 0)) {
                display_state( outfile, "Full Duplex Mode:",fs->full_duplex_mode, fdx,sizeof(fdx));
		if (fs->full_duplex_mode == FDX_ENB) /* it is enabled */
                      display_state( outfile, "Full Duplex State:",fs->full_duplex_state, fdx_states,sizeof(fdx_states));
	}
	
	fprintf(outfile, "\n%s FDDI characteristics \n\n", ifname);

	fprintf(outfile,"%-35s","Link Address:");
	fprintf( outfile, "%02X-%02X-%02X-%02X-%02X-%02X\n",
		fs->mla[0],		fs->mla[1],
		fs->mla[2],		fs->mla[3],
		fs->mla[4],		fs->mla[5] );
	if (strncmp(ifname, "mfa", 3) == 0 ) {
	    fprintf(outfile,"%-35s","Firmware Revision:");
            fprintf(outfile,"V%c.%c\n", ((int)fs->fw_rev[0]/16 + '0'), 
		((int)fs->fw_rev[0]%16 + '0'));
	    fprintf(outfile,"%-35s","Hardware Revision:");
            fprintf(outfile,"V%c\n", ((int)fs->phy_rev[0] + '0')); 
	} else if ((strncmp(ifname, "fta", 3) == 0 ) || (strncmp(ifname, "faa", 3) == 0 )) {
	    fprintf(outfile,"%-35s","Firmware Revision:");
	    fprintf( outfile,"%c%c%c%c\n",
		    fs->fw_rev[3],fs->fw_rev[2],fs->fw_rev[1],fs->fw_rev[0]);
	    fprintf(outfile,"%-35s","Hardware Revision:");
	    fprintf( outfile,"%c%c%c%c\n",
		fs->phy_rev[3],fs->phy_rev[2],fs->phy_rev[1],fs->phy_rev[0]);
	} else {
	    fprintf(outfile,"%-35s","Firmware Revision:");
	    fprintf( outfile,"%c%c%c%c\n",
		    fs->fw_rev[0],fs->fw_rev[1],fs->fw_rev[2],fs->fw_rev[3]);
	    fprintf(outfile,"%-35s","ROM Revision:");
	    fprintf( outfile,"%c%c%c%c\n",
		fs->phy_rev[0],fs->phy_rev[1],fs->phy_rev[2],fs->phy_rev[3]);
	}
	fprintf(outfile,"%-35s%-4d\n","SMT Version ID:",
			fs->smt_version);
	fprintf(outfile,"%-35s%-6.3f ms \n","Requested TRT:",
			fs->t_req * 80.0 / 1000000.0 );
	if ((strncmp(ifname, "fta", 3) == 0) || (strncmp(ifname, "faa", 3) == 0)) {
	    fprintf(outfile,"%-35s%-6.1f ms \n","Maximum TRT:",
			(double)fs->t_max);
	} else
	    fprintf(outfile,"%-35s%-6.3f ms \n","Maximum TRT:",
			fs->t_max * 80.0 / 1000000.0 );
	fprintf(outfile,"%-35s%-6.3f ms \n","Valid Transmission Time:",
			fs->tvx * 80.0 / 1000000.0 );
	fprintf(outfile,"%-35s%d \n","LEM Threshold:",fs->lem_threshold); 
	if((strcmp(fs->fw_rev,fddirev) >= 0) || (strncmp(ifname, "mfa", 3) == 0))
		fprintf(outfile,"%-35s%-6.3f ms \n","Restricted Token Timeout:",
			fs->rtoken_timeout * 80.0 / 1000000.0 );
	
	if ((strncmp(ifname, "fta", 3) == 0) || (strncmp(ifname, "faa", 3) == 0)) {
	    if (fs->pmd_type < 4) 
              display_state( outfile, "PMD Type",fs->pmd_type , fta_pmd_types,sizeof(fta_pmd_types));
	    else
              display_state( outfile, "PMD Type",fs->pmd_type % 100, fta_pmd_types_cont,sizeof(fta_pmd_types_cont));
	    /* Counter interval update */
	    if (fs->cnt_interval < 0)
		fs->cnt_interval = 0;
	    fprintf(outfile,"%-35s%-4d sec \n","Counter Update Interval:",
						fs->cnt_interval); 
	} else if (strncmp(ifname, "mfa", 3) == 0)
              display_state( outfile, "PMD Type",fs->pmd_type,mfa_pmd_types,sizeof(mfa_pmd_types));
	    else
              display_state( outfile, "PMD Type",fs->pmd_type % 99 ,pmd_types,sizeof(pmd_types));
}
		
#endif /* CTR_FDDI */

display_trncount( outfile, ifname, trncount, flag )
	FILE *outfile;
	char ifname[];
	struct trncount *trncount;
	int flag;
{
	int i;
	time_t now = time(0);

	fprintf( outfile, "\n%s Token ring counters at %s",
			ifname, ctime(&now) );
	if  (flag == -1)  {
		errno == EPERM;
		perror( "Warning: failed to zero counters" );
	}
	fprintf( outfile, "\n");
	fprintf( outfile, "%12u seconds since last zeroed\n",
						trncount->trn_second);
	fprintf( outfile, "%12u bytes received\n",
						trncount->trn_bytercvd );
	fprintf( outfile, "%12u bytes sent\n",
						trncount->trn_bytesent );
	fprintf( outfile, "%12u data blocks received\n",
						trncount->trn_pdurcvd );
	fprintf( outfile, "%12u data blocks sent\n",
						trncount->trn_pdusent );
	fprintf( outfile, "%12u multicast bytes received\n",
						trncount->trn_mbytercvd );
	fprintf( outfile, "%12u multicast blocks received\n",
						trncount->trn_mpdurcvd );
	fprintf( outfile, "%12u multicast bytes sent\n",
						trncount->trn_mbytesent );
	fprintf( outfile, "%12u unrecognized frames\n",
						trncount->trn_pduunrecog );
	fprintf( outfile, "%12u unrecognized multicast frames\n",
						trncount->trn_mpduunrecog );
	fprintf( outfile, "%12u transmit failures\n",
						trncount->trn_xmit_fail );
	fprintf( outfile, "%12u transmit underrun errors\n",
						trncount->trn_xmit_underrun );
	fprintf( outfile, "%12u line errors\n",
						trncount->trn_line_error );
	fprintf( outfile, "%12u internal error\n",
						trncount->trn_internal_error );
	fprintf( outfile, "%12u burst error\n",
						trncount->trn_burst_error );
	fprintf( outfile, "%12u ARI/FCI error\n",
						trncount->trn_ari_fci_error );
	fprintf( outfile, "%12u abort delimiters transmitted\n",
						trncount->trn_ad_trans );
	fprintf( outfile, "%12u lost frame errors\n",
						trncount->trn_lost_frame_error);
	fprintf( outfile, "%12u receive data overruns\n",
						trncount->trn_rcv_congestion_error );
	fprintf( outfile, "%12u frame copied errors\n",
						trncount->trn_frame_copied_error );
	fprintf( outfile, "%12u token errors\n",
						trncount->trn_token_error );
	fprintf( outfile, "%12u hard errors\n",
						trncount->trn_hard_error );
	fprintf( outfile, "%12u soft errors\n",
						trncount->trn_soft_error );
	fprintf( outfile, "%12u adapter resets\n",
						trncount->trn_adapter_reset );
	fprintf( outfile, "%12u signal loss\n",
						trncount->trn_signal_loss );
	fprintf( outfile, "%12u beacon transmits\n",
						trncount->trn_xmit_beacon );
	fprintf( outfile, "%12u ring recoverys\n",
						trncount->trn_ring_recovery );
	fprintf( outfile, "%12u lobe wire faults\n",
						trncount->trn_lobe_wire_fault );
	fprintf( outfile, "%12u removes received\n",
						trncount->trn_remove_received );
	fprintf( outfile, "%12u single station\n",
						trncount->trn_single_station );
	fprintf( outfile, "%12u self test failures\n",
						trncount->trn_selftest_fail );
}

char *get_beacon_type(), *get_major_vector(), *get_open_status();
void get_ring_status();
u_short get_ring_speed();

display_trnchars( outfile, ifname, trnchar)
	FILE *outfile;
	char ifname[];
	struct trnchar *trnchar;
{
	char *valuep;
	u_short value;

	fprintf(outfile, "\n%s Token ring and host information: \n", ifname);

	fprintf( outfile, "\n");
	fprintf(outfile,"%-40s","MAC address:");
	fprintf( outfile, "%02X-%02X-%02X-%02X-%02X-%02X\n",
		trnchar->mac_addr[0] & 0xff,
		trnchar->mac_addr[1] & 0xff,
		trnchar->mac_addr[2] & 0xff,
		trnchar->mac_addr[3] & 0xff,
		trnchar->mac_addr[4] & 0xff,
		trnchar->mac_addr[5] & 0xff);
	fprintf(outfile,"%-40s","Group address:");
	fprintf( outfile, "%02X-%02X-%02X-%02X-%02X-%02X\n",
		trnchar->grp_addr[0] & 0xff,
		trnchar->grp_addr[1] & 0xff,
		trnchar->grp_addr[2] & 0xff,
		trnchar->grp_addr[3] & 0xff,
		trnchar->grp_addr[4] & 0xff,
		trnchar->grp_addr[5] & 0xff);
	fprintf(outfile,"%-40s","Functional address:");
	fprintf( outfile, "%02X-%02X-%02X-%02X-%02X-%02X\n",
		trnchar->func_addr[0] & 0xff,
		trnchar->func_addr[1] & 0xff,
		trnchar->func_addr[2] & 0xff,
		trnchar->func_addr[3] & 0xff,
		trnchar->func_addr[4] & 0xff,
		trnchar->func_addr[5] & 0xff);

	fprintf(outfile,"%-40s%d\n","Physical drop number:",
			trnchar->drop_numb);

	fprintf(outfile,"%-40s","Upstream neighbor address:");
	fprintf( outfile, "%02X-%02X-%02X-%02X-%02X-%02X\n",
		trnchar->upstream_nbr[0] & 0xff,
		trnchar->upstream_nbr[1] & 0xff,
		trnchar->upstream_nbr[2] & 0xff,
		trnchar->upstream_nbr[3] & 0xff,
		trnchar->upstream_nbr[4] & 0xff,
		trnchar->upstream_nbr[5] & 0xff);

	fprintf(outfile,"%-40s%d\n","Upstream physical drop number:",
			trnchar->upstream_drop_numb);
	fprintf(outfile,"%-40s%d\n","Transmit access priority:",
			trnchar->upstream_drop_numb);

        valuep = get_major_vector(trnchar->last_major_vector);
	fprintf(outfile,"%-40s%s\n","Last major vector:", valuep);
	if (trnchar->last_major_vector == MV_REPORT_MONTIOR_ERR) {
	    fprintf(outfile,"%-40s%s\n","Monitor error code:",
		    trnchar->monitor_error_code);
	}
	{
	    u_char status_p[100];
	    get_ring_status(trnchar->ring_status, status_p);
	    fprintf(outfile,"%-40s%s\n","Ring status:", status_p);
	}
	if (trnchar->monitor_contd)
		valuep = "Yes";
	else 
		valuep = "No";

	fprintf(outfile,"%-40s%s\n","Monitor contender:", valuep);

	fprintf(outfile,"%-40s%dms\n","Soft error timer value:",
			trnchar->soft_error_timer);
	fprintf(outfile,"%-40s%d\n","Local ring number:",
			trnchar->ring_number);

	valuep = get_beacon_type(trnchar->beacon_transmit_type);
	fprintf(outfile,"%-40s%s\n","Reason for transmitting beacon:", valuep);
	valuep = get_beacon_type(trnchar->beacon_receive_type);
	fprintf(outfile,"%-40s%s\n","Reason for receiving beacon:", valuep);

	fprintf(outfile,"%-40s","Last beacon upstream neighbor address:");
	fprintf( outfile, "%02X-%02X-%02X-%02X-%02X-%02X\n",
		trnchar->beacon_una[0] & 0xff,
		trnchar->beacon_una[1] & 0xff,
		trnchar->beacon_una[2] & 0xff,
		trnchar->beacon_una[3] & 0xff,
		trnchar->beacon_una[4] & 0xff,
		trnchar->beacon_una[5] & 0xff);
	fprintf(outfile,"%-40s%d\n","Beacon station physical drop number:",
			trnchar->beacon_stn_drop_numb);

	value = get_ring_speed(trnchar->ring_speed);
	if (value) 
		fprintf(outfile,"%-40s%dMbs\n","Ring speed:", value);
	else	 
		fprintf(outfile,"%-40s%s\n","Ring speed:", "Unknown");
	if (trnchar->etr == 1)
		fprintf(outfile,"%-40s%s\n","Early token release:", "True");
	else
		fprintf(outfile,"%-40s%s\n","Early token release:", "False");
	valuep = get_open_status(trnchar->open_status);
	fprintf(outfile,"%-40s%s\n","Open status:", valuep);
	fprintf(outfile,"%-40s%s\n","Token ring chip:", trnchar->token_ring_chip);
}

char *
get_beacon_type(beacon)
u_short beacon;
{
	switch(beacon) {
		case BT_SET_RECOV_MODE:
			return("Set recovery mode");
		case BT_SET_SIGNAL_LOSS:
			return("Signal loss");
		case BT_SET_BIT_STREAMING:
			return("No contention frames received");
		case BT_SET_CONT_STREAMING:
			return("Contention streaming");
		default:
			return("No beacon");
	}
}

char *
get_major_vector(vector)
u_short vector;
{
	switch(vector) {
		case MV_RESPONSE:
			return("Response");
		case MV_BEACON:
			return("Beacon");
		case MV_CLAIM_TOKEN:
			return("Claim token");
		case MV_RING_PURGE:
			return("Ring purge");
		case MV_ACTIVE_MON_PRES:
			return("Active monitor present");
		case MV_STANDBY_MON_PRES:
			return("Standby monitor present");
		case MV_DUP_ADDR_TEST:
			return("Duplicate address test");
		case MV_LOBE_MEDIA_TEST:
			return("Lobe media test");
		case MV_TRANSMIT_FORW:
			return("Transmit forward");
		case MV_RMV_RING_STATION:
			return("Remove ring station");
		case MV_CHANGE_PARM:
			return("Change parameters");
		case MV_INIT_RING_STN:
			return("Initialize ring station");
		case MV_REQ_STN_ADDR:
			return("Request station address");
		case MV_REQ_STN_STATE:
			return("Request station state");
		case MV_REQ_STN_ATTACH:
			return("Request station attachment");
		case MV_REQ_INIT:
			return("Request initialization");
		case MV_REPORT_STN_ADDR:
			return("Report station address");
		case MV_REPORT_STN_STATE:
			return("Report station state");
		case MV_REPORT_STN_ATTACH:
			return("Report station attachment");
		case MV_REPORT_NEW_MONITOR:
			return("Report new monitor");
		case MV_REPORT_SUA_CHANGE:
			return("Report change in stored upstream address");
		case MV_REPORT_RNG_POLL_FAIL:
			return("Report ring poll failure");
		case MV_REPORT_MONTIOR_ERR:
			return("Report monitor error");
		case MV_REPORT_ERR:
			return("Report error");
		case MV_REPORT_TRANSMIT_FORW:
			return("Report transmit forward");
		default:
			return("Unknown");
	}
}

void
get_ring_status(status, status_p)
u_short status;
u_char *status_p;
{


    if (status == MIB1231_RSTATUS_NO_PROB)  {
	strcpy(status_p, "No problems detected");
	return;
    }
    
    if (status == MIB1231_RSTATUS_NO_STATUS)  {
	strcpy(status_p, "No status");
	return;
    }
    
    if (status & MIB1231_RSTATUS_RING_RECOVERY) {
	strcpy(status_p, "Ring recovery ");
	status_p += strlen("Ring recovery ");
    }
    
    if (status & MIB1231_RSTATUS_SINGLE_STATION) {
	strcpy (status_p, "Single station ");
	status_p += strlen("Single station ");
    }
    
    if (status & MIB1231_RSTATUS_REMOVE_RCVD) {
	strcpy(status_p, "Remove received ");
	status_p += strlen("Remove received ");
    }
    
    if (status & MIB1231_RSTATUS_AUTO_REM_ERROR) {
	strcpy(status_p, "Auto removal error ");
	status_p += strlen("Auto removal error ");
    }
    
    if (status & MIB1231_RSTATUS_LOBE_WIRE_FAULT) {
	strcpy(status_p, "Lobe wire fault ");
	status_p += strlen("Lobe wire fault ");
    }
    
    if (status & MIB1231_RSTATUS_TRANSMIT_BEACON) {
	strcpy(status_p, "Transmit beacon ");
	status_p += strlen("Transmit beacon ");
    }
    
    if (status & MIB1231_RSTATUS_SOFT_ERROR) {
	strcpy(status_p, "Soft error ");
	status_p += strlen("Soft error ");
    }
    
    if (status & MIB1231_RSTATUS_HARD_ERROR) {
	strcpy(status_p, "Hard error ");
	status_p += strlen("Hard error ");
    }
    
    if (status & MIB1231_RSTATUS_SIGNAL_LOSS) {
	strcpy(status_p, "Signal loss ");
	status_p += strlen("Signal loss ");
    }
    
}

u_short
get_ring_speed(speed)
{
	switch(speed) {
		case MIB1231_RSPEED_1_MEG:
			return(1);
		case MIB1231_RSPEED_4_MEG:
			return(4);
		case MIB1231_RSPEED_16_MEG:
			return(16);
		default:
			return(0);
         }
}


char *
get_open_status(status)
u_short status;
{
	switch(status) {
		case MIB1231_ROSTATUS_NOOPEN:
			return("Not opened");	
			break;
		case MIB1231_ROSTATUS_BADPARM:
			return(" Bad parameters");	
			break;
		case MIB1231_ROSTATUS_LOBEFAILED:
			return("Lobe test failed");	
			break;
		case MIB1231_ROSTATUS_SIG_LOSS:
			return("Signal loss");	
			break;
		case MIB1231_ROSTATUS_INS_TIMEOUT:
			return("Insertion timeout");	
			break;
		case MIB1231_ROSTATUS_RING_FAILED:
			return("Ring purge timeout");	
			break;
		case MIB1231_ROSTATUS_BEACONING:
			return("Beaconing");	
			break;
		case MIB1231_ROSTATUS_DUPLICATE_MAC:
			return("Duplicate MAC address");	
			break;
		case MIB1231_ROSTATUS_REQ_FAILED:
			return("No response from RPS");	
			break;
		case MIB1231_ROSTATUS_REM_RECVD:
			return("Remove received");	
			break;
		case MIB1231_ROSTATUS_OPEN:
			return("Open");	
			break;
	}
}



