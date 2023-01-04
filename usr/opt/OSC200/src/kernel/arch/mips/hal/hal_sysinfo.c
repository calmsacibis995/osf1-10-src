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
static char *rcsid = "@(#)$RCSfile: hal_sysinfo.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/07/08 08:39:29 $";
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
 *
 *   Modification history:
 *
 * 8 Oct 91 -- jaa
 *	fix GSI_BUS_STRUCT, GSI_BUS_NAME, GSI_CTLR_STRUCT, GSI_CTLR_NAME,
 *	GSI_DEV_STRUCT, GSI_DEV_NAME to look through the bus/ctlr/dev
 *	tree built at boot time.
 *
 * 05 Jun 91 -- map for pcameron (Phil Cameron)
 *	Print messages for unsupported ops if ULT_BIN_COMPAT is defined
 *
 * 31 May 91 -- map
 *	Ported to OSF
 *
 */


#include "ult_bin_compat.h"
#include <sys/types.h>
#include <sys/errno.h>
#include <dec/sas/mop.h>

/* 	Above sys dependancies to be part of a reduced set exhibiting
 *	minimal change.
 */

#include <hal/hal_sysinfo.h>
#include <io/common/devdriver.h>
#include <sys/sysconfig.h>
#include <sys/proc.h>

/*
 *	Routines for generic retrieval and storage of kernel information.
 *	These routines are meant to serve as a substitute for the abuse of
 *	ioctl(), fcntl(), and reading/writing /dev/mem.
 *
 *      These system calls are extensible.  In order to  prevent  them
 *      from becoming dumping grounds for inappropriate operations, new
 *      operations should be proposed and reviewed via the design review
 *	process.
 */


extern	char	consmagic[4];
extern	char	bootctlr[2];

extern	int	cpu;

extern	int	ws_display_type;
extern	int	ws_display_units;
extern int ws_num_controllers;
extern char *(*ws_graphics_name_proc)();

/*
 *	Retrieve system information
 */
