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
static char *rcsid = "@(#)$RCSfile: srconfig.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/12/20 21:49:14 $";
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>

#include <nlist.h>
#include <sys/file.h>
#include <paths.h>

#include <sys/ioctl.h>
#include <net/if_trn_sr.h>

extern int errno;
static int canonical = 1;

void print_trnrif();
void haddr_convert();

main(argc, argv)
   int argc ;
   char **argv ;
{
   u_int su_privileges = 0;
   void dump_table(), show_entry(), read_counter(), read_attr(), disable_entry(), delete_entry(), enab_SR(), disab_SR(), sr_help();

   if (getuid() == 0)
	su_privileges = 1;
	
   if (argc > 1)
   {
	if (strcasecmp(argv[1], "-RTable") == 0 || 
	    strcasecmp(argv[1], "-RT") == 0)
	{
		if (argc != 2 && argc != 3)
		{
		    fprintf(stderr, "srconfig: Invalid Input\n");
		    sr_help();
		}
                else
		{
		    if (argc == 2)
		    	dump_table(_PATH_UNIX, _PATH_KMEM);
		    else
		    if ((argc == 3) && (strcasecmp(argv[2],  "-u") == 0))
		    {
			canonical = 0;
		    	dump_table(_PATH_UNIX, _PATH_KMEM);
		    }
		    else
		    {
		    	fprintf(stderr, "srconfig: Invalid Input\n");
		    	sr_help();
		    }
		}
	}
	else
	if (strcasecmp(argv[1],  "-REntry") == 0 ||
	    strcasecmp(argv[1],  "-RE") == 0)
	{
		if (argc != 3 && argc != 4)
		{
		    fprintf(stderr, "srconfig: Invalid Input");
		    sr_help();
		}
		else
		    if (argc == 3)
			show_entry(argv[2]);
		    else
		    if ((argc == 4) && (strcasecmp(argv[2], "-u") == 0))
		    {
			canonical = 0;
			show_entry(argv[3]);
		    }
		    else
		    {
			fprintf(stderr, "srconfig: Invalid Input");
			sr_help();
		    }
	}    
	else 
	if (strcasecmp(argv[1],  "-RCounter") == 0 ||
	    strcasecmp(argv[1],  "-RC") == 0)
        {
                if (argc != 2)
                {
		    fprintf(stderr, "srconfig: Invalid Input");
                    sr_help();
                }
                else
                    read_counter();
        }
        else
	if (strcasecmp(argv[1],  "-RAttr") == 0 ||
	    strcasecmp(argv[1],  "-RA") == 0)
        {
                if (argc != 2)
                {
		    fprintf(stderr, "srconfig: Invalid Input");
                    sr_help();
                }
                else
                    read_attr();
        }
        else
        if (strcasecmp(argv[1],  "-DisEntry") == 0 ||
	    strcasecmp(argv[1],  "-DisE") == 0)
	{
		if (su_privileges)
		{
                    if (argc != 3 && argc != 4)
		    {
		    	fprintf(stderr, "srconfig: Invalid Input\n");
                    	sr_help();
		    }
		    else
		    if (argc == 3)
                    	disable_entry(argv[2]);
		    else
		    if ((argc == 4) && (strcasecmp(argv[2], "-u") == 0))
		    {
			canonical = 0;
                    	disable_entry(argv[3]);
		    }
		    else
		    {
			fprintf(stderr, "srconfig: Invalid Input");
			sr_help();
		    }
		}
                else
                    fprintf(stderr, "srconfig: Insufficient Privileges\n");
        }
        else
        if (strcasecmp(argv[1],  "-DelEntry") == 0 ||
	    strcasecmp(argv[1],  "-De") == 0)
	{
		if (su_privileges)
		{
                    if (argc != 3 && argc != 4)
		    {
		    	fprintf(stderr, "srconfig: Invalid Input\n");
                    	sr_help();
		    }
		    else
		    if (argc == 3)
                    	delete_entry(argv[2]);
		    else
		    if ((argc == 4) && (strcasecmp(argv[2], "-u") == 0))
		    {
			canonical = 0;
                    	delete_entry(argv[3]);
		    }
		    else
		    {
			fprintf(stderr, "srconfig: Invalid Input");
			sr_help();
		    }
		}
                else
                    fprintf(stderr, "srconfig: Insufficient Privileges\n");
        }
        else
/*	This is not allowed
	if (strcasecmp(argv[1],  "-EnaSR") == 0 ||
	    strcasecmp(argv[1],  "-E") == 0)
	{
		if (su_privileges)
		{
		    if (argc != 2)
		    {
		    	fprintf(stderr, "srconfig: Invalid Input");
                    	sr_help();
		    }
                    else
			enab_SR();
		}
		else
		    fprintf(stderr, "srconfig: Insufficient privileges\n");
	}
	else
	if (strcasecmp(argv[1],  "-DisSR") == 0 ||
	    strcasecmp(argv[1],  "-DisS") == 0)
	{
		if (su_privileges)
		{
		    if (argc != 2)
		    {
		    	fprintf(stderr, "srconfig: Invalid Input");
                    	sr_help();
		    }
                    else
			disab_SR();
		}
		else
		    fprintf(stderr, "srconfig: Insufficient privileges\n");
	}
	else
*/
        if (strcasecmp(argv[1],  "-ZCounter") == 0 ||
	    strcasecmp(argv[1],  "-Z") == 0)
        {
		if (su_privileges)
		{
                    if (argc < 2)
		    {
		    	fprintf(stderr, "srconfig: Invalid Input\n");
                    	sr_help();
		    }
                    else 
		    if (argc != 2 )
		    {
		    	fprintf(stderr, "srconfig: Invalid Input\n");
		   	sr_help();
		    }
		    else
                    	zero_counter();
		}
		else
		    fprintf(stderr, "srconfig: Insufficient privileges\n");
        }
	else
	if (strcasecmp(argv[1],  "-SetAgeTimer") == 0 ||
	    strcasecmp(argv[1],  "-SetA") == 0)
        {
                if (su_privileges)
                {
                    if (argc < 2)
                    {
                        fprintf(stderr, "srconfig: Invalid Input\n");
                        sr_help();
                    }
                    else 
		    if (argc != 3 )
		    {
			fprintf(stderr, "srconfig: Invalid Input\n");
			sr_help();
		    }
                    else
                        set_aging_timer(argv[2]);
                }
                else
                    fprintf(stderr, "srconfig: Insufficient privileges\n");
       	}
       	else
	if (strcasecmp(argv[1],  "-SetDscTimer") == 0 ||
	    strcasecmp(argv[1],  "-SetD") == 0)
        {
                if (su_privileges)
                {
                    if (argc < 2)
                    {
                        fprintf(stderr, "srconfig: Invalid Input\n");
                        sr_help();
                    }
                    else
		    if (argc != 3 )
		    {
			fprintf(stderr, "srconfig: Invalid Input\n");
			sr_help();
		    }
                    else
                        set_discovery_timer(argv[2]);
                }
                else
                    fprintf(stderr, "srconfig: Insufficient privileges\n");
        }
	else
        if (strcasecmp(argv[1],  "-SetMaxEntry") == 0 ||
	    strcasecmp(argv[1],  "-SetM") == 0)
        {
		if (su_privileges)
		{
                    if (argc < 2)
		    {
		    	fprintf(stderr, "srconfig: Invalid Input\n");
                    	sr_help();
		    }
                    else 
		    if (argc != 3 )
		    {
		    	fprintf(stderr, "srconfig: Invalid Input\n");
		   	sr_help();
		    }
		    else
                    	set_SR_size(argv[2]);
		}
		else
		    fprintf(stderr, "srconfig: Insufficient privileges\n");
        }
	else
	{
		printf("\nsrconfig: Invalid input : %s.\n", argv[1]); 
		sr_help();
	}
   }
   else
	sr_help();
}


