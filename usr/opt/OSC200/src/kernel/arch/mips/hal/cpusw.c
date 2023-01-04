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
static char *rcsid = "@(#)$RCSfile: cpusw.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/10/13 12:07:38 $";
#endif

/*
 * Some old history:
 *
 * 15-Jun-1989  afd
 *      Added cpu_initialize routine to call system specific
 *	initialization routine (for splm, intr vectors, hz, etc).
 *
 */



/*
 * The following routines are the ones used by DEC's HAL
 * that are implemented via a cpu switch table.
 *
 * Most of these have been pulled from machdep.c for hw indep.
 *
 */

#include <machine/cpu.h>
#include <machine/machparam.h>	/* PGSHIFT, NBPG */
#include <sys/vmmac.h>		/* btop, ptob */
#include <sys/types.h>
#include <hal/cpuconf.h>
#include <hal/entrypt.h>

#include <io/common/devdriver.h>

int	cpu;	            /* System type, value defined in cpuconf.h */
struct	cpusw	*cpup;      /* pointer to cpusw entry */
unsigned cpu_systype;       /* systype word in boot PROM */
int     cpu_subtype;        /* value defined in cpuconf.h */

struct cpusw *cpuswitch_entry();

int     qmapbase;           	/* Physical address of Qbus map regs */
struct  mem_bitmap  rex_map[1]; /* allocate a mem_bitmap structure */

extern  int  rex_base;      /* Base of ROM Executive callback table
				used by DS5000 systems */
extern  int  rex_magicid;   /* Console type id */

int	console_magic;	    /* console id */

char    bootctlr[2];
char    consmagic[5];

extern  int  nocpu();

/*
 * "Machine check" error handler call;
 * Called from trap().
 *
 */
machine_check(ep, code, sr, cause, signo)
        int *ep;		/* exception frame ptr */
        int code;		/* trap code (trap type) */
        int sr, cause;		/* status and cause regs */
	int *signo;		/* set to signo if want to kill process */
{
	if (cpup->machcheck == nocpu)
		panic("no trap error routine configured");
	else
		return((*(cpup->machcheck))(ep, code, sr, cause, signo));
}


/*
 * Call machine dependent code to handle machine specific operations
 */
machine_configure()
{
	if (cpup->config == nocpu)
                panic("No configuration routine configured\n");
	else
		return((*(cpup->config))()); 
}

                                                                       
/*
 * DELAY(n) should be n microseconds, roughly. 
 */
DELAY(n)
	int n;
{
	microdelay(n);
}


read_todclk()
{
        if (cpup->readtodr == nocpu)
                panic("No read TOD routine configured\n");
        else
                return((*(cpup->readtodr))());
}


write_todclk(yrtime)
long    yrtime;
{
        if (cpup->writetodr == nocpu)
                panic("No write TOD routine configured\n");
        else
                return((*(cpup->writetodr))(yrtime));
}


/*
 * Machine dependent badaddr.
 *
 * On machines such as on the BI, access to NXM in BI does not
 * necessarily cause error interrupts or exceptions.  The only
 * error indication may be the setting of error bits in error registers.
 * NB:	ptr is a pointer to a bus or controller struct and 
 *	is used by machines that access io space via mailboxes
 *	currently Alpha AXP systems only
 */
badaddr(addr,len, ptr)
caddr_t addr;
int len;
struct  bus_ctlr_common *ptr;
{
        int status, s;
        extern int cold;        /* booting in progress */


        if (cold)
                status =  (*(cpup->badaddr))(addr,len, ptr);
        else {
                s = splhigh();  /* Disable interrupts */
                switch(cpu) {
                case DS_3100:
                case DS_5000:
                case DS_5000_100:
                case DS_MAXINE:
                case DS_5000_300:
                case DS_5100:
                case DS_5400:
                case DS_5500:
                case DS_5800:
                default:
                        status = (*(cpup->badaddr))(addr,len, ptr);
                        break;
                }
                splx(s);        /* Restore interrupts */
        }
        return(status);
}
                                  
/* 
 * wrapper routine to badaddr(); no real use/reason
 * except lots of historical use.
 * NB:	ptr is a pointer to a bus or controller struct and 
 *	is used by machines that access io space via mailboxes
 *	currently Alpha AXP systems only
 */
