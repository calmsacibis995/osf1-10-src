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
static char *rcsid = "@(#)$RCSfile: gethost.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/10/12 16:34:04 $";
#endif


/************************************************************************
 *			Modification History				*
 *									*
 * 29-Jun-90	Fred L. Templin						*
 *	Added "getsysinfo()" call on the MIPS side to retrieve name	*
 *	of boot interface						*
 *									*
 * 03-Feb-89	R. Bhanukitsiri						*
 *	Reflect V3.2 source pool changes.  Merge in VAX code from V3.0.	*
 *									*
 ************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <sys/file.h>
#include <machine/cpuconf.h>
#include <dec/sas/mop.h>
#ifdef mips
#include <sys/sysinfo.h>
#include <arch/mips/hal/hal_sysinfo.h>
#endif /* mips */
#ifdef __alpha
#include <sys/sysinfo.h>
#include <arch/alpha/hal/hal_sysinfo.h>
#endif /* __alpha */
main()
{
	char	srvip[40];
	char	cliip[40];
	char	netmask[40];
	char	brdcast[40];
	char	syscall[60];
	char	netdev[10];
	int	offset;
	int	kernel;
	int 	i;
	FILE	*hosts;
	struct	netblk netblk;
	struct  netblk *netblk_ptr;
	/******* get rpb struct from kernel space **********************/
	getsysinfo(GSI_NETBLK, (char *)&netblk, sizeof (struct netblk));
	netblk_ptr = (struct netblk *)&netblk;
{
	int i = getsysinfo(GSI_BOOTTYPE, netdev, 10);
}
	/******** extract server and client ip addresses ***************/
	exip(netblk_ptr->srvipadr, srvip);
	exip(netblk_ptr->cliipadr, cliip);
	exip(netblk_ptr->netmsk, netmask);
	/******** create /etc/hosts file *******************************/
	/******** 	entry 1 = client *******************************/
	/********       entry 2 = server *******************************/
	hosts=fopen("/etc/hosts","w");
	if(hosts==NULL)
		exit(2);
	fprintf(hosts,"127.0.0.1 localhost\n");
	fprintf(hosts,"%s %s\n",cliip,netblk_ptr->cliname);
	fprintf(hosts,"%s %s\n",srvip,netblk_ptr->srvname);
	fclose(hosts);
	/******** create netstart file *********************************/
	/******** 	sets hostname and runs ifconfig (like rc.local)*/
	hosts=fopen("/netstart","w");
	if(hosts==NULL)
		exit(2);
	fprintf(hosts,"hostname %s\n",netblk_ptr->cliname);
	fprintf(hosts,"ifconfig %s `hostname` netmask %s\n",netdev,netmask);
	fprintf(hosts,"ifconfig lo0 localhost\n");
	fclose(hosts);
	/******** output SERVER and CLIENT assignments for potential eval ***/
	fprintf(stdout,"SERVER=%s\n",netblk_ptr->srvname);
	fprintf(stdout,"CLIENT=%s\n",netblk_ptr->cliname);
	fprintf(stdout,"SERVERIP=%s\n",srvip);
	fprintf(stdout,"CLIENTIP=%s\n",cliip);
	/******** execute netstart ******************************************/
	chmod("/netstart",0500);
	system("2>&1 /netstart > /dev/null");
	exit(0);
}
/************************************************************************/
/*									*/
/*	exip - extract ip string from a long				*/
/************************************************************************/
exip(ipaddr,str)
	unsigned long ipaddr;
	char *str;
{
int i=0,j=0,flg=0;
int numb[4];
numb[0]= ((ipaddr >> 24) & 0377);
numb[1]= ((ipaddr >> 16) & 0377);
numb[2]= ((ipaddr >> 8) & 0377);
numb[3]= (ipaddr & 0377);
sprintf(str,"%d.%d.%d.%d",numb[0],numb[1],numb[2],numb[3]);
return(0);
}
