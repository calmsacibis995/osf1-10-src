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
static char	*sccsid = "@(#)$RCSfile: mkioconf.c,v $ $Revision: 4.6.10.5 $ (DEC) $Date: 1993/12/21 21:18:26 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
 
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* 
 * derived from mkioconf.c	5.1 (Berkeley) 5/8/85";
 */
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*	Change History							*
 *									*
 * 3-20-91	robin-							*
 *		Made changes to support new device data structures.	*
 *									*
 * 4-1-91	robin-							*
 *		Made chand to set the interrupt vector numbers		*
 *									*
 * 6-14-91	Brian Stevens						*
 *		Removed a warning message printed when controllers	*
 *		are specified in the config file with no vector.	*
 */

#include <stdio.h>
#include "y.tab.h"
#include "config.h"
#include <io/common/devdriver.h>

/*
 * build the ioconf.c file
 */


char	*qu();
char	*intv();
char	*intv2();
char	*qumscp();
struct controller  *controller_head;
struct controller  *controller_cur;
struct device  *Kdev_head;
struct device  *Kdev_cur;
struct bus  *bus_head;
struct bus  *bus_cur;


/*
 * no driver routines for these devices
 */
char *tbl_nodriver[] = { "dssc", "hsc", "mscp", 0 } ; 

dec_ioconf()
{
	register struct device_entry *dp, *mp, *np;
	int i;
	register int uba_n, slave;
	int fatal_error = 0;
	int directconn;
	FILE *fp;
	extern int isconfigured();
	register char **ptr = tbl_nodriver;
	struct unique_list *uql, *uqlh;

	uql = uqlh = 0;

	fp = fopen(path("ioconf.c"), "w");
	if (fp == 0) {
		perror(path("ioconf.c"));
		Exit(1);
	}

	fprintf(fp, "#include <sys/param.h>\n");
	fprintf(fp, "#include <sys/buf.h>\n");
	fprintf(fp, "#include <sys/map.h>\n");
	fprintf(fp, "#include <sys/vm.h>\n");
	fprintf(fp, "#include <sys/config.h>\n");
	fprintf(fp, "#include <io/common/devdriver.h>\n");
	fprintf(fp, "\n");

	if (machine == MACHINE_VAX)
		fprintf(fp, "#include <dec/io/mba/vax/mbavar.h>\n");

        fprintf(fp, "#include <io/dec/uba/ubavar.h>\n\n");
	fprintf(fp, "\n");
	/* 
	 * set up null routines for devices that do not have a
	 * driver associated with them.
	 */

	fprintf(fp,"int nulldev();\n");

	while(*ptr) { 
		if (!isconfigured(*ptr)) {
			ptr++;
			continue;
		}
    		for (dp = dtab; dp != 0; dp = dp->d_next) 
		    if(eq(dp->d_name, *ptr)) 
		    {
			    /* This structure should be defined in a data file, NOT here.
			     * fix this when the hsc, .... get ported.
			     */
			fprintf(fp, "/*struct driver %sdriver;*/ ", *ptr);
			fprintf(fp, " /* no %sdriver */\n", *ptr);
		    }
		ptr++;
	}

	fprintf(fp, "\n");

	/* set up a link list that looks like the dtab link list.  We want to keep the
	 * dtab data around unchanged but we need to change it so to solve this
	 * delema, make a copy that we can change. 
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next) {
			if(uql == (struct unique_list *) 0)
			{
				uql = (struct  unique_list *)malloc(sizeof (struct  unique_list));
				uqlh = uql;
				uql->name = dp->d_name;
				uql->number = dp->d_unit;
				uql->dp = dp;
				uql->next = (struct unique_list *)0;
			}else{

				uql->next = (struct unique_list *)malloc(sizeof (struct unique_list));
				uql = uql->next;
				uql->name = dp->d_name;
				uql->number = dp->d_unit;
				uql->dp = dp;
				uql->next = (struct unique_list *)0;
			}
		}
	make_unique(uqlh);

	/*
	 * Now generate  interrupt vectors for the unibus
	 */
	for (uql = uqlh; uql != 0; uql = uql->next) {

		dp = uql->dp;
		/* A bus is at the 'top level' and may be 'un-attached'
		 * but everything else should hook up.
		 */
		if((dp->d_type == CONTROLLER) || (dp->d_type == DEVICE)){
			if (((dp->d_conn) == 0) && (!eq(dp->d_wildcard,"*"))) {
				printf("%s%d isn't connected to anything\n",
				       dp->d_name, dp->d_unit);
				fatal_error++;
			}
		}

		if (dp->d_type == CONTROLLER)
			fprintf(fp,"extern struct driver %sdriver;\n",dp->d_name);

		/* If its a BUS or CONTROLLER and there is no vector passed in set up
		 * a nulldev vector to process any interrupts.  We expect vectors
		 * on all controlers but don't "force" it.  So if we get a controller
		 * without a vector we will issue a <WARNING> message but let it pass.
		 */
		if (dp->d_vec == 0) {
			if (((dp->d_type == CONTROLLER) || (dp->d_type == BUS)) && dp->d_unit >= 0)
				fprintf(fp,"int (*%sint%d[])() = { nulldev, 0 };\t/* no interrupt routine for %s */\n",
				dp->d_name,dp->d_unit,dp->d_name);
		}else{
			struct idlst *ip;
			if ((dp->d_type != CONTROLLER) && (dp->d_type != BUS))
				continue;
			if(ip = dp->d_vec) {
				fprintf(fp, "extern ");
				
				for (;;) {
#ifdef __alpha
 				    fprintf(fp,"%s()", ip->id);
#else 
				    fprintf(fp,"X%s%d()", ip->id,  dp->d_unit);
#endif /* __alpha */
				    ip = ip->id_next;
				    if (ip == 0)
					break;
				    fprintf(fp, ", ");
				}
				fprintf(fp, ";\n");
				fprintf(fp, "int\t (*%sint%d[])()\t= { ", dp->d_name,
					 dp->d_unit);
				ip = dp->d_vec;
				for (;;) {
#ifdef __alpha
 				    fprintf(fp,"%s", ip->id);
#else 
				    fprintf(fp,"X%s%d", ip->id,  dp->d_unit);
#endif /* __alpha__ */
				    ip = ip->id_next;
				    if (ip == 0)
					break;
				    fprintf(fp, ", ");
				}
				fprintf(fp, ", 0 } ;\n\n");

				/* If it wants a CSR and the value is zero
				 * its an error
				 */
				if (dp->d_addr == 0 && needs_csr(dp->d_name)) {
					printf("must specify csr address for %s%d\n",
					       dp->d_name, dp->d_unit);
					fatal_error++;
				}
	
				/* 'controller' config file entries should NOT
				 * have drive, unit or slave keywords associated
				 * with it.
				 */
				if (dp->d_drive != UNKNOWN || dp->d_slave != UNKNOWN) {
					printf("drives need their own entries; dont ");
					printf("specify drive or slave for %s%d\n",
					       dp->d_name, dp->d_unit);
					fatal_error++;
				}
			}
		}
	}

	/* Process device entry testing
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next) 
	{
		if (dp->d_type != DEVICE )
			continue;
		if ((dp->d_type == DEVICE) && (dp->d_vec != 0)) 
		{
			printf("interrupt vectors should not be ");
			printf("given for unit/drive %s%d\n",
			    dp->d_name, dp->d_unit);
			fatal_error++;
			continue;
		}
		if ((dp->d_type == DEVICE ) && (dp->d_addr != 0)) 
		{
			printf("csr addresses should be given only ");
			printf("on controllers, not on %s%d\n",
			    dp->d_name, dp->d_unit);
			fatal_error++;
			continue;
		}

		slave = dp->d_drive;
	}

	/* Call the routine that finds all the buses and creates a list
	 * of bus data structures which are used to make the kernel data
	 * structure.
	 */
	connect_bus();

	/* Create the data srtuctures for each controller type entry
	 */
	connect_controller();

	/* Get the device data set up */
	connect_device();

	/* Build a full port_list data structure
	 */
	start_port_entry(fp);
	make_port_entry(fp);
	end_port_entry(fp);

	/* build a full bus_list data structure 
	 */
	start_bus_entry(fp);
	make_bus_entry(fp);
	end_bus_entry(fp);

	/* Build a full controller_list data structure
	 */
	start_controller_entry(fp);
	make_controller_entry(fp);
	end_controller_entry(fp);

	/* Build a full device_list data structure
	 */
	start_dev_entry(fp);
	make_dev_entry(fp);
	end_dev_entry(fp);

	(void) fclose(fp);

	if (fatal_error)
		Exit(1);
}


