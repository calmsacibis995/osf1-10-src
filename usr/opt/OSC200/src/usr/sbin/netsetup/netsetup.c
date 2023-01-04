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
static char *sccsid = "@(#)$RCSfile: netsetup.c,v $ $Revision: 4.2.18.10 $ (DEC) $Date: 1993/12/15 19:48:32 $";
#endif
/*
 *  Sets up:
 *      /etc/rc.config
 *	/etc/hosts
 *	/etc/hosts.equiv
 *	/etc/networks
 *
 *  Modifies:
 *	/etc/rc.config
 *	/etc/hosts
 *	/etc/hosts.equiv
 *	/etc/networks
 *
 *  Does NOT handle:
 *      configuring gated.conf
 *
 * Modification History:
 *      18 Jun 1991    walker    
 *      initial version
 *
 *      19 Aug 1991    walker
 *      only do fgrep on /etc/hosts if /etc/hosts exists.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/stream.h>
#include <stropts.h>
#include <sys/strkinfo.h>
#include <sys/wait.h>
#include <netdb.h>
#include <ctype.h>

#define	HOSTS		"/etc/hosts"
#define	HOSTS_EQUIV	"/etc/hosts.equiv"
#define	NETWORKS	"/etc/networks"
#define	GATEDCONF	"/etc/gated.conf"
#define	ROUTES		"/etc/routes"
#define	RCCONFIG	"/etc/rc.config"
#define	LOCALHOST	"localhost"

#define	STR_GATEWAY	"gateway"
#define	STR_INTERFACE	"interface"

#define	NO	0
#define	YES	1

#define	TRUE	1
#define	FALSE	0

#define CLASSA	1
#define CLASSB	2 
#define CLASSC	3

/* main menu choices */
#define M_CONFIG	1
#define M_DAEMONS	2
#define M_HOST_INFO	3
#define M_DISPLAY	4
#define M_EXIT		5

/* net_host_info menu choices */
#define H_HOSTS		1
#define H_HOSTSEQUIV	2
#define H_NETWORKS	3
#define H_EXIT		4

/* static route destinations */
#define	R_NETWORK	0x1	/* /etc/routes network destination */
#define	R_HOST		0x2	/* /etc/routes host destination */
#define	R_DEFAULT	0x4	/* /etc/routes default destination */

/* maximum number of adapters */
#define	MAX_NETDEVS	24

/* modes for add/delete host information */
#define	ADD	1
#define	DELETE	2

#define	ADD_STR		"add"
#define	DELETE_STR	"delete"

#define	CLEAR		"clear"

#define plural(s) (index(s,' ')||index(s,'\t'))
#define iswhite(s) isspace(s)

#define GET_VALUE(command, buf)			\
{						\
	buf[0] = '\0';				\
	fp = popen(command, "r");		\
	if (fp) {				\
		int len;			\
		fgets(buf, 256, fp);		\
		pclose(fp);			\
		len = strlen(buf);		\
		if (buf[len-1] == '\n')		\
			buf[len-1] = '\0';	\
	}					\
}

int ClearScreen;	/* if 0, don't clear screen; if 1, clear screen */

/* Non-integer function declarations */
FILE *edit();
char *prompt();
char *quote();


main(argc, argv)
int argc;
char **argv;
{
	int i, done;
	int status;
	pid_t child;
	char value[80];

	for (i = 3; i < 20; i++)
		close(i);
	dup2(1, 3);

	if (getuid()) {
		fprintf(stderr, 
	"\nYou must be root to set up the system on a local area network.\n");
		leave(1);
	} 

	/*
	 * configure streams modules
	 */
	if ((child = fork()) == 0) {
		execl("/usr/sbin/strsetup", "strsetup", "-i", (char *)0);    
		exit(1);
	}
	waitpid(child, &status, 0);
	if (status) {
		printf("UNABLE to configure STREAMS devices\n");
		prompt(value,10,"Press RETURN to continue ");
	}

	done = 0;
	/* don't clear screen first time menu is displayed */
	ClearScreen = 0;
	do {
		switch(menu()) {
		case M_CONFIG:
			system(CLEAR);
			net_config();
			break;
		case M_DAEMONS:
			system(CLEAR);
			net_daemons();
			break;
		case M_HOST_INFO:
			system(CLEAR);
			net_host_info();
			break;
		case M_DISPLAY:
			system(CLEAR);
			net_display();
			break;
		case M_EXIT:
			done = 1;
			break;
		default:
			printf("\nYour choice is invalid\n");
			prompt(value,10,"Press RETURN to continue ");
			break;
		}
	} while (!done);

	printf("\n\
To perform additional tasks in setting up the network, see the Network\n\
Configuration reference.  For the netsetup modifications to take\n\
effect, either restart the network services on this system with the following\n\
command:\n\n\
\t/usr/sbin/rcinet restart\n\n\
or reboot this system with the following command:\n\n\
\t/usr/sbin/shutdown -r now\n\n\
NOTE: If you are going to use the \'/usr/sbin/rcinet restart\' command,\n\
      warn the users that the network services on this system are going\n\
      to be restarted.  Also, any NFS filesystems not mounted via fstab or\n\
      automount will not be remounted.\n\n");

	for (;;) {
		if (yesno(NO,"Do you want netsetup to automatically restart the network\n\services on this system")) {
			if (system("/usr/sbin/rcinet restart") != 0)
				leave(1);
			break;
		} else {
			printf("\nRestart the network services on this system using\n\
one of the commands listed above when you are ready.");
			break;
		}
	}

	if (status)
		printf("\n\nnetsetup was UNABLE to configure STREAMS devices\n");
	leave(0);
}


menu()
{
	char choice[10];

	/* don't clear the screen the first time this menu is displayed */
	if (ClearScreen == 0)
		ClearScreen = 1;	/* clear screen after first time */
	else
		system(CLEAR);

	printf("\n\n\t\t\t**** MAIN MENU ****\n");
	printf("\n\t\t1  Configure Network Interfaces\n\n");
	printf("\t\t2  Enable/Disable Network Daemons and Add Static Routes\n\n");
	printf("\t\t3  Add/Delete Host Information\n\n");
	printf("\t\t4  Display Network Configuration\n\n");
	printf("\t\t5  Exit\n\n");
	prompt(choice, 2, "Enter the number for your choice: ");

	return(atoi(choice));
}


net_config()
{
	if_names_blk buf;

	/* find the network adapters on this machine */
	find_network_adapters(&buf);

	if (buf.curr_cnt <= 0)
		printf("Cannot find any network adapters on this machine\n");

	/* query the user for interface information */
	get_interface_info(&buf);

	return;
}


find_network_adapters(buf)
if_names_blk *buf;
{
	struct strioctl str;
	int fd;

	bzero(buf, sizeof(if_names_blk));
	if ((fd = open("/dev/streams/kinfo", O_RDWR, 0)) < 0) {
		printf("Cannot open /dev/streams/kinfo\n");
		return;
	}
	str.ic_cmd = KINFO_GET_IF_NAMES;
	str.ic_timout = 15;
	str.ic_len = sizeof(if_names_blk);
	str.ic_dp = (char *)buf;
	if (ioctl(fd, I_STR, &str) < 0) {
		perror(" ioctl");
	}
	close(fd);
	return;
}


/*
 * this routine query's the user for information pertaining to a 
 * network interface
 */
