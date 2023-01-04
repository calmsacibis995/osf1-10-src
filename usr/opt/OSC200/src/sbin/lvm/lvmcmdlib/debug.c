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
static char	*sccsid = "@(#)$RCSfile: debug.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:48:08 $";
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
 *   debug.c
 *   
 *   Contents:
 *
 *   This file containes many functions which only dumps differnt
 *   information on the stdout. This modules are only linked to
 *   the commands if -DDEBUG is specified on the command line when
 *   compiling a LVM command. If the -DDEBUG mechanism is not going
 *   to be used, this module don't have to be delivered or maintained.
 *
 *   BE ADVISED: all the code here contained is subject to #ifdef DEBUG;
 * 		 this means that if you turn off -DDEBUG in the Makefile,
 *		 this will generate an empty object file in the library
 *
 *   void dbg_entry(char *fname)
 *   void dbg_exit()
 *   void dbg_indent()
 *	Show a debug track of the function calls within a command.
 *
 *   void dbg_createvg_dump(struct lv_createvg *)
 *	Dumps all the elements of the struct lv_createvg.
 *
 *   void dbg_installpv_dump(struct lv_installpv *)
 *	Dumps all the elements of the struct lv_installpv.
 *
 *   void dbg_attachpv_dump(struct lv_attachpv *)
 *	Dumps all the elements of the struct lv_attachpv.
 *
 *   void dbg_queryvg_dump(struct lv_queryvg *, int mode)
 *	Dumps all the elements of the struct lv_queryvg.
 *
 *   void dbg_querylv_dump(struct lv_querylv *, int mode)
 *	Dumps all the elements of the struct lv_querylv.
 *
 *   void dbg_querypv_dump(struct lv_querylv *, int mode)
 *	Dumps all the elements of the struct lv_querypv.
 *
 *   void dbg_statuslv_dump(struct lv_statuslv * dmp_struct)
 *	Dumps all the elements of the struct lv_statuslv.
 *
 *   void dbg_changepv_dump(struct lv_changepv * dmp_struct)
 *	Dumps all the elements of the struct lv_changepv.
 *
 *   void dbg_lvID_dump(int * dmp_struct)
 *	Dumps all the elements of the (formerly) struct lv_lvID.
 *
 *   void dbg_pvID_dump(int * dmp_struct)
 *	Dumps all the elements of the (formerly) struct lv_pvID.
 *
 *   void dbg_querypvpath_dump(struct lv_querypvpath * dmp_struct, int mode)
 *	Dumps all the elements of the struct lv_querypvpath.
 *
 *   void dbg_lvsize_dump(struct lv_lvsize * dmp_struct, int mode)
 *	Dumps all the elements of the struct lv_lvsize.
 *
 *   void dbg_querypvmap_dump(struct lv_querypvmap * dmp_struct, int mode)
 *	Dumps all the elements of the struct lv_querypvmap.
 *
 *   void dbg_uniqueID_dump(lv_uniqueID_t * dmp_struct)
 *	Dumps all the elements of the struct lv_uniqueID_t.
 *
 *   void dbg_activatevg_dump(int * dmp_struct)
 *	Dumps all the elements of the struct lv_activatevg.
 *
 *   void dbg_lxmap_dump(char *msg, struct lxmap *lxp, int cnt)
 *	Dumps a logical extents extend/reduce map.
 *
 *   void dbg_pxmap_dump(char *msg, struct pxmap *pxp, int cnt)
 *	Dumps a physical extents map.
 *
 *   int dummy_ioctl(int fd, int request, char *argp)
 *	Returns 0; used to avoid accessing the driver.
 *
 *   void dbg_removepv_dump(unsigned short pv_key)
 *	Dumps all the elements of the struct lv_activatevg.
 */

#ifdef DEBUG

#include "lvmcmds.h"

/* Debug prints are nested as the routine calls get nested */
#define DEBUG_NEST	"   "
#define DEBUG_BANNER	"[dbg] "
#define IOCTL_BANNER	"[iAD] " /* ioctl Arguments Dump: to "grep" them */

#define dbg_ioctl_dump()	(dumping_ioctl_args = TRUE)

static void dbg_lxmap_dump(char *msg, struct lxmap *lxp, int cnt);
static void dbg_pxmap_dump(char *msg, struct pxmap *pxp, int cnt);
static int nesting = 0;
static int dumping_ioctl_args = FALSE;



void
dbg_entry(char *fname)
{
   nesting++;
   debug_msg("Entering function %s\n", fname);
}