char *intv(dev)
	register struct device_entry *dev;
{
	static char buf[20];

	if (dev->d_vec == 0)
		return ("     0");
	sprintf(buf, "%sint%d", dev->d_name, dev->d_unit);
	return (buf);
}

char *
qu(num)
{

	if (num == QUES)
		return ("'?'");
	if (num == UNKNOWN)
		return (" -1");
	sprintf(errbuf, "%3d", num);
	return (errbuf);
}


/*
 * needs_vector(name) things connect to this device need to supply a vector
 */
needs_vector(dev)
struct device_entry *dev;
{
	if(dev->d_type == CONTROLLER)
		return (1);
	else
		return (0);
}

char *tbl_needs_csr[] = { "uba", "uda", 0};


/*
 * needs_csr(name)
 * things connect to this device need to specify a csr.
 */
needs_csr(name)
register char *name;
{
    register char **ptr = tbl_needs_csr;
    
    while(*ptr)
	{
	    if(eq(*ptr,name))return(1);
	    ptr++;
	}
    return(0);
}

/*
 *dump out vector for each name device "str" which is connected to nexus 
 */
dump_vec_adapt(str,fpb)
register	char	*str;
register	FILE	*fpb;
{
    register	struct device_entry *dp, *mp;
   
    for (dp = dtab; dp != 0; dp = dp->d_next) {
	if(eq(dp->d_name,str))  {
	    if ((mp = dp->d_conn) == TO_NEXUS && (dp->d_unit != -1)) {
		if(eq(dp->d_name,"ci")) {
		    if(dp->d_unit == 0) {
			fprintf(fpb, "#include \"../h/types.h\"\n");
			fprintf(fpb, "#include \"../io/ci/ciadapter.h\"\n");
		    }
		    fprintf(fpb,"X%sint%d(stray_arg)\n",str,dp->d_unit);
		    fprintf(fpb,"int stray_arg;\n");
		    fprintf(fpb,"{\n");
		    fprintf(fpb,"/* stray_arg is not used here but locore calls it with\n");
		    fprintf(fpb," * an argument that is the offset into the scb data structure;\n");
		    fprintf(fpb," * which the stray interrupt routine uses to find where the\n");
		    fprintf(fpb," * stray came from.  The unused arg keeps everything consistent.\n");
		    fprintf(fpb," */\n");
		    fprintf(fpb,"\textern CIISR ci_isr[];\n");
		    fprintf(fpb,"\t(*ci_isr[%d].isr)(ci_isr[%d].pccb);\n",dp->d_unit,dp->d_unit);
		    fprintf(fpb,"}\n");
		} else {
		    fprintf(fpb,"X%serr%d(stray_arg)\n",str,dp->d_unit);
		    fprintf(fpb,"int stray_arg;\n");
		    fprintf(fpb,"{\n");
		    fprintf(fpb,"/* stray_arg is not used here but locore calls it with\n");
		    fprintf(fpb," * an argument that is the offset into the scb data structure;\n");
		    fprintf(fpb," * which the stray interrupt routine uses to find where the\n");
		    fprintf(fpb," * stray came from.  The unused arg keeps everything consistent.\n");
		    fprintf(fpb," */\n");
		    fprintf(fpb,"\textern %serrors();\n",str);
		    fprintf(fpb,"\t%serrors(%d);\n",str,dp->d_unit);
		    fprintf(fpb,"}\n");
		}
	    }		
	}
    } /* End for loop */
 
}


