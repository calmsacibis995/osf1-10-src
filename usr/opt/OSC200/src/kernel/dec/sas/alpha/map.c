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
/*
 * SIMULATOR Environment mapping routines
 *
 * 25-Oct-90 -- rjl
 *
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <machine/pte.h>
#include <machine/rpb.h>
#include <machine/pcb.h>

#define PGSIZE 8192
#define NPTEPG 1024
#define KSEGBASE 0xfffffc0000000000
#define KSEGTOP  0xfffffe0000000000

/* #define DEBUG */

/*
 * boot_vtop
 *
 * translate virtual address to physical address:
 * 	virtual addr -> pte -> pfn -> physical address
 *
 */
long
boot_vtop(vaddr)
long vaddr;
{
	static long pgshift = 0;
	struct pte *pptr;		/* pointer to the pagetables	*/
	long physaddr;
	extern long msb();

	if (pgshift == 0) {
		/* initialize the shift factor */
		pgshift = msb((unsigned long)PGSIZE);
	}

	/*
	 * Calculate the virtual address of the page tables entry (pte) 
	 * that maps the virtual address.  
	 */
	pptr = ((struct pte *)(PGTBL_ADDR) + ((long)(vaddr) >> pgshift));
	physaddr = ((pptr ->pg_pfnum) << pgshift ) | 
		   ((vaddr & ~(-1L << pgshift))); 
	return (physaddr);
}

/*
 * firstpfn
 *
 * find the first unused pfn
 *
 */
long
firstpfn()
{
	struct pte *pptr;		/* pointer to the pagetables	*/
	struct pte pte;

	struct rpb *rpb = (struct rpb *)HWRPB_ADDR; /* the rpb	*/
	struct rpb_mdt *md;		/* memory descriptor table */
	struct rpb_cluster *rc;		/* a memory cluster	   */
	int	lpfn;			/* my last pfn			*/
	int npfns;			/* number of pfns used for me	*/
	int i;

	extern char end;		/* end of the program		*/
	extern char start;		/* start of the program		*/


	/*
	 * calculate the address of the memory descriptor
	 * table
	 */
	md = (struct rpb_mdt *)(HWRPB_ADDR + rpb->rpb_mdt_off);
	for( i=0 ; i < md->rpb_numcl ; i++ ){
		/*
		 * Is this the first we can use?
		 */
		if( md->rpb_cluster[i].rpb_usage == 0 )
			break;
	}
	rc = &md->rpb_cluster[i];
	/*
	 * Calculate the virtual address of the page tables that
	 * map the boot program
	 */
	pptr = (struct pte *) (PGTBL_ADDR +
		(((BOOT_ADDR >> 23) & 0x3ff) * PGSIZE) +
		(((BOOT_ADDR >> 13) & 0x3ff) * sizeof(struct pte)));
	/*
	 * Find the first pte that maps us in the range specified
	 * by this descriptor
	 */
	npfns = ((&end - (char *)0x20000000) + PGSIZE)/PGSIZE;
	npfns--;
	pte = pptr[npfns];
	lpfn = pte.pg_pfnum;
	if( lpfn >= rc->rpb_pfn && lpfn < rc->rpb_pfn+rc->rpb_pfncount )
		return ++lpfn;
	else
		return -1;
}

/* 
 * boot program virtual to pfn, this routine can only be called for addresses
 * mapped by the boot page tables, it depends on the self mapping seg2 ptable
 */
long
bvtopfn(vaddr)
long vaddr;
{
	struct pte *pptr;		/* pointer to the pagetables	*/

	/*
	 * Calculate the virtual address of the page tables that
	 * map the boot program
	 */
	pptr = (struct pte *) (PGTBL_ADDR +
		(((vaddr >> 23) & 0x3ff) * PGSIZE) +
		(((vaddr >> 13) & 0x3ff) * sizeof(struct pte)));
	printf("bvtopfn:vaddr=%lx pte @ %lx\n", vaddr, pptr);
	return pptr->pg_pfnum;
}

/*
 * Setup to load the image into either mapped address space or kseg space
 * in the proper operating environment.
 *
 */
