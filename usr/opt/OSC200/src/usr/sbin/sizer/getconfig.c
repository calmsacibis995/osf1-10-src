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
static char     *sccsid = "@(#)$RCSfile: getconfig.c,v $ $Revision: 4.4.25.8 $ (DEC) $Date: 1993/12/21 20:53:34 $";
#endif lint

/************************************************************************
 *
 * Name: getconfig.c
 *
 * Modification History
 *
 * Jan 21, 1992 - fred
 *	Add DS5100_MDC_HACK so sizer feeds mdc0 to MAKEDEV instead of
 *	dc0 on a DS5100.
 *
 * Oct 26, 1991 - fred
 *	Add "options OSF". Removed "options ULT_BIN_ISATTY_FIX".
 *	Comment out "makeoptions ASOPTS=-w" (enable compiler warnings).
 *	Removed "#options STREAMS_DEBUG" and "options UNIX_UNI".
 *	Added comments about options. Removed "options RT_SCHED_MON".
 *	Make maxdsiz, dfldsiz, maxssiz, and dflssiz configurable.
 *	Remove "pseudo-device memd".
 *
 * Jun 5, 1991 - jaa
 *	ported to OSF/1.  modified to use new bus/controller/device
 *	kernel structures.  in porting, we don't use /dev/kmem anymore.
 *	getsysinfo(3) was modified to return the needed information.
 *	NOTE: ifdef TODO's in this code will have to be done as the 
 *	pieces are added (i.e. floating csr space, workstation specifics)
 *
 * Nov 28, 1990 - Darrell Dunnuck
 *	Changed sizer so that it will not add a line containing
 *	"pseudo-device scsnet" if /tmp/.config exists and
 *	contains any pseudo-devices.  This is to keep from getting
 *	two "pseudo-device scsnet" lines in the config file.
 *
 * Oct 15, 1990 - Paul Grist
 *      Added additional support for Prestoserve at installation to
 *      make pr0 at istallation time if nvram is present.
 *
 * Sept 4, 1990 - Vince Wallace
 *	Added option & pseudo-device netman to the default config file.
 * 
 * Aug 10, 1990 - Randall Brown
 *	Added support for xcons and ws pseudo-devices. Only used on mips side
 *
 * May 21, 1990 - Robin
 *	Added presto NVRAM suport.
 *
 * Dec 12, 1989 - Alan Frechette
 *	For all SCSI controllers allow full blown configuration of all
 *	possible scsi devices. Also only make the MAKEDEV entries for
 *	the scsi devices which are alive.
 *
 * Nov 3, 1989 - Alan Frechette
 *	Added config file lines "maxdsiz 64" and "smmax 1024" for 3MAX.
 *
 * Oct 11, 1989 - Alan Frechette
 *	Fixed sizer to no longer get the CSR for a UNIBUS controller
 *	from a UNIBUS device structure. Allow DECNET option for VAX
 *	as well as for MIPS.
 *
 * July 14, 1989 - Alan Frechette
 *	Added (XMINODE) check and support for RIGEL (VAX_6400).
 *
 * July 10, 1989 - Alan Frechette
 *	Make sure we always check for SMP option all the time.
 *
 * July 6, 1989 - Alan Frechette
 *	Added check for a VAXSTAR cputype.
 *
 * May 10, 1989 - Alan Frechette
 *	Added option SMP.
 *
 * May 02, 1989 - Alan Frechette
 *	Changes to deal with new unique "cpu" handling for both
 *	vax and mips architectures.
 *
 * Feb 12, 1989 - Alan Frechette
 *	New sizer code which supports multiple architectures.
 *      This file is a complete redesign and rewrite of the 
 *	original V3.0 sizer code by Tungning Cherng.
 *
 ***********************************************************************/
#define	ULT_BIN_COMPAT	1
#include "sizer.h"
#include <string.h>	/* need this for strtok() --bg */

/****************************************************************
* Name:		getconfig					*
*    								*
* Abstract:	Create the configuration file for the system.	*
* 		Create the makedevices file for the system.	*
*								*
* Inputs:	None.						*
*								*
* Outputs:	None.						*
*								*
* Return Values: None.						*
*								*
* Side Effects:	The configuration file and the makedevices file	*
*		are  created in  the /tmp directory under the	*
*		following pathnames:				*
*		1) Configuration File is under /tmp/sysname.	*
*		2) Makedevices File is under /tmp/sysname.devs.	*
*								*
*		Many other routines are called to handle and to	*
*		get the configuration file information.		*
****************************************************************/

/*
 * 1/21/92 -- Fred Canter (I'll burn for this one!)
 *
 * The DS5100 uses dc0 in the config file for the tty lines,
 * but needs mdc0 in MAKEDEV (dc0 = major 21, mdc0 = major 0).
 */
#define	DS5100_MDC_HACK

#ifdef	DS5100_MDC_HACK
int	cpu_index;
#endif


