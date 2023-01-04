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
static char *rcsid = "@(#)$RCSfile: latcp.c,v $ $Revision: 1.1.11.4 $ (DEC) $Date: 1993/11/23 22:12:53 $";
#endif

#include <fcntl.h>
#include <dec/lat/lat.h>
#include <latcp.h>
#include <stropts.h> 
#include <nlist.h>
#include <sys/stream.h>
#include <sys/utsname.h>
#include <sys/syslog.h>
#include <sys/strkinfo.h>
#include <string.h> 
#include <sys/types.h> 

#include <stdio.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sys/tty.h>
#include <sys/stat.h>
#include <machine/hal_sysinfo.h>


static char    usage[] = {"     where option is one of the following:\n \
\t-s \n \
\t-h\n \
\t-A -a service [-i descript] [-o]\n \
\t-A -p tty -H node {-R remote_port | -V service_name} [-Q]\n \
\t-D {-a service | -p tty}\n \
\t-i descript -a service\n \
\t-g list -a service\n \
\t-G list -a service\n \
\t-x rating -a service\n \
\t-n node\n \
\t-m time\n \
\t-d [ -C | -N | -S | -P [-p tty | -L | -I] ]\n \
\t-e adaptor\n \
\t-E adaptor\n \
\t-z \n \
\t-r\n"};


int  showflag = -1;     /* display port characteristics */
int  grpmask[] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
int  fd;
int  optflag = -1;     /* Initialized for later verification. */
struct  utsname utsname;