BADADDR(addr,len, ptr)
	caddr_t	addr;
	int	len;
	struct  bus_ctlr_common *ptr;
{
	return(badaddr(addr,len, ptr));
}


startrtclock()
{
        if ((*(cpup->startclocks))() < 0)
                panic("No start clock routine configured\n");
}


stopclocks()
{
        if ((*(cpup->stopclocks))() < 0)
                panic("No stop clock routine configured\n");
}


/*
 * Call system specific initialization routine 
 * (for splm, intr vectors, etc).
 */
cpu_initialize(argc, argv, environ, vector)
int argc;
char *argv[];
char *environ[];
char *vector[];
{

        /*
         * Get the "systype" word from the boot PROM, which contains
         * the processor type, the system implementation type, firmware
         * rev and hardware rev.
         *
         * Also set "cpu" (the ULTRIX system type) based on both
         * processor type and system implementation type.
         *
         * Initialize "cpup" to point to the cpusw table entry for the
	 * system that we are currently running on.
         *
         * As soon as we have "cpup" set, call the cpu initialization
	 * routine to fill in the splm, interupt vector table, 
	 * ipl mask, etc.
	 *
         * This MUST be done before any spl routines can be called 
	 * (thus before any printfs can be done).
         */

	console_magic = (int)environ;   /* save console id */
        if(console_magic == REX_MAGIC) {
                rex_base = (int)vector;
                rex_magicid = (int)environ;
                cpu_systype = rex_getsystype();
        } else {
                rex_base = 0;
                rex_magicid = 0;
                cpu_systype = xtob(prom_getenv("systype"));
        }
        cpu = system_type(cpu_systype);
        cpup = cpuswitch_entry(cpu);
        cpu_subtype = system_subtype(cpu);

        if ((*(cpup->init))() < 0)
                panic("No initialization routine configured\n");

        /* 'cpu' should be setup before calling init_boot_cpu */
	/* TODO:        init_boot_cpu(); */

}


/*
 * Determine what we are running on and return the system type.
 * To be used as the index into the cpu switch (system specific 
 * switch table).
 *
 * Parameter:
 *      cpu_systype             Entire system type work from the PROM
 *
 * Return:
 *      Value to be stored in cpu, defined in cpuconf.h
 */
system_type(cpu_systype)
        unsigned  cpu_systype;
{
        int ret_val = UNKN_SYSTEM;      /* Assume we don't know yet */

        switch (GETCPUTYPE(cpu_systype)) {
        case R3000_CPU:
        /* case R2000a_CPU: */
                switch (GETSYSTYPE(cpu_systype)) {
                case ST_DS3100:
                        ret_val = DS_3100;
                        break;
                case ST_DS5000:
                        ret_val = DS_5000;
                        break;
                case ST_DS5000_100:
                        ret_val = DS_5000_100;
                        break;
                case ST_DSMAXINE:
                        ret_val = DS_MAXINE;
                        break;
                case ST_DS5000_300:
                        ret_val = DS_5000_300;
                        break;
                case ST_DS5100:
                        ret_val = DS_5100;
                        break;
                case ST_DS5400:
                        ret_val = DS_5400;
                        break;
                case ST_DS5500:
                        ret_val = DS_5500;
                        break;
                case ST_DS5800:
                        ret_val = DS_5800;
                        break;
                }
                break;
        }
        return(ret_val);
}


/*
 * If processor design is shared, return global definition of system
 * subtype which can be used to differentiate between systems 
 * with identical systype values, but they may have system or product 
 * specific functionality.
 *
 * Parameter:
 *      cpu             cpu type derrived from prom systype word
 *
 * Return:
 *      Value to be stored in cpu_subtype, defined in cpuconf.h
 */
system_subtype(cpu)
        int  cpu;
{
        int ret_val = NO_CPU_VARIANT;   /* Assume no subtype */

        switch (cpu) {

          case DS_5000_300:
            ret_val = get_kn03_subtype();
            break;

        }

        return(ret_val);
}


/*
 * Get pointer to cpusw table entry for the system we are currently
 * running on.  The pointer returned by this routine will go into 
 * "cpup".
 *
 * The "cpu" variable (DEC-UNIX system type) is passed in and compared 
 * to the system_type entry in the cpusw table for a match.
 */