void
dbg_exit()
{
   nesting--;
   if (nesting < 0) {
      debug_msg("(((dbg_exit: unbalanced calls)))\n", NULL);
      nesting = 0;
   }

   /* Sure we're stopping this */
   dumping_ioctl_args = FALSE;
}



void
dbg_indent()
{
   register int i;

   for (i = 0; i < nesting; i++)
      fputs(DEBUG_NEST, debugfile);
   fputs(dumping_ioctl_args ? IOCTL_BANNER : DEBUG_BANNER, debugfile);
}



void
dbg_createvg_dump(struct lv_createvg * vgcr_struct)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_createvg_dump"));

   debug_msg("\tlv_createvg->path\t= %s\n", vgcr_struct->path);
   debug_msg("\tlv_createvg->vg_id.id1\t= %d\n", vgcr_struct->vg_id.id1);
   debug_msg("\tlv_createvg->vg_id.id2\t= %d\n", vgcr_struct->vg_id.id2);
   debug_msg("\tlv_createvg->pv_flags\t= %x\n", vgcr_struct->pv_flags);
   debug_msg("\tlv_createvg->maxlvs\t= %d\n", vgcr_struct->maxlvs);
   debug_msg("\tlv_createvg->maxpvs\t= %d\n", vgcr_struct->maxpvs);
   debug_msg("\tlv_createvg->maxpxs\t= %d\n", vgcr_struct->maxpxs);
   debug_msg("\tlv_createvg->pxsize\t= %d\n", vgcr_struct->pxsize);
   debug_msg("\tlv_createvg->pxspace\t= %d\n", vgcr_struct->pxspace);
   debug_msg("\tlv_createvg->maxdefects\t= %d\n", vgcr_struct->maxdefects);
   debug(dbg_exit());
}

void
dbg_installpv_dump(struct lv_installpv *pvinst_struct)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_installpv_dump"));

   debug_msg("\tlv_installpv->path\t= %s\n", pvinst_struct->path);
   debug_msg("\tlv_installpv->pxspace\t= %d\n", pvinst_struct->pxspace);
   debug_msg("\tlv_installpv->pv_flags\t= 0x%08x\n", pvinst_struct->pv_flags);
   debug_msg("\tlv_installpv->maxdefects\t= %d\n", pvinst_struct->maxdefects);
   debug(dbg_exit());
}

void
dbg_attachpv_dump(char *attachpv)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_attachpv_dump"));

   debug_msg("\tlv_attachpv: path\t= %s\n", attachpv);
   debug(dbg_exit());
}

void
dbg_queryvg_dump(struct lv_queryvg *vgqu_struct, int mode)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_queryvg_dump"));

   debug_msg("%s ioctl\n", (mode != DBG_AFTER) ? "before" : "after");
   debug_msg("\tlv_queryvg->maxlvs\t= %d\n", vgqu_struct->maxlvs);
   debug_msg("\tlv_queryvg->maxpvs\t= %d\n", vgqu_struct->maxpvs);
   debug_msg("\tlv_queryvg->pxsize\t= %d\n", vgqu_struct->pxsize);
   debug_msg("\tlv_queryvg->freepxs\t= %d\n", vgqu_struct->freepxs);
   debug_msg("\tlv_queryvg->cur_lvs\t= %d\n", vgqu_struct->cur_lvs);
   debug_msg("\tlv_queryvg->cur_pvs\t= %d\n", vgqu_struct->cur_pvs);
   debug_msg("\tlv_queryvg->status\t= 0x%08x\n", vgqu_struct->status);
   debug(dbg_exit());
}

void
dbg_querylv_dump(struct lv_querylv *qu_struct, int mode)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_querylv_dump"));

   debug_msg("%s ioctl\n", (mode != DBG_AFTER) ? "before" : "after");
   debug_msg("\tlv_querylv->minor_num\t= %d\n", qu_struct->minor_num);
   debug_msg("\tlv_querylv->numpxs\t= %d\n", qu_struct->numpxs);
   debug_msg("\tlv_querylv->numlxs\t= %d\n", qu_struct->numlxs);
   debug_msg("\tlv_querylv->maxlxs\t= %d\n", qu_struct->maxlxs);
   debug_msg("\tlv_querylv->lv_flags\t= 0x%08x\n", qu_struct->lv_flags);
   debug_msg("\tlv_querylv->sched_strat\t= %d\n", qu_struct->sched_strat);
   debug_msg("\tlv_querylv->maxmirrors\t= %d\n", qu_struct->maxmirrors);
   debug(dbg_exit());
}