get_interface_info(ifnames_buf)
if_names_blk *ifnames_buf;
{
	int ask;
	int class;
	struct in_addr addr;
	FILE *fp;
	char buf[256];
	char cmd[256];
	char defaultval[256];
	char num_netconfig[256];
	char value[256];
	char hostname[256];
	char host[80];
	char domain[80];
	char ifname[256];
	char device[MAX_NETDEVS][80];
	char flags[256];
	char ipaddress[256];
	char ipaddress2[256];
	char netmask[256];
	char speed[8];
	char sliptty[256];
	char slipbaud[256];
	char slipflags[256];
	int localfound, hostfound;
	int remlocal, remhost, remaddr, tokenringdev;
	int i, next, firstfree, num, num_config, mode, slipdev, devsfound;
	char *def_netmask, *p;
	struct in_addr netmask_addr;
	struct stat statbuf;

	printf("\n***** CONFIGURE/DELETE NETWORK INTERFACES *****\n\n\
You can configure or delete network interfaces.  Configuration information\n\
is updated in %s and %s.  Choose \"configure\" or \"delete\"\n\
at the prompt.\n", RCCONFIG, HOSTS);

    for (;;) {
	if (prompt(buf, 128,
    		"\nEnter whether you want to \"(c)onfigure\" or \"(d)elete\" network interfaces.\n\
If you are finished, press the RETURN key: ") == NULL) {
		break;
	}
	if (strncmp(buf,"configure",strlen(buf)) == NULL) {
		if (yesno(YES,"You want to \"configure\" interfaces.  Is this correct"))
			mode = ADD;
		else
			continue;
	} else if (strncmp(buf,DELETE_STR,strlen(buf)) == NULL) {
		if (yesno(YES,"You want to \"delete\" interfaces.  Is this correct"))
			mode = DELETE;
		else
			continue;
	}
	else {
		printf("\n\"%s\" is invalid\n",buf);
		continue;
	}
	bzero(device, sizeof(device));
	if (mode == ADD) {
	   /****************************************************************
	    * network interface
	    */
	   next = 0;
	   do {
		/* fill in the default values */
		num_netconfig[0] = '\0';
		sprintf(cmd, "/usr/sbin/rcmgr get NUM_NETCONFIG");
		GET_VALUE(cmd, num_netconfig);
		if (num_netconfig[0] != '\0') {
			num_config = atoi(num_netconfig);
			/* check if the maximum number of network
			 * interfaces is already configured */
			if (num_config >= MAX_NETDEVS) {
				if (!yesno(NO,"\nThe maximum number of network interfaces is configured.\nDo you want to reconfigure one"))
				break;
			}
			if ((ifnames_buf->curr_cnt > 0) &&
			    (num_config == ifnames_buf->curr_cnt)) {
				if (!yesno(NO,"\nAll the network interfaces are configured.\nDo you want to reconfigure one"))
					break;
			}
		} else
			num_config = 0;

		hostname[0] = '\0';
		sprintf(cmd, "/usr/sbin/rcmgr get HOSTNAME");
		GET_VALUE(cmd, hostname);
		devsfound = 0;
		for (i = 0; i < MAX_NETDEVS; i++) {
			device[i][0] = '\0';
			sprintf(cmd, "/usr/sbin/rcmgr get NETDEV_%d", i);
			GET_VALUE(cmd, value);
			strcpy(&(device[i][0]),value);
			if (device[i][0] != '\0') {
				devsfound++;
				if (devsfound == num_config)
					break;
			}
		}

		/* set the maximum number of network adapters */
		sprintf(cmd, "/usr/sbin/rcmgr set MAX_NETDEVS %d",MAX_NETDEVS);
		if (system(cmd) < 0) {
			printf("error editing the /etc/rc.config file, errno = %d\n", errno);
			leave(1);
		}

		printf("\n\
You will now be asked a series of questions about the system.\n\
Default answers are shown in square brackets ([]).  To use a\n\
default answer, press the RETURN key.\n\n");

		if (ifnames_buf->curr_cnt > 0) {
			printf("This machine contains the following network interfaces:\n");
			for (i = 0; i < ifnames_buf->curr_cnt; i++) {
				printf("         %s%d\n", 
					ifnames_buf->info[i].if_name,
					ifnames_buf->info[i].if_unit);
			}
		}
		do {
			/* set default to next interface in the list,
			 * if there is one, else set default to null */
			if ((ifnames_buf->curr_cnt > 0) && (next >= 0)) {
				sprintf(defaultval,"%s%d", 
					ifnames_buf->info[next].if_name,
					ifnames_buf->info[next].if_unit);
			} else
				defaultval[0] = '\0';
			prompt(value, 10, 
				"\nWhich interface do you want to configure [%s]: ",
				defaultval);
			if (value[0] == '\0')
				strcpy(value,defaultval);
		} while (!yesno(YES, 
			"You want to configure \"%s\".  Is this correct", value));

		/* check that the user entered a valid device name which
		 * is one that is actually in the hardware configuration */
		if (ifnames_buf->curr_cnt > 0) {
			for (i = 0; i < ifnames_buf->curr_cnt; i++) {
				sprintf(defaultval,"%s%d", 
					ifnames_buf->info[i].if_name,
					ifnames_buf->info[i].if_unit);
				if (strcmp(defaultval,value) == 0)
					break;
			}
			if (i >= ifnames_buf->curr_cnt) {
				printf("Interface \"%s\" is invalid\n",value);
				continue;
			}
			/* save index to next interface in the list and
		 	 * if last one was configured, set next to -1 */
			next = ++i < ifnames_buf->curr_cnt ? i : -1;
		}

		/* Check that the user entered a device name that is
		 * already in the software configuration.  Find the
		 * index in the device array to either the NETDEV_n that
		 * matches a device the user wants to reconfigure or the
		 * first free NETDEV_n for configuring a new device. */
		firstfree = 0;
		for (i = 0; i < MAX_NETDEVS; i++) {
			if (strcmp(&(device[i][0]),value) == 0) {
				num = i;
				break;
			} else if ((device[i][0] == '\0') && (firstfree == 0)) {
				num = i;
				firstfree = 1;
			}
		}

		/* check to see if configuring a SLIP device */
		if (strncmp(value,"sl",2) == NULL) {
			slipdev = 1;	/* SLIP device */
			slipflags[0] = '\0';
			sliptty[0] = '\0';
			slipbaud[0] = '\0';
		} else
			slipdev = 0;	/* not a SLIP device */

		/* check to see if configuring a token ring device */
		if (strncmp(value,"tra",3) == NULL)
			tokenringdev = 1;	/* token ring device */
		else
			tokenringdev = 0;	/* not a token ring device */

		/* check to see if this is a new interface being configured
		 * or an existing interface that may be reconfigured */
		netmask[0] = '\0';
		bzero(flags, sizeof(flags));
		ipaddress[0] = '\0';
		ipaddress2[0] = '\0';
		speed[0] = '\0';
		if (device[num][0] == '\0') {
			/* configuring a new interface */
			sprintf(&(device[num][0]), "%s", value);
			num_config++;
			sprintf(num_netconfig,"%d",num_config);
		} else {
			/* ask user if they want to reconfigure an interface */
			printf("\nInterface \"%s\" is already configured.\n",
				value);
			if (yesno(NO, "Do you want to reconfigure it")) {
				value[0] = '\0';
				sprintf(cmd, "/usr/sbin/rcmgr get IFCONFIG_%d",
					num);
				GET_VALUE(cmd, value);
				if (slipdev) {
					/* get SLIP ifconfig parameters */
					sscanf(value, "%s %s netmask %s %256c",
					  ipaddress, ipaddress2, netmask, flags);
					/* get SLIP alattach parameters */
					value[0] = '\0';
					sprintf(cmd, "/usr/sbin/rcmgr get SLIPTTY_%d", num);
					GET_VALUE(cmd, value);
					if (value[0] != '\0') {
						if (strncmp(value,"tty",3) == NULL) {
							/* no flags */
							slipflags[0] = '\0';
							/* tty and baud rate */
							sscanf(value, "%s %s",
								sliptty, slipbaud);
						} else {
							/* flags */
							for (i = 0; ; i++)  {
							    if (value[i] != 't')
								slipflags[i] = value[i];
							    else {
								slipflags[i-1] = '\0';
								break;
							    }
							}
							/* tty and baud rate */
							sscanf(&value[i], "%s %s",
								sliptty, slipbaud);
						}
					}
				} else if (tokenringdev) {
					/* get ifconfig parameters */
					sscanf(value, "%s netmask %s speed %s %256c",
					      ipaddress, netmask, speed, flags);
				} else {
					/* get ifconfig parameters */
					sscanf(value, "%s netmask %s %256c",
						ipaddress, netmask, flags);
				}
			} else {
				if (ifnames_buf->curr_cnt <= 1)
					break;     /* done configuring */
				else
					continue;  /* configure next interface */
			}
		}

		if (slipdev)
			printf("\nInterface \"%s\" is a Serial Line Internet Protocol (SLIP) interface.\n", &(device[num][0]));

		if (tokenringdev)
			printf("\nInterface \"%s\" is a Token Ring interface.\n", &(device[num][0]));

		/***************************************************************
		 * hostname for the system - used to set HOSTNAME
		 */
		if (hostname[0] != '\0') {
			/* save original domain name if there is one */
			if ((p = strchr(hostname,'.')) == NULL)
				domain[0] = '\0';
			else
				strcpy(domain,p);
		}
		if (num_config == 1) {
			if (hostname[0] != '\0') {
				ask = FALSE;
				printf("\n");
			} else
				ask = TRUE;
			for (;;) {
				if (ask) {
					strcpy(defaultval, hostname);
					prompt(hostname, 256,
						"\nEnter the hostname for the system [%s]: ",
						defaultval);
					if (hostname[0] == '\0') {
						if (defaultval[0] == '\0') {
							printf("\nYou must enter a hostname for the system.");
							continue;
						} else
							strcpy(hostname, defaultval);
					}
				}
				/* if the user did not specify the domain name,
				   add the original domain name to the host
				   name entered by the user if there is one */
				if (((p = strchr(hostname,'.')) == NULL) &&
					(domain[0] != NULL))
						strcat(hostname,domain);
				ask = TRUE;
				if (yesno(YES,"The hostname for the system is \"%s\".\nIs this correct",hostname))
					break;
			}
			strcpy(ifname, hostname);
		}

		/***************************************************************
		 * name for the interface
		 */
		if (num_config > 1) {
			for (;;) {
				ifname[0] = '\0';
				prompt(ifname, 256,
					"\nEnter the name for %sinterface \"%s\" []: ",
					((slipdev == 0) ? "" : "SLIP "),
					&(device[num][0]));
				if (ifname[0] == '\0') {
					if (yesno(YES,"There is no name for interface \"%s\".\nIs this correct",&(device[num][0]),ifname))
						break;
				} else {
					/* if the user did not specify the
					   domain name, add the original domain
					   name to the interface name entered by
					   the user if there is one */
					if (((p = strchr(ifname,'.')) == NULL) &&
					     (domain[0] != NULL))
						strcat(ifname,domain);
					if (yesno(YES,"The name for %sinterface \"%s\" is \"%s\".\nIs this correct",
					    ((slipdev == 0) ? "" : "SLIP "),
					    &(device[num][0]),ifname))
						break;
				}
			}
		}

		/* save host/interface name without the domain name
		   if there is one */
		if ((p = strchr(ifname,'.')) == NULL)
			host[0] = '\0';
		else {
			strcpy(host,ifname);
			p = strchr(host,'.');
			*p = '\0';
		}

		/***************************************************************
		 * IP address
		 */
		if (ipaddress[0] != '\0') {
			ask = FALSE;
			printf("\n");
		} else 
			ask = TRUE;
		for (;;) {
			if (ask) {
				strcpy(defaultval, ipaddress);
				prompt(ipaddress, 256, 
					"\nEnter the Internet Protocol (IP) address for %sinterface \"%s\"\nin dot notation [%s]: ",
					((slipdev == 0) ? "" : "SLIP "),
					&(device[num][0]), defaultval);
				if (ipaddress[0] == '\0')
					strcpy(ipaddress, defaultval);
			}
			ask = TRUE;
			if (good_addr(ipaddress)) {
				if (yesno(YES,
				    "The IP address for %sinterface \"%s\" is \"%s\".\nIs this correct",
				    ((slipdev == 0) ? "" : "SLIP "),
				    &(device[num][0]), ipaddress))
					break;
			} else {
				printf("\n\
The format of the Internet Protocol (IP) address is incorrect.\n\
The address must consist of four integers seperated by '.'\n\
Each integer must be less than or equal to 255.  This number was assigned\n\
to you by the NIC.  An example IP address is: 130.180.3.12\n");
				ipaddress[0] = '\0';
			}
		}
		addr.s_addr = inet_addr(ipaddress);

		/***************************************************************
		 * IP address for remote SLIP interface
		 */
		if (slipdev) {
			if (ipaddress2[0] != '\0') {
				ask = FALSE;
				printf("\n");
			} else 
				ask = TRUE;
			for (;;) {
				if (ask) {
					strcpy(defaultval, ipaddress2);
					prompt(ipaddress2, 256, 
						"\nEnter the IP address of the remote SLIP interface\nin dot notation [%s]: ", defaultval);
					if (ipaddress2[0] == '\0')
						strcpy(ipaddress2, defaultval);
				}
				ask = TRUE;
				if (good_addr(ipaddress2)) {
					if (yesno(YES,
						"The IP address for the remote SLIP interface is \"%s\".\nIs this correct", ipaddress2))
						break;
				} else {
					printf("\n\
The format of the Internet Protocol (IP) address is incorrect.\n\
The address must consist of four integers seperated by '.'\n\
Each integer must be less than or equal to 255.  This number was assigned\n\
to you by the NIC.  An example IP address is: 130.180.3.12\n");
					ipaddress2[0] = '\0';
				}
			}
		}

		/***************************************************************
		 * netmask
		 */
		printf("\n\
Subnetworks allow the systems on a local area network to be on different\n\
physical networks.  For the following question, use the default answer\n\
unless the existing local area network is using subnet routing.\n\
If the local area network is using subnet routing, you need to know\n\
the subnet mask.\n\n");

		/* determine network class and default netmask */
		if (IN_CLASSA(ntohl(addr.s_addr))) {
			class = CLASSA;
			netmask_addr.s_addr = htonl(IN_CLASSA_NET);
			def_netmask = (char *)inet_ntoa(netmask_addr);
		} else if (IN_CLASSB(ntohl(addr.s_addr))) {
			class = CLASSB;
			netmask_addr.s_addr = htonl(IN_CLASSB_NET);
			def_netmask = (char *)inet_ntoa(netmask_addr);
		} else {
			class = CLASSC;
			netmask_addr.s_addr = htonl(IN_CLASSC_NET);
			def_netmask = (char *)inet_ntoa(netmask_addr);
		}

		if (netmask[0] != '\0') {
			strcpy(defaultval,netmask);
			ask = FALSE;
		} else {
			strcpy(defaultval,def_netmask);
			ask = TRUE;
		}

		for (;;) {
			if (ask) {
				/* if using subnet routing - prompt
				 * user for netmask */
				netmask[0] = '\0';
				prompt(netmask,256,
					"\nEnter the subnet mask in dot notation [%s]: ", defaultval);
				if (netmask[0] == '\0')
					strcpy(netmask, defaultval);

				ask = TRUE;
				if (!good_netmask(netmask, class)) {
					if (class == CLASSA) {
						printf("\n\
You provided an incorrect subnet mask.  The mask should be of the form\n\
255.n.n.n where each 'n' is a number less than or equal to 255.\n\
The default netmask is %s\n", defaultval);
						continue;
					} else {
						printf("\n\
You provided an incorrect subnet mask.  The mask should be of the form\n\
255.255.n.n where each 'n' is a number less than or equal to 255.\n\
The default netmask is %s\n", defaultval);
						continue;
					}
				}
			}
			ask = TRUE;
			if (yesno(YES,"The subnet mask for \"%s\" is \"%s\".\nIs this correct",&(device[num][0]),netmask))
				break;
		}

		/************************************************************
		 * Token ring specfic. Should be changed to identify
		 * new token ring devices as they are added to the product
		 * line.
		 */
		if (tokenringdev) {
		    for (;;) {
			printf("\n\
The Token Ring interface operates at 16Mbs or 4Mbs. Please specify the\n\
speed at which the adapter should enter the ring.\n");
			strcpy(defaultval, "16");
			prompt(speed, 8,
			       "\nEnter the Token Ring speed [%s]: ", defaultval);
			if (speed[0] == '\0') {
				strcpy(speed, defaultval);
			} else if (!good_speed(speed)) {
				printf("speed \"%s\" is invalid\n", speed);
				continue;
			}
			if (yesno(YES,"The interface will enter the ring at %sMbs. Is this correct", speed))
				break;
		    }			
		}

		/***************************************************************
		 * ifconfig flags
		 */
		printf("\n\
For the following question USE THE DEFAULT ANSWER unless you would like\n\
to add additional flags (found in the ifconfig reference page) to the \n\
ifconfig command.  Normally, you will USE THE DEFAULT ANSWER.\n");

		if (flags[0] != '\0') {
			if (yesno(YES,"\nThe ifconfig flags for \"%s\" are \"%s\".\nIs this correct",&(device[num][0]),flags))
				ask = FALSE;
			else {
				flags[0] = '\0';
				ask = TRUE;
			}
		}

		if (ask) {
			/* 
			 * there are no ifconfig flags defined in /etc/rc.config			 * so put one in there 
			 */
			ask = yesno(NO,"\nDo you want to use additional ifconfig flags for this interface");
			if (ask) {
				for (;;) {
					flags[0] = '\0';
					printf("The current ifconfig command looks like:\n\n\
\tifconfig %s %s%s%s netmask %s%s%s\n", &(device[num][0]), ipaddress,
			   ((ipaddress2[0] == '\0') ? "" : " "),
			   ipaddress2, netmask,
			   ((speed[0] == '\0') ? "" : " speed "), speed);
					prompt(flags,256,
						"\nEnter any additional flags separated by a space: ");
					if (flags[0] == '\0') {
						if (yesno(YES,"You do not want any additional flags for \"%s\".\nIs this correct", &(device[num][0])))
							break;
					} else {
						if (yesno(YES,"The ifconfig flags for \"%s\" are \"%s\".\nIs this correct", &(device[num][0]), flags))
							break;
					}
				}
			} 
		}

		if (num_config == 1) {
			if (!yesno(YES,"\nThe configuration looks like:\n\n\
\tsystem hostname: \"%s\"\n\
\tifconfig %s %s%s%s netmask %s%s%s%s%s\n\nIs this correct",
			   hostname, &(device[num][0]), ipaddress,
			   ((ipaddress2[0] == '\0') ? "" : " "),
			   ipaddress2, netmask,
			   ((speed[0] == '\0') ? "" : " speed "), speed,
			   ((flags[0] == '\0') ? "" : " "), flags))
				break;
		} else {
			if (!yesno(YES,"\nThe configuration looks like:\n\n\
\tinterface \"%s\" name: \"%s\"\n\
\tifconfig %s %s%s%s netmask %s%s%s%s%s\n\nIs this correct",
			   &(device[num][0]), ifname,
			   &(device[num][0]), ipaddress,
			   ((ipaddress2[0] == '\0') ? "" : " "),
			   ipaddress2, netmask,
			   ((speed[0] == '\0') ? "" : " speed "), speed,
			   ((flags[0] == '\0') ? "" : " "), flags))
				break;
		}

		/*************************************************************
		 * slattach command paramters for SLIP interface
		 */
		if (slipdev) {
		    do {
			printf("\n\
The slattach command attaches a tty line to a Serial Line Internet\n\
Protocol (SLIP) interface (see the slattach reference page).\n");

			/* slattach flags */
			if (slipflags[0] == '\0')
				ask  = TRUE;
			else {
				ask = FALSE;
				printf("\n");
			}
			for (;;) {
				if (ask) {
					prompt(slipflags,80,"\nEnter the slattach flags for \"%s\" separated by a space.\n\
If you do not want any flags, press the RETURN key: ", &(device[num][0]));
				}
				ask = TRUE;
				if (slipflags[0] == '\0') {
					if (yesno(YES,"You do not want any flags.  Is this correct"))
						break;
				} else {
					if (yesno(YES,"The slattach flag%s %s %s.  Is this correct",
				   	   plural(slipflags) ? "s" : "",
				   	   plural(slipflags) ? "are" : "is",
				   	   quote(slipflags)))
						break;
				}
			}

			/* tty line to attach to SLIP interface */
			if (sliptty[0] == '\0')
				ask  = TRUE;
			else {
				ask = FALSE;
				printf("\n");
			}
			for (;;) {
				if (ask) {
					strcpy(defaultval, sliptty);
					prompt(sliptty,80,"\nEnter the tty line to attach to \"%s\" [%s]: ", &(device[num][0]), defaultval);
				}
				if (sliptty[0] == '\0')
					strcpy(sliptty, defaultval);
				ask = TRUE;
				if (yesno(YES,"The tty line to attach to \"%s\" is \"%s\".  Is this correct", &(device[num][0]), sliptty))
					break;
			}

			/* tty line baud rate */
			if (slipbaud[0] == '\0')
				strcpy(slipbaud,"9600");  /* default baud rate */
			ask = FALSE;
			printf("\n");
			for (;;) {
				if (ask) {
					strcpy(defaultval,"9600");
					prompt(slipbaud,80,"\nEnter the baud rate for tty line \"%s\" [%s]: ", sliptty, defaultval);
				}
				if (slipbaud[0] == '\0')
					strcpy(slipbaud, defaultval);
				ask = TRUE;
				if (yesno(YES,"The baud rate for tty line \"%s\" is \"%s\".  Is this correct", sliptty, slipbaud))
					break;
				strcpy(defaultval, slipbaud);
			}
		    } while (!yesno(YES,"\nThe slattach command looks like:\n\n\
\tslattach %s%s%s %s \n\nIs this correct",
			   slipflags, ((slipflags[0] == '\0') ? "" : " "),
			   sliptty, slipbaud));
		}

		/**************************************************************
		 * update rc.config
		 */
		printf("\n***** UPDATING /etc/rc.config *****\n\n");
		sprintf(cmd, "/usr/sbin/rcmgr set HOSTNAME %s", hostname);
		if (system(cmd) < 0) {
			printf("error editing the /etc/rc.config file, errno = %d\n", errno);
			leave(1);
		}

		sprintf(cmd, "/usr/sbin/rcmgr set NUM_NETCONFIG %s", num_netconfig);
		if (system(cmd) < 0) {
			printf("error editing the /etc/rc.config file, errno = %d\n", errno);
			leave(1);
		}

		sprintf(cmd,"/usr/sbin/rcmgr set NETDEV_%d %s",num,&(device[num][0]));
		if (system(cmd) < 0) {
			printf("error editing the /etc/rc.config file, errno = %d\n", errno);
			leave(1);
		}

		sprintf(cmd, "/usr/sbin/rcmgr set IFCONFIG_%d \"%s%s%s netmask %s%s%s%s%s\"",
			num, ipaddress, ((ipaddress2[0] == '\0') ? "" : " "),
			ipaddress2, netmask,
			((speed[0] == '\0') ? "" : " speed "), speed,
			((flags[0] == '\0') ? "" : " "), flags);

		if (system(cmd) < 0) {
			printf("error editing the /etc/rc.config file, errno = %d\n", errno);
			leave(1);
		}

		if (slipdev) {
			sprintf(cmd, "/usr/sbin/rcmgr set SLIPTTY_%d \"%s%s%s %s\"",
				num, slipflags,
				((slipflags[0] == '\0') ? "" : " "),
				sliptty, slipbaud);
			if (system(cmd) < 0) {
				printf("error editing the /etc/rc.config file, errno = %d\n", errno);
				leave(1);
			}
		}

		printf("\"%s\" is configured in %s\n",&(device[num][0]),RCCONFIG);

		/**************************************************************
		 * update /etc/hosts
		 */

		/* look to see if entries exist for localhost and for host
		 * with same name or IP address */
		hostfound = FALSE;
		localfound = FALSE;
		remlocal = FALSE;
		remhost = FALSE;
		remaddr = FALSE;
		fgrep(LOCALHOST,HOSTS,buf,sizeof(buf));
		if (buf[0] != '\0') {
			cmd[0] = '\0';
			sscanf(buf, "%s", cmd);
			if (strcmp(cmd, "127.0.0.1") == 0)
				localfound = TRUE;
			else
				remlocal = TRUE;
			fgrep(ifname,HOSTS,buf,sizeof(buf));
			if (buf[0] != '\0') {
				cmd[0] = '\0';
				sscanf(buf, "%s", cmd);
				if (strcmp(cmd, ipaddress) == 0)
					hostfound = TRUE;
				else
					remhost = TRUE;
			}
			if (!hostfound && ifname[0] != '\0') {
				fgrep(ipaddress,HOSTS,buf,sizeof(buf));
				if (buf[0] != '\0') {
					cmd[0] = '\0';
					sscanf(buf, "%s", cmd);
					if (strcmp(cmd, ipaddress) == 0)
						remaddr = TRUE;
				}
			}
		}

		if (!localfound || (!hostfound && ifname[0] != '\0')) {
			fp = edit(HOSTS);
			if (fp == NULL) {
				fprintf(stderr, "Can't open %s for editing\n", HOSTS);
				prompt(cmd,10,"Press RETURN to continue ");
			} else {
				printf("\n***** UPDATING %s *****\n\n",HOSTS);
				if (remlocal) {
					fprintf(fp, "g/^.*[ 	]%s.*$/d\n", LOCALHOST);
					fprintf(fp, "w\n");
				}
				if (remhost) {
					fprintf(fp, "g/^.*[ 	]%s.*$/d\n", ifname);
					fprintf(fp, "w\n");
				}
				if (remaddr) {
					fprintf(fp, "g/^.*%s.*$/d\n", ipaddress);
					fprintf(fp, "w\n");
				}
				fprintf(fp, "l\n");
				fprintf(fp, "$a\n");
				if (!localfound) 
					fprintf(fp, "127.0.0.1 %s\n", LOCALHOST);
				if (!hostfound && ifname[0] != '\0')
					fprintf(fp, "%s %s%s%s\n", ipaddress,
						ifname, host[0] ? " " : "",
						host[0] ? host : "");
				fprintf(fp, ".\n");
				fprintf(fp, "w\n");
				fprintf(fp, "q\n");
				fflush(fp);
				pclose(fp);
				printf("\"%s %s%s%s\" is configured in %s\n",
					ipaddress, ifname, host[0] ? " " : "",
					host[0] ? host : "",HOSTS);
			}
		}
		/* finished adding if only one interface on the system or if
		 * all interfaces on the system are configured */
		if ((ifnames_buf->curr_cnt <= 1) ||
		    (num_config == ifnames_buf->curr_cnt))
			break;
		/* finished adding if the maximum number of
		 * interfaces are configured */
		if (num_config >= MAX_NETDEVS) {
			printf("\nThe maximum number of network interfaces is configured\n");
			break;
		}
	   } while (yesno(YES,"\nDo you want to configure another network interface"));
	} else { /* deleting network interfaces */
	   do {
		/* get current software configuration */
		num_netconfig[0] = '\0';
		sprintf(cmd, "/usr/sbin/rcmgr get NUM_NETCONFIG");
		GET_VALUE(cmd, num_netconfig);
		if (num_netconfig[0] != '\0')
			num_config = atoi(num_netconfig);
		else
			num_config = 0;

		if (num_config == 0) {
			printf("\nThere are no network interfaces configured\n");
			break;
		}
		if (num_config == 1) {
			if (!yesno(NO,"\nThere is only one interface configured.\nDo you want to continue"))
				break;
		}
		devsfound = 0;
		for (i = 0; i < MAX_NETDEVS; i++) {
			device[i][0] = '\0';
			sprintf(cmd, "/usr/sbin/rcmgr get NETDEV_%d", i);
			GET_VALUE(cmd, value);
			strcpy(&(device[i][0]),value);
			if (device[i][0] != '\0') {
				devsfound++;
				if (devsfound == num_config)
					break;
			}
		}

		printf("\n\
Enter the name of the network interface you want to delete when prompted.\n\
WARNING!!  When you delete an interface, it will automatically be taken\n\
down.  Before deleting an interface, make sure that there are no users\n\
accessing that interface.  If you do not want to delete an interface,\n\
press RETURN at the prompt that asks which interface you want to delete.\n\n\
The following network interfaces are configured:\n");
		devsfound = 0;
		for (i = 0; i < MAX_NETDEVS; i++) {
			if (device[i][0] != '\0') {
				printf("\t%s\n",&(device[i][0]));
				devsfound++;
				if (devsfound == num_config)
					break;
			}
		}
		printf("\n");

		/* find the default value for the interface name */
		for (i = 0; i < MAX_NETDEVS; i++) {
			if ((device[i][0]) != '\0')
				break;
		}

		/* set default to next interface in the list,
		 * if there is one, else set default to null */
		if (i < MAX_NETDEVS) {
			strcpy(defaultval,&(device[i][0]));
		} else
			defaultval[0] = '\0';

		do {
			prompt(ifname, 10, 
				"Which interface do you want to delete [%s]: ",
				defaultval);
			if (ifname[0] == '\0')
				strcpy(ifname,defaultval);
		} while (!yesno(YES, 
			"You want to delete \"%s\".  Is this correct", ifname));

		/* check to see that the interface the user wants to delete
		 * is in the software configuration */
		for (num = 0; num < MAX_NETDEVS; num++) {
			if (strcmp(&(device[num][0]),ifname) == 0)
				break;
		}
		if (num >= MAX_NETDEVS) {
			printf("\n\"%s\" is not configured\n",ifname);
			printf("\"%s\" is not deleted\n",ifname);
			continue;
		}

		/* check to see if deleting a SLIP device */
		if (strncmp(ifname,"sl",2) == NULL) {
			slipdev = 1;	/* SLIP device */
			printf("\nInterface \"%s\" is a Serial Line Internet Protocol (SLIP) interface.\n", ifname);
		} else
			slipdev = 0;	/* not a SLIP device */

		/* get IP address of interface being deleted */
		value[0] = '\0';
		sprintf(cmd, "/usr/sbin/rcmgr get IFCONFIG_%d", num);
		GET_VALUE(cmd, value);
		sscanf(value,"%s",ipaddress);

		/* if SLIP interface, kill slattach process if it exists */
		if (slipdev) {
			/* get SLIP alattach parameters */
			value[0] = '\0';
			sprintf(cmd, "/usr/sbin/rcmgr get SLIPTTY_%d", num);
			GET_VALUE(cmd, value);
			if (value[0] != '\0') {
				if (strncmp(value,"tty",3) == NULL) {
					/* get tty name */
					sscanf(value, "%s", sliptty);
				} else {
					/* skip over flags */
					for (i = 0; ; i++)  {
					    if (value[i] == 't')
						break;
					}
					/* get tty name */
					sscanf(&value[i], "%s", sliptty);
				}
				/* look for slattach proccess id file */
				sprintf(value, "/var/run/%s.pid", sliptty);
				if (stat(value, &statbuf) == 0) {
					/* kill slattach for interface */
					sprintf(cmd, "/bin/kill `/bin/cat %s`", value);
					printf("\n***** KILLING SLATTACH FOR \"%s\" *****\n\n\t%s\n\n", sliptty, cmd);
					if (system(cmd) != 0)
						printf("error killing slattach for \"%s\", errno = %d\n", sliptty, errno);
				}
			}
		}

		/* shut down the interface */
		sprintf(cmd, "/sbin/ifconfig %s down", ifname);
		printf("\n***** TAKING DOWN \"%s\" *****\n\n\t%s\n\n", ifname, cmd);
		if (system(cmd) != 0)
			printf("ifconfig error, errno = %d\n", errno);
		printf("Interface \"%s\" is down\n", ifname);

		/* delete interface from /etc/rc.config */
		printf("\n***** UPDATING %s *****\n\n",RCCONFIG);
		/* decrement number of interfaces configured */
		num_config--;
		sprintf(num_netconfig,"%d",num_config);
		fp = edit(RCCONFIG);
		if (fp == NULL) {
			fprintf(stderr, "%s is not set up.\n", RCCONFIG);
			prompt(cmd,10,"Press RETURN to continue ");
			return;
		}
		fprintf(fp,"/^NUM_NETCONFIG=.*$/s//NUM_NETCONFIG=\"%d\"/\n",
			num_config);
		device[num][0] = '\0';
		fprintf(fp,"/^NETDEV_%d=.*$/s//NETDEV_%d=/\n", num, num);
		fprintf(fp,"/^IFCONFIG_%d=.*$/s//IFCONFIG_%d=/\n", num, num);
		if (slipdev)
			fprintf(fp,"/^SLIPTTY_%d=.*$/s//SLIPTTY_%d=/\n", num, num);
		/* when deleting last interface, reset other software
		 * configuration variables */
		if (num_config == 0) {
			fprintf(fp,"/^RWHOD=.*$/s//RWHOD=/\n");
			fprintf(fp,"/^ROUTED=.*$/s//ROUTED=/\n");
			fprintf(fp,"/^ROUTED_FLAGS=.*$/s//ROUTED_FLAGS=/\n");
			fprintf(fp,"/^GATED=.*$/s//GATED=/\n");
			fprintf(fp,"/^GATED_FLAGS=.*$/s//GATED_FLAGS=/\n");
		}
		fprintf(fp, "w\n");
		fprintf(fp, "q\n");
		fflush(fp);
		pclose(fp);
		printf("\"%s\" is deleted from %s\n", ifname, RCCONFIG);

		/* delete interface from /etc/hosts */
		fgrep(ipaddress,HOSTS,buf,sizeof(buf));
		if (buf[0] != '\0') {
			printf("\n***** UPDATING %s *****\n\n",HOSTS);
			fp = edit(HOSTS);
			if (fp == NULL) {
				fprintf(stderr, "%s is not set up.\n", HOSTS);
				prompt(cmd,10,"Press RETURN to continue ");
				return;
			}
			fprintf(fp, "g/^.*%s.*$/d\n", ipaddress);
			fprintf(fp, "w\n");
			fprintf(fp, "q\n");
			fflush(fp);
			pclose(fp);
			printf("\"%s\" is deleted from %s\n", buf, HOSTS);
		}

		/* warn user that no interfaces are configured */
		if (num_config == 0) {
			printf("\nThere are no network interfaces configured\n");
			break;
		}
	   } while (yesno(NO,"\nDo you want to delete another network interface"));
	}
    }
}