struct cpusw *
cpuswitch_entry(cpu)
        int cpu;                        /* the DEC-UNIX system type */
{
        register int i;                 /* loop index */

        for (i = 0; cpusw[i].system_type != 0; i++) {
                if (cpusw[i].system_type == cpu)
                        return(&cpusw[i]);
        }
        panic("processor type not configured");
}
                                                 

/*
 * machine dependent memory sizer
 */
machine_memsize(fpage)
unsigned int fpage;
{
	int i;

	if ((i = (*(cpup->memsize))(fpage)) <= 0)
                panic("No memory sizing routine configured\n");

        if ((cpu == DS_5400) || ( cpu == DS_5500)) {
                /*
                 * On a Q bus, we need to grab 32K bytes of memory
                 * for the Qbus map registers.  Here we simply use
                 * the last 32Kb of physical memory.  Note that the Qbus
                * map must be on a 32Kb boundary.
                 * This causes us to lose at least 32Kb of memory.
                 */
                i -= (i % 8);   /* make sure we'on 32Kb boundary */
                i -= 8;         /* lose the last 32Kb            */
                qmapbase = i * 4096;
        }
	return(i);
}



/*
 * Size memory by walking thru memory invoking the BADADDR macro,
 * which calls a processor specific badaddr routine.
 */
msize_baddr(fpage)
{
        register int i;
        extern VEC_nofault(), VEC_trap();
        extern  (*causevec[])();

        causevec[EXC_DBE>>CAUSE_EXCSHIFT] = VEC_nofault;
        for (i = fpage; i < btop(K0SIZE); i++) {
                if (BADADDR(PHYS_TO_K1((unsigned)ptob(i)),4, 0))
                        break;
                clearseg(i);
        }
        causevec[EXC_DBE>>CAUSE_EXCSHIFT] = VEC_trap;
        return(i);
}
                        

/*
 * Size memory by using the bitmap.
 */
msize_bitmap(fpage)
{
        register int i,j;
        int *bitmap, memsize, start = 0;
        u_int bitmaplen;
        struct mem_bitmap *mbmp;

        if(console_magic == REX_MAGIC) {
                mbmp = rex_map;
                /* rex_getbitmap returns number of bytes;       */
		/* the algorithm below uses number of longwords */
                bitmaplen = rex_getbitmap(rex_map) / 4;
                if (mbmp->mbmap_pagesize != NBPG)
                    rex_printf("msize_bitmap: bitmap page size not NBPG \n");
                bitmap = (int *)mbmp->mbmap_bitmap;
        } else {
                bitmap = (int *)xtob((char *)prom_getenv("bitmap"));
                bitmaplen = xtob(prom_getenv("bitmaplen"));
        }
        memsize = 0;
        /* if DS_5000, rom is returning the first and third 
	   64k chunks as bad */
        /* assume they are good */
        if (cpu == DS_5000) {
            bitmap += 2;
            memsize += 64;
            /* start at the second longword, 
	       since we hardwired the first two */
            start = 2; 
        }
        for (i = start; i < bitmaplen; i++, bitmap++) {
                if (*bitmap == 0xffffffff) {
                        memsize += 32;
                       continue;
                } else {
                        for (j = 0; j < 32; j++) {
                                if (*bitmap & (1 << j))
                                        memsize += 1;
                                else
                                        break;
                        }
                        break;
                }
        }
        if ((cpu == DS_5800) || (cpu == DS_5500) || (cpu == DS_5100)) {
                /*
                 * Bitmap page representation is 512 byte pages,
                 * so convert to natural machine page size.
                 *
                 * Take the number of (512-size) pages * 512 to get the
                 * total memory size, then divide by 4k by shifting
                 * right 12, to get the number of 4k pages available.
                 * This drops the remainder, hence leaving any partial
                 * page as unusable. This corrects the problem of
                 * marking bitmaps which don't fall on 4k boundries
                 * as unusable which was caused by using btoc().
                 *
                 */
                memsize = (memsize * 512) >> PGSHIFT ;
        }
        for (i = fpage; i < memsize; i++) {
                clearseg(i);
        }
        return(memsize);
}



halt_cpu()
{
        printf("\nHalting cpu... (transfer to monitor)\n\n");
        prom_restart();
        for (;;) /* not reached */
             ;
}
