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
static char *rcsid = "@(#)$RCSfile: hal_sysinfo.c,v $ $Revision: 1.1.15.5 $ (DEC) $Date: 1993/10/19 21:50:05 $";
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

#include "ult_bin_compat.h"
#include <sys/types.h>
#include <sys/errno.h>
#include <dec/sas/mop.h>

/* 	Above sys dependancies to be part of a reduced set exhibiting
 *	minimal change.
 */

#include <hal/hal_sysinfo.h>
#include <sys/sysinfo.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <sys/sysconfig.h>
#include <sys/proc.h>
#include <arch/alpha/prom.h>
#include <machine/rpb.h>

#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>

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


extern	int	cpu;

extern	int	ws_display_type;
extern	int	ws_display_units;
extern int ws_num_controllers;
extern char *(*ws_graphics_name_proc)();
extern char *(*ws_keyboard_name_proc)();
extern char *(*ws_pointer_name_proc)();
extern int (*ws_graphics_get_width_proc)();
extern int (*ws_graphics_get_height_proc)();

extern int consDev;
extern int generic_console_active;

#define BOOTDEVLEN 80
#define BOOTTYPELEN 10
extern char bootdevice[BOOTDEVLEN];	/* the name of the bootdevice */
extern char boottype[BOOTTYPELEN];	/* the name of the boottype */

#define BOOTEDFILELEN	80
char bootedfile[BOOTEDFILELEN];

/*
 *	Retrieve system information
 */
