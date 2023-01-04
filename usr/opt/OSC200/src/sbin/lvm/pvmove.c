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
static char	*sccsid = "@(#)$RCSfile: pvmove.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/08/12 17:53:13 $";
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
 * pvmove.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */

/*
 *   pvmove:
 *   Moves allocated physical extents
 *   from one physical volume to one or more other physical volumes.
 */

/* Each file containing a main() has some privilege: see "lvmcmds.h" */
#define LVM_CMD_MAIN_FILE
#include "lvmcmds.h"

/*
 *   Here are all the declarations that are specific to this command,
 *   that is, file inclusions, definitions, variables, types, etc.
 */

/* Group together all the info about a LV involved in the relocation */
typedef struct {
   char *path;		/* clean path name */
   unsigned short min;	/* minor number */
   lx_descr_t *map;	/* allocation map */
   int maxlx_cnt;	/* dimensions of allocation map */
   int curlx_cnt;
   int pxperlx_cnt;
   int flags;		/* as returned by LVM_QUERYLV */
   int sched_strat;	/* as returned by LVM_QUERYLV */
   lxmap_t *reduce_map;	/* what we shall supply to LVM_REDUCELV */
   lxmap_t *extend_map;	/* what we shall supply to LVM_EXTENDLV */
   int changes_cnt;	/* how many entries are in these two arrays */
   int changes_index;	/* how many entries have been used */
} involved_lv_descr;

/* Group together all the info about a PV involved in the relocation */
typedef struct {
   char *path;		/* clean path name */
   unsigned short key;	/* PV key to communicate with the LVM driver */
   px_descr_t *map;	/* allocation map */
   int px_cnt;		/* number of PE's in PV */
   int free_px_cnt;	/* number of free PE's in PV */
   int missing;		/* status of the PV; if it is missing, it's unusable */
} involved_pv_descr;

/* VG owner of PV's involved; has to be set with some processing */
char *vg_path;
int vg_fd;

/* LV belonging to the same VG to which SourcePhysicalVolumePath belongs */
char **lv_in_vg_names;
int lv_in_vg_cnt;

/* PV belonging to the same VG to which SourcePhysicalVolumePath belongs */
char **pv_in_vg_names;
int pv_in_vg_cnt;

/* One is the source PV; dynamic is the number of dest PV's */
involved_pv_descr src_pv;
involved_pv_descr *dst_pv;
char **dst_pv_path;
int dst_pv_cnt;

/* List of LV's involved in the move */
involved_lv_descr *involved_lv;
int involved_lv_cnt;

/* Local functions */
static int check_usage_semantics();
static int set_defaults();
static int setup_tabs();
static int set_dest_pv();
static int set_involved_lv();
static int get_src_pv_map();
static int get_involved_lv_info(involved_lv_descr *lvdp);
static int relocate_lv_extents(involved_lv_descr *lvdp);
static lx_copy_t *get_new_le(involved_lv_descr *lvdp,
			    involved_pv_descr **destpvp, px_descr_t *pxp);
static involved_pv_descr *get_not_full_pv_map(
			    involved_pv_descr *last_returned_pv);
static void save_reloc_info(involved_lv_descr *lvdp, involved_pv_descr *pvdp,
			    px_descr_t *pxp, int px_index, lx_copy_t *new_le);
static int move_extents(involved_lv_descr *lvdp);
static int add_mirror(involved_lv_descr *lvdp);
static int remove_mirror(involved_lv_descr *lvdp);

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

#define USAGE	"Usage: pvmove  [-n LogicalVolumeName] \
 SourcePhysicalVolumePath [ DestinationPhysicalVolumePath... ]\n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	1
#define OPTIONS_WITH_VALUE	"n"
char nflag; char *LogicalVolumeName;

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	0
#define OPTIONS_WITHOUT_VALUE	""

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char *SourcePhysicalVolumePath;

/* Extra args (optional) */
char **DestinationPhysicalVolumePath;	/* maybe a list => array of pointers */
int DestinationPhysicalVolumePath_cnt;	/* number of items in the above array */