net_daemons()
{
	char cmd[256];
	char value[256];
	FILE *fp;
	char answer[256];
	char dest[10];
	char host1[100];
	char host2[100];
	char metric[10];
	char routed[10];
	char gated[10];
	char flags[100];
	char type[10];
	char ifconfig[MAX_NETDEVS][80];
	int ask, len, num_config, destflag, i, found, interface;
	int router;
	struct stat statbuf;
	char *p;

	printf("\n***** ENABLE/DISABLE NETWORK DAEMONS AND ADD STATIC ROUTES *****\n");

	sprintf(cmd, "/usr/sbin/rcmgr get NUM_NETCONFIG");
	GET_VALUE(cmd, value);
	num_config = atoi(value);

	if (num_config == 0) {
		printf("\n\
There are no network interfaces configured.  First configure\n\
the network interfaces and then configure the daemons.\n");
		return;
	} else {
		printf("\n\
You can choose whether you want to enable or disable rwhod (rwho\n\
daemon), and either gated (gateway daemon) or routed (route daemon).\n\
You can also configure static routes.  Daemons and static route commands\n\
are executed when the network is started up.\n");
	}

	if (yesno(NO,"\nDo you want to run rwhod")) {
		sprintf(cmd, "/usr/sbin/rcmgr set RWHOD yes");
		if (system(cmd) < 0) {
			printf("error editing the /etc/rc.config file, errno = %d\n", errno);
			leave(1);
		}
		printf("\nrwhod is enabled.\n");
	} else {
		sprintf(cmd, "/usr/sbin/rcmgr set RWHOD no");
		if (system(cmd) < 0) {
			printf("error editing the /etc/rc.config file, errno = %d\n", errno);
			leave(1);
		}
		printf("\nrwhod is disabled.\n");
	}

	if (num_config == 1) {
		/* turn routing off */
		sprintf(cmd, "/usr/sbin/rcmgr set ROUTER no");
		if (system(cmd) < 0) {
			printf("error editing the /etc/rc.config file, errno = %d\n", errno);
			leave(1);
		}
		if (system("/usr/sbin/iprsetup -r") < 0)
			printf("route setup error, errno = %d\n", errno);
	} else {
		if (yesno(NO,"\nAre you setting up your system to be an IP router")) {
			/* turn routing on */
			sprintf(cmd, "/usr/sbin/rcmgr set ROUTER yes");
			if (system(cmd) < 0) {
				printf("error editing the /etc/rc.config file, errno = %d\n", errno);
				leave(1);
			}
			if (system("/usr/sbin/iprsetup -s") < 0)
				printf("route setup error, errno = %d\n", errno);
			router = 1;
		} else {
			/* turn routing off */
			sprintf(cmd, "/usr/sbin/rcmgr set ROUTER no");
			if (system(cmd) < 0) {
				printf("error editing the /etc/rc.config file, errno = %d\n", errno);
				leave(1);
			}
			if (system("/usr/sbin/iprsetup -r") < 0)
				printf("route setup error, errno = %d\n", errno);
			router = 0;
		}
	}

	/* configure routed, gated or neither */
	printf("\n\
You can run either gated or routed but not both.  Choose \"(g)ated\"\n\
or \"(r)outed\" at the prompt.\n");
	for (;;) {
		prompt(answer,20,
			"\nDo you want to run \"(g)ated\" or \"(r)outed.\n\
If you do not want either, then press the RETURN key: ");
		len = strlen(answer);
		if (answer[0] == NULL) {
			if (yesno(YES,"You do not want to run gated or routed.  Is this correct"))
			{
				sprintf(cmd, "/usr/sbin/rcmgr set ROUTED no");
				if (system(cmd) < 0) {
					printf("error editing the /etc/rc.config file, errno = %d\n", errno);
					leave(1);
				}
				sprintf(cmd, "/usr/sbin/rcmgr set GATED no");
				if (system(cmd) < 0) {
					printf("error editing the /etc/rc.config file, errno = %d\n", errno);
					leave(1);
				}
				fp = edit(RCCONFIG);
				if (fp == NULL) {
					fprintf(stderr, "%s is not set up.\n", RCCONFIG);
					prompt(cmd,10,"Press RETURN to continue ");
					return;
				}
				fprintf(fp,"/^ROUTED_FLAGS=.*$/s//ROUTED_FLAGS=/\n");
				fprintf(fp,"/^GATED_FLAGS=.*$/s//GATED_FLAGS=/\n");
				fprintf(fp, "w\n");
				fprintf(fp, "q\n");
				fflush(fp);
				pclose(fp);
				printf("\nrouted and gated are disabled.\n");
				break;
			}
		} else if (strncmp(answer,"routed",len) == NULL) {
			if (yesno(YES,"You want to run routed.  Is this correct"))
			{
				sprintf(cmd, "/usr/sbin/rcmgr get ROUTED_FLAGS");
				GET_VALUE(cmd, flags);
				ask = FALSE;
				if (flags[0] == '\0')
					strcpy(flags,"-q");
				printf("\n\
You can use flags (see the routed reference page) when you run\n\
the route daemon.\n");

				for (;;) {
					if (ask) {
						prompt(flags,80,"\nEnter the routed flags separated by a space.\n\
If you do not want any flags, press the RETURN key: ");
					}
					ask = TRUE;
					if (flags[0] == '\0') {
						if (yesno(YES,"You do not want any flags.  Is this correct"))
						{
							fp = edit(RCCONFIG);
							if (fp == NULL) {
								fprintf(stderr, "%s is not set up.\n", RCCONFIG);
								prompt(cmd,10,"Press RETURN to continue ");
								return;
							}
							fprintf(fp,"/^ROUTED_FLAGS=.*$/s//ROUTED_FLAGS=/\n");
							fprintf(fp, "w\n");
							fprintf(fp, "q\n");
							fflush(fp);
							pclose(fp);
							break;
						}
					} else {
						if (yesno(YES,"The routed flag%s %s %s.  Is this correct",
					   	   plural(flags) ? "s" : "",
					   	   plural(flags) ? "are" : "is",
					   	   quote(flags)))
						{
							sprintf(cmd, "/usr/sbin/rcmgr set ROUTED_FLAGS \"%s\"", flags);
							if (system(cmd) < 0) {
								printf("error editing the /etc/rc.config file, errno = %d\n", errno);
								leave(1);
							}
							break;
						}
					}
				}

				sprintf(cmd, "/usr/sbin/rcmgr set ROUTED yes");
				if (system(cmd) < 0) {
					printf("error editing the /etc/rc.config file, errno = %d\n", errno);
					leave(1);
				}
				sprintf(cmd, "/usr/sbin/rcmgr set GATED no");
				if (system(cmd) < 0) {
					printf("error editing the /etc/rc.config file, errno = %d\n", errno);
					leave(1);
				}
				fp = edit(RCCONFIG);
				if (fp == NULL) {
					fprintf(stderr, "%s is not set up.\n", RCCONFIG);
					prompt(cmd,10,"Press RETURN to continue ");
					return;
				}
				fprintf(fp,"/^GATED_FLAGS=.*$/s//GATED_FLAGS=/\n");
				fprintf(fp, "w\n");
				fprintf(fp, "q\n");
				fflush(fp);
				pclose(fp);
				printf("\nrouted is enabled.\n");
				break;
			}
		} else if (strncmp(answer,"gated",len) == NULL) {
			if (yesno(YES,"You want to run gated.  Is this correct"))
			{
				sprintf(cmd, "/usr/sbin/rcmgr get GATED_FLAGS");
				GET_VALUE(cmd, flags);
				if (flags[0] == '\0') {
					ask = TRUE;
				} else
					ask = FALSE;
				printf("\n\
You can use flags (see the gated reference page) when you run\n\
the gateway daemon.\n");

				for (;;) {
					if (ask) {
						prompt(flags,80,"\nEnter the gated flags separated by a space.\n\
If you do not want any flags, press the RETURN key: ");
					}
					ask = TRUE;
					if (flags[0] == '\0') {
						if (yesno(YES,"You do not want any flags.  Is this correct"))
						{
							fp = edit(RCCONFIG);
							if (fp == NULL) {
								fprintf(stderr, "%s is not set up.\n", RCCONFIG);
								prompt(cmd,10,"Press RETURN to continue ");
								return;
							}
							fprintf(fp,"/^GATED_FLAGS=.*$/s//GATED_FLAGS=/\n");
							fprintf(fp, "w\n");
							fprintf(fp, "q\n");
							fflush(fp);
							pclose(fp);
							break;
						}
					} else {
						if (yesno(YES,"The gated flag%s %s %s.  Is this correct",
					   	   plural(flags) ? "s" : "",
					   	   plural(flags) ? "are" : "is",
					   	   quote(flags)))
						{
							sprintf(cmd, "/usr/sbin/rcmgr set GATED_FLAGS \"%s\"", flags);
							if (system(cmd) < 0) {
								printf("error editing the /etc/rc.config file, errno = %d\n", errno);
								leave(1);
							}
							break;
						}
					}
				}
				if ((stat(GATEDCONF, &statbuf) == -1) &&
				    (errno == ENOENT)) {
					sprintf(value, "passiveinterfaces ");
					found = 0;
					for (i = 0; i < MAX_NETDEVS; i++) {
						sprintf(cmd, "/usr/sbin/rcmgr get IFCONFIG_%d", i);
						GET_VALUE(cmd, answer);
						strcpy(&(ifconfig[i][0]),answer);
						if ((ifconfig[i][0]) != '\0') {
							/* isolate the IP address */
							p = strchr(&(ifconfig[i][0]), ' ');
							*++p = '\0';
							strcat(value, &(ifconfig[i][0]));
							found++;
							if (found == num_config)
								break;
						}
					}
					if (router)
						strcat(value, "\nRIP yes\nEGP no\n");
					else
						strcat(value, "\nRIP quiet\nEGP no\n");
					/* create default gated.conf file */
					printf("\n***** CREATING DEFAULT /etc/gated.conf FILE *****\n\n");
					fp = edit(GATEDCONF);
					if (fp == NULL) {
						fprintf(stderr, "%s is not set up.\n", RCCONFIG);
						prompt(cmd,10,"Press RETURN to continue ");
						return;
					}
					fprintf(fp, "a\n");
					fprintf(fp, value);
					fprintf(fp, ".\n");
					fprintf(fp, "w\n");
					fprintf(fp, "q\n");
					fflush(fp);
					pclose(fp);
				}
				sprintf(cmd, "/usr/sbin/rcmgr set GATED yes");
				if (system(cmd) < 0) {
					printf("error editing the /etc/rc.config file, errno = %d\n", errno);
					leave(1);
				}
				sprintf(cmd, "/usr/sbin/rcmgr set ROUTED no");
				if (system(cmd) < 0) {
					printf("error editing the /etc/rc.config file, errno = %d\n", errno);
					leave(1);
				}
				fp = edit(RCCONFIG);
				if (fp == NULL) {
					fprintf(stderr, "%s is not set up.\n", RCCONFIG);
					prompt(cmd,10,"Press RETURN to continue ");
					return;
				}
				fprintf(fp,"/^ROUTED_FLAGS=.*$/s//ROUTED_FLAGS=/\n");
				fprintf(fp, "w\n");
				fprintf(fp, "q\n");
				fflush(fp);
				pclose(fp);
				printf("\ngated is enabled.\n");
				break;
			}
		} else
			printf("\n\"%s\" is invalid\n",answer);
	}

	/* add static routes */
	printf("\n\
Static route commands can be configured in %s that will be\n\
executed when the network is started up on this system.\n", ROUTES);
	for (;;) {
		if (!yesno(NO,"\nDo you want to add a static route"))
			break;

		dest[0] = '\0';
		for (;;) {
			prompt(dest,10,
				"\nYou can route to a default gateway, a network or a host.\
\nEnter \"(d)efault\", \"(n)et\", or \"(h)ost\": ");

			if (dest[0] == '\0') {
				if (yesno(YES,"Are you finished"))
					break;
				else
					continue;
			}
			destflag = 0;
			len = strlen(dest);
			/* check for host or network destination */
			if (strncmp(dest,"host",len) == NULL) {
				destflag = R_HOST;
				strcpy(dest,"host");
			} else if (strncmp(dest,"net",len) == NULL) {
				destflag = R_NETWORK;
				strcpy(dest,"net");
			} else if (strncmp(dest,"default",len) == NULL) {
				/* /etc/routes default gateway */
				destflag = R_DEFAULT;
				strcpy(dest,"default");
			} else {
				printf("\n\"%s\" is invalid\n",dest);
				continue;
			}
			if (yesno(YES,"You want to route to \"%s\".  Is this correct",dest)) {
				/* set host or network destination string */
				switch (destflag) {
					case R_DEFAULT:
						/* strings are set above */
						break;
					case R_HOST:
						strcpy(dest,"-host");
						break;
					case R_NETWORK:
						strcpy(dest,"-net");
						break;
					default:
						destflag = 0;
						printf("\n\"%s\" is invalid\n",dest);
						break;
				}
				if (destflag > 0)
					break;
			}
		}
		if (dest[0] == '\0')
			break;

		host1[0] = '\0';
		if (destflag != R_DEFAULT) {
			for (;;) {
				prompt(host1,100,
					"\nEnter the destination name or IP address: ");
				if (host1[0] == '\0') {
					if (yesno(YES,"Are you finished"))
						break;
					else
						continue;
				}
				/* check for valid host or network destination */
				if (destflag & R_HOST) {
					/* check for valid host address or name */
					if ((inet_addr(host1) == -1) &&
				    	(gethostbyname(host1) == 0)) {
						printf("\n\"%s\" is invalid\n",host1);
						continue;
					}
				} else if (destflag & R_NETWORK) {
					/* check for valid network address or name */
					if ((inet_network(host1) == -1) &&
				    	(getnetbyname(host1) == 0)) {
						printf("\nNetwork \"%s\" is invalid\n",host1);
						continue;
					}
				}

				if (yesno(YES,"The destination is \"%s\".  Is this correct",host1))
					break;
			}
			if (host1[0] == '\0')
				continue;
		}

		interface = 0;
		if (destflag != R_DEFAULT) {
			for (;;) {
				strcpy(value,STR_GATEWAY);
				prompt(answer,20,
"\nYou can route to \"%s\" via a gateway or an interface.\n\
Enter \"(g)ateway\" or \"(i)nterface\" [%s]: ",host1,value);
				len = strlen(answer);
				if (len == 0 ||
				    strncmp(answer,STR_GATEWAY,len) == NULL)
					interface = 0;
				else if (strncmp(answer,STR_INTERFACE,len) == NULL)
					interface = 1;
				else {
					printf("\n\"%s\" is invalid\n",answer);
					continue;
				}
				if (yesno(YES,"You want to route to \"%s\" via %s.\nIs this correct",host1,interface ? "an interface" : "a gateway"))
					break;
			}
		}

		for (;;) {
			prompt(host2,100,"\nEnter the %s name or IP address: ",interface ? "interface" : "gateway");

			if (host2[0] == '\0') {
				if (yesno(YES,"Are you finished"))
					break;
				else
					continue;
			}
			if (interface) {
				/* check for a valid host name or IP address */
				if ((inet_addr(host2) == -1) &&
			    	    (gethostbyname(host2) == 0)) {
					found = 0;
					/* check for a valid interface name */
					for (i = 0; i < MAX_NETDEVS; i++) {
						sprintf(cmd, "/usr/sbin/rcmgr get NETDEV_%d", i);
						GET_VALUE(cmd, answer);
						if (strcmp(host2, answer) == 0) {
							sprintf(cmd, "/usr/sbin/rcmgr get IFCONFIG_%d", i);
							GET_VALUE(cmd, answer);
							host2[0] = '\0';
							sscanf(answer, "%s", host2);
							found++;
							break;
						}
					}
					if (!found) {
						printf("\n\"%s\" is invalid\n", host2);
						continue;
					}
				}
			}
			else if ((inet_addr(host2) == -1) &&
			    (gethostbyname(host2) == 0)) {
				printf("\n\"%s\" is invalid\n", host2);
				continue;
			}
			if (yesno(YES,"The %s is \"%s\".  Is this correct",
					interface ? "interface" : "gateway",
					host2))
				break;
		}
		if (host2[0] == '\0')
			continue;

		if (yesno(YES,"\nThe static route command looks like:\n\n\
\t/sbin/route add %s %s%s%s\n\nIs this correct",dest,host1,
interface ? " -interface " : " ",host2))
		{
			fp = edit(ROUTES);
			if (fp == NULL) {
				fprintf(stderr, "cannot edit %s\n", ROUTES);
				prompt(cmd,10,"Press RETURN to continue ");
				return;
			}
			fprintf(fp, "$a\n%s %s%s%s\n",dest,host1,
				interface ? " -interface " : " ",host2);
			fprintf(fp, ".\n");
			fprintf(fp, "w\n");
			fflush(fp);
			printf("\n\"%s %s%s%s\" is added to %s\n",dest,
				host1,interface ? " -interface " : " ",
				host2,ROUTES);
			fprintf(fp, "q\n");
			fflush(fp);
			pclose(fp);
		}
	}  /* end of adding static routes */
}


