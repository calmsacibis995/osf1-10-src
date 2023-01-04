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
static char	*sccsid = "@(#)$RCSfile: logvol.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/11/13 10:44:32 $";
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
 *   logvol.c
 *   
 *   Contents:
 *	Functions for dealing with logical volumes. Since logical volumes
 *	information is not stored in the lvmtab file, these functions
 *	cannot go in the lvmtab.c library module. But many functions are
 *	very similar (e.g., getlvnames) to those there defined
 *	(lvmtab_getpvnames); therefore, when a similar function is needed
 *	the interface is identical to that of the function defined in
 *	lvmtab.c
 *
 *   int getlvnames(char *vg_path, char ***name_list, int *items_in_list)
 *	Returns OK or NOT_OK. Allocates and initializes an array of strings
 *	containing the name of all the LV belonging to the specified volume
 *	group.
 *      Note that the returned table is a copy; this allows the caller to
 *	do whatever he wants to.
 *
 *   int lvtovg(char *lv_path, char **vg_path)
 *	Returns OK or NOT_OK; tries to get to the path of the VG owning
 *	the LV; if it gets to that, stores the address of a copy
 *	of the string containing the name of the VG indirectly.
 *
 *   int isalv(char *path)
 *	Returns TRUE or FALSE; checks if path is a special file (block)
 *	and some other things.
 *
 *   int openvg_and_querylv(char *lvpath, char **vgpath, int *vg_fd, 
 *	  	          unsigned short *lv_minor, struct lv_querylv *querylv)
 *	Returns OK or NOT_OK. Opens a volume group dependent on
 *	lvpath and queries the logical volume. 
 *	vgpath and vg_fd are set and the structure querylv is filled.
 *
 *   int cutdown_lv(int vg_fd, char *vgpath, unsigned short lv_minor_num,
 *		   char *lvpath, int op_type, int new_val,
 *		   int user_confirmation)
 *	Returns OK or NOT_OK. cutdown_lv() reads the logical volume map 
 *	and reduce it dependent on op_type. If op_type = REMOVE_LV_EXTENTS 
 *	logical extents will be removed, else mirrors will be removed.
 *	The number of elements to be reduced to is stored in "new_val". The 
 *	logical extents are removed from the end of the map, but useful data 
 *	might get lost. Therefor the user is asked for confirmation if the
 *	user_confirmation is set to ASK_USER.
 *
 *   int resync_lv(int vg_fd, unsigned short lv_minor)
 *	Resynchronises the stale LE of the given LV within the VG that
 *	can be accessed through the (already open) vg_fd parameter.
 *	Returns OK or NOT_OK;
 *
 *   int fill_lv_map(int vg_fd, char *vgpath, char *lvpath, 
 *		unsigned short lv_minor, unsigned short lv_flags, 
 *		lx_descr_t **lvmap, char **pvls, int pvl_cnt, 
 *		int mirr_cnt, int lx_cnt)
 *	fill_lv_map() can be called recursivly, but it presumes that the
 *	first time it's called it is to extend the size of the LV in
 *	number of extents.
 * 	fill_lv_map() reads (the first time called) the logical volume map 
 *	"lv_map" for the logical volume with the minor number "minor_num", 
 *	and then all physical volumes with the pv_keys belonging to the VG 
 *	"vgpath". fill_lv_map() 
 *	then reads the physical volume mapfrom the available physical volumes, 
 *	and reserves space for the new logical extents in the logical volume 
 * 	map (lv_map). lv_map has room for maxlx_cnt of logical extents each 
 *	containing LVM_MAXCOPIES mirrors, but contains currenetly curlx_cnt 
 *	logical extents with "pxperlx_cnt" physical extents per logical volume.
 *	In case of a STRICT allocation policy, the operation of finding 
 *	physical extents which can be allocated to a logical volume, is very 
 *	slow since it has to be verified that the new physical extent doesn't 
 *	belong to the same physical volume as any of the other physical extents
 *	of that logical extent.
 *	A pointer to the filled lv_map is passed to the caller in the
 *	lvmap argument.
 *
 *   local_getpvmap(int vg_fd, char *vgpath, char **pvs, px_descr_t **pv_map,
 * 		int *px_cnt, unsigned short *pv_key)
 * 	local_getpvmap() reads the physical map from one of the PVs in pvs.
 * 	If the physical volume has the state "no allocation" (LVM_PVNOALLOC)
 * 	the next PV on pvs is read.
 * 	If the getpvmap() fails local_getpvmap() tries to read the map from the 
 *	next PV in pvs.
 *
 *   check_lv_map(lx_descr_t *lv_map, int total_mirr, int start, int end)
 * 	check_lv_map() checkes if all of the logical extents in lv_map are
 * 	allocated. The field "in_use" in lv_map will say wheter the entry is
 * 	allocated (TRUE) or not (FALSE). OK is returned if they are, else NOT_OK.
 *
 *   extend_lv_map(int vg_fd, unsigned short lv_minor, char *lvpath, 
 *		lx_descr_t *lv_map, int old_lx_cnt, int old_mirr_cnt, 
 *		int new_lx_cnt, int new_mirr_cnt)
 *	extend_lv_map gets a pointer to a filled lv_map passed in the
 *	lv_map argument. After doing some initialization, it calles
 *	exetend_lv() which actually extends the map.
 *
 */