void
dbg_querypv_dump(struct lv_querypv *qu_struct, int mode)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_querypv_dump"));

   debug_msg("%s ioctl\n", (mode != DBG_AFTER) ? "before" : "after");
   debug_msg("\tlv_querypv->pv_key\t= %d\n", qu_struct->pv_key);
   debug_msg("\tlv_querypv->pv_flags\t= 0x%08x\n", qu_struct->pv_flags);
   debug_msg("\tlv_querypv->px_count\t= %d\n", qu_struct->px_count);
   debug_msg("\tlv_querypv->px_free\t= %d\n", qu_struct->px_free);
   debug_msg("\tlv_querypv->px_space\t= %d\n", qu_struct->px_space);
   debug_msg("\tlv_querypv->pv_rdev\t= %d\n", qu_struct->pv_rdev);
   debug_msg("\tlv_querypv->maxdefects\t= %d\n", qu_struct->maxdefects);
   debug_msg("\tlv_querypv->bbpool_len\t= %d\n", qu_struct->bbpool_len);
   debug(dbg_exit());
}

void
dbg_statuslv_dump(struct lv_statuslv *dmp_struct)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_statuslv_dump"));

   debug_msg("\tlv_statuslv->minor_num\t= %d\n", dmp_struct->minor_num);
   debug_msg("\tlv_statuslv->maxlxs\t= %d\n", dmp_struct->maxlxs);
   debug_msg("\tlv_statuslv->lv_flags\t= 0x%08x\n", dmp_struct->lv_flags);
   debug_msg("\tlv_statuslv->sched_strat\t= %d\n", dmp_struct->sched_strat);
   debug_msg("\tlv_statuslv->maxmirrors\t= %d\n", dmp_struct->maxmirrors);
   debug(dbg_exit());
}



void
dbg_changepv_dump(struct lv_changepv *dmp_struct)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_changepv_dump"));

   debug_msg("\tlv_changepv->pv_key\t= %d\n", dmp_struct->pv_key);
   debug_msg("\tlv_changepv->pv_flags\t= 0x%08x\n", dmp_struct->pv_flags);
   debug_msg("\tlv_changepv->maxdefects\t= %d\n", dmp_struct->maxdefects);
   debug(dbg_exit());
}



void
dbg_lvID_dump(int *dmp_struct)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_lvID_dump"));

   debug_msg("\tlv_lvID: minor_num\t= %d\n", *dmp_struct);
   debug(dbg_exit());
}



void
dbg_pvID_dump(int *dmp_struct)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_pvID_dump"));

   debug_msg("\tlv_pvID: pv_key\t= %d\n", *dmp_struct);
   debug(dbg_exit());
}



void
dbg_querypvpath_dump(struct lv_querypvpath *dmp_struct, int mode)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_querypvpath_dump"));

   debug_msg("%s ioctl\n", (mode != DBG_AFTER) ? "before" : "after");
   debug_msg("\tlv_querypvpath->path\t= %s\n", dmp_struct->path);
   debug_msg("\tlv_querypvpath->pv_key\t= %d\n", dmp_struct->pv_key);
   debug_msg("\tlv_querypvpath->pv_flags\t= 0x%08x\n", dmp_struct->pv_flags);
   debug_msg("\tlv_querypvpath->px_count\t= %d\n", dmp_struct->px_count);
   debug_msg("\tlv_querypvpath->px_free\t= %d\n", dmp_struct->px_free);
   debug_msg("\tlv_querypvpath->px_space\t= %d\n", dmp_struct->px_space);
   debug_msg("\tlv_querypvpath->pv_rdev\t= %d\n", dmp_struct->pv_rdev);
   debug_msg("\tlv_querypvpath->maxdefects\t= %d\n", dmp_struct->maxdefects);
   debug_msg("\tlv_querypvpath->bbpool_len\t= %d\n", dmp_struct->bbpool_len);
   debug(dbg_exit());
}



void
dbg_lvsize_dump(struct lv_lvsize *dmp_struct, int mode, int want_map)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_lvsize_dump"));

   debug_msg("%s ioctl\n", (mode != DBG_AFTER) ? "before" : "after");
   debug_msg("\tlv_lvsize->minor_num\t= %d\n", dmp_struct->minor_num);
   debug_msg("\tlv_lvsize->size\t= %d\n", dmp_struct->size);
   if (want_map == DBG_WITH_MAP)
      dbg_lxmap_dump("\tlv_lvsize->extents", dmp_struct->extents,
	       dmp_struct->size);
   debug(dbg_exit());
}



