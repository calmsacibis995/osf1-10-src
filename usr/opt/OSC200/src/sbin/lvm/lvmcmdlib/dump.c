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
static char	*sccsid = "@(#)$RCSfile: dump.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/05 15:53:46 $";
#endif 
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
 *   dump.c
 *   
 *   Contents:
 *
 *   All the routines and data structures which are related to the 
 *   dump (i.e., visualization) of information of the objects of LVM
 *   (that is, volume groups, physical volumes, logical volumes) are here.
 *   Supplied pathnames are NOT assumed to be "clean", that is (as far as VG
 *   and PV are concerned) in a form such that they can be successfully
 *   retrieved inside the lvmtab file.
 *   Private variables contain the information returned by the
 *   ioctl calls.
 *
 *   int dump_vg(char *vg_path, int mode)
 *	Returns OK or NOT_OK. mode is either VERBOSE or TERSE. Dumps info
 *	about a VG.
 *
 *   int dump_lv(char *lv_path, int mode)
 *	Returns OK or NOT_OK. mode is either VERBOSE or TERSE. Dumps info
 *	about a PV.
 *
 *   int dump_pv(char *pv_path, int mode)
 *	Returns OK or NOT_OK. mode is either VERBOSE or TERSE. Dumps info
 *	about a PV.
 *
 *   void dump_vg_title()
 *   void dump_lv_title()
 *   void dump_pv_title()
 *   void dump_lx_title()
 *   void dump_px_title()
 *   void dump_lvdistr_title()
 *   void dump_pvdistr_title()
 *	Set things so that a header is printed out before the info about
 *	one of the types of objects is printed.
 */

/*
 *  Modification History:  dump.c
 *
 *  24-Apr-91     Terry Carruthers
 *	Modified function dump_queried_lv function to correct bug
 *      in "for loops" which determine which physical volumes
 *      are used by the logical volume.
 *
 *  12-Jun-91     Terry Carruthers
 *	Modified function dump_vg to correct actions needed when
 *      a volume group is not in the activated state.
 *
 */

#include "lvmcmds.h"

/*
 *   Two flavours of output formats are available, using one of the
 *   following definitions:
 *   1) TABLE_FMT
 *	ps-like format, that is, a line with titles, then one row for
 *	each "thing" which has to be dumped
 *   2) INDENT_FMT
 *   	lpc-like format, with nesting driven by the logical nesting of
 *	"things" to be dumped
 *   These values are used in the info_group_tab (~250 lines below)
 */

#define TABLE_FMT	1
#define INDENT_FMT	2

/* This is the tabulation string */
#define NEST_STRING	"   "


/* This is the tabulation macro */
#define do_nest()	{register int j;\
			 for (j = 0; j < print_nesting; j++) \
			      printf(NEST_STRING);}

/*
 *   This is very important: it defines the size of the small
 *   buffers which have to hold the string to be printed out for each
 *   possible type of information concerning each possible object
 *   (VG, PV, LV, LX, PX); in this small buffers we'll sprintf all
 *   the numbers, strings and whatever else has to be dumped.
 */

#define P_INFOMAX 	22
#define P_INFOMAX2 	42

/*
 *   Due to i18n, we can't initialize static tables with messages;
 *   still, we don't want our nice reports to be printed out in a messy
 *   way; so, here's a couple of useful macros; note that we check for
 *   P_INFOMAX
 */
#define init_tables()	if (first_time) set_tables(), first_time = FALSE
#define set_header(tab, idx, msg) {                                    \
	register char *s; register int l, oldl;                        \
	   s = msg;   l = strlen(s);   oldl = tab[idx].out_size;       \
	   if (l > oldl || oldl >= P_INFOMAX2)                          \
	      tab[idx].out_size = (l < P_INFOMAX2) ? l : P_INFOMAX2 - 1; \
	   tab[idx].header = s;                                        \
	}
#define set_g_hdr(tab, idx, msg) (tab[idx].general_header = msg)

/* Macro to prepare the bit masks for the low-level dump routine */
#define bit(i)		(1 << (i))
#define ALL_BITS	(~0)
#define ALL_INFO	ALL_BITS

/* Macro to use one entry of one table defined as below */
#define dump_value(table, entry, value) 	\
	(sprintf(table[entry].info, table[entry].info_fmt, \
	 table[entry].out_size, table[entry].out_size, (value)), \
	 table[entry].must_print = TRUE)

/* Macro to convert a LX/PX status to a suitable string */
#define lx_status(st)	(((st) & LVM_PXSTALE) ? MSG_LX_STALE : \
			 ((st) & LVM_PVMISSING) ? MSG_LX_MISSING : \
			 MSG_LX_CURRENT)

/* "Groups" of information are the information related to one object */
typedef struct {
   short print_mode;		/* TABLE_FMT or INDENT_FMT */
   char *general_header;	/* what this info group contains */
} info_group;

/* Structure to keep the formats together in a table */
typedef struct {
   char *info_fmt;		/* printf fmt */
   int out_size;		/* how much space must be used when printing */
   char must_print;		/* should this actually be printed out? */
   char rm_lead_0s;		/* remove leading zeros before printing */
   char *header;		/* title of this field */
   char info[P_INFOMAX2];	/* buffer where we sprintf the info */
} print_fmt;

/* ID's for print VG */
#define P_VG_NAME		0
#define P_VG_STATUS		1
#define P_VG_MAX_LV		2
#define P_VG_CUR_LV		3
#define P_VG_OPEN_LV		4
#define P_VG_MAX_PV		5
#define P_VG_CUR_PV		6
#define P_VG_ACT_PV		7
#define P_VG_PX_SIZE		8
#define P_VG_MAX_PX_PER_PV	9
#define P_VG_VGDA_CNT		10
#define P_VG_PX_CNT		11
#define P_VG_USED_PX		12
#define P_VG_FREE_PX		13
#define VG_PRINT_FMT		14

/* Special values for print VG; they require calls to other functions */
#define P_VG_LV_INFO		14
#define P_VG_PV_INFO		15

/* Tables of formats: table for VG */
static print_fmt vg_fmt[VG_PRINT_FMT] = {
/* info_fmt	out_size   must_print rm_lead_0s */
   {"%-*.*s",	40,	   FALSE,     FALSE},    /*   P_VG_NAME	*/
   {"%-*.*s",	12,	   FALSE,     FALSE},    /*   P_VG_STATUS	*/
   {"%*.*u",	7,	   FALSE,     TRUE},     /*   P_VG_MAX_LV	*/
   {"%*.*u",	7,	   FALSE,     TRUE},     /*   P_VG_CUR_LV	*/
   {"%*.*u",	7,	   FALSE,     TRUE},     /*   P_VG_OPEN_LV	*/
   {"%*.*u",	7,	   FALSE,     TRUE},     /*   P_VG_MAX_PV	*/
   {"%*.*u",	7,	   FALSE,     TRUE},     /*   P_VG_CUR_PV	*/
   {"%*.*u",	7,	   FALSE,     TRUE},     /*   P_VG_ACT_PV	*/
   {"%*.*u",	9,	   FALSE,     TRUE},     /*   P_VG_PX_SIZE	*/
   {"%*.*u",	13,	   FALSE,     TRUE},     /*   P_VG_MAX_PX_PER_PV*/
   {"%*.*u",	8,	   FALSE,     TRUE},     /*   P_VG_PX_CNT	*/
   {"%*.*u",	8,	   FALSE,     TRUE},     /*   P_VG_USED_PX	*/
   {"%*.*u",	8,	   FALSE,     TRUE},     /*   P_VG_FREE_PX	*/
   {"%*.*u",	4,	   FALSE,     TRUE}      /*   P_VG_VGDA_CNT*/
};