mapimage(vaddr, spfn, npfn, entry)
unsigned long vaddr, entry;
long spfn, npfn;
{
	long seg1, seg2, seg3;		/* VA segments			*/
	struct pte pte;			/* prototype pte		*/
	long tpte;			/* temp pte			*/
	static long seg1pfn;
	long seg2pfn, seg3pfn;		/* page table pfns		*/
	struct rpb *rpb = (struct rpb *)HWRPB_ADDR; /* the rpb		*/
	struct rpb_percpu *percpu;	/* ptr to per-cpu portion of HWRPB */
	struct rpb_mdt *md;		/* memory descriptor table	*/
	struct rpb_cluster *rc;		/* a memory cluster		*/
	long seg1addr, seg2addr, seg3addr; /* physical addrs		*/
	long *saddr=(long *)(spfn*PGSIZE+KSEGBASE); /* kseg address of image */
	long osf_palrev;		/* PALcode Revision for OSF1	*/
	int i;

	static struct pcb osf_pcb;

	extern long palmode;
	extern long ldqp();

	percpu = (struct rpb_percpu *) (HWRPB_ADDR + rpb->rpb_percpu_off +
		(rpb->rpb_cpuid * rpb->rpb_slotsize));

#ifdef DEBUG
	printf("RPB CPUID <0x%lx>\n", rpb->rpb_cpuid);
	printf("RPB SLOTSIZE <0x%lx>\n", rpb->rpb_slotsize);
	printf("RPB VPTB <0x%lx>\n", rpb->rpb_vptb);
	printf("RPB SELFREF <0x%lx>\n", (long)rpb->rpb_selfref);
	printf("HWRPB_ADDR <0x%lx>\n", HWRPB_ADDR);
	for (i=0;i<16;++i) {
		printf("BOOTPCB <0x%lx>\n", ((long *)(&percpu->rpb_pcb))[i]);
	}
#endif
	if (palmode != 1)		/* do only if running VMS PAL */
		seg1pfn = mfpr_ptbr();

	/*
	 * Are we copying onto ourself
	 */
	if( (unsigned long)vaddr < (unsigned long)saddr ){
		printf("Bootstrap address collision, image loading aborted\n");
		exit(1);
	}
	/*
	 * Do we have a VMS palcode kernel image, an OSF palcode kernel image
	 * or something else?
	 *
	 * Ideally the image should be typed so that it's easy to tell.
	 * but we need to use a heuristic based on knowledge of the kernel
	 * addresses.
	 * VMS PALcode kernels have a larger SCB as a result of using a
	 * shadow SCB for I/O vectoring, OSF PALcode kernels do not use
	 * the shadow.
	 */
	if( entry - vaddr < 0x6000 ) {
		/*
	  	 * Switch to osf palcode - we can only do this once!
		 */
		if (palmode == 0) {
			printf("Current PAL Revision <0x%lx>\n",
				percpu->rpb_palrev);
#ifdef DEBUG
			printf("PAL Revisions Available\n");
			for (i=0;i<16;++i)
				printf("\t%d - <0x%lx>\n",
					i, percpu->rpb_palrev_avail[i]);
#endif
			printf("Switching to OSF PALcode ");

			osf_palrev = percpu->rpb_palrev_avail[PALvar_OSF1];

			if( osf_palrev == 0 ){
				printf("Failed: PALcode Not Available\n");
			} else {
				long	hwpcb;
				long	vptb;

				hwpcb = mfpr_pcbb();
				vptb = mfpr_vptb();

#ifdef DEBUG
				printf("\nHWPCB <0x%lx>\n", hwpcb);
				printf("VPTB <0x%lx>\n", vptb);
				printf("PTBR <0x%lx>\n", seg1pfn);
#endif
				/* ksp has same position for both osf/vms */
				osf_pcb.pcb_ksp = percpu->rpb_pcb.rpb_ksp;
				osf_pcb.pcb_usp = 0L;
				osf_pcb.pcb_ptbr = mfpr_ptbr();
				osf_pcb.pcb_cc = 0;
				osf_pcb.pcb_asn = 0;
				osf_pcb.pcb_proc_uniq = 0L;
				osf_pcb.pcb_fen = 0L;
				osf_pcb.pcb_palscr[0] = 0L;
				osf_pcb.pcb_palscr[1] = 0L;

				hwpcb = boot_vtop((long)(&osf_pcb));

				switch(i = switch_env(hwpcb, vptb)){
				case 0:
					printf("Succeeded\n");
					palmode = 1;
					percpu->rpb_palrev = osf_palrev;
					break;
				case 1:
					printf("Failed: Unknown PAL Variant\n");
					palmode = 0;
					break;
				case 2:
					printf("Failed: PALcode Not Loaded\n");
					palmode = 0;
					break;
				default:
					printf("Failed: Status <0x%lx>\n",i);
					palmode = 0;
					break;
				}
			}
			printf("New PAL Revision <0x%lx>\n",
				percpu->rpb_palrev);
		}

		if( palmode == 1 ){
			if( (unsigned long)vaddr >= (unsigned long)KSEGBASE &&
				(unsigned long)vaddr < (unsigned long)KSEGTOP ){
				printf("Loading into KSEG Address Space\n");
				return ( (vaddr - KSEGBASE)/8192 + 1 + npfn);
			} 
		}
	} else
		palmode = 0;

	printf("Mapping Image Address Space\n");

	/*
	 * build the prototype pte;
	 */
	*(long *)&pte = 0;
	pte.pg_v = 1;
	pte.pg_flt_on = 0;
	pte.pg_gh = 0;
	pte.pg_prot = PROT_KW;

	/*
	 * calculate the address of the memory descriptor
	 * table
	 */
	md = (struct rpb_mdt *)(HWRPB_ADDR + rpb->rpb_mdt_off);
	for( i=0 ; i < md->rpb_numcl ; i++ ){
		/*
		 * Is this the first we can use?
		 */
		if( md->rpb_cluster[i].rpb_usage == 0 )
			break;
	}
	rc = &md->rpb_cluster[i];

	/*
	 * Allocate the segment 2 pagetable page
	 *
	 */
	seg2pfn = spfn++;
	seg3pfn = spfn++;
	if(  seg3pfn >= rc->rpb_pfn+rc->rpb_pfncount ){
		printf("Not enough memory in the cluster to map\n");
		exit(1);
	}
	/*
	 * Calculate initial segment values
	 */
	seg1 = (vaddr >> 33) & 0x3ff;
	seg2 = (vaddr >> 23) & 0x3ff;
	seg3 = (vaddr >> 13) & 0x3ff;

	/*
	 * Get the seg1 pte and check that it doesn't already point
	 * to a page
	 */
	/*
	 * Using struct pte and the appropriate operations fail here.
	 * Passing of structures is done on the stack and ldqp returns
	 * in registers.
	 */
	seg1addr = seg1pfn * PGSIZE;
	tpte = ldqp( seg1addr+seg1*sizeof(struct pte));
	if( tpte ){
		printf("Secondlevel page table for vaddr=%lx already used\n",
			vaddr);
		seg2pfn = ( tpte >> 32 ) & 0xffffffff;
	} else {
		/*
		 * set the level 1 pte
		 */
		pte.pg_pfnum = seg2pfn;
		stqp( seg1addr+seg1*sizeof(struct pte), pte);
	}

	/*
	 * Check the level 2 pte to make sure it's not in use
	 */
	seg2addr = seg2pfn * PGSIZE;
#ifdef flamingo_found_problem
	tpte = ldqp( seg2addr+seg2*sizeof(struct pte));
	if( tpte ){
		printf("Third level page table for vaddr=%lx already used\n",
			vaddr);
		exit(1);
	} else {
		/*
		 * build the second level page table
		 */
		pte.pg_pfnum = seg3pfn;
		stqp( seg2addr+seg2*sizeof(struct pte), pte );
	}
	
#endif

        for( i=0 ; i<NPTEPG ; i++ )
                stqp( seg2addr + i * sizeof(struct pte), 0L);

                pte.pg_pfnum = seg3pfn;
                stqp( seg2addr+seg2*sizeof(struct pte), pte );


	seg3addr = seg3pfn * PGSIZE;
        for( i=0 ; i<NPTEPG ; i++ )
                stqp( seg3addr + i * sizeof(struct pte), 0L);


	/*
	 * Rather than map the 2nd and 3rd level page tables I'll
	 * address then physically, this is easier for now.
	 */

	/*
	 * build the third level page table
	 */
	for( i=0 ; i<npfn ; i++, seg3++ ){
		if( seg3 > 1023 ){
			printf("overflowed third level page table\n");
			exit(1);
		}
		pte.pg_pfnum = spfn++;
		stqp( seg3addr+seg3*sizeof(struct pte), pte);
	}

	printf("Mapping complete\n");
	return spfn;
}

/* Routine to calculate the shift factor to use for a given page size.
 * Taken from pmap.c
 */

long
msb(m)		/* Find the position of the most significant bit */
	unsigned long m;
{
	unsigned long n;
	long shift, log;

	if (m == 0)
		return (-1);
	for (shift = NBBY*sizeof(unsigned long)/2, log = 0; shift; shift >>= 1)
		if (n = m>>shift) {
			log += shift;
			m = n;
		}
	return log;
}