main(argc,argv)
int  argc;
char **argv;
{

     extern char       *optarg;
     extern int        optind;
     extern int        opterr;

     char        *descript = NULL;
     char        *devname = NULL;
     char        *grpstr = NULL;
     char        *port_name = NULL;
     char        *rem_node_nm = NULL;
     char        *rem_prt_nm = NULL;
     char        *rem_svc_nm = NULL;
     char        svcname[LAT_MAXNAME+1];
     char        dev_tty[LAT_MAXNAME+1];
     char        ttys[LAT_MAXNAME+1];
     char        sdev[LATCPDEV_LEN];
     char 	 desc_str[80];
     char 	 get_str[80];
     int         adaptor_len, host_len, str_len, i, errflg, port_num = -1;
     int         Aflag, Dflag, oflag, gflag, Gflag, xflag;
     int         iflag, aflag, Pflag, Lflag, Iflag, Qflag;
     int         pflag, Rflag, Vflag, Hflag, dflag, trunflag; 
     int         optno, c, group, ind, bitind, noterm = ~TERMINAL_FOUND;
     int 	 link_added = 0;
     unsigned short	mtimer = 0; 
     long	stat_rating = DYNAMIC_RATING;

     int         ret, num, service_found,link_found,adapt_found;
     unsigned int	save_node_flags;
     char	 default_service_index_found;
     char        nodename[LAT_MAXNAME+1]; 
     char        adapterinx[LAT_MAXNAME+1]; 
     char        *nodeptr = NULL; 
     char        *charptr = NULL; 
     char        hostname[LAT_MAXNAME+1]; 
     char        adaptorname[LAT_MAXNAME+1]; 
     char        servicename[LAT_MAXNAME+1];
     char        *lh_ifnameptr = NULL;
     char        *adaptor_nameptr = NULL;
     char        *ls_nameptr = NULL;
     char        *ln_nameptr = NULL;
     char 	 *str1=NULL;
     char 	 *str2=NULL;

     struct  latioctl_service latioctl_service;
     struct  latioctl_node latioctl_node;
     struct  latioctl_link latioctl_link;

     struct latioctl_port latioctl_port,get_port;
     struct latioctl_host latioctl_host;
     struct stat statbuf;

#define BOOTEDFILELEN 80
     char bootedfile[BOOTEDFILELEN];

     if_names_blk ifblk; /* interface info blk in strkinfo.h */
     
     struct nlist nl[] = {
             { "lat_open" },
             "",
     };

     optno = errflg = c = 0;
     Aflag = Dflag = oflag = gflag = Gflag = xflag = 0;
     iflag = aflag = Pflag = Lflag = Iflag = Qflag = 0;
     pflag = Rflag = Vflag = Hflag = dflag = trunflag = 0;

     if (argc < 2)     {
           /* must be latcp and at least 1 argument for */
	   /*  at least a total of 2 argument count */
	   fprintf(stderr,"Usage: %s      {option}\n",argv[0]);
           fprintf(stderr, "%s", usage);
           exit(2);
     }

     /* Get the name of the running kernel. */
     bootedfile[0] = '/';
     getsysinfo(GSI_BOOTEDFILE, &bootedfile[1], BOOTEDFILELEN, 0, 0);

     /* Determine if LAT is in the running kernel. */
     if ( nlist(bootedfile, nl) < 0) {
             fprintf(stderr, "Could not find kernel %s.\n", bootedfile);
             exit(2);
     }
     if ( nl[0].n_type == 0 ) {
             fprintf(stderr,
                     "LAT has not been built into the running kernel, %s.\n",
		     bootedfile);
             fprintf(stderr,
                     "latcp cannot be executed.\n");
             fprintf(stderr, "\n");
             exit(2);
     }

     while ((c = getopt(argc, argv, OPTSTR)) != EOF) {
           optno++;

           switch (c) {
           case 's':
                 optflag = CPSTART;
                 break;
           case 'h':
                 optflag = CPSTOP;
                 break;
           case 'd': 
                 optflag = CPSHOW_CHAR;
                 dflag = 1;
                 break;
           case 'N':
                 if ( !dflag ) {
                       fprintf(stderr,"Invalid usage. -N requires preceding -d option.\n");
                       exit(2);
                 }
                 optflag = CPSHOW_NODE;
                 break;
           case 'S':
                 if ( !dflag ) {
                       fprintf(stderr,"Invalid usage. -S requires preceding -d option.\n");
                       exit(2);
                 }
                 optflag = CPSHOW_SRVR;
                 break;
           case 'C':
                 if ( !dflag ) {
                       fprintf(stderr,"Invalid usage. -C requires preceding -d option.\n");
                       exit(2);
                 }
                 optflag = CPSHOW_CB_CTR;
                 break;
           case 'P':
                 if ( !dflag ) {
                       fprintf(stderr,"Invalid usage. -P requires preceding -d option.\n");
                       exit(2);
                 }
                 Pflag = 1;
                 optflag = CPSHOW_PRT;
                 break;
           case 'p':
                 if (!Pflag && !Aflag && !Dflag) {
                       fprintf(stderr,
			 "Invalid usage. -p requires preceding -P, -A or -D option.\n");
                       exit(2);
                 }
                 if (Dflag)
                       optflag = CPDEL_PRT;
                 if (Aflag && !aflag)
                       optflag = CPSET_PRT;
                 if (Pflag)
                       optflag = CPSHOW_PRT;
                 pflag = 1;
                       port_name = optarg;
                 break;
           case 'L':
                 if (!Pflag) {
                       fprintf(stderr,"Invalid usage. -L requires preceding -P option.\n");
                       exit(2);
                 }
                 Lflag = 1;
                 break;  
           case 'I':
                 if (!Pflag) {
                       fprintf(stderr,"Invalid usage. -I requires preceding -P option.\n");
                       exit(2);
                 }
                 Iflag = 1;
                 break;
           case 'z':
                 optflag = CPZERO_CTR;
                 break;
           case 'g':
                 gflag = 1;
                 optflag = CPSET_GRPS;
                 grpstr = optarg;
                 break;
           case 'G':
                 Gflag = 1;
                 optflag = CPDEL_GRPS;
                 grpstr = optarg;
                 break;
           case 'm':
                 optflag = CPSET_MULTI;
                 charptr = optarg;
                 if (check_number(charptr)) {
                       fprintf(stderr,"Invalid multicast interval. Valid multicast intervals are (10 - 180).\n");
                       exit(2);
                 }
                 mtimer = (unsigned short)(atoi(optarg));
                 break;
           case 'n':
                 optflag = CPSET_NODE;
                 str_len = strlen(optarg);
                 if ( str_len > LAT_MAXNAME ) {
                       fprintf(stderr, "Node name %s is too long. Maximum name lenth is %d\n", optarg, LAT_MAXNAME);
                       exit(2);
                 }
                 strcpy(nodename, optarg);
                 break;
           case 'A':
                 Aflag = 1;
                 break;
           case 'a':
                 if (!Aflag && !Dflag && !gflag && !Gflag && !xflag && !iflag) {
                       fprintf(stderr,
                           "Invalid usage. -a requires preceding -A, -D, -g, -G, -x, or -i option.\n");
                       exit(2);
                 }

                 if (Dflag)    
                       optflag = CPDEL_SVC;
                 if (Aflag || iflag)
                       optflag = CPSET_SVC;
                 aflag = 1;
                 str_len = strlen(optarg);
                 if ( str_len > LAT_MAXNAME ) {
                       fprintf(stderr, "Service name %s is too long. Maximum name length is %d\n", optarg, LAT_MAXNAME);
                       exit(2);
                 }
                 strcpy(svcname,optarg);
                 break;
           case 'i':
                 optflag = CPSET_SVC;
                 iflag = 1;
                 descript = optarg;
                 strcpy(desc_str, optarg);
                 break;
           case 'o':
                 if (!Aflag || !aflag) {
                       fprintf(stderr, "Invalid usage. -o requires preceding -A, and -a option.\n");
                       exit(2);
                 }
                 optflag = CPSET_SVC;
                 oflag = 1;
                 break;
           case 'x':
                 xflag = 1;
                 optflag = CPSET_RATING;
                 strcpy(get_str, optarg);
                 stat_rating = (unsigned long)(atol(optarg));
                 break;
           case 'D':
                 Dflag = 1;
                 break;
           case 'H':
                 if (!Aflag) {
                       fprintf(stderr,"Invalid usage. -H requires preceding -A option.\n");
                       exit(2);
                 }
                 Hflag = 1;
                 optflag = CPSET_PRT;
                 rem_node_nm = optarg;
                 break;
           case 'R':
                 if (!Aflag) {
                       fprintf(stderr,"Invalid usage. -R requires preceding -A option.\n");
                       exit(2);
                 }
                 Rflag = 1;
                 optflag = CPSET_PRT;
                 rem_prt_nm = optarg;
                 break;
           case 'V':
                 if (!Aflag) {
                       fprintf(stderr,"Invalid usage. -V requires preceding -A option.\n");
                       exit(2);
                 }
                 Vflag = 1;
                 optflag = CPSET_PRT;
                 rem_svc_nm = optarg;
                 break;
           case 'Q':
                 if (!Aflag) {
                       fprintf(stderr,"Invalid usage. -Q requires preceding -A option.\n");
                       exit(2);
                 }
                 Qflag = 1;
                 optflag = CPSET_PRT;
                 break;
           case 'e':
                 optflag = CPSET_ADAPT;
                 adaptor_len = strlen(optarg);
                 if ( adaptor_len > LAT_MAXNAME ) {
                       fprintf(stderr, "Invalid adaptor name %s.\n", optarg);
                       exit(2);
                 }
                 strcpy(adaptorname, optarg);
		 break;
           case 'E':
                 optflag = CPDEL_ADAPT;
                 adaptor_len = strlen(optarg);
                 if ( adaptor_len > LAT_MAXNAME ) {
                       fprintf(stderr, "Invalid adaptor name %s.\n", optarg);
                       exit(2);
                 }
                 strcpy(adaptorname, optarg);
                 break;
           case 'r': 
                 optflag = CPRESET;
                 break;
           case '?':
                 errflg++;
           }
     }   /* end of while (c = getopt())  */

     if ((errflg) | (optno < 1) | (argc != optind) )   {
	   fprintf(stderr,"Usage: %s      {option}\n",argv[0]);
           fprintf(stderr, "%s", usage);
           exit (2);
     }

     if (!dflag) {
	   if ( getuid() != 0 ) {
		fprintf(stderr, "You must su to root before executing this latcp command.\n\n");
		exit(2);
	   }
     }

     if ( (fd = open(LATCPDEV, 2)) < 0 )  {
	   perror(LATCPDEV);
           exit(2);
     }

     uname(&utsname);		/* get information about running kernel */

     switch (optflag)  {

     case CPSTART:	/* start LAT:  latcp -s  */
           if ( chk_cmd_line(optno) < 0 )
                exit(2);

	   /* First do a get node and save information to use later when doing
 	   /* the set node. Check if nodename defined, if not defined set
	   /*  hostname as default nodename */
	   if (ioctl(fd, LIOCGNODE, &latioctl_node) < 0){
		fprintf(stderr, "LIOCGNODE ioctl failed. errno=%d.\n", errno);
		exit(2);
     	   }
	   if (!(latioctl_node.ln_flags&LATIOCTL_NODE_NAME)){
	   /* set nodename from hostname */
		if ((host_len=strlen(utsname.nodename))> LAT_MAXNAME){ 
			trunflag = 1;
			host_len=LAT_MAXNAME; 
		} 
           	if ( name_check(utsname.nodename, host_len) ) {
                 	fprintf(stderr, "Invalid node name %s.\n", utsname.nodename);
                 	exit(2);
           	}
		strncpy((char *)&latioctl_node.ln_name[0],utsname.nodename,host_len); 
		latioctl_node.ln_name[host_len] = NULL;
           	makeupper(&latioctl_node.ln_name[0], host_len);
		if((charptr=strchr((char *)&latioctl_node.ln_name[0],'.')) !=NULL) {
			*charptr='\0';
			fprintf(stdout,"LAT node name set to %s.\n",&latioctl_node.ln_name[0]);
                }
                else if (trunflag) {
                        fprintf(stdout,"Default LAT node name would be greater than %d characters in length.\n  Node name truncated to %s.\n",LAT_MAXNAME,&latioctl_node.ln_name[0
]);
                        trunflag = 0;
                }

	   }

	   /* initialize flags and set nodename if not set */
	   latioctl_node.ln_flags = 0;
	   latioctl_node.ln_flags |= LATIOCTL_NODE_NAME;
	   if (latioctl_node.ln_satimer<MIN_MULTI_TIMER
		||latioctl_node.ln_satimer>MAX_MULTI_TIMER){ 
	   	/* setup default multitimer */
	   	latioctl_node.ln_satimer = LAT_MULTI_TIMER;
	   	latioctl_node.ln_flags |= LATIOCTL_NODE_SATIMER ;
	   }

	   /* setup default node descriptor and max number of services */
	   /* and hosts allowed */
	   strcpy((char *)&latioctl_node.ln_desc[0],OSNAME);
	   strcat((char *)&latioctl_node.ln_desc[0],utsname.release);
	   strcat((char *)&latioctl_node.ln_desc[0],NODEID);
	   latioctl_node.ln_maxservices = MAX_SERVICES;
	   latioctl_node.ln_maxhosts = MAX_HOSTS;
	   latioctl_node.ln_flags |= LATIOCTL_NODE_DESC
		|LATIOCTL_NODE_MAXHOSTS|LATIOCTL_NODE_MAXSERVICES  ;
           if (ioctl(fd, LIOCSNODE, &latioctl_node) < 0) {
                if (errno == EINVAL)
                        fprintf(stderr, "Invalid argument received for LIOCSNODE ioctl.\n");
                else if (errno == EACCES)
			fprintf (stderr, "No privileges for attempted operation.\n");
               	else
			fprintf(stderr, "LIOCSNODE ioctl failed. errno=%d.\n",errno);
		exit(2);
	   }

           /* Check if service name or names defined, if none defined set */
	   /* nodename as default servicename */
     	   /* Must index thru latioctl_service structures to get each */
	   /* service's information */
       	   service_found=0;
	   bzero((char *)&latioctl_service, sizeof(struct latioctl_service));
	   for (i=0; i<latioctl_node.ln_maxservices ;i++){
		latioctl_service.ls_index= i; /* int to char ok */
		if (ioctl(fd, LIOCGSERVICE, &latioctl_service) < 0) {
			if (errno != ENOENT) {
				fprintf(stderr, "LIOCGSERVICE ioctl failed. errno=%d.\n", errno);
				exit(2);
     	   		}
		}
		else{
			service_found++;
			break;
		}
	   }

	   if (!service_found ){ /* take nodename as default service name */
          	/* Set service name with node name*/
		/* and set default service description */
	   	latioctl_service.ls_flags = LATIOCTL_SERVICE_NAME
			|LATIOCTL_SERVICE_DESC|LATIOCTL_SERVICE_DEFAULT;
		strcpy((char *)&latioctl_service.ls_name[0],(char *)&latioctl_node.ln_name[0]);
	 	strcpy((char *)&latioctl_service.ls_desc[0],OSNAME);
		strcat((char *)&latioctl_service.ls_desc[0],utsname.release);
		strcat((char *)&latioctl_service.ls_desc[0],SVCID);
                lioc_aservice(fd, latioctl_service);
	   }

           /* Chk if adaptor/adaptors defined */
	   /* Get interface information from the kernel using pseudo driver kinfo */
	   /*    CALL TO KINFO */
	   get_kinfo(&ifblk);

	   /* and get the adaptor name. */ 
	   /* check if adaptor defined */
	   link_found=0;

	   for (i=0; i<(ifblk.curr_cnt); i++){
		sprintf(&latioctl_link.ll_ifname[0],"%s%d", ifblk.info[i].if_name,ifblk.info[i].if_unit);
		if ( ioctl(fd, LIOCGLINK, &latioctl_link) < 0) {
			if (errno != EADDRNOTAVAIL) {
                     		if (errno == ENXIO) 
                       			fprintf (stderr, "No such adaptor received for LIOCGLINK ioctl.\n");
				else 
					fprintf(stderr, "LIOCGLINK ioctl failed. errno=%d.\n", errno);
			        exit(2);
			}
                }
		else{
			link_found++;
			break;
     		}
	   }
	   	
	   if (!link_found){
		link_added = 0;		/* no adaptors added yet */
	   /* if no adaptor defined, get them from kinfo ifblk*/
	   /* and add links */
		for (i=0; i<(ifblk.curr_cnt); i++){
			sprintf(&latioctl_link.ll_ifname[0],"%s%d",
				ifblk.info[i].if_name,ifblk.info[i].if_unit);
           		if ( ioctl(fd, LIOCALINK, &latioctl_link) < 0){
                		if (errno == EPFNOSUPPORT)
					continue;	
                		else if (errno == ENXIO) 
               	       			fprintf (stderr, "No such adaptor received for LIOCALINK ioctl.\n");
                		else if (errno == EACCES) 
                       			fprintf (stderr, "No privileges for attempted operation.\n");
				else if (errno == EADDRINUSE) 
                        		fprintf (stderr, "The link is already defined. Link not added.\n");
                		else if (errno == ENOBUFS)
                       			fprintf(stderr, "No buffer available. errno=%d.\n",errno);
				else
                       			fprintf(stderr, "LIOCALINK ioctl failed. errno=%d.\n", errno);
               			exit(2);
			}	
			link_added++;	/* successfully configured adaptor */
		}
	   }
	  

           /* Start up LAT.  */
	   if (link_added | link_found) {
	   	latioctl_node.ln_flags = LATIOCTL_NODE_STATE;
	   	latioctl_node.ln_state = LAT_STATE_ON;
           	if (ioctl(fd, LIOCSNODE, &latioctl_node) < 0) {
                	fprintf(stderr, "WARNING: LAT not started.");
                	if (errno == EINVAL)
                       		fprintf(stderr, "Invalid argument received for LIOCSNODE ioctl.\n");
                 	else if (errno == EACCES)
				fprintf (stderr, "No privileges for attempted operation.\n");
               	 	else
				fprintf(stderr, "LIOCSNODE ioctl failed. errno=%d.\n", errno);
		 	exit(2);
           	}
           	fprintf(stdout, "LAT started.\n");
	   }
	   else {
		fprintf(stderr, "LAT not started - no valid adaptors available.\n");
		exit(2);
	   }
           break;



     case CPSTOP:	/* Stop driver:   latcp -h */
           if ( chk_cmd_line(optno) < 0 )
                 exit(2);

           fprintf(stdout, "\nAttempting to halt the driver. Please wait.....\n");
           /* Stop LAT.  */
	   latioctl_node.ln_flags = LATIOCTL_NODE_STATE;
	   latioctl_node.ln_state = LAT_STATE_OFF;
           if (ioctl(fd, LIOCSNODE, &latioctl_node) < 0) {
                 if (errno == EINVAL)
                        fprintf(stderr, "Invalid argument received for LIOCSNODE ioctl.\n");
                 else if (errno == EACCES)
			fprintf (stderr, "No privileges for attempted operation.\n");
               	 else
			fprintf(stderr, "LIOCSNODE ioctl failed. errno=%d.\n",
				 errno);
                 fprintf(stderr, "LAT not stopped.\n");
                 exit(2);
           }
           fprintf(stdout, "\nLAT stopped.\n");
           break;


     case CPSET_NODE:	/* Change or set nodename:   latcp -n nodename */

           if ( chk_cmd_line(optno) < 0 )
                 exit(2);
           if ( name_check(nodename, str_len) ) {
                 fprintf(stderr, "Invalid node name %s.\n", nodename);
                 exit(2);
           }
           makeupper(nodename, str_len);

	   strcpy((char *)&latioctl_node.ln_name[0],nodename);
	   latioctl_node.ln_flags = LATIOCTL_NODE_NAME;
	   if (ioctl(fd, LIOCSNODE, &latioctl_node) < 0){
                 if (errno == EINVAL)
                        fprintf(stderr, "Invalid argument received for LIOCSNODE ioctl.\n");
                 else if (errno == EACCES)
			fprintf (stderr, "No privileges for attempted operation.\n");
               	 else
			fprintf(stderr, "LIOCSNODE ioctl failed. errno=%d.\n",
				 errno);
                 exit(2);
           }
           break;

     case CPSHOW_CB_CTR:	/* Display server characteristics:  latcp -d -S */
     case CPSHOW_SRVR:		/* Display server counters:  latcp -d -C */
           if ( chk_cmd_line(optno - dflag) < 0 )
                 exit(2);

           if (optflag == CPSHOW_SRVR)
                 dmp_cbs();
           else
                 if (optflag == CPSHOW_CB_CTR)
			dmp_counters();
           break;

     case CPRESET:		/* Reset node & service names & timer:  latcp -B  */
	   /* reset multicast timer, nodename & desc, and servicename & desc */
           if ( chk_cmd_line(optno) < 0 )
                 exit(2);

	   if ((host_len=strlen(utsname.nodename))> LAT_MAXNAME){
		trunflag = 1;
		host_len=LAT_MAXNAME;
	   }
           if ( name_check(utsname.nodename, host_len) ) {
               	fprintf(stderr, "Invalid node name %s.\n", utsname.nodename);
               	exit(2);
           }
	   strncpy((char *)&latioctl_node.ln_name[0],utsname.nodename,host_len); 
	   latioctl_node.ln_name[host_len] = NULL;
           makeupper(&latioctl_node.ln_name[0], host_len);
	   if((charptr=strchr((char *)&latioctl_node.ln_name[0],'.')) !=NULL) {
		*charptr='\0';
		fprintf(stdout,"LAT node name set to %s.\n",&latioctl_node.ln_name[0]);
           }
           else if (trunflag) {
                fprintf(stdout,"Default LAT node name would be greater than %d characters in length.\n  Node name truncated to %s.\n",LAT_MAXNAME,&latioctl_node.ln_name[0
]);
                trunflag = 0;
           }


	   latioctl_node.ln_satimer = LAT_MULTI_TIMER;

	   latioctl_node.ln_flags = LATIOCTL_NODE_NAME|LATIOCTL_NODE_SATIMER;
	   if (ioctl(fd, LIOCSNODE, &latioctl_node) < 0){
                 if (errno == EINVAL)
                        fprintf(stderr, "Invalid argument received for LIOCSNODE ioctl.\n");
                 else if (errno == EACCES)
			fprintf (stderr, "No privileges for attempted operation.\n");
               	 else
			fprintf(stderr, "LIOCSNODE ioctl failed. errno=%d.\n",
				 errno);
                 exit(2);
           }


           /* Check if default service name defined, if defined keep the default */
	   /* servicename(nodename) and delete the other services defined */
	   default_service_index_found=-1;
	   strcpy((char *)&latioctl_service.ls_name[0],(char *)&latioctl_node.ln_name[0]);
	   latioctl_service.ls_flags = LATIOCTL_SERVICE_NAME;
	   if (ioctl(fd, LIOCGSERVICE, &latioctl_service) < 0){
		if (errno != ENOENT) {
			fprintf(stderr, "LIOCGSERVICE ioctl failed. errno=%d.\n", errno);
			exit(2);
     	   	}
	   }		
	   else{
		if (latioctl_service.ls_flags & LATIOCTL_SERVICE_DEFAULT) {
			/* If not optional, mark index so default service is
			 * not deleted and rating and desc are reset to defaults
			 * else if optional service then don't mark index
			 * and delete it with the other services./
	   		default_service_index_found=latioctl_service.ls_index;
                	/* reset rating to DYNAMIC */
	        	latioctl_service.ls_rating = DYNAMIC_RATING; 
	   		latioctl_service.ls_flags &= ~LATIOCTL_SERVICE_RATING_STATIC;
	   		latioctl_service.ls_flags &= ~LATIOCTL_SERVICE_DEFAULT;
	   		latioctl_service.ls_flags |= LATIOCTL_SERVICE_RATING_DYANMIC;
                	/* reset service description */
	   		strcpy((char *)&latioctl_service.ls_desc[0],OSNAME);
			strcat((char *)&latioctl_service.ls_desc[0],utsname.release);
			strcat((char *)&latioctl_service.ls_desc[0],SVCID);
	   		latioctl_service.ls_flags |= LATIOCTL_SERVICE_DESC;
                	lioc_sservice(fd, latioctl_service);
	   	}
	   }

     	   /* Must index thru latioctl_service structures to delete all */
	   /* services except if one with the nodename as the default servicename is found */
	   /* Get maxservices from node. */
	   latioctl_node.ln_flags = LATIOCTL_NODE_MAXSERVICES  ;
	   if (ioctl(fd, LIOCGNODE, &latioctl_node) < 0){
                fprintf(stderr, "LIOCGNODE ioctl failed. errno=%d.\n", errno);
                exit(2);
           }
	   latioctl_service.ls_flags = 0  ;
	   for (i=0; i<latioctl_node.ln_maxservices;i++){
		latioctl_service.ls_index= i;
	   	if (default_service_index_found!=latioctl_service.ls_index){
			if (ioctl(fd, LIOCDSERVICE, &latioctl_service) < 0){
				if (errno != ENOENT) {
                			if (errno == EACCES) {
                       				fprintf (stderr, "No privileges for attempted operation.\n");
                       				exit(2);
					}
					else{
						fprintf(stderr, "LIOCDSERVICE ioctl failed. errno=%d.\n", 
						errno);
						exit(2);
	   				}
     	   			}
			}
     	   	}
	   }

	   if (default_service_index_found == -1){ /* take nodename as default service name */
		
          	/* Set service name with node name*/
		/* and set default service description */
		latioctl_service.ls_flags = LATIOCTL_SERVICE_NAME
					   |LATIOCTL_SERVICE_DEFAULT
					   |LATIOCTL_SERVICE_DESC;
		strcpy((char *)&latioctl_service.ls_name[0],(char *)&latioctl_node.ln_name[0]);
	   	strcpy((char *)&latioctl_service.ls_desc[0],OSNAME);
		strcat((char *)&latioctl_service.ls_desc[0],utsname.release);
		strcat((char *)&latioctl_service.ls_desc[0],SVCID);
                lioc_aservice(fd, latioctl_service);
           }
	   break;

     case CPSET_ADAPT:	/* Set an adaptor:  latcp -e  */
           if ( chk_cmd_line(optno) < 0 )
                exit(2);

	   adaptor_name_check(adaptorname);

	   strcpy(&latioctl_link.ll_ifname[0],adaptorname);
	   if (ioctl(fd, LIOCALINK, &latioctl_link) < 0){
               	if (errno == ENXIO) 
               		fprintf (stderr, "No such adaptor received for LIOCALINK ioctl.\n");
               	else if (errno == EACCES) 
               		fprintf (stderr, "No privileges for attempted operation.\n");
		else if (errno == EADDRINUSE) 
                        fprintf (stderr, "The link is already defined. Link not added.\n");
                else if (errno == ENOBUFS)
                	fprintf(stderr, "No buffer available. errno=%d.\n",errno);
                else if (errno == EPFNOSUPPORT)
                	fprintf(stderr, "Protocol family not supported.\n");
		else
                	fprintf(stderr, "LIOCALINK ioctl failed. errno=%d.\n", errno);
                exit(2);
	   }
	   break;

     case CPDEL_ADAPT:	/* Delete an adaptor:  latcp -E  */
           if ( chk_cmd_line(optno) < 0 )
                exit(2);

	   adaptor_name_check(adaptorname);

	   strcpy(&latioctl_link.ll_ifname[0],adaptorname);
	   if (ioctl(fd, LIOCDLINK, &latioctl_link) < 0){
                if (errno == ENXIO) 
               	       fprintf (stderr, "No such adaptor received for LIOCDLINK ioctl.\n");
                else if (errno == EACCES) 
                       fprintf (stderr, "No privileges for attempted operation.\n");
                else if (errno == EADDRNOTAVAIL) 
                       fprintf (stderr, "The link is not defined. Link not deleted.\n");
                else
		       fprintf(stderr, "LIOCDLINK ioctl failed. errno=%d.\n", errno);
                exit(2);
	   }
	   break;


     case CPSHOW_CHAR:	 
           /* latcp -d : display node characteristics */
           if ( chk_cmd_line(optno) < 0 )
                 exit(2);

           dmp_nodechar();
	   break;
     case CPSHOW_NODE:  
           /* latcp -d -N : display node conunter */
           if ( chk_cmd_line(optno - dflag) < 0 )
                 exit(2);

           dmp_node_counters();
           break;

     case CPZERO_CTR: 
 	   /* latcp -z : set all counters to zero */ 
           if ( chk_cmd_line(optno) < 0 )
                 exit(2);
           zero_counter();
           break;

     case CPSET_GRPS:  
     case CPDEL_GRPS:   
           /*latcp -g 5 -a service1:enable service node group 5*/
           /*latcp -G 5 -a service1:disable service node group 5*/

           /* Check that no invalid options specified */
           optno = optno - gflag - Gflag - aflag;
           if (optno > 0) {
                 fprintf(stderr, "Invalid or duplicate option(s) specified.\n"); 
                 exit(2);
           }
           if ( chk_cmd_line(optno - aflag) < 0 )
                 exit(2);

           if ( !aflag ) {
                 fprintf(stderr, "A service name is required for this option.\n");
                 exit(2);
           }

           if (name_check(svcname, strlen(svcname))) {
                 fprintf(stderr, "Invalid service name %s.\n", svcname);
                 exit(2);
           }
	
	   set_groups(grpstr,svcname);

           break;

     case CPSET_MULTI:		
           /* latcp -m 70 : set multicast timer interval */
           if ( chk_cmd_line(optno) < 0 )
                 exit(2);

           if (mtimer < MIN_MULTI_TIMER || mtimer > MAX_MULTI_TIMER) {
                 fprintf(stderr,"Invalid multicast interval. Valid multicast intervals are (10 - 180)\n");
                 exit(2);
           }

	   bzero((char *)&latioctl_node, sizeof(latioctl_node));

	   latioctl_node.ln_satimer = (char)mtimer;
	   latioctl_node.ln_flags = LATIOCTL_NODE_SATIMER;
           if (ioctl(fd, LIOCSNODE, &latioctl_node) < 0) {
                 if (errno == EINVAL)
                       fprintf(stderr, "Invalid argument received for LIOCSNODE ioctl.\n");
                 else if (errno == EACCES)
			fprintf (stderr, "No privileges for attempted operation.\n");
                 else
                       fprintf(stderr, "LIOCSNODE ioctl failed. errno=%d.\n", errno);
	   }
           break;

     case CPSET_RATING: 	/* latcp -x132 -a serv1 : set the static rating to 132 */
           /* Check that no invalid options specified */
           optno = optno - xflag - aflag;
           if (optno > 0) {
                 fprintf(stderr, "Invalid or duplicate option(s) specified.\n");
                 exit(2);
           }

           if ( !aflag ) {
                 fprintf(stderr, "A service name is required for this option.\n");
                 exit(2);
           }

           check_rating(get_str);

           if (name_check(svcname, strlen(svcname))) {
                 fprintf(stderr, "Invalid service name %s.\n", svcname);
                 exit(2);
           }
	
           if (stat_rating < DYNAMIC_RATING || stat_rating > MAX_STAT_RATING) {  
                 fprintf(stderr,"Invalid rating value. Valid static ratings are 0-255, -1 starts dynamic ratings.\n");
                 exit(2);
           }

	   bzero((char *)&latioctl_service, sizeof(latioctl_service));

           strcpy((char *)latioctl_service.ls_name, svcname);
	   latioctl_service.ls_flags |= LATIOCTL_SERVICE_NAME;

           if ( stat_rating == DYNAMIC_RATING ) 
	   	latioctl_service.ls_flags |= LATIOCTL_SERVICE_RATING_DYANMIC;
           else {
	        latioctl_service.ls_rating = (char)stat_rating;
	   	latioctl_service.ls_flags |= LATIOCTL_SERVICE_RATING_STATIC;
	   }

           lioc_sservice(fd, latioctl_service);

           break;

     case CPSET_SVC:  		

           /* latcp -A -a serv1 [-i id]: add new service */
           /* latcp -i desc -a serv1: replace service description*/ 
           /* The -i option is not valid without the -a option with CPSET_SVC.*/
           /* The -a option is not valid without the -A option with CPSET_SVC.*/

           /* Check that no invalid options specified */
           optno = optno - Aflag - aflag - iflag - oflag;
           if (optno > 0) {
               fprintf(stderr, "Invalid or duplicate option(s) specified.\n");
               exit(2);
           }
           if (strlen(desc_str) > LAT_MAXDESC) {
               fprintf(stderr,"Argument too long. The maximum description is 64 characters\n");
	       exit(2);
           }
           if (name_check(svcname, strlen(svcname))) {
               fprintf(stderr, "Invalid service name %s.\n", svcname);
               exit(2);
           }

	   set_service(oflag, aflag, iflag, Aflag, descript, svcname);

           break;

     case CPDEL_SVC:		

           /* latcp -D -a serv1 : delete a service */
           /* Check that only valid option specified */
           optno = optno - Dflag - aflag;
           if (optno > 0) {
                 fprintf(stderr, "Invalid or duplicate option(s) specified.\n");
                 exit(2);
           }
           if (name_check(svcname, strlen(svcname))) {
               fprintf(stderr, "Invalid service name %s.\n", svcname);
               exit(2);
           }
	   bzero(&latioctl_service, sizeof(latioctl_service));
           strcpy((char *)latioctl_service.ls_name, svcname);
           makeupper(&latioctl_service.ls_name[0], strlen((char *)&latioctl_service.ls_name[0]));
	   latioctl_service.ls_flags = LATIOCTL_SERVICE_NAME;

           if (ioctl(fd, LIOCDSERVICE, &latioctl_service) < 0) {
                 if (errno == ENOENT)
                       fprintf (stderr, "Service name %s not found.\n", svcname);
                 else {
                	if (errno == EACCES)
                		fprintf (stderr, "No privileges for attempted operation.\n");
			else 
                       		fprintf(stderr, "LIOCDSERVICE ioctl failed.errno=%d.\n", errno);
		}
                exit(2);
           } 
           break;

     case CPDEL_PRT:  

           /* latcp -D -p tty38 : delete an application port */  
           /* Check that only valid option specified */
           optno = optno - Dflag - dflag - pflag;
           if (optno > 0) {
                 fprintf(stderr, "Invalid or duplicate option(s) specified.\n");
                 exit(2);
           }
           bzero(&latioctl_port, sizeof(latioctl_port));
           port_num = validate_port(port_name);

           strcpy(dev_tty,"/dev/");
           strcat(dev_tty,port_name);
           bzero(&statbuf, sizeof(statbuf));
           if (stat(dev_tty, &statbuf) < 0) {
		 if (errno==ENOENT) 
                       fprintf(stderr, "No such device.\n");
                 return;
           }
           latioctl_port.lp_devno = (dev_t)statbuf.st_rdev;
           latioctl_port.lp_flags |= LATIOCTL_PORT_RESET;

	   bzero(&get_port, sizeof(latioctl_port));
           get_port.lp_devno = latioctl_port.lp_devno;
           if (ioctl(fd, LIOCGPORT, &get_port) < 0) {
                 if (errno == ENODEV) {
                       if (pflag) {
                            fprintf(stderr,"No such device.\n");
                            exit(2);
                       }
                 }
                 else  if (errno == ENOTCONN) {
                            if (!pflag) {
                            return;
                       }
                 }
		 else if (errno == ERANGE) {
                            fprintf(stderr,"Minor number on port %s out of range.\n", port_name);
                            exit(2);
                 }
                 else {
                        fprintf(stderr,"LIOCGPORT ioctl failed. errno=%d.\n",errno);
                        exit(2);
                 }
           }

	   if (!(get_port.lp_flags & LATIOCTL_PORT_OUTBOUND)) {
                 fprintf(stderr,"Port %s is not an application port.\n", port_name);
		 exit(2);
	   }

           if (ioctl(fd, LIOCSPORT, &latioctl_port) < 0) {  
                 if (errno == EACCES) 
                       fprintf(stderr, "No privileges for attempted operation.\n");
		 else if (errno == ENODEV) 
                            fprintf(stderr,"No such device.\n");
		 else if (errno == EINVAL) 
                            fprintf(stderr,"Invalid argument.\n");
		 else if (errno == EROFS) 
                            fprintf(stderr,"Read only file system.\n");
		 else if (errno == ERANGE)
                            fprintf(stderr,"Minor number on port %s out of range.\n", port_name);
                 else  
                       fprintf(stderr,"LIOCSPORT ioctl failed. errno=%d.\n", errno);
                 exit(2);
           }
           break;

     case CPSET_PRT:	

           /* latcp -A -ptty32 -Hunxts10 -Rremotemodem -Vmodem -Q */
           /* map a service on terminal server into host's tty */
           /* Check that only valid option specified */
           optno=optno - Aflag - dflag - pflag - Hflag - Rflag - Vflag - Qflag;
           if (optno > 0) {
                 fprintf(stderr, "Invalid or duplicate option(s) specified.\n");
                 exit(2);
           }

           /* Check that at least one of the -R and -V options was specified */
           if ((Rflag + Vflag) == 0) {
                 fprintf(stderr, "Invalid options given. At least one option,");
                 fprintf(stderr, " -R or -V, must be specified.\n");
                 exit(2);
           }

           setup_port(&latioctl_port, pflag, port_name);
	   set_remote_node(&latioctl_port, Hflag, rem_node_nm);
	   set_remote_port(&latioctl_port, Rflag, rem_prt_nm);
	   set_remote_service(&latioctl_port, Vflag, rem_svc_nm);

           /* if the Q flag is specified, confusingly enough, it means */
           /* that queueing is not desired, or nonqueued access */
           if (Qflag)
                 latioctl_port.lp_flags |= LATIOCTL_PORT_NONQUEUED; 

	   latioctl_port.lp_flags |= LATIOCTL_PORT_OUTBOUND; 

           if (ioctl(fd, LIOCSPORT, &latioctl_port) < 0) {  
                 if (errno == EACCES) 
                       fprintf(stderr, "No privileges for attempted operation.\n");
		 else if (errno == ENODEV) 
                            fprintf(stderr,"No such device.\n");
		 else if (errno == EINVAL) 
                            fprintf(stderr,"Invalid argument.\n");
		 else if (errno == EROFS) 
                            fprintf(stderr,"Read only File System.\n");
		 else if (errno == ERANGE)
                            fprintf(stderr,"Minor number on port %s out of range.\n", port_name);
                 else  
                       fprintf(stderr,"LIOCSPORT ioctl failed. errno=%d.\n", errno);
                 exit(2);
           }
           break;

     case CPSHOW_PRT:	 

           /* latcp -d -P [-ptty32 | -L | -I] : display port characteristics */
           /* Check that only valid options specified */
           optno = optno - dflag - Pflag - pflag - Lflag - Iflag;
           if (optno > 0) {
                 fprintf(stderr, "Invalid or duplicate option(s) specified.\n");
                 exit(2);
           }

           /* Check that only one of the -p, -L, and -I options was specified */
           if ((pflag + Lflag + Iflag) > 1) {
                 fprintf(stderr, "Invalid options. Only one option, -p, -L or -I, may be specified.\n");
                 exit(2);
           }

           if (pflag) {			/* latcp -d -P -p tty03 */
                 port_num = validate_port(port_name);
           	 show_port(port_name, SHOW_BOTH_PRT,pflag,&noterm);
           	 if (noterm)
		       fprintf(stdout,"No terminal(s) found.\n");
	   }
           else if (Lflag) {		/* latcp -d -P -L */
		 for(i = 0; i < MAX_POSS_TTY; i++) {
                      integer_to_tty(i,ttys);
           	      show_port(ttys, SHOW_APPL_PRT,pflag,&noterm);
		 }
           	 if (noterm)
		      fprintf(stdout,"No terminal(s) found.\n");
	   }
           else if (Iflag) {		/* latcp -d -P -I */
		 for(i = 0; i < MAX_POSS_TTY; i++) {
                      integer_to_tty(i,ttys);
           	      show_port(ttys, SHOW_INTR_PRT,pflag,&noterm);
		 }
           	 if (noterm)
		      fprintf(stdout,"No terminal(s) found.\n");
	   }
           else { 			/* latcp -d -P */
		 for(i = 0; i < MAX_POSS_TTY; i++) {
                      integer_to_tty(i,ttys);
           	      show_port(ttys, SHOW_BOTH_PRT,pflag,&noterm);
                 }
           	 if (noterm)
		      fprintf(stdout,"No terminal(s) found.\n");
	   }
           break;

     default:
           if (Dflag) {
                 fprintf(stderr,"Invalid usage. -D requires a following -a or -p option.\n");
                 exit(2);
           }
           if (Aflag) {
                 fprintf(stderr,"Invalid usage. -A requires a following -a or -p option.\n");
                 exit(2);
           }
           break;
     }
     close(fd);
     exit(0);
}