getconfig()
{
	struct	rlimit rlp;
	int index;
	char devpath[PATHSIZE], path[PATHSIZE];
	char boot[80];
	extern char *Tzone;
	extern char *Sysname;
#ifdef	__alpha
	int physmem = 0;
#endif
	Scs_sysidflag = 0;
#ifdef TODO_FLOAT
	Float_flag = 0;
#endif /* TODO_FLOAT */
	tc_callout = 0;
	eisa_callout = 0;

	/* Check for a valid configuration file name */
	checksysname();

	/* Create the makedevices file */
	sprintf(devpath, "/tmp/%s.devs", Sysname);
   	if((Fpdevs = fopen(devpath, "w")) == NULL) {
		fprintf(stderr, "Cannot open (%s).\n", devpath);
		quitonerror(-10);
	}

	/* Create the configuration file */
	sprintf(path, "/tmp/%s", Sysname);
	if((Fp = fopen(path, "w")) == NULL) {
		fprintf(stderr, "Cannot open (%s).\n",path);
		quitonerror(-11);
	}

	fprintf(Fpdevs, "MAKEDEV  ");

	fprintf(Fp, "ident\t\t\"%s\"\n\n", Sysname);

	/* Get any "options" from the /tmp/.config file */
	if(!getconfig_string("options")) 
		print_options();

	/* Get any "makeoptions" from the /tmp/.config file */
	if(!getconfig_string("makeoptions")) 
		print_makeoptions();

	fprintf(Fp, "#\n# Special options (see configuring the kernel chapter");
	fprintf(Fp, "\n# in the Guide to System Administration)\n#\n");
	/* Get the "timezone" information */
	if(Tzone != NULL && strlen(Tzone))
		fprintf(Fp, "timezone\t%s\n", Tzone);
	else {
		struct timeval tval;
		struct timezone tzb;

		gettimeofday(&tval, &tzb);
		fprintf(Fp, "timezone\t%d dst %d\n", 
			tzb.tz_minuteswest / 60, tzb.tz_dsttime);
	}

	getmaxcpu();
	fprintf(Fp, "processors\t%d\n", Maxcpu);

	if(getrlimit(RLIMIT_DATA, &rlp) == -1) {
		rlp.rlim_cur = (unsigned long)(32*1024*1024);
		rlp.rlim_max = (unsigned long)(128*1024*1024);
	}
	fprintf(Fp, "dfldsiz\t\t%ld\n", rlp.rlim_cur);
	fprintf(Fp, "maxdsiz\t\t%ld\n", rlp.rlim_max);

	if(getrlimit(RLIMIT_STACK, &rlp) == -1) {
		rlp.rlim_cur = (unsigned long)(2*1024*1024);
		rlp.rlim_max = (unsigned long)(32*1024*1024);
	}
	fprintf(Fp, "dflssiz\t\t%ld\n", rlp.rlim_cur);
	fprintf(Fp, "maxssiz\t\t%ld\n", rlp.rlim_max);

	index = getcpu(NODISPLAY);
#ifdef	DS5100_MDC_HACK
	
	cpu_index = index;	/* save cputbl index for later use */
#endif
#ifdef __alpha
	/* Reduce maxusers on systems with <= 32MB of memory. */
	if (getsysinfo(GSI_PHYSMEM, &physmem, sizeof (physmem)) == 1) {
	    /* physmem returned in Kbytes */
	    if ((physmem <= (16*1024)) && (cputbl[index].maxusers > 8))
		fputs("maxusers\t8\n", Fp);
	    else if ((physmem <= (32*1024)) && (cputbl[index].maxusers > 16))
		fputs("maxusers\t16\n", Fp);
	    else
		fprintf(Fp, "maxusers\t%d\n", cputbl[index].maxusers);
	} else
#endif	/* __alpha */
	    fprintf(Fp, "maxusers\t%d\n", cputbl[index].maxusers);

/*
 * A quick fix for mips pool; should make a getsysinfo call
 * to get the system's utsname.machine value and put it
 * here, so it works arcross multiple architectures
 */
#ifdef mips
	fprintf(Fp, "machine\t\tmips\n\n");
#endif
#ifdef __alpha
	fprintf(Fp, "machine\t\talpha\n\n");
#endif
#ifdef vax
	fprintf(Fp, "machine\t\tvax\n\n");
#endif

	if(Maxcpu > 1)
		fprintf(Fp, "options\t\tSMP\n");

	getroot(NODISPLAY);
	getswap(NODISPLAY);
	fprintf(Fp, "\n\n");

	getdevices();

	fprintf(Fpdevs, "%s  ", "cam");
	if (cputbl[index].cputype == DEC_4000) 
		fprintf(Fpdevs, "%s  ", "cobtty");
	
	fprintf(Fp, "\n");

	fprintf(Fp, "\nscs_sysid\t%d\n", Scs_sysidflag ? getsysid() : 1);

	/* Get any "hardware" from the /tmp/.config file */
	getconfig_string("hardware");

	/* Get any "pseudo-devices" from the /tmp/.config file */
	if(!getconfig_string("pseudo-device"))
		print_pseudo_device();
	/* Handle configuration dependent pseudo devices */
	print_opt_pseudo_device();

	fprintf(Fpdevs, "\n");
	fclose(Fpdevs);
	fclose(Fp);
	fprintf(stdout, "\nConfiguration file complete.\n");
}

print_makeoptions()
{
	fprintf(Fp, "#\n# Makeoptions (DO NOT CHANGE)\n#\n\n");
	fprintf(Fp, "#makeoptions\tASOPTS=\"-w\"\n");
	fprintf(Fp, "makeoptions\tCDEBUGOPTS=\"-g3\"\n");
	fprintf(Fp, "makeoptions\tPROFOPTS=\"-DPROFILING -DPROFTYPE=4\"\n");
	fprintf(Fp, "\n");
}