void
 sr_help()
{
	printf("\nToken Ring Source Routing Management Usage :\n\n");
	printf(" srconfig -RTable [-u]			: Read the entire Source Routing Table\n");
	printf(" srconfig -REntry [-u] mac_address	: Read a Source Routing Table entry\n");
 	printf(" srconfig -RCounter			: Read the Source Routing Counters\n");
	printf(" srconfig -RAttr			: Read the Source Routing Attributes\n");
	printf(" srconfig -DISEntry [-u] mac_address	: Disable a Source Routing Table entry\n");
	printf(" srconfig -DElentry [-u] mac_address	: Delete a Source Routing Table entry\n");
/*	This is not allowed
	printf(" srconfig -Enasr			: Enable Source Routing\n");
	printf(" srconfig -DISSr			: Disable Source Routing\n");
*/
	printf(" srconfig -Zcounter			: Zero the Source Routing Counters\n");
	printf(" srconfig -SETAgetimer new_timer	: Set the Source Routing Aging Timer\n");
	printf(" srconfig -SETDsctimer new_timer	: Set the extended LAN Discovery timer\n");
	printf(" srconfig -SETMaxentry new_srt_size	: Set the max number of entries allowed\n");

}

void
 show_entry(mac_add_p)
char *mac_add_p;
{
	u_char maddr[6];
	int s;
	char *ip_addr, *inet_ntoa();
	struct srreq srreq_ent;
	void print_rif();

	if (mac_aton(mac_add_p, maddr))
		return;

	s = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (s < 0) 
	{ 
		fprintf(stderr, "srconfig: ");	
		perror("Error in socket call\n");
		exit(1);
	}		 
    
	srreq_ent.srreq_type = SR_RENT;
	bcopy(maddr, srreq_ent.srreq_data.srtab.srt_mac_addr, 6);
 
	/* Read the srreq structure */
	if ( ioctl(s, SIOCSRREQR, (caddr_t)&srreq_ent) < 0 ) 
	{
		if (errno == ENXIO)
		   printf("%s : Address not found in the SR database\n", mac_add_p);
		else
		   if (errno == ENODEV)
	    	      fprintf(stderr, "srconfig: source routing is not enabled\n");
		   else
		      perror("SIOCSRREQR");
		exit(1);
	}

	close(s);

	/* print the contents of the entry */
	ether_print(srreq_ent.srreq_data.srtab.srt_mac_addr);
	if (srreq_ent.srreq_data.srtab.srt_ip_addr.s_addr)
		printf("(ip = %s) ", inet_ntoa(srreq_ent.srreq_data.srtab.srt_ip_addr));
	switch (srreq_ent.srreq_data.srtab.srt_status)
	{
	case RIF_NOROUTE:
		printf(" No Route\n");
		break;
	case RIF_REDISCOVER:
		printf(" Rediscover\n");
		break;
	case RIF_ONRING:
		printf(" On Ring\n");
		break;
	case RIF_HAVEROUTE:
		printf(" Have Route\n");
		print_trnrif(&srreq_ent.srreq_data.srtab.srtab_rif);
		break;
	case RIF_STALE:
		if (srreq_ent.srreq_data.srtab.srtab_rif.rif_lth > 2)
		{
			printf(" Stale (Have Route)\n");
			print_trnrif(&srreq_ent.srreq_data.srtab.srtab_rif);
		}
		else
			if (srreq_ent.srreq_data.srtab.srtab_rif.rif_rt == SR_ALL_ROUTE_EXPLORE)
				printf(" Stale (incomplete)\n");
			else
				printf(" Stale (On Ring)\n");
		break;
	default:
		printf(" unknow status\n");
		break;
	}
	printf("\n");
	return;
}