net_host_info()
{
	int done;
	char value[80];

	done = 0;
	do {
		switch(update_menu())
		{
		case H_HOSTS:
			system(CLEAR);
			update_hosts();
			break;
		case H_HOSTSEQUIV:
			system(CLEAR);
			update_hostsequiv();
			break;
		case H_NETWORKS:
			system(CLEAR);
			update_networks();
			break;
		case H_EXIT:
			done = 1;
			break;
		default:
			printf("\nYour choice is invalid\n");
			prompt(value,10,"Press RETURN to continue ");
			break;
		}
	} while (!done);
	return;
}


update_menu()
{
	char choice[10];

	system(CLEAR);
	printf("\n\n\t\t\t**** HOST INFORMATION MENU ****\n");
	printf("\n\t\t1  /etc/hosts\n\n");
	printf("\t\t2  /etc/hosts.equiv\n\n");
	printf("\t\t3  /etc/networks\n\n");
	printf("\t\t4  Exit\n\n");
	prompt(choice, 2, "Enter the number for your choice: ");

	return(atoi(choice));
}


update_hosts()
{
	char buf[256];
	char cmd[256];
	char ipaddress[256];
	char machalias[256];
	char machname[256];
	char hostname[80];
	struct in_addr addr;
	FILE *fp;
	struct hostent *hp;
	int i, noreplace, mode;

	printf("\n***** ADD/DELETE HOSTS IN %s *****\n\n\
You can add or delete hosts in %s.  When finished, press\n\
the RETURN key at the prompt that asks whether you want to add\n\
or delete hosts.\n", HOSTS, HOSTS);

    for (;;) {
	if (prompt(buf, 128,
    		"\nEnter whether you want to \"(a)dd\" or \"(d)elete\" hosts in %s.\n\
If you are finished, press the RETURN key: ",HOSTS) == NULL) {
		break;
	}
	if (strncmp(buf,ADD_STR,strlen(buf)) == NULL) {
		if (yesno(YES, "You want to \"add\" to %s.  Is this correct", HOSTS))
			mode = ADD;
		else
			continue;
	} else if (strncmp(buf,DELETE_STR,strlen(buf)) == NULL) {
		if (yesno(YES, "You want to \"delete\" from %s.  Is this correct", HOSTS))
			mode = DELETE;
		else
			continue;
	}
	else {
		printf("\n\"%s\" is invalid\n",buf);
		continue;
	}
	if (mode == ADD) {
		printf("\n\
Enter the host name, aliases and Internet Protocol (IP) address for\n\
each host in the network.  Enter this information on separate lines\n\
when prompted.  This information is added to the %s file.\n", HOSTS);

		fp = edit(HOSTS);
		if (fp == NULL) {
			fprintf(stderr, "%s is not set up.\n", HOSTS);
			prompt(cmd,10,"Press RETURN to continue ");
			return;
		}
		for (;;) {
			noreplace = 0;
			if (prompt(machname, 128,
			    "\nEnter the name of the host to add to %s.\n\
If you are finished, press the RETURN key: ",HOSTS) == NULL)
			{
				break;
			}
			if (!yesno(YES, "You want to add host \"%s\".\nIs this correct", machname))
				continue;
			hp = gethostbyname(machname);
			if (hp != NULL) {
				printf("\n\"%s\" already exists with address %s\n",
				      machname, inet_ntoa(*(int *)hp->h_addr));
				if (!yesno(NO, "Do you want to replace it")) {
					noreplace = 1;
					continue;
				}
				/* want to replace, delete current entry */
				fprintf(fp, "g/%s[ 	]%s/d\n",
					inet_ntoa(*(int *)hp->h_addr), machname);
				fprintf(fp, "w\n");
				fflush(fp);
			}
			do {
				if (NULL == prompt(machalias, 128,
				    "\nEnter any aliases for \"%s\", separated by a space.\n\
If you do not want any aliases, press the RETURN key: ", machname))
				{
					i = yesno(YES,
						"\nYou do not want any aliases for \"%s\".\nIs this correct",
									machname);
				} else {
					i = yesno(YES,
					  "\nThe alias%s for \"%s\" %s %s.\nIs this correct",
							plural(machalias) ? "es":"",
							machname,
							plural(machalias) ? "are":"is",
							quote(machalias));
				}
			} while (i == NO);
			do {
				ipaddress[0] = '\0';
				prompt(ipaddress, 256, 
				"\nEnter the Internet Protocol (IP) address of \"%s\"\nin dot notation: ",
					machname);
				if (good_addr(ipaddress)) {
					fgrep(ipaddress,HOSTS,buf,sizeof(buf));
					if (buf[0] != '\0') {
						cmd[0] = '\0';
						sscanf(buf, "%s %s", cmd, hostname);
						if (strcmp(cmd, ipaddress) == 0) {
							printf("\n\"%s\" with address \"%s\" already\nexists in %s\n",
							      hostname,ipaddress,HOSTS);
							if (!yesno(NO, "Do you want to replace it"))
							{
								printf("\"%s\" with address \"%s\" is not added to %s\n",
									machname,ipaddress,HOSTS);
								noreplace = 1;
								break;
							}
							/* want to replace, delete current entry */
							fprintf(fp, "g/%s.*$/d\n",
								ipaddress);
							fprintf(fp, "w\n");
							fflush(fp);
						}
					}
					i = yesno(YES,
					"\nThe IP address for \"%s\" is \"%s\".\nIs this correct", 
						machname, ipaddress);
				} else {
					printf("\n\
The format of the Internet Protocol (IP) address is incorrect.\n\
The address must consist of four integers seperated by '.'\n\
Each integer must be less than or equal to 255.  This number was assigned\n\
to you by the NIC.  An example IP address is: 130.180.3.12\n\n");
					ipaddress[0] = '\0';
					i = NO;
				}
			} while (i == NO);

			if (noreplace)
				continue;	/* don't want to replace */

			addr.s_addr = inet_addr(ipaddress);

			if (yesno(YES,"\nThe %s entry looks like:\n\n\
\t%s %s %s\n\nIs this correct", HOSTS, inet_ntoa(addr), machname, machalias))
			{
				fprintf(fp, "$a\n");
				fprintf(fp, "%s %s %s\n", inet_ntoa(addr),
					machname, machalias);
				fprintf(fp, ".\n");
				fprintf(fp, "w\n");
				fflush(fp);
				printf("\n\"%s  %s  %s\" is added to %s\n",
					inet_ntoa(addr), machname, machalias, HOSTS);
			}
		}
		fprintf(fp, "q\n");
		fflush(fp);
		pclose(fp);
	} else {  /* deleting hosts */
		printf("\n\
Enter the host name for each host you want to delete when prompted.\n\
This information is deleted from the %s file.\n", HOSTS);

		fp = edit(HOSTS);
		if (fp == NULL) {
			fprintf(stderr, "%s is not set up.\n",
				HOSTS);
			prompt(cmd,10,"Press RETURN to continue ");
			return;
		}
		for (;;) {
			if (prompt(machname, 128,
			    "\nEnter the name of the host to delete from %s.\n\
If you are finished, press the RETURN key: ",HOSTS) == NULL)
			{
				break;
			}
			if (yesno(YES, "You want to delete host \"%s\".\nIs this correct", machname))
			{
				fgrep(machname,HOSTS,buf,sizeof(buf));
				if (buf[0] != '\0') {
					cmd[0] = '\0';
					hostname[0] = '\0';
					sscanf(buf, "%s %s",cmd,hostname);
					if (strncmp(machname,hostname,strlen(machname)) == 0)
					{
						fprintf(fp, "g/^.*[ 	]%s.*$/d\n", machname);
						fprintf(fp, "w\n");
						fflush(fp);
						printf("\n\"%s\" is deleted from %s\n",
							machname,HOSTS);
					} else {
						printf("\n\"%s\" is not in %s\n",
							machname, HOSTS);
						printf("\"%s\" is not deleted from %s\n",
							machname, HOSTS);
					}
				} else {
					printf("\n\"%s\" is not in %s\n",
						machname, HOSTS);
					printf("\"%s\" is not deleted from %s\n",
						machname, HOSTS);
				}
			}
		}
		fprintf(fp, "q\n");
		fflush(fp);
		pclose(fp);
	}  /* end else deleting hosts */
    }  /* end for adding/deleting hosts */
}