/* ID's for print LV */
#define P_LV_NAME		0
#define P_LV_VGNAME		1
#define P_LV_PERM		2
#define P_LV_STATUS		3
#define P_LV_WRITE_VER		4
#define P_LV_MIRRORS		5
#define P_LV_SCHED		6
#define P_LV_LX_CNT		7
#define P_LV_USED_PX		8
#define P_LV_BBLOCK_POL		9
#define P_LV_ALLOC		10
#define P_LV_USED_PV		11
#define LV_PRINT_FMT		12

/* Special values for print LV; they require calls to other functions */
#define P_LV_PV_INFO		12
#define P_LV_LX_INFO		13

static print_fmt lv_fmt[LV_PRINT_FMT] = {
/* info_fmt	out_size   must_print rm_lead_0s */
   {"%-*.*s",	40,	   FALSE,     FALSE},    /*   P_LV_NAME	*/
   {"%-*.*s",	40,	   FALSE,     FALSE},    /*   P_LV_VGNAME	*/
   {"%-*.*s",	13,	   FALSE,     FALSE},    /*   P_LV_PERM	*/
   {"%-*.*s",	18,	   FALSE,     FALSE},    /*   P_LV_STATUS	*/
   {"%-*.*s",	13,	   FALSE,     FALSE},    /*   P_LV_WRITE_VER*/
   {"%*.*u",	13,	   FALSE,     TRUE},     /*   P_LV_MIRRORS	*/
   {"%-*.*s",	13,	   FALSE,     FALSE},    /*   P_LV_SCHED	*/
   {"%*.*u",	8,	   FALSE,     TRUE},     /*   P_LV_LX_CNT	*/
   {"%*.*u",	8,	   FALSE,     TRUE},     /*   P_LV_USED_PX	*/
   {"%-*.*s",	13,	   FALSE,     FALSE},    /*   P_LV_BBLOCK_POL*/
   {"%-*.*s",	13,	   FALSE,     FALSE},    /*   P_LV_ALLOC	*/
   {"%*.*u",	8,	   FALSE,     TRUE}      /*   P_LV_USED_PV	*/
};

/* ID's for print PV */
#define P_PV_NAME		0
#define P_PV_VGNAME		1
#define P_PV_STATUS		2
#define P_PV_ALLOC		3
#define P_PV_VGDA_CNT		4
#define P_PV_CUR_LV		5
#define P_PV_PX_SIZE		6
#define P_PV_PX_CNT		7
#define P_PV_FREE_PX		8
#define P_PV_USED_PX		9
#define P_PV_STALE_PX		10
#define PV_PRINT_FMT		11

/* Special values for print PV; they require calls to other functions */
#define P_PV_LV_INFO		11
#define P_PV_PX_INFO		12

static print_fmt pv_fmt[PV_PRINT_FMT] = {
/* info_fmt	out_size   must_print rm_lead_0s */
   {"%-*.*s",	40,	   FALSE,     FALSE},    /*   P_PV_NAME	*/
   {"%-*.*s",	40,	   FALSE,     FALSE},    /*   P_PV_VGNAME	*/
   {"%-*.*s",	13,	   FALSE,     FALSE},    /*   P_PV_STATUS	*/
   {"%-*.*s",	13,	   FALSE,     FALSE},    /*   P_PV_ALLOC	*/
   {"%*.*u",	4,	   FALSE,     TRUE},     /*   P_PV_VGDA_CNT*/
   {"%*.*u",	7,	   FALSE,     TRUE},     /*   P_PV_CUR_LV	*/
   {"%*.*u",	9,	   FALSE,     TRUE},     /*   P_PV_PX_SIZE	*/
   {"%*.*u",	8,	   FALSE,     TRUE},     /*   P_PV_PX_CNT	*/
   {"%*.*u",	8,	   FALSE,     TRUE},     /*   P_PV_FREE_PX	*/
   {"%*.*u",	8,	   FALSE,     TRUE},     /*   P_PV_USED_PX	*/
   {"%*.*u",	8,	   FALSE,     TRUE}      /*   P_PV_STALE_PX*/
};

/* ID's for print LX */
#define P_LX_ID			0
#define P_LX_PV1		1
#define P_LX_PX1		2
#define P_LX_STAT1		3
#define P_LX_PV2		4
#define P_LX_PX2		5
#define P_LX_STAT2		6
#define P_LX_PV3		7
#define P_LX_PX3		8
#define P_LX_STAT3		9
#define LX_PRINT_FMT		10

static print_fmt lx_fmt[LX_PRINT_FMT] = {
/* info_fmt	out_size   must_print rm_lead_0s */
   {"%*.*u",	4,	   FALSE,     FALSE},    /*   P_LX_ID	*/
   {"%-*.*s",	12,	   FALSE,     FALSE},    /*   P_LX_PV1 */
   {"%*.*u",	4,	   FALSE,     FALSE},    /*   P_LX_PX1 */
   {"%-*.*s",	8,	   FALSE,     FALSE},    /*   P_LX_STAT1 */
   {"%-*.*s",	12,	   FALSE,     FALSE},    /*   P_LX_PV2 */
   {"%*.*u",	4,	   FALSE,     FALSE},    /*   P_LX_PX2 */
   {"%-*.*s",	8,	   FALSE,     FALSE},    /*   P_LX_STAT2 */
   {"%-*.*s",	12,	   FALSE,     FALSE},    /*   P_LX_PV3 */
   {"%*.*u",	4,	   FALSE,     FALSE},    /*   P_LX_PX3 */
   {"%-*.*s",	8,	   FALSE,     FALSE},    /*   P_LX_STAT3 */
};

/* ID's for print PX */
#define P_PX_ID			0
#define P_PX_STAT		1
#define P_PX_LV			2
#define P_PX_LX			3
#define PX_PRINT_FMT		4

static print_fmt px_fmt[PX_PRINT_FMT] = {
/* info_fmt	out_size   must_print rm_lead_0s */
   {"%*.*u",	4,	   FALSE,     FALSE},    /*   P_PX_ID	*/
   {"%-*.*s",	8,	   FALSE,     FALSE},    /*   P_PX_STAT */
   {"%-*.*s",	20,	   FALSE,     FALSE},    /*   P_PX_LV */
   {"%*.*u",	4,	   FALSE,     FALSE},    /*   P_PX_LX */
};

/* ID's for print LV distribution */
#define P_LVDISTR_PV			0
#define P_LVDISTR_LX			1
#define P_LVDISTR_PX			2
#define LVDISTR_PRINT_FMT		3

static print_fmt lvdistr_fmt[LVDISTR_PRINT_FMT] = {
/* info_fmt	out_size   must_print rm_lead_0s */
   {"%-*.*s",	12,	   FALSE,     FALSE},    /*   P_LVDISTR_PV */
   {"%*.*u",	9,	   FALSE,     TRUE},     /*   P_LVDISTR_LX */
   {"%*.*u",	9,	   FALSE,     TRUE}      /*   P_LVDISTR_PX */
};

/* ID's for print PV distribution */
#define P_PVDISTR_LV			0
#define P_PVDISTR_LX			1
#define P_PVDISTR_PX			2
#define PVDISTR_PRINT_FMT		3