static char digits[] = "0123456789abcdef"; 
char *
 ether_sprintf(ap)
        u_char *ap;
{
        register i;
        static char etherbuf[18];
        register char *cp = etherbuf;

        for (i = 0; i < 6; i++) {
                *cp++ = digits[*ap >> 4];
                *cp++ = digits[*ap++ & 0xf];
                *cp++ = ':';
        }
        *--cp = 0;
        return (etherbuf);
}


struct nlist nl[] = {
#define	X_SRTAB	0
	{ "_G_srt_tab" },
#define	X_ARP_SRTAB	1
	{ "_arp_srtab" },
	{ "" },
};

/*
 * Dump the entire Source Routing table
 */
void 
 dump_table(kernel, mem)
	char *kernel, *mem;
{
	extern int h_errno;
	caddr_t *G_srt_tab, *arp_srtab;
	struct srtab *srtabent;
	struct arpsrtab *arpsrtabent;
	int bknum, mf, sz;
	int srt_nbckts = SRT_NBCKTS, arp_srt_nbcks = ARP_SRT_NBCKS;
	char *malloc();
	off_t lseek();

	if (nlist(kernel, nl) < 0 || nl[X_SRTAB].n_type == 0) {
		fprintf(stderr, "srconfig: %s bad namelist\n", kernel);
		exit(1);
	}
	mf = open(mem, O_RDONLY);
	if (mf < 0) {
		fprintf(stderr, "srconfig: cannot open %s\n", mem);
		exit(1);
	}

	sz = srt_nbckts * sizeof(caddr_t);
	G_srt_tab = (caddr_t *)malloc((u_int)sz);
	if (G_srt_tab == NULL) {
		printf("srconfig: can't get memory for srtab.\n");
		exit(1);
	}
	klseek(mf, (off_t)nl[X_SRTAB].n_value, L_SET);
	if (read(mf, (char *)G_srt_tab, sz) != sz) {
		perror("srconfig: error reading srtab");
		exit(1);
	}

	sz = sizeof(struct srtab);
	srtabent = (struct srtab *)malloc((u_int)sz);

	for (bknum = 0; bknum < srt_nbckts; bknum++) {
	    klseek(mf, (off_t)G_srt_tab[bknum], L_SET);
	    if (read(mf, (char *)srtabent, sz) != sz) {
		perror("srconfig: error reading srtab");
		exit(1);
	    }
	    while (srtabent)
	    {
		if (srtabent->srt_status != RIF_NOTUSE)
			dump_entry(srtabent);
		if (srtabent->srt_next_hash) {
			klseek(mf, (off_t)srtabent->srt_next_hash, L_SET);
	    		if (read(mf, (char *)srtabent, sz) != sz) {
				perror("srconfig: error reading srtab");
				exit(1);
	    		}
		} else
			break;
	    }	
	}

	sz = arp_srt_nbcks * sizeof(caddr_t);
	arp_srtab = (caddr_t *)malloc((u_int)sz);
	klseek(mf, (off_t)nl[X_ARP_SRTAB].n_value, L_SET);
	if (read(mf, (char *)arp_srtab, sz) != sz) {
		perror("srconfig: error reading arp_srtab");
		exit(1);
	}

	sz = sizeof(struct arpsrtab);
	arpsrtabent = (struct arpsrtab *)malloc((u_int)sz);

	for (bknum = 0; bknum < arp_srt_nbcks; bknum++) {
	    klseek(mf, (off_t)arp_srtab[bknum], L_SET);
	    if (read(mf, (char *)arpsrtabent, sz) != sz) {
		perror("srconfig: error reading arp_srtab");
		exit(1);
	    }
	    while (arpsrtabent)
	    {
		if (arpsrtabent->srt_status != RIF_NOTUSE)
			dump_arp_entry(arpsrtabent);
		if (arpsrtabent->srt_next_hash) {
			klseek(mf, (off_t)arpsrtabent->srt_next_hash, L_SET);
	    		if (read(mf, (char *)arpsrtabent, sz) != sz) {
				perror("srconfig: error reading arp_srtab");
				exit(1);
	    		}
		} else
			break;
	    }	
	}
	close(mf);
}


dump_entry(srtabent)
struct srtab *srtabent;
{
	ether_print(srtabent->srt_mac_addr);
	if (srtabent->srt_ip_addr.s_addr)
		printf("(ip = %s) ", inet_ntoa(srtabent->srt_ip_addr));
	switch (srtabent->srt_status) {
	case RIF_NOROUTE:
		printf(" No Route\n");
		break;
	case RIF_REDISCOVER:
		printf(" Rediscover\n");
		break;
	case RIF_ONRING:
		printf(" On Ring\n");
		break;
	case RIF_HAVEROUTE:
		printf(" Have Route\n");
		print_trnrif(&srtabent->srtab_rif);
		break;
	case RIF_STALE:
		if (srtabent->srtab_rif.rif_lth > 2)
		{
			printf(" Stale (Have Route)\n");
			print_trnrif(&srtabent->srtab_rif);
		}
		else
		if (srtabent->srtab_rif.rif_rt == SR_ALL_ROUTE_EXPLORE)
			printf(" Stale (incomplete)\n");
		else
			printf(" Stale (On Ring)\n");
		break;
	default:
		printf("unknow status\n");
		break;
	}
	printf("\n");
}
		