int
hal_getsysinfo(p, args, retval) 	/* addition to 256 = getsysinfo()  */
	struct proc *p;			
	void *args;
	int *retval;
{
	register struct args {
		unsigned	 op;
		char		*buffer;
		unsigned	 nbytes;
		int		*start;
		char		*arg;
		unsigned	 flag;
	} *uap = (struct args *) args;

	int error = 0;
	u_short progenv = 0;
	extern struct netblk netblk ; /* DDF : need for NETBLK*/

	switch (uap->op) {

	      case GSI_CONSTYPE:	
		if( uap->nbytes < sizeof(consmagic) ) 
		    error = EINVAL;
		else {
		    error = copyout((caddr_t)consmagic,
			(caddr_t)uap->buffer, sizeof(consmagic) );
		    *retval = 1;
		}
		break;

	      case GSI_BOOTCTLR:	
		if( uap->nbytes < sizeof(bootctlr) ) 
		    error = EINVAL;
		else {
		    error = copyout((caddr_t)bootctlr,
			(caddr_t)uap->buffer, sizeof(bootctlr) );
		    *retval = 1;
		}
		break;

	      case GSI_WSD_TYPE:	/* Workstation Display Type Info */
		if( uap->nbytes < sizeof(ws_display_type) )
		    error = EINVAL;
		else {
		    error = copyout((caddr_t)&ws_display_type,
			(caddr_t)uap->buffer, sizeof(ws_display_type) );
		    *retval = 1;
		}
		break;

	      case GSI_WSD_UNITS:	
		if( uap->nbytes < sizeof(ws_display_units) )
		    error = EINVAL;
		else {
		    error = copyout((caddr_t)&ws_display_units,
			(caddr_t)uap->buffer, sizeof(ws_display_units) );
		    *retval = 1;
		}
		break;

              case GSI_GRAPHICTYPE:
                {
                    char null_char = 0;
                    char *name;
                    int start;

                    error = copyin((caddr_t)uap->start,
                                       (caddr_t)&start, sizeof(int));
                    if (error)
                        break;
                    /* check that nbytes is at least at large as the sizer of*/
                    /* each string in ws_module_name */
                    if ( uap->nbytes < 8 ) {
                        error = EINVAL;
                    } else if (ws_num_controllers == 0 ||
                               ws_graphics_name_proc == 0) {
                        error = copyout((caddr_t)&null_char,
                                            (caddr_t)uap->buffer, 1);
                        *retval = 1;
                    } else if ((start < 0) || (start >= ws_num_controllers)) {
                        error = EINVAL;
                    } else {
                        name = (*ws_graphics_name_proc)(start);
                        error = copyout((caddr_t)name,(caddr_t)uap->buffer, 8);
                        *retval = 1;
                        start++;
                        if (start == ws_num_controllers) {
                            start = 0;
                        }
                    }
                    error = copyout((caddr_t)&start,
                                (caddr_t)uap->start, sizeof(int));
                    break;
                }

	      case GSI_CPU:
		if( uap->nbytes < sizeof(cpu) ) 
		    error = EINVAL;
		else {
		    error = copyout((caddr_t)&cpu,
			(caddr_t)uap->buffer, sizeof(cpu) );
		    *retval = 1;
		}
		break;

	      case GSI_BUS_STRUCT: 
	      case GSI_BUS_NAME:
	      case GSI_BUS_PNAME:
		{
			struct bus *bus, *nexus_bus;
			struct bus *get_bus_struct(), *config_nexus_bus();

			LOCK_TOPOLOGY_TREE;
			nexus_bus = config_nexus_bus();
			/* short cut for "nexus" */
			if(uap->start == (int *) -1) {
			    if(uap->op == GSI_BUS_STRUCT) 
			        error = copyout((caddr_t)nexus_bus, 
						(caddr_t)uap->buffer,
					   	min(uap->nbytes,
						sizeof(struct bus)));
			    else if(uap->op == GSI_BUS_NAME)	
				error = copyoutstr((caddr_t)nexus_bus->bus_name,
					      (caddr_t)uap->buffer,
					      min(strlen(nexus_bus->bus_name)+1,
					      uap->nbytes),
					      (int*)0);
			    else 
				error = copyoutstr((caddr_t)nexus_bus->pname,
					     (caddr_t)uap->buffer,
					     min(strlen(nexus_bus->pname)+1,
					     uap->nbytes),
					     (int*)0);
			    UNLOCK_TOPOLOGY_TREE;
			    *retval = 1;
			    break;
			}
			if((bus = get_bus_struct(uap->start, nexus_bus))) {
			    if(uap->op == GSI_BUS_STRUCT) 
				error = copyout((caddr_t)bus, 
						(caddr_t)uap->buffer,
						min(uap->nbytes,
						sizeof(struct bus)));
			    else if(uap->op == GSI_BUS_NAME)	
				error = copyoutstr((caddr_t)bus->bus_name, 
						   (caddr_t)uap->buffer,
						   min(strlen(bus->bus_name)+1,
						   uap->nbytes),
						   (int*)0);
			    else
				error = copyoutstr((caddr_t)bus->pname, 
						   (caddr_t)uap->buffer,
						   min(strlen(bus->pname)+1,
						   uap->nbytes),
						   (int*)0);
			    *retval = 1;
			} else 
			    error = ENOENT;
			UNLOCK_TOPOLOGY_TREE;
		}
		break;

	      case GSI_CTLR_STRUCT: 
	      case GSI_CTLR_NAME:
	      case GSI_CTLR_PNAME:
		{
			struct bus *nexus_bus, *config_next_bus();
			struct controller *ctlr, *get_ctlr_struct();
			
			LOCK_TOPOLOGY_TREE;
			nexus_bus = config_nexus_bus();
			if((ctlr = get_ctlr_struct(uap->start, nexus_bus)) != 
			   (struct controller *)0) {
			    if(uap->op == GSI_CTLR_STRUCT)
			        error = copyout((caddr_t)ctlr, 
						(caddr_t)uap->buffer,
						min(uap->nbytes,
						sizeof(struct controller)));
			    else if(uap->op == GSI_CTLR_NAME)	 
			         error = copyoutstr((caddr_t)ctlr->ctlr_name,
						(caddr_t)uap->buffer,
					        min(strlen(ctlr->ctlr_name)+1, 
						uap->nbytes),
						(int*)0);
			    else
			         error = copyoutstr((caddr_t)ctlr->pname,
						(caddr_t)uap->buffer,
					        min(strlen(ctlr->pname)+1, 
						uap->nbytes),
						(int*)0);
			    *retval = 1;
			} else 
				error = ENOENT;
			UNLOCK_TOPOLOGY_TREE;
		}
		break;

	      case GSI_DEV_STRUCT: 
	      case GSI_DEV_NAME:
		{
			struct bus *nexus_bus, *config_nexus_bus();
			struct device *dev, *get_dev_struct();
			
			LOCK_TOPOLOGY_TREE;
			nexus_bus = config_nexus_bus();
			if((dev = get_dev_struct(uap->start, nexus_bus)) != 
			   (struct device *)0) {
				error = ((uap->op == GSI_DEV_STRUCT) ? 
					 copyout((caddr_t)dev, 
						 (caddr_t)uap->buffer,
						 min(uap->nbytes,
						     sizeof(struct device)))
					 : copyoutstr((caddr_t)dev->dev_name, 
						      (caddr_t)uap->buffer,
					      min(strlen(dev->dev_name) + 1, 
						  uap->nbytes), (int*)0));
				*retval = 1;
			} else 
				error = ENOENT;
			UNLOCK_TOPOLOGY_TREE;
		}
		break;

	      case GSI_MAX_CPU:	
		{
			extern cpu_avail;
			error = copyout((caddr_t)&cpu_avail, 
					(caddr_t)uap->buffer,
					min(uap->nbytes, sizeof(cpu_avail)));
			*retval = 1;
		}
		break;

	      case GSI_PRESTO:
		{
		        extern prsize;
			*retval = prsize;
		}
		break;

	      case GSI_SCS_SYSID:	/* used by sizer to get ci_first_port */
		{			/* for SCS_SYSID in config file  */
		        extern u_short ci_first_port;
			error = copyout((caddr_t)&ci_first_port, 
					(caddr_t)uap->buffer,
					min(uap->nbytes,sizeof(ci_first_port)));
			*retval = 1;
		}
		break;

	      case GSI_NETBLK:		/* get contents of netblk structure */
		if( uap->nbytes < sizeof(struct netblk) )
		    error = EINVAL;
		else {
		    error = copyout((caddr_t)&netblk,
			(caddr_t)uap->buffer, sizeof(struct netblk) );
		    *retval = 1;
		}
		break;

		/*
	 	 * Return the state of a loaded device driver.
	 	 * NOTE: this is likely to be replaced with the 1.1
		 * approach to making cfgmgr stateless.
		 */
              case GSI_DEV_MOD:
                 {
			int driver_get_state();
        		dev_mod_t buf;
        		char *start;

                        /* Get the suggested start value */
                        if (error = copyin(uap->start,&(char *)start,
                                sizeof(char *)))
                                break;

                        /* Get the needed struct */
                        if(error = driver_get_state(&start, &buf))
                                break;

                        /* Copy the struct to use space */
                        if(uap->nbytes != sizeof(dev_mod_t)) {
                                error = EINVAL;
                                break;
                        }
                        if(error = copyout((caddr_t)&buf,
                            (caddr_t)uap->buffer, uap->nbytes))
                                break;

                        /* Update start */
                        error = copyout((caddr_t)&start, (caddr_t)uap->start,
                                        sizeof(char *));
                        *retval = 0;
                 }
                break;

	      default:
		error = EINVAL;
		break;
	}
	return (error);
}