main(int argc, char **argv)
{
   register int lv_index;
   register involved_lv_descr *lvdp;

   /* Initialize the i18n (internationalization) support */
   msg_init();

   /* Set defaults specific to this command */
   set_defaults();

   /* See if the user typed a proper request */
   if (check_usage(argc, argv) != OK) {
      print_usage(USAGE);
      print_arg_error();
      exit(1);
   }

   init_debug();

   /* Get list of PV's in VG, etc. */
   if (setup_tabs() != OK)
      exit(FATAL_ERROR);

   /* Decide and/or check the grouyp of Dest PV */
   if (set_dest_pv() != OK)
      exit(FATAL_ERROR);

   /* Which LV's should be involved in the pvmove? */
   if (set_involved_lv() != OK)
      exit(FATAL_ERROR);

   /* Need the allocation map of the Src PV */
   if (get_src_pv_map() != OK)
      exit(FATAL_ERROR);

   /* 
    * Because of consistency reasons, the interupts are disabled 
    * from here on.
    */
   disable_intr();

   /* Now, try to figure out a "relocation" for each of the LV's involved */
   debug_msg("loop on get...relocat...extents\n", NULL);
   for (lv_index = 0, lvdp = involved_lv; lv_index < involved_lv_cnt;
	 lv_index++, lvdp++) {
      if (get_involved_lv_info(lvdp) != OK ||
          relocate_lv_extents(lvdp) != OK)
	 exit(FATAL_ERROR);
   }

   /* Now, try to actually "relocate" each of the LV's involved */
   debug_msg("loop on move_extents\n", NULL);
   for (lv_index = 0, lvdp = involved_lv; lv_index < involved_lv_cnt;
	 lv_index++, lvdp++) {
      if (move_extents(lvdp) != OK)
	 exit(FATAL_ERROR);
   }
   printf(MSG_PV_MOVED, SourcePhysicalVolumePath);


   /* Clean exit */
   return(0);
}



int
check_usage(int argc, char **argv)
{
   /* Call the general-purpose routine to check usage syntax */
   if (parse_args(&argc, &argv, OPTIONS_WITHOUT_VALUE, 
         OPTIONS_WITH_VALUE, REQ_ARGS_NUM) != OK)
      return(NOT_OK);

   /* Check whether usage syntax is correct */
   if (check_usage_syntax() != OK)
      return(NOT_OK);

   /* If we get to this point, usage syntax is correct */
   if (check_usage_semantics() != OK)
      return(NOT_OK);

   return(OK);
}



int
check_usage_syntax()
{
   register int i;

   /*
    *   See which options with value have been used;
    *   save a flag, and the value if supplied;
    *   whenever possible, check for correct usage
    *   of values for options
    */

   if (nflag = used_opt('n')) LogicalVolumeName = value_of_opt('n');

   /* Set references to mandatory arguments */
   SourcePhysicalVolumePath = next_arg();

   /* Set references to extra arguments */
   DestinationPhysicalVolumePath = left_arg(&DestinationPhysicalVolumePath_cnt);

   return(OK);
}



/*
 *   Here are all the subroutines that are specific to this command,
 *   that is, routines that don't fit into the LVM cmd's library
 */

static int
check_usage_semantics()
{
   register int i, l;
   register char **cpp;
   char *cp;
   char *src_pv_path;

   /* Since we're going to scan the name list, build a table of clean paths */
   src_pv_path = mk_save_clean_path(SourcePhysicalVolumePath);
   if ((dst_pv_cnt = DestinationPhysicalVolumePath_cnt) > 0) {
      dst_pv_path = (char **)checked_alloc(sizeof(char *) * dst_pv_cnt);

      /* Check that the Source is not in the list of Dest */
      for (i = 0, cpp = DestinationPhysicalVolumePath;
	       i < DestinationPhysicalVolumePath_cnt; i++, cpp++) {

         /* Store it, just in case the user is right... */
         dst_pv_path[i] = mk_save_clean_path(*cpp);

         if (eq_string(dst_pv_path[i], src_pv_path)) {
            usage_error("SourcePhysicalVolumePath", MSG_SRC_IN_DEST_SET);
	    return(NOT_OK);
         }
      }
   }
   else
      /* Remark that we have to extract the Dest from lvmtab */
      dst_pv_path = NULL;

   /* Nothing to complain about; store the name of the Src PV */
   src_pv.path = src_pv_path;
   return(OK);
}



static int
set_defaults()
{
}