/*******************************************************************/
/*
ROUTINE:	check_rating()
INPUTS:		rating
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine check rating
*/
/********************************************************************/
check_rating(rating)
char rating[];
{
     int i = 0;

     do {
           if (isdigit(rating[i]))
                  i++;
           else 
                  if (rating[i] == '-' && rating[i+1] == '1' && i == 0)
                        i++;
                  else {
                        fprintf(stderr,"Invalid rating value. Valid static ratings are 0-255, -1 starts dynamic ratings.\n");
                        exit(2);
                  }
     }
     while (rating[i] != '\0');
}
/*******************************************************************/
/*
ROUTINE:	check_number(charptr)
INPUTS:		charptr
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine check number.
*/
/********************************************************************/
check_number(charptr)
char *charptr;
{
     do {
           if (isdigit(*charptr))
                  charptr++;
           else 
                  return -1;
     } while (*charptr != '\0');
     return 0;
}


/*******************************************************************/
/*
ROUTINE:	zero_counter()
INPUTS:		none 
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine reset node counter, link counter, 
		session counter, and port counter.
*/
/********************************************************************/
zero_counter()
{
     char i, adapterinx[LAT_MAXNAME+1]; 
     if_names_blk if_names_blk;
     struct latioctl_nodectrs latioctl_nodectrs;
     struct latioctl_link latioctl_link;
     struct latioctl_linkctrs latioctl_linkctrs;
     struct latioctl_slotctrs latioctl_slotctrs;
     struct latioctl_portctrs latioctl_portctrs;

     bzero((char *)&latioctl_nodectrs, sizeof(latioctl_nodectrs));
     if (ioctl(fd, LIOCZNODECTRS, &latioctl_nodectrs) < 0){
          if (errno == EACCES)
		fprintf (stderr, "No privileges for attempted operation.\n");
          else
          	fprintf(stderr, "LIOCZNODECTRS ioctl failed. errno=%d.\n", errno);
	  exit(2);
     }

     bzero(if_names_blk, sizeof(if_names_blk));
     get_kinfo(&if_names_blk);
     for (i = 0; i < if_names_blk.curr_cnt; i++) {
          sprintf(adapterinx, "%d", if_names_blk.info[i].if_unit);
   	  strcat(if_names_blk.info[i].if_name, adapterinx);	
          bzero(&latioctl_link, sizeof(latioctl_link));
     	  strcpy(latioctl_link.ll_ifname, if_names_blk.info[i].if_name);

	  if ( ioctl(fd, LIOCGLINK, &latioctl_link) < 0) {
	        if (errno == EADDRNOTAVAIL) 
                     fprintf(stderr,"Link not defined for LAT.\n");
	        else {
                     if (errno == ENXIO) 
                     	fprintf (stderr, "No such adaptor received for LIOCGLINK ioctl.\n");
	             else
			fprintf(stderr, "LIOCGLINK ioctl failed. errno=%d.\n",errno);
	             exit(2);
	        }
	  }
	  else {
	        bzero(&latioctl_linkctrs, sizeof(latioctl_linkctrs));
     	        strcpy(latioctl_linkctrs.llc_ifname, if_names_blk.info[i].if_name);
                if (ioctl(fd, LIOCZLINKCTRS, &latioctl_linkctrs) < 0) {  
                     if (errno == ENXIO) 
                          fprintf (stderr, "Adaptor %s does not exist.\n",latioctl_linkctrs.llc_ifname);
               	     else if (errno == EACCES) 
                		fprintf (stderr, "No privileges for attempted operation.\n");
	             else if (errno == EADDRNOTAVAIL) 
                          fprintf (stderr, "Link not defined for LAT.\n");
                     exit(2);
		}
	  }
     }

     bzero((char *)&latioctl_portctrs, sizeof(latioctl_portctrs));
     if (ioctl(fd,  LIOCZPORTCTRS, &latioctl_portctrs) < 0){
                if (errno == EACCES) 
               	     fprintf (stderr, "No privileges for attempted operation.\n");
		else
          	     fprintf(stderr, " LIOCZPORTCTRS ioctl failed. errno=%d.\n", errno);
		exit(2);
    }
}