/*
 * MSCP devices represent wildcards in the kernel via " -1" instead of '?'.
 * This is necessary because the '?' is actually a 63 which conflicts with
 * a valid unit number of 63.
 */


char *
qumscp(num)
{

	if ((num == QUES) || (num == UNKNOWN))
		return ("-1");
	sprintf(errbuf, "%3d", num);
	return (errbuf);

}

dump_dispatch(str,fpb)
register	char	*str;
register	FILE	*fpb;
{

    register	struct device_entry *dp, *mp;
    for (dp = dtab; dp != 0; dp = dp->d_next) {
	if(eq(dp->d_name,str))  {
	    if ((mp = dp->d_conn) == TO_NEXUS && (dp->d_unit != -1)) {
    		if( eq(str, "ci") ) {
		        fprintf(fpb,"extern int X%sint%d();\n",str,dp->d_unit);
    		} else {
			fprintf(fpb,"extern int X%serr%d();\n",str,dp->d_unit);
		}
	    }		
	}
    }
    if( eq(str, "ci") ) {
	fprintf(fpb,"int\t(*%sintv[])() = {\n",str );
    } else {
        fprintf(fpb,"\nstruct bus_dispatch %serr_dispatch[] = {\n",str);
    }
    for (dp = dtab; dp != 0; dp = dp->d_next) {
	if(eq(dp->d_name,str))  {
	    if ((mp = dp->d_conn) == TO_NEXUS && (dp->d_unit != -1)) {
    		if( eq(str, "ci") ) {
		    fprintf(fpb,"\t X%sint%d ,\n",str,dp->d_unit);
    		} else {
		    fprintf(fpb,"\t{0x%x,(int)X%serr%d},\n",
				dp->d_unit,str,dp->d_unit);
		}
	    }		
	}
    }
    if( eq(str, "ci") ) {
        fprintf(fpb,"\t 0 };\n\n");
    } else {
        fprintf(fpb,"\t{-1,-1} };\n");
    }

}


/*
 * can things connect to this name?  Things can connect to a bus, a controller
 * and (I guess) a Master device type.
 */
can_connect(dp)
struct device_entry *dp;
{
  	switch ((int)dp->d_type){
	      case BUS:
		     return(1);
		     break;
	      case CONTROLLER:
		     return(1);
		     break;
	      case MASTER:
		     return(1);
		     break;
	      default:
		     return(0);
	};
}


uba_num(dp)
struct device_entry *dp;
{
	if(eq(dp->d_name ,"uba")) return(dp->d_unit);
	if(dp->d_conn == TO_NEXUS || dp->d_conn == 0) return(UNKNOWN);
	return(uba_num(dp->d_conn));
}

is_cpu_declared(cp)
register char *cp;
{
        struct cputype *cpup;

	for (cpup = cputype; cpup; cpup = cpup->cpu_next)
	        if (strcmp(cpup->cpu_name, cp) == 0)
		        return(1);
	return(0);
}