dump_arp_entry(arpsrtabent)
struct arpsrtab *arpsrtabent;
{
	printf("Target Node IP Address %s ", inet_ntoa(arpsrtabent->srt_ip_addr));
	switch (arpsrtabent->srt_status) {
	case RIF_NOROUTE:
		printf(" No Route\n");
		break;
	case RIF_REDISCOVER:
		printf(" Rediscover\n");
		break;
	case RIF_STALE:
		printf(" Stale (incomplete)\n");
		break;
	default:
		printf(" unknown status\n");
		break;
	}
}


/*
 * print out trn_rif structure 
 */
void
 print_trnrif(trnrif)
struct trn_rif *trnrif;
{
	union short_char {
        short   sh[8];
        u_char  uch[16];
    	} *sh_uch;

		printf("   Routing Information: ");
		switch (trnrif->rif_rt) {
		case SR_SPECIFIC_ROUTE:
			printf("SRF, ");
			break;
		case SR_ALL_ROUTE_EXPLORE:
			printf("ARE, ");
			break;
		case SR_SPAN_TREE_EXPLORE:
			printf("STE, ");
			break;
		default:
			printf("Error in Routing Type Definition\n");
			break;
		}
		printf("length %d, direction %d, largest frame ",
			trnrif->rif_lth,
			trnrif->rif_dir);
		switch (trnrif->rif_lf) {
		case TRN_LF_516:
			printf("516 octets\n");
			break;
		case TRN_LF_1500:
			printf("1500 octets\n");
			break;
		case TRN_LF_2052:
			printf("2502 octets\n");
			break;
		case TRN_LF_4472:
			printf("4472 octets\n");
			break;
		case TRN_LF_8144:
			printf("8144 octets\n");
			break;
		case TRN_LF_11407:
			printf("11407 octets\n");
			break;
		case TRN_LF_17800:
			printf("17800 octets\n");
			break;
		case TRN_LF_65535:
			printf("65535 octets\n");
			break;
		default:
			printf("Unknown Size\n");
			break;
		}
		sh_uch = (union short_char *)trnrif->rif_rd;
    		printf("     Route Descriptors: %02X%02X %02X%02X %02X%02X %02X%02X %02X% 02X %02X%02X %02X%02X %02X%02X\n",
        	sh_uch->uch[0], sh_uch->uch[1], sh_uch->uch[2], sh_uch->uch[3],
        	sh_uch->uch[4], sh_uch->uch[5], sh_uch->uch[6], sh_uch->uch[7],
        	sh_uch->uch[8],sh_uch->uch[9],sh_uch->uch[10],sh_uch->uch[11],
        	sh_uch->uch[12],sh_uch->uch[13],sh_uch->uch[14],sh_uch->uch[15]);

	return;
}


/*
 * Seek into the kernel for a value.
 */
klseek(fd, base, off)
	int fd, off;
	off_t base;
{
	off_t lseek();

	(void)lseek(fd, base, off);
}


