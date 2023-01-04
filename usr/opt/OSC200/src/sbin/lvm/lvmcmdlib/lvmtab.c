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
static char	*sccsid = "@(#)$RCSfile: lvmtab.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/22 16:07:42 $";
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
 *   lvmtab.c
 *   
 *   Contents:
 *
 *   All the routines and data structures which are related to the 
 *   /etc/lvmtab files are herein contained.
 *   In general, the file is loaded in a suitable format into memory
 *   with lvmtab_read(), and stored back with lvmtab_write(); various
 *   functions work on the memory representation, to add volume groups,
 *   etc.
 *   Note that all the references to path names are considered to be
 *   homogeneous; that is, all paths are absolute and "well formatted"
 *   (no double slashes, or /./, etc.)
 *   Here's a brief description of how the lvmtab file looks like:
 *	int vg_in_system
 *	then, for vg_in_system times:
 *	   char vg_path[PATH_MAX];		paths of VG in the system
 *	   lv_uniqueID_t vg_id;
 *	   int pv_cnt;
 *	   then, for pv_cnt times:
 *	      char pv_path[PATH_MAX];		paths of PV in VG
 *	   
 *   int lvmtab_read()
 *	Returns OK or NOT_OK. Reads the LVM table from /etc, and stores
 *	the information there contained into data structures visible to
 *	this module only.  File locking is used to safeguard the table;
 *	only open is permitted at a time.  The lock is implicitly cleared
 *	when the program exits.  If the table doesn't exist, it is
 *	created and initialized via lvmtab_update().
 *
 *   int lvmtab_write()
 *	Returns OK or NOT_OK. Reverse operation of the lvmtab_read() above.
 *	Works in a temporary scratch file (lvmtab.sav) and moves it
 *	(atomically in UN*X), replacing the old file.
 *
 *   static int lvmtab_update(int overwrite)
 *	Returns OK, NOT_OK or RETRY.  Like lvmtab_write(), except it has
 *	a parameter, `overwrite'.  If overwrite is set, the temporary
 *	file is moved, overwriting any existing file.  If overwrite
 *	is not set and the table exists, it abandons its work and simply
 *	returns RETRY.
 *
 *   int lvmtab_getvgnames(char ***name_list, int *items_in_list)
 *	Returns OK or NOT_OK. Allocates and initializes an array of strings
 *	containing the name of all the VG belonging to the system.
 *      Note that the returned table is a copy; this allows the caller to
 *	do whatever he wants to.
 *
 *   int lvmtab_getpvnames(char *vg_path, char ***name_list, int *items_in_list)
 *	Returns OK or NOT_OK. Allocates and initializes an array of strings
 *	containing the name of all the PV belonging to the specified volume
 *	group.
 *      Note that the returned table is a copy; this allows the caller to
 *	do whatever he wants to.
 *
 *   int lvmtab_isvgdefined(char *vg_path)
 *	Returns TRUE or FALSE; checks if the given VG belongs to
 *	the system.
 *
 *   int lvmtab_getvgid(char *vg_path, lv_uniqueID_t *vg_id)
 *	Returns OK or NOT_OK; searches for the given path in the list
 *	of the volume groups belonging to the system, indirectly returning
 *	the unique id of the volume group (if it is found).
 *
 *   int lvmtab_addvg(char *vg_path, lv_uniqueID_t *vg_id, int must_write)
 *	Returns OK or NOT_OK; adds the given path to the list
 *	of the volume groups belonging to the system; checks if the
 *	specified volume group was already there.
 *	must_write allows the caller to decide if the change has to be
 *	recorded on disk.
 *
 *   int lvmtab_removevg(char *vg_path, int must_write)
 *	Returns OK or NOT_OK; removes the given path from the list
 *	of the volume groups belonging to the system.
 *	must_write allows the caller to decide if the change has to be
 *	recorded on disk.
 *
 *   int lvmtab_ispvinvg(char *vg_path, char *pv_path)
 *	Returns TRUE or FALSE; checks if the given PV belongs to the
 *	specified VG.
 *
 *   int lvmtab_ispvinsomevg(char *pv_path, char **vg_path)
 *	Returns TRUE or FALSE; checks if the given PV belongs to one
 *	of the VG's of the system; if it's defined, stores the address
 *	of the string containing the name of the VG indirectly.
 *
 *   int lvmtab_addpvtovg(char *vg_path, char *pv_path, int must_write)
 *	Returns OK or NOT_OK; adds the given path to the list
 *	of the physical volumes belonging to the VG; checks if the
 *	specified physical volume was already there.
 *	must_write allows the caller to decide if the change has to be
 *	recorded on disk.
 *
 *   int lvmtab_removepvfromvg(char *vg_path, char *pv_path, int must_write)
 *	Returns OK or NOT_OK; removes the given path from the list
 *	of the physical volumes belonging to the VG.
 *	must_write allows the caller to decide if the change has to be
 *	recorded on disk.
 */

/*
 *  Modification History:  lvmtab.c
 *
 *  18-Dec-92	Bob Wu
 *	Added lvmtab_update routine and integrated lvmtab_read and lvmtab_write.
 *	The new functionality:
 *	-   Uses file locking on the lvmtab to ensure that only one
 *	    application at a time can be using the lvmtab at a time.
 *	-   Atomically updates the lvmtab by using a temp file.  Once
 *	    the temp file is updated and consistent, it is atomically
 *	    moved, replacing the original lvmtab.
 *	-   Goes through great pains to ensure that the lvmtab is always
 *	    consistant.  Much of the code is to guard against possible
 *	    race conditions (i.e. two users at once), or multiple users
 *	    trying to create the original lvmtab.
 */


#include "lvmcmds.h"


/* Values for the status of the slots */
#define FREE		0
#define TAKEN		1

/* How many slots do we allocate when we run out of them */
#define SLOTS_TO_ALLOC	10

/* Values for the status of the memory image of lvmtab */
#define NEEDREADING	0
#define READRIGHTNOW	1

/* All read/write's are checked for the return values */
#define bad_read(fd, ptr, siz)  (read((fd), (ptr), (siz)) != (siz))
#define bad_write(fd, ptr, siz)  (write((fd), (ptr), (siz)) != (siz))

/*
 *   Local types
 */

/* What we store about a physical volume in the lvmtab file */
typedef struct {
   short slot_status;		/* FREE or TAKEN */
   char pv_path[PATH_MAX];	/* pathname of the physical volume */
} pv_descr;

/* What we store about a volume group in the lvmtab file */
typedef struct {
   short slot_status;		/* FREE or TAKEN */
   char vg_path[PATH_MAX];	/* pathname of the volume group */
   lv_uniqueID_t vg_id;		/* unique ID of the volume group */
   unsigned int pv_cnt;		/* number of physical volumes in this VG */
   pv_descr *pv;		/* physical volumes belonging to this VG */
   short slot_cnt;		/* number of allocated slots for pv */
} vg_descr;

/* Logical lay-out of the file */
typedef struct {
   unsigned int vg_cnt;		/* number of VG's in this system */
   vg_descr *vg;		/* volume groups defined in this system */
   short slot_cnt;		/* number of allocated slots for vg */
   short status;		/* NEEDREADING, etc. */
} file_info;



/*
 *   Local variables: keep hidden all that can be hidden
 */

/* This is the memory image of the file */
static file_info lvmtab = {
   0, NULL, 0, NEEDREADING
};

/* This the file descriptor of the file */
static int lvmtab_fd = -1;


/*
 *   Local functions
 */

static int write_info(int fd);
static int read_info(int fd);
static int read_vg_info(int fd, unsigned int num_of_vg);
static int read_pv_info(int fd, vg_descr *vgdp, unsigned int num_of_pv);
static int write_vg_info(int fd);
static int write_pv_info(int fd, vg_descr *vgdp);
static vg_descr *vg_search(char *vg_path);
static pv_descr *pv_search(vg_descr *vgdp, char *pv_path);
static vg_descr *new_vgslot();
static pv_descr *new_pvslot(vg_descr *vg);
static void cleanup_tab();
static int remove_scratch_file();
static int lvmtab_update(int overwrite);


/* UNIT_TEST was used to verify multiple race conditions.
** Pauses were inserted at critical points so the interactions of
** multiple instances could be examined.
*/
#ifdef	UNIT_TEST
#include <signal.h>

void int_trap(int which)
{
    (void)signal (SIGINT, int_trap);
}
#endif


int
lvmtab_read()
{
   int ret_code;
   int lock_fd;
   struct stat fdstat, fnstat;

   debug(dbg_entry("lvmtab_read"));

#ifdef	UNIT_TEST
   int_trap(0);		/* Prime the signal trap */
#endif

   /* Maybe we've been already called */
   if (lvmtab.status != NEEDREADING) {
      debug(dbg_exit());
      return(OK);
   }

   if (lvmtab_fd == -1){
retry:
      /* Try opening the file and getting exclusive access.
      */
      debug_msg("trying to open R/O\n", NULL);
      if ((lvmtab_fd = open(LVMTABPATH, O_RDONLY)) < 0){
	 if ((stat(LVMTABPATH, &fnstat) == -1) && (errno != ENOENT)){
	    /* Unknown error */
	    prgname_perror(LVMTABPATH);
	    debug_msg("stat()\n", NULL);
	    debug(dbg_exit());
	    return(NOT_OK);
	 }

	 /* The file is not there.  Try to create a new one.
	 ** If someone else creates it for us, simply retry to acquire
	 ** the file lock on the LVM table.
	 */
	 switch (lvmtab_update(0)){
	 case RETRY:
	    debug_msg("lvmtab already created\n", NULL);
	    goto retry;

	 case NOT_OK:
	    prgname_perror(LVMTABPATH);
	    debug_msg("open()\n", NULL);
	    debug(dbg_exit());
	    return(NOT_OK);

	 case OK:
	    /* We created the file and are holding a file lock on it.
	    ** We simply fall through.  This reaquires the lock we
	    ** already have, but this is irrelevent in UN*X.
	    */
	    debug_msg("lvmtab created\n", NULL);
	    break;
	 }
      }

      debug_msg("about to flock\n", NULL);
      /* Got the file, now get exclusive access to it.
      ** The file may be replaced while we sleep, so after we get
      ** the lock we double check to make sure that the file still exists.
      ** N.B. flock is used instead of the more portable fcntl since we
      **   desire exclusive locking.  Fcntl would have required us to
      **   open the file for R/W access, even though we may only need R/O.
      */
      if ((flock(lvmtab_fd, LOCK_EX) == -1)
      || (fstat(lvmtab_fd, &fdstat) == -1) || (stat(LVMTABPATH, &fnstat) == -1)
      || (fdstat.st_dev != fnstat.st_dev) || (fdstat.st_ino != fnstat.st_ino)){
	 debug_msg("retrying to lock lvmtab\n", NULL);
	 close(lvmtab_fd);
	 goto retry;
      }
   }

#ifdef	UNIT_TEST
    debug_msg("got the lvmtab, pausing\n", NULL);
    pause();
#endif

   /* The file is there. If it's not corrupted, we have all the info */
   ret_code = read_info(lvmtab_fd);
   debug_msg("read_info %s\n", (ret_code == OK ? "OK" : "NOT_OK"));

   if (ret_code == OK) {
      /* Further requests to read shall be immediately "srved" */
      lvmtab.status = READRIGHTNOW;
   }

   debug(dbg_exit());
   return(ret_code);
}


int
lvmtab_write()
{
   /* Update the lvmtab with our internal data structures.
   ** Do this by dumping to a temporary file, and then replacing
   ** the existing file with our new one.
   ** Routine assumes that the existing file has a file lock already
   ** in place.
   ** When this exits, the old file is removed (and it's FD closed),
   ** our FD is placed into the global lvmtab_fd.
   */
   int ret_code;

   debug(dbg_entry("lvmtab_write"));
   ret_code = lvmtab_update(1);
   debug(dbg_exit());
   return(ret_code);
}


static int
lvmtab_update(int overwrite)
{
   int tab_fd;
   struct flock lock;
   struct stat fdstat, fnstat;

   debug(dbg_entry("lvmtab_update"));

   /* Assertions.  If called from lvmtab_write, overwrite should be set,
   ** and the file already open.  Otherwise, it should be totally uninit'd.
   */
   if ((overwrite && lvmtab_fd == -1) || (!overwrite && lvmtab_fd != -1)){
      debug_msg(overwrite ? "overwrite\n" : "create\n", NULL);
      debug(dbg_exit());
      return(NOT_OK);
   }

retry1:
   /* Create temp scratch file */
   tab_fd = open(SCRATCHLVMTABPATH, O_RDWR|O_CREAT, LVMTABPERMISS);

   /* This should never happen, if we are superuser */
   if (tab_fd < 0) {
      prgname_perror(SCRATCHLVMTABPATH);
      debug_msg("open()\n", NULL);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Get exclusive access to this file.  Just to ensure that this
   ** is still the same file that we open'ed, do a stat/fstat to make
   ** sure that another process didn't rename us in the meantime.
   ** Above all, we want to make sure that the lvmtab is consistant.
   */
   lock.l_type = F_WRLCK;
   lock.l_whence = SEEK_SET;
   lock.l_start = (off_t)0;
   lock.l_len = (off_t)0;
   if ((fcntl(tab_fd, F_SETLKW, &lock) == -1)
   || (fstat(tab_fd, &fdstat) == -1) || (stat(SCRATCHLVMTABPATH, &fnstat) == -1)
   || (fdstat.st_dev != fnstat.st_dev) || (fdstat.st_ino != fnstat.st_ino)){
      debug_msg("retrying to lock tmp lvmtab\n", NULL);
      close(tab_fd);
      goto retry1;
   }

#ifdef	UNIT_TEST
   debug_msg("acquired tmp file, pausing\n", NULL);
   pause();
#endif

   if (!overwrite && (stat(LVMTABPATH, &fnstat) != -1)){
      /* The lvmtab was already created while we slept getting the lock.
      ** We should return, and let the caller try to acquire the lock
      ** on the original file.
      */
      (void)remove_scratch_file();
      close(tab_fd);
      debug(dbg_exit());
      return(RETRY);
   }

   /* Ok. Now that we have exclusive access, its safe to truncate
   ** the file and record all the information so far collected, and
   ** then move it, atomically replacing the original file.
   ** N.B. For POSIX (which doesn't have ftruncate), this can be
   ** emulated by opening and closing the file again with the O_TRUNC
   ** flag set before we actually do any writes.  We don't open
   ** intially with O_TRUNC, because we could corrupt the file before
   ** we have acquired the file lock.
   */
   if (ftruncate(tab_fd, 0) == -1){
      debug_msg("couldn't truncate the tmp lvmtab\n", NULL);
   }
   else if (write_info(tab_fd) != OK){
      debug_msg("couldn't write the info to lvmtab\n", NULL);
   }
   else if (fsync(tab_fd) == -1){
      debug_msg("couldn't fsync the info to lvmtab\n", NULL);
   }
   else if (rename(SCRATCHLVMTABPATH, LVMTABPATH) < 0) {
      debug_msg("can't rename\n", NULL);
      print_prgname();
      fprintf(stderr, MSG_CREATE_LVMTAB, LVMTABPATH);
      perror("");
   }
   else {
      /* We have atomically replaced lvmtab with our new version.
      ** Swap the FD's (for future lvmtab_reads), and close the old file.
      ** Reset the file pointer to the start (for future reads).
      */
      int tmp_fd;

      tmp_fd = lvmtab_fd;
      lvmtab_fd = tab_fd;
      if (tmp_fd >= 0)
	 close(tmp_fd);

      lseek(lvmtab_fd, 0, SEEK_SET);
      debug(dbg_exit());
      return(OK);
   }

   /* If we got here, an error occurred while trying to write the file,
   ** or moving it.
   */
   (void)remove_scratch_file();
   close(tab_fd);
   debug(dbg_exit());

   return(NOT_OK);
}



int
lvmtab_getvgnames(char ***name_list, int *items_in_list)
{
   unsigned int num_of_vg;
   unsigned int num_of_slot;
   register unsigned int i;
   register unsigned int len;
   register unsigned int name_idx;
   register vg_descr *vgdp;
   char **name_table;

   debug(dbg_entry("lvmtab_getvgnames"));

   num_of_vg = lvmtab.vg_cnt;
   num_of_slot = lvmtab.slot_cnt;
   name_table = NULL;

   /* Allocate dynamically (and fill in) */
   if (num_of_vg > 0) {

      /*
       *   For each name of volume groups, we allocate only what is
       *   really needed; the user can thus overwrite the table we pass
       *   back.
       *   Allocate an extra slot, so that we can store a NULL
       *   for the end of the list.
       */

      name_table = (char **)checked_alloc((num_of_vg + 1)* sizeof(char *));
      name_idx = 0;
      vgdp = lvmtab.vg;
      for (i = 0; i < num_of_slot; i++, vgdp++) {
	 
	 if (vgdp->slot_status == FREE)
	    continue;

	 len = strlen(vgdp->vg_path);

	 /* Allocate space for the '\0', too */
	 name_table[name_idx] = checked_alloc(len + 1);
	 strcpy(name_table[name_idx], vgdp->vg_path);
	 name_idx++;
      }
      name_table[name_idx] = NULL;
   }

   /*
    *   Note that there should be no chance of returning NOT_OK, since
    *   checked_alloc would exit, if there's no more memory.
    */

   /* Store indirect returned values */
   *name_list = name_table;
   *items_in_list = num_of_vg;

   debug(dbg_exit());
   return(OK);
}



int
lvmtab_getpvnames(char *vg_path, char ***name_list, int *items_in_list)
{
   unsigned int num_of_pv;
   unsigned int num_of_slot;
   register unsigned int i;
   register unsigned int len;
   register unsigned int name_idx;
   register pv_descr *pvdp;
   register vg_descr *vgdp;
   char **name_table;

   debug(dbg_entry("lvmtab_getpvnames"));

   debug_msg("\t(VG: \"%s\"):\n", vg_path);

   vgdp = vg_search(vg_path);
   if (vgdp == NULL) {

      /* Probably wrong usage: clear output parameters, anyway */
      *name_list = NULL;
      *items_in_list = 0;
      debug(dbg_exit());
      return(NOT_OK);
   }

   num_of_pv = vgdp->pv_cnt;
   num_of_slot = vgdp->slot_cnt;
   name_table = NULL;

   debug_msg("\t%d PV's in VG\n", num_of_pv);

   /* Allocate dynamically (and fill in) */
   if (num_of_pv > 0) {

      /*
       *   For each name of volume groups, we allocate only what is
       *   really needed; the user can thus overwrite the table we pass
       *   back.
       *   Allocate an extra slot, so that we can store a NULL
       *   for the end of the list.
       */

      name_table = (char **)checked_alloc((num_of_pv + 1)* sizeof(char *));
      name_idx = 0;
      pvdp = vgdp->pv;
      for (i = 0; i < num_of_slot; i++, pvdp++) {
	 
	 if (pvdp->slot_status == FREE)
	    continue;

	 len = strlen(pvdp->pv_path);

	 /* Allocate space for the '\0', too */
	 name_table[name_idx] = checked_alloc(len + 1);
	 strcpy(name_table[name_idx], pvdp->pv_path);

	 debug_msg("\tstored \"%s\"\n", name_table[name_idx]);

	 name_idx++;
      }
      name_table[name_idx] = NULL;
   }

   /* Store indirect returned values */
   *name_list = name_table;
   *items_in_list = num_of_pv;

   debug(dbg_exit());
   return(OK);
}



int
lvmtab_isvgdefined(char *vg_path)
{
   /* Very simple */
   if (vg_search(vg_path) != NULL)
      return(TRUE);
   else
      return(FALSE);
}



int
lvmtab_getvgid(char *vg_path, lv_uniqueID_t *vg_id)
{
   register vg_descr *vgdp;

   debug(dbg_entry("lvmtab_getvgid"));

   /* Search for the VG, copying back its unique ID */
   vgdp = vg_search(vg_path);

   /* If we don't find it, we set to zero the unique ID */
   if (vgdp != NULL) {
      memcpy((char *)vg_id, (char *)&vgdp->vg_id, sizeof(lv_uniqueID_t));
      debug(dbg_exit());
      return(OK);
   }
   else {
      bzero((char *)vg_id, sizeof(lv_uniqueID_t));
      debug(dbg_exit());
      return(NOT_OK);
   }
}



int
lvmtab_addvg(char *vg_path, lv_uniqueID_t *vg_id, int must_write)
{
   register vg_descr *vgdp;

   debug(dbg_entry("lvmtab_addvg"));

   /* Check if the VG is not already there */
   vgdp = vg_search(vg_path);
   if (vgdp != NULL) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Create and initialize a new entry */
   vgdp = new_vgslot();
   strcpy(vgdp->vg_path, vg_path);
   memcpy((char *)&vgdp->vg_id, (char *)vg_id, sizeof(lv_uniqueID_t));

   /* No PV are there (yet), but slots might already have been allocated */
   vgdp->pv_cnt = 0;

   /* Try to write, if required; if write fails, free the slot */
   if (must_write == DOWRITE) {

      if (lvmtab_write() != OK) {
	 vgdp->slot_status = FREE;
	 lvmtab.vg_cnt--;
	 debug(dbg_exit());
         return(NOT_OK);
      }
   }
   else {
      /* Further requests to read might want to "undo" this operation */
      lvmtab.status = NEEDREADING;
   }

   debug(dbg_exit());
   return(OK);
}



int
lvmtab_removevg(char *vg_path, int must_write)
{
   vg_descr *vgdp;

   debug(dbg_entry("lvmtab_removevg"));

   /* Check if the VG is in the system, indeed */
   vgdp = vg_search(vg_path);
   if (vgdp == NULL) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Check if it is empty (no PV belonging to it) */
   if (vgdp->pv_cnt > 0) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Release it, keeping the slot for future use */
   vgdp->slot_status = FREE;
   lvmtab.vg_cnt--;

   /* Try to write, if required; if write fails, free the slot */
   if (must_write == DOWRITE) {

      if (lvmtab_write() != OK) {
	 vgdp->slot_status = TAKEN;
	 lvmtab.vg_cnt++;
	 debug(dbg_exit());
         return(NOT_OK);
      }
   }
   else {
      /* Further requests to read might want to "undo" this operation */
      lvmtab.status = NEEDREADING;
   }

   debug(dbg_exit());
   return(OK);
}



int
lvmtab_ispvinvg(char *vg_path, char *pv_path)
{
   register vg_descr *vgdp;
   register pv_descr *pvdp;

   debug(dbg_entry("lvmtab_ispvinvg"));

   /* Search for the VG, then for the PV */
   vgdp = vg_search(vg_path);
   if (vgdp != NULL) {
      pvdp = pv_search(vgdp, pv_path);
      if (pvdp != NULL) {
	 debug(dbg_exit());
	 return(TRUE);
      }
   }

   debug(dbg_exit());
   return(FALSE);
}



int
lvmtab_ispvinsomevg(char *pv_path, char **vg_path)
{
   register unsigned int num_of_slot;
   register unsigned int i;
   register unsigned int len;
   register vg_descr *vgdp;

   debug(dbg_entry("lvmtab_ispvinsomevg"));

   /* Scan all the VG of the system */
   num_of_slot = lvmtab.slot_cnt;
   vgdp = lvmtab.vg;
   *vg_path = NULL;

   for (i = 0; i < num_of_slot; i++, vgdp++)
      if (vgdp->slot_status == TAKEN)
         if (pv_search(vgdp, pv_path) != NULL) {

	    /* Return the name of the owner VG */
	    len = strlen(vgdp->vg_path);
	    *vg_path = checked_alloc(len + 1);
	    strcpy(*vg_path, vgdp->vg_path);

	    debug(dbg_exit());
	    return(TRUE);
	 }

   debug(dbg_exit());
   return(FALSE);
}



int
lvmtab_addpvtovg(char *vg_path, char *pv_path, int must_write)
{
   register vg_descr *vgdp;
   register pv_descr *pvdp;

   debug(dbg_entry("lvmtab_addpvtovg"));

   /* Check if the VG is there */
   vgdp = vg_search(vg_path);
   if (vgdp == NULL) {
      debug_msg("No VG \"%s\" in lvmtab\n", vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Check if the PV is already there */
   pvdp = pv_search(vgdp, pv_path);
   if (pvdp != NULL) {
      debug_msg("PV \"%s\" already in lvmtab\n", pv_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Create and initialize a new entry */
   pvdp = new_pvslot(vgdp);
   strcpy(pvdp->pv_path, pv_path);

   /* Try to write, if required; if write fails, free the slot */
   if (must_write == DOWRITE) {

      if (lvmtab_write() != OK) {
         debug_msg("failed to write the lvmtab\n", NULL);
	 pvdp->slot_status = FREE;
	 vgdp->pv_cnt--;
	 debug(dbg_exit());
         return(NOT_OK);
      }
   }
   else {
      /* Further requests to read might want to "undo" this operation */
      lvmtab.status = NEEDREADING;
   }

   debug(dbg_exit());
   return(OK);
}



int
lvmtab_removepvfromvg(char *vg_path, char *pv_path, int must_write)
{
   vg_descr *vgdp;
   pv_descr *pvdp;

   debug(dbg_entry("lvmtab_removepvfromvg"));

   /* Get to the VG */
   vgdp = vg_search(vg_path);
   if (vgdp == NULL) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Get to the PV */
   pvdp = pv_search(vgdp, pv_path);
   if (pvdp == NULL) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Release it, keeping the slot for future use */
   pvdp->slot_status = FREE;
   vgdp->pv_cnt--;

   /* Try to write, if required; if write fails, free the slot */
   if (must_write == DOWRITE) {

      if (lvmtab_write() != OK) {
	 pvdp->slot_status = TAKEN;
	 vgdp->pv_cnt++;
	 debug(dbg_exit());
         return(NOT_OK);
      }
   }
   else {
      /* Further requests to read might want to "undo" this operation */
      lvmtab.status = NEEDREADING;
   }

   debug(dbg_exit());
   return(OK);
}



/*
 *   Local functions
 */

static int
write_info(int fd)
{
   unsigned int num_of_vg;

   debug(dbg_entry("write_info"));

   /* Update the on-file description of the VG of the system */
   num_of_vg = lvmtab.vg_cnt;

   /* See description of the file structure in the beginning */
   if (bad_write(fd, &num_of_vg, sizeof(num_of_vg))) {
      /* Further requests to read shall get the actual picture of lvmtab */
      lvmtab.status = NEEDREADING;

      debug_msg("failed writing num_of_vg\n", NULL);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Store the information concerning each volume group */
   if (num_of_vg > 0) 
      if (write_vg_info(fd) != OK) {
         /* Further requests to read shall get the actual picture of lvmtab */
         debug_msg("failed writing info on vg\n", NULL);
         lvmtab.status = NEEDREADING;
	 debug(dbg_exit());
         return(NOT_OK);
      }

   debug(dbg_exit());
   return(OK);
}



static int
write_vg_info(int fd) 
{
   unsigned int num_of_pv;
   unsigned int num_of_slot;
   register int i;
   register vg_descr *vgdp;

   debug(dbg_entry("write_vg_info"));

   /* Store the information concerning each volume group */
   num_of_slot = lvmtab.slot_cnt;
   for (vgdp = lvmtab.vg, i = 0; i < num_of_slot; i++, vgdp++) {

      /* Is this just left over? */
      if (vgdp->slot_status == FREE)
	 continue;

      num_of_pv = vgdp->pv_cnt;

      if (bad_write(fd, vgdp->vg_path, sizeof(vgdp->vg_path)) ||
          bad_write(fd, &vgdp->vg_id, sizeof(vgdp->vg_id)) ||
          bad_write(fd, &num_of_pv, sizeof(num_of_pv)) ) {
	 debug(dbg_exit());
	 return(NOT_OK);
      }

      debug_msg("\tVG: \"%s\"\n", vgdp->vg_path);

      /* Store the information concerning each physical volume */
      if (num_of_pv > 0)
         if (write_pv_info(fd, vgdp) != OK) {
	    debug(dbg_exit());
            return(NOT_OK);
         }
   }

   debug(dbg_exit());
   return(OK);
}



static int
write_pv_info(int fd, vg_descr *vgdp) 
{
   register unsigned int i;
   register pv_descr *pvdp;
   unsigned int num_of_slot;

   debug(dbg_entry("write_pv_info"));

   /* Load the information concerning each physical volume */
   num_of_slot = vgdp->slot_cnt;
   for (pvdp = vgdp->pv, i = 0; i < num_of_slot; i++, pvdp++) {

      /* Is this just left over? */
      if (pvdp->slot_status == FREE)
	 continue;

      if (bad_write(fd, pvdp->pv_path, sizeof(pvdp->pv_path))) {
	 debug(dbg_exit());
	 return(NOT_OK);
      }

      debug_msg("\t\tPV: \"%s\"\n", pvdp->pv_path);
   }

   debug(dbg_exit());
   return(OK);
}



static int
read_info(int fd)
{
   unsigned int num_of_vg;

   debug(dbg_entry("read_info"));

   /* Since we are reading from file, all that is in memory is obsolete */
   cleanup_tab();

   /* See description of the file structure in the beginning */
   if (bad_read(fd, &num_of_vg, sizeof(num_of_vg))) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Allocate space for each of the volume groups in the system */
   if (num_of_vg > 0) {

      /* Load the information concerning each volume group */
      if (read_vg_info(fd, num_of_vg) != OK) {
	 debug(dbg_exit());
         return(NOT_OK);
      }
   }

   debug(dbg_exit());
   return(OK);
}



static int
read_vg_info(int fd, unsigned int num_of_vg) 
{
   unsigned int num_of_pv;
   register int i;
   register vg_descr *vgdp;

   debug(dbg_entry("read_vg_info"));

   /* Load the information concerning each volume group */
   for (i = 0; i < num_of_vg; i++) {

      vgdp = new_vgslot();

      if (bad_read(fd, vgdp->vg_path, sizeof(vgdp->vg_path)) ||
          bad_read(fd, &vgdp->vg_id, sizeof(vgdp->vg_id)) ||
          bad_read(fd, &num_of_pv, sizeof(num_of_pv)) ) {
	 debug(dbg_exit());
	 return(NOT_OK);
      }

      debug_msg("\tVG: \"%s\"\n", vgdp->vg_path);

      /* Allocate space for each of the physical volumes in the VG */
      if (num_of_pv > 0) {

         /* Load the information concerning each physical volume */
         if (read_pv_info(fd, vgdp, num_of_pv) != OK) {
	    debug(dbg_exit());
            return(NOT_OK);
         }
      }
   }

   debug(dbg_exit());
   return(OK);
}



static int
read_pv_info(int fd, vg_descr *vgdp, unsigned int num_of_pv) 
{
   register int i;
   register pv_descr *pvdp;

   debug(dbg_entry("read_pv_info"));

   /* Load the information concerning each physical volume */
   for (i = 0; i < num_of_pv; i++) {

      pvdp = new_pvslot(vgdp);

      if (bad_read(fd, pvdp->pv_path, sizeof(pvdp->pv_path))) {
	 debug(dbg_exit());
	 return(NOT_OK);
      }

      debug_msg("\t\tPV: \"%s\"\n", pvdp->pv_path);
   }

   debug(dbg_exit());
   return(OK);
}



static vg_descr *
vg_search(char *vg_path)
{
   register unsigned int i;
   register unsigned int num_of_slot;
   register vg_descr *vgdp;

   debug(dbg_entry("vg_search"));

   /* Scan the array of vg descriptors, checking for the vg_path */
   num_of_slot = lvmtab.slot_cnt;
   vgdp = lvmtab.vg;

   for (i = 0; i < num_of_slot; i++, vgdp++) {
      if (vgdp->slot_status == TAKEN)
         if (eq_string(vg_path, vgdp->vg_path)) {
	    debug(dbg_exit());
	    return(vgdp);
         }
   }

   debug_msg("didn't find \"%s\"\n", vg_path);
   debug(dbg_exit());
   return(NULL);
}



static pv_descr *
pv_search(vg_descr *vgdp, char *pv_path)
{
   register unsigned int i;
   register unsigned int num_of_slot;
   register pv_descr *pvdp;

   debug(dbg_entry("pv_search"));

   /* Scan the array of pv descriptors, checking for the pv_path */
   num_of_slot = vgdp->slot_cnt;
   pvdp = vgdp->pv;

   for (i = 0; i < num_of_slot; i++, pvdp++)
      if (pvdp->slot_status == TAKEN)
         if (eq_string(pv_path, pvdp->pv_path)) {
	    debug(dbg_exit());
	    return(pvdp);
         }

   debug_msg("didn't find \"%s\"\n", pv_path);
   debug(dbg_exit());
   return(NULL);
}



static vg_descr *
new_vgslot()
{
   register vg_descr *vgdp;
   register unsigned int i;
   register unsigned int slot_cnt;
   vg_descr *old_vg, *new_vg;
   unsigned int old_size, old_slot_num;
   unsigned int new_size, new_slot_num;

   debug(dbg_entry("new_vgslot"));

   if (lvmtab.vg == NULL) {
      
      /* No slots there. Allocate some */
      vgdp = (vg_descr *)checked_alloc(SLOTS_TO_ALLOC * sizeof(vg_descr));

      /* Set them free */
      for (i = 0; i < SLOTS_TO_ALLOC; i++)
	 vgdp[i].slot_status = FREE;

      /* Save the new area allocated; vgdp is the ptr to be returned */
      lvmtab.vg = vgdp;
      lvmtab.slot_cnt = SLOTS_TO_ALLOC;

      /* This is redundant, but very cheap and clarifying */
      lvmtab.vg_cnt = 0;
   }
   else {

      /* Search for one free */
      slot_cnt = lvmtab.slot_cnt;
      for (vgdp = lvmtab.vg, i = 0; i < slot_cnt; i++, vgdp++)
	 if (vgdp->slot_status == FREE)
	    break;

      if (i == slot_cnt) {
	 
	 /* Must extend the array */

         /* Save old info, into a new larger area */
         old_vg = lvmtab.vg;

         old_slot_num = lvmtab.slot_cnt;
         old_size = old_slot_num * sizeof(vg_descr);
         new_slot_num = old_slot_num + SLOTS_TO_ALLOC;
         new_size = new_slot_num * sizeof(vg_descr);

         /* The extend routine will check for NULL ptr, etc. */
         new_vg = (vg_descr *)
		  checked_realloc((char *)old_vg, old_size, new_size);

	 /* Set free the new slots; the old ones have already been copied */
         for (i = old_slot_num; i < new_slot_num; i++)
	    new_vg[i].slot_status = FREE;

	 /* What used to be out of bounds, is now a new area */
	 vgdp = &new_vg[old_slot_num];

         /* Save new values */
         lvmtab.vg = new_vg;
         lvmtab.slot_cnt = new_slot_num;
      }
      
   }

   /* One more is taken */
   lvmtab.vg_cnt++;
   vgdp->slot_status = TAKEN;

   debug(dbg_exit());
   return(vgdp);
}



static pv_descr *
new_pvslot(vg_descr *vg)
{
   register pv_descr *pvdp;
   register unsigned int i;
   register unsigned int slot_cnt;
   pv_descr *old_pv, *new_pv;
   unsigned int old_size, old_slot_num;
   unsigned int new_size, new_slot_num;

   debug(dbg_entry("new_pvslot"));

   if (vg->pv == NULL) {
      
      /* No slots there. Allocate some */
      pvdp = (pv_descr *)checked_alloc(SLOTS_TO_ALLOC * sizeof(pv_descr));

      /* Set them free */
      for (i = 0; i < SLOTS_TO_ALLOC; i++)
	 pvdp[i].slot_status = FREE;

      /* Save the new area allocated; pvdp is the ptr to be returned */
      vg->pv = pvdp;
      vg->slot_cnt = SLOTS_TO_ALLOC;

      /* This is redundant, but very cheap and clarifying */
      vg->pv_cnt = 0;
   }
   else {

      /* Search for one free */
      slot_cnt = vg->slot_cnt;
      for (pvdp = vg->pv, i = 0; i < slot_cnt; i++, pvdp++)
	 if (pvdp->slot_status == FREE)
	    break;

      if (i == slot_cnt) {
	 
	 /* Must extend the array */

         /* Save old info, into a new larger area */
         old_pv = vg->pv;

         old_slot_num = vg->slot_cnt;
         old_size = old_slot_num * sizeof(pv_descr);
         new_slot_num = old_slot_num + SLOTS_TO_ALLOC;
         new_size = new_slot_num * sizeof(pv_descr);

         /* The extend routine will check for NULL ptr, etc. */
         new_pv = (pv_descr *)
		  checked_realloc((char *)old_pv, old_size, new_size);

	 /* Set free the new slots; the old ones have already been copied */
         for (i = old_slot_num; i < new_slot_num; i++)
	    new_pv[i].slot_status = FREE;

	 /* What used to be out of bounds, is now a new area */
	 pvdp = &new_pv[old_slot_num];

         /* Save new values */
         vg->pv = new_pv;
         vg->slot_cnt = new_slot_num;
      }
      
   }

   /* One more is taken */
   vg->pv_cnt++;
   pvdp->slot_status = TAKEN;

   debug(dbg_exit());
   return(pvdp);
}



static int
remove_scratch_file()
{
   debug(dbg_entry("remove_scratch_file"));

   if (unlink(SCRATCHLVMTABPATH) < 0){
      print_prgname();
      fprintf(stderr, MSG_REMOVE_FILE, SCRATCHLVMTABPATH);
      perror("");
      debug_msg("(unlink)\n", NULL);
      debug(dbg_exit());
      return(NOT_OK);
   }
   debug(dbg_exit());
   return(OK);
}



static void
cleanup_tab()
{
   register vg_descr *vgdp;
   register pv_descr *pvdp;
   register unsigned int i, j;
   register unsigned int vg_slot;
   register unsigned int pv_slot;

   debug(dbg_entry("cleanup_tab"));

   vg_slot = lvmtab.slot_cnt;
   for (i = 0, vgdp = lvmtab.vg; i < vg_slot; i++, vgdp++) {

      /* If this slot was in use, free all the PV slots, too */
      if (vgdp->slot_status != FREE) {

	 pv_slot = vgdp->slot_cnt;
         for (j = 0, pvdp = vgdp->pv; j < pv_slot; j++, pvdp++)
            pvdp->slot_status = FREE;

         vgdp->slot_status = FREE;
         vgdp->pv_cnt = 0;
      }
   }

   lvmtab.vg_cnt = 0;
   /* Keep the space allocated for lvmtab.vg: might be useful later */

   debug(dbg_exit());
}
