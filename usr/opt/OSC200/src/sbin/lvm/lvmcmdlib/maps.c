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
static char	*sccsid = "@(#)$RCSfile: maps.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:48:25 $";
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
 *   maps.c
 *   
 *   Contents:
 *	Functions to deal with the allocation maps of LV's and PV's;
 *	These functions access the device driver (via ioctl), and build
 *	a reasonable data structure for them.
 *
 *   int pvkeys_and_names(int vg_fd, char *vg_path, char ***pv_names,
 *		int *pv_cnt);
 *	Returns OK or NOT_OK. vg_fd must be an already open file descriptor
 *	for the vg_path group file; this routine builds an internal (internal
 *	to this module) map of PV path names to pv_keys, so that the 
 *	pvpathtopvkey() and pvkeytopvpath() functions can quickly work.
 *	Note that these functions are also needed for getpvmap().
 *	The triple pointer is set to point to the table of names.
 *	The number of entries is returned via pv_cnt.
 *
 *   unsigned short pvpathtopvkey(char *pv_path)
 *	Returns the key for the PV belonging to the VG that was specified
 *	when getpvkeys() was called, so that ioctl's that need the key can
 *	be used; note that the main purpose of this function is to allow
 *	an easy use of getpvmap(). BAD_PV_KEY is returned on error. This
 *	function can be called only after getpvkeys() has been called.
 *
 *   char *pvkeytopvpath(unsigned short pv_key)
 *	The dual function of the previous one. NULL is returned on error.
 *	The string returned is a newly allocated string, so the user can
 *	overwrite it, etc.
 *
 *   int getpvmap(int vg_fd, char *vg_path, unsigned short pv_key,
 *		  px_descr_t **pv_map, int *px_cnt)
 *	This function reads, stores into newly allocated memory, and returns,
 *	all of the information related to the allocation map of a PV;
 *	the PV is specified with its key; the owning VG is specified
 *	with the file descriptor of its opened "group" file, and its name;
 *	the function returns OK or NOT_OK; if OK is returned, then on output
 *	the pointers are set to:
 *	*px_cnt: number of PE's within the PV;
 *	*pv_map: points to a monodimensional matrix (whose dimension is
 *	[*px_cnt]) of px_descr_t entries, such that the caller
 *	can get all the inormation about a physical extent by simply referencing
 *	it with [px_index]).
 *	
 *   int lvminors_and_names(int vg_fd, char *vg_path, char ***lv_names,
 *		int *lv_cnt);
 *	Returns OK or NOT_OK. vg_fd must be an already open file descriptor
 *	for the vg_path group file; this routine builds an internal (internal
 *	to this module) map of LV path names to minor numbers, so that the 
 *	lvpathtolvminor() and lvminortolvpath() functions can quickly work.
 *	Note that these functions are also needed for getlvmap().
 *	The triple pointer is set to point to the table of names.
 *	The number of entries is returned via lv_cnt.
 *
 *   unsigned short lvpathtolvminor(char *lv_path)
 *	Returns the minor number for the LV belonging to the VG that was
 *	specified when getlvminors() was called, so that ioctl's that need it
 *	can be used; note that the main purpose of this function is to allow
 *	an easy use of getlvmap(). BAD_LV_MINOR is returned on error. This
 *	function can be called only after lvminors_and_names() has been called.
 *
 *   char *lvminortolvpath(unsigned short lv_minor)
 *	The dual function of the previous one. NULL is returned on error.
 *	The string returned is a newly allocated string, so the user can
 *	overwrite it, etc.
 *
 *   int getlvmap(int vg_fd, char *vg_path, unsigned short lv_minor,
 *		  lx_descr_t **lv_map, int *maxlx_cnt, int *curlx_cnt,
 *		  int *pxperlx_cnt)
 *	This function reads, stores into newly allocated memory, and returns,
 *	all of the information related to the allocation map of a LV;
 *	the LV is specified with its minor number; the owning VG is specified
 *	with the file descriptor of its opened "group" file, and its name;
 *	the function returns OK or NOT_OK; if OK is returned, then on output
 *	the pointers are set to:
 *	*curlx_cnt: number of LE's within the LV;
 *	*maxlx_cnt: maximum number of LE's allowed within the LV;
 *	*lx_cnt: number of LE's within the LV;
 *	*pxperlx_cnt: number of data copies of each LE within the LV:
 *		this number can be 0, 1, 2 or 3. (0 means the LV has no LE)
 *	*lv_map: points to an array of lx_descr_t entries, such that the caller
 *	can get all the inormation about a logical extent by simply referencing
 *	it with [lx_index][mirror_index]; see the definition of lx_descr_t.
 *	This array will contain a number of entries equal to the current 
 *	maximum value of allowed LE for the LV; this allows the caller to
 *	use this map for setting entries which are free, and call extend_lv()
 *	(see below) to get the LV bigger; or, by calling reduce_lv(), to get
 *	the LV smaller (in either mirrors count or LE count).
 *	
 *   int reduce_lv(int vg_fd, unsigned short lv_minor, lx_descr_t *lv_map,
 *		int mode, int old_size, int new_size, int invariant);
 *	Notifies the driver that a LV has to be reduced.
 *	Returns OK or NOT_OK; puts the last old_size - new_size LE's of the
 *	provided map in the structure required by the ioctl call; note
 *	that depending on "mode", the reduction can be either the
 *	number of mirrors or the number of LE's belonging to the LV.
 *	"invariant" is the size of the dimension that DOES not have to change.
 *
 *   int extend_lv(int vg_fd, unsigned short lv_minor, lx_descr_t *lv_map,
 *		int mode, int old_size, int new_size, int invariant);
 *	Notifies the driver that a LV has to be extended.
 *	Returns OK or NOT_OK; puts the last new_size - old_size LE's of the
 *	provided map in the structure required by the ioctl call; note
 *	that depending on "mode", the extension can be either the
 *	number of mirrors or the number of LE's belonging to the LV.
 *	"invariant" is the size of the dimension that DOES not have to change.
 */