start_bus_entry(fp)
register	FILE	*fp;
{
	struct device_entry *bp;
	struct bus *bpl, *bplh, *bpf, *bpt; /* list, head, follow, trace */
	int found;

	/* Entries created in the bus structure are defined in other
	 * files so we need to declare them as extern functions.
	 */
	fprintf(fp,"\n");
	bplh = bpl = 0;
	for (bp = dtab; bp != 0; bp = bp->d_next)
	{
		if ( bp->d_type == BUS )
		{
			/* create a list of bus names making sure there are no
			 * duplicates.
			 */
			if(bpl == 0)
			{
				bpl = (struct bus *)malloc(sizeof (struct bus));
				bplh = bpl;
				bpl->bus_name = bp->d_name;
				bpl->nxt_bus = 0;
			}else{
				/* look for a match in the existing list */
				found = 0;
				for (bpf=bplh; bpf != 0; bpf=bpf->nxt_bus)
				{
					if( eq(bpf->bus_name, bp->d_name)  ){
						found = 0;
						break;
					}
                                        found = 1;
					bpt = bpf;
				}
				/* fell out so its not in the list, make one */
				if (found == 1)
				{
					bpl = (struct bus *)malloc(sizeof (struct bus));
					bpt->nxt_bus = bpl;
					bpl->nxt_bus = 0;
					bpl->bus_name = bp->d_name;
				}
			}
		}
	}
	if(bplh == 0){
		fprintf(stderr,"No 'bus' entry in the config file\n");
	}

	/* Walk the list we just made and make an entry for each function call */
	for (bpf=bplh; bpf != 0; bpf=bpf->nxt_bus)
	{
		fprintf(fp,"extern int %sconfl1();\n",bpf->bus_name);
		fprintf(fp,"extern int %sconfl2();\n",bpf->bus_name);
	}
        fprintf(fp,"\n");

	fprintf(fp,"\n/* struct	bus {\t*/\n");
	fprintf(fp,"/*\tu_long	        *bus_mbox	/* bus mail box */\n");
	fprintf(fp,"/*\tstruct bus	*nxt_bus	/* next bus				*/\n");
	fprintf(fp,"/*\tstruct controller *ctlr_list;	/* controllers connected to this bus */\n");
	fprintf(fp,"/*\tstruct bus	*bus_hd;	/* pointer to bus this bus connected to */\n");
	fprintf(fp,"/*\tstruct bus	*bus_list;	/* buses connected to this bus */\n");
	fprintf(fp,"/*\tint		bus_type;	/* bus type 			*/\n");
	fprintf(fp,"/*\tchar		*bus_name;	/* bus name 			*/\n");
	fprintf(fp,"/*\tint		bus_num;	/* bus number			*/\n");
	fprintf(fp,"/*\tint		slot;		/* node or slot number		*/\n");
	fprintf(fp,"/*\tchar		*connect_bus;	/* conected to bus name 	*/\n");
	fprintf(fp,"/*\tint		connect_num;	/* connected to bus number	*/\n");
	fprintf(fp,"/*\tint		(*confl1)();	/* Level 1 configuration routine */\n");
	fprintf(fp,"/*\tint		(*confl2)();	/* Level 2 configuration routine */\n");
	fprintf(fp,"/*\tchar		*pname;		/* port name, if needed		*/\n");
	fprintf(fp,"/*\tstruct port	*port;		/* pointer to port structure	*/\n");
	fprintf(fp,"/*\tint		(**intr)();	/* interrupt routine(s) for this bus  */\n");
	fprintf(fp,"/*\tint		alive;		/* alive indicator		*/\n");
	fprintf(fp,"/*\tstruct bus_framework	*framework; /* Subsystem expansion routines */\n");
	fprintf(fp,"/*\tchar		*driver_name;	/* name of controlling driver */\n");
	fprintf(fp,"/*\tvoid		*private[8];	/* Reserved for bus use		*/\n");
	fprintf(fp,"/*\tvoid		*conn_priv[8];	/* Reserved for connected bus use*/\n");
	fprintf(fp,"/*\tvoid 		*rsvd[8];	/* Reserved for future expansion */\n");
	fprintf(fp,"/* };\t*/\n\n\n");
}

make_bus_entry(fp)
FILE *fp;
{
	struct bus *bp;

	fprintf(fp,"struct bus bus_list[] = {\n");

	for (bp = bus_head; bp != 0; bp = bp->bus_list)
	{
	        fprintf(fp,"\t{ ");

		fprintf(fp,"0,");		/* mbox				*/
		fprintf(fp,"0,");		/* next bus                     */
		fprintf(fp,"0,");		/* ctlr_list			*/
		fprintf(fp,"0,");		/* bus_hd			*/
		fprintf(fp,"0,");		/* bus_list			*/
		fprintf(fp,"%d,",bp->bus_type);	/* bus type			*/
		fprintf(fp,"\"%s\",",bp->bus_name); /* bus name			*/
		fprintf(fp,"%d,",bp->bus_num);	/* bus_num			*/
		fprintf(fp,"%d,",bp->slot);	/* node or slot number		*/
		fprintf(fp,"\"%s\",",bp->connect_bus);/* conected to bus name   */
		fprintf(fp,"%d,",bp->connect_num);/* connected to bus number 	*/
		fprintf(fp,"%sconfl1,",bp->bus_name);/* lev1 routine 		*/
		fprintf(fp,"%sconfl2,",bp->bus_name);/* lev2 routine 		*/
		if(eq(bp->pname,"")){
		   fprintf(fp,"\"\",");		/* port name			*/
		   fprintf(fp,"0,");		/* port struct			*/
		}else{
		   fprintf(fp,"\"%s\",",bp->pname);/* port name			*/
		   fprintf(fp,"&%sport,",bp->pname);/* port struct		*/
	        }
		fprintf(fp,"%sint%d,",bp->bus_name,bp->bus_num == -1 ? 0 : bp->bus_num);/* intr routine */
		fprintf(fp,"%d,",bp->alive);    /* alive			*/
		fprintf(fp,"0,");		/* framework			*/
		fprintf(fp,"\"\",");		/* driver_name 			*/
		fprintf(fp,"0,");		/* private			*/
		fprintf(fp,"0,");		/* conn_private			*/
		fprintf(fp,"0");		/* RSVD				*/
		fprintf(fp,"},\n");
	}
}

end_bus_entry(fp)
FILE *fp;
{
	/* Make one NULL entry at the end (or as only entry).
	 */

	fprintf(fp,"\t{ ");
	fprintf(fp,"0,");		/* bus_mbox			*/
	fprintf(fp,"0,");		/* next bus			*/
	fprintf(fp,"0,");		/* ctlr_list			*/
	fprintf(fp,"0,");		/* bus_hd			*/
	fprintf(fp,"0,");		/* bus_list			*/
	fprintf(fp,"0,");		/* bus type			*/
	fprintf(fp,"(char *)0,"); 	/* bus_name			*/
	fprintf(fp,"0,");		/* bus_num			*/
	fprintf(fp,"0,"); 		/* node or slot number		*/
	fprintf(fp,"\"\",");		/* conected to bus name    	*/
	fprintf(fp,"0,");		/* connected to bus number 	*/
	fprintf(fp,"0,");		/* lev1 routine 		*/
	fprintf(fp,"0,");		/* lev2 routine 		*/
	fprintf(fp,"\"\",");		/* port name			*/
	fprintf(fp,"0,");		/* port struct			*/
	fprintf(fp,"0,");		/* interrupt routine		*/
	fprintf(fp,"0,");		/* alive			*/
	fprintf(fp,"0,");		/* framework			*/
	fprintf(fp,"(char *)0,"); 	/* driver_name			*/
	fprintf(fp,"0,");		/* private			*/
	fprintf(fp,"0,");		/* conn private			*/
	fprintf(fp,"0");		/* Reserved			*/
	fprintf(fp,"}\n");


	fprintf(fp,"};\n"); 
}