static void
dbg_lxmap_dump(char *msg, struct lxmap *lxp, int cnt)
{
   register int i;

   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_lxmap_dump"));

   debug_msg("%s =\n", msg);
   debug_msg("", NULL);
   fprintf(debugfile, "\t %6.6s %8.8s %8.8s %8.8s %10.10s\n", "", "lx_num",
	    "pv_key", "px_num", "status");

   for (i = 0; i < cnt; i++, lxp++) {
      debug_msg("", NULL);
      fprintf(debugfile, "\t %5d] %8d %8d %8d 0x%08x\n", i, lxp->lx_num,
	       lxp->pv_key,
	       lxp->px_num, lxp->status);
   }
   debug(dbg_exit());
}



void
dbg_querypvmap_dump(struct lv_querypvmap *dmp_struct, int mode)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_querypvmap_dump"));

   debug_msg("%s ioctl\n", (mode != DBG_AFTER) ? "before" : "after");
   debug_msg("\tlv_querypvmap->pv_key\t= %d\n", dmp_struct->pv_key);
   debug_msg("\tlv_querypvmap->numpxs\t= %d\n", dmp_struct->numpxs);
   if (mode == DBG_AFTER)
      dbg_pxmap_dump("\tlv_querypvmap->map", dmp_struct->map,
	       dmp_struct->numpxs);
   debug(dbg_exit());
}



static void
dbg_pxmap_dump(char *msg, struct pxmap *pxp, int cnt)
{
   register int i;

   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_pxmap_dump"));

   debug_msg("%s =\n", msg);
   debug_msg("", NULL);
   fprintf(debugfile, "\t %6.6s %8.8s %8.8s %10.10s\n", "", "lv_minor",
	    "lv_extent", "status");

   for (i = 0; i < cnt; i++, pxp++) {
      debug_msg("", NULL);
      fprintf(debugfile, "\t %5d] %8d %8d 0x%08x\n", i, pxp->lv_minor,
	       pxp->lv_extent, pxp->status);
   }
   debug(dbg_exit());
}



void
dbg_uniqueID_dump(lv_uniqueID_t *dmp_struct)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_uniqueID_dump"));

   debug_msg("\tlv_uniqueID_t->id1\t= 0x%08x\n", dmp_struct->id1);
   debug_msg("\tlv_uniqueID_t->id2\t= 0x%08x\n", dmp_struct->id2);
   debug(dbg_exit());
}



void
dbg_activatevg_dump(int *dmp_struct)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_activatevg_dump"));

   debug_msg("\tlv_activatevg: flags\t= 0x%08x\n", *dmp_struct);
   debug(dbg_exit());
}



void
dbg_removepv_dump(unsigned short pv_key)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_removepv_dump"));

   debug_msg("\tlv_removepv: pv_key\t= %d\n", pv_key);
   debug(dbg_exit());
}
/* * * * * 	Template for next dbg_XXX_dump 
	YY = field
	XX = formats
	WW = type of structure (e.g., querylv)



void
dbg_WW_dump(struct lv_WW *dmp_struct)
{
   debug(dbg_ioctl_dump());
   debug(dbg_entry("dbg_WW_dump"));

   debug_msg("\tlv_WW->YY\t= %XX\n", dmp_struct->YY);
   debug(dbg_exit());
}
 * * * * * */

int
dummy_ioctl(int fd, int request, char *argp)
{
   FILE *fp;
   char *fname;
   char buf[50];
   static short first_time = TRUE;
   int ret_code;
   static char suffix[20];

   debug(dbg_entry("dummy_ioctl"));

   switch (request) {
      case LVM_QUERYPV: fname = "QUERYPV"; break;
      case LVM_QUERYPVPATH: fname = "QUERYPVPATH"; break;
      case LVM_QUERYPVMAP: fname = "QUERYPVMAP"; break;
      default:
	    errno = ENXIO;
	    debug(dbg_exit());
	    return(-1);
   }

   if (first_time) {
      first_time = FALSE;
      if (isatty(fileno(stdin))) {
         printf("*** Simulation step.\n");
         printf("*** Input suffix for file name \"%s\" to be read:\n", fname);
      }
      if (fgets(suffix, sizeof(suffix) - 1, stdin) == NULL) {
         debug_msg("debug.dummy_ioctl: can't read suffix from ", NULL);
         perror("(stdin)");
         debug(dbg_exit());
         return(-1);
      }
      suffix[strlen(suffix) - 1] = '\0';
      debug_msg("debug.dummy_ioctl: suffix is \"%s\"\n", suffix);
   }

   sprintf(buf, "%s.%s", fname, suffix);
   fname = buf;

   if ((fp = fopen(fname, "r")) == NULL) {
      debug_msg("debug.dummy_ioctl: can't open ", NULL);
      perror(fname);
      debug(dbg_exit());
      return(-1);
   }

   debug_msg("debug.dummy_ioctl: read from \"%s\"\n", fname);
   ret_code = fill_struct(fp, request, argp);

   fclose(fp);
   debug(dbg_exit());
   return(ret_code);
}



