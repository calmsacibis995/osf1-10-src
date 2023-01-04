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
static char *rcsid = "@(#)$RCSfile: translate.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/10/29 20:36:36 $";
#endif

#include <stdio.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/memlog.h>

void
translate()
{
	FILE 	*nm, *logfile, *transfile;
	char	logstr[128], transtr[256], *logstrpt, *transtrpt;
	long 	mem_count;
	int 	type;
	void 	*caller;
	int 	size;
	char	*routine;
	long	addr;
	char	symb[128];

        nm = fopen("/vmunix.nm", "r");
	if(nm == (FILE *)NULL) {
		printf("memlog: /vmunix.nm open failed, errno=%d\n", errno);
		exit(1);
	}


        logfile = fopen("/memloglog", "r");
	if(logfile == (FILE *)NULL) {
		printf("memlog: /memloglog open failed, errno=%d\n", errno);
		exit(1);
	}

        transfile = fopen("./memxlog", "w");
	if(transfile == (FILE *)NULL) {
		printf("memlog: ./memxlog open failed, errno=%d\n", errno);
		exit(1);
	}

	
	while(1) {
            logstrpt = fgets (logstr, 128, logfile);
	    if(logstrpt == (char *)NULL) 
		break;

	    sscanf(logstrpt,"%18d %2d %16lx %10d", &mem_count, &type, &caller, &size);
	    /*printf("%19d %1d %16lx %10d", mem_count, type, caller, size);*/
	    switch(type) {
		case MALLOC_LOG:
			routine = "malloc";
			break;
		case FREE_LOG:
			routine = "free";
			break;
		case KALLOC_LOG:
			routine = "kalloc";
			break;
		case KFREE_LOG:
			routine = "kfree";
			break;
		case KGET_LOG:
			routine = "kget";
			break;
		case KMEM_ALLOC_LOG:
			routine = "kmem_alloc";
			break;
		case KMEM_FREE_LOG:
			routine = "kmem_free";
			break;
		case ZALLOC_LOG:
			routine = "zalloc";
			break;
		case ZGET_LOG:
			routine = "zget";
			break;
		case ZFREE_LOG:
			routine = "zfree";
			break;
		case H_KMEM_ALLOC_LOG:
			routine = "h_kmem_alloc";
			break;
		case H_KMEM_ZALLOC_LOG:
			routine = "h_kmem_zalloc";
			break;
		case H_KMEM_FREE_LOG:
			routine = "h_kmem_free";
			break;
		case H_KMEM_FAST_ALLOC_LOG:
			routine = "h_kmem_fast_alloc";
			break;
		case H_KMEM_FAST_ZALLOC_LOG:
			routine = "h_kmem_fast_zalloc";
			break;
		case H_KMEM_FAST_FREE_LOG:
			routine = "h_kmem_fast_free";
			break;
		default:
			break;
	    }
	    getsymb(nm, (long)caller, symb);
	    sprintf(transtr, "%18d %20s %22s %10d\n", mem_count, routine, symb, size);
            fputs(transtr, transfile);
	}

	return;
}

getsymb(nm, addr, symb)
FILE	*nm;
long	addr;
char 	*symb;
 {
	char 	symb1[128], symb2[128];
	long	addr1, addr2;
	char 	*pt1, *pt2, *tmpp;

	pt1 = symb1;
	pt2 = symb2;
	rewind(nm);

	getnext(nm, pt1, &addr1);
	if(addr < addr1)
		return(0);

	while(getnext(nm, pt2, &addr2)) {
		if(addr >= addr2) {
			addr1 = addr2;
			tmpp = pt1;
			pt1 = pt2;
			pt2 = tmpp;
		}
		else {
			strcpy(symb, pt1);
			return(1);
		}
	}
	strcpy(symb, pt1);
	return(1);

 }


getnext(nm, name, addr)
FILE * nm;
char * name;
long *addr;
{
        char   	nmstr[128], *nmstrpt;
	char 	*namept;
	char 	addst[17];
	long 	addval;
	int	i;

        nmstrpt = fgets (nmstr, 128, nm);

	if(nmstrpt == (char *)NULL) {
		/*printf("read failed, nmstrpt=%lx, errno=%d\n",nmstrpt, errno);*/
		return(0);
	}
	
	namept = name;
	while((*nmstrpt != ' ') && (*nmstrpt != '|')) {
		*namept++ = *nmstrpt++;
	}
	*namept = '\0';
		
	while(*nmstrpt != '|') {
		nmstrpt++;
	}

	nmstrpt++;

	for(i=0; i<16; i++) 
		addst[i] = *nmstrpt++;

	addst[16] = '\0';

	sscanf(addst, "%lx", &addval);
	/*printf("addval=%lx\n", addval);*/
	
	if(addval == 0) {
		printf("strtol failed, addval=%lx, errno=%d\n",addval, errno);
		return(0);
	}
	*addr = addval;

	return(1);
}