start_dev_entry(fp)
register	FILE	*fp;
{

	fprintf(fp,"\n/* struct	device_list {\t*/\n");
	fprintf(fp,"/*\tstruct device	*nxt_dev;	/* pointer to next dev on this ctlr */\n");
	fprintf(fp,"/*\tstruct controller *ctlr_hd;	/* pointer to ctlr for this device */\n");
	fprintf(fp,"/*\tchar		*dev_type;	/* device type			*/\n");
	fprintf(fp,"/*\tchar		*dev_name;	/* device name			*/\n");
	fprintf(fp,"/*\tint		logunit;	/* logical unit	number		*/\n");
	fprintf(fp,"/*\tint		unit;		/* physical unit number		*/\n");
	fprintf(fp,"/*\tint		ctlr_num;	/* controller number for this device */\n");
	fprintf(fp,"/*\tchar		*ctlr_name;	/* controller name connected to */\n");
	fprintf(fp,"/*\tint		alive;		/* alive indicator ( -1 if at   */\n");
	fprintf(fp,"/*\t                                /* nexus, -2 if dead, -4 if     */\n");
	fprintf(fp,"/*\t                                /* read only, -8 if write only )*/\n");
	fprintf(fp,"/*\tvoid		private[8];	/* reserved for device use	*/\n");
	fprintf(fp,"/*\tvoid 		conn_priv[8];	/* Reserved for connected ctlr use*/\n");
	fprintf(fp,"/*\tvoid 		rsvd[8];	/* reserved for future expansion */\n");
	fprintf(fp,"/* };\t*/\n\n\n");


	fprintf(fp,"struct device device_list[] = {\n");
}


make_dev_entry(fp)
register	FILE	*fp;
{
	struct device *dp;

        for(dp = Kdev_head; dp != 0; dp = dp->nxt_dev)
	{
		  /* Check for NEXUS connect when not OK */
		  check_nexus(dp,QUES);

		fprintf(fp,"\t{ ");
		fprintf(fp,"0,");			/* Next device struct	*/
		fprintf(fp,"0,");			/* pointer to ctlr	*/
		fprintf(fp,"\"%s\",",dp->dev_type);	/* device type		*/
		fprintf(fp,"\"%s\",",dp->dev_name); 	/* device name		*/
		fprintf(fp,"%d,",dp->logunit);		/* logical unit number  */
		fprintf(fp,"%d,",dp->unit);		/* physical unit number */
		fprintf(fp,"\"%s\",",dp->ctlr_name);	/* controller name	*/
		fprintf(fp,"%d,",dp->ctlr_num);		/* controller number	*/
		fprintf(fp,"%d,",dp->alive);		/* alive indicator	*/
		fprintf(fp,"0,");			/* reserved for device  */
		fprintf(fp,"0,");			/* conn_private			*/
		fprintf(fp,"0");			/* reserved		*/
		fprintf(fp,"},\n");
	}
}

end_dev_entry(fp)
register FILE *fp;
{

	/* Make one NULL entry at the end (or as only entry).
	 */

	fprintf(fp,"\t{ ");
	fprintf(fp,"0,");	/* Next device struct	*/	
	fprintf(fp,"0,");	/* pointer to ctlr	*/
	fprintf(fp,"\"\",");	/* device type		*/
	fprintf(fp,"(char *)0,"); 	/* device name		*/
	fprintf(fp,"0,");	/* logical unit number  */
	fprintf(fp,"0,");	/* physical unit number */
	fprintf(fp,"\"\",");	/* controller name	*/
	fprintf(fp,"0,");	/* controller number	*/
	fprintf(fp,"0,");	/* alive indicator	*/
	fprintf(fp,"0,");	/* reserved for device  */
	fprintf(fp,"0,");	/* conn private		*/
	fprintf(fp,"0");	/* reserved		*/
	fprintf(fp,"}\n");


	fprintf(fp,"};\n"); 
}

start_port_entry(fp)
register	FILE	*fp;
{
	fprintf(fp,"\n/* struct	port {\t*/\n");
	fprintf(fp,"/*\tint	(*conf)();		/* config routine for this port */\n");
	fprintf(fp,"/*};\t*/\n\n\n");

}