static int
setup_tabs()
{
   /*
    *   Check if the source PV belongs to some VG; if dest. PV's have been
    *   supplied, do the same with them.
    */

   debug(dbg_entry("setup_tabs"));

   if (lvmtab_read() != OK) {
      print_prgname();
      fprintf(stderr, MSG_LVMTAB_READ_ERROR, LVMTABPATH);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Get to the owning VG */
   if (!lvmtab_ispvinsomevg(src_pv.path, &vg_path)) {
      print_prgname();
      fprintf(stderr, MSG_PV_NOT_IN_VG, src_pv.path, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* We have to open the "group" special file */
   if (check_and_openvg(vg_path, &vg_fd) == NULL) {
      print_prgname();
      fprintf(stderr, MSG_CANT_OPEN_VG, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Set-up the names to key translation for PV's; get the PV names */
   if (pvkeys_and_names(vg_fd, vg_path, &pv_in_vg_names, &pv_in_vg_cnt) != OK) {
      print_prgname();
      fprintf(stderr, MSG_CANT_GET_PV_NAMES, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Store the key of Src PV, since we'll need it often */
   src_pv.key = pvpathtopvkey(src_pv.path);

   /* Set-up the names to minors translation for LV's; get the LV names */
   if (lvminors_and_names(vg_fd, vg_path, &lv_in_vg_names, &lv_in_vg_cnt) !=
	    OK) {
      print_prgname();
      fprintf(stderr, MSG_CANT_GET_LV_NAMES, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   debug(dbg_exit());
   return(OK);
}



static int
set_dest_pv()
{
   register int i, j;
   register char **cpp, **npp;
   register involved_pv_descr *pdp;
   struct lv_querypvpath qpv;

   /*
    *   If they have been supplied, check that the Dest are in the VG;
    *   linear search ^ 2; note that dst_pv_path is either NULL or set
    *   to the list of clean path names.
    */

   debug(dbg_entry("set_dest_pv"));

   if (dst_pv_cnt > 0) {
      for (i = 0, cpp = dst_pv_path;
	       i < DestinationPhysicalVolumePath_cnt; i++, cpp++) {
   
         for (j = 0, npp = pv_in_vg_names; j < pv_in_vg_cnt; j++, npp++)
	    if (eq_string(*cpp, *npp))
	       break;
   
         if (j == pv_in_vg_cnt) {
	    /* Not found */
	    print_prgname();
	    fprintf(stderr, MSG_PV_NOT_IN_VG, *cpp, vg_path);
	    debug(dbg_exit());
	    return(NOT_OK);
         }
      }
   }
   else {

      /* Prepare the list of Dest PV with all the PV's in VG, but src_pv */
      dst_pv_cnt = pv_in_vg_cnt - 1;
      dst_pv_path = (char **)checked_alloc(sizeof(char *) * dst_pv_cnt);

      for (i = 0, cpp = dst_pv_path, npp = pv_in_vg_names;
	       i < pv_in_vg_cnt; i++, npp++) {

	 /* Skip the Src PV */
	 if (eq_string(src_pv.path, *npp))
	    continue;

	 /* Copy the string */
	 *cpp = checked_alloc(strlen(*npp) + 1);
	 strcpy(*cpp, *npp);

	 /* Get to the next entry */
	 cpp++;
      }
   }

   /*
    *   Wherever they come from (either the command line, or the list of
    *   PV's in VG, save the pointers to the names into the description
    *   table.
    *   Allocate the space, first.
    */

   dst_pv = (involved_pv_descr *)checked_alloc(sizeof(involved_pv_descr) *
						dst_pv_cnt);

   debug_msg("set_dest_pv: initialize the table of dest PV's:\n", NULL);

   /* Initialize each entry of the table describing the Dest PV's */
   for (i = 0, pdp = dst_pv; i < dst_pv_cnt; i++, pdp++) {

      pdp->path = dst_pv_path[i];
      pdp->key = pvpathtopvkey(pdp->path);

      /* Call the driver for some info, mainly the count of free PE */
      qpv.path = pdp->path;

      if (query_driver(vg_fd, LVM_QUERYPVPATH, &qpv) < 0) {
         print_prgname();
         fprintf(stderr, MSG_QUERYPVPATH_FAILED, pdp->path);
         lvm_perror(LVM_QUERYPVPATH);
	 debug(dbg_exit());
         return(NOT_OK);
      }

      if ((qpv.pv_flags & LVM_PVNOALLOC) == LVM_PVNOALLOC) {
         print_prgname();
         fprintf(stderr, MSG_CANT_ALLOC_ON_PV, pdp->path);
	 debug(dbg_exit());
         return(NOT_OK);
      }

      /* pdp->key could be checked/got also here */
      pdp->px_cnt = qpv.px_count;
      pdp->free_px_cnt = qpv.px_free;
      pdp->missing = ((qpv.pv_flags & LVM_PVMISSING) != 0);

      /* Warn the user */
      if (pdp->missing)
	 printf(MSG_WARN_PV_MISSING, pdp->path);

      debug_msg("[%2d]\n", i);
      debug_msg("  path \"%s\"\n", pdp->path);
      debug_msg("  key: %d\n", pdp->key);
      debug_msg("  free PE:  %d\n", pdp->free_px_cnt);
      debug_msg("  total PE: %d\n", pdp->px_cnt);
   }

   debug(dbg_exit());
   return(OK);
}



static int
set_involved_lv()
{
   char *vgforlv;
   char **involved_lv_names;
   register involved_lv_descr *lvdp;
   register int i;

   debug(dbg_entry("set_involved_lv"));

   /* LV names have already be read in by setup_tables() */

   /* Any LV in VG? */
   if (lv_in_vg_cnt == 0) {
      print_prgname();
      fprintf(stderr, MSG_NOLV_IN_VG, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* If a LV was supplied, check if it belongs to this VG */
   if (nflag) {
      if (lvtovg(LogicalVolumeName, &vgforlv) != OK ||
	       !eq_string(vg_path, vgforlv)) {
	 print_prgname();
	 debug_msg("lvtovg -> %s\n", vgforlv ? vgforlv : "NULL");
	 fprintf(stderr, MSG_LV_NOT_IN_VG, LogicalVolumeName, vg_path);
	 debug(dbg_exit());
	 return(NOT_OK);
      }

      /* Good. So the list must contain only this one */
      involved_lv_names = &LogicalVolumeName;
      involved_lv_cnt = 1;
   }
   else {
      /* All the LV's are involved */
      involved_lv_names = lv_in_vg_names;
      involved_lv_cnt = lv_in_vg_cnt;
   }

   /* Allocate the structures describing the LV involved */
   involved_lv = (involved_lv_descr *)checked_alloc(involved_lv_cnt * 
						sizeof(involved_lv_descr));

   debug_msg("set_involved_lv: initialize the table:\n", NULL);

   /* Store the names and the minor numbers */
   for (i = 0, lvdp = involved_lv; i < involved_lv_cnt; i++, lvdp++) {
      lvdp->path = involved_lv_names[i];
      lvdp->min = lvpathtolvminor(lvdp->path);

      debug_msg("[%d]\n", i);
      debug_msg("  path %s\n", lvdp->path);
      debug_msg("  min %d\n", lvdp->min);
   }

   debug(dbg_exit());
   return(OK);
}



static int
get_src_pv_map()
{
   /* The VG is already open, the key-name is set-up; just get the map */
   debug(dbg_entry("get_src_pv_map"));

   if (getpvmap(vg_fd, vg_path, src_pv.key,
	           &src_pv.map, &src_pv.px_cnt) != OK) {
      print_prgname();
      fprintf(stderr, MSG_PV_MAP_NOT_READ, src_pv.path);
      debug(dbg_exit());
      return(NOT_OK);
   }
   debug(dbg_exit());
   return(OK);
}



static int
get_involved_lv_info(involved_lv_descr *lvdp)
{
   struct lv_querylv qlv;

   debug(dbg_entry("get_involved_lv_info"));

   /* Call the library function for getting the allocation map */
   if (getlvmap(vg_fd, vg_path, lvdp->min, (lx_descr_t **)&lvdp->map, &lvdp->maxlx_cnt,
            &lvdp->curlx_cnt, &lvdp->pxperlx_cnt) != OK) {

      print_prgname();
      fprintf(stderr, MSG_LV_MAP_NOT_READ, lvdp->path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Call ioctl, to query some policy info on the LV */
   qlv.minor_num = lvdp->min;

   if (query_driver(vg_fd, LVM_QUERYLV, &qlv) < 0) {
      print_prgname();
      fprintf(stderr, MSG_QUERYLV_FAILED, lvdp->path);
      lvm_perror(LVM_QUERYLV);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Store what we need */
   lvdp->flags = qlv.lv_flags;
   lvdp->sched_strat = qlv.sched_strat;

   debug(dbg_exit());
   return(OK);
}



static int
relocate_lv_extents(involved_lv_descr *lvdp)
{
   register int i;
   register int changes_cnt;
   px_descr_t *pxp;
   lx_copy_t *new_le;
   involved_pv_descr *destpvp;

   debug(dbg_entry("relocate_lv_extents"));

   /* Scan the Src PV; count how many PE are allocated to this LV */
   changes_cnt = 0;
   for (pxp = src_pv.map, i = 0; i < src_pv.px_cnt; i++, pxp++) {

      /* Is it the same LV? */
      if (pxp->lv_min == lvdp->min)
	 changes_cnt++;
   }

   /* Store the computed number in the description of the LV */
   lvdp->changes_cnt = changes_cnt;
   lvdp->changes_index = 0;
   lvdp->reduce_map = NULL;
   lvdp->extend_map = NULL;

   debug_msg("relocate_lv_extents: LV %s has \n", lvdp->path);
   debug_msg("%d LE on Src PV\n", changes_cnt);

   /* Maybe we don't have nothing to do */
   if (changes_cnt == 0) {
      debug(dbg_exit());
      return(OK);
   }

   /* Allocate space for the remapping information */
   lvdp->reduce_map = (lxmap_t *)checked_alloc(changes_cnt * sizeof(lxmap_t));
   lvdp->extend_map = (lxmap_t *)checked_alloc(changes_cnt * sizeof(lxmap_t));

   debug_msg("relocate_lv_extents: moving \"%s\" from src PV\n", lvdp->path);

   /* Scan again the PV, trying to figure out relocation of LE's */
   for (pxp = src_pv.map, i = 0; i < src_pv.px_cnt; i++, pxp++) {

      /* Is it the same LV? */
      if (pxp->lv_min == lvdp->min) {

	 /* Needs relocation */
	 if ((new_le = get_new_le(lvdp, &destpvp, pxp)) == NULL) {
	    print_prgname();
	    fprintf(stderr, MSG_NO_PE_MOVE, pxp->lx_index, lvdp->path);
	    debug(dbg_exit());
	    return(NOT_OK);
	 }

	 debug_msg("PE %d\n", i);
	 debug_msg("  (LE %d) is moved to\n", pxp->lx_index);
	 debug_msg("  PV %d\n", new_le->pv_key);
	 debug_msg("  (\"%s\")\n", destpvp->path);
	 debug_msg("  PE %d\n", new_le->px_index);

	 /*
	  *   Now we know where this piece of data might be moved; save the
	  *   info, because we'll do all of the relocation when we know
	  *   whether we can do it all or not.
	  */
	 
	 save_reloc_info(lvdp, destpvp, pxp, i, new_le);
      }
   }
      
   debug(dbg_exit());
   return(OK);
}



static lx_copy_t *
get_new_le(involved_lv_descr *lvdp, involved_pv_descr **destpvp,
	   px_descr_t *pxp)
{
   static lx_copy_t lx_buf;
   lx_copy_t *lxp;
   register int i;
   int px_max;
   int free_px_index;
   register px_descr_t *map_ptr;
   register unsigned short min;
   register unsigned short lx_idx;
   register involved_pv_descr *pdp;
   involved_pv_descr *last_returned_pv;

   debug(dbg_entry("get_new_le"));

   lxp = NULL;
   *destpvp = NULL;

   /*
    *   Get room for a LE from the pool of Dest PV's, so that the PE described
    *   by pxp can be moved there; we have to scan the map(s) of some of the
    *   Dest PV's, taking care of the (possibly set) strictness of allocation.
    */

   last_returned_pv = NULL;
   do {

      /* Get a PV that has some space free */
      if ((pdp = get_not_full_pv_map(last_returned_pv)) == NULL) {
	 print_prgname();
         fprintf(stderr, MSG_NOT_ENOUGH_FREE_PX, lvdp->path);
	 debug(dbg_exit());
         return(NULL);
      }
      last_returned_pv = pdp;

      /* Assume there's no free PE on this PV */
      free_px_index = -1;

      /*
       *   Now, depending on strictness requirements, we search for a free PE;
       *   we do more or less the same thing in both branches of the "if",
       *   so that one scan is enough: notice that if strictness is required,
       *   we have to scan all the PV to make sure that it is ok.
       */

      if ((lvdp->flags & LVM_STRICT) != 0) {

         min = pxp->lv_min;
         lx_idx = pxp->lx_index;

	 /*
	  *   Search for one free PE, and check that no other PE belongs
	  *   to the same LE.
	  */

         for (map_ptr = pdp->map, i = 0, px_max = pdp->px_cnt; 
	          i < px_max; i++, map_ptr++) {

	    /* Free PE? */
	    if (free_px_index == -1 && map_ptr->lv_min == 0)
	       free_px_index = i;

	    /* Copy of the LE we are trying to relocate? */
	    if (map_ptr->lv_min == min && map_ptr->lx_index == lx_idx) {

	       /* Not good for us; we might have already found a PE */
	       free_px_index = -1;
	       break;
	    }
	 }
      }
      else {

	 /* Just search for one free PE */
         for (map_ptr = pdp->map, i = 0, px_max = pdp->px_cnt; 
	          i < px_max; i++, map_ptr++) {

	    /* Free PE? */
	    if (map_ptr->lv_min == 0) {
	       free_px_index = i;
	       break;
	    }
	 }
      }

      /* Found one suitable PE? */
      if (free_px_index != -1) {
	 lxp = &lx_buf;
	 lxp->pv_key = pdp->key;
	 lxp->px_index = free_px_index;
	 *destpvp = pdp;
      }

   } while (lxp == NULL);

   debug(dbg_exit());
   return(lxp);
}



static involved_pv_descr *
get_not_full_pv_map(involved_pv_descr *last_returned_pv)
{
   register involved_pv_descr *pdp;
   register int i;
   int px;

   debug(dbg_entry("get_not_full_pv_map"));

   /*
    *   To avoid looping on non-full PV's, which are unsuitable for
    *   the caller (because of strict allocation policy), we allow the
    *   caller to supply us with the starting point of the search
    *   (which is linear)
    */

   if (last_returned_pv == NULL) {
      i = 0;
      pdp = dst_pv;
   }
   else {
      pdp = last_returned_pv + 1;
      i = pdp - dst_pv;
   }

   /* Scan the descriptors of the Dest PV's, loading the map if necessary */
   for (; i < dst_pv_cnt; i++, pdp++) {

      /* Skip those unavailable, or full */
      if (pdp->missing || pdp->free_px_cnt == 0)
	 continue;

      /* Got one: need loading the map? */
      if (pdp->map == NULL) {

	 /* Yes. Do it, if possible; if not, complain but try the next one */
	 if (getpvmap(vg_fd, vg_path, pdp->key, &pdp->map, &px) != OK) {
            print_prgname();
            fprintf(stderr, MSG_PV_MAP_NOT_READ, pdp->path);
	    continue;
	 }
	 else {
	    
	    /* A quick check... */
	    debug_msg("get_not_full_pv_map:\n", NULL);
	    debug_msg("   px (%d)\n", px);
	    debug_msg("   %s\n", (px == pdp->px_cnt) ? "==" : "!=");
	    debug_msg("   pdp->px_cnt (%d)\n", pdp->px_cnt);

	 }
      }

      debug(dbg_exit());
      return(pdp);
   }

   debug(dbg_exit());
   return(NULL);
}



static void
save_reloc_info(involved_lv_descr *lvdp, involved_pv_descr *pvdp,
		px_descr_t *pxp, int px_index, lx_copy_t *new_le)
{
   int chg_idx;
   lxmap_t *drivmap_ptr;
   px_descr_t *dest_px;

   debug(dbg_entry("save_reloc_info"));

   /* Just add this relocation info to that related to the involved LV */

   /*
    *   We will proceed like this:
    *   1) LVM_REDUCE, to remove all the LE from the Src PV
    *   2) LVM_EXTEND, to add those LE back, allocating them on some other
    *      PV
    *   3) Transfer the actual data
    *
    *   In order to do so, we store the information about the PE belonging
    *   to the Src PV into the array of LE to be given to LVM_REDUCELV, and
    *   the information about the (already found) new LE into the array of
    *   LE to be given to LVM_EXTENDLV.
    */

   /* Get an index of the next not-yet-used entry in the 2 relocation maps */
   chg_idx = lvdp->changes_index++;

   /* Save the info for the reduction step */
   drivmap_ptr = &lvdp->reduce_map[chg_idx];
   drivmap_ptr->lx_num = pxp->lx_index;
   drivmap_ptr->pv_key = src_pv.key;
   drivmap_ptr->px_num = px_index;
   drivmap_ptr->status = pxp->status;

   /* Save the info for the extension step */
   drivmap_ptr = &lvdp->extend_map[chg_idx];
   drivmap_ptr->lx_num = pxp->lx_index;
   drivmap_ptr->pv_key = new_le->pv_key;
   drivmap_ptr->px_num = new_le->px_index;
   drivmap_ptr->status = pxp->status;

   /*
    *   Record (in the in-memory copy of the PV map) that that PE has
    *   been allocated
    */
   dest_px = &pvdp->map[new_le->px_index];
   dest_px->lv_min = pxp->lv_min;
   dest_px->lx_index = pxp->lx_index;
   dest_px->status = pxp->status;

   debug(dbg_exit());
}



static int
move_extents(involved_lv_descr *lvdp)
{
   register int i;
   register int max;
   register lxmap_t *lxmp;
   register lxmap_t *reduce_lxmp;
   struct lv_lvsize lv_changesize;
   struct lv_resynclx lv_sync;
   register struct lv_resynclx *lsp;

   debug(dbg_entry("move_extents"));

   printf(MSG_MOVING_LV, lvdp->path);

   /*
    *   This might seem quite a strange algorithm, but we need to use
    *   the ioctl's which are available, so here is how it goes:
    *   1) if the source LE's have less than three mirrors, then:
    *           1. Add the destination physical volume as a mirror of the
    *              source physical volume.  This will automatically be set as
    *              STALE by the OS.
    *           2. Resync the logical extent, this will copy the source data
    *              to the destination physical extent.
    *           3. Remove the source physical extent from the logical extent.
    *   2) if the source LE's have three mirrors, then:
    *           1. Remove the source physical extent from the logical extent.
    *           2. Add the destination physical volume as a mirror of the
    *              source physical volume.  This will automatically be set as
    *              STALE by the OS.
    *           3. Resync the logical extent, this will copy the source data
    *              to the destination physical extent.
    *
    *	Unfortunately, there can be the case that all of the mirrors
    *	of the LE reside on the source PV (no strict allocation, e.g.);
    *	this has lead to the unpleasant decision that we need to call
    *   the LVM_REDUCELV, LVM_EXTENDLV and LVM_RESYNCLX ioctls for EACH
    *	mirror to be moved.
    */

   /* Try to speed up computation, when it's time for sync'ing each LE */
   lsp = &lv_sync;
   max = lvdp->changes_cnt;
   lsp->minor_num = lvdp->min;

   /* Set the fields which are common to both ways */
   lv_changesize.minor_num = lvdp->min;

   /* Implement the above described algorithms */
   if (lvdp->pxperlx_cnt < LVM_MAXCOPIES) {

      debug_msg("move_extents: algorithm 1\n", NULL);

      /* Add temporary mirror */
      if (add_mirror(lvdp) != OK) {
	 debug(dbg_exit());
	 return(NOT_OK);
      }

      lv_changesize.size = 1;

	/* Algorithm 1 was implemented as a single loop.  For each extend,
	 * the destination physical extend is added, resync, and the source
	 * physical extend is reduce.  If an error is encountered, the 
	 * old algrithm is difficult to recover from.
	 *
	 * For easier recovery, the loop is divided into three parts.
	 * The first loop adds all the destination physcial extends.
	 * The second loop resync, the destination physcial extends with
	 * the source physcial extends.  The third loop reduces the source
	 * physical extends.
	 *
	 * If at any time, an error is encountered, the spare mirror can
	 * be removed with minimal side effect.
	 *
	 * Algorithm 2 is not modified.  It is only possible to encounter
	 * algo 2, if mirroring is enabled.
	 */

	 /* Extend */
     for (i = 0, lxmp = lvdp->extend_map; i < max; i++, lxmp++) {
         debug(dbg_lvsize_dump(&lv_changesize, DBG_BEFORE, DBG_WITH_MAP));
     	 lv_changesize.extents = lxmp;
         if (ioctl(vg_fd, LVM_EXTENDLV, &lv_changesize) < 0) {
            print_prgname();
            fprintf(stderr, MSG_EXTENDLV_FAILED);
            lvm_perror(LVM_EXTENDLV);
            debug_msg("ioctl(EXTENDLV)\n", NULL);
	    debug(dbg_exit());

	    /* Extend failed.  Remove temp. mirror. */
	    remove_mirror(lvdp);
            return(NOT_OK);
         }
     }

     /* Resync */
     for (i = 0, lxmp = lvdp->extend_map; i < max; i++, lxmp++) {
     	/* Store number of LE to be sync'ed */
     	lsp->lx_num = lxmp->lx_num;
     	if (ioctl(vg_fd, LVM_RESYNCLX, lsp) < 0) {
            debug_msg("syncing entry %d\n", i);
            debug_msg("   LE %d: failure\n", lsp->lx_num);
            print_prgname();
            fprintf(stderr, MSG_RESYNCLX_FAILED);
                  lvm_perror(LVM_RESYNCLX);
            debug_msg("ioctl(RESYNCLX)\n", NULL);
            debug(dbg_exit());

	    /* Resync failed.  Remove temp. mirror before exit. */
     		/* Best Effort Reduce */
     		for (i = 0, lxmp = lvdp->extend_map; i < max; i++, lxmp++) {
     	 		lv_changesize.extents = lxmp;
         		ioctl(vg_fd, LVM_REDUCELV, &lv_changesize);
		}
	    	remove_mirror(lvdp);
            return(NOT_OK);
         }
     }

     /* Reduce */
     for (i = 0, reduce_lxmp=lvdp->reduce_map; i < max; i++,reduce_lxmp++) {
         debug(dbg_lvsize_dump(&lv_changesize, DBG_BEFORE, DBG_WITH_MAP));
     	 lv_changesize.extents = reduce_lxmp;
         if (ioctl(vg_fd, LVM_REDUCELV, &lv_changesize) < 0) {
            print_prgname();
            fprintf(stderr, MSG_REDUCELV_FAILED);
            lvm_perror(LVM_REDUCELV);
    	    debug_msg("ioctl(REDUCELV)\n", NULL);
	    debug(dbg_exit());

	    /* Reduce failed.  Remove temp mirror anyway. */
	    remove_mirror(lvdp);
            return(NOT_OK);
         }
      }
      
      /* Remove temporary mirror */
      if (remove_mirror(lvdp) != OK) {
	 debug(dbg_exit());
	 return(NOT_OK);
      }

      debug_msg("move_extents: algorithm 1 worked good\n", NULL);
   }
   else {

      debug_msg("move_extents: algorithm 2\n", NULL);
      lv_changesize.size = 1;

      for (i = 0, lxmp = lvdp->extend_map, reduce_lxmp = lvdp->reduce_map;
	       i < max; i++, lxmp++, reduce_lxmp++) {

         /* Reduce */
         lv_changesize.extents = reduce_lxmp;

         debug(dbg_lvsize_dump(&lv_changesize, DBG_BEFORE, DBG_WITH_MAP));
         if (ioctl(vg_fd, LVM_REDUCELV, &lv_changesize) < 0) {
            print_prgname();
            fprintf(stderr, MSG_REDUCELV_FAILED);
            lvm_perror(LVM_REDUCELV);
	    debug_msg("ioctl(REDUCELV)\n", NULL);
	    debug(dbg_exit());
            return(NOT_OK);
         }
      
         /* Extend */
         lv_changesize.extents = lxmp;

         debug(dbg_lvsize_dump(&lv_changesize, DBG_BEFORE, DBG_WITH_MAP));
         if (ioctl(vg_fd, LVM_EXTENDLV, &lv_changesize) < 0) {
            print_prgname();
            fprintf(stderr, MSG_EXTENDLV_FAILED);
            lvm_perror(LVM_EXTENDLV);
	    debug_msg("ioctl(EXTENDLV)\n", NULL);
	    debug(dbg_exit());
            return(NOT_OK);
         }

         /* Resync */

	 /* Store number of LE to be sync'ed */
	 lsp->lx_num = lxmp->lx_num;
	 if (ioctl(vg_fd, LVM_RESYNCLX, lsp) < 0) {

	    debug_msg("syncing entry %d\n", i);
	    debug_msg("   LE %d: failure\n", lsp->lx_num);

            print_prgname();
            fprintf(stderr, MSG_RESYNCLX_FAILED);
      	    lvm_perror(LVM_RESYNCLX);
	    debug_msg("ioctl(RESYNCLX)\n", NULL);
	    debug(dbg_exit());
            return(NOT_OK);
	 }
      }

      debug_msg("move_extents: algorithm 2 worked good\n", NULL);
   }

   debug(dbg_exit());
   return(OK);
}



static int
add_mirror(involved_lv_descr *lvdp)
{
   struct lv_statuslv statuslv;

   debug(dbg_entry("add_mirror"));

   statuslv.minor_num = lvdp->min;
   statuslv.maxlxs = lvdp->maxlx_cnt;
   statuslv.lv_flags = lvdp->flags;
   statuslv.sched_strat = lvdp->sched_strat;

   /* Remember that "maxmirrors" is in 0..2, whereas "pxperlx" is in 1..3 */
   statuslv.maxmirrors = lvdp->pxperlx_cnt;

   /* Dump struct statuslv if in debug mode */
   debug(dbg_statuslv_dump(&statuslv));

   if (ioctl(vg_fd, LVM_CHANGELV, &statuslv) < 0) {
      print_prgname();
      fprintf(stderr, MSG_LV_NOT_CHANGED, lvdp->path);
      lvm_perror(LVM_CHANGELV);
      debug_msg("ioctl(CHANGELV)\n", NULL);
      return(NOT_OK);
   }

   debug(dbg_exit());
   return(OK);
}



static int
remove_mirror(involved_lv_descr *lvdp)
{
   struct lv_statuslv statuslv;

   debug(dbg_entry("remove_mirror"));

   statuslv.minor_num = lvdp->min;
   statuslv.maxlxs = lvdp->maxlx_cnt;
   statuslv.lv_flags = lvdp->flags;
   statuslv.sched_strat = lvdp->sched_strat;

   /* Remember that "maxmirrors" is in 0..2, whereas "pxperlx" is in 1..3 */
   statuslv.maxmirrors = lvdp->pxperlx_cnt - 1;

   /* Dump struct statuslv if in debug mode */
   debug(dbg_statuslv_dump(&statuslv));

   if (ioctl(vg_fd, LVM_CHANGELV, &statuslv) < 0) {
      print_prgname();
      fprintf(stderr, MSG_LV_NOT_CHANGED, lvdp->path);
      lvm_perror(LVM_CHANGELV);
      debug_msg("ioctl(CHANGELV)\n", NULL);
      return(NOT_OK);
   }

   debug(dbg_exit());
   return(OK);
}