int
hal_setsysinfo(cp, args, retval)             /* addition to 257 = setsysinfo */
        struct proc *cp;
        void *args;
        int *retval;
{
        register struct args {
                unsigned        op;
                caddr_t         buffer;
                unsigned        nbytes;
                caddr_t         arg;
                unsigned        flag;
        } *uap = (struct args *) args;
	
	register caddr_t        p;
	int error = 0;

	/* Add platform specific setsysinfo() cases here */

	switch(uap->op) {

		default:
		  error = EINVAL;
		  break;
	}
	return (error);
}

/*
 * Process the HAL specific forms of SSI_NVPAIRS.
 *
 * Name-Value pairs form of input
 *
 * An action routine for each `name' is found in the switch statement
 * below. It is the responsibility of this action routine to validate
 * its value (setting error if an error exists) and also
 * to advance the argument buffer pointer `ptr' so that it is ready for
 * the next `name' copyin in the for-loop which appears in the SSI_NVPAIRS
 * case of the SWOE setsysinfo routine.
 */
int
hal_setsysinfo_nvpair(ptr, name)
	caddr_t	*ptr;
	int	name;
{
	int error = 0;

	switch (name) {
	      /*
	       * Set loadable driver config info
	       */
	      case SSIN_LOAD_CONFIG:
		{
			int add_config_entry();
			struct config_entry config_st;

#if     SEC_BASE
        		if (!privileged(SEC_SYSATTR, 0))
          		    break;
#else
        		if (suser(u.u_cred, &u.u_acflag)) {
          		    uprintf("setsysinfo: Sorry, must be super-user\n");
          		    break;
        		}
#endif
			if (error = copyin(*ptr, &config_st, 
				sizeof(struct config_entry))) {
				break;
			}
			*ptr += sizeof(struct config_entry);
			error = add_config_entry(&config_st);
		}
		break;
	      case SSIN_DEL_CONFIG:
		{
			int del_config_entry();
			struct config_entry config_st;

#if     SEC_BASE
        		if (!privileged(SEC_SYSATTR, 0))
          		    break;
#else
        		if (suser(u.u_cred, &u.u_acflag)) {
          		    uprintf("setsysinfo: Sorry, must be super-user\n");
          		    break;
        		}
#endif
			if (error = copyin(*ptr, &config_st, 
				sizeof(struct config_entry))) {
				break;
			}
			*ptr += sizeof(struct config_entry);
			error = del_config_entry(&config_st);
		}
		break;
	      /*
	       * Set the driver method's state.  
	       * NOTE: this is likely to change with the 1.1 approach
	       * to making cfgmgr stateless.
	       */
	      case SSIN_LOAD_DEVSTATE:
		{
			int add_driver_state_entry();
			dev_mod_t devmod_st;

#if     SEC_BASE
        		if (!privileged(SEC_SYSATTR, 0))
          		    break;
#else
        		if (suser(u.u_cred, &u.u_acflag)) {
          		    uprintf("setsysinfo: Sorry, must be super-user\n");
          		    break;
        		}
#endif
			if (error = copyin(*ptr, &devmod_st, 
				sizeof(dev_mod_t))) {
				break;
			}
			*ptr += sizeof(dev_mod_t);
			error = add_driver_state_entry(&devmod_st);
		}
		break;
	      case SSIN_UNLOAD_DEVSTATE:
		{
			int del_driver_state_entry();
			dev_mod_t  devmod_st;

#if     SEC_BASE
        		if (!privileged(SEC_SYSATTR, 0))
          		    break;
#else
        		if (suser(u.u_cred, &u.u_acflag)) {
          		    uprintf("setsysinfo: Sorry, must be super-user\n");
          		    break;
        		}
#endif
			if (error = copyin(*ptr, &devmod_st, 
				sizeof(dev_mod_t))) {
				break;
			}
			*ptr += sizeof(dev_mod_t);
			error = del_driver_state_entry(&devmod_st);
		}
		break;

	     default:
		error = EINVAL;
		break;
	}
	return(error);
}