/*******************************************************************/
/*
ROUTINE:	set_groups()
INPUTS:		grpstr,svcname
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine add or delete group number.
*/
/********************************************************************/
set_groups(grpstr,svcname)
char *grpstr, svcname[];
{
     char *charptr;
     int ig, group, ind, bitind; 
     struct latioctl_service latioctl_service;

     /* get group list from driver */
     bzero((char *)&latioctl_service, sizeof(latioctl_service));
     latioctl_service.ls_flags = LATIOCTL_SERVICE_GROUPS;
     strcpy((char *)latioctl_service.ls_name, svcname);
     latioctl_service.ls_flags |= LATIOCTL_SERVICE_NAME;
     if (ioctl(fd, LIOCGSERVICE, &latioctl_service) < 0) 
           if (errno != ENOENT)
                 fprintf(stderr,"LIOCGSERVICE ioctl failed. errno=%x.\n", errno);
     /* Validate the group list specified. */
     grpstr = (strtok(grpstr, " ,"));

     for ( ; grpstr != NULL; grpstr=(strtok(0, " ,")))   {
           charptr = grpstr;
           if (check_number(charptr)) {
                 fprintf(stderr, "Invalid group value. Valid groups are (0 - 255).\n");
                 exit(2);
           }
           group = (unsigned)(atoi(grpstr));
           if (group < 0 || group > MAXGROUPSNUMB)  {
                fprintf(stderr, "Invalid group value. Valid groups are (0 - 255).\n");
                exit(2);
           }
           ind = (group >> 3);
           bitind = group & MASK;
           if (optflag == CPSET_GRPS)   { 
                latioctl_service.ls_groups[ind] |= grpmask[bitind];
	   }
	   else 
                latioctl_service.ls_groups[ind] &= ~grpmask[bitind];
     }

     /* set group to default 0 if the last group is deleted */
     ig = 0; 
     while (!latioctl_service.ls_groups[ig] && ig++ < LAT_MAXGROUPS)
           ;
     if (ig >= LAT_MAXGROUPS) 
           latioctl_service.ls_groups[0] |= LAT_DEFAULTGROUP;

     /* set group list to driver */
     latioctl_service.ls_flags = LATIOCTL_SERVICE_GROUPS;
     strcpy((char *)latioctl_service.ls_name, svcname);
     latioctl_service.ls_flags |= LATIOCTL_SERVICE_NAME;

     lioc_sservice(fd, latioctl_service);
}