update_hostsequiv()
{
	char trustname[256];
	char trustuser[256];
	char buf[256];
	char cmd[256];
	struct in_addr addr;
	FILE *fp;
	struct hostent *hp;
	int i, mode;

	printf("\n***** ADD/DELETE HOSTS IN %s *****\n\n\
You can add or delete trusted hosts in %s.  When finished,\n\
press the RETURN key at the prompt that asks whether you want to add\n\
or delete trusted hosts.\n", HOSTS_EQUIV, HOSTS_EQUIV);

    for (;;) {
	if (prompt(buf, 128,
    		"\nEnter whether you want to \"(a)dd\" or \"(d)elete\" hosts in %s.\n\
If you are finished, press the RETURN key: ",HOSTS_EQUIV) == NULL) {
		break;
	}
	if (strncmp(buf,ADD_STR,strlen(buf)) == NULL) {
		if (yesno(YES, "You want to \"add\" to %s.  Is this correct", HOSTS_EQUIV))
			mode = ADD;
		else
			continue;
	} else if (strncmp(buf,DELETE_STR,strlen(buf)) == NULL) {
		if (yesno(YES, "You want to \"delete\" from %s.  Is this correct", HOSTS_EQUIV))
			mode = DELETE;
		else
			continue;
	}
	else {
		printf("\n\"%s\" is invalid\n",buf);
		continue;
	}

	if (mode == ADD) {
		/*
		 * Set up /etc/hosts.equiv with trusted hosts here.
		 */
		printf("\n\
Enter the names of trusted hosts.  Trusted hosts are systems you\n\
consider to be secure.  Be careful if you select trusted hosts.  Any\n\
users on a trusted host can log in to the system without password\n\
verification if they have a valid account on the system.  The names of\n\
the trusted hosts are stored in the %s file.\n", HOSTS_EQUIV);

		fp = edit(HOSTS_EQUIV);
		if (fp == NULL) {
			fprintf(stderr, "%s is not set up\n", HOSTS_EQUIV);
			prompt(cmd,10,"Press RETURN to continue ");
			return;
		}

		for (;;) {
			if (prompt(trustname, 128,
			    "\nEnter the name of the trusted host to add to %s.\n\
If you are finished, press the RETURN key: ",HOSTS_EQUIV) == NULL)
			{
				break;
			}
			if ((hp = gethostbyname(trustname)) == NULL) {
				printf("\n\"%s\" does not exist\n",
					trustname);
				printf("\"%s\" is not added to %s\n",
					trustname, HOSTS_EQUIV);
			} else {
				if (yesno(YES, "You want to add trusted host \"%s\".\nIs this correct", trustname))
				{
					printf("\n\
You may want to restrict access by trusted host \"%s\" to specific\n\
users.  Enter the name of a trusted user at the prompt.\n",
trustname);

					for (;;) {
					    prompt(trustuser,80,
					    "\nEnter the login name of a trusted user.\
\nIf you do not want a trusted user, press the RETURN key: ");
					    if (trustuser[0] == '\0') {
						if (yesno(YES,"You do not want to add a trusted user.  Is this correct"))
							break;
					    } else {
						if (yesno(YES,"The trusted user is \"%s\".\nIs this correct", trustuser))
							break;
					    }
					}
					fprintf(fp, "$a\n%s %s\n", trustname,
						trustuser);
					fprintf(fp, ".\n");
					fprintf(fp, "w\n");
					fflush(fp);
					if (trustuser[0] == '\0') {
						printf("\n\"%s\" is added to %s\n",
							trustname, HOSTS_EQUIV);
					} else {
						printf("\n\"%s %s\" is added to %s\n",
							trustname, trustuser, HOSTS_EQUIV);
					}
				}
			}
		}
		fprintf(fp, "q\n");
		fflush(fp);
		pclose(fp);
	} else {  /* deleting hosts */
		printf("\n\
Enter the host name for each host you want to delete when prompted.\n\
This information is deleted from the %s file.\n", HOSTS_EQUIV);

		fp = edit(HOSTS_EQUIV);
		if (fp == NULL) {
			fprintf(stderr, "%s is not set up.\n", HOSTS_EQUIV);
			prompt(cmd,10,"Press RETURN to continue ");
			return;
		}

		for (;;) {
			if (prompt(trustname, 128,
			    "\nEnter the name of the trusted host to delete from %s.\n\
If you are finished, press the RETURN key: ",HOSTS_EQUIV) == NULL)
			{
				break;
			}
			if (yesno(YES, "You want to delete trusted host \"%s\".\nIs this correct", trustname))
			{
				fgrep(trustname,HOSTS_EQUIV,buf,sizeof(buf));
				if (buf[0] != '\0') {
					cmd[0] = '\0';
					sscanf(buf, "%s", cmd);
					if (strcmp(cmd, trustname) == 0) {
						fprintf(fp, "g/^%s.*$/d\n", trustname);
						fprintf(fp, "w\n");
						fflush(fp);
						printf("\n\"%s\" is deleted from %s\n",
							trustname, HOSTS_EQUIV);
					} else {
						printf("\n\"%s\" is not in %s\n",
							trustname, HOSTS_EQUIV);
						printf("\"%s\" is not deleted from %s\n",
							trustname, HOSTS_EQUIV);
					}
				} else {
					printf("\n\"%s\" is not in %s\n",
						trustname, HOSTS_EQUIV);
					printf("\"%s\" is not deleted from %s\n",
						trustname, HOSTS_EQUIV);
				}
			}
		}
		fprintf(fp, "q\n");
		fflush(fp);
		pclose(fp);
	}  /* end else deleting hosts */
    }
}