print_options()
{
	fprintf(Fp, "#\n# Dependency options (DO NOT CHANGE)\n#\n");
	fprintf(Fp, "options\t\tGENERIC\n");
#ifdef __alpha
	fprintf(Fp, "#");
#endif
	fprintf(Fp, "options\t\tUERF\n");
	fprintf(Fp, "options\t\tMACH\n");
	fprintf(Fp, "options\t\tOSF\n");
#ifdef __alpha
	fprintf(Fp, "#");
#endif
	fprintf(Fp, "options\t\tULT_BIN_COMPAT\n");
	fprintf(Fp, "options\t\t_LMF_\n");
	fprintf(Fp, "options\t\tBIN_COMPAT\n");
/*	fprintf(Fp, "options\t\tMACH_CO_INFO\n");	*/
/*	fprintf(Fp, "options\t\tMACH_DEVICE\n");	*/
/*	fprintf(Fp, "options\t\tMACH_EMULATION\n");	*/
/*	fprintf(Fp, "options\t\tMACH_HOST\n"); /* processor sets for MP */
/*	fprintf(Fp, "options\t\tMACH_IPC_STATS\n");	*/
	fprintf(Fp, "options\t\tMACH_IPC_TCACHE\n");
	fprintf(Fp, "options\t\tMACH_IPC_WWA\n");
	fprintf(Fp, "options\t\tMACH_IPC_XXXHACK\n");
/*	fprintf(Fp, "options\t\tMACH_NET\n");		*/
/*	fprintf(Fp, "options\t\tMACH_PAGEMAP\n");	*/
/*	fprintf(Fp, "options\t\tMACH_SCTIMES\n");	*/
/*	fprintf(Fp, "options\t\tMACH_XP\n");		*/
/*	fprintf(Fp, "options\t\tMACH_XP_FPD\n");	*/
/*	fprintf(Fp, "options\t\tMACH_XP_SC\n");		*/
	fprintf(Fp, "options\t\tCOMPAT_43\n");
#ifdef __alpha
	fprintf(Fp, "#");
#endif
	fprintf(Fp, "options\t\tOSF_MACH_O\n");
	fprintf(Fp, "options\t\tBUFCACHE_STATS\n");
	fprintf(Fp, "options\t\tINOCACHE_STATS\n");
	fprintf(Fp, "options\t\tSTAT_TIME\n");
	fprintf(Fp, "options\t\tVAGUE_STATS\n");
	fprintf(Fp, "options\t\tSTREAMS\n");
	fprintf(Fp, "options\t\tSTRNULL\n");
	fprintf(Fp, "options\t\tSTRECHO\n");
	fprintf(Fp, "options\t\tSTRPASS\n");
/*	fprintf(Fp, "options\t\tSTRLOG\n");	what is this? */
	fprintf(Fp, "options\t\tSTRTMUX\n");
	fprintf(Fp, "options\t\tSTRSC\n");
	fprintf(Fp, "options\t\tINET\n");
	fprintf(Fp, "options\t\tUIPC\n");
	fprintf(Fp, "options\t\tDLI\n");
	fprintf(Fp, "#options\t\tLAT\n");
	fprintf(Fp, "options\t\tXTISO\n");
	fprintf(Fp, "options\t\tSTRKINFO\n");
	fprintf(Fp, "options\t\tNFS\n");
	fprintf(Fp, "options\t\tCDFS\n");
	fprintf(Fp, "options\t\tSYSV_COFF\n");
	fprintf(Fp, "options\t\tTRN\n");
#ifdef __alpha
	fprintf(Fp, "#");
#endif
	fprintf(Fp, "options\t\tSYSV_ELF\n");
	fprintf(Fp, "options\t\tTIMOD\n");
	fprintf(Fp, "options\t\tTIRDWR\n");
	fprintf(Fp, "options\t\tUFS\n");
	fprintf(Fp, "options\t\tQUOTA\n");
	fprintf(Fp, "options\t\tSVTT\n");
	fprintf(Fp, "options\t\tLABELS\n");
#ifdef __alpha
	fprintf(Fp, "options\t\talpha\n");
#else if defined (mips)
	fprintf(Fp, "options\t\tPMAX\n");
#endif
#if SEC_BASE
	fprintf(Fp, "options\t\tSEC_BASE\n");
	fprintf(Fp, "#options\t\tAUDIT\n");
#endif
	fprintf(Fp, "\n");

	fprintf(Fp, "#\n# Realtime options (see Realtime Installation ");
	fprintf(Fp, "Guide)\n#\n");
	fprintf(Fp, "# Note: \"options RT\" always configured\n#\n");
	fprintf(Fp, "options\t\tRT\n");
	fprintf(Fp, "#options\t\tRT_PREEMPT\n");
	fprintf(Fp, "#options\t\tRT_SCHED\n");
	fprintf(Fp, "#options\t\tRT_SCHED_RQ\n");
	fprintf(Fp, "#options\t\tRT_SCHED_OPT\n");
	fprintf(Fp, "#options\t\tRT_PML\n");
	fprintf(Fp, "#options\t\tRT_TIMER\n");
	fprintf(Fp, "#options\t\tUNIX_LOCKS\n");
/*	fprintf(Fp, "#options\t\tUNIX_UNI\n"); not currently used	*/
	fprintf(Fp, "\n");
}