#include "lvmcmds.h"



int
getlvnames(char *vg_path, char ***name_list, int *items_in_list)
{
   unsigned int num_of_lv;
   register char *fname;
   register char *lvname;
   char namebuf[PATH_MAX];
   char *cleanpath;
   register unsigned int lvname_len;
   register unsigned int vgname_len;
   char **name_table;
   DIR *dirp;
   struct dirent *dp;

   debug(dbg_entry("getlvnames"));

   /* "Standardise" the path */
   cleanpath = mk_clean_path(vg_path);
   if (cleanpath == NULL) {
      print_prgname();
      fprintf(stderr, MSG_NO_CLEAN_PATH, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }
   vgname_len = strlen(cleanpath);

   /* Use library functions to open the directory */
   dirp = opendir(cleanpath);
   if (dirp == NULL) {

      /* Probably wrong VG name: clear output parameters, anyway */
      *name_list = NULL;
      *items_in_list = 0;
      print_prgname();
      fprintf(stderr, MSG_OPENDIR, cleanpath);
      perror("");
      debug(dbg_exit());
      return(NOT_OK);
   }

   debug_msg("logvol.getlvnames: looking for LV's (\"%s\")\n",
	    cleanpath);

   /* Scan the directory, skipping what's useless */
   num_of_lv = 0;
   name_table = NULL;
   while ((dp = readdir(dirp)) != NULL) {

      /* Store the name in a register, so that it gets faster */
      fname = dp->d_name;

      debug_msg("\texamining \"%s\"...\n", fname);

      /* See if it can be an LV: it should be a block device */
      if (eq_string(fname, ".") || eq_string(fname, "..") ||
	  eq_string(fname, GROUP))
	 continue;

      strcpy(namebuf, cleanpath);
      namebuf[vgname_len] = '/';
      strcpy(&namebuf[vgname_len + 1], fname);

      if (!isalv(namebuf))
	 continue;

      /*
       *   It's an LV (probably); add a new pointer to the table.
       *   Allocate space to store the name: "/dev/vgXX" (cleanpath)
       *   plus 1 for '/', plus lvname_len, plus 1 for '\0'
       */

      lvname_len = strlen(fname);
      lvname = checked_alloc(vgname_len + 1 + lvname_len + 1);
      strcpy(lvname, namebuf);

      name_table = (char **)checked_realloc((char *)name_table,
					    num_of_lv * sizeof(char *), 
	       			 	    (num_of_lv + 1) * sizeof(char *));

      debug_msg("\tfound \"%s\"\n", lvname);

      /* Store it in the table */
      name_table[num_of_lv++] = lvname;
   }
   closedir(dirp);

   /*
    *   Allocate an extra slot, so that we can store a NULL
    *   for the end of the list.
    */

   name_table = (char **)checked_realloc((char *)name_table,
					 num_of_lv * sizeof(char *), 
	       				 (num_of_lv + 1) * sizeof(char *));
   name_table[num_of_lv] = NULL;

   /* Store indirect returned values */
   *name_list = name_table;
   *items_in_list = num_of_lv;

   debug(dbg_exit());
   return(OK);
}



int
lvtovg(char *lv_path, char **vg_path)
{
   char *vg_name;
   char buf[PATH_MAX];
   char wdir[PATH_MAX];
   register int c;
   register int src_i, dst_i;
   register char *cp;

   debug(dbg_entry("lvtovg"));

   /* Set the indirect result, just in case... */
   *vg_path = NULL;

   debug_msg("\tcalled with \"%s\"\n", lv_path);

   /* Is it really a LV? */
   if (!isalv(lv_path)) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Skip sequences of "./" */
   while (lv_path[0] == '.' && lv_path[1] == '/')
      lv_path += 2;

   /* Remove sequences of "///", and "/./." while copying lv_path into buf */
   for (src_i = dst_i = 0; (c = lv_path[src_i]) != '\0'; ) {
      buf[dst_i++] = c;
      src_i++;
      if (c == '/') {
	 while (lv_path[src_i] == '/')
	    src_i++;

	 while (lv_path[src_i] == '.' && lv_path[src_i + 1] == '/')
	    src_i += 2;
      }
   }
   buf[dst_i] = '\0';

   /*
    *   Now, figure out what sort of VG does this belong to.
    *   Let vg_name point to something like "/dev/vgXX/lv...";
    *   later on we cut it to "/dev/vgXX", and allocate space for 
    *   the string to be returned.
    */

   /* First, see if it is an absolute pathname */
   if (strncmp(buf, DEVPATH, sizeof(DEVPATH) - 1) == 0) 
      vg_name = buf;
   else {
      
      /*
       *   Second, it might be just the name; something like "lv03", or
       *   "vgXX/lv03"
       */

      if (isalnum(buf[0])) {

	 /* Are we under "/" or "/dev" or "/dev/vgXX"? */
	 if (getcwd(wdir, sizeof(wdir)) == NULL) {
	    print_prgname();
	    perror(MSG_GETCWD);
	    debug_msg("(getcwd)\n", NULL);
	    debug(dbg_exit());
	    return(NOT_OK);
	 }

	 strcat(wdir, buf);
	 vg_name = wdir;
      }
      else {

	 /* Bad thing: something like (cwd: /dev/vgYY) ../vgXX/lv03 */
	 print_prgname();
	 fprintf(stderr, MSG_CANNOT_FIGURE_VG_FOR_LV, lv_path);
	 debug(dbg_exit());
	 return(NOT_OK);
      }
   }

   debug_msg("\tfull path of LV is \"%s\"\n", vg_name);

   /* Clean it up, and store it back; count the '/' (should be 3) */
   for (cp = vg_name, c = 0; *cp != '\0' && c != 3; )
      if (*cp++ == '/')
	 c++;

   /* Sanity check */
   if (c != 3) {
      print_prgname();
      fprintf(stderr, MSG_CANNOT_FIGURE_VG_FOR_LV, lv_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Seemed easy, in the beginning? Zero the 3rd '/' */
   --cp;
   *cp = '\0';
   *vg_path = checked_alloc(strlen(vg_name) + 1);
   strcpy(*vg_path, vg_name);

   debug_msg("\tVG for LV is \"%s\"\n", vg_name);

   debug(dbg_exit());
   return(OK);
}



int
isalv(char *path)
{
   dev_t devnum;

   debug(dbg_entry("isalv"));

   /* If it's not a block device, then it can't be an LV */
   if (special_f_tst(path, S_IFBLK, &devnum) == NOT_OK) {
      debug(dbg_exit());
      return(FALSE);
   }

   /* Check for range of minor number */
   if (!in_range(minor(devnum), 1, LVM_MAXLVS)) {
      debug(dbg_exit());
      return(FALSE);
   }

   /* Any other check not involving the owning VG might be useful */

   debug(dbg_exit());
   return(TRUE);
}

int
openvg_and_querylv(char *lvpath, char **vgpath, int *vg_fd, 
		   struct lv_querylv *querylv)
{
	char *vg_tmp_path;
   	dev_t lv_dev_num;
	unsigned short lv_minor;

	debug(dbg_entry("openvg_and_querylv"));

	/* Get the volume group path from the lvpath */
	if (lvtovg(lvpath, &vg_tmp_path) != OK) {
		print_prgname();
		fprintf(stderr, MSG_VG_PATH_NOT_GEN, lvpath);
		debug(dbg_exit());
		return(NOT_OK);
	}

	/*
    	 * check VolumeGroupName and open the VG control file.
 	 * check_and_openvg() prints the needed error messages.
	 */
       	if ((*vgpath = check_and_openvg(vg_tmp_path, vg_fd)) == NULL) {
	       debug(dbg_exit());
	       return(NOT_OK);
	}

	/* Get the minor number of the logical volume from LVDD */
   	if (special_f_tst(lvpath, S_IFBLK, &lv_dev_num) == NOT_OK) {
      		print_prgname();
		fprintf(stderr, MSG_CANT_GET_LV_MINOR, lvpath);
		debug(dbg_exit());
      		return(NOT_OK);
   	}
	lv_minor = minor(lv_dev_num);

	/* Get the current setting of the logical volume */
	querylv->minor_num = lv_minor;

	if (query_driver(*vg_fd, LVM_QUERYLV, querylv) == -1) {
		print_prgname();
		fprintf(stderr, MSG_LV_NOT_QUERIED, lvpath);
      		lvm_perror(LVM_QUERYLV);
		debug(dbg_exit());
		return(NOT_OK);
	}

	debug(dbg_exit());
	return(OK);
}


int 
cutdown_lv(int vg_fd, char *vgpath, unsigned short lv_minor_num, char *lvpath, 
           int op_type, int new_val, int user_confirmation)
{
   lx_descr_t *lv_map;
   int maxlx_cnt, curlx_cnt, pxperlx_cnt;
   char *yes, answer[10];

   debug(dbg_entry("cutdown_lv"));

   /* Get the current logical volume map */
   if (getlvmap(vg_fd, vgpath, lv_minor_num, &lv_map, &maxlx_cnt,
	   &curlx_cnt, &pxperlx_cnt) != OK) {
      print_prgname();
      fprintf(stderr, MSG_LV_MAP_NOT_READ, lvpath);
      debug(dbg_exit());
      return(NOT_OK);
   }

   if (op_type == REMOVE_LV_EXTENTS) {
      /* Remove some of the logical extents */

      /* If needed, ask for confirmation */
      if (user_confirmation == ASK_USER) {

         printf(MSG_USER_CONFIRMATION);
	 if (read_line(answer, sizeof(answer)) == EOF) {
            printf(MSG_LV_NOT_REDUCED, lvpath);
	    debug(dbg_exit());
	    return(OK);
         }

         /* Get the internationalized yes response */
         yes = nl_langinfo(YESSTR);
	 debug_msg("user said        \"%s\"\n", answer);
	 debug_msg("nl_langinfo says \"%s\"\n", yes);

         if (*yes != *answer) {
            printf(MSG_LV_NOT_REDUCED, lvpath);
	    debug(dbg_exit());
	    return(OK);
         }
      }
      if (reduce_lv(vg_fd, lv_minor_num, lv_map, op_type, curlx_cnt, new_val, 
	     pxperlx_cnt) != OK) {
	 print_prgname();
         fprintf(stderr, MSG_REDUCE_LX_FAILED, lvpath);
	 debug(dbg_exit());
	 return(NOT_OK);
      }
   }
   else {
      /* 
       * Remove mirrors from the logical extents.
       * In this case the new_val element is the new number of mirrors
       * which the LV should have after the reductions. Since the
       * reduce_lv() functions operates with "# of physical extents for
       * each logical extent" which is one higher then number of mirrors 
       * (i.e., # of physical extents per logical extents includes the
       * original, but number of mirrors doesn't), new_val has to be
       * incremented before calling reduce_lv().
       */
      if (reduce_lv(vg_fd, lv_minor_num, lv_map, op_type, pxperlx_cnt, 
	     new_val + 1, curlx_cnt) != OK) {
	 print_prgname();
         fprintf(stderr, MSG_REDUCE_MIRRORS_FAILED, lvpath);
	 debug(dbg_exit());
	 return(NOT_OK);
      }
   }
   printf(MSG_LV_REDUCED, lvpath);
   debug(dbg_exit());
   return(OK);
}



int
resync_lv(int vg_fd, unsigned short lv_minor)
{
   int resynclv;

   debug(dbg_entry("resync_lv"));

   /* Prepare the data for the system call */
   resynclv = lv_minor;
   if (ioctl(vg_fd, LVM_RESYNCLV, &resynclv) < 0) {
      print_prgname();
      fprintf(stderr, MSG_RESYNCLV_FAILED);
      lvm_perror(LVM_RESYNCLV);
      debug_msg("ioctl(RESYNCLV)\n", NULL);
      debug(dbg_exit());
      return(NOT_OK);
   }

   debug(dbg_exit());
   return(OK);
}



int
fill_lv_map(int vg_fd, char *vgpath, char *lvpath, unsigned short lv_minor_num,
  unsigned short lv_flags, lx_descr_t **lvmap, char **pvls, int pvl_cnt, 
  int mirr_cnt, int le_cnt)
{
   static px_descr_t *pv_map;	/* pv_map has to be static */
   lx_descr_t *lv_map;
   struct lv_querylv querylv;
   struct lv_statuslv statuslv;
   static char **pvs_in_vg;
   static int pvs_cnt, px_cnt;
   char **pvs;
   int maxlx_cnt, curlx_cnt, pxperlx_cnt;
   int i, j, k, start, end, mirr_start, total_mirr,  lx_cnt;
   int can_allocate, old_mirror_cnt;
   unsigned short px_index;
   unsigned short pv_key;
   static int first_time = TRUE;

   debug(dbg_entry("fill_lv_map"));


   /* Get the current lv_map the first time fill_lv_map is called */
   if (first_time) {
      if (getlvmap(vg_fd, vgpath, lv_minor_num, &lv_map, &maxlx_cnt, 
	   &curlx_cnt, &pxperlx_cnt) != OK) {
         print_prgname();
         fprintf(stderr, MSG_LV_MAP_NOT_READ, lvpath);
         debug(dbg_exit());
         return(NOT_OK);
      }
   }
   else {
      /* Get the current nr. of LX from the driver */
      querylv.minor_num = lv_minor_num;

      /* Dump struct statuslv if in debug mode */
      debug(dbg_querylv_dump(&querylv, DBG_BEFORE));

      if (ioctl(vg_fd, LVM_QUERYLV, &querylv) == -1) {
         print_prgname();
         fprintf(stderr, MSG_LV_NOT_QUERIED, lvpath);
         lvm_perror(LVM_QUERYLV);
         debug(dbg_exit());
         return(NOT_OK);
      }
      curlx_cnt = querylv.maxlxs;
      pxperlx_cnt = querylv.maxmirrors + 1;
      /* let lv_map point to the map alloacted last time called */
      lv_map  = *lvmap;
   }

   /* 
    * pvkeys_and_names() reads the pv_keys into memory (for later use
    * by mapping between pv_key and pv_path) and returns a list of 
    * physical volumes belonging to vgpath.
    */
   if (first_time && pvkeys_and_names(vg_fd, vgpath, &pvs_in_vg, &pvs_cnt) 
		!= OK) {
      print_prgname();
      fprintf(stderr, MSG_PV_NAMES_NOT_READ, vgpath);
      debug(dbg_exit());
      return(NOT_OK);
   }

   debug_msg("fill_lv_map: got %d PV's \n", pvs_cnt);
   debug_msg("for VG \"%s\": \n", vgpath);
   debug_msg("\"%s\"...\n", pvs_cnt > 0 ? pvs_in_vg[0] : "");

   /* 
    * If PhysicalVolumePath on command line, use those, else use the
    * PVs read from the VG.
    */
   if (pvl_cnt > 0)
      pvs = pvls;
   else
      pvs = pvs_in_vg;

   /* 
    * calculate where in lv_map to start/end the allocation and
    * the number of mirrors to add and the start of the first mirror.
    */
   if (mirr_cnt != 0) {
      start = 0;
      end = curlx_cnt;
      mirr_start = pxperlx_cnt;
      total_mirr = mirr_cnt + 1;
   }
   else {
      start = curlx_cnt;
      end = le_cnt;
      mirr_start = 0;
      /* 
       * If the lv_map didn't have any logical extents allocated on it
       * until now, total_mirr has to be set to 1 since at least 
       * one physical extent has to be allocated for each logical extent.
       */
      if (pxperlx_cnt == 0)
	 total_mirr = 1;
      else 
	 total_mirr = pxperlx_cnt;
   }

   debug_msg("extension: \n", NULL);
   debug_msg("\tstart      %d\n", start);
   debug_msg("\tend        %d\n", end);
   debug_msg("\tmirr_start %d\n", mirr_start);
   debug_msg("\ttotal_mirr %d\n", total_mirr);

   if (first_time) {
      first_time = FALSE;
      /* read the physical map the first time fill_lv_map is called */
      if (local_getpvmap(vg_fd, vgpath, pvs, lvpath, &pv_map, &px_cnt, 
		&pv_key) != OK){
         debug(dbg_exit());
         return(NOT_OK);
      }
   }

   debug_msg("fill_lv_map: start allocating\n", NULL);

   /*
    * A fast allocation strategy can be used if the allocation
    * strategy is not strict, or if it's strict and we only extend
    * the size of the logical volume (i.e., no mirrors are added).
    */
   if (!(lv_flags & LVM_STRICT) ||
       ((lv_flags & LVM_STRICT) && total_mirr == 1)) {

      /* Use a fast allocation strategy */

      for (lx_cnt = start, px_index = 0 ; lx_cnt < end ; lx_cnt++) {
         for (j = mirr_start; j < total_mirr; px_index++) {
	    /* End of the physical volume map ? */
            if (px_index == px_cnt) {
	       /*
		* Free up the space allocated by the current physical
		* extent map and get the map from the next PV.
		*/
               free((char *)pv_map);

   	       /* read the physical map from the next PV in pvs */
   	       if (local_getpvmap(vg_fd, vgpath, pvs, lvpath, &pv_map, &px_cnt, 
		     &pv_key) != OK){
		  debug(dbg_exit());
		  return(NOT_OK);
		}

		px_index = 0;
            }
	    /* 
	     * continue if physical extent is stale or already belongs to
	     * a logical extent. A physical extent belonging to a logical
	     * can newer have the element "lv_min" set to 0 since a
	     * LV created by the user always has a minor number != 0
	     */
            if ((pv_map[px_index].status & LVM_PXSTALE) ||
                (pv_map[px_index].lv_min != 0)) 
	       continue;

            lv_map[lx_cnt][j].pv_key = pv_key;
            lv_map[lx_cnt][j].px_index = px_index;
            lv_map[lx_cnt][j].status = pv_map[px_index].status;
            lv_map[lx_cnt][j].in_use = TRUE;

	    /* Mark the PX as in use */
	    pv_map[px_index].status |= LVM_PXSTALE;

	    debug_msg("LE        %d\n", lx_cnt);
	    debug_msg("   mirror %d\n", j);
	    debug_msg("   will use PV %d\n", pv_key);
	    debug_msg("            PE %d\n", px_index);

            j++;
         }
      }
   }
   else {
      /* 
       * The slow allocation strategy has to be used. To save memory
       * the following allocation strategy is used; for each physical
       * volume map read, as many logical extents as possible is
       * allocated in lv_map. In some cases the lv_map has to be
       * scanned many times in order to allocate space for all logical
       * extents. For each scan through lv_map only one mirror (in best
       * case) can be allocated. The lv_map contains the field in_use
       * which will tell whether the different physical extents of
       * a logical extent are allocated or not.
       *
       * It is also not possible when the STRICT allocation strategy is
       * selcted to add two mirrors to a logical extent and have only one 
       * PV to take the physical extents from.
       */
      if (pvl_cnt == 1) {
	 if ((total_mirr - pxperlx_cnt) == 2) {
	    print_prgname();
	    fprintf(stderr, MSG_MIRR_NOT_ADDED);
	    debug(dbg_exit());
	    return(NOT_OK);
         }
      }

	debug_msg("END = %d\n", end);
      for (px_index = 0, lx_cnt = start; TRUE ; px_index++) {
	 
	 debug_msg("Big loop: work on LE %d\n", lx_cnt);

   	 /* End of the physical volume map ? */
         if (px_index == px_cnt) {
   	    /*
   	     * Free up the space allocated by the current physical
   	     * extent map and get the map from the next PV.
   	     */
            free((char *)pv_map);

	    debug_msg("try to load a new PV map\n", NULL);

   	    /* read the physical map from one of the PV in pvs */
   	    if (local_getpvmap(vg_fd, vgpath, pvs, lvpath, &pv_map, &px_cnt,
	          &pv_key) != OK) {
	       debug(dbg_exit());
	       return(NOT_OK);
	    }

   	    px_index = 0;
         }

	 /* 
	  * If the physical extent is stale or belongs to another logical
	  * volume, get the next. If a physical extent belongs to another
	  * logical extent, the entry lv_min will always be different
	  * from 0 since the minor number 0 is reserved for the group file
	  */
	 if ((pv_map[px_index].status & LVM_PXSTALE)
	      || pv_map[px_index].lv_min != 0) 
	    continue;

         debug_msg("PV %d:\n", pv_key);
         debug_msg("PE %d is free\n", px_index);

	 /* Does this logical extent have all mirrors allocated ? */
	 for (i = 0; i < total_mirr && lv_map[lx_cnt][i].in_use; i++)
	    continue; 

	 if (i != total_mirr) {

	    can_allocate = TRUE;

	    /* One mirror need still to be allocated; check for strictness */
	    if (lv_flags & LVM_STRICT) {
	       for (j = 0; j < i; j++)
		  if (lv_map[lx_cnt][j].pv_key == pv_key) {
		     /* 2 mirrors of the same LE can't live on the same PV */
		     can_allocate = FALSE;
		     break;
		  }
	    }

	    if (can_allocate) {

               lv_map[lx_cnt][i].pv_key = pv_key;
               lv_map[lx_cnt][i].px_index = px_index;
               lv_map[lx_cnt][i].status = pv_map[px_index].status;
               lv_map[lx_cnt][i].in_use = TRUE;

	       debug_msg("LE        %d\n", lx_cnt);
	       debug_msg("   end    %d\n", end);
	       debug_msg("   mirror %d\n", i);
	       debug_msg("   will use PV %d\n", pv_key);
	       debug_msg("            PE %d\n", px_index);

	       lx_cnt++;
	    }

         }

	 /* End of lv_map ? */
	 if (lx_cnt == end) {
	       debug_msg("            PE %d\n", px_index);
	    /* 
	     * Check if all of the new logical extents are allocated or if
	     * we need another go through lv_map
	     */
	    if (check_lv_map(lv_map + start, total_mirr, start, end) == OK)
	       break;
	    else {
	       lx_cnt = start;
	       continue;
	    }
	 }  
      }
   }
   *lvmap = lv_map;
   debug(dbg_exit());
   return(OK);
}


int
local_getpvmap(int vg_fd, char *vgpath, char **pvs, char *lvpath,
  px_descr_t **pv_map, int *px_cnt, unsigned short *pv_key)
{
   static unsigned short pv_key_tab[LVM_MAXPVS + 1];
   static int key_cnt;
   static int first_time = TRUE;
   char **lvs;
   int i, lv_cnt;
   register struct lv_querypv querypv;
   register struct lv_querylv querylv;

   debug(dbg_entry("local_getpvmap"));

   /* Get the PV key for all the physical volumes in pvs if first time called*/
   if (first_time) {
      debug_msg("\tfirst call; build local table\n", NULL);
      key_cnt = 0;
      first_time = FALSE;
      for (i = 0; pvs[i] ; i++) {

         debug_msg("\task for key of \"%s\"\n", pvs[i]);

         if ((pv_key_tab[i] = pvpathtopvkey(pvs[i])) == BAD_PV_KEY) {
	     /* an error here should never happen.  */
            debug_msg("lvextend.local_getpvmap: \n", NULL);
            debug_msg("pv_key couldn't be retrieved for PV '%s'\n", pvs[i]);
	    debug(dbg_exit());
            return(NOT_OK);
	 }

         debug_msg("\tPV \"%s\"\n", pvs[i]);
         debug_msg("\tgot key %d\n", pv_key_tab[i]);

      }
      /* Put a guard at the end */
      pv_key_tab[i] = BAD_PV_KEY;
   }

   /* Get the next available physical extent map */
   for (; pv_key_tab[key_cnt] != BAD_PV_KEY; key_cnt++) {

      debug_msg("\ttry to getpvmap for PV with key %d\n", pv_key_tab[key_cnt]);

      if (getpvmap(vg_fd, vgpath, pv_key_tab[key_cnt], pv_map, px_cnt) != OK) {

	 print_prgname();
         fprintf(stderr, MSG_PV_MAP_NOT_READ, 
			 pvkeytopvpath(pv_key_tab[key_cnt]));
      }  
      else {
	 debug_msg("\tgot the map\n", NULL);
	 /* Test if the PV has some user-dedicated PE's */
	 if (*px_cnt != 0) {
	    
            /* Get the state of the PV. If LVM_PVNOALLOC, continue */

	    /* Set input parameters */
	    querypv.pv_key = pv_key_tab[key_cnt];

 	    debug_msg("\tquery PV with key \"%d\"\n", pv_key_tab[key_cnt]);

            if (ioctl(vg_fd, LVM_QUERYPV, &querypv) < 0)
               continue;

            if (querypv.pv_flags & LVM_PVNOALLOC) {
               debug_msg("\tPhysical volume with pv_key \"%d\" has the\
			  LVM_PVNOALLOC flag set\n", pv_key_tab[key_cnt]);
   	       print_prgname();
	       fprintf(stderr, MSG_NOALLOCPV_WARNING, 
		               pvkeytopvpath(querypv.pv_key));
	       continue;
  	    }

	    *pv_key = pv_key_tab[key_cnt++];
	    debug(dbg_exit());
	    return(OK);
	 }
	 else
	    debug_msg("No PE available, although the PV is available\n", NULL);
      }
   }
   /* It's an error if there are not any more PV's to be read from */
   print_prgname();
   fprintf(stderr, MSG_NOT_ENOUGH_FREE_PX, lvpath);
   /*
    * If the LV has a strict allocation policy, the above message can
    * confuse the system administrator since, although one of the PV's
    * has space left, the cause of the failure can be that PX's can't
    * be allocated to more PV's, as wanted.
    * Try therefore to give the system administrtor a clue what the
    * couse of failure could be.
    */
   if (lvminors_and_names(vg_fd, vgpath, &lvs, &lv_cnt) == OK) {
      querylv.minor_num = lvpathtolvminor(lvpath);
      debug_msg("querylv.minor_num = %d\n", querylv.minor_num);
      if (ioctl(vg_fd, LVM_QUERYLV, &querylv) == 0) {
         debug_msg("querylv.lv_flags = %d\n", querylv.lv_flags);
         if (querylv.lv_flags & LVM_STRICT)
            fprintf(stderr, MSG_FAILURE_CLUE, lvpath);
      }
   }
   debug(dbg_exit());
   return(NOT_OK);
}




int
check_lv_map(lx_descr_t *lv_map, int total_mirr, int start, int end)
{
   register lx_descr_t *lxdp;
   register int i, j;

   debug(dbg_entry("check_lv_map"));

   lxdp = lv_map;
   for (i = start; i < end ; i++, lxdp++) {
      for (j = 0; j < total_mirr; j++) {
         if (!lxdp[0][j].in_use) {
	    debug_msg("not in use mirror %d\n", j);
	    debug_msg("of LE %d\n", i);
	    debug(dbg_exit());
            return(NOT_OK);
	 }
      }
   }
   debug(dbg_exit());
   return(OK);
}



int
extend_lv_map(int vg_fd, unsigned short lv_minor, char *lvpath, 
  lx_descr_t *lv_map, int old_lx_cnt, int old_mirr_cnt, int new_lx_cnt, 
  int new_mirr_cnt)
{
   struct lv_statuslv statuslv;
   struct lv_querylv querylv;
   unsigned short old_mirror_cnt;

   debug(dbg_entry("extend_lv_map"));

   /* Get the current setting of the logical volume */
   querylv.minor_num = lv_minor;

   /* Dump struct statuslv if in debug mode */
   debug(dbg_querylv_dump(&querylv, DBG_BEFORE));

   if (ioctl(vg_fd, LVM_QUERYLV, &querylv) == -1) {
      print_prgname();
      fprintf(stderr, MSG_LV_NOT_QUERIED, lvpath);
      lvm_perror(LVM_QUERYLV);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* 
    *   If the mirrors has been increased we must communicate to the driver
    *   before doing the actual extension.
    */
   if ((new_mirr_cnt - 1) > querylv.maxmirrors) {

      /*
       *   Initialize lv_statuslv to be used by LVM_CHANGELV
       */
      statuslv.minor_num = lv_minor;
      statuslv.maxlxs = querylv.maxlxs;
      statuslv.lv_flags = querylv.lv_flags;
      statuslv.sched_strat = querylv.sched_strat;
      statuslv.maxmirrors = new_mirr_cnt - 1;

      old_mirror_cnt = querylv.maxmirrors;

      /* Dump struct statuslv if in debug mode */
      debug(dbg_statuslv_dump(&statuslv));

      if (ioctl(vg_fd, LVM_CHANGELV, &statuslv) == -1) {
         print_prgname();
         fprintf(stderr, MSG_LV_NOT_CHANGED, lvpath);
         lvm_perror(LVM_CHANGELV);
         debug_msg("ioctl(CHANGELV)\n", NULL);
         return(NOT_OK);
      }
   }

   /* Then finally extend the logica volume */

   if (extend_lv(vg_fd, lv_minor, lv_map, old_lx_cnt, old_mirr_cnt,
         new_lx_cnt, new_mirr_cnt) != OK) {

      print_prgname();
      fprintf(stderr, MSG_LVDD_COULD_NOT_EXTEND, lvpath);

      if (old_mirr_cnt != new_mirr_cnt) {
         /* Try to undo the last call to LVM_CHANGELV */
	 statuslv.maxmirrors = old_mirror_cnt;

	 /* Dump struct statuslv if in debug mode */
	 debug(dbg_statuslv_dump(&statuslv));

	 if (ioctl(vg_fd, LVM_CHANGELV, &statuslv) == -1) {
	    print_prgname();
	    fprintf(stderr, MSG_LV_NOT_CHANGED_BACK, lvpath);
	    lvm_perror(LVM_CHANGELV);
	    debug_msg("ioctl(CHANGELV)\n", NULL);
	    return(NOT_OK);
	 }
      }

      debug(dbg_exit());
      return(NOT_OK);
   }
   debug(dbg_exit());
   return(OK);
}