#include "lvmcmds.h"

/* Structures for mapping pathnames to quicker handles */
typedef struct {
   char *path;
   unsigned short min;
} lv_conv;

typedef struct {
   char *path;
   unsigned short key;
} pv_conv;

/* Where we keep the conversion info */
static lv_conv *lv_conv_tab = NULL;
static int lv_tab_entries = 0;
static pv_conv *pv_conv_tab = NULL;
static int pv_tab_entries = 0;



int
pvkeys_and_names(int vg_fd, char *vg_path, char ***pv_names,
		 int *pv_cnt)
{
   char **pv_paths;
   int pv_asked, pv_tot;
   struct lv_querypvpath *querypv_array;
   char *pv_queried;
   register int i;
   register pv_conv *ptp;
   register struct lv_querypvpath *qpvp;

   debug(dbg_entry("pvkeys_and_names"));

   /* Some information has to be retrieved from the PV's */
   if (lvmtab_read() != OK ||
	    lvmtab_getpvnames(vg_path, &pv_paths, &pv_tot) != OK) {
      print_prgname();
      fprintf(stderr, MSG_NO_PVNAMES, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Clear out (safety) */
   pv_conv_tab = NULL;
   pv_tab_entries = 0;
   *pv_names = NULL;
   *pv_cnt = 0;

   /* Query PV one by one, so that we can save all the name-PV mapping */
   if (pv_tot > 0) {
      pv_asked = pv_tot;
      if ((pv_tot = multiple_query_pv(vg_fd, pv_paths, pv_asked, &querypv_array,
	       &pv_queried)) != pv_asked) {
         print_prgname();
         fprintf(stderr, MSG_QUERYPVS_FAILED);
      }

      debug_msg("after multiple_query_pv\n", NULL);

      /* Create the internal table */
      pv_conv_tab = (pv_conv *)checked_alloc(pv_tot * sizeof(pv_conv));
      pv_tab_entries = pv_tot;

      debug_msg("maps.pvkeys_and_names: build the table\n", NULL);

      /* Store the data */
      for (i = 0, ptp = pv_conv_tab, qpvp = querypv_array;
	   i < pv_asked;
	   i++, qpvp++) {

         debug_msg("\"%s\" was queried?\n", pv_paths[i]);
         debug_msg("\t%s\n", pv_queried[i] ? "yes" : "no");

	 /* Check if we have data about it */
	 if (!pv_queried[i])
	    continue;

	 ptp->path = pv_paths[i];
	 ptp->key = qpvp->pv_key;

	 debug_msg("path: \"%s\"\n", ptp->path);
	 debug_msg("key: %d\n", ptp->key);
	 ptp++;
      }

      /* Free memory allocated by multiple_query_pv() */
      free((char *)querypv_array);

      /* DON'T free the list of names returned by lvmtab_getpvnames()!! */
   }

   /* Return indirectly the list of names and the number of entries */
   *pv_names = pv_paths;
   *pv_cnt = pv_tot;
   debug(dbg_exit());
   return(OK);
}



unsigned short
pvpathtopvkey(char *pv_path)
{
   register int i;
   register pv_conv *ptp;

   debug(dbg_entry("pvpathtopvkey"));

   /* Linear search */
   for (i = 0, ptp = pv_conv_tab; i < pv_tab_entries; i++, ptp++) {
      if (eq_string(ptp->path, pv_path)) {
	 debug(dbg_exit());
	 return(ptp->key);
      }
   }
   debug(dbg_exit());
   return(BAD_PV_KEY);
}



char *
pvkeytopvpath(unsigned short pv_key)
{
   register int i;
   register pv_conv *ptp;

   debug(dbg_entry("pvkeytopvpath"));

   /* Linear search */
   for (i = 0, ptp = pv_conv_tab; i < pv_tab_entries; i++, ptp++) {
      if (pv_key == ptp->key) {
	 debug(dbg_exit());
	 return(ptp->path);
      }
   }
   debug(dbg_exit());
   return(NULL);
}



int
getpvmap(int vg_fd, char *vg_path, unsigned short pv_key,
	px_descr_t **pv_map, int *px_cnt)
{
   struct lv_querypv qpv;
   struct lv_querypvmap qpvmap;
   int px_count;
   px_descr_t *pv_descr;
   register px_descr_t *pdp;
   pxmap_t *driver_map;
   register pxmap_t *dmp;
   register int i;

   debug(dbg_entry("getpvmap"));

   /* Clean entry */
   *pv_map = NULL;
   *px_cnt = 0;

   /* Call the driver to know the number of PE's on this PV */
   qpv.pv_key = pv_key;
   if (ioctl(vg_fd, LVM_QUERYPV, &qpv) < 0) {
      print_prgname();
      fprintf(stderr, MSG_QUERYPV_FAILED);
      lvm_perror(LVM_QUERYPV);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Get the map */
   px_count = qpv.px_count;
   if (px_count == 0) {
      driver_map = NULL;
      pv_descr = NULL;
   }
   else {

      /* Allocate space so that the driver can store the map */
      driver_map = (pxmap_t *)checked_alloc(px_count * sizeof(pxmap_t));

      /* Call again the driver to get the map */
      qpvmap.pv_key = pv_key;
      qpvmap.numpxs = px_count;
      qpvmap.map = driver_map;

      if (ioctl(vg_fd, LVM_QUERYPVMAP, &qpvmap) < 0) {
         print_prgname();
         fprintf(stderr, MSG_QUERYPVMAP_FAILED);
         lvm_perror(LVM_QUERYPVMAP);
	 debug(dbg_exit());
         return(NOT_OK);
      }

      /*
       *   Allocate space for our 1-to-1 map; this might sound silly,
       *   since the structure returned by the driver currently matches
       *   perfectly our structure; but we feel like spending some
       *	CPU cycle, in the name of coherent architecture; actually,
       *   the main reason is that, in this way, we are independent of
       *   future changes of the driver.
       */

      pv_descr = (px_descr_t *)checked_alloc(px_count * sizeof(px_descr_t));
      for (i = 0, pdp = pv_descr, dmp = driver_map;
	    i < px_count; i++, pdp++, dmp++) {
         pdp->lv_min = dmp->lv_minor;
         pdp->lx_index = dmp->lv_extent;
         pdp->status = dmp->status;
      }
   }

   /* Store and return results */
   *pv_map = pv_descr;
   *px_cnt = px_count;
   debug(dbg_exit());
   return(OK);
}



int
lvminors_and_names(int vg_fd, char *vg_path, char ***lv_names,
		   int *lv_cnt)
{
   char **lv_paths;
   int lv_tot;
   struct lv_querylv *querylv_array;
   register int i;
   register lv_conv *ltp;
   register struct lv_querylv *qlvp;

   debug(dbg_entry("lvminors_and_names"));

   /* Some information has to be retrieved from the LV's */
   if (getlvnames(vg_path, &lv_paths, &lv_tot) != OK) {
      print_prgname();
      fprintf(stderr, MSG_NO_LVNAMES, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Clear out (safety) */
   lv_conv_tab = NULL;
   lv_tab_entries = 0;
   *lv_names = NULL;
   *lv_cnt = 0;

   /* Query LV one by one, so that we can save all the name-LV mapping */
   if (lv_tot > 0) {
      if (multiple_query_lv(vg_fd, lv_paths, lv_tot, &querylv_array) != OK) {
         print_prgname();
         fprintf(stderr, MSG_QUERYLVS_FAILED);
	 debug(dbg_exit());
         return(NOT_OK);
      }

      /* Create the internal table */
      lv_conv_tab = (lv_conv *)checked_alloc(lv_tot * sizeof(lv_conv));
      lv_tab_entries = lv_tot;

      debug_msg("maps.lvminors_and_names: build the table\n", NULL);

      /* Store the data */
      for (i = 0, ltp = lv_conv_tab, qlvp = querylv_array;
	   i < lv_tot;
	   i++, ltp++, qlvp++) {
	 ltp->path = lv_paths[i];
	 ltp->min = qlvp->minor_num;

	 debug_msg("path: \"%s\"\n", ltp->path);
	 debug_msg("min: %d\n", ltp->min);
      }

      /* Free memory allocated by multiple_query_lv() */
      free((char *)querylv_array);

      /* DON'T free the list of names returned by getlvnames()!! */
   }

   /* Return indirectly the list of names and the number of entries */
   *lv_names = lv_paths;
   *lv_cnt = lv_tot;
   debug(dbg_exit());
   return(OK);
}



unsigned short
lvpathtolvminor(char *lv_path)
{
   register int i;
   register lv_conv *ltp;

   debug(dbg_entry("lvpathtolvminor"));

   /* Linear search */
   for (i = 0, ltp = lv_conv_tab; i < lv_tab_entries; i++, ltp++) {
      if (eq_string(ltp->path, lv_path)) {
	 debug(dbg_exit());
	 return(ltp->min);
      }
   }
   debug(dbg_exit());
   return(BAD_LV_MINOR);
}



char *
lvminortolvpath(unsigned short lv_minor)
{
   register int i;
   register lv_conv *ltp;

   debug(dbg_entry("lvminortolvpath"));

   /* Linear search */
   for (i = 0, ltp = lv_conv_tab; i < lv_tab_entries; i++, ltp++) {
      if (lv_minor == ltp->min) {
	 debug(dbg_exit());
	 return(ltp->path);
      }
   }
   debug(dbg_exit());
   return(NULL);
}



int
getlvmap(int vg_fd, char *vg_path, unsigned short lv_minor,
	lx_descr_t **lv_map, int *maxlx_cnt, int *curlx_cnt, int *pxperlx_cnt)
{
   struct lv_querylv qlv;
   struct lv_lvsize qlvmap;
   int lx_count;
   int used_px;
   int max_lx_count;
   int pxperlx_count;
   lx_descr_t *lv_descr;
   register lx_copy_t *lcp;
   lxmap_t *driver_map;
   register lxmap_t *dmp;
   register int i, j;

   debug(dbg_entry("getlvmap"));

   /* Clean entry */
   *lv_map = NULL;
   *maxlx_cnt = 0;
   *curlx_cnt = 0;
   *pxperlx_cnt = 0;

   /* Call the driver to know the number of LE's on this LV */
   qlv.minor_num = lv_minor;

   if (ioctl(vg_fd, LVM_QUERYLV, &qlv) < 0) {
      print_prgname();
      fprintf(stderr, MSG_QUERYLV_FAILED_NONAME);
      lvm_perror(LVM_QUERYLV);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Get the map */
   lx_count = qlv.numlxs;
   used_px = qlv.numpxs;

   if (lx_count == 0) {
      driver_map = NULL;
   }
   else {

      /* Allocate space so that the driver can store the map */
      driver_map = (lxmap_t *)checked_alloc(used_px * sizeof(lxmap_t));

      /* Call again the driver to get the map */
      qlvmap.minor_num = lv_minor;
      qlvmap.size = used_px;
      qlvmap.extents = driver_map;

      if (ioctl(vg_fd, LVM_QUERYLVMAP, &qlvmap) < 0) {
         print_prgname();
         fprintf(stderr, MSG_QUERYLVMAP_FAILED);
         lvm_perror(LVM_QUERYLVMAP);
	 debug(dbg_exit());
         return(NOT_OK);
      }
   }

   /*
    *   Don't forget to store the number of PE per LE; the driver returns
    *   0..2, that is, the number of mirrors; but, in case the LV is
    *   completely empty (just created, not extended), the 0 means
    *   "no PE at all"
    */

   pxperlx_count = qlv.maxmirrors;
   if (lx_count > 0)
      pxperlx_count++;

   /*   Allocate space for our map. We want the index to be the LE number */
   max_lx_count = qlv.maxlxs;

   lv_descr = (lx_descr_t *)checked_alloc(max_lx_count * sizeof(lx_descr_t));
   for (i = 0, dmp = driver_map; i < used_px; i++, dmp++) {

      debug_msg("maps.getlvmap: LE %s bound\n", 
	       (dmp->lx_num < max_lx_count) ? "in" : "OUT OF");
      debug_msg("(LE      %d\n", dmp->lx_num);
      debug_msg(" maxlxs: %d)\n", max_lx_count);

      /* Search for the next free mirror for this LE */
      for (j = 0, lcp = &lv_descr[dmp->lx_num][0]; j < LVM_MAXCOPIES;
	       j++, lcp++)
	 if (!lcp->in_use)
	    break;

      /* Check for inconsistency */
      debug_msg("maps.getlvmap: %s than LVM_MAXCOPIES for \n",
	       (j < LVM_MAXCOPIES) ? "less" : "more");
      debug_msg("LE %d\n", dmp->lx_num);
      debug((j < LVM_MAXCOPIES) ? NULL : (j = LVM_MAXCOPIES - 1));

      /* Dump the data there */
      lcp->pv_key = dmp->pv_key;
      lcp->px_index = dmp->px_num;
      lcp->status = dmp->status;
      lcp->in_use = TRUE;
   }

   /* Store and return results */
   *lv_map = lv_descr;
   *maxlx_cnt = max_lx_count;
   *curlx_cnt = lx_count;
   *pxperlx_cnt = pxperlx_count;
   debug(dbg_exit());
   return(OK);
}



int
reduce_lv(int vg_fd, unsigned short lv_minor, lx_descr_t *lv_map, int mode,
	  int old_size, int new_size, int invariant)
{
   register lx_copy_t *lcp;
   register lx_descr_t *ldp;
   int dmap_lx_entries;
   lxmap_t *change_map;
   register lxmap_t *chmp;
   struct lv_lvsize lvsize;
   register int i, j;

   debug(dbg_entry("reduce_lv"));

   /* Check input param's, to some extent */
   if (mode != REMOVE_LV_MIRRORS && mode != REMOVE_LV_EXTENTS ||
	    old_size <= new_size) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Allocate the structure that has to be passed to ioctl */
   dmap_lx_entries = (old_size - new_size) * invariant;
   change_map = (lxmap_t *)checked_alloc(dmap_lx_entries * sizeof(lxmap_t));

   /* Decrement the two sizes, so that they can be used as indexes */
   old_size--;
   new_size--;

   /* While filling in the flat structure for the driver, use a pointer */
   chmp = change_map;

   debug_msg("maps.reduce_lv\n", NULL);

   /* Do the job; either reduce one dimension or the other */
   if (mode == REMOVE_LV_MIRRORS) {

      /* invariant is the number of LE's; scan them all */
      for (i = 0; i < invariant; i++) {

	 /* Free the mirrors from the last valid backward */
	 for (j = old_size, lcp = &lv_map[i][j]; j > new_size; j--, lcp--) {
	    chmp->lx_num = i;
	    chmp->pv_key = lcp->pv_key;
	    chmp->px_num = lcp->px_index;

	    debug_msg("free LE %d\n", i);
	    debug_msg("    mir %d\n", j);
	    debug_msg("    pv  %d\n", lcp->pv_key);
	    debug_msg("    px  %d\n", lcp->px_index);

	    /* chmp->status is meaningless, in this case */

	    /* Move to the next entry in the flat structure */
	    chmp++;
	 }
      }
   }
   else {
      /* mode == REMOVE_LV_EXTENTS */

      /* Free the extents from the last valid backward */
      for (i = old_size; i > new_size; i--) {

	 /* Free all the PE's that this LE had */
	 for (j = 0, lcp = &lv_map[i][j]; j < invariant; j++, lcp++) {
	    chmp->lx_num = i;
	    chmp->pv_key = lcp->pv_key;
	    chmp->px_num = lcp->px_index;
	    /* chmp->status is meaningless, in this case */

	    debug_msg("free LE %d\n", i);
	    debug_msg("    mir %d\n", j);
	    debug_msg("    pv  %d\n", lcp->pv_key);
	    debug_msg("    px  %d\n", lcp->px_index);

	    /* Move to the next entry in the flat structure */
	    chmp++;
	 }
      }
   }

   /* Prepare the stuff for the ioctl */
   lvsize.minor_num = lv_minor;
   lvsize.size = dmap_lx_entries;
   lvsize.extents = change_map;

   /* Call it */
   debug(dbg_lvsize_dump(&lvsize, DBG_BEFORE, DBG_WITH_MAP));
   if (ioctl(vg_fd, LVM_REDUCELV, &lvsize) < 0) {
      print_prgname();
      fprintf(stderr, MSG_REDUCELV_FAILED);
      lvm_perror(LVM_REDUCELV);
      free((char *)change_map);
      debug_msg("ioctl(REDUCELV)\n", NULL);
      debug(dbg_exit());
      return(NOT_OK);
   }

   free((char *)change_map);
   debug(dbg_exit());
   return(OK);
}



int
extend_lv(int vg_fd, unsigned short lv_minor_num, lx_descr_t *lv_map,
   int old_lx_cnt, int old_mirr_cnt, int new_lx_cnt, int new_mirr_cnt)
{
   register lx_copy_t *lcp;
   register lx_descr_t *ldp;
   int dmap_lx_entries;
   lxmap_t *change_map;
   register lxmap_t *chmp;
   struct lv_lvsize lvsize;
   register int i, j;

   debug(dbg_entry("extend_lv"));

   /* Check input param's, to some extent */
   if ((old_lx_cnt >= new_lx_cnt) && (old_mirr_cnt >= new_mirr_cnt)) {
      debug(dbg_exit());
      debug_msg("Bad usage:\n", NULL);
      debug_msg("   old_lx_cnt:%d\n", old_lx_cnt);
      debug_msg("   new_lx_cnt:%d\n", new_lx_cnt);
      debug_msg("   old_mirr_cnt:%d\n", old_mirr_cnt);
      debug_msg("   new_mirr_cnt:%d\n", new_mirr_cnt);
      return(NOT_OK);
   }

   debug_msg("   old_lx_cnt:%d\n", old_lx_cnt);
   debug_msg("   new_lx_cnt:%d\n", new_lx_cnt);
   debug_msg("   old_mirr_cnt:%d\n", old_mirr_cnt);
   debug_msg("   new_mirr_cnt:%d\n", new_mirr_cnt);

   /* Allocate the structure that has to be passed to ioctl */
   dmap_lx_entries = (new_lx_cnt - old_lx_cnt) * (new_mirr_cnt - old_mirr_cnt);
   change_map = (lxmap_t *)checked_alloc(dmap_lx_entries * sizeof(lxmap_t));

   /* While filling in the flat structure for the driver, use a pointer */
   chmp = change_map;

   /* 
    * Add extents from the old_lx_cnt/old_mirr_cnt to the 
    * new_lx_cnt/new_mirr_cnt coordinates
    */
   for (i = old_lx_cnt; i < new_lx_cnt; i++) {
      for (j = old_mirr_cnt, lcp = &lv_map[i][j]; j < new_mirr_cnt; j++, lcp++) {
         chmp->lx_num = i;
         chmp->pv_key = lcp->pv_key;
         chmp->px_num = lcp->px_index;

         debug_msg("add LE  %d\n", i);
         debug_msg("    mir %d\n", j);
         debug_msg("    pv  %d\n", lcp->pv_key);
         debug_msg("    px  %d\n", lcp->px_index);

         /* chmp->status is meaningless, in this case */

         /* Move to the next entry in the flat structure */
         chmp++;
      }
   }

   /* Prepare the stuff for the ioctl */
   lvsize.minor_num = lv_minor_num;
   lvsize.size = dmap_lx_entries;
   lvsize.extents = change_map;

   /* Call it */
   debug(dbg_lvsize_dump(&lvsize, DBG_BEFORE, DBG_WITH_MAP));
   if (ioctl(vg_fd, LVM_EXTENDLV, &lvsize) < 0) {
      print_prgname();
      fprintf(stderr, MSG_EXTENDLV_FAILED);
      lvm_perror(LVM_EXTENDLV);
      debug_msg("ioctl(EXTENDLV)\n", NULL);
      free((char *)change_map);
      debug(dbg_exit());
      return(NOT_OK);
   }

   free((char *)change_map);
   debug(dbg_exit());
   return(OK);
}