update_networks()
{
	char cmd[256];
	char netname[256];
	char netalias[256];  
	char netnum[256];  
	char netno[80];  
	struct in_addr addr;
	char buf[512];
	FILE *fp;
	struct netent *np;
	int i, mode, noreplace;

	printf("\n***** ADD/DELETE NETWORKS IN %s *****\n\n\
You can add or delete networks in %s.  When finished, press\n\
the RETURN key at the prompt that asks whether you want to add\n\
or delete networks.\n", NETWORKS, NETWORKS);

    for (;;) {
	if (prompt(buf, 128,
    		"\nEnter whether you want to \"(a)dd\" or \"(d)elete\" networks in %s.\n\
If you are finished, press the RETURN key: ",NETWORKS) == NULL) {
		break;
	}
	if (strncmp(buf,ADD_STR,strlen(buf)) == NULL) {
		if (yesno(YES, "You want to \"add\" to %s.  Is this correct", NETWORKS))
			mode = ADD;
		else
			continue;
	} else if (strncmp(buf,DELETE_STR,strlen(buf)) == NULL) {
		if (yesno(YES, "You want to \"delete\" from %s.  Is this correct", NETWORKS))
			mode = DELETE;
		else
			continue;
	}
	else {
		printf("\n\"%s\" is invalid\n",buf);
		continue;
	}
	if (mode == ADD) {
		printf("\n\
Enter the network name, aliases and network number for the\n\
network.  Enter this information on separate lines when prompted.\n\
This information is added to the %s file.\n", NETWORKS);

		fp = edit(NETWORKS);
		if (fp == NULL) {
			fprintf(stderr, "%s is not set up.\n", NETWORKS);
			prompt(cmd,10,"Press RETURN to continue ");
			return;
		}
		for (;;) {
			noreplace = 0;
			if (prompt(netname, 128,
			    "\nEnter the name of the network to add to %s.\n\
If you are finished, press the RETURN key: ",NETWORKS) == NULL)
			{
				break;
			}
			if (!yesno(YES, "You want to add network \"%s\".\nIs this correct", netname))
				continue;
			fgrep(netname,NETWORKS,buf,sizeof(buf));
			if (buf[0] != '\0') {
				cmd[0] = '\0';
				sscanf(buf, "%s %s", cmd, netno);
				if (strcmp(cmd, netname) == 0) {
					printf("\n\"%s\" already exists with network number %s\n",
						netname, netno);
					if (!yesno(NO, "\nDo you want to replace it")) {
						noreplace = 1;
						continue;
					}
					/* want to replace, delete current entry
					 */
					fprintf(fp, "g/%s/d\n", netname);
					fprintf(fp, "w\n");
					fflush(fp);
				}
			} else {
				np = getnetbyname(netname);
				if (np) {
					if (np->n_net < 128)
						addr.s_addr = np->n_net << IN_CLASSA_NSHIFT;
					else if (np->n_net < 65536)
						addr.s_addr = np->n_net << IN_CLASSB_NSHIFT;
					else if (np->n_net < 16777216L)
						addr.s_addr = np->n_net << IN_CLASSC_NSHIFT;
					else
						addr.s_addr = np->n_net;
					addr.s_addr = htonl(addr);

					printf("\n\"%s\" already exists with network number %s\n",
						netname, inet_ntoa(addr));
					if (!yesno(NO, "\nDo you want to replace it")) {
						noreplace = 1;
						continue;
					}
				}
			}
			do {
				if (NULL == prompt(netalias, 128,
				    "\nEnter any aliases for \"%s\", separated \
by a space.\nIf you do not want any aliases, \
press the RETURN key: ", netname))
				{
					i = yesno(YES,
						"You do not want any aliases for \"%s\".\nIs this correct",
									netname);
				} else {
					i = yesno(YES,
					  "The alias%s for \"%s\" %s %s.\nIs this correct",
							plural(netalias) ? "es":"",
							netname,
							plural(netalias) ? "are":"is",
							quote(netalias));
				}
			} while (i == NO);
			do {
				printf("\n\
For a Class A network, the network number is in the range 1 through\n\
126.  For a Class B network, the network number consists of two fields\n\
separated by periods.  The first field is in the range 128 through 191,\n\
and the second field is in the range 1 through 254.  For a Class C\n\
network, the network number consists of three fields separated by\n\
periods.  The first field is in the range 192 through 223, the second\n\
field is in the range 0 through 255, and the third field is in the\n\
range 1 through 254:\n\n\
   Class A:  1       through 126\n\
   Class B:  128.1   through 191.254\n\
   Class C:  192.0.1 through 223.255.254\n");
				netnum[0] = '\0';
				prompt(netnum, 256, 
				"\nEnter the network number of \"%s\": ",
					netname);
				if (good_netnum(netnum)) {
					fgrep(netnum,NETWORKS,buf,sizeof(buf));
					if (buf[0] != '\0') {
						cmd[0] = '\0';
						sscanf(buf, "%s %s", cmd, netno);
						if (strcmp(netnum, netno) == 0) {
							printf("\n\"%s\" with number \"%s\" already exists in %s\n",
							      cmd,netno,NETWORKS);
							if (!yesno(NO, "\nDo you want to replace it"))
							{
								printf("\n\"%s\" with network number \"%s\" is not added to %s\n",
									netname,netnum,NETWORKS);
								noreplace = 1;
								break;
							}
							fprintf(fp, "g/^.*[ 	]%s.*$/d\n",
								netnum);
							fprintf(fp, "w\n");
							fflush(fp);
						}
					}
					i = yesno(YES,
					"The network number for \"%s\" is \"%s\".\nIs this correct", 
						netname, netnum);
				} else {
					printf("\nThe format of the network number is incorrect.\n");
					i = NO;
				}
			} while (i == NO);

			if (noreplace)
				continue;	/* don't want to replace */

			if (yesno(YES,"\nThe %s entry looks like:\n\n\
\t%s %s %s\n\nIs this correct", NETWORKS, netname, netnum, netalias))
			{
				fprintf(fp, "$a\n");
				fprintf(fp, "%s %s %s\n", netname,
					netnum, netalias);
				fprintf(fp, ".\n");
				fprintf(fp, "w\n");
				fflush(fp);
				printf("\n\"%s  %s  %s\" is added to %s\n",
					netname, netnum, netalias, NETWORKS);
			}
		}
		fprintf(fp, "q\n");
		fflush(fp);
		pclose(fp);
	} else {  /* deleting networks */
		printf("\n\
Enter the name for each network you want to delete when prompted.\n\
This information is deleted from the %s file.\n", NETWORKS);

		fp = edit(NETWORKS);
		if (fp == NULL) {
			fprintf(stderr, "%s is not set up.\n",
				NETWORKS);
			prompt(cmd,10,"Press RETURN to continue ");
			return;
		}
		for (;;) {
			if (prompt(netname, 128,
			    "\nEnter the name of the network to delete from %s.\n\
If you are finished, press the RETURN key: ",NETWORKS) == NULL)
			{
				break;
			}
			if (yesno(YES, "\nYou want to delete network \"%s\".\nIs this correct", netname))
			{
				fgrep(netname,NETWORKS,buf,sizeof(buf));
				if (buf[0] != '\0') {
					cmd[0] = '\0';
					sscanf(buf, "%s",cmd);
					if (strcmp(cmd,netname) == 0) {
						fprintf(fp, "g/^%s.*$/d\n", netname);
						fprintf(fp, "w\n");
						fflush(fp);
						printf("\n\"%s\" is deleted from %s\n",
							netname,NETWORKS);
					} else {
						printf("\n\"%s\" is not in %s\n",
							netname, NETWORKS);
						printf("\"%s\" is not deleted from %s\n",
							netname, NETWORKS);
					}
				} else {
					printf("\n\"%s\" is not in %s\n",
						netname, NETWORKS);
					printf("\"%s\" is not deleted from %s\n",
						netname, NETWORKS);
				}
			}
		}
		fprintf(fp, "q\n");
		fflush(fp);
		pclose(fp);
	}  /* end else deleting hosts */
    }  /* end for adding/deleting hosts */
}