print_pseudo_device()
{
        int ws = 0;

	fprintf(Fp, "\n#\n# Pseudodevice Definitions (see configuring the \n");
	fprintf(Fp, "# kernel chapter in the Guide to System Administration)");
	fprintf(Fp, "\n#\n");
	fprintf(Fp, "pseudo-device\tcpus\t%d\n", Maxcpu);
	fprintf(Fp, "pseudo-device\tpty\t255\n");
	fprintf(Fp, "pseudo-device\tlv\t3\n");
	fprintf(Fp, "pseudo-device\tether\n");
	fprintf(Fp, "pseudo-device\tsl\t2\n");
	fprintf(Fp, "pseudo-device\tloop\n");
	fprintf(Fp, "pseudo-device\tstrthreads\t1\n");
	fprintf(Fp, "pseudo-device\tstrheap\t384\n");
	fprintf(Fp, "pseudo-device\tstrpush\t16\n");
	fprintf(Fp, "pseudo-device\tnetthreads\t2\n");
	fprintf(Fp, "pseudo-device\tmbclusters\t1024\n");
#ifndef __alpha
	fprintf(Fp, "#pseudo-device\t\"i146818clock\"\n");
#endif
/*	fprintf(Fp, "pseudo-device\tmemd\n");	*/
#ifndef __alpha
	fprintf(Fp, "pseudo-device\tpmax\n");
#endif
#ifdef TODO_SCS
	if(Scs_sysidflag)
		fprintf(Fp, "pseudo-device\tscsnet\n");
#endif /* TODO_SCS */
#ifndef __alpha
	fprintf(Fp, "pseudo-device\tult_bin\n");
#endif
	fprintf(Fp, "pseudo-device\tsysv_hab\n");
	fprintf(Fp, "pseudo-device\tsvid_three_hab\n");
	fprintf(Fp, "pseudo-device\tsvr_four_hab\n");
	fprintf(Fp, "pseudo-device\tsoe_two_hab\n");
	fprintf(Fp, "pseudo-device\trt_hab\n");
}

print_opt_pseudo_device()
{
    int ws = 0, gc = 0;

    /*
     * get the "units" and the console type; if there are units, *OR*
     *  the console type is GENERIC CONSOLE (2), we want ws/xcons...
     */
    getsysinfo(GSI_WSD_UNITS, &ws, sizeof(ws));
    getsysinfo(GSI_WSD_CONS, &gc, sizeof(gc));

    /*
    **  we only care whether this is non-zero
    **  (it could be negative, and mean there
    **  are *LOTS* of graphic devices available)
    **	ALSO, if the kernel can't find a known
    **	graphic display, search the NAME.list
    **	to see if any known graphic devices
    **	are being installed. The clue, here, is the existence of
    **	the file 'GraphicDevices' in the layered product kit.
    **	The contents of this file are ignored--its existence is
    **	the signal we're looking for. -bg
    */
    if (ws || (gc == 2) || testThirdPartyGraphics ()) {
	    fprintf(Fp, "pseudo-device\txcons\n");
	    fprintf(Fp, "pseudo-device\tws\n");
	    fprintf(Fpdevs, "xcons ");
    }

    if(getsysinfo(GSI_PRESTO, 0, 0, 0, 0) > 0) {
	    fprintf(Fp, "pseudo-device\tpresto\n");
	    fprintf(Fpdevs, "pr0 ");
    }
}


/****************************************************************
* Name:		getdevices					*
*    								*
* Abstract:	Get the device information for the given system.* 
*		This routine will find all the alive ADAPTERS, 	*
*		CONTROLLERS, and DEVICES for the given system 	*
*		and place them into the alive device list table *
*		"adltbl[]". This is accomplished by reading the	*
*		device information from the appropriate kernel 	*
*		data structures in kernel memory. The I/O space	*
*		for the system is determined at system bootup	*
*		time during kernel autoconfiguration. The kernel*
*		autconfiguration code probes the I/O space to 	*
*		find all the alive devices in the system and it	*
*		updates its data structures accordingly.	*
*								*
* Inputs:	None.						*
*								*
* Outputs:	None.						*
*								*
* Return Values: None.						*
*								*
* Side Effects:	Many other routines are called to handle the	*
*		sizing of the system I/O space in order to     	*
*		find all the alive devices in the system. The 	*
*		following routines are called:			*
*								*
* getfloat()		Gets UNIBUS FLOATING device info.	*
* config_bus()  Finds all the busses, controllers and devices   *
*               and prints them out                             *
****************************************************************/
getdevices()
{
	config_bus(-1);

	/* Now find devices in FLOATING ADDRESS SPACE */
	getfloat();
	printdevice();
}

config_bus(busaddr)
	caddr_t busaddr;
{
	struct bus bus;
	char bus_name[20];
	char connect_bus[20];
	char port_name[20];

	do {
		if(getsysinfo(GSI_BUS_STRUCT, &bus, sizeof(struct bus), 
			   busaddr, 0) == -1)
				break;
		/*
		 * Ignore the ones with ALV_NOSIZER set because they
		 * refer to loadable drivers (which aren't specified
		 * in the config file).
		 */
		if((bus.alive & ALV_ALIVE) &&
		   ((bus.alive & ALV_NOSIZER) == 0)) {
			bzero(bus_name, sizeof(bus_name));
			getsysinfo(GSI_BUS_NAME, bus_name, sizeof(bus_name),
				   busaddr, 0);
			bzero(port_name, sizeof(port_name));
			getsysinfo(GSI_BUS_PNAME, port_name, sizeof(port_name),
				   busaddr, 0);
			bzero(connect_bus, sizeof(connect_bus));
			if(bus.bus_hd > (struct bus *)0)
				getsysinfo(GSI_BUS_NAME, connect_bus, 
					   sizeof(connect_bus), bus.bus_hd, 0);
			else
				strcpy(connect_bus, "nexus");
			bus.bus_name = bus_name;
			bus.pname = port_name;
			bus.connect_bus = connect_bus;
			print_dev(BUS, &bus, 0, 0);
			if(bus.bus_list)
				config_bus(bus.bus_list);
			if(bus.ctlr_list)
				config_ctlr(&bus);
		}
	} while(busaddr = (caddr_t)bus.nxt_bus);
}