ether_print(cp)
	u_char *cp;
{
	if (canonical)
		printf("Target Node MAC Address %02x-%02x-%02x-%02x-%02x-%02x ", cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
	else {
		haddr_convert(cp);
		printf("Target Node MAC Address %02x:%02x:%02x:%02x:%02x:%02x ", cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
	}
}


void
 read_counter()
{
    int s;
    struct srreq tab_counters;

    bzero(&tab_counters, sizeof(struct srreq));
    tab_counters.srreq_type = SR_RATTR;

    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    if ( ioctl(s, SIOCSRREQR, (caddr_t)&tab_counters) < 0 )
    {
	if (errno == ENODEV)
	    fprintf(stderr, "srconfig: source routing is not enabled\n");
	else
	{
	    fprintf(stderr, "srconfig: ");
            perror("SIOCSRREQR");
	}
        exit(1);
    }

    /* Output the counters */
    printf("\n  ARE Frames Sent          : %ld\n", tab_counters.srreq_data.attrs.ARE_sent);
    printf("  ARE Frames Received      : %ld\n", tab_counters.srreq_data.attrs.ARE_rcvd);
    printf("  Route Discovery Failures : %ld\n", tab_counters.srreq_data.attrs.SR_discover_fail);
    close(s);
    return;
}


void
 read_attr()
{
    int s;
    struct srreq tab_attr;
    int is_SR_enab();

    bzero(&tab_attr, sizeof(struct srreq));
    tab_attr.srreq_type = SR_RATTR;

    if (!is_SR_enab())
	exit(0);

    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    if ( ioctl(s, SIOCSRREQR, (caddr_t)&tab_attr) < 0 )
    {
	if (errno == ENODEV)
	    fprintf(stderr, "srconfig: source routing is not enabled\n");
	else
	{
	    fprintf(stderr, "srconfig: ");
            perror("SIOCSRREQR");
	}
        exit(1);
    }

    printf(" Current SR Aging Timer     : %d\n", tab_attr.srreq_data.attrs.SR_aging_timer);
    printf(" Current SR Discovery Timer : %d\n", tab_attr.srreq_data.attrs.SRDT_extended);
    printf(" Current SR Table size      : %d\n", tab_attr.srreq_data.attrs.SR_table_size);

    close(s);

    return;
}


void
 disable_entry(mac_add_p)
char *mac_add_p;
{
    u_char maddr[6];
    int s;
    struct srreq dis_req_tb;
 
    if (mac_aton(mac_add_p, maddr))
        return;

    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    bcopy(maddr, dis_req_tb.srreq_data.srtab.srt_mac_addr, 6);
    dis_req_tb.srreq_type = SR_DISENT;

    if ( ioctl(s, SIOCSRREQW, (caddr_t)&dis_req_tb) )
    {
        if (errno == ENXIO)
            printf("%s : Address not found in the SR database\n", mac_add_p);
        else
	    if (errno == ENODEV)
	       fprintf(stderr, "srconfig: source routing is not enabled\n");
	    else
	    {
	       fprintf(stderr, "srconfig: ");
               perror("SIOCSRREQW");
	    }
        exit(1);
    } else
	printf("srconfig: %s SR entry disabled\n", mac_add_p);
/* SHOULD WE NOTIFY THE USER ON THE SUCCESS OF THE COMMAND ??? */

    close(s);

    return;
}

void
 delete_entry(mac_add_p)
char *mac_add_p;
{
    u_char maddr[6];
    int s;
    struct srreq del_req_tb;

    if (mac_aton(mac_add_p, maddr))
       return;

    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    bcopy(maddr, del_req_tb.srreq_data.srtab.srt_mac_addr, 6);
    del_req_tb.srreq_type = SR_DELENT;

    if ( ioctl(s, SIOCSRREQW, (caddr_t)&del_req_tb) )
    {
        if (errno == ENXIO)
            printf("%s : Address not found in the SR database\n", mac_add_p);
        else
	    if (errno == ENODEV)
	       fprintf(stderr, "srconfig: source routing is not enabled\n");
	    else
	    {
	       fprintf(stderr, "srconfig: ");
               perror("SIOCSRREQW");
	    }
        exit(1);
    }
    else
	printf("srconfig: %s SR entry deleted\n", mac_add_p);

    close(s);

    return;
}


int
 is_SR_enab()
{
    int s, ret;
    struct srreq tb_sr_enab;

    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    tb_sr_enab.srreq_type = SR_ISENAB;

    if ( ioctl(s, SIOCSRREQR, (caddr_t)&tb_sr_enab) )
    {
	if (errno == ENODEV)
	{
		printf(" Source Routing is not enabled\n");
		ret = 0;
	}
	else
	{
		fprintf(stderr, "srconfig: ");
        	perror("SIOCSRREQR");
        	exit(1);
	}
    }
    else
    {
	printf(" Source Routing is enabled\n");
	ret = 1;
    }
    close(s);
    return(ret);
}


void
 enab_SR()
{
    int s;
    struct srreq tb_sr_enab;

    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    tb_sr_enab.srreq_type = SR_ENAB;

    if ( ioctl(s, SIOCSRREQW, (caddr_t)&tb_sr_enab) )
    {
	fprintf(stderr, "srconfig: ");
        perror("SIOCSRREQW");
        exit(1);
    }
    else
	printf("srconfig: Source Routing enabled\n");
    close(s);
    return;
}




void
 disab_SR()
{
    int s;
    struct srreq tb_sr_disab;

    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    tb_sr_disab.srreq_type = SR_DISAB;

    if ( ioctl(s, SIOCSRREQW, (caddr_t)&tb_sr_disab) )
    {
	fprintf(stderr, "srconfig: ");
        perror("SIOCSRREQW");
        exit(1);
    }
    else
	printf("srconfig: Source Routing disabled\n");
    close(s);
    return;
}


/*
 *				m a c _ a t o n 
 *
 */
mac_aton(a, n)
        char *a;
        u_char *n;
{
        int i, o[6];

        i = sscanf(a, "%x-%x-%x-%x-%x-%x", &o[0], &o[1], &o[2],
                                           &o[3], &o[4], &o[5]);
        if (i != 6) {
	    i = sscanf(a, "%x:%x:%x:%x:%x:%x", &o[0], &o[1], &o[2],
                                           &o[3], &o[4], &o[5]);
	    if (i != 6) {
                fprintf(stderr, "srconfig: invalid MAC address '%s'\n", a);
                return (1);
	    }
        }
        for (i=0; i<6; i++)
                n[i] = o[i];
	if (!canonical)
		haddr_convert(n);

        return (0);
}



void
 print_rif(p_rif)
struct trn_rif *p_rif;
{
    union short_char {
	short	sh[8];
	u_char	uch[16];
    } *sh_uch;

    printf("       Routing Type : ");
    switch (p_rif->rif_rt)
    {
	case SR_SPECIFIC_ROUTE:
	    printf("Specifically Routed Frame (SRF)\n");	        
	    break;

	case SR_ALL_ROUTE_EXPLORE:
	    printf("All Routes Explorer (ARE)\n");
	    break;

	case SR_SPAN_TREE_EXPLORE:
	    printf("Spanning Tree Explorer (STE)\n");
	    break;

	default:
	   printf("Error in Routing Type Definition\n");
	   break;
    }

    printf("       Length : %d\n", p_rif->rif_lth);
    printf("       Direction : %d\n", p_rif->rif_dir);
    printf("       Largest Frame : ");
    switch (p_rif->rif_lf)
    {
	case TRN_LF_516:
	    printf("516 octets\n");
	    break;

	case TRN_LF_1500:
	    printf("1500 octets\n");
	    break;

	case TRN_LF_2052:
	    printf("2052 octets\n");
   	    break;

	case TRN_LF_4472:
	    printf("4472 octets\n");
            break;

        case TRN_LF_8144:
	    printf("8144 octets\n");
            break;

        case TRN_LF_11407:
	    printf("11407 octets\n");
            break;

        case TRN_LF_17800:
	    printf("17800 octets\n");
            break;

        case TRN_LF_65535:
	    printf("65535 octets\n");
            break;

	default:
	    printf("Unknown Size\n");
	    break;
    }
 
    sh_uch = (union short_char *)p_rif->rif_rd;
    printf("       Route Descriptors : %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X\n",
	sh_uch->uch[0], sh_uch->uch[1], sh_uch->uch[2], sh_uch->uch[3], 
	sh_uch->uch[4], sh_uch->uch[5], sh_uch->uch[6], sh_uch->uch[7],
	sh_uch->uch[8], sh_uch->uch[9], sh_uch->uch[10], sh_uch->uch[11],
	sh_uch->uch[12], sh_uch->uch[13], sh_uch->uch[14], sh_uch->uch[15]);

    printf("       Route Descriptors : %04X, %04X, %04X, %04X, %04X, %04X, %04X, %04X \n",
		p_rif->rif_rd[0], p_rif->rif_rd[1], p_rif->rif_rd[2],	
		p_rif->rif_rd[3], p_rif->rif_rd[4], p_rif->rif_rd[5],	
		p_rif->rif_rd[6], p_rif->rif_rd[7] );	

    return;
}


zero_counter()
{
    int s;
    struct srreq ag_timer_attr;

    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    ag_timer_attr.srreq_type = SR_ZATTR;

    if ( ioctl(s, SIOCSRREQW, (caddr_t)&ag_timer_attr) < 0 )
    {
	if (errno == ENODEV)
	    fprintf(stderr, "srconfig: source routing is not enabled\n");
	else
	{
	    fprintf(stderr, "srconfig: ");
            perror("SIOCSRREQW");
	}
        exit(1);
    }
    else
	printf("srconfig: SR counter zeroed\n");
    close(s);
    return (0);
}
        

set_aging_timer(ag_timer)
char *ag_timer;
{
    int s, new_aging_timer;
    struct srreq ag_timer_attr;

    if ( (new_aging_timer=atoi(ag_timer)) <=1 )
    {
	printf("Illegal Aging Timer value.\n");
	return (1);
    }

    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    bzero(&ag_timer_attr, sizeof(struct srreq));
    ag_timer_attr.srreq_type = SR_RATTR;

    if ( ioctl(s, SIOCSRREQR, (caddr_t)&ag_timer_attr) < 0 )
    {
	if (errno == ENODEV)
	    fprintf(stderr, "srconfig: source routing is not enabled\n");
	else
	{
	    fprintf(stderr, "srconfig: ");
            perror("SIOCSRREQR");
	}
        exit(1);
    }
    printf(" Current SR Aging Timer is : %d\n", ag_timer_attr.srreq_data.attrs.SR_aging_timer);

    bzero(&ag_timer_attr, sizeof(struct srt_attr));
    ag_timer_attr.srreq_type = SR_SATTR;
    ag_timer_attr.srreq_data.attrs.SR_aging_timer = new_aging_timer;

    if ( ioctl(s, SIOCSRREQW, (caddr_t)&ag_timer_attr) < 0 )
    {
	if (errno == ENODEV)
	    fprintf(stderr, "srconfig: source routing is not enabled\n");
	else
	{
	    fprintf(stderr, "srconfig: ");
            perror("SIOCSRREQW");
	}
        exit(1);
    } else
    	printf(" New SR Aging Timer is : %d\n", new_aging_timer);
    close(s);
    return (0);
}
        

set_discovery_timer(disc_timer)
char *disc_timer;
{
    int s, new_disc_timer;
    struct srreq disc_timer_attr;

    if ( (new_disc_timer=atoi(disc_timer)) <=1 )
    {
	printf("Illegal Route Discovery Timer value.\n");
	return (1);
    }
	
    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    bzero(&disc_timer_attr, sizeof(struct srt_attr));
    disc_timer_attr.srreq_type = SR_RATTR;

    if ( ioctl(s, SIOCSRREQR, (caddr_t)&disc_timer_attr) < 0 )
    {
	if (errno == ENODEV)
	    fprintf(stderr, "srconfig: source routing is not enabled\n");
	else
	{
	    fprintf(stderr, "srconfig: ");
            perror("SIOCSRREQR");
	}
        exit(1);
    }
    printf(" Current SR Discovery Timer is : %d\n", disc_timer_attr.srreq_data.attrs.SRDT_extended);

    bzero(&disc_timer_attr, sizeof(struct srt_attr));
    disc_timer_attr.srreq_type = SR_SATTR;
    disc_timer_attr.srreq_data.attrs.SRDT_extended = new_disc_timer;

    if ( ioctl(s, SIOCSRREQW, (caddr_t)&disc_timer_attr) < 0 )
    {
	if (errno == ENODEV)
	    fprintf(stderr, "srconfig: source routing is not enabled\n");
	else
	{
	    fprintf(stderr, "srconfig: ");
            perror("SIOCSRREQW");
	}
        exit(1);
    }
    else 
    	printf(" New SR Discovery Timer is : %d\n", new_disc_timer);

    close(s);
    return (0);
}


set_SR_size(tab_size)
char *tab_size;
{
    int s, new_size;
    struct srreq tab_size_attr;

    if ( ((new_size=atoi(tab_size)) % SRT_NBCKTS) ||
	 (new_size <= 1024) || (new_size > 2048) )
    {
	fprintf(stderr, "Illegal SR Table size -- Size must be multiple of 256 and between 1024 and 2048.\n");
  	return (1);
    }

    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (s < 0)
    {
	    fprintf(stderr, "srconfig: ");
            perror("Error in socket call\n");
            exit(1);
    }

    bzero(&tab_size_attr, sizeof(struct srt_attr));
    tab_size_attr.srreq_type = SR_RATTR;

    if ( ioctl(s, SIOCSRREQR, (caddr_t)&tab_size_attr) < 0 )
    {
	if (errno == ENODEV)
	    fprintf(stderr, "srconfig: source routing is not enabled\n");
	else
	{
	    fprintf(stderr, "srconfig: ");
            perror("SIOCSRREQR");
	}
        exit(1);
    }
    printf(" Current SR Table size is : %d\n", tab_size_attr.srreq_data.attrs.SR_table_size);
    if (new_size <= tab_size_attr.srreq_data.attrs.SR_table_size)
    {
	printf(" Can't set size (%d) less than the current value.\n", new_size);
	exit(1);
    }

    bzero(&tab_size_attr, sizeof(struct srt_attr));
    tab_size_attr.srreq_type = SR_SATTR;
    tab_size_attr.srreq_data.attrs.SR_table_size = new_size;

    if ( ioctl(s, SIOCSRREQW, (caddr_t)&tab_size_attr) < 0 )
    {
	if (errno == ENODEV)
	    fprintf(stderr, "srconfig: source routing is not enabled\n");
	else
	{
	    fprintf(stderr, "srconfig: ");
	    perror("SIOCSRREQW");
	}
	exit(1);
    } 
    else
    	printf(" New SR Table size is : %d\n", new_size);
    close(s);
    return (0);
}

/*
 * haddr_convert()
 * --------------
 * 
 * Converts a backwards address to a canonical address and a canonical address
 * to a backwards address.
 *
 * Uses a sizeable translation table to do the conversion.  Longwords are used
 * in an attempt to make this routine fast since it will be called often.
 *
 * INPUTS:
 *  adr - pointer to six byte string to convert (unsigned char *)
 *
 * OUTPUTS:
 *  The string is updated to contain the converted address.
 *
 * CALLER:
 *  many
 *
 * USAGE OF OTHER ROUTINES:
 *
 */

unsigned long con_table[256] = {
0x00,   /* 0x00 */  0x80,   /* 0x01 */  0x40,   /* 0x02 */  0xc0,   /* 0x03 */
0x20,   /* 0x04 */  0xa0,   /* 0x05 */  0x60,   /* 0x06 */  0xe0,   /* 0x07 */
0x10,   /* 0x08 */  0x90,   /* 0x09 */  0x50,   /* 0x0a */  0xd0,   /* 0x0b */
0x30,   /* 0x0c */  0xb0,   /* 0x0d */  0x70,   /* 0x0e */  0xf0,   /* 0x0f */
0x08,   /* 0x10 */  0x88,   /* 0x11 */  0x48,   /* 0x12 */  0xc8,   /* 0x13 */
0x28,   /* 0x14 */  0xa8,   /* 0x15 */  0x68,   /* 0x16 */  0xe8,   /* 0x17 */
0x18,   /* 0x18 */  0x98,   /* 0x19 */  0x58,   /* 0x1a */  0xd8,   /* 0x1b */
0x38,   /* 0x1c */  0xb8,   /* 0x1d */  0x78,   /* 0x1e */  0xf8,   /* 0x1f */
0x04,   /* 0x20 */  0x84,   /* 0x21 */  0x44,   /* 0x22 */  0xc4,   /* 0x23 */
0x24,   /* 0x24 */  0xa4,   /* 0x25 */  0x64,   /* 0x26 */  0xe4,   /* 0x27 */
0x14,   /* 0x28 */  0x94,   /* 0x29 */  0x54,   /* 0x2a */  0xd4,   /* 0x2b */
0x34,   /* 0x2c */  0xb4,   /* 0x2d */  0x74,   /* 0x2e */  0xf4,   /* 0x2f */
0x0c,   /* 0x30 */  0x8c,   /* 0x31 */  0x4c,   /* 0x32 */  0xcc,   /* 0x33 */
0x2c,   /* 0x34 */  0xac,   /* 0x35 */  0x6c,   /* 0x36 */  0xec,   /* 0x37 */
0x1c,   /* 0x38 */  0x9c,   /* 0x39 */  0x5c,   /* 0x3a */  0xdc,   /* 0x3b */
0x3c,   /* 0x3c */  0xbc,   /* 0x3d */  0x7c,   /* 0x3e */  0xfc,   /* 0x3f */
0x02,   /* 0x40 */  0x82,   /* 0x41 */  0x42,   /* 0x42 */  0xc2,   /* 0x43 */
0x22,   /* 0x44 */  0xa2,   /* 0x45 */  0x62,   /* 0x46 */  0xe2,   /* 0x47 */
0x12,   /* 0x48 */  0x92,   /* 0x49 */  0x52,   /* 0x4a */  0xd2,   /* 0x4b */
0x32,   /* 0x4c */  0xb2,   /* 0x4d */  0x72,   /* 0x4e */  0xf2,   /* 0x4f */
0x0a,   /* 0x50 */  0x8a,   /* 0x51 */  0x4a,   /* 0x52 */  0xca,   /* 0x53 */
0x2a,   /* 0x54 */  0xaa,   /* 0x55 */  0x6a,   /* 0x56 */  0xea,   /* 0x57 */
0x1a,   /* 0x58 */  0x9a,   /* 0x59 */  0x5a,   /* 0x5a */  0xda,   /* 0x5b */
0x3a,   /* 0x5c */  0xba,   /* 0x5d */  0x7a,   /* 0x5e */  0xfa,   /* 0x5f */
0x06,   /* 0x60 */  0x86,   /* 0x61 */  0x46,   /* 0x62 */  0xc6,   /* 0x63 */
0x26,   /* 0x64 */  0xa6,   /* 0x65 */  0x66,   /* 0x66 */  0xe6,   /* 0x67 */
0x16,   /* 0x68 */  0x96,   /* 0x69 */  0x56,   /* 0x6a */  0xd6,   /* 0x6b */
0x36,   /* 0x6c */  0xb6,   /* 0x6d */  0x76,   /* 0x6e */  0xf6,   /* 0x6f */
0x0e,   /* 0x70 */  0x8e,   /* 0x71 */  0x4e,   /* 0x72 */  0xce,   /* 0x73 */
0x2e,   /* 0x74 */  0xae,   /* 0x75 */  0x6e,   /* 0x76 */  0xee,   /* 0x77 */
0x1e,   /* 0x78 */  0x9e,   /* 0x79 */  0x5e,   /* 0x7a */  0xde,   /* 0x7b */
0x3e,   /* 0x7c */  0xbe,   /* 0x7d */  0x7e,   /* 0x7e */  0xfe,   /* 0x7f */
0x01,   /* 0x80 */  0x81,   /* 0x81 */  0x41,   /* 0x82 */  0xc1,   /* 0x83 */
0x21,   /* 0x84 */  0xa1,   /* 0x85 */  0x61,   /* 0x86 */  0xe1,   /* 0x87 */
0x11,   /* 0x88 */  0x91,   /* 0x89 */  0x51,   /* 0x8a */  0xd1,   /* 0x8b */
0x31,   /* 0x8c */  0xb1,   /* 0x8d */  0x71,   /* 0x8e */  0xf1,   /* 0x8f */
0x09,   /* 0x90 */  0x89,   /* 0x91 */  0x49,   /* 0x92 */  0xc9,   /* 0x93 */
0x29,   /* 0x94 */  0xa9,   /* 0x95 */  0x69,   /* 0x96 */  0xe9,   /* 0x97 */
0x19,   /* 0x98 */  0x99,   /* 0x99 */  0x59,   /* 0x9a */  0xd9,   /* 0x9b */
0x39,   /* 0x9c */  0xb9,   /* 0x9d */  0x79,   /* 0x9e */  0xf9,   /* 0x9f */
0x05,   /* 0xa0 */  0x85,   /* 0xa1 */  0x45,   /* 0xa2 */  0xc5,   /* 0xa3 */
0x25,   /* 0xa4 */  0xa5,   /* 0xa5 */  0x65,   /* 0xa6 */  0xe5,   /* 0xa7 */
0x15,   /* 0xa8 */  0x95,   /* 0xa9 */  0x55,   /* 0xaa */  0xd5,   /* 0xab */
0x35,   /* 0xac */  0xb5,   /* 0xad */  0x75,   /* 0xae */  0xf5,   /* 0xaf */
0x0d,   /* 0xb0 */  0x8d,   /* 0xb1 */  0x4d,   /* 0xb2 */  0xcd,   /* 0xb3 */
0x2d,   /* 0xb4 */  0xad,   /* 0xb5 */  0x6d,   /* 0xb6 */  0xed,   /* 0xb7 */
0x1d,   /* 0xb8 */  0x9d,   /* 0xb9 */  0x5d,   /* 0xba */  0xdd,   /* 0xbb */
0x3d,   /* 0xbc */  0xbd,   /* 0xbd */  0x7d,   /* 0xbe */  0xfd,   /* 0xbf */
0x03,   /* 0xc0 */  0x83,   /* 0xc1 */  0x43,   /* 0xc2 */  0xc3,   /* 0xc3 */
0x23,   /* 0xc4 */  0xa3,   /* 0xc5 */  0x63,   /* 0xc6 */  0xe3,   /* 0xc7 */
0x13,   /* 0xc8 */  0x93,   /* 0xc9 */  0x53,   /* 0xca */  0xd3,   /* 0xcb */
0x33,   /* 0xcc */  0xb3,   /* 0xcd */  0x73,   /* 0xce */  0xf3,   /* 0xcf */
0x0b,   /* 0xd0 */  0x8b,   /* 0xd1 */  0x4b,   /* 0xd2 */  0xcb,   /* 0xd3 */
0x2b,   /* 0xd4 */  0xab,   /* 0xd5 */  0x6b,   /* 0xd6 */  0xeb,   /* 0xd7 */
0x1b,   /* 0xd8 */  0x9b,   /* 0xd9 */  0x5b,   /* 0xda */  0xdb,   /* 0xdb */
0x3b,   /* 0xdc */  0xbb,   /* 0xdd */  0x7b,   /* 0xde */  0xfb,   /* 0xdf */
0x07,   /* 0xe0 */  0x87,   /* 0xe1 */  0x47,   /* 0xe2 */  0xc7,   /* 0xe3 */
0x27,   /* 0xe4 */  0xa7,   /* 0xe5 */  0x67,   /* 0xe6 */  0xe7,   /* 0xe7 */
0x17,   /* 0xe8 */  0x97,   /* 0xe9 */  0x57,   /* 0xea */  0xd7,   /* 0xeb */
0x37,   /* 0xec */  0xb7,   /* 0xed */  0x77,   /* 0xee */  0xf7,   /* 0xef */
0x0f,   /* 0xf0 */  0x8f,   /* 0xf1 */  0x4f,   /* 0xf2 */  0xcf,   /* 0xf3 */
0x2f,   /* 0xf4 */  0xaf,   /* 0xf5 */  0x6f,   /* 0xf6 */  0xef,   /* 0xf7 */
0x1f,   /* 0xf8 */  0x9f,   /* 0xf9 */  0x5f,   /* 0xfa */  0xdf,   /* 0xfb */
0x3f,   /* 0xfc */  0xbf,   /* 0xfd */  0x7f,   /* 0xfe */  0xff,   /* 0xff */
} ;

void 
haddr_convert(addr)
u_char *addr;
{
  u_long i;

  for (i=0; i<6; i++) {
        *addr = con_table[*addr] & 0xFF;
        addr++;
  }; 
}