net_display()
{
	int i, devsfound, num_config;
	FILE *fp;
	if_names_blk if_buf, *ifnames_buf;
	char cmd[256];
	char hostname[80];
	char num_netconfig[80];
	char value[80];
	char device[MAX_NETDEVS][80];
	char ifconfig[MAX_NETDEVS][80];
	char sliptty[MAX_NETDEVS][80];
	char rwhod[80];
	char routed[80];
	char routed_flags[80];
	char gated[80];
	char gated_flags[80];
	char router[80];
	char max_netdevs[80];

	printf("\n***** DISPLAY NETWORK CONFIGURATION *****\n\n\
Obtaining hardware and software configuration.  Please wait...\n\n");
	find_network_adapters(&if_buf);

	ifnames_buf = &if_buf;
	if (ifnames_buf->curr_cnt <= 0)
		printf("Cannot find any network adapters on this machine\n\n");

	hostname[0] = '\0';
	sprintf(cmd, "/usr/sbin/rcmgr get HOSTNAME");
	GET_VALUE(cmd, hostname);
	num_netconfig[0] = '\0';
	sprintf(cmd, "/usr/sbin/rcmgr get NUM_NETCONFIG");
	GET_VALUE(cmd, num_netconfig);
	num_config = atoi(num_netconfig);
	bzero(device, sizeof(device));
	devsfound = 0;
	for (i = 0; i < MAX_NETDEVS; i++) {
		value[0] = '\0';
		sprintf(cmd, "/usr/sbin/rcmgr get NETDEV_%d", i);
		GET_VALUE(cmd, value);
		strcpy(&(device[i][0]),value);
		value[0] = '\0';
		sprintf(cmd, "/usr/sbin/rcmgr get IFCONFIG_%d", i);
		GET_VALUE(cmd, value);
		strcpy(&(ifconfig[i][0]),value);
		if (strncmp(&(device[i][0]),"sl",2) == NULL) {
			/* SLIP device - get slattach parameters */
			value[0] = '\0';
			sprintf(cmd, "/usr/sbin/rcmgr get SLIPTTY_%d", i);
			GET_VALUE(cmd, value);
			strcpy(&(sliptty[i][0]),value);
		}
		if (device[i][0] != '\0') {
			devsfound++;
			if (devsfound == num_config)
				break;
		}
	}
	rwhod[0] = '\0';
	sprintf(cmd, "/usr/sbin/rcmgr get RWHOD");
	GET_VALUE(cmd, rwhod);
	routed[0] = '\0';
	sprintf(cmd, "/usr/sbin/rcmgr get ROUTED");
	GET_VALUE(cmd, routed);
	routed_flags[0] = '\0';
	sprintf(cmd, "/usr/sbin/rcmgr get ROUTED_FLAGS");
	GET_VALUE(cmd, routed_flags);
	gated[0] = '\0';
	sprintf(cmd, "/usr/sbin/rcmgr get GATED");
	GET_VALUE(cmd, gated);
	gated_flags[0] = '\0';
	sprintf(cmd, "/usr/sbin/rcmgr get GATED_FLAGS");
	GET_VALUE(cmd, gated_flags);
	router[0] = '\0';
	sprintf(cmd, "/usr/sbin/rcmgr get ROUTER");
	GET_VALUE(cmd, router);
	max_netdevs[0] = '\0';
	sprintf(cmd, "/usr/sbin/rcmgr get MAX_NETDEVS");
	GET_VALUE(cmd, max_netdevs);

	if (ifnames_buf->curr_cnt > 0) {
		printf("Current network adapters on this system:\n\n");
		for (i = 0; i < ifnames_buf->curr_cnt; i++) {
			printf("         %s%d\n", 
				ifnames_buf->info[i].if_name,
				ifnames_buf->info[i].if_unit);
		}
	}

	printf("\nCurrent software configuration in /etc/rc.config:\n\n");
	printf("HOSTNAME = %s\n",hostname);
	printf("NUM_NETCONFIG = %s\n",num_netconfig);
	devsfound = 0;
	for (i = 0; i < MAX_NETDEVS; i++) {
		if ((device[i][0]) != '\0') {
			printf("NETDEV_%d = %s\n",i,&(device[i][0]));
			devsfound++;
		}
		if ((ifconfig[i][0]) != '\0')
			printf("IFCONFIG_%d = %s\n",i,&(ifconfig[i][0]));
		if (strncmp(&(device[i][0]),"sl",2) == NULL) {
			/* SLIP device - print slattach parameters */
			printf("SLIPTTY_%d = %s\n",i,&(sliptty[i][0]));
		}
		if (devsfound == num_config)
			break;
	}
	printf("RWHOD = %s\n",rwhod);
	printf("ROUTED = %s\n",routed);
	printf("ROUTED_FLAGS = %s\n",routed_flags);
	printf("GATED = %s\n",gated);
	printf("GATED_FLAGS = %s\n",gated_flags);
	printf("ROUTER = %s\n",router);
	printf("MAX_NETDEVS = %s\n",max_netdevs);
	prompt(value,10,"\nPress RETURN to continue ");
}