config_ctlr(bus)
	struct bus *bus;
{
	caddr_t ctlraddr;
	char ctlr_name[20];
	char ctlr_pname[20];
	struct controller ctlr;

	ctlraddr = (caddr_t)bus->ctlr_list;
	do {
		if(getsysinfo(GSI_CTLR_STRUCT, &ctlr, 
			sizeof(struct controller), ctlraddr, 0) == -1)
				break;
		/*
		 * Ignore the ones with ALV_NOSIZER set because they
		 * refer to loadable drivers (which aren't specified
		 * in the config file).
		 */
		if((ctlr.alive & ALV_ALIVE) &&
		   ((ctlr.alive & ALV_NOSIZER) == 0)) {
			bzero(ctlr_name, sizeof(ctlr_name));
			getsysinfo(GSI_CTLR_NAME, ctlr_name, sizeof(ctlr_name),
				   ctlraddr, 0);
			bzero(ctlr_pname, sizeof(ctlr_pname));
			getsysinfo(GSI_CTLR_PNAME,ctlr_pname,sizeof(ctlr_pname),
				   ctlraddr, 0);
			ctlr.ctlr_name = ctlr_name;
			ctlr.pname = ctlr_pname;
			print_dev(CONTROLLER, bus, &ctlr, 0);
			if(ctlr.dev_list)
				config_dev(bus, &ctlr);
		}
	} while(ctlraddr = (caddr_t)ctlr.nxt_ctlr);
}

config_dev(bus, ctlr)
	struct bus *bus;
	struct controller *ctlr;
{
	caddr_t devaddr;
	char dev_name[20];
	struct device dev;

	devaddr = (caddr_t)ctlr->dev_list;
	do {
		if(getsysinfo(GSI_DEV_STRUCT, &dev, sizeof(dev), devaddr, 0) == -1)
				break;
		/*
		 * Ignore the ones with ALV_NOSIZER set because they
		 * refer to loadable drivers (which aren't specified
		 * in the config file).
		 */
		if((dev.alive & ALV_ALIVE) &&
		   ((dev.alive & ALV_NOSIZER) == 0)) {
			bzero(dev_name, sizeof(dev_name));
			getsysinfo(GSI_DEV_NAME, dev_name, 
				   sizeof(dev_name), devaddr, 0);
			dev.dev_name = dev_name;
			print_dev(DEVICE, bus, ctlr, &dev);
		}
	} while(devaddr = (caddr_t)dev.nxt_dev);
}

print_dev(type, bus, ctlr, dev)
	int type;
	struct bus *bus;
	struct controller *ctlr;
	struct device *dev;
{
	int index, j, nodeindex, num, conn_num, slot = -1, rctlr = 0;
	char *name, *conn_name, *port_name;
	char devname[DEVNAMESIZE], conname[DEVNAMESIZE];
	char tdevname[DEVNAMESIZE], tconname[DEVNAMESIZE];

	port_name = "";
	switch (type) {
	      case BUS:
		name = bus->bus_name;
		num = bus->bus_num;
		conn_name = bus->connect_bus;
		conn_num = bus->connect_num;
		slot = bus->slot;
		port_name = bus->pname;
		if(strcmp(bus->bus_name, "ci") == 0)
			Scs_sysidflag++;
		break;
	      case CONTROLLER:
		name = ctlr->ctlr_name;
		num = ctlr->ctlr_num;
		conn_name = bus->bus_name;
		conn_num = ctlr->bus_num;
		slot = ctlr->slot;
		rctlr = ctlr->rctlr;
		port_name = ctlr->pname;
		break;
	      case DEVICE:
		name = dev->dev_name;
		num = dev->logunit;
		conn_name = ctlr->ctlr_name;
		conn_num = dev->ctlr_num;
		break;
	}

	sprintf(devname, "%s%d", name, num);

	if(conn_num == -1)
		sprintf(conname, "%s?", conn_name);
	else
		sprintf(conname, "%s%d", conn_name, conn_num);

	if((index = match_device_name(name)) == -1)
		fprintf(Fp, "%-16s%-10s at %-10s", 
			"#UNSUPPORTED", devname, conname);
	else 
		fprintf(Fp, "%s%-16s%-10s at %-10s", 
			devtbl[index].supportedflag ? "" : "#",
			devtbl[index].devstr, devname, conname);

	if(*port_name != '\0')
		fprintf(Fp, "%-6s%s ", "port", port_name);

	if(slot != -1)
		fprintf(Fp, "%-6s%d ", "slot", slot);

	/* Print out the node type if one exists */
	if((nodeindex = match_node_name(conn_name)) != -1) {
		switch (nodetbl[nodeindex].nodetype) {
		      case BINODE:
		      case XMINODE: 
			fprintf(Fp, "%s%d  ", 
				nodetbl[nodeindex].nodename, slot);
			break;
		      case CINODE:
		      case MSINODE:
			fprintf(Fp, "%s %d  ", 
				nodetbl[nodeindex].nodename, rctlr);
			break;
		}
	}

