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
 *	@(#)$RCSfile: dmpfmt.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:14:41 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/* dmpfmt.h	5.1 - 86/12/09 - 06:00:00 */
#ifndef _DMPFMT_H_
#define _DMPFMT_H_

/************************************************************************/
/*                                                                      */
/* MODULE NAME    =    dump                                             */
/* COMPONENT NAME =    Dmp                                              */
/* LPP NAME       =    BOS                                              */
/*                                                                      */
/* INCLUDE  NAME  =    dmpfmt                                           */
/*                                                                      */
/* DESCRIPTIVE NAME =  Include file for Dump Formatter                  */
/*                                                                      */
/* COPYRIGHT = ####-### (C) COPYRIGHT IBM CORP 1984                     */
/*             LICENSED MATERIAL - PROGRAM PROPERTY OF IBM              */
/*             REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083    */
/*                                                                      */
/*                                                                      */
/* STATUS =            Unit Test                                        */
/*                                                                      */
/* FUNCTION =          Define Global Variables                          */
/*                                                                      */
/*                                                                      */
/* CHANGE ACTIVITY =                                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#include <fcntl.h>
#include <stdio.h>

 extern int screen_size;       /* number of lines between prompts */
 extern int b_flag;            /* input mode flag          */
 extern int f_line[];          /* output line   */
 extern int buff[];            /* diskette block buffer    */
 extern char dump_id[];        /* dump message ID          */
 extern char toc_buf[];        /* diskette toc   buffer    */
 extern char info_buff[];      /* diskette information buffer    */
 extern char d_entry[];        /* dump table entry buffer    */
 extern char *ptr;             /* pointer to buffer        */
 extern char *l_ptr;           /* pointer to buffer        */
 extern char *dp;              /* dump entry data pointer  */
 extern char *dp1;             /* dump entry data pointer  */
 extern int subctr;            /* substructure counter     */
 extern int path_diff;         /* data read from disk byte adjustment */
 extern int blk_cnt;           /* count of blocks read     */
 extern long block;            /* structure starting block number   */
 extern char  name[];          /* structure name           */
 extern char  type[];          /* structure name           */
 extern long vrmlen;           /* structure length         */
 extern unsigned long st_addr; /* structure starting address */
 extern long s_length;         /* substructure length      */
 extern unsigned long addr;    /* structure address        */
 extern char  fpath[];         /* path name of diskette    */
 extern int  fdes;             /* file descriptor          */
 extern char  header[];        /* Component Header Buffer  */
 extern char  s_header[];      /* Structure Header Buffer  */
 extern char  hd_buf[];        /* header build buffer      */
 extern char  cbhdr[];         /* Control Block Header     */
 extern int sy_str;            /* symptom string flag      */
 extern long symstr;           /* # of lines in symptom string  */
 extern long linecnt;          /* # of lines output        */
 extern int  *p_bk;            /* pointer to block addresses */
 extern int  page_blks;        /* page of block addresses    */


	struct d_info {         /* diskette information     */
		char     abend[8];
		char     year;
		char     month;
		char     day;
		char     hour;
		char     minutes;
		char     seconds;
		char     admy;
		char     bdmy;
		long     mod_iocn;
		long     mod_addr;
		long     mod_offa;
		char     mod_vern[8];
		long     tocptr;
		long     len_dump;
		long     len_toc;
		long     d_flag;
		long     len_info;
		long     ab_addr;
		long     c_start;
		char     mod_name[8];
	   };

	struct d_toc  {                  /* diskette toc   */
		long     pbn;
		char     dname[8];
		long     dlength;
	     };
 extern struct dte    {
		char    name[8];    /* dump subentry name      */
		long    len;        /* length                  */
		long    dptr;       /* offset address of data  */
		long    resv;       /* reserved                */
	     } *pde;



 extern struct dt_enty  {               /* dump table entry  */
		long     dt_len;
		char     dtname[8];
		long     *dt_ptr;
		long     resv;
	     };

 extern struct d_table  {                /* dump table       */
		long     t_len;
		struct dt_enty ta_en;
	     };

	struct s_head  {                 /* structure header   */
		long     t_len;
		long     st_len;
	     };
	struct ss_head  {                /* substructure header   */
		long     su_len;
	     };


#define TOCSIZE    32               /* num of d_info entries  */
#define STDOUT_FDES  1              /* stdout file descriptor */

 extern struct d_toc to_buff[];          /* table of contents structure  */
 extern struct d_info *ptr_info;        /*  pointer to information struc  */
 extern struct dt_enty  *ptr_dte;       /*  pointer dump entries          */
 extern struct d_table *ptr_dt;        /*  pointer dump table structure  */
 extern struct s_head  *ptr_head;      /*  pointer header        */
 extern struct ss_head  *ptr_shead;      /* pointer to substructure     */


#define INFOSIZE   52              /* num of d_info entries  */
#define SUCCESS    0            /* Successful return code   */
#define FAIL       1            /* UnSuccessful return code */
#define RASDATA    rasdata      /* RAS data type            */
#define VRMPAGE0   vrmpage0     /* vrmpage0 data type       */
#define VRMCODE    vrmcode      /* vrmcode data type        */
#define VRMDATA    vrmdata      /* vrmdata data type        */
#define TRCDATA    trcdata      /* trcdata data type        */
#define TRCDATA1   trcdata1     /* trcdata data type        */
#define TRCDATA2   trcdata2     /* trcdata data type        */
#define TRCDATA3   trcdata3     /* trcdata data type        */
#define TRCDATA4   trcdata4     /* trcdata data type        */
#define TRCDATA5   trcdata5     /* trcdata data type        */
#define REGDATA    reg#_        /* General Purpose Register */
#define LOMEMORY   lomemory     /* Low Memory               */
#define HIMEMORY   himemory     /* High Memory              */
#define SQADATA    sqa data     /* System Queue Area Data   */
#define VMMSTATI   vmmstati     /* VMM Static Area Data     */
#define VMMSCBSA   vmmscbsa     /* VMM SCB Data             */
#define VMMSSBSA   vmmssbsa     /* VMM SSB Data             */
#define REGISTER   register     /* Register Data            */

#define VDATA      1            /* vrmdata data type        */
#define TDATA      2            /* trcdata data type        */
#define CDATA      3            /* vrmcode data type        */
#define PDATA      4            /* vrmpage0 data type       */
#define RDATA      5            /* RAS data type            */
#define DDATA      6            /* dump table entries       */
#define GDATA      7            /* Register Data            */
#define ADATA      8            /* System Queue Area Data   */
#define LDATA      9            /* Low Memory               */
#define EDATA      10           /* Register Data            */
#define WDATA      11           /* VMM Static Data          */
#define XDATA      12           /* VMM SCB Data             */
#define YDATA      13           /* VMM SSB Data             */
#define BDATA      14           /* All VRM Control Blocks   */
#define HDATA      15           /* High Memory              */
#define SDATA      16           /* Error Log Buffer         */
#define VMM_ADATA  18           /* VMM Arm position data    */
#define VMM_PDATA  19           /* VMM pfh data             */
#define VMM_MDATA  20           /* VMM xpte map data        */
#define VMM_IDATA  21           /* VMM ipte data            */
#define VMM_XDATA  22           /* VMM xpte data            */
#define VMM_TDATA  23           /* VMM tdisk  map data      */
#define FDATA      24           /* Hardfile data            */
#define UDATA      25           /* Unformatted VRM CBs      */
#define NDATA      26           /* Bus Memory               */
#define N_NDATA    27           /* non volatile ram         */
#define N_ADATA    28           /* area table data          */
#define N_MDATA    29           /* vm$map data              */
#define N_EDATA    30           /* new error log data       */
#define N_EPDATA   31           /* error log start address  */
#define N_VDATA    32           /* new vrm control blocks   */
#define N_XDATA    33           /* new vmm xpte data        */
#define N_XPDATA   34           /* new vmm xpte data addrs  */
#define N_XMDATA   35           /* new vmm xpte map         */
#define N_XMPDATA  36           /* new vmm xpte map addrs   */
#define N_IPT2     37           /* 2nd half of ipte data    */
#define IDATA      100          /* VMM IPTE Data            */
#define JDATA      101          /* VMM XPTE Data            */
#define READW      r+           /* open file for read/write */


#endif /* _DMPFMT_H_ */