/*******************************************************************/
/*
ROUTINE:	lioc_sservice()
INPUTS:		fd, structptr
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine tells the driver to set a service.
*/
/********************************************************************/
lioc_sservice(fd, latioctl_service)
int fd;
struct  latioctl_service latioctl_service;
{
     if (ioctl(fd, LIOCSSERVICE, &latioctl_service) < 0) {
           if (errno == ENOENT)
                 fprintf (stderr, "Service name %s not found.\n",
			 &latioctl_service.ls_name[0]);
           else if (errno == EINVAL)
                 fprintf(stderr, "Invalid rating or missing service name. errno=%d.\n", errno);
           else if (errno == EACCES) 
                 fprintf (stderr, "No privileges for attempted operation.\n");
           else 
                 fprintf(stderr, "LIOCSSERVICE ioctl failed. errno=%d.\n");
           exit(2);
     }
}

/*******************************************************************/
/*
ROUTINE:	lioc_aservice()
INPUTS:		fd,  structptr
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine tells the driver to add a service.
*/
/********************************************************************/
lioc_aservice(fd, latioctl_service)
int fd;
struct  latioctl_service latioctl_service;
{
  if (ioctl(fd, LIOCASERVICE, &latioctl_service) < 0) {
          fprintf (stderr, "Service not added - ");
          if (errno == EINVAL)
          	fprintf (stderr, "Invalid argument received for LIOCASERVICE ioctl.\n");
          else if (errno == EEXIST) 
               	     fprintf(stderr, "Service name %s already exists.\n", &latioctl_service.ls_name[0]);
          else if (errno == ENOBUFS)
                     fprintf(stderr, "No buffer available. errno=%d.\n",errno);
          else if (errno == ENOSPC)
                     fprintf(stderr, "Only %d services are supported.\n",MAX_SVCS);
          else if (errno == EACCES) 
                     fprintf (stderr, "No privileges for attempted operation.\n");
	  else
                     fprintf(stderr, "LIOCASERVICE ioctl failed. errno=%d.\n", errno);
          exit(2);
     }
}

