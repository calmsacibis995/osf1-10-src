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
static char *rcsid = "@(#)$RCSfile: console.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/07/08 08:38:16 $";
#endif

/*
 * Generic console interface routines used by rest of
 * the kernel.  The following presents a hardware- (console-)
 * independent interface to the rest of the kernel.
 * These functions are part of a system's HAL.
 */

#include <sys/types.h>
#include <sys/reboot.h>

/* When boot() is clean of file system functionality
 * this next group of includes can be taken out 
 */
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <sys/mount.h>
#include <ufs/ufsmount.h>
#include <ufs/fs.h>

#include <hal/entrypt.h>
#include <hal/cpuconf.h>

#include <io/common/devdriver.h>
#include <io/dec/tc/tc.h>

extern  int  console_magic;
extern  int  rex_base;
extern	int  cpu;

console_putchar(c)
	int c;
{
	if(rex_base) {
		rex_printf("%c",c);
	}
	else
		prom_putchar(c);
}

console_restart()
{
	if(console_magic != REX_MAGIC)
		prom_restart(); /* always enters command mode */
	else {
		rex_rex('h');
	}
	/* doesn't return */
}

console_reboot()
{
	if(console_magic != REX_MAGIC)
		prom_reboot();  /* follows $bootmode */
	else {
		rex_rex('r');
	}
	/* doesn't return */
}

console_boot()
{
        if(console_magic != REX_MAGIC)
                prom_autoboot();        /* always reboots */
        else {
                rex_rex('b');
        }
        /* doesn't return */
}

/*
 * TODO: HAL
 *     : should put this in cpu switch table	
 *
 * The following only works for the generic kernel;
 * custom kernels get undefined references 
 *
 * TODO: change the knxx_machineid() routines to
 *	 access enet-address in system proms directly;
 * 	 removes dependency on ifnet structure and 
 *	 (proper) config of network device.
 *
 * Better yet, have the per-system cpu_initialize()
 * code get the value on startup, and save it in
 * a global variable that gets returned in this routine.
 * A whole lot simpler & faster.
 *
 */
u_int
console_getsystemid()
{
	u_int	id;
	
	id = 52759;	/* HAL: for now */
/*
	switch(cpu) {
	case DS_5000:
		id = kn02_machineid();
		break;
	case DS_5000_300:
		id = kn03_machineid();
		break;
	case DS_5100:
		id = kn230_machineid();
		break;
	case DS_5500:
		id = kn220_machineid();
		break;
	case DS_3100:
	case DS_5000_100:
	case DS_MAXINE:
	case DS_5400:
	case DS_5800:
	default:
		printf("Machine does not implement getsystemid \n");
		break;
	}
 */
	return(id);
}


/*
 * Routine to convert TURBOchannel boot slot into 
 * logical controller number.
 */
getbootctlr()
{
  extern char **ub_argv;
  extern char bootctlr[];
  extern char consmagic[];
  extern struct tc_slot tc_slot[];
  char *cp;
  int i;

  bootctlr[0] = '\0';
  cp = (char *)&console_magic;
  for(i=0;i<4;i++)
        consmagic[i] = *cp++;
  consmagic[4]='\0';
  if(!rex_base) {
    return(0);
  }
  else {
    cp = (char *)&ub_argv[1][0];
    while(*cp != '/')
      cp++;
    cp--;
    if(((strncmp(cp+2,"rz",2)==0)) || (strncmp(cp+2,"tz",2) == 0)) {
      for(i=0;i<=8;i++) {
        if((strcmp(tc_slot[i].devname,"asc")==0) &&
           (tc_slot[i].slot == *cp - '0')) {
          bootctlr[0] = tc_slot[i].unit + '0';
          continue;
        }
      }
    } else {
        if((strncmp(cp+2,"mop",3)==0) ||
           (strncmp(cp+2,"tftp",4)==0)) {
                for(i=0;i<=8;i++) {
                        if(((strcmp(tc_slot[i].devname,"ln")==0) ||
                           (strcmp(tc_slot[i].devname,"fza")==0)) &&
                           (tc_slot[i].slot == *cp - '0')) {
                                bootctlr[0] = tc_slot[i].unit + '0';
                                continue;
                        }
                }
        }
    }

    return(0);
  }
}

/*
 * Generalized interface for netbootchk to use to
 * get the boot device in the boot string
 */
char *
getbootdev()
{
	char *bootdev;
	extern char **ub_argv;
	extern int ub_argc;
	int i;

	if(console_magic != REX_MAGIC)
		bootdev = (char *)prom_getenv("boot");
	
	else {	/* REX console parse */
		for(i=1; i < ub_argc; i++) {
			if((ub_argv[i][0] != '-') && (ub_argv[i][0] != NULL))
				if(ub_argv[i][1] == '/')
					bootdev = (char *)ub_argv[i];
		}
	}
	return(bootdev);
}


/*
 * Generalized interface for netbootchk to use to
 * get the type of device network boot is being done
 * over ( ln, ne, fza, etc.).
 */