int
hal_getsysinfo(p, args, retval) 	/* addition to 256 = getsysinfo()  */
	struct proc *p;			
	void *args;
	long *retval;
{
	register struct args {
		unsigned long	 op;
		char		*buffer;
		unsigned long	 nbytes;
		int		*start;
		char		*arg;
		unsigned long	 flag;
	} *uap = (struct args *) args;

	int error = 0;
	u_short progenv = 0;
	extern struct netblk netblk ; /* DDF : need for NETBLK*/

	switch (uap->op) {

	       case GSI_PROM_ENV:
	       {
		  char *buf, command[16], *prom_getenv();
		  int maxlen = (uap->nbytes > MAX_ENVIRON_LENGTH) ?
					MAX_ENVIRON_LENGTH : uap->nbytes;

		  if (error = copyinstr(uap->arg, command,
						sizeof(command), (int *)0))
			break;

		  buf = prom_getenv(command);
		  if (buf != (char *)0)
			error = copyoutstr(buf, uap->buffer, maxlen, (int*)0);
		  else
			error = EINVAL;

		  break;
	      }

	      case GSI_IEEE_FP_CONTROL:
	      {
		long temp;
		temp = ieee_get_fp_control();
		error = copyout((caddr_t)&temp,
			(caddr_t)uap->buffer, sizeof(temp) );
		break;
	      }
	      case GSI_IEEE_STATE_AT_SIGNAL:
	      { 
		long arglst[3];
		arglst[0] = arglst[1] = arglst[3] = 0;
		arglst[2] = ieee_get_state_at_signal(&arglst[0], &arglst[1]);
		error = copyout((caddr_t)arglst,
			(caddr_t)uap->buffer, sizeof(arglst) );
		break;
	      }
	      case GSI_CONSTYPE:
		/*
		 * printf("sysinfo: GSI_CONSTYPE not implemented on alpha\n");
		 */
		error = EINVAL;
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

	      case GSI_WSD_CONS:	
		{
		    int devcons = ((consDev == GRAPHIC_DEV) ? 0 : 1);

		    if (!devcons && generic_console_active)
			    devcons = 2;

		    if( uap->nbytes < sizeof(devcons) )
			error = EINVAL;
		    else {
			error = copyout((caddr_t)&devcons,
					(caddr_t)uap->buffer,
					sizeof(devcons) );
			*retval = 1;
		    }
		    break;
		}

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

              case GSI_GRAPHIC_RES:
                {
                    char null_char = 0;
		    struct {
			int width;
			int height;
		    } res_buf;
                    int start;

                    error = copyin((caddr_t)uap->start,
                                       (caddr_t)&start, sizeof(int));
                    if (error)
                        break;
                    /* check that nbytes is at least at large as the size of */
                    /* the structure we will put info into */
                    if ( uap->nbytes < sizeof(res_buf) ) {
                        error = EINVAL;
                    } else if (ws_num_controllers == 0 ||
                               ws_graphics_get_width_proc == 0 ||
			       ws_graphics_get_height_proc == 0) {
			res_buf.width = 0;
			res_buf.height = 0;
                        error = copyout((caddr_t)&res_buf,
					(caddr_t)uap->buffer, sizeof(res_buf));
                        *retval = 1;
                    } else if ((start < 0) || (start >= ws_num_controllers)) {
                        error = EINVAL;
                    } else {
			res_buf.width = (*ws_graphics_get_width_proc)(start);
			res_buf.height = (*ws_graphics_get_height_proc)(start);
                        error = copyout((caddr_t)&res_buf,(caddr_t)uap->buffer,
					sizeof(res_buf));
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

	      case GSI_KEYBOARD:
                {
                    char null_char = 0;
                    char *name = &null_char;
		    int size = 1;

		    /*
		     * make sure there *is* such a routine, first...
		     * then fetch a ptr to the name
		     */
                    if (ws_keyboard_name_proc) {
			    name = (*ws_keyboard_name_proc)();
			    if (name)
				    size = strlen(name) + 1;
			    else
				    name = &null_char;
		    }

		    /*
		     * check that nbytes is at least at large as the
		     *  size of the name string
		     */
		    if (uap->nbytes < size)
			    error = EINVAL;
		    else {
			    error = copyout((caddr_t)name,
					    (caddr_t)uap->buffer, size);
			    *retval = 1;
		    }
                    break;
                }

	      case GSI_POINTER:
                {
                    char null_char = 0;
                    char *name = &null_char;
		    int size = 1;

		    /*
		     * make sure there *is* such a routine, first...
		     * then fetch a ptr to the name
		     */
                    if (ws_pointer_name_proc) {
			    name = (*ws_pointer_name_proc)();
			    if (name)
				    size = strlen(name) + 1;
			    else
				    name = &null_char;
		    }

		    /*
		     * check that nbytes is at least at large as the
		     *  size of the name string
		     */
		    if (uap->nbytes < size)
			    error = EINVAL;
		    else {
			    error = copyout((caddr_t)name,
					    (caddr_t)uap->buffer, size);
			    *retval = 1;
		    }
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
							   min(strlen(nexus_bus->bus_name) + 1, uap->nbytes),
							   (int*)0);
				else	/* GSI_BUS_PNAME */
					error = copyoutstr((caddr_t)nexus_bus->pname,
							   (caddr_t)uap->buffer,
							   min(strlen(nexus_bus->pname) + 1, uap->nbytes),
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
							   min(strlen(bus->bus_name) + 1, uap->nbytes),
							   (int*)0);
				else 	/* GSI_BUS_PNAME */
					error = copyoutstr((caddr_t)bus->pname, 
							   (caddr_t)uap->buffer,
							   min(strlen(bus->pname) + 1, uap->nbytes),
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
			    else	/* GSI_CTLR_PNAME */
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
							  uap->nbytes),
						   (int*)0));
					*retval = 1;
			} else 
				error = ENOENT;
			UNLOCK_TOPOLOGY_TREE;
		}
		break;

	      case GSI_BOOTDEV:	
		if( (int)uap->nbytes < BOOTDEVLEN ) 
		    error = EINVAL;
		else {
		    char buf[BOOTDEVLEN];
		    if (!bootdevice_parser(bootdevice, buf))
			    strcpy(buf, bootdevice);
		    error = copyoutstr((caddr_t)buf,
			(caddr_t)uap->buffer, BOOTDEVLEN, (int*)0);
		    *retval = 1;
		}
		break;

	      case GSI_BOOTEDFILE:	
		if( (int)uap->nbytes < BOOTEDFILELEN ) 
		    error = EINVAL;
		else {
		    error = copyoutstr((caddr_t)bootedfile,
			(caddr_t)uap->buffer, BOOTEDFILELEN, (int*)0);
		    *retval = 1;
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

	      case GSI_LURT:		/* get contents of HWRPB Lurt structure */



	       /* If this release of the OS does not know the licensing info
		* in the systems LURT tables that are contained in the user
		* process lmf() then that process may call into the kernel for the
		* LURT information.  The kernel will obtain it from the HWRPB
		* as its set up by the console.  This routine expects a pointer
		* to an array of u_longs which has enough elements to hold all
		* the LURT entries found in the HWRPB and a count of the elements
		* to insure the kernel stays within the bounds of the struct.
		* 
		* If the LURT information is not in the HWRPB all zeros will be returned.
		*/

	       {
		       struct rpb_dsr *dsr;
		       extern struct rpb *rpb;
		       u_long  *lurt_data;
		       long   lurt_count;
		       int cnt;
		       u_long *a_word;
		       a_word = (u_long *)uap->buffer;

		       /* If the rpb_vers is not 5 or greater the rpb
			* is old and does not have this data in it.
			*/

		       if(rpb->rpb_vers < 5 ){
			       for (cnt=0,lurt_count=0;
				    cnt < (uap->nbytes / sizeof(long));
				    cnt++, a_word++)
			       {
				       /* Zero the buffer a word at a time */
				       copyout((caddr_t)&lurt_count,
					       (caddr_t)a_word,
					       sizeof(long));
				       error = EINVAL;
			       }
		       } else {
			       /* The Dynamic System Recognision struct
				* has the system smm as the first element.
				*/
			       dsr =  ((struct rpb_dsr *) ((char *)rpb
				      + (rpb->rpb_dsr_off)));
			       lurt_data = (u_long *)((char *)dsr + dsr->rpb_lurt_off);
			       /* the first entry in the data table is its size */
			       lurt_count = *lurt_data;
			       /* jump past the size field to the real data of intrest */
			       lurt_data++;

			       /* Copy the lurt data out to the user area. */

			       copyout((caddr_t)lurt_data,
				       (caddr_t)uap->buffer, 
				       (lurt_count * sizeof(long))  > uap->nbytes ? 
				       uap->nbytes : 
				       (lurt_count * sizeof(long)));
			       *retval = 1;
		
		       }
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
        long *retval;
{
        register struct args {
                unsigned long   op;
                caddr_t         buffer;
                unsigned long   nbytes;
                caddr_t         arg;
                unsigned long   flag;
        } *uap = (struct args *) args;
	
	register caddr_t        p;
	int error = 0;

	/* Add platform specific setsysinfo() cases here */

	switch(uap->op) {

		case SSI_PROM_ENV:
		    {
			char buf[MAX_ENVIRON_LENGTH+1], command[16];
#if     SEC_BASE
        		if (!privileged(SEC_SYSATTR, 0))
			     error = EPERM;
			     break;
#else
        		if (suser(u.u_cred, &u.u_acflag)) {
			     uprintf("setsysinfo: Sorry, must be super-user\n");
			     error = EPERM;
			     break;
       			}
#endif
			if (error = copyinstr(uap->arg, command,
						sizeof(command), (int*)0))
			     break;
			
			if (error = copyinstr(uap->buffer, buf,
						sizeof(buf), (int*)0))
			     break;
			
			if (prom_setenv(command, buf) != (int)0)
			     error = EINVAL;
			break;
		    }
		case SSI_IEEE_FP_CONTROL:
		    {
			long temp;
			if (error = copyin(uap->buffer, &temp, sizeof(temp))) {
				break;
			}
			ieee_set_fp_control(temp);
			break;
		    }
		case SSI_IEEE_STATE_AT_SIGNAL:
		    {
			long arglst[2];
			if (error = copyin(uap->buffer, arglst,
							sizeof(arglst))) {
				break;
			}
			ieee_set_state_at_signal(arglst[0], arglst[1]);
			break;
		    }
		case SSI_IEEE_IGNORE_STATE_AT_SIGNAL:
			ieee_ignore_state_at_signal();
			break;
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
	long	name;
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

/*
 * The boot device string for ALPHA platforms is in the following format:
 *
 * for RUBY and COBRA
 *
 *   protocol hose slot channel remote_addr unit boot_dev_type ctlr_dev_type
 *
 * for FLAMINGO
 *
 *   protocol tc_num slot channel remote_addr unit boot_dev_type ctlr_dev_type
 */
struct protocol_struct 
{
    char *protocol_name;
    char *disk_name;
} protocols[] = 
{
    { "mscp", "ra", },
    { "scsi", "rz", },
    { "dssi", "rz", },
    { "mop",  "MOP", },
    { "bootp",  "BOOTP", },
    { "network",  "NETWORK", },
    { "MSCP", "ra", },
    { "SCSI", "rz", },
    { "DSSI", "rz", },
    { "MOP",  "MOP", },
    { "BOOTP",  "BOOTP", },
    { "NETWORK",  "NETWORK", },
    { (char *)0, (char *)0, },
};

extern char *atob();
extern struct cpusw *cpup;

int
bootdevice_parser(str, dev)
char *str, *dev;
{
	struct protocol_struct *pp;
	char buffer[BOOTDEVLEN];
	int field_no, ctlr_no, unit_no, disk_no, hose_no, slot_no, boot_type=0;
	int bus_no;
	char *field[8], **fieldp, *cp, *devp;
#define protocol	field[0]
#define hose_f		field[1]
#define slot_f		field[2]
#define channel		field[3]
#define remote_addr	field[4]
#define unit		field[5]
#define boot_dev_type	field[6]
#define ctlr_dev_type	field[7]

	/*
	 * Check the sanity of the boot string
	 */
	if( !str || !(*str)) 
	{
#ifdef DEBUG_PRINT_ERRORS
printf("bootdevice_parser: input string null or empty\n");
#endif /* DEBUG_PRINT_ERRORS */
		return(0);
	}
	
	/*
	 * Now process the boot string...
	 */
#ifdef DEBUG_PRINT_INPUT
printf("bootdevice_parser: input string '%s'\n", str);
#endif /* DEBUG_PRINT_INPUT */

	strcpy(buffer, str);
	cp = buffer;

	bzero(dev, 8);				/* set 1st 8 chars to \0 */
	
	/* set the fields to a known value */
	for (field_no = 0; field_no < 8; field_no++) field[field_no] = NULL;

	field_no = 0;
	fieldp = &field[0];
	
	while (*cp && field_no < 8) 
	{
		while (*cp && *cp == ' ') cp++;	/* skip over any blanks */
		if (!*cp) break;		/* end of string - end scan */

		*fieldp++ = cp++;		/* set next field pointer */
		field_no++;			/* bump field number */
		
		while (*cp && *cp != ' ') cp++;	/* skip to next blank or end */
		if (!*cp) break;		/* end of string - end scan */

		*cp++ = '\0';		/* put null byte on blank */
					/* this effectively ends prev field */
	}

	if (field_no < 6) {
#ifdef DEBUG_PRINT_ERRORS
printf("bootdevice_parser: not enough fields in input (%d)\n", field_no);
#endif /* DEBUG_PRINT_ERRORS */
		return(0); /* error, not enough fields present */
	}

	/*
	 * Interpret the boot string values, and set the
	 *  device string with appropriate value(s)
	 */
	for (pp = protocols; pp->protocol_name; pp++) 
		if (strcmp(pp->protocol_name, protocol) == 0)
			break;

	if (!pp->protocol_name)
	{
#ifdef DEBUG_PRINT_ERRORS
printf("bootdevice_parser: unknown protocol name '%s'\n", protocol);
#endif /* DEBUG_PRINT_ERRORS */
		return(0);
	}
	
	devp = dev;
	strcpy(devp, pp->disk_name); /* copy in name */
	devp += strlen(pp->disk_name);

	/*
	 * for MSCP, we only need to set the disk type name and unit number
	 */
	if (!strcmp(pp->disk_name, "ra")) 
	{
		strcpy(devp, unit); /* add on unit number(s) */ 
	}

	/*
	 * for SCSI, we need to calculate the unit number
	 * from the channel and unit fields for FLAMINGO/RUBY;
	 * for COBRA, the slot and unit fields are used.
	 */
	else if (!strcmp(pp->disk_name, "rz")) 
	{
		ctlr_no = unit_no = 0;
		switch (cpup->system_type) {
		  case DEC_4000:
		    cp = atob(slot_f, &ctlr_no); /* from slot */
		    break;

		  case DEC_7000:
		    /*
		     * Ruby has the problem that channel number isn't 
		     * enough to define the disk name. There may be an
		     * arbitrary number of controllers, but the only
		     * information available in the console supplied
		     * boot string is slot number.  Cobra will have 
		     * this problem also when Fbus support is added.
		     */
		    cp = atob(slot_f, &slot_no);
		    cp = atob(hose_f, &hose_no);
		    cp = atob(channel, &bus_no);
		    ctlr_no=get_adapter("xza",hose_no,slot_no,bus_no);	
		    break;

		  case DEC_3000_500:
		  case DEC_3000_300:
		    cp = atob(slot_f, &slot_no);
		    cp = atob(channel, &bus_no); /* from channel */
		    ctlr_no = get_tc_adapter("tcds",slot_no,bus_no);
		    break;

		  case DEC_2000_300:
		    /*
		     * For kn121 systems,  the channel number field
		     * is logically the same as our notion of 
		     * controller. SCSIA is the 1st scsi controller,
		     * SCSIB, the second, and so on. In the boot string
		     * channel A = 0, B = 1, and so on.
		     *
		     * Therefore all that is needed to build the disk
		     * unit number is the channel field which = ctrl_no.
		     * Still read the slof_f to advance the pointer.
		     */
 		    cp = atob(slot_f, &slot_no);   /* Not Used */ 
 		    cp = atob(channel, &ctlr_no);  /* channel = ctlr*/
		    break;

		  default:
		    panic("bootdevice_parser: processor type not configured\n");
		    break;
		}

		cp = atob(unit, &unit_no);
		if (boot_dev_type && (cpup->system_type != DEC_7000))
		{
			 /* check for field present */
			cp = atob(boot_dev_type, &boot_type);
			if (boot_type != 0) *dev = 't'; /* change to tape!! */
		}
		if (unit_no >= 100) unit_no /= 100;
		disk_no = (ctlr_no * 8) + unit_no;
		cp = devp;
		if (disk_no >= 100) 
		{
			*cp++ = '0' + (disk_no / 100);
			disk_no %= 100;
		}
		if (disk_no >= 10 || cp != devp) 
		{
			*cp++ = '0' + (disk_no / 10);
			disk_no %= 10;
		}
		*cp++ = '0' + disk_no;
		*cp = '\0';
	}
	/*
	 * for all others, just copy the remaining fields into the
	 * translation buffer from the input buffer
	 */
	else 
	{
		strcpy(devp, str + strlen(pp->protocol_name));
	}
	return(1);
}

/*
 * Track down the bus structures, matching the controller we booted from.
 * Then return the controller number from the ctlr structure. (4 levels)
 *
 * sys_bus-> bus -> bus -> bus -> controller
 *
 * such as lsb->iop->xmi->xza->skz
*/
int
get_adapter(ctlr_na, hose_no, slot_no,bus_no)
	char	*ctlr_na;
	int	hose_no, slot_no, bus_no;
{
	extern struct bus *system_bus;
	struct bus *ioppnt,*xmipnt,*xzapnt;
	struct controller *ctlr;

#ifdef DEBUG_PRINT_INPUT
printf("get_adapter: Looking for adapter '%s', hose = %d, slot = %d\n", ctlr_na, hose_no, slot_no);
#endif /* DEBUG_PRINT_INPUT */
	for(ioppnt = system_bus->bus_list; ioppnt != (struct bus *)0 ;
					ioppnt = ioppnt->nxt_bus) {
		for(xmipnt = ioppnt->bus_list; xmipnt!=(struct bus *)0;
						xmipnt = xmipnt->nxt_bus) {
			if(xmipnt->bus_num == hose_no) {
				for(xzapnt = xmipnt->bus_list ;
						xzapnt!=(struct bus *)0;
						xzapnt = xzapnt->nxt_bus) {
					if((*xzapnt->bus_name == *ctlr_na) &&
						(xzapnt->slot == slot_no)){
						for(ctlr = xzapnt->ctlr_list ;
						ctlr != (struct controller*)0 ;
						ctlr = ctlr->nxt_ctlr) {
							if(ctlr->slot==bus_no)
							return(ctlr->ctlr_num);
						}
					}
				}
			}
		}
	}
}

/*
 * Track down the bus structures, matching the controller we booted from.
 * Then return the controller number from the ctlr structure. (2 levels)
 *
 * sys_bus -> bus -> controller
 *
 * such as tc->tcds->asc
 */
int
get_tc_adapter(ctlr_na, slot_no, bus_no)
    char *ctlr_na;
    int slot_no, bus_no;
{
    extern struct bus *system_bus;
    struct bus *tcpnt;
    struct controller *ctlr;

#ifdef DEBUG_PRINT_INPUT
printf("get_tc_adapter: Looking for '%s', slot = %d\n", ctlr_na, slot_no);
#endif /* DEBUG_PRINT_INPUT */

    for(tcpnt = system_bus->bus_list; tcpnt != (struct bus *)0 ;
				tcpnt = tcpnt->nxt_bus) {
	if((strncmp(tcpnt->bus_name, ctlr_na, 4) == 0) &&
				(tcpnt->slot == slot_no)){
	    for(ctlr = tcpnt->ctlr_list ; ctlr != (struct controller*)0 ;
				ctlr = ctlr->nxt_ctlr) {
		if(ctlr->slot==bus_no) return(ctlr->ctlr_num);
	    }
	}
    }
    /* if we can't find the specified bus, assume a tc controller */
    return( tc_get_unit_num(slot_no) );
}