/*******************************************************************/
/*
ROUTINE:	set_service()
INPUTS:		oflag, aflag, iflag, Aflag, descript, svcname
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine adds a new service or changes a service id
*/
/********************************************************************/
set_service(oflag, aflag, iflag, Aflag, descript, svcname)
int oflag, aflag, iflag, Aflag;
char *descript, svcname[];
{
     int i;
     struct latioctl_service latioctl_service;

     bzero((char *)&latioctl_service, sizeof(latioctl_service));

     if (iflag && (!aflag)) {
           fprintf(stderr,"Invalid usage.  -i option requires a following -a option\n");
           exit(2);
     }

     strcpy((char *)latioctl_service.ls_name, svcname);
     makeupper(&latioctl_service.ls_name[0], strlen((char *)&latioctl_service.ls_name[0]));
     latioctl_service.ls_flags = LATIOCTL_SERVICE_NAME | LATIOCTL_SERVICE_DESC;

     if (!oflag)
           latioctl_service.ls_flags |= LATIOCTL_SERVICE_DEFAULT;

     if (descript) { 
	   /* Use description supplied by user */
           strcpy((char *)latioctl_service.ls_desc, descript);
           if ( (latioctl_service.ls_desc[0] == '-') || (latioctl_service.ls_desc[0] == '/') ) {
                 fprintf(stderr, "Invalid service descriptor %s.\n", descript);
                 exit(2);
           }
     } 
     else {
	   /* use default service description */
	   strcpy((char *)latioctl_service.ls_desc,OSNAME);
	   strcat((char *)latioctl_service.ls_desc,utsname.release);
	   strcat((char *)latioctl_service.ls_desc,SVCID);
     }
     if (Aflag)  { 
           /* add new service # latcp -A -a new_service */
           lioc_aservice(fd, latioctl_service);
     }
     else {     
           /* Replace description: latcp -i id -a svc */
           latioctl_service.ls_flags &= ~LATIOCTL_SERVICE_DEFAULT;
           lioc_sservice(fd, latioctl_service);
     }
}


/*******************************************************************/
/*
ROUTINE:	dmp_nodechar()
INPUTS:		none 
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine displays node characteristic information. 
*/
/********************************************************************/
dmp_nodechar()
{
     int bitind, group, i, j, service_index;
     char str2[80];
     struct latioctl_service latioctl_service;
     struct latioctl_node latioctl_node;
     struct latioctl_link latioctl_link;
     if_names_blk if_names_blk;

     bzero(&latioctl_node, sizeof(latioctl_node));
     latioctl_node.ln_flags = LATIOCTL_NODE_STATE;
     if (ioctl(fd, LIOCGNODE, &latioctl_node) < 0) {
          fprintf(stderr, "LIOCGNODE ioctl failed. errno=%d.\n", errno);
	  exit(2);
     }

     makeupper(latioctl_node.ln_name, strlen((char *)latioctl_node.ln_name));
     fprintf(stdout,"\nNode name:  \%s\n", latioctl_node.ln_name);
     fprintf(stdout, "Multicast timer:  \t%d seconds\n", latioctl_node.ln_satimer);
     fprintf(stdout, "LAT version:  %d\t\tECO:\t%d\n", LAT_SW_VERSION, LAT_ECO);

     /* get interface information from kernel to check if adapter defined */
     bzero(if_names_blk, sizeof(if_names_blk));
     get_kinfo(&if_names_blk);
     fprintf(stdout,"Selected Interface Name(s): ");
     for (i = 0; i < if_names_blk.curr_cnt; i++) {
	  sprintf(str2, "%d", if_names_blk.info[i].if_unit);
   	  strcat(if_names_blk.info[i].if_name, str2);	
     	  bzero(&latioctl_link, sizeof(latioctl_link));
     	  strcpy(latioctl_link.ll_ifname, if_names_blk.info[i].if_name);

	  if ( ioctl(fd, LIOCGLINK, &latioctl_link) < 0) {
		if (errno != EADDRNOTAVAIL) {
                      if (errno == ENXIO) 
                       		fprintf (stderr, "No such adaptor received for LIOCGLINK ioctl.\n");
		      else
				fprintf(stderr, "LIOCGLINK ioctl failed. errno=%d.\n",errno);
		      exit(2);
		}
          }
	  else {
                fprintf(stdout,"  %s", latioctl_link.ll_ifname);
	  }

     }
     fprintf(stdout, "\n"); 
     if (!latioctl_node.ln_state)
	  fprintf(stdout,"LAT Protocol is not active\n");
     else 
	  fprintf(stdout,"LAT Protocol is active\n");
     fprintf(stdout, "\n"); 

     /* get service name from driver and print out service name */
     for(service_index = 0; service_index < MAX_SERVICES; service_index++) { 
	  bzero(&latioctl_service, sizeof(latioctl_service));
	  latioctl_service.ls_index = (char)service_index;
	  if (ioctl(fd, LIOCGSERVICE, &latioctl_service) < 0) {
	  	if (errno != ENOENT) {
                       fprintf(stderr,"LIOCSGSERVICE ioctl failed. errno=%d.\n", errno);
                       return;
		}
	  }
	  else { 
          	fprintf(stdout, "\nService information\n");
	        fprintf(stdout, "\tService name: \t%s", latioctl_service.ls_name);
	        if (latioctl_service.ls_flags & LATIOCTL_SERVICE_DEFAULT) 
		      fprintf(stdout, "\n");
		else 
		      fprintf(stdout, "\t(Optional)\n");

          	fprintf(stdout, "\tService ID: \t%s\n", latioctl_service.ls_desc);
		if (latioctl_service.ls_flags &  LATIOCTL_SERVICE_RATING_DYANMIC) 
          	      fprintf(stdout, "\tRating: \tDynamic \t%d\n", latioctl_service.ls_rating);
	 	else 
          	      fprintf(stdout, "\tRating: \tStatic \t\t%d\n", latioctl_service.ls_rating);

                fprintf(stdout, "\tGroups:\t\t");
                for (i = 0; i < LAT_MAXGROUPS; i++) 
                   if (latioctl_service.ls_groups[i]) { 
                      for (bitind = 0; bitind < 8; bitind++) 
                         if (latioctl_service.ls_groups[i] & grpmask[bitind])  {
                             group = ((i << 3) + bitind);
                             fprintf(stdout, "%d", group);
                             if (latioctl_service.ls_groups[i] >> bitind+1) {
                                 fprintf(stdout, ",");
			     }
                             else for (j = i+1; j < LAT_MAXGROUPS; j++) 
                                     if (latioctl_service.ls_groups[j]) {
                                         fprintf(stdout, ",", group);
                                         break;
                                     }
                         }
                     }
                fprintf (stdout, "\n");
          }
     }
}

/*******************************************************************/
/*
ROUTINE:	dmp_node_counters()
INPUTS:		none 
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:       This routine gets the node counter information for display. 
*/
/********************************************************************/
dmp_node_counters()
{
     struct latioctl_nodectrs latioctl_nodectrs;

     bzero(&latioctl_nodectrs, sizeof(latioctl_nodectrs));
     if (ioctl(fd, LIOCGNODECTRS, &latioctl_nodectrs) < 0) { 
            fprintf(stderr, "LIOCGNODECTRS ioctl failed, errno=%d\n", errno);
	    exit(2);
     }
     /* display counter information */
     fprintf(stdout, "LATCP Node Counters \n");
     fprintf(stdout, "%12d  Receive frames\n", latioctl_nodectrs.lnc_pdusrcvd);
     fprintf(stdout, "%12d  Receive errors\n", latioctl_nodectrs.lnc_badpdus_rcvd);
     fprintf(stdout, "%12d  Receive duplicates\n", latioctl_nodectrs.lnc_duppdusrcvd);
     fprintf(stdout, "%12d  Transmit frames\n", latioctl_nodectrs.lnc_pdussent);
     fprintf(stdout, "%12d  Retransmissions\n", latioctl_nodectrs.lnc_duppdussent); 
}

/*******************************************************************/
/*
ROUTINE:	setup_port()
INPUTS:		latioctl_port, pflag, port_name
OUTPUTS:	None
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine sets up local virtual terminal tty.
*/
/*******************************************************************/
setup_port(latioctl_port, pflag, port_name)
struct latioctl_port *latioctl_port;
int pflag;
char *port_name;
{
      int port_num;
      char dev_tty[80];
      struct latioctl_port get_port;
      struct stat statbuf;

      bzero(&latioctl_port->lp_flags, sizeof(get_port));

      latioctl_port->lp_flags &= ~LATIOCTL_PORT_NONQUEUED; 

      if (!pflag) {
            fprintf(stderr,"Invalid options specified. The -p option is required.\n");
            exit(2);
      } 
      else {
            validate_port(port_name);
            strcpy(dev_tty,"/dev/");
            strcat(dev_tty,port_name);
            bzero(&statbuf, sizeof(statbuf));
            if (stat(dev_tty, &statbuf) < 0)
                 return;
            latioctl_port->lp_devno = (dev_t)statbuf.st_rdev;
      }

      bzero(&get_port, sizeof(get_port));
      get_port.lp_devno = (dev_t)latioctl_port->lp_devno;
      if (ioctl(fd, LIOCGPORT, &get_port) < 0) {
           if (errno == ENODEV) {
                 if (pflag) {
                      fprintf(stderr,"No such device.\n");
                      exit(2);
                 }
           }
           else  if (errno == ENOTCONN) {
                      if (!pflag) {
                            return;
                      }
                 }
	   else if (errno == ERANGE) {
                   fprintf(stderr,"Minor number on port %s out of range.\n", port_name);
                   exit(2);
                }
           else {
                 fprintf(stderr,"LIOCGPORT ioctl failed. errno=%d.\n",errno);
                 exit(2);
           }
      }

      if ((get_port.lp_flags & LATIOCTL_PORT_CONNECTED) || (get_port.lp_flags & LATIOCTL_PORT_OUTBOUND) || (get_port.lp_flags & LATIOCTL_PORT_NONQUEUED)) {
            if (get_port.lp_flags & LATIOCTL_PORT_OUTBOUND) 
                  fprintf(stderr,"Port %s has already been defined.\n", port_name);
            else 
                  fprintf(stderr,"Port %s is in use.  Cannot use as an application port.\n", port_name);
            exit(2);
      }
}