int
fill_struct(FILE *fp, int request, char *argp)
{
   struct lv_querypv *qpv;
   struct lv_querypvpath *qpvp;
   struct lv_querypvmap *qpvm;
   register int i, max;
   register pxmap_t *pxp;
   char PVid[100];
   int x1, x2, x3, x4, x5;

   debug(dbg_entry("fill_struct"));

   switch (request) {
      case LVM_QUERYPV: 
	    qpv = (struct lv_querypv *)argp;
	    sprintf(PVid, "PV.pv_key: %d", qpv->pv_key);
	    if (get_to(fp, PVid) != OK ||
	        getnum(fp, "%x", &x1) != OK ||
	        getnum(fp, "%d", &x2) != OK ||
	        getnum(fp, "%d", &x3) != OK ||
	        getnum(fp, "%d", &x4) != OK) {
	       debug(dbg_exit());
	       return(-1);
	    }
	    qpv->pv_flags = x1;
	    qpv->px_count = x2;
	    qpv->px_free = x3;
	    qpv->px_space = x4;
	 break;
      case LVM_QUERYPVPATH:
	    qpvp = (struct lv_querypvpath *)argp;
	    sprintf(PVid, "PV.path: %s", qpvp->path);
	    if (get_to(fp, PVid) != OK ||
		getnum(fp, "%d", &x1) != OK ||
	        getnum(fp, "%x", &x2) != OK ||
	        getnum(fp, "%d", &x3) != OK ||
	        getnum(fp, "%d", &x4) != OK) {
	       debug(dbg_exit());
	       return(-1);
	    }
	    qpvp->pv_key = x1;
	    qpvp->pv_flags = x2;
	    qpvp->px_count = x3;
	    qpvp->px_free = x4;
	 break;
      case LVM_QUERYPVMAP:
	    qpvm = (struct lv_querypvmap *)argp;
	    sprintf(PVid, "PV.pv_key: %d", qpvm->pv_key);
	    if (get_to(fp, PVid) != OK) {
               debug(dbg_exit());
               return(-1);
	    }
	    for (pxp = qpvm->map, i = 0, max = qpvm->numpxs;
		     i < max; i++, pxp++) {
	       if (getnum(fp, "%d", &x1) != OK ||
	           getnum(fp, "%d", &x2) != OK ||
	           getnum(fp, "%x", &x3) != OK) {
		  debug(dbg_exit());
	          return(-1);
	       }
	       pxp->lv_minor = x1;
	       pxp->lv_extent = x2;
	       pxp->status = x3;
	    }
	 break;
   }
   debug(dbg_exit());
   return(0);
}



int 
get_to(FILE *fp, char *line)
{
   char buf[100];

   debug(dbg_entry("get_to"));

   for (;;) {

      if (fgets(buf, sizeof(buf), fp) == NULL) {
         debug(dbg_exit());
	 return(NOT_OK);
      }
      buf[strlen(buf) - 1] = '\0';

      if (eq_string(buf, line))
	 break;
   }

   debug(dbg_exit());
   return(OK);
}



int
getnum(FILE *fp, char *fmt, int *var)
{
   int c;
   int got_digit;

   for (got_digit = FALSE; !got_digit; ) {
      switch (c = getc(fp)) {
         case EOF:
            return(NOT_OK);
         case '#':
	    while ((c = getc(fp)) != '\n')
	       if (c == EOF)
	          return(NOT_OK);
	    break;
         case '\n':
	    break;
         default:
	       if (isdigit(c)) {
		  got_digit = TRUE;
		  ungetc(c, fp);
	       }
	       else
		  return(NOT_OK);
      }
   }

   if (fscanf(fp, fmt, var) != 1)
      return(NOT_OK);
   while (getc(fp) != '\n')
      continue;
   return(OK);
}

#endif /* DEBUG */