static struct bus *
config_nexus_bus()
{
	extern struct bus *system_bus;

	if (system_bus != (struct bus *)0)
		return (system_bus);
	else
		panic ("Pointer to nexus bus (*system_bus) is NULL : no root bus");
}

/*
 * start at root of bus tree (nexus), and search looking for a match
 * on the user bus.  
 */
struct bus *
get_bus_struct(user_bus, start_bus)
	struct bus *user_bus, *start_bus;
{
	struct bus *nxtbus, *busp, *get_bus_struct();
	
	for(nxtbus = start_bus; nxtbus; nxtbus = nxtbus->nxt_bus) {
		if(nxtbus == user_bus)
			return(nxtbus);
		if(nxtbus->bus_list)
			if(busp = get_bus_struct(user_bus, nxtbus->bus_list))
				return(busp);
	}
	return((struct bus *)0);
}

struct controller *
get_ctlr_struct(user_ctlr, bus)
	struct controller *user_ctlr;
	struct bus *bus;
{
	struct controller *get_ctlr_struct(), *ctlr;
	struct bus *nxtbus;

	for(nxtbus = bus; nxtbus; nxtbus = nxtbus->nxt_bus) {
		for(ctlr = nxtbus->ctlr_list; ctlr; ctlr = ctlr->nxt_ctlr) {
			if(ctlr == user_ctlr)
				return(ctlr);
		}
		if(nxtbus->bus_list)
			if(ctlr = get_ctlr_struct(user_ctlr, nxtbus->bus_list))
				return(ctlr);
	}
	return((struct controller *)0);
}

struct device *
get_dev_struct(user_dev, bus)
	struct device *user_dev;
	struct bus *bus;
{
	struct device *get_dev_struct(), *dev;
	struct controller *ctlr;
	struct bus *nxtbus;

	for(nxtbus = bus; nxtbus; nxtbus = nxtbus->nxt_bus) {
		for(ctlr = nxtbus->ctlr_list; ctlr; ctlr = ctlr->nxt_ctlr) {
			for(dev = ctlr->dev_list; dev; dev = dev->nxt_dev) {
				if(dev == user_dev)
					return(dev);
			}
		}
		if(nxtbus->bus_list)
			if(dev = get_dev_struct(user_dev, nxtbus->bus_list))
				return(dev);
	}
	return((struct device *)0);
}