	if(type != BUS && bus->bus_type == BUS_UNIBUS){
		unsigned short csr;

		if(strcmp(name, "idc") == 0)
			csr = 0175606;
		else
			csr = (unsigned short)ctlr->physaddr & 0xffff;

		switch(Cpu) {
		      case DS_5400:
		      case DS_5500:
		      case DS_5800:
			csr |= 0160000;
			break;
		}
		if( type == CONTROLLER )
			fprintf(Fp, "csr 0%6o  ", csr);

		/* Check for a floating address device */
		if(csr > 0160000 && csr < 0170000) {
#ifdef TODO_FLOAT
			if(!Float_flag) {
				fprintf(stdout, 
					"The installation software found ");
				fprintf(stdout, 
					"these devices in the floating\n");
				fprintf(stdout, "address space:\n\n");
				Float_flag = 1;
			}
#endif
			if(index != -1) {
				j = index;
				index = adltbl[j].device_index;
				sprintf(tdevname, "%s%d",
					devtbl[index].devname, adltbl[j].device_unit);
			}
			else {
				j = index;
				sprintf(tdevname, "%s", adltbl[j].unsupp_devname);
			}
			sprintf(tconname, "%s%d",
				adltbl[j].conn_name, adltbl[j].conn_number);
			fprintf(stdout, "\t%-10s\t", tdevname);
			fprintf(stdout, "on %-10s\t", tconname);
			fprintf(stdout, "at 0%o", csr);
		}
	}

	if(index != -1) {
		if(devtbl[index].flags)
			fprintf(Fp, "%-10s0x%x ", 
				"flags", devtbl[index].flags);
	
		if(strlen(devtbl[index].ivectors) != 0)
			fprintf(Fp, "%-10s%s ", "vector", 
				devtbl[index].ivectors);
	
		/* Make special file for this device */
		if(devtbl[index].makedevflag) 
#ifndef	DS5100_MDC_HACK
			fprintf(Fpdevs, "%s  ", devname);
#else
			if ((cputbl[cpu_index].cputype == DS_5100) &&
			     (strcmp("dc0", devname) == 0))
				fprintf(Fpdevs, "%s  ", "mdc0");
			else
				fprintf(Fpdevs, "%s  ", devname);
#endif
	}
	if(type == DEVICE && dev->unit != -1) 
		fprintf(Fp, "%-10s%d ", "drive", dev->unit);
	fprintf(Fp, "\n");
	if(type == BUS && bus->bus_type == BUS_TC && tc_callout == 0) {
		fprintf(Fp, "callout after_c \"../bin/mktcdata\"\n");
		tc_callout++;
	}
	if(type == BUS && bus->bus_type == BUS_EISA && eisa_callout == 0) {
		fprintf(Fp, "callout after_c \"../bin/mkeisadata\"\n");
		eisa_callout++;
	}
}

/* we may use this for floating space devices */

/****************************************************************
* Name:		printdevice					*
*    								*
* Abstract:	Print out the alive device information to the 	*
*		configuration file. This routine loops through	*
*		the alive device list table "adltbl[]" and 	*
*		prints out the alive ADAPTERS, CONTROLLERS, and *
*		DEVICES information to the configuration file.	*
*								*
* Inputs:	None.						*
*								*
* Outputs:	None.						*
*								*
* Return Values: None.						*
****************************************************************/
printdevice()
{

	struct alive_device_list *adl;
	int index, nodeindex, i;
	char tdevname[DEVNAMESIZE], tconname[DEVNAMESIZE];
	char *unsupported = "#UNSUPPORTED";
	char *harddevstr;

	/* Loop through the alive device linked list */
	for(i = 0; i < Adlindex; i++) {
		/* Get device index and node index */
		index = adltbl[i].device_index;
		nodeindex = adltbl[i].node_index;
		/* Print out for an unsupported device */
		if(index == -1) {
			sprintf(tdevname, "%s", adltbl[i].unsupp_devname);
			if(adltbl[i].conn_number == (int) '?')
			    sprintf(tconname, "%s?", adltbl[i].conn_name);
			else
			    sprintf(tconname, "%s%d",
		    	    	adltbl[i].conn_name, adltbl[i].conn_number);
			fprintf(Fp, "%-16s%-10s at %-10s", unsupported,
					tdevname, tconname);
			if(nodeindex != -1) {
				switch (nodetbl[nodeindex].nodetype) {
				      case BINODE:
				      case XMINODE:
					fprintf(Fp, "%s%d  ", 
						nodetbl[nodeindex].nodename,
						adltbl[i].node_number);
					break;
				      case CINODE:
				      case MSINODE:
					fprintf(Fp, "%s %d  ", 
						nodetbl[nodeindex].nodename,
						adltbl[i].rctlr);
					break;
				}
			}
			if(adltbl[i].csr != -1)
		    		printcsr(i);
			if(adltbl[i].device_drive != -1)
				fprintf(Fp,"drive %d", adltbl[i].device_drive);
			fprintf(Fp, "\n");
			continue;
		}

		/* Print out the device type */
		fprintf(Fp, "%s", devtbl[index].supportedflag ? "" : "#");

		fprintf(Fp, "%-16s%-16s", 
			"device", hardtbl[devtbl[index].devtype].typename);

		/* Print out the device name and the connection name */
		sprintf(tdevname, "%s%d",
				devtbl[index].devname, adltbl[i].device_unit);

		sprintf(tconname, "%s%d",
			adltbl[i].conn_name, adltbl[i].conn_number);

		fprintf(Fp, "%-10s at %-10s", tdevname, tconname);

		if(adltbl[i].node_number != -1)
			fprintf(Fp, "slot %d ", adltbl[i].node_number);

		/* Print out the node type if one exists */
		if(nodeindex != -1) {
			switch (nodetbl[nodeindex].nodetype) {
			      case BINODE:
			      case XMINODE: 
				fprintf(Fp, "%s%d  ", 
					nodetbl[nodeindex].nodename,
					adltbl[i].node_number);
				break;
			      case CINODE:
			      case MSINODE:
				fprintf(Fp, "%s %d  ", 
					nodetbl[nodeindex].nodename,
					adltbl[i].rctlr);
				break;
			}
		}

		if(adltbl[i].csr != -1)
		    	printcsr(i);

	    	if(devtbl[index].flags)
			fprintf(Fp, "flags 0x%x  ", devtbl[index].flags);

	    	if(strlen(devtbl[index].ivectors) != 0)
			fprintf(Fp, "vector %s ", devtbl[index].ivectors);

		if(adltbl[i].device_drive != -1) 
			fprintf(Fp, "drive %d", adltbl[i].device_drive);
		fprintf(Fp, "\n");

		/* Make special file for this device */
		if(devtbl[index].makedevflag && adltbl[i].alive_unit)
			printmakedev(i);
	}
}