/*******************************************************************/
/*
ROUTINE:	set_remote_node()
INPUTS:		latioctl_port, Hflag, rem_node_nm
OUTPUTS:	None
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine set remote node on terminal server.
*/
/*******************************************************************/
set_remote_node(latioctl_port, Hflag, rem_node_nm)
struct latioctl_port *latioctl_port;
int Hflag;
char *rem_node_nm;
{
     if (!Hflag) {
           fprintf(stderr,"Invalid options specified. The -H option is required.\n");
           exit(2);
     } 
     if (name_check(rem_node_nm, strlen(rem_node_nm))) {
           fprintf(stderr,"Invalid remote node name %s.\n", rem_node_nm);
           exit(2);
     }
     if (strlen(rem_node_nm) > LAT_MAXNAME) {
           fprintf(stderr, "Remote node name %s is too long.\nMaximum length is %d.\n", rem_node_nm, LAT_MAXNAME);
           exit(2);
     }
     strcpy((char *)latioctl_port->lp_node,rem_node_nm);
     latioctl_port->lp_flags |= LATIOCTL_PORT_NODE;
}

/*******************************************************************/
/*
ROUTINE:	set_remote_port()
INPUTS:	        latioctl_port, Rflag, rem_prt_nm
OUTPUTS:	None
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine set remote port on terminal server.
*/
/*******************************************************************/
set_remote_port(latioctl_port, Rflag, rem_prt_nm)
struct latioctl_port *latioctl_port;
int Rflag;
char *rem_prt_nm;
{
     if (!Rflag)
          return;
     if (name_check(rem_prt_nm,strlen(rem_prt_nm))) {
          fprintf(stderr,"Invalid remote port name-%s\n", rem_prt_nm);
          exit(2);
     }

     if (strlen(rem_prt_nm) > LAT_MAXNAME)  {
          fprintf(stderr, "Remote port name %s is too long.\nMaximum length is %d.\n", rem_prt_nm, LAT_MAXNAME);
          exit(2);
     }
     strcpy((char *)latioctl_port->lp_remoteport,rem_prt_nm);
     latioctl_port->lp_flags |= LATIOCTL_PORT_REMOTEPORT;
}

/*******************************************************************/
/*
ROUTINE:	set_remote_service()
INPUTS:	        latioctl_port, Vflag, rem_svc_nm
OUTPUTS:	None
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine set remote node on terminal server.
*/
/*******************************************************************/
set_remote_service(latioctl_port, Vflag, rem_svc_nm)
struct latioctl_port *latioctl_port;
int Vflag;
char *rem_svc_nm;
{
     if (!Vflag)
           return;

     if(name_check(rem_svc_nm, strlen(rem_svc_nm))){
           fprintf(stderr, "Invalid remote service name- %s\n", rem_svc_nm);
           exit(2);
     }

     if (strlen(rem_svc_nm) > LAT_MAXNAME)  {
     	  fprintf(stderr, "Remote service name %s is too long.\n",rem_svc_nm);
      	  fprintf(stderr, "Maximum length is %d.\n",  LAT_MAXNAME);
          exit(2);
     }
     strcpy((char *)latioctl_port->lp_service,rem_svc_nm);
     latioctl_port->lp_flags |= LATIOCTL_PORT_SERVICE;
}


/*******************************************************************/
/*
ROUTINE:	show_port()
INPUTS:		port_name, showflag,pflag,noterm 
OUTPUTS:	None
CALLED BY:	main
CALLS:	 	None	

FUNCTION:       This routine display port characteristics.	
*/
/********************************************************************/

show_port(port_name, showflag,pflag,noterm)
int showflag,pflag;
int *noterm;
char *port_name;
{

     char dev_tty[80],tty[80];
     struct latioctl_port latioctl_port;
     struct stat statbuf;

     bzero(&latioctl_port, sizeof(latioctl_port));
     bzero(&statbuf, sizeof(statbuf));
     bzero(dev_tty, 80);

     strcpy(dev_tty,"/dev/");
     strcat(dev_tty,port_name);
     if (stat(dev_tty, &statbuf) < 0)
	   return;
     latioctl_port.lp_devno = (dev_t)statbuf.st_rdev;
     latioctl_port.lp_flags = 0;

     if (ioctl(fd, LIOCGPORT, &latioctl_port) < 0)  {
           if (errno == ENODEV) {
                 if (pflag) { 
                      fprintf(stderr,"No such device.\n");
		      exit(2);
                 }  
           }
	   else  if (errno == ENOTCONN) { 
                      if (pflag) {
                            return;
                      }  
                 }
	   else if (errno == ERANGE) {
                   fprintf(stderr,"Minor number on port %s out of range.\n", port_name);
                   return;
                }
	   else { 
                 fprintf(stderr,"LIOCGPORT ioctl failed. errno=%d.\n",errno);
                 exit(2);
           }
	   return;
     }

     /* display one port chararcteristics */
     if (!latioctl_port.lp_node[0])
            return;
     switch(showflag) {
	case SHOW_BOTH_PRT: /* display either application or interactive port */ 
           if (latioctl_port.lp_flags & LATIOCTL_PORT_OUTBOUND)    
		show_appl_port(&latioctl_port, port_name,&noterm);
           else 
		show_intr_port(&latioctl_port, port_name,pflag,&noterm); 
	   break;

	case SHOW_APPL_PRT: /* display application port  */ 
           if (latioctl_port.lp_flags & LATIOCTL_PORT_OUTBOUND)    
		show_appl_port(&latioctl_port, port_name,&noterm);
	   break;

	case SHOW_INTR_PRT: /* display interactive port */ 
           if (latioctl_port.lp_flags & ~LATIOCTL_PORT_REMOTEPORT)    
		show_intr_port(&latioctl_port, port_name,pflag,&noterm);
           break;

	default:
                 fprintf(stderr,"No terminal(s) found.\n");
     }
}

/*******************************************************************/
/*
ROUTINE:        integer_to_tty()
INPUTS:         integer, tty
OUTPUTS:        none
CALLED BY:      show_port
CALLS:          None

FUNCTION:       This routine convert an integer to ttyxx.
*/
/********************************************************************/
integer_to_tty(integer, tty)
int integer;
char tty[];
{
    int i, tens, ones;

    tens = integer/62;
    ones = integer % 62;


    if ((ones >= 0) && (ones <= 9))		/* ttyAB where B is '0'-'9' */
          ones += '0';
    else if ((ones >= 10) && (ones <= 35))	/* ttyAB where B is 'a'-'z' */
          ones += 'W';
    else if ((ones >= 36) && (ones <= 61))	/* ttyAB where B is 'A'-'Z' */
          ones += 29;
    tens += '0';

    tty[0] = 't';
    tty[1] = 't';
    tty[2] = 'y';
    tty[3] = tens;
    tty[4] = ones;
    tty[5] = '\0';
}

/*******************************************************************/
/*
ROUTINE:	show_appl_port()
INPUTS:		port_name, latioctl_port,noterm
OUTPUTS:	none 	
CALLED BY:	show_port
CALLS:	 	None	

FUNCTION:	This routine display the characteristics of an application
		port.
*/
/********************************************************************/
show_appl_port(latioctl_port, port_name,noterm)
struct latioctl_port *latioctl_port;
char *port_name;
int **noterm;
{
     char tty[80];

     **noterm = TERMINAL_FOUND;
     fprintf(stdout,"\nLocal Port Name = ");
     fprintf(stdout,"%s     ", port_name);
     fprintf(stdout,"<application>\n");
		
     if (latioctl_port->lp_node[0] != NULL)
           fprintf(stdout, "\n\tSpecified Remote Node Name = %s\n", latioctl_port->lp_node);
     if (latioctl_port->lp_remoteport[0] != NULL)
           fprintf(stdout, "\tSpecified Remote Port Name = %s\n", latioctl_port->lp_remoteport);
     if (latioctl_port->lp_service[0] != NULL)
           fprintf(stdout, "\tSpecified Remote Service Name = %s\n", latioctl_port->lp_service);
     /* display actual port info when the port is active */

     if ((latioctl_port->lp_node[0] != NULL) && (latioctl_port->lp_flags & LATIOCTL_PORT_CONNECTED)) {
           fprintf(stdout, "\tActual Remote Node Name = %s\n", latioctl_port->lp_node);
     }
     if ((latioctl_port->lp_remoteport[0] != NULL) && (latioctl_port->lp_flags & LATIOCTL_PORT_CONNECTED)) {
           fprintf(stdout, "\tActual Remote Port Name = %s\n", latioctl_port->lp_remoteport);
     }
}

/*******************************************************************/
/*
ROUTINE:	show_intr_port()
INPUTS:		latioctl_port, port_name, pflag, noterm
OUTPUTS:	none 	
CALLED BY:	show_port
CALLS:	 	None	

FUNCTION:	This routine display the characteristics of an interactive
		port.
*/
/********************************************************************/
show_intr_port(latioctl_port, port_name, pflag,noterm)
struct latioctl_port *latioctl_port;
int pflag;
int **noterm;
char *port_name;
{
     char tty[80];

     if (!(latioctl_port->lp_flags & LATIOCTL_PORT_CONNECTED)) {
	   return;   /* no terminal found */
     }
 
     **noterm = TERMINAL_FOUND;
     fprintf(stdout,"\nLocal Port Name = ");
     fprintf(stdout,"%s     ", port_name);
     fprintf(stdout,"<interactive>\n");

     if (latioctl_port->lp_node[0] != NULL)
          fprintf(stdout,"\n\tActual Remote Node Name = %s\n", latioctl_port->lp_node);
     if (latioctl_port->lp_remoteport[0] != NULL)
          fprintf(stdout,"\tActual Remote Port Name = %s\n", latioctl_port->lp_remoteport);
}