/*
 * exit the program with status
 */
leave(status)
int status;
{
	if (status == 0)
		printf("\n\n***** NETWORK SETUP COMPLETE *****\n\n");
	else
		printf("\n\n***** NETWORK SETUP FAILED *****\n\n");
	exit(status);
}

char *
prompt(buf, len, str, a1, a2, a3, a4, a5)
char *str, *buf;
int len;
long a1, a2, a3, a4, a5;
{
	char mybuf[512];

	printf(str, a1, a2, a3, a4, a5);
	fflush(stdout);
	if (gets(mybuf) == NULL) {
		eof();
		return(NULL);
	}
	strncpy(buf, mybuf, len);
	buf[len-1] = '\0';
	if (buf[0] == '\0')
		return(NULL);
	return(buf);
}

yesno(t, s, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11)
char *s;
int t;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11;
{
	char buf[512];

	for (;;) {
		printf(s, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
		switch(t) {
		case 0:
			printf(" [no]? ");
			break;
		case 1:
			printf(" [yes]? ");
			break;
		default:
			printf(" (yes or no)? ");
			break;
		}
		fflush(stdout);
		buf[0] = '\0';
		if (gets(buf) == NULL) {
			eof();
			return(t);
		}
		if (buf[0] == 'y' || buf[0] == 'Y')
			return(1);
		if (buf[0] == 'n' || buf[0] == 'N')
			return(0);
		if ((t != 2) && (buf[0] == '\0' || buf[0] == '\n'))
			return(t);
	}
	/* To supress the silly warning about statement not reached, */
	/* we comment out the following:  */
	/* return(0); */
}


good_netmask(netmask, class)
char *netmask;
int class;
{
	struct in_addr mask_addr;

	if (good_addr(netmask) == FALSE)
		return(FALSE);
	if ((mask_addr.s_addr = ntohl(inet_addr(netmask))) == -1L) 
		return(FALSE);
	if (class == CLASSA) { 
		if ((mask_addr.s_addr & IN_CLASSA_NET) != IN_CLASSA_NET)
			return(FALSE);
	} else if (class == CLASSB) {
		if ((mask_addr.s_addr & IN_CLASSB_NET) != IN_CLASSB_NET)
 			return(FALSE);
	} else if (class == CLASSC) {
		if ((mask_addr.s_addr & IN_CLASSC_NET) != IN_CLASSC_NET)
			return(FALSE);
	} else
		return(FALSE);

	return(TRUE);
}


good_netnum(netnum)
char *netnum;
{
	int num[4], n;

	for (n = 0; n < 4; n++)
		num[n] = 0;

	n = sscanf(netnum, "%d.%d.%d.%d", &num[0], &num[1], &num[2], &num[3]);

	/* invalid network number */
	if ( (n == 0) || (num[0] == 0) || (num[0] > 223) )
		return(FALSE);

	/* class A network number */
	if ( (num[0] >= 1) && (num[0] <= 126) )
		return(TRUE);

	/* class B network number */
	if ( (num[0] >= 128) && (num[0] <= 191) )
		if ( (num[1] >= 1) && (num[1] <= 254) )
			return(TRUE);

	/* class C network number */
	if ( (num[0] >= 192) && (num[0] <= 223) )
		if ( (num[1] >= 0) && (num[1] <= 255) )
			if ( (num[2] >= 1) && (num[2] <= 254) )
				return(TRUE);

	/* invalid network number */
	return(FALSE);
}


good_addr(ipaddress)
char *ipaddress;
{
	int num[4], n;

	n = sscanf(ipaddress, "%d.%d.%d.%d", &num[0], &num[1], &num[2], &num[3]);
	if (n != 4)
		return(FALSE);
	for (n = 0; n < 4; n++) {
		if (num[n] > 255)
		return(FALSE);
	}
	return(TRUE);
}


good_speed(speed)
char *speed;
{
	int num, n;

	n = sscanf(speed, "%d", &num);
	if (n != 1)
		return(FALSE);
	if (num != 4 && num != 16)
		return (FALSE);

	return(TRUE);
}


FILE*
edit(file)
char *file;
{
	FILE	*fp;
	char	buf[MAXPATHLEN];

	/*
	 * close stdout, popen the file for editing, restore
	 * stdout, and return the stream pointer.
	 */
	close(1);
	sprintf(buf, "/bin/ed - %s", file);
	fp = popen(buf, "w");
	dup2(3, 1);
	if (fp == NULL)
		fprintf(stderr, "Cannot open %s for editing.\n", file);
	return(fp);
}


/* 
 *		e o f
 *
 * Unexpected EOF, try to recover.
 */
eof()
{
	if (isatty(0) && (freopen((const char *)ttyname(0),"r",stdin) != NULL)) {
		printf("\n");
		return(0);
	}
	fprintf(stderr,"\nUnexpected EOF.  Exiting.\n");
	leave(1);
}




/* quote() -  make string like: abc def ghi, become: "abc", "def", "ghi". */

char *
quote(s) 
char *s;
{
	int state, oldstate, white;
	static char buff[256];
	char *s2 = buff;

	state = oldstate = white = iswhite(' ');  /* prime it */
	while (*s) {
		if ((state = iswhite(*s)) != oldstate) {
			*s2++ = '\"';
			if (state == white)
				*s2++ = ',';
		}
		*s2++ = *s++;
		oldstate = state;
	}

	/* if last char was not a space put ending quote*/
	if (state == 0)       
		*s2++ = '\"';
	*s2 = '\0';

	/* strip off last comma */
	while (--s2 > buff) 	
		if (!iswhite(*s2)) {
			if (*s2 == ',')
				*s2 = ' ';
			break;
		}
	return(buff);
}


fgrep(str,file,buf,size)
char *str, *file, *buf;
int size;
{
	char cmd[80];
	FILE *fp;
	int len;
	struct stat statbuf;

	*buf = '\0';
	if (str[0] == '\0')
		return;
	if (stat(file, &statbuf) == 0) {
		sprintf(cmd, "/usr/bin/fgrep \"%s\" %s", str, file);
		fp = popen(cmd, "r"); 
		if (fp) {
			while (fgets(buf, size, fp)) {
				len = strlen(buf);
				if (buf[len-1] == '\n') 
					buf[len-1] = '\0';  
				if (buf[0] == '#')
					continue;
			}
			pclose(fp);
		} else
			printf("Cannot open %s\n",file);
	}
	return;
}