/****************************************************************
* Name:		printcsr					*
*    								*
* Abstract:	Print out the CSR address of a device to the	*
*		configuration file.				*
*								*
* Inputs:							*
* i		The current index in the alive device table.	*
*								*
* Outputs:	None.						*
*								*
* Return Values: None.						*
****************************************************************/
printcsr(i)
int i;
{

	long csr;
	int index, j;
	char tdevname[DEVNAMESIZE], tconname[DEVNAMESIZE];

	index = adltbl[i].device_index;
	if(index != -1 && strcmp(devtbl[index].devname,"idc") == 0)
		adltbl[i].csr = 0175606;

	/* Print out the CSR address for this device */
	switch(Cpu) {
	case DS_5400:
	case DS_5500:
	case DS_5800:
		csr = adltbl[i].csr;
		csr |= 0160000;
		fprintf(Fp, "csr 0%6o  ", csr);
		break;
	default:
		csr = adltbl[i].csr;
		fprintf(Fp, "csr 0%6o  ", csr);
		break;
	}

	/* Check for a floating address device */
	if(csr > 0160000 && csr < 0170000) {
#ifdef TODO_FLOAT
		if(!Float_flag) {
			fprintf(stdout, "The installation software found ");
			fprintf(stdout, "these devices in the floating\n");
			fprintf(stdout, "address space:\n\n");
			Float_flag = 1;
		}
#endif
		if(index != -1) {
			j = i;
			index = adltbl[j].device_index;
			sprintf(tdevname, "%s%d",
				devtbl[index].devname, adltbl[j].device_unit);
		}
		else {
			j = i;
			sprintf(tdevname, "%s", adltbl[j].unsupp_devname);
		}
		sprintf(tconname, "%s%d",
		    		adltbl[j].conn_name, adltbl[j].conn_number);
		fprintf(stdout, "\t%-10s\t", tdevname);
		fprintf(stdout, "on %-10s\t", tconname);
		fprintf(stdout, "at 0%o\n", csr);
	}
}

/****************************************************************
* Name:		printmakedev					*
*    								*
* Abstract:	Print out the MAKEDEVICE information to the 	*
*		makedevices file.				*
*								*
* Inputs:							*
* i		The current index in the alive device table.	*
*								*
* Outputs:	None.						*
*								*
* Return Values: None.						*
****************************************************************/
printmakedev(i)
int i;
{

	int index, number;
	long offset;
	char devname[20];

	/* Print the the MAKEDEV entry for this device */
	index = adltbl[i].device_index;
	sprintf(devname, "%s%d", devtbl[index].devname,
			adltbl[i].device_unit);
	switch(Cpu) {
	case DS_5400:
	case DS_5500:
	case DS_5800:
		/* Special cases for "dz" and "dhu" devices */
		if(strcmp(devtbl[index].devname, "dz") == 0)
		    sprintf(devname, "dzv%d", adltbl[i].device_unit);
		else if(strcmp(devtbl[index].devname, "dhu") == 0)
		    sprintf(devname, "dhv%d", adltbl[i].device_unit);
		break;
	default:
		break;
	}

	/* Special cases for "dmb" devices */
#ifndef __alpha /* Alpha systems don't currently support the bi bus... */
	if(strcmp(devtbl[index].devname, "dmb") == 0) {
		number = adltbl[i].device_unit;
		offset = reset_anythg(NL_dmb_lines);
		offset = lseek(Kmem, offset+number*4, 0);
		read(Kmem, &number, sizeof(number));
		if(number == 16)
			sprintf(devname,"dhb%d",adltbl[i].device_unit);
		else
			sprintf(devname,"dmb%d",adltbl[i].device_unit);
	}
#endif /* __alpha */
	fprintf(Fpdevs, "%s  ", devname);
}