static print_fmt pvdistr_fmt[PVDISTR_PRINT_FMT] = {
/* info_fmt	out_size   must_print rm_lead_0s */
   {"%-*.*s",	20,	   FALSE,     FALSE},    /*   P_PVDISTR_LV */
   {"%*.*u",	9,	   FALSE,     TRUE},     /*   P_PVDISTR_LX */
   {"%*.*u",	9,	   FALSE,     TRUE}      /*   P_PVDISTR_PX */
};

/* Definitions for the groups of information */
#define VG_GROUP	0
#define LV_GROUP	1
#define PV_GROUP	2
#define LX_GROUP	3
#define PX_GROUP	4
#define LVDISTR_GROUP	5
#define PVDISTR_GROUP	6
#define MAX_GROUP	7

/* Table of the groups of information */
info_group info_group_tab[MAX_GROUP] = {
/*  print_mode */
   {INDENT_FMT}, /* VG_GROUP */
   {INDENT_FMT}, /* LV_GROUP */
   {INDENT_FMT}, /* PV_GROUP */
   {TABLE_FMT},  /* LX_GROUP */
   {TABLE_FMT},  /* PX_GROUP */
   {TABLE_FMT},  /* LVDISTR_GROUP */
   {TABLE_FMT}   /* PVDISTR_GROUP */
};

/*
 *   Flags to know whether we have to print out the titles or not;
 *   we have quite a weird mechanism to print out the title, but we want
 *   a dynamic computation of which parts of the title need to be printed.
 */

static int vg_title = FALSE;
static int lv_title = FALSE;
static int pv_title = FALSE;
static int lx_title = FALSE;
static int px_title = FALSE;
static int lvdistr_title = FALSE;
static int pvdistr_title = FALSE;

/* Nest output relatively to the nesting of objects to be dumped */
static int print_nesting;

/* Local functions */
static int dump_queried_lv(struct lv_querylv *qlvp, unsigned int info_mask, 
		char *vg_path, int vg_fd, char *lv_path);
static int dump_queried_pv(struct lv_querypvpath *qpvp, unsigned int info_mask,
		char *vg_path, int vg_fd, int px_size);
static int dump_lx(int lx_idx, lx_descr_t *lxp, unsigned int info_mask,
		int pxperlx_cnt);
static int dump_px(int px_idx, px_descr_t *pxp, unsigned int info_mask);
static int dump_lvdistr(unsigned int info_mask, char *pv_path, 
		int tot_used_lx, int tot_used_px);
static int dump_pvdistr(unsigned int info_mask, char *lv_path, 
		int tot_used_lx, int tot_used_px);
static void shift_left();
static void shift_right();
static void print_header(int group_id, print_fmt *table, int count);
static void print_out(int group_id, print_fmt *table, int count);
static void rm_lead_0s();
static void set_tables();

/* Flag used to initialize the messagised tables */
static int first_time = TRUE;