make_port_entry(fp)
register	FILE	*fp;
{

	extern struct controller *controller_head;
	extern struct bus *bus_head;
	register struct unique_list *uql, *uqlh;
	register struct unique_list *uqbl, *uqblh;
	register struct controller *cp;
	register struct bus *bp;

	uql = uqlh = 0;
        for(cp=controller_head;cp != 0; cp=cp->nxt_ctlr)
	{
		  /* Check for NEXUS connect when not OK */
		  check_nexus(cp,QUES);

		  if(!eq(cp->pname,"")){
			  if(uql == (struct unique_list *) 0){
				  uql = (struct  unique_list *)malloc(sizeof (struct  unique_list));
				  uqlh = uql;
				  uql->name = cp->pname;
				  uql->number = 0;
				  uql->dp = (struct device_entry *)cp;
				  uql->next = (struct unique_list *)0;
			  }else{
				  uql->next = (struct unique_list *)malloc(sizeof (struct unique_list));
				  uql = uql->next;
				  uql->name = cp->pname;
				  uql->number = 0;
				  uql->dp = (struct device_entry *)cp;
				  uql->next = (struct unique_list *)0;
			  }
		  }
        }

	uqbl = uqblh = 0;
        for(bp=bus_head; bp != 0; bp=bp->bus_list)
	{

		  if(!eq(bp->pname,"")){
			  if(uqbl == (struct unique_list *) 0){
				  uqbl = (struct  unique_list *)malloc(sizeof (struct  unique_list));
				  uqblh = uqbl;
				  uqbl->name = bp->pname;
				  uqbl->number = 0;
				  uqbl->dp = (struct device_entry *)bp;
				  uqbl->next = (struct unique_list *)0;
			  }else{
				  uqbl->next = (struct unique_list *)malloc(sizeof (struct unique_list));
				  uqbl = uqbl->next;
				  uqbl->name = bp->pname;
				  uqbl->number = 0;
				  uqbl->dp = (struct device_entry *)bp;
				  uqbl->next = (struct unique_list *)0;
			  }
		  }
        }

	make_unique(uqlh);
	/* Look through the entire device list looking for each controller
	 * entry and make a port structure entry.
	 */
	for (uql = uqlh; uql != 0; uql = uql->next) {
		fprintf(fp,"extern int %sinit();\n",uql->name);
		fprintf(fp,"struct port %sport = {\n",uql->name);
		fprintf(fp,"\t%sinit\n",uql->name);
                fprintf(fp,"};\n\n");
        }

	make_unique(uqblh);
	/* Look through the entire bus list looking for each 
	 * entry and make a port structure entry.
	 */
	for (uqbl = uqblh; uqbl != 0; uqbl = uqbl->next) {
		fprintf(fp,"extern int %sinit();\n",uqbl->name);
		fprintf(fp,"struct port %sport = {\n",uqbl->name);
		fprintf(fp,"\t%sinit\n",uqbl->name);
                fprintf(fp,"};\n\n");
        }
}
end_port_entry(fp)
register	FILE	*fp;
{

	/* Make one NULL entry at the end (or as only entry).
	 */
        fprintf(fp,"\n\n");
}


start_controller_entry(fp)
register	FILE	*fp;
{


	fprintf(fp,"\n/* struct	controller {\t*/\n");
	fprintf(fp,"/*\tu_long         *ctlr_mbox;	/* ctlr mailbox */\n");
	fprintf(fp,"/*\tstruct controller *nxt_ctlr;	/* pointer to next ctlr on this bus */\n");
	fprintf(fp,"/*\tstruct device	*dev_list;	/* devices connected to this ctlr */\n");
	fprintf(fp,"/*\tstruct bus	*bus_hd;	/* pointer to bus for this ctlr   */\n");
	fprintf(fp,"/*\tstruct driver	*driver;	/* pointer to driver structure for */\n");
	fprintf(fp,"/*\t				/* this controller 		   */\n");
	fprintf(fp,"/*\tint		ctlr_type;	/* controller type		*/\n");
	fprintf(fp,"/*\tchar		*ctlr_name;	/* controller name		*/\n");
	fprintf(fp,"/*\tint		ctlr_num;	/* controller number		*/\n");
	fprintf(fp,"/*\tchar		*bus_name;	/* bus name			*/\n");
	fprintf(fp,"/*\tint		bus_num;	/* bus number connected to 	*/\n");
	fprintf(fp,"/*\tint		rctlr;		/* remote controller number	*/\n");
	fprintf(fp,"/*\t				/* e.g. ci node or scsi id	*/\n");
	fprintf(fp,"/*\tint		slot;		/* node or slot number		*/\n");
	fprintf(fp,"/*\tint		alive;		/* alive indicator		*/\n");
	fprintf(fp,"/*\tchar		*pname;		/* port name			*/\n");
	fprintf(fp,"/*\tstruct port	*port;		/* port structure		*/\n");
	fprintf(fp,"/*\tint		(**intr)();	/* interrupt routine(s) for this ctlr */\n");
	fprintf(fp,"/*\tcaddr_t		addr;		/* virtual address of controller */\n");
	fprintf(fp,"/*\tcaddr_t		addr2;		/* virtual address of second ctlr */\n");
	fprintf(fp,"/*\t				/* register space		  */\n");
	fprintf(fp,"/*\tint		flags;		/* flags from from config 	*/\n");
	fprintf(fp,"/*\tint		bus_priority;	/* bus priority from from config */\n");
	fprintf(fp,"/*\tint		ivnum;		/* interrupt vector number	*/\n");
	fprintf(fp,"/*\tint		priority;	/* system ipl level		*/\n");
	fprintf(fp,"/*\tint		cmd;		/* cmd for go routine		*/\n");
	fprintf(fp,"/*\tcaddr_t		physaddr;	/* physical address of addr	*/\n");
	fprintf(fp,"/*\tcaddr_t		physaddr2;	/* physical address of addr2	*/\n");
	fprintf(fp,"/*\tvoid 		*private[8];	/* Reserved for ctlr use	*/\n");
	fprintf(fp,"/*\tvoid 		*conn_priv[8];	/* Reserved for connected bus use*/\n");
	fprintf(fp,"/*\tvoid 		*rsvd[8];	/* reserved for future expansion */\n");
        fprintf(fp,"/* };\t*/\n\n\n");

        fprintf(fp,"\n\n");

	fprintf(fp,"struct controller controller_list[] = {\n");

}