getboottype(boottype, bootdev)
	char *boottype;
	char *bootdev;
{
	int j,k;

	switch (cpu) {
		case DS_5000:
		case DS_5000_100:
		case DS_MAXINE:
		case DS_5000_300:
			if (console_magic != REX_MAGIC) {
                        /*
                         * Walk the TURBOchannel looking for the nth
                         * instance of a LANCE or DEFZA.
                         */
			  if ((bootdev[4] > '0') && (bootdev[4] <= '3')) {
				int k, unit = bootdev[4] - '0';
                                extern struct tc_slot tc_slot[];
                                for (k = 0; k < 3; k++) {
                                  if (!strcmp(tc_slot[k].devname,"ln"))
                                    if (--unit == 0) {
                                      bcopy("ln",boottype,2);
                                      boottype[2] = tc_slot[k].unit+'0';
                                      boottype[3] = '\0';
                                      break;
                                    }
                                  if (!strcmp(tc_slot[k].devname,"fza"))
                                   if (--unit == 0) {
                                      bcopy("fza",boottype,3);
                                      boottype[3] = tc_slot[k].unit+'0';
                                      boottype[4] = '\0';
                                      break;
                                   }
                                }

                          } else {
                          /*
                           * For unit = 0 and default, use "ln0"
                           */
                                bcopy("ln0",boottype,3);
                                boottype[3] = '\0';
                          }
                        } else { /* New TURBOchannel console */
                              extern struct tc_slot tc_slot[];
                              /*
                               * Walk the TURBOchannel looking for the
                               * nth instance of a LANCE or DEFZA.
                               */
                              j=bootdev[0]-0x30;
                              for(k = 0; k <= 6; k++) {
                                if(tc_slot[k].slot == j) {
                                  if (!strcmp(tc_slot[k].devname,"ln")) {
                                    bcopy("ln",boottype,2);
                                    boottype[2] = tc_slot[k].unit+'0';
                                    boottype[3] = '\0';
                                    break;
                                  }
                                  if (!strcmp(tc_slot[k].devname,"fza")) {
                                    bcopy("fza",boottype,3);
                                    boottype[3] = tc_slot[k].unit+'0';
                                    boottype[4] = '\0';
                                    break;
                                  }
                                }
                              } /* END FOR */
                        } /* END ELSE */
                        break;
                case DS_5500:
			bcopy("ne0",boottype,3);
                        boottype[3] = '\0';
                        break;
		case DS_3100:
		case DS_5100:
		case DS_5400:
		default:
			bcopy("ln0",boottype,3);
			boottype[3] = '\0';
			break;
	} /* end of switch */
}

/*************************************************************
 *
 * boot() is called by bsd/reboot() to bring the system
 * down, and potentially re-boot it.
 *
 *************************************************************/

/*
 * Some old history:
 *
 * 08-Aug-91    dws
 *      Unmount UFS filesystems on a clean shutdown.
 *
 * 29-Mar-90 gmm
 *      Changed some splhigh() to splextreme() since splhigh() now same
 *      as splclock()
 *
 * 25 Jul 89 -- chet
 *      Change unmount and cache flushing code in boot()
 *
 * 19-Jun-89 -- condylis
 *      Tightened up unmounting of file systems in boot().
 *
 * 15-June-1989 kong
 *      Changed code in boot to work around (not fix) a panic hang,
 *      see the comments in "boot".
 *
 * 06-Apr-89 -- prs
 *      Added SMP accounting lock in boot().
 *
 */

int     waittime = -1;
int     shutting_down = 0;

struct saved_tlbs {
	union tlb_hi tlb_high;
	union tlb_lo tlb_low;
} saved_tlbs[NTLBENTRIES];

boot(paniced, arghowto)
        int paniced, arghowto;
{
        register int howto;
        register int devtype;

	save_tlb(saved_tlbs); 

        /*
         * "shutting_down" is used by device drivers to determine
         * that the system is shutting down.
         */
        shutting_down = 1;
        
	if(cpu != DS_5500)
        	netnuke(0);     /* no more ether packets, thanks. */
        howto = arghowto;
        if ((howto&RB_NOSYNC)==0 && waittime < 0 && bfreelist[0].b_forw) {
                /*
                 *  Force an accurate time into the root file system 
		 *  super - block.
                 */
                mounttab[0].um_fs->fs_fmod = 1;         /* XXX */
                waittime = 0;
                /*
                 * Unmount ufs filesystems
                 */
                if (paniced != RB_PANIC) {
                        extern int nmount;
                        struct ufsmount *ump;

                        for (ump = &mounttab[nmount-1]; ump > &mounttab[0]; ump--) {
                                if (ump->um_mountp == DEADMOUNT ||
                                    ump->um_mountp == NULLMOUNT ||
                                    ump->um_mountp == rootfs)
                                        continue;
                                (void) dounmount((struct vnode *)0, ump->um_mountp, MNT_FORCE);
                        }
                }
                (void) splnet();        /* block software interrupts */
                printf("syncing disks... ");

                sync((struct proc *)NULL, (void *)NULL, (int *)NULL);
                wbflush();
                { register struct buf *bp;
                  int iter, nbusy;
                  int obusy = 0;

                  for (iter = 0; iter < 20; iter++) {
                        nbusy = 0;
                        for (bp = &buf[nbuf]; --bp >= buf; )
                          if ((bp->b_flags & (B_BUSY|B_INVAL)) == B_BUSY)
                                        nbusy++;
                        if (nbusy == 0)
                                break;
                        printf("%d ", nbusy);

                        if (nbusy != obusy)
                                iter = 0;
                        obusy = nbusy;

                        DELAY(40000 * iter);
                  }
                  if (nbusy)
                        printf("failed\n");
                  else
                        printf("done\n");
                }
                /*
                 * If we've been adjusting the clock, the todr
                 * will be out of synch; adjust it now.
                 */
                resettodr();
        }
        (void) splhigh();                       /* extreme priority */
        (void) save_context_all();
        devtype = major(rootdev);
        if (howto&RB_HALT) {
                printf("halting.... (transferring to monitor)\n\n");
                console_restart();  /* always enters command mode */
                /* doesn't return */
        } else {
                if (paniced == RB_PANIC) {
                        dumpsys();
                        console_reboot();       /* follows $bootmode */
                        /* doesn't return */
                }
        }
        printf("rebooting.... (transferring to monitor)\n\n");
        console_boot();                 /* always reboots */
        /* doesn't return */
        for (;;)        /* chicken */
                ;
}