int
dump_vg(char *vg_path, int mode)
{
   /* A huge number of local variables */
   register int i;
   unsigned int info_mask;

   /* VG related variables */
   int vg_fd;
   struct lv_queryvg queryvg;
   char *clean_path;
   int active_pv;
   int vgda_cnt;
   int total_px;
   int free_px;
   int open_lv;

   /* PV related variables */
   char **pv_names;
   int pv_cnt;
   struct lv_querypvpath *querypv_array;
   char *pv_queried;
   register struct lv_querypvpath *qpvp;

   /* LV related variables */
   char **lv_names;
   int lv_cnt;
   struct lv_querylv *querylv_array;
   register struct lv_querylv *qlvp;

   debug(dbg_entry("dump_vg"));

   init_tables();

   /* Another library function takes care of generating the clean path */
   if ((clean_path = check_and_openvg(vg_path, &vg_fd)) == NULL) {
      debug(dbg_exit());
      return(NOT_OK);
   }
   
   /* Nothing to do to initialize the structure for the system call */

   if (query_driver(vg_fd, LVM_QUERYVG, &queryvg) < 0) {
      print_prgname();
      fprintf(stderr, MSG_QUERYVG_FAILED, vg_path);
      lvm_perror(LVM_QUERYVG);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* ULTRIX/OSF:  Modified to check for not activated state
    * and only do a partial display for volume group in this
    * state.  This prevents much work that never succeeds
    * anyway because of state.
    */

   if ( !(queryvg.status & LVM_VGACTIVATED) ) {

       /* VG state is not activated, so just print 
	* this information and return
	*/
       dump_value(vg_fmt, P_VG_NAME, clean_path);
       dump_value(vg_fmt, P_VG_STATUS, (queryvg.status & LVM_VGACTIVATED) ?
					MSG_VG_ON : MSG_VG_OFF);

       /* Do actual print */
       if (vg_title) {
           print_header(VG_GROUP, vg_fmt, entries(vg_fmt));
	   vg_title = FALSE;
       }
       print_out(VG_GROUP, vg_fmt, entries(vg_fmt));
       
       return(OK);

   }

   /* Some information has to be retrieved from the PV's */
   if (lvmtab_read() != OK ||
	    lvmtab_getpvnames(clean_path, &pv_names, &pv_cnt) != OK) {
      print_prgname();
      fprintf(stderr, MSG_NO_PVNAMES, clean_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Query multiple PV, if any. Of course, at least one PV should be there */
   active_pv = 0;
   vgda_cnt = 0;
   total_px = 0;
   free_px = 0;
   if (pv_cnt > 0) {
      if (multiple_query_pv(vg_fd, pv_names, pv_cnt, &querypv_array,
	       &pv_queried) != pv_cnt) {
         print_prgname();
         fprintf(stderr, MSG_QUERYPVS_FAILED);
      }

      /* Scan all the gathered information, to pick what we need */
      for (qpvp = querypv_array, i = 0; i < pv_cnt; i++, qpvp++) {

	 /* Check if we have data about it */
	 if (!pv_queried[i])
	    continue;

	 /* Some numbers... */
	 total_px += qpvp->px_count;
	 free_px += qpvp->px_free;

	 /* An active PV is an attached PV */
	 if ((qpvp->pv_flags & LVM_NOTATTACHED) == 0)
	    active_pv++;

	 /* On each PV there are either 2 or 0 VGDA */
	 if ((qpvp->pv_flags & LVM_NOVGDA) == 0)
	    vgda_cnt += 2;
      }
   }

   /* Some information has to be retrieved from the LV's */
   if (getlvnames(clean_path, &lv_names, &lv_cnt) != OK) {
      print_prgname();
      fprintf(stderr, MSG_NO_LVNAMES, clean_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Query multiple LV, if any. */
   open_lv = 0;
   if (lv_cnt > 0) {
      if (multiple_query_lv(vg_fd, lv_names, lv_cnt, &querylv_array) != OK) {
         print_prgname();
         fprintf(stderr, MSG_QUERYLVS_FAILED);
	 debug(dbg_exit());
         return(NOT_OK);
      }

      /* Scan all the gathered information, to pick what we need */
      for (qlvp = querylv_array, i = 0; i < lv_cnt; i++, qlvp++) {

	 /* An active LV is a not disabled LV */
	 if ((qlvp->lv_flags & LVM_DISABLED) == 0)
	    open_lv++;
      }
   }

   /* Should print all that's available... */
   dump_value(vg_fmt, P_VG_NAME, clean_path);
   dump_value(vg_fmt, P_VG_STATUS, (queryvg.status & LVM_VGACTIVATED) ?
					MSG_VG_ON : MSG_VG_OFF);
   dump_value(vg_fmt, P_VG_MAX_LV, queryvg.maxlvs);
   dump_value(vg_fmt, P_VG_CUR_LV, queryvg.cur_lvs);
   dump_value(vg_fmt, P_VG_OPEN_LV, open_lv);
   dump_value(vg_fmt, P_VG_MAX_PV, queryvg.maxpvs);
   dump_value(vg_fmt, P_VG_CUR_PV, queryvg.cur_pvs);
   dump_value(vg_fmt, P_VG_ACT_PV, active_pv);
   dump_value(vg_fmt, P_VG_PX_SIZE, queryvg.pxsize);
   dump_value(vg_fmt, P_VG_MAX_PX_PER_PV, queryvg.maxpxs);
   dump_value(vg_fmt, P_VG_PX_CNT, total_px);
   dump_value(vg_fmt, P_VG_USED_PX, total_px - free_px);
   dump_value(vg_fmt, P_VG_FREE_PX, queryvg.freepxs);
   dump_value(vg_fmt, P_VG_VGDA_CNT, vgda_cnt);

   debug_msg("dump.dump_vg: free_px %s queryvg.freepxs\n",
            (free_px != queryvg.freepxs) ? "!=" : "==");

   /* Do actual print */
   if (vg_title) {
      print_header(VG_GROUP, vg_fmt, entries(vg_fmt));
      vg_title = FALSE;
   }
   print_out(VG_GROUP, vg_fmt, entries(vg_fmt));

   /* ...including what is considered to be verbose */
   if (mode == VERBOSE) {

      /*
       *   Print info about each LV in VG; we have already queried
       *   all the LV's, so we just have to call the low-level routine
       *   that takes care of this; supply this routine with a mask
       *   of what we want from it.
       */

      shift_right();
      dump_lv_title();
      info_mask = bit(P_LV_NAME) |
		  bit(P_LV_LX_CNT) |
		  bit(P_LV_STATUS) |
		  bit(P_LV_USED_PX) |
		  bit(P_LV_USED_PV);

      for (qlvp = querylv_array, i = 0; i < lv_cnt; i++, qlvp++)
         if (dump_queried_lv(qlvp, info_mask, clean_path, vg_fd, lv_names[i])
		  != OK) {
            print_prgname();
            fprintf(stderr, MSG_DUMPLV_FAILED, lv_names[i]);
	    /* Don't need to return(NOT_OK); print other LV's info, instead */
	 }
      shift_left();

      /* Same as above, but for PV's */
      shift_right();
      dump_pv_title();
      info_mask = bit(P_PV_NAME) |
		  bit(P_PV_STATUS) |
		  bit(P_PV_PX_CNT) |
		  bit(P_PV_FREE_PX);

      for (qpvp = querypv_array, i = 0; i < pv_cnt; i++, qpvp++) {

	 /* Check if we have data about it */
	 if (!pv_queried[i])
	    continue;

         if (dump_queried_pv(qpvp, info_mask, clean_path, vg_fd,
		  queryvg.pxsize) != OK) {
            print_prgname();
            fprintf(stderr, MSG_DUMPPV_FAILED, qpvp->path);
	    /* Don't need to return(NOT_OK); print other PV's info, instead */
	 }
      }
      shift_left();
   }

   debug(dbg_exit());
   return(OK);
}



int
dump_lv(char *lv_path, int mode)
{
   struct lv_querylv qlv;
   char *vg_path;
   char *clean_path;
   int vg_fd;
   unsigned int info_mask;
   dev_t lv_dev_num;

   debug(dbg_entry("dump_lv"));

   init_tables();

   /* Get path name of VG owning this LV */
   if (lvtovg(lv_path, &vg_path) != OK) {
      print_prgname();
      fprintf(stderr, MSG_NO_VGFORLV, lv_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Another library function takes care of generating the clean path */
   if ((clean_path = check_and_openvg(vg_path, &vg_fd)) == NULL) {
      debug(dbg_exit());
      return(NOT_OK);
   }
   
   /* If it's not a block device, then it can't be an LV */
   if (special_f_tst(lv_path, S_IFBLK, &lv_dev_num) == NOT_OK) {
      print_prgname();
      fprintf(stderr, MSG_NOT_LV, lv_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Set input parameters */
   qlv.minor_num = minor(lv_dev_num);

   /* Ask the driver some info */
   if (query_driver(vg_fd, LVM_QUERYLV, &qlv) < 0) {
      print_prgname();
      fprintf(stderr, MSG_QUERYLV_FAILED, lv_path);
      lvm_perror(LVM_QUERYLV);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Prepare the mask of bits for the low level routine */
   info_mask = (ALL_INFO & ~bit(P_LV_USED_PV));
   if (mode == TERSE) {
      /* Turn off the bits for extra-info */
      info_mask &= ~bit(P_LV_PV_INFO);
      info_mask &= ~bit(P_LV_LX_INFO);
   }

   /* Print the information */
   if (dump_queried_lv(&qlv, info_mask, vg_path, vg_fd, lv_path) != OK) {
      print_prgname();
      fprintf(stderr, MSG_DUMPLV_FAILED, lv_path);
      debug(dbg_exit());
      return(NOT_OK);
   }
   debug(dbg_exit());
   return(OK);
}



int
dump_pv(char *pv_path, int mode)
{
   struct lv_querypvpath qpv;
   struct lv_queryvg queryvg;
   char *vg_path;
   char *clean_path;
   int vg_fd;
   unsigned int info_mask;

   debug(dbg_entry("dump_pv"));

   init_tables();

   /* Get path name of VG owning this PV */
   if (lvmtab_read() != OK ||
	    !lvmtab_ispvinsomevg(pv_path, &vg_path)) {
      print_prgname();
      fprintf(stderr, MSG_NO_VGFORPV, pv_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Another library function takes care of generating the clean path */
   if ((clean_path = check_and_openvg(vg_path, &vg_fd)) == NULL) {
      debug(dbg_exit());
      return(NOT_OK);
   }
   
   /* Set input parameters */
   qpv.path = pv_path;

   /* Ask the driver some info */
   if (query_driver(vg_fd, LVM_QUERYPVPATH, &qpv) < 0) {
      print_prgname();
      fprintf(stderr, MSG_QUERYPVPATH_FAILED, pv_path);
      lvm_perror(LVM_QUERYPVPATH);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /*
    *   One information can be accessed only via QUERYVG;
    *   nothing to do to initialize the structure for the system call.
    */

   if (query_driver(vg_fd, LVM_QUERYVG, &queryvg) < 0) {
      print_prgname();
      fprintf(stderr, MSG_QUERYVG_FAILED, vg_path);
      lvm_perror(LVM_QUERYVG);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Prepare the mask of bits for the low level routine */
   info_mask = ALL_INFO;
   if (mode == TERSE) {
      /* Turn off the bits for extra-info */
      info_mask &= ~bit(P_PV_LV_INFO);
      info_mask &= ~bit(P_PV_PX_INFO);
   }

   /* Print the information */
   if (dump_queried_pv(&qpv, info_mask, vg_path, vg_fd, queryvg.pxsize) != OK) {
      print_prgname();
      fprintf(stderr, MSG_DUMPPV_FAILED, pv_path);
      debug(dbg_exit());
      return(NOT_OK);
   }
   debug(dbg_exit());
   return(OK);
}



void
dump_vg_title()
{
   /* Ridicoulous */
   vg_title = TRUE;
}



void
dump_lv_title()
{
   /* Ridicoulous */
   lv_title = TRUE;
}



void
dump_pv_title()
{
   /* Ridicoulous */
   pv_title = TRUE;
}





void
dump_lx_title()
{
   /* Ridicoulous */
   lx_title = TRUE;
}



void
dump_px_title()
{
   /* Ridicoulous */
   px_title = TRUE;
}



void
dump_lvdistr_title()
{
   /* Ridicoulous */
   lvdistr_title = TRUE;
}



void
dump_pvdistr_title()
{
   /* Ridicoulous */
   pvdistr_title = TRUE;
}



/*
 *   Local functions
 */


static int
dump_queried_lv(struct lv_querylv *qlvp, unsigned int info_mask, 
		char *vg_path, int vg_fd, char *lv_path)
{
   unsigned int subinfo_mask;
   int maxlx_cnt, curlx_cnt, pxperlx_cnt;
   lx_descr_t *lv_map;
   char **pv_names;
   int pv_cnt;
   register lx_descr_t *lxp;
   register int i, j, k;
   register unsigned short pv_key;
   int tot_used_lx;
   int tot_used_px;
   int tot_used_pv;
   int stale_lv;
   int lx_is_on_pv;

   debug(dbg_entry("dump_queried_lv"));

   debug_msg("vg_path: \"%s\"\n", vg_path);
   debug_msg("lv_path: \"%s\"\n", lv_path);
   debug_msg("info_mask: %08x\n", info_mask);

   /* Quite dumb, but lengthy */

   /* Do we have to dump some information that we can get only from the map? */
   if (info_mask & (bit(P_LV_LX_INFO) |
		    bit(P_LV_STATUS) |
		    bit(P_LV_PV_INFO) |
		    bit(P_LV_USED_PV))) {

      /* Get the map of LV */
      if (getlvmap(vg_fd, vg_path, qlvp->minor_num, &lv_map, &maxlx_cnt,
	       &curlx_cnt, &pxperlx_cnt) != OK) {
	 print_prgname();
         fprintf(stderr, MSG_LV_MAP_NOT_READ, lv_path);
	 debug(dbg_exit());
         return(NOT_OK);
      }

      /* Set up the pv_key <-> pv_name conversion */
      if (pvkeys_and_names(vg_fd, vg_path, &pv_names, &pv_cnt) != OK) {
	 print_prgname();
         fprintf(stderr, MSG_CANT_GET_PV_NAMES, vg_path);
	 debug(dbg_exit());
	 return(NOT_OK);
      }
   }

   debug_msg("returned pv_cnt: %d\n", pv_cnt);

   if (info_mask & bit(P_LV_NAME))
      dump_value(lv_fmt, P_LV_NAME, lv_path);
   if (info_mask & bit(P_LV_VGNAME))
      dump_value(lv_fmt, P_LV_VGNAME, vg_path);
   if (info_mask & bit(P_LV_PERM))
      dump_value(lv_fmt, P_LV_PERM, (qlvp->lv_flags & LVM_RDONLY) ?
		MSG_LV_RDONLY : MSG_LV_RDWR);
   if (info_mask & bit(P_LV_STATUS)) {

      /* If LV is active... */
      if ((qlvp->lv_flags & LVM_DISABLED) == 0) {

         /* ...see if at least one extent is stale */
         stale_lv = FALSE;
         for (i = 0, lxp = lv_map; !stale_lv && i < curlx_cnt; i++) {
            for (j = 0; !stale_lv && j < pxperlx_cnt; j++)
	       if ((lxp[i][j].status & LVM_PXSTALE) != 0)
	          stale_lv = TRUE;
         }
      }
      dump_value(lv_fmt, P_LV_STATUS,
		(qlvp->lv_flags & LVM_DISABLED) ? MSG_LV_CLOSED :
		stale_lv ? MSG_LV_OPENSTALE : MSG_LV_OPENSYNCD);
   }
   if (info_mask & bit(P_LV_WRITE_VER))
      dump_value(lv_fmt, P_LV_WRITE_VER, (qlvp->lv_flags & LVM_VERIFY) ?
		MSG_LV_VERIFY : MSG_LV_NOVERIFY);
   if (info_mask & bit(P_LV_MIRRORS))
      dump_value(lv_fmt, P_LV_MIRRORS, qlvp->maxmirrors);
   if (info_mask & bit(P_LV_SCHED))
      dump_value(lv_fmt, P_LV_SCHED, qlvp->sched_strat == LVM_SEQUENTIAL ?
		MSG_LV_SEQUENTIAL : MSG_LV_PARALLEL);
   if (info_mask & bit(P_LV_LX_CNT))
      dump_value(lv_fmt, P_LV_LX_CNT, qlvp->numlxs);
   if (info_mask & bit(P_LV_USED_PX))
      dump_value(lv_fmt, P_LV_USED_PX, qlvp->numpxs);
   if (info_mask & bit(P_LV_BBLOCK_POL))
      dump_value(lv_fmt, P_LV_BBLOCK_POL, (qlvp->lv_flags & LVM_NORELOC) ?
		MSG_LV_NORELOC : MSG_LV_RELOC);
   if (info_mask & bit(P_LV_ALLOC))
      dump_value(lv_fmt, P_LV_ALLOC, (qlvp->lv_flags & LVM_STRICT) ?
		MSG_LV_STRICT : MSG_LV_NONSTRICT);
   if (info_mask & bit(P_LV_USED_PV)) {

      /* For each PV in VG, see whether it is used by this LV */
      tot_used_pv = 0;
      for (i = 0; i < pv_cnt; i++) {

	 if ((pv_key = pvpathtopvkey(pv_names[i])) == BAD_PV_KEY)
	    continue;
	 
	 /* Scan the LV map, to see if we use PX's from that PV */
	 lx_is_on_pv = FALSE;
	 for (k = 0; !lx_is_on_pv && k < curlx_cnt; k++) {

	    /* Scan the mirrors, to see if they are on that PV */
            for (j = 0; !lx_is_on_pv && j < pxperlx_cnt; j++) {

               /* Increase number of PV's used by this LV */
               /* ULTRIX/OSF: Added a check for "in_use". A pv_key of
                *        0 is valid, but 0 is also the initialized 
                *        pv_key value in the array.
                */
               if (lv_map[k][j].pv_key == pv_key && 
                   lv_map[k][j].in_use) {
                  tot_used_pv++;
                  lx_is_on_pv = TRUE;
	       }
            }
         }
      }
      dump_value(lv_fmt, P_LV_USED_PV, tot_used_pv);
   }

   /* Do actual print */
   if (lv_title) {
      print_header(LV_GROUP, lv_fmt, entries(lv_fmt));
      lv_title = FALSE;
   }
   print_out(LV_GROUP, lv_fmt, entries(lv_fmt));

   /* Additional info required? */
   if (info_mask & bit(P_LV_PV_INFO)) {
      shift_right();
      dump_lvdistr_title();
      subinfo_mask = ALL_INFO;

      /* For each PV in VG, see how much it is used by this LV */
      for (i = 0; i < pv_cnt; i++) {

	 if ((pv_key = pvpathtopvkey(pv_names[i])) == BAD_PV_KEY)
	    continue;

	 tot_used_lx = 0;
	 tot_used_px = 0;
         for (j = 0, lxp = lv_map; j < curlx_cnt; j++, lxp++) {
	    
            /* If needed, increase number of PX and LX present on that PV */
	    lx_is_on_pv = FALSE;
            for (k = 0; k < pxperlx_cnt; k++) {
	       /* Increase number of PX present on that PV */
               /* ULTRIX/OSF: modified lxp[j][k] to lxp[0][k] because
                *        the for statement above bumps both j and
                *        lxp which is a double hit on the array index.
                *        Also added a check for "in_use". A pv_key of
                *        0 is valid, but 0 is also the initialized 
                *        pv_key value in the array.
                */
	       if (lxp[0][k].pv_key == pv_key && lxp[0][k].in_use) {
		  lx_is_on_pv = TRUE;
		  tot_used_px++;
	       }
            }
	    if (lx_is_on_pv)
	       tot_used_lx++;
         }

	 /* Do we actually use this PV? If not, don't print anything */
	 if (tot_used_px > 0) {
	    if (dump_lvdistr(subinfo_mask, pv_names[i], tot_used_lx,
		     tot_used_px) != OK) {
	       debug(dbg_exit());
	       return(NOT_OK);
            }
	 }
      }
      shift_left();
   }

   /* Additional info required? */
   if (info_mask & bit(P_LV_LX_INFO)) {
      shift_right();
      dump_lx_title();
      subinfo_mask = ALL_INFO;
      for (i = 0, lxp = lv_map; i < curlx_cnt; i++, lxp++) {
	 if (dump_lx(i, lxp, subinfo_mask, pxperlx_cnt) != OK) {
	    debug(dbg_exit());
	    return(NOT_OK);
         }
      }
      shift_left();
   }

   debug(dbg_exit());
   return(OK);
}



static int
dump_queried_pv(struct lv_querypvpath *qpvp, unsigned int info_mask,
		char *vg_path, int vg_fd, int px_size)
{
   unsigned int subinfo_mask;
   px_descr_t *pv_map;
   int px_cnt;
   char **lv_names;
   int lv_cnt;
   register px_descr_t *pxp;
   register int i, j, k;
   register unsigned short lv_minor;
   register int lx_index;
   int tot_used_lv;
   int tot_used_lx;
   int tot_used_px;
   int tot_stale_px;

   debug(dbg_entry("dump_queried_pv"));

   /* Do we have to dump some information that we can get only from the map? */
   if (info_mask & (bit(P_PV_PX_INFO) |
	            bit(P_PV_LV_INFO) |
                    bit(P_PV_CUR_LV) |
	            bit(P_PV_STALE_PX))) {

      /* Get the map of PV */
      if (getpvmap(vg_fd, vg_path, qpvp->pv_key, &pv_map, &px_cnt) != OK) {
	 print_prgname();
         fprintf(stderr, MSG_PV_MAP_NOT_READ, qpvp->path);
	 debug(dbg_exit());
         return(NOT_OK);
      }

      /* Set up the lv_minor <-> lv_name conversion */
      if (lvminors_and_names(vg_fd, vg_path, &lv_names, &lv_cnt) != OK) {
	 print_prgname();
         fprintf(stderr, MSG_CANT_GET_LV_NAMES, vg_path);
	 debug(dbg_exit());
	 return(NOT_OK);
      }
   }

   /* Very dumb, but lengthy */
   if (info_mask & bit(P_PV_NAME))
      dump_value(pv_fmt, P_PV_NAME, qpvp->path);
   if (info_mask & bit(P_PV_VGNAME))
      dump_value(pv_fmt, P_PV_VGNAME, vg_path);
   if (info_mask & bit(P_PV_STATUS))
      dump_value(pv_fmt, P_PV_STATUS, (qpvp->pv_flags & LVM_PVMISSING) ?
	       MSG_PV_UNAVAILABLE : MSG_PV_AVAILABLE);
   if (info_mask & bit(P_PV_ALLOC))
      dump_value(pv_fmt, P_PV_ALLOC, (qpvp->pv_flags & LVM_PVNOALLOC) ?
	       MSG_NO : MSG_YES);

   if (info_mask & bit(P_PV_CUR_LV)) {
      /* For each LV in VG, see if it uses this PV */
      tot_used_lv = 0;
      for (i = 0; i < lv_cnt; i++) {

         if ((lv_minor = lvpathtolvminor(lv_names[i])) == BAD_LV_MINOR)
            continue;
      
         for (j = 0, pxp = pv_map; j < px_cnt; j++, pxp++) {
            if (pxp->lv_min == lv_minor) {
               tot_used_lv++;
	       break;
	    }
	 }
      }
      dump_value(pv_fmt, P_PV_CUR_LV, tot_used_lv);
   }

   if (info_mask & bit(P_PV_VGDA_CNT))
      dump_value(pv_fmt, P_PV_VGDA_CNT, (qpvp->pv_flags & LVM_NOVGDA) ?
	       0 : 2);
   if (info_mask & bit(P_PV_PX_SIZE))
      dump_value(pv_fmt, P_PV_PX_SIZE, px_size);
   if (info_mask & bit(P_PV_PX_CNT))
      dump_value(pv_fmt, P_PV_PX_CNT, qpvp->px_count);
   if (info_mask & bit(P_PV_FREE_PX))
      dump_value(pv_fmt, P_PV_FREE_PX, qpvp->px_free);
   if (info_mask & bit(P_PV_USED_PX))
      dump_value(pv_fmt, P_PV_USED_PX, qpvp->px_count - qpvp->px_free);

   if (info_mask & bit(P_PV_STALE_PX)) {

      /* Count stale PX */
      tot_stale_px = 0;
      for (i = 0, pxp = pv_map; i < px_cnt; i++, pxp++) {
         if (pxp->status & LVM_PXSTALE)
            tot_stale_px++;
      }
      dump_value(pv_fmt, P_PV_STALE_PX, tot_stale_px);
   }

   /* Do actual print */
   if (pv_title) {
      print_header(PV_GROUP, pv_fmt, entries(pv_fmt));
      pv_title = FALSE;
   }
   print_out(PV_GROUP, pv_fmt, entries(pv_fmt));

   /* Additional info required? */
   if (info_mask & bit(P_PV_LV_INFO)) {
      shift_right();
      dump_pvdistr_title();
      subinfo_mask = ALL_INFO;

      /* For each LV in VG, see how much it is used by this LV */
      for (i = 0; i < lv_cnt; i++) {

	 if ((lv_minor = lvpathtolvminor(lv_names[i])) == BAD_LV_MINOR)
	    continue;
	 
	 tot_used_lx = 0;
	 tot_used_px = 0;
         for (j = 0, pxp = pv_map; j < px_cnt; j++) {

	    /* If needed, increase number of PX used by that LV */
	    if (pxp[j].lv_min == lv_minor) {
	       tot_used_px++;

	       /*
		*   Scan the previous PX, to see if we met this LX before
		*   This algorithm is awful, but the other way is to get
		*   the LV map; as you probably noticed, the information
		*   about LVM is spread in different places
		*/

	       lx_index = pxp[j].lx_index;
               for (k = 0; k < j; k++) {
                  if (pxp[k].lv_min == lv_minor &&
			  pxp[k].lx_index == lx_index)
	             break;
	       }

	       /* If we didn't break the loop, this is a not yet counted LX */
	       if (k == j)
                  tot_used_lx++;
	    }
         }

	 /* Does this LV use some PX of this PV? */
	 if (tot_used_lx > 0)
	    if (dump_pvdistr(subinfo_mask, lv_names[i], tot_used_lx,
		     tot_used_px) != OK) {
	       debug(dbg_exit());
	       return(NOT_OK);
            }
      }
      shift_left();
   }

   /* Additional info required? */
   if (info_mask & bit(P_PV_PX_INFO)) {
      shift_right();
      dump_px_title();
      subinfo_mask = ALL_INFO;
      for (i = 0, pxp = pv_map; i < px_cnt; i++, pxp++) {
	 if (dump_px(i, pxp, subinfo_mask) != OK) {
	    debug(dbg_exit());
	    return(NOT_OK);
         }
      }
      shift_left();
   }

   debug(dbg_exit());
   return(OK);
}



static int
dump_lx(int lx_idx, lx_descr_t *lxp, unsigned int info_mask, int pxperlx_cnt)
{
   char *cp;

   debug(dbg_entry("dump_lx"));

   /* As usual, "trivial and lengthy" */
   if (pxperlx_cnt > 0) {
      if (info_mask & bit(P_LX_ID))
         dump_value(lx_fmt, P_LX_ID, lx_idx);
      if (info_mask & bit(P_LX_PV1))
         dump_value(lx_fmt, P_LX_PV1,
		  (cp = pvkeytopvpath(lxp[0][0].pv_key)) == NULL? "???":cp);
      if (info_mask & bit(P_LX_PX1))
         dump_value(lx_fmt, P_LX_PX1, lxp[0][0].px_index);
      if (info_mask & bit(P_LX_STAT1))
         dump_value(lx_fmt, P_LX_STAT1, lx_status(lxp[0][0].status));
   }

   if (pxperlx_cnt > 1) {
      if (info_mask & bit(P_LX_PV2))
         dump_value(lx_fmt, P_LX_PV2,
		  (cp = pvkeytopvpath(lxp[0][1].pv_key)) == NULL? "???":cp);
      if (info_mask & bit(P_LX_PX2))
         dump_value(lx_fmt, P_LX_PX2, lxp[0][1].px_index);
      if (info_mask & bit(P_LX_STAT2))
         dump_value(lx_fmt, P_LX_STAT2, lx_status(lxp[0][1].status));
   }

   if (pxperlx_cnt > 2) {
      if (info_mask & bit(P_LX_PV3))
         dump_value(lx_fmt, P_LX_PV3,
		  (cp = pvkeytopvpath(lxp[0][2].pv_key)) == NULL? "???":cp);
      if (info_mask & bit(P_LX_PX3))
         dump_value(lx_fmt, P_LX_PX3, lxp[0][2].px_index);
      if (info_mask & bit(P_LX_STAT3))
         dump_value(lx_fmt, P_LX_STAT3, lx_status(lxp[0][2].status));
   }

   if (lx_title) {
      print_header(LX_GROUP, lx_fmt, entries(lx_fmt));
      lx_title = FALSE;
   }
   print_out(LX_GROUP, lx_fmt, entries(lx_fmt));

   debug(dbg_exit());
   return(OK);
}



static int
dump_px(int px_idx, px_descr_t *pxp, unsigned int info_mask)
{
   char *cp;
   int free_px;

   debug(dbg_entry("dump_px"));

   free_px = (pxp->lv_min == 0);
   if (info_mask & bit(P_PX_ID))
      dump_value(px_fmt, P_PX_ID, px_idx);
   if (info_mask & bit(P_PX_STAT))
      dump_value(px_fmt, P_PX_STAT,
		  free_px ? MSG_PX_FREE :
		  (pxp->status & LVM_PXSTALE) ? MSG_PX_STALE : MSG_PX_CURRENT);
   if (info_mask & bit(P_PX_LV))
      dump_value(px_fmt, P_PX_LV,
		  free_px ? "" :
		  (cp = lvminortolvpath(pxp->lv_min)) == NULL? "???":cp);
   if (info_mask & bit(P_PX_LX))
      dump_value(px_fmt, P_PX_LX, pxp->lx_index);

   if (px_title) {
      print_header(PX_GROUP, px_fmt, entries(px_fmt));
      px_title = FALSE;
   }
   print_out(PX_GROUP, px_fmt, entries(px_fmt));

   debug(dbg_exit());
   return(OK);
}



static int
dump_lvdistr(unsigned int info_mask, char *pv_path, int tot_used_lx,
	 int tot_used_px)
{
   debug(dbg_entry("dump_lvdistr"));

   if (info_mask & bit(P_LVDISTR_PV))
      dump_value(lvdistr_fmt, P_LVDISTR_PV, pv_path);
   if (info_mask & bit(P_LVDISTR_LX))
      dump_value(lvdistr_fmt, P_LVDISTR_LX, tot_used_lx);
   if (info_mask & bit(P_LVDISTR_PX))
      dump_value(lvdistr_fmt, P_LVDISTR_PX, tot_used_px);

   if (lvdistr_title) {
      print_header(LVDISTR_GROUP, lvdistr_fmt, entries(lvdistr_fmt));
      lvdistr_title = FALSE;
   }
   print_out(LVDISTR_GROUP, lvdistr_fmt, entries(lvdistr_fmt));

   debug(dbg_exit());
   return(OK);
}



static int
dump_pvdistr(unsigned int info_mask, char *lv_path, int tot_used_lx,
	 int tot_used_px)
{
   debug(dbg_entry("dump_pvdistr"));

   if (info_mask & bit(P_PVDISTR_LV))
      dump_value(pvdistr_fmt, P_PVDISTR_LV, lv_path);
   if (info_mask & bit(P_PVDISTR_LX))
      dump_value(pvdistr_fmt, P_PVDISTR_LX, tot_used_lx);
   if (info_mask & bit(P_PVDISTR_PX))
      dump_value(pvdistr_fmt, P_PVDISTR_PX, tot_used_px);

   if (pvdistr_title) {
      print_header(PVDISTR_GROUP, pvdistr_fmt, entries(pvdistr_fmt));
      pvdistr_title = FALSE;
   }
   print_out(PVDISTR_GROUP, pvdistr_fmt, entries(pvdistr_fmt));

   debug(dbg_exit());
   return(OK);
}



static void
print_header(int group_id, print_fmt *table, int count)
{
   register int i;
   int print_fmt;
   char *general_hdr;

   debug(dbg_entry("print_header"));

   print_fmt = info_group_tab[group_id].print_mode;

   /* Print a general header */
   general_hdr = info_group_tab[group_id].general_header;
   do_nest();
   printf("--- %s ---", general_hdr);

   putchar('\n');

   if (print_fmt == TABLE_FMT) {
      do_nest();
      for (i = 0; i < count; i++, table++) {
         if (table->must_print) {
            printf("%-*.*s", table->out_size, table->out_size, table->header);
            putchar(' ');
         }
      }
      putchar('\n');
   }

   debug(dbg_exit());
}



static void
print_out(int group_id, print_fmt *table, int count)
{
   register int i;
   int print_fmt;

   debug(dbg_entry("print_out"));

   print_fmt = info_group_tab[group_id].print_mode;

   /* Depending on what has been chose as suitable, print the data */
   if (print_fmt == TABLE_FMT) {
      do_nest();
      for (i = 0; i < count; i++, table++) {
         if (table->must_print) {
            if (table->rm_lead_0s)
	       rm_lead_0s(table->info);
            printf(table->info);
            putchar(' ');
         }
      }
   }
   else {
      for (i = 0; i < count; i++, table++) {
         if (table->must_print) {
	    do_nest();
            if (table->rm_lead_0s)
	       rm_lead_0s(table->info);
            printf("%-*.*s %-*.*s\n", P_INFOMAX, P_INFOMAX, table->header,
				      P_INFOMAX2, P_INFOMAX2, table->info);
         }
      }
   }
   putchar('\n');

   debug(dbg_exit());
}



static void
rm_lead_0s(register char *str)
{
   register char *cp;
   register int l, i;

   l = strlen(str);

   /* Get to the first significant digit */
   for (cp = str; *cp == '0'; cp++)
      continue;

   /* If the number is made only of 0's, save one of them */
   if (*cp == '\0')
      cp--;

   /* Now, *cp is the first "good" digit; shift left all the "good" digits.  */
   for (i = 0; *cp != '\0'; i++, cp++)
      str[i] = *cp;

   /* Now, add trailing blanks */
   while (i < l)
      str[i++] = ' ';
   str[l] = '\0';
}



static void
shift_left()
{
   /* Nesting gets less deep */
   print_nesting--;
   if (print_nesting < 0) {
      print_nesting = 0;
      debug_msg("shift_left called more times than shift_right\n", NULL);
   }
   else
      putchar('\n');
}



static void
shift_right()
{
   /* Nesting gets deeper */
   print_nesting++;
}



static void
set_tables()
{
   register print_fmt *pfp;
   register info_group *igp;

   debug(dbg_entry("set_tables"));

   /* For each title string in the format tables, get the locale title */
   pfp = vg_fmt;
   set_header(pfp, P_VG_NAME, MSG_PP_VG_NAME);
   set_header(pfp, P_VG_STATUS, MSG_PP_VG_STATUS);
   set_header(pfp, P_VG_MAX_LV, MSG_PP_VG_MAX_LV);
   set_header(pfp, P_VG_CUR_LV, MSG_PP_VG_CUR_LV);
   set_header(pfp, P_VG_OPEN_LV, MSG_PP_VG_OPEN_LV);
   set_header(pfp, P_VG_MAX_PV, MSG_PP_VG_MAX_PV);
   set_header(pfp, P_VG_CUR_PV, MSG_PP_VG_CUR_PV);
   set_header(pfp, P_VG_ACT_PV, MSG_PP_VG_ACT_PV);
   set_header(pfp, P_VG_PX_SIZE, MSG_PP_VG_PX_SIZE);
   set_header(pfp, P_VG_MAX_PX_PER_PV, MSG_PP_VG_MAX_PX_PER_PV);
   set_header(pfp, P_VG_PX_CNT, MSG_PP_VG_PX_CNT);
   set_header(pfp, P_VG_USED_PX, MSG_PP_VG_USED_PX);
   set_header(pfp, P_VG_FREE_PX, MSG_PP_VG_FREE_PX);
   set_header(pfp, P_VG_VGDA_CNT, MSG_PP_VG_VGDA_CNT);

   pfp = lv_fmt;
   set_header(pfp, P_LV_NAME, MSG_PP_LV_NAME);
   set_header(pfp, P_LV_VGNAME, MSG_PP_LV_VGNAME);
   set_header(pfp, P_LV_PERM, MSG_PP_LV_PERM);
   set_header(pfp, P_LV_STATUS, MSG_PP_LV_STATUS);
   set_header(pfp, P_LV_WRITE_VER, MSG_PP_LV_WRITE_VER);
   set_header(pfp, P_LV_MIRRORS, MSG_PP_LV_MIRRORS);
   set_header(pfp, P_LV_SCHED, MSG_PP_LV_SCHED);
   set_header(pfp, P_LV_LX_CNT, MSG_PP_LV_LX_CNT);
   set_header(pfp, P_LV_USED_PX, MSG_PP_LV_USED_PX);
   set_header(pfp, P_LV_BBLOCK_POL, MSG_PP_LV_BBLOCK_POL);
   set_header(pfp, P_LV_ALLOC, MSG_PP_LV_ALLOC);
   set_header(pfp, P_LV_USED_PV, MSG_PP_LV_USED_PV);

   pfp = pv_fmt;
   set_header(pfp, P_PV_NAME, MSG_PP_PV_NAME);
   set_header(pfp, P_PV_VGNAME, MSG_PP_PV_VGNAME);
   set_header(pfp, P_PV_STATUS, MSG_PP_PV_STATUS);
   set_header(pfp, P_PV_ALLOC, MSG_PP_PV_ALLOC);
   set_header(pfp, P_PV_VGDA_CNT, MSG_PP_PV_VGDA_CNT);
   set_header(pfp, P_PV_CUR_LV, MSG_PP_PV_CUR_LV);
   set_header(pfp, P_PV_PX_SIZE, MSG_PP_PV_PX_SIZE);
   set_header(pfp, P_PV_PX_CNT, MSG_PP_PV_PX_CNT);
   set_header(pfp, P_PV_FREE_PX, MSG_PP_PV_FREE_PX);
   set_header(pfp, P_PV_USED_PX, MSG_PP_PV_USED_PX);
   set_header(pfp, P_PV_STALE_PX, MSG_PP_PV_STALE_PX);

   pfp = lx_fmt;
   set_header(pfp, P_LX_ID, MSG_PP_LX_ID);
   set_header(pfp, P_LX_PV1, MSG_PP_LX_PV1);
   set_header(pfp, P_LX_PX1, MSG_PP_LX_PX1);
   set_header(pfp, P_LX_STAT1, MSG_PP_LX_STAT1);
   set_header(pfp, P_LX_PV2, MSG_PP_LX_PV2);
   set_header(pfp, P_LX_PX2, MSG_PP_LX_PX2);
   set_header(pfp, P_LX_STAT2, MSG_PP_LX_STAT2);
   set_header(pfp, P_LX_PV3, MSG_PP_LX_PV3);
   set_header(pfp, P_LX_PX3, MSG_PP_LX_PX3);
   set_header(pfp, P_LX_STAT3, MSG_PP_LX_STAT3);

   pfp = px_fmt;
   set_header(pfp, P_PX_ID, MSG_PP_PX_ID);
   set_header(pfp, P_PX_STAT, MSG_PP_PX_STAT);
   set_header(pfp, P_PX_LV, MSG_PP_PX_LV);
   set_header(pfp, P_PX_LX, MSG_PP_PX_LX);

   pfp = lvdistr_fmt;
   set_header(pfp, P_LVDISTR_PV, MSG_PP_LVDISTR_PV);
   set_header(pfp, P_LVDISTR_LX, MSG_PP_LVDISTR_LX);
   set_header(pfp, P_LVDISTR_PX, MSG_PP_LVDISTR_PX);

   pfp = pvdistr_fmt;
   set_header(pfp, P_PVDISTR_LV, MSG_PP_PVDISTR_LV);
   set_header(pfp, P_PVDISTR_LX, MSG_PP_PVDISTR_LX);
   set_header(pfp, P_PVDISTR_PX, MSG_PP_PVDISTR_PX);

   igp = info_group_tab;
   set_g_hdr(igp, VG_GROUP, MSG_PVG_GROUP);
   set_g_hdr(igp, LV_GROUP, MSG_PLV_GROUP);
   set_g_hdr(igp, PV_GROUP, MSG_PPV_GROUP);
   set_g_hdr(igp, LX_GROUP, MSG_PLX_GROUP);
   set_g_hdr(igp, PX_GROUP, MSG_PPX_GROUP);
   set_g_hdr(igp, LVDISTR_GROUP, MSG_PLVDISTR_GROUP);
   set_g_hdr(igp, PVDISTR_GROUP, MSG_PPVDISTR_GROUP);

   debug(dbg_exit());
}