make_controller_entry(fp)
register	FILE	*fp;
{
	extern struct controller *controller_head;
	struct controller *cp;

        for(cp=controller_head;cp != 0; cp=cp->nxt_ctlr)
	{
		  /* Check for NEXUS connect when not OK */
		  check_nexus(cp,QUES);

	          fprintf(fp,"\t{ ");
		  fprintf(fp,"0,");			/* ctlr mbox */
		  fprintf(fp,"0,");			/* pointer to next ctlr on this bus */
		  fprintf(fp,"0,");			/* devices connected to this ctlr */
		  fprintf(fp,"0,");			/* pointer to bus for this ctlr   */
		  fprintf(fp,"&%sdriver,",cp->ctlr_name);/* pointer to driver structure for */
		                                        /* this controller 	*/
		  fprintf(fp,"%d,",cp->ctlr_type);	/* controller type	*/
		  fprintf(fp,"\"%s\",",cp->ctlr_name);	/* controler name	*/
		  fprintf(fp,"%d,",cp->ctlr_num);	/* controller number	*/
		  fprintf(fp,"\"%s\",",cp->bus_name ? cp->bus_name : "");	/* Bus name		*/
		  fprintf(fp,"%d,",cp->bus_num);	/* bus number		*/
		  fprintf(fp,"%d,",cp->rctlr);		/* remote ctlr ci/scsi node id	*/
		  fprintf(fp,"%d,",cp->slot);		/* node or slot number	*/
		  fprintf(fp,"%d,",cp->alive);		/* alive indicator	*/
		  fprintf(fp,"\"%s\",",cp->pname);	/* port name		*/
		  if(eq(cp->pname,""))
		          fprintf(fp,"0,");		/* ptr to port data	*/
		  else
			  fprintf(fp,"&%sport,",cp->pname);
		  fprintf(fp,"%sint%d,",cp->ctlr_name,cp->ctlr_num == -1 ? 0 : cp->ctlr_num);/* intr routine */
		  if(cp->addr)
			  fprintf(fp,"(caddr_t)0x%x,",cp->addr);	/* addr	*/
		  else
			  fprintf(fp,"0,");
		  if(cp->addr2)
			  fprintf(fp,"(caddr_t)0x%x,",cp->addr2);/* addr2	*/
		  else
			  fprintf(fp,"0,");
		  			                /* register space	*/
		  fprintf(fp,"0x%x,",cp->flags);	/* flags from config	*/
		  fprintf(fp,"%d,",cp->priority);	/* bus_priority		*/
		  fprintf(fp,"0x%x,",cp->ivnum);  	/* ivnum		*/
		  fprintf(fp,"0,");			/* priority		*/
		  fprintf(fp,"0,");			/* cmd			*/
		  fprintf(fp,"0,");			/* physaddr		*/
		  fprintf(fp,"0,");			/* physaddr2		*/
		  fprintf(fp,"0,");			/* private		*/
		  fprintf(fp,"0,");			/* conn_private		*/
		  fprintf(fp,"0");			/* Rsvd			*/
		  fprintf(fp,"},\n");

	  }
}

end_controller_entry(fp)
register	FILE	*fp;
{
	fprintf(fp,"\t{ ");
	fprintf(fp,"0,");		/* ctlr mbox */
	fprintf(fp,"0,");		/* pointer to next ctlr on this bus */
	fprintf(fp,"0,");		/* devices connected to this ctlr */
	fprintf(fp,"0,");		/* pointer to bus for this ctlr   */
	fprintf(fp,"0,");		/* pointer to driver structure for */
					/* this controller 		   */
	fprintf(fp,"0,");		/* controller type	*/
	fprintf(fp,"(char *)0,");	/* controler name	*/
	fprintf(fp,"0,");		/* controller number	*/
	fprintf(fp,"\"\",");		/* bus name		*/
	fprintf(fp,"0,");		/* bus number		*/
	fprintf(fp,"0,");		/* remote ctlr ci/scsi node id	*/
	fprintf(fp,"0,");		/* node or slot number	*/
	fprintf(fp,"0,");		/* alive indicator	*/
	fprintf(fp,"\"\",");		/* port name		*/
	fprintf(fp,"0,");		/* ptr to port data	*/
	fprintf(fp,"0,");		/* interrupt routine	*/
	fprintf(fp,"0,");		/* addr		  	*/
	fprintf(fp,"0,");		/* addr2		*/
			                /* register space	*/
	fprintf(fp,"0,");		/* bus_priority		*/
	fprintf(fp,"0,");		/* ivnum		*/
	fprintf(fp,"0,");		/* priority		*/
        fprintf(fp,"0,");		/* cmd			*/
	fprintf(fp,"0,");		/* physaddr		*/
	fprintf(fp,"0,");		/* physaddr2		*/
	fprintf(fp,"0,");		/* private		*/
	fprintf(fp,"0,");		/* conn private		*/
        fprintf(fp,"0");		/* Rsvd			*/
	fprintf(fp,"}\n");
	fprintf(fp,"};\n");
}