/*******************************************************************/
/*
ROUTINE:	validate_port()
INPUTS:		ptr 
OUTPUTS:	port_num
CALLED BY:	latcp 
CALLS:	 	None	

FUNCTION:	This routine validates the port name specified by the user. 
*/
/********************************************************************/
validate_port(ptr)
char *ptr;
{

#define INV_PORT_MSG "Invalid port name.  One port may be specified in the form ttywx, where w is \na number from 0-9 and x is an alphanumeric from 0-9, a-z or A-Z.\n"

     register i;
     int str_len, port_num;
     char *number_ptr;

     if ((strncmp(ptr,"tty",3)) != 0) {
           fprintf(stderr, "%s", INV_PORT_MSG);
           exit(2);
     }
     str_len = strlen(ptr);
     if (str_len != 5) {
           fprintf(stderr, "%s", INV_PORT_MSG);
           exit(2);
     }

     if (*(ptr + 3) < '0' || *(ptr + 3) > '9') {
           fprintf(stderr, "%s", INV_PORT_MSG);
           exit(2);
     }

     if (*(ptr + 4) < '0' || *(ptr + 4) > '9') {
           if (*(ptr + 4) < 'a' || *(ptr + 4) > 'z')
           	if (*(ptr + 4) < 'A' || *(ptr + 4) > 'Z') { 
           		fprintf(stderr, "%s", INV_PORT_MSG);
                 	exit(2);
           	}
     }
}

/********************************************************************/
/* 
ROUTINE:	chk_cmd_line()
INPUTS:		num_opt
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine checks that multiple options are not specified on
		the command line where not permitted.*/
/********************************************************************/

chk_cmd_line(num_opt)
int        num_opt;
{
     if ( num_opt > 1 ) {
           fprintf(stderr, "Multiple options not supported.\n");
           return(-1);
     }
     else
           return(1);
}


/********************************************************************/
/* 
ROUTINE:	dmp_cbs ()
INPUTS:		none 
OUTPUTS:	none 	
CALLED BY:	main - CPSHOW_SRVR 
CALLS:	 	None	


FUNCTION:	This routine does an ioctl to get information for all virtual
		circuit. Information is displayed for the server characteristics
		option: latcp -d -S.
		Display for server characterics: 
		LATCP Server Characteristics for LAT_08002B08C74D 
		Ethernet address:  08-00-2B-08-C7-4D */
/********************************************************************/

dmp_cbs()
{

     struct latioctl_node latioctl_node;
     struct  latioctl_host latioctl_host;
     int	 vc_found;
     int         i,j;

     vc_found = 0;
     /* get maxhosts from node */
     latioctl_node.ln_flags |= LATIOCTL_NODE_MAXHOSTS;
     if (ioctl(fd, LIOCGNODE, &latioctl_node) < 0){
           fprintf(stderr, "LIOCGNODE ioctl failed. errno=%d.\n", errno);
           exit(2);
     }
     /* index thru latioctl_hosts to get servers info */
     for (i=0; i<(latioctl_node.ln_maxhosts );i++){
	   latioctl_host.lh_index=i;
	   if (ioctl(fd, LIOCGHOST, &latioctl_host) < 0){
                if (errno == EINVAL) {
                        fprintf(stderr, "Invalid argument received for LIOCGHOST ioctl.\n");
                        exit(2);
                }
		else if (errno != ENOENT) {
			fprintf(stderr, "LIOCGHOST ioctl failed. errno=%d.\n", errno);
                        exit(2);
                }
	   }
     	   else{
		vc_found++;
	   	/* Display server characteristics */
	   	fprintf(stdout,"\nLATCP Server Characteristics for %s\n",
			latioctl_host.lh_name);
           	fprintf(stdout,"Ethernet address:  ");
     	   	for (j=0; j<5;j++){
			fprintf(stdout,"%02x-", latioctl_host.lh_hwaddr[j]);
	   	}
           	fprintf(stdout,"%02x\n",latioctl_host.lh_hwaddr[j]);
	   }
     }
     if (!vc_found){
           fprintf(stdout, "No virtual circuits established.\n");
           exit(2);
     }
}


/********************************************************************/
/* 
ROUTINE:	dmp_counters()
INPUTS:		none 
OUTPUTS:	none 	
CALLED BY:	main - CPSHOW_CB_CTR 
CALLS:	 	None	

/* 
FUNCTION:	This routine does an ioctl to get the counter information from 
		all circuit blocks. This information is used in the server 
		counter displays: latcp -d -C 
		Display for server counters: 
		LATCP Server Counters for LAT_08002B08C74D 
		2385  Frames received 
		2385  Frames transmitted 
		0  Retransmissions 
		0  Frames received out of sequence   not in silver */
/********************************************************************/
	
dmp_counters()
{


     struct latioctl_node latioctl_node;
     struct  latioctl_host latioctl_host;
     int	 vc_found;
     int         i;

     vc_found = 0;
     /* get maxhosts from node */
     latioctl_node.ln_flags |= LATIOCTL_NODE_MAXHOSTS;
     if (ioctl(fd, LIOCGNODE, &latioctl_node) < 0){
           fprintf(stderr, "LIOCGNODE ioctl failed. errno=%d.\n", errno);
           exit(2);
     }
     /* index thru latioctl_hosts to get servers info */
     for (i=0; i<latioctl_node.ln_maxhosts;i++){
	   latioctl_host.lh_index=i;
	   if (ioctl(fd, LIOCGHOST, &latioctl_host) < 0){
                if (errno == EINVAL) {
                        fprintf(stderr, "Invalid argument received for LIOCGHOST ioctl.\n");
			exit(2);
                }
		else if (errno != ENOENT)  {
			fprintf(stderr, "LIOCGHOST ioctl failed. errno=%d.\n", errno);
			exit(2);
                }
	   }
     	   else{
		vc_found++;
           	/* Display counter information */
           	fprintf(stdout, "\nLATCP Server Counters for %s\n",
			 latioctl_host.lh_name);
           	fprintf(stdout, "%10d  Frames received\n",
			 latioctl_host.lh_pdusrcvd);
           	fprintf(stdout, "%10d  Frames transmitted\n",
			 latioctl_host.lh_pdussent);
           	fprintf(stdout, "%10d  Retransmissions\n",
			 latioctl_host.lh_duppdussent);
	   	/* duppdusrcvd used in silver lat_vc.c for out of seq & dup. start msg */
           	/* fprintf(stdout, "\n%10d  Frames received out of sequence\n", latioctl_host.?); */
	   }
     }
     if (!vc_found){
           fprintf(stdout, "No virtual circuits established.\n");
           exit(2);
     }
}


/********************************************************************/
/* 
ROUTINE:	name_check ()
INPUTS:		ptr, len
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine validates user supplied node and service names. If
		the name is invalid, -1 is returned. Otherwise, zero (0) is
		returned. */
/********************************************************************/

name_check(ptr, len) 
register char    *ptr;
int        len;
{
     register int i;

     /* Check for valid length.  Name may not begin with a "-". */
     if ((len > LAT_MAXNAME) || !(strncmp("-", ptr, 1))){
           return(-1);
     }
     for(i=0; i<len; i++, ptr++) {
           if (!(isalnum(*ptr)))           /* if not alphanumeric */
                 /* Check for only other valid characters. */
                 if (*ptr != '.' && *ptr != '_' && 
		     *ptr != '-' && *ptr != '$'){
                       return(-1);
		 }
     }
     return(0);
}


/********************************************************************/
/* 
ROUTINE:	makeupper ()
INPUTS:		ptr, len
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine converts any lower case letters in the input string		to upper case. */
/********************************************************************/

makeupper(ptr, len)
register char    *ptr;
int        len;
{
     register int i;

     for (i = 0; i < len; i++, ptr++) 
           if (islower(*ptr))
                 *ptr = toupper(*ptr);
}


/********************************************************************/
/* 
ROUTINE:	get_kinfo 
INPUTS:		ptr 
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine gets interface information from the kernel. */
/********************************************************************/

get_kinfo(kptr)
if_names_blk *kptr;
{
	int kinfo_fd;
	struct strioctl strioc; 


	kinfo_fd=open(KINFODEV,O_RDONLY);
	if (kinfo_fd<0)
	{
		fprintf(stderr, "\nKinfo open failed. errno=%d.\n",errno); 
		exit(2);
	}
	strioc.ic_cmd = KINFO_GET_IF_NAMES;
	strioc.ic_timout = 0;
	strioc.ic_len = sizeof (if_names_blk);
	strioc.ic_dp =  (char *) kptr;
	if ((ioctl(kinfo_fd,I_STR,&strioc))<0) {
		fprintf(stderr, "\nKinfo I_STR failed. errno=%d.\n",errno); 
		close(kinfo_fd);
		exit(2);
	}
	if (kptr->more){
		fprintf(stdout,"The kinfo ""more"" field indicates that a second read is needed.");
		fprintf(stdout,"More than 145 adaptors were found. This should not happen.\n");
	}
	close(kinfo_fd);
}

/********************************************************************/
/* 
ROUTINE:	adaptor_name_check ()
INPUTS:		ptr 
OUTPUTS:	none 	
CALLED BY:	main
CALLS:	 	None	

FUNCTION:	This routine validates user supplied adaptor names. If the name 		is invalid, -1 is returned. Otherwise, zero (0) is returned. */
/********************************************************************/

adaptor_name_check(ptr)
char    *ptr;
{
        register int i;
	if_names_blk	ifblk;
	char	kadapt[LAT_MAXNAME+1];
	int	adapt_found;
	int	tmp;
	get_kinfo(&ifblk);
	for (i=0;i<(ifblk.curr_cnt); i++){
		sprintf(&kadapt[0],"%s%d",ifblk.info[i].if_name,
				ifblk.info[i].if_unit);
		if((strncmp(kadapt,ptr,strlen(ptr)))==0){
			adapt_found++;
			break;
		}
	}
	if (!adapt_found){
               	fprintf(stderr, "Invalid adaptor name %s.\n", ptr);
		exit(2);
	}

        return(0);
}