/*
**  This code is local stuff to analyze system files
**  to see if a third-party device that could qualify
**  as a graphic server display is being configured
**  as the sole display device. -bg
*/
#include <stdio.h>
#define THIRD_PARTY_DISP    "/GraphicDevices"
#define BUCKET_SIZE 16
/*
**  structure to serve as a bucket
**  for text from a system file
*/
typedef struct _myBucket
{
    struct _myBucket    *next;      /* linked list pointer */
    int     used;                   /* # of pointers in 'list' */
    char    *list[BUCKET_SIZE]; /* pointers to strings */
} BUCKET, *pBUCKET;
/*
**  given a bucket of pointers to dynamically allocated
**  buffers (strings, in this case), free each pointer
**  in the array, then free the array itself.
*/
static void freeBucket (pBUCKET plist)
{
    int ix;
    pBUCKET this;

    while (plist)
    {
        for (ix = 0; ix < plist->used; ix++) free (plist->list[ix]);
        this = plist->next;
        free (plist);
        plist = this;
    }
}
/*
**  given an open file descriptor, build a bucket (list) of
**  strings (without newlines) and return a pointer to the
**  base of the bucket list. Bucket entries only include
**  items of significance (not comments that begin with '#'
**  and go to the newline).
*/
static pBUCKET fillBucket (char *fileName)
{
    int filesize;
    char    *buffer;
    char    *ptr;
    FILE    *tpFile;
    pBUCKET plist, base;
    int     linesize;
    char    *pchar;
    int     ix;

    if (! (tpFile = fopen (fileName, "r"))) return ((pBUCKET) 0);
    filesize = getFileSize (tpFile);
    if (filesize == -1)
    {
        char erbuf [128];

        sprintf (erbuf, "sizing %s", fileName);
        perror (erbuf);
        fclose (tpFile);
        return ((pBUCKET) 0);
    }
    if (!(buffer = (char *) malloc (filesize)))
    {
        fprintf (stderr, "no memory available\n");
        fclose (tpFile);
        return ((pBUCKET)0);
    }
    if (fread (buffer, sizeof (char), filesize, tpFile) <= 0)
    {
        char erbuf [128];

        sprintf (erbuf, "reading %s", fileName);
        perror (erbuf);
        free (buffer);
        fclose (tpFile);
        return ((pBUCKET) 0);
    }
    fclose (tpFile);
    /*
    **  create the base bucket
    */
    plist = base = (pBUCKET) malloc (sizeof (BUCKET));
    plist->next = (pBUCKET) 0;
    plist->used = 0;
    bzero (plist->list, BUCKET_SIZE * sizeof (char *));
    for (ix = 0; ix < filesize; ix++)
    {
        if (buffer [ix] == '#') /* skip comment to newline */
        {
            while ((buffer [ix] != '\n') && (ix < filesize)) ix++;
            continue;
        }
        if (buffer [ix] == '\n')
        {
            continue;
        }
        if (plist->used >= BUCKET_SIZE)
        {
            plist->next = (pBUCKET) malloc (sizeof (BUCKET));
            plist = plist->next;
            plist->used = 0;
            bzero (plist->list, BUCKET_SIZE * sizeof (char *));
        }
        for (linesize = ix;
            buffer [linesize] != '\n' && linesize < filesize;
            linesize++);
        pchar = (char *) malloc (linesize + 1);
        plist->list[plist->used] = pchar;
        while ((buffer [ix] != '\n') && ix < filesize)
            *pchar++ = buffer [ix++];
        *pchar = '\0';  /* terminate the string */
        plist->used++;
    }
    free (buffer);
    return base;
}
/*
**  return filesize and guarantee that the file
**  pointer is set to the beginning of the file
*/
static int  getFileSize (FILE *tpFile)
{
    int filesize;

    /*
    **  seek to the end
    */
    if (fseek (tpFile, 0L, SEEK_END))
    {
        return -1;
    }
    /*
    ** find out where we are
    */
    filesize = ftell (tpFile);
    /*
    **  rewind to the beginning of the file.
    */
    if (fseek (tpFile, 0L, SEEK_SET))
    {
        return -1;
    }
    return filesize;
}
/*
**  test for existence of known third party graphics
**  devices in $Sysname.name file.
**  returns TRUE if there is an installed device
**  else returns false.
*/
static int testThirdPartyGraphics ()
{
    pBUCKET thirdParty, productList, tpWork, plWork;
    int     tpLine;
    int     plLine;
    char    *pTp, *pPl;
	extern char *Sysname;
	char	tempBuf[256];
	char	copy[1024];
	FILE	*pf;
	char    *pathname;  /* first token from list entry is the 
						**	path to the kit's directory
             			*/


	/*
	**	First, build a NAME.list file name and try to open
	**	it (in case we're being run from doconfig, or with
	**	a specific config file specified. If NAME.list doesn't
	**	exist return FALSE. -bg
	*/
	if (Sysname)	/* if there is a NAME.list ...*/
	{
		strcpy (tempBuf, Sysname);
		strcat (tempBuf, ".list");
		productList = fillBucket (tempBuf);
	}
	else productList = (pBUCKET) 0;
    if (! productList) 	/* no third-party stuff configured */
        return 0;
    for (plWork = productList; plWork; plWork = plWork->next)
    {
        for (plLine = 0; plLine < plWork->used; plLine++)
        {
            pPl = plWork->list[plLine];
			strcpy (copy, pPl);
			/* Test for blank lines and comment lines */
			if ( ((strlen (copy)-1) != (strspn (copy, " \t")))
                            && (copy[0] != '#')) 
			{
				pathname = strtok(copy,":\n");
				bzero (tempBuf, sizeof (tempBuf));
				strcpy (tempBuf, pathname);
				strcat (tempBuf, THIRD_PARTY_DISP);
				if (pf = fopen (tempBuf, "r"))
				{
					freeBucket (productList);
					return 1;
				}
			}
        }
    }
    freeBucket (productList);
    return 0;
}