connect_controller()
{
	register struct device_entry *dp;
	struct controller *cp;

	/* Look through the entire device list looking for each controller
	 * entry and make a controller structure entry and link it onto a list.
	 */

	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if ( dp->d_type != CONTROLLER )
			continue;

       /* qar16825 fix - bypass dp entries created to parse wildcard ? (ie xmi?)
           device connections, they shouldn't be linked onto this list */
                if(dp->d_unit == -1)
                        continue;

		cp = (struct controller *)malloc(sizeof (struct controller));
		if(controller_head == 0){
			controller_head = cp;
		} else {
			controller_cur->nxt_ctlr = cp;
		}
		controller_cur = cp;

		cp->nxt_ctlr 	= 0;
		cp->dev_list 	= 0;
		cp->bus_hd   	= 0;
		cp->driver	= 0;
		cp->ctlr_type	= 0;
		cp->ctlr_name	= dp->d_name;
		cp->ctlr_num	= dp->d_unit;
		switch ((int)dp->d_conn){
		      case ((int)TO_NEXUS):
			cp->bus_name = "nexus";
			cp->bus_num  = -1;
			break;
		      case 0:
			cp->bus_name = "";
			cp->bus_num  = 0;
			break;
		      default:
			cp->bus_name = dp->d_conn->d_name;
			cp->bus_num  = dp->d_conn->d_unit;
		};
		/* If the entry is wildcard connected make this by setting the
		 * bus name its connected to as "*" and the bus number to -99 (why not
		 */
		if(eq(dp->d_wildcard, "*"))
		{
			cp->bus_name = dp->d_wildcard;
			cp->bus_num  = -99;
		}
		cp->rctlr	= dp->d_rcntl;
		cp->slot	= dp->d_slot;
		cp->alive	= dp->d_disable;
		cp->pname 	= dp->d_port_name ? dp->d_port_name : "";
		cp->port	= 0;
		cp->addr	= (caddr_t)dp->d_addr;
		cp->addr2 	= (caddr_t)dp->d_addr2;
		cp->flags	= dp->d_flags;
		cp->bus_priority= 0;
		cp->ivnum	= dp->d_ivnum;
		cp->priority	= dp->d_pri;
		cp->cmd		= 0;
		cp->physaddr	= 0;
		cp->physaddr2	= 0;

	}
}

connect_bus()
{
	register struct device_entry *dp;
	struct bus *bp;
	/* Look through the entire device list looking for each controller
	 * entry and make a controller structure entry and link it onto a list.
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if ( dp->d_type != BUS )
			continue;
		/* the first bus entry in the device_list is a dummy to start
		 * things off, sometimes??.  Can't seem to get rid of
		 * it without core dumps
		 * so it will be skipped over here if the name is NULL.
		 */
		if( eq(dp->d_name,"")) {
			continue;
		}

       /* qar16825 fix - bypass dp entries created to parse wildcard ? (ie xmi?)
           device connections, they shouldn't be linked onto this list */
                if(dp->d_unit == -1)
                        continue;

		bp = (struct bus *)malloc(sizeof (struct bus));
		if(bus_head == 0){
			bus_head = bp;
		} else {
			bus_cur->bus_list = bp;
        	}
		bus_cur = bp;

		bp->ctlr_list 	= 0;
	        bp->bus_hd     	= 0;
		bp->bus_list   	= 0;
		bp->bus_type	= 0;
		bp->bus_name	= dp->d_name;
		bp->bus_num	= dp->d_unit;
		bp->slot	= dp->d_slot;
/*printf("dp->d_conn %d TO_NEXUS %d\n",dp->d_conn,TO_NEXUS);*/
		switch ((int)dp->d_conn){
		      case ((int)TO_NEXUS):
			bp->connect_bus = "nexus";
			bp->connect_num  = -1;
			break;
		      case 0:
			bp->connect_bus = "";
			bp->connect_num  = 0;
			break;
		      default:
			bp->connect_num	= dp->d_conn->d_unit;
			bp->connect_bus = dp->d_conn->d_name?dp->d_conn->d_name:"";
		}
		if(eq(dp->d_wildcard, "*"))
		 {
			 bp->connect_bus = dp->d_wildcard;
			 bp->connect_num = -99;
		 }
/*printf("CONNECT_BUS port %s\n",dp->d_port_name);*/
		bp->pname	= dp->d_port_name ? dp->d_port_name : "";
		bp->port	= 0;
		bp->alive	= dp->d_disable;
	}
}

connect_device()
{
	register struct device_entry *dp;
	struct device *cp;

	/* Look through the entire device list looking for each controller
	 * entry and make a controller structure entry and link it onto a list.
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if ( dp->d_type != DEVICE )
			continue;

       /* qar16825 fix - bypass dp entries created to parse wildcard ? (ie xmi?)
           device connections, they shouldn't be linked onto this list */
                if(dp->d_unit == -1)
                        continue;

		cp = (struct device *)malloc(sizeof (struct device));
		if(Kdev_head == 0){
			Kdev_head = cp;
		} else {
			Kdev_cur->nxt_dev = cp;
		}

		Kdev_cur 	= cp;
		cp->ctlr_hd 	= 0;
		cp->dev_type   	= dp->d_type_string ? dp->d_type_string : "";
		cp->dev_name	= dp->d_name;
		cp->logunit	= dp->d_unit;  /* Logical # i.e. ra# */
		cp->unit	= dp->d_drive; /* Physical # i.e. unit # or drive # */
		cp->ctlr_num	= ((int)dp->d_conn > 0 ) ? dp->d_conn->d_unit : 0;
		cp->ctlr_name	= ((int)dp->d_conn > 0 ) ? dp->d_conn->d_name : ""; 
		cp->alive	= dp->d_disable;
		if(eq(dp->d_wildcard, "*"))
		 {
			 cp->ctlr_name = "*";
			 cp->ctlr_num = -99;
		 }

	}
}

make_unique(uql)
struct unique_list *uql;
{
	struct unique_list *ptr, *last;
	char name[256];
	int number;

	if(uql == (struct unique_list *) 0) return;

	number = uql->number;
	strcpy(name, uql->name);
	last   = uql;

	/* check every entry in the list for a match on the first entry in
	 * the list.  If it matches take it out of the list.
	 */
	for (ptr=uql->next; ptr != 0; ptr = ptr->next)
	{
		if((eq(name, ptr->name)) && (number == ptr->number))
		   {
			   last->next = ptr->next;
		   }else{
			   last = ptr;
		   }
	}
	/* call it again changing the 'top of the list' which we
	 * will match on.
	 */
	make_unique(uql->next);
}

dev_param(dp, str, num)
	register struct device_entry *dp;
	register char *str;
	long	num;
{
	yyerror("invalid parameter");
}
