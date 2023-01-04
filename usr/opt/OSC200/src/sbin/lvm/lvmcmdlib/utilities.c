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
static char	*sccsid = "@(#)$RCSfile: utilities.c,v $ $Revision: 4.2.4.4 $ (DEC) $Date: 1993/01/08 15:27:57 $";
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
 *   utilities.c
 *   
 *   Contents:
 *
 *   char *checked_alloc(unsigned int bytes)
 *	Just like malloc(), but either finds enough memory, or exits 
 *	printing an error message. Cleans also the area (0's).
 *
 *   char *checked_realloc(char *old_area, unsigned int old_size,
 *		unsigned int new_size)
 *	Using the above routine, reallocs a memory area, copying the 
 *	data there contained (if needed).
 *
 *   char *memdup(char *area, unsigned int bytes)
 *	Duplicates a memory area, returning a pointer to the new area.
 *
 *   int rm_dir(char *path)
 *	This function removes path. If path is a directory rm_dir
 *	presumes that it is a VG directory (i.e., no subdirectories
 *	under path). It then deletes all files under VG directory
 *	and finally the VG directory itself.
 *	In case of error NOT_OK is returned, else OK.
 *
 *   int special_f_tst(char *path, int file_type, dev_t *devnum)
 *      Test if file pointed at by "path" is of type "file_type".
 *	If the devnum pointer is not NULL and path is a special file,
 *	its device number is indirectly returned.
 *	NOT_OK is returned by falure and OK by success
 *
 *   char *mk_clean_path(char *path)
 *	This function takes parts of a file name (i.e., vgname, ./vgname,
 *	dev/vgname, /dev////vgname) and generates a clean path which
 *	containes only two /'es. All application which want to store
 *	path-names in /etc/lvmtab should call this function to
 *	get a consistent path which later can be looked up.
 *	The returned path is stored in newly allocated memory, so the
 *	caller can do whatever s/he wants with it.
 *
 *   char *mk_save_clean_path(char *path)
 *	Just like the one above, but returns a pointer to a newly allocated
 *	area, containing the clean path.\
 *
 *   int install_pv(int vg_fd, char *vgname, struct lv_installpv *installpv, 
 *		    char **pv_paths)
 *	install_pv installes the physical volumes in pv_paths into a
 *	volume group addressed by vg_fd. /etc/lvmtab is also updated.
 *	If one of the pvs in the pv_paths couldn't be installed NOT_OK
 *	is returned, else OK is returned.
 *
 *   int isblock_and_clean(char *path, char **ret_path, int mode)
 * 	Test if path is a block special file and returnes a clean path 
 *	starting from root in the second argument. If successful OK is 
 *	returned, else NOT_OK.
 * 	isblock_and_clean() displays the needed error messages.
 *	Depending on mode ([DONT]CHECKLVMTAB) checks if the path is a PV
 *	inside a VG.
 *
 *   char *check_and_openvg(char *vgpath, int *vg_fd)
 *	If "vgpath" is a volume group in /etc/lvmtab, the group
 *	special file will be opened and the fd will be returned in
 *	"vg_fd". In order to check if "vgpath" is in /etc/lvmtab, 
 *	lvmtab is read into memory. In order to do the lookup in
 *	/etc/lvmtab, a clean and consistent path has to be generated.
 *	If no errors, this clean path is returned, else NULL is returned.
 *	check_and_openvg displays the needed error messages.
 *
 *   int attach_pvs(int vg_fd, char **pvs, int pvs_cnt)
 *	The physical volumes pointed to by "pvs" are attached to
 *	the VG referenced by vg_fd. Error message is printed if
 *	a physical volume cannot be attached. Return OK or NOT_OK.
 *
 *   int deattach_pvs(int vg_fd, char **pvs, int pvs_cnt, int mode)
 *	The physical volumes pointed to by "pvs" are deattached from
 *	the VG referenced by vg_fd. Error message is printed if
 *	a physical volume cannot be deattached, depending on mode.
 *	Return OK or NOT_OK.
 *
 *   int multiple_query_lv(int vg_fd, char **lv_names, unsigned int lv_cnt,
 *		  struct lv_querylv **querylv_array_ptr) 
 *   int multiple_query_pv(int vg_fd, char **pv_names, unsigned int pv_cnt,
 *		  struct lv_querypvpath **querypv_array_ptr, char **pv_queried) 
 *	These functions query LV's and PV's, allocating a table which is
 *	indirectly returned to the caller via the double pointer.
 *	Return OK or NOT_OK.
 *
 *   int query_driver(int vg_fd, int op_code, void *buf)
 *	All the LVM_QUERY* which we use are called by this function; a
 *	sort of caching is implemented to avoid calling the driver too often.
 *	The interface is just like ioctl.
 *
 *   void disable_intr()
 *	Prevents the program to be interrupted (SIGINTR, etc.); this
 *	should be called before critical steps in the program are taken.
 *
 */

#include "lvmcmds.h"

/*
 *   We have a sort of local "cache" for keeping information queried from
 *   the driver; we keep it hidden in this module.
 *
 *   (If you wonder why these data/functions are not kept in another,
 *   single library module, the answer is that the bureaucracy of 
 *   releases suggested to not do it)
 *   The idea is to have a general purpose buffer (so that we can avoid
 *   to call too often the driver), where we store different types of
 *   structures; retrieval is provided through a "type" and a "key".
 *   We keep MAXINFOCACHE pieces of information; in a very simple
 *   way, when we run out of slots, we discard the oldest stored data.
 *   The need for this arises from the fact that the info returned by 
 *   the driver is needed in many of the layers of the library/command
 *   modules, but we don't want to access the driver whenever we need
 *   just a small piece of data.
 *
 *   We use these local functions:
 *
 *   cached_data *cache_search(int type, cache_key *key)
 *	Searches for the information that might have been
 *	previously stored with the specified type/key; if nothing is found,
 *	NULL is returned. Note that the returned data area will be
 *	deallocated (released) after the next n calls to cache_store();
 *	this means that the caller should make a copy of the data.
 *
 *   void cache_store(cached_data *cdp, int type, cache_key *key)
 *	Stores the given information in the cache.
 */

#define MAXINFOCACHE	30	/* to be tuned */

typedef struct {
   int size;		/* number of bytes saved */
   char *address;	/* the data that have to be preserved */
} cached_data;

typedef union {
   unsigned short lv_min;  /* key used when type is LVM_QUERYLV/MAP */
   unsigned short pv_key;  /* key used when type is LVM_QUERYPV/MAP */
   char *pv_path;	   /* key used when type is LVM_QUERYPVPATH */
   int vg_fd;		   /* key used when type is LVM_QUERYVG */
} cache_key;

#define eq_keys(type, k1, k2)  ((type) == LVM_QUERYPVPATH ? \
				eq_string((k1)->pv_path, (k2)->pv_path) : \
				(memcmp(k1, k2, sizeof(cache_key)) == 0) )

typedef struct {
   short in_use;	/* flag to know if this cell is free/busy */
   int type;		/* which type of info is stored here (QUERYVG, ...) */
   cache_key key;	/* depending on the type, can be different things */
   cached_data info;	/* the actual data */
} info_cache;

info_cache info_cache_tab[MAXINFOCACHE];	/* the cache area */
int ic_next_to_use = 0;				/* next slot to be used */

static cached_data *cache_search(int type, cache_key *key);
static void cache_store(cached_data *cdp, int type, cache_key *key);


/*
 * The code of the library functions 
 */

char *
checked_alloc(unsigned int bytes)
{
   register char *ret_val;

   /* Do they really want something? */
   if (bytes == 0)
      return(NULL);

   /* Allocate memory, and check if there's enough */
   ret_val = (char *)malloc(bytes);
   if (ret_val == NULL) {
      print_prgname();
      debug_msg("utilities.checked_alloc: exit\n", NULL);
      perror("");
      exit(FATAL_ERROR);
   }

   /* Clean it */
   bzero(ret_val, bytes);
   return(ret_val);
}



char *
checked_realloc(char *old_area, unsigned int old_size, unsigned int new_size)
{
   char *ret_val;
   register unsigned int min, i;

   if (new_size == 0) {

      /* No space is required; probably a reduction was required */
      ret_val = NULL;
      if (old_area != NULL)
	 free(old_area);
   }
   else {

      /* Allocate space */
      ret_val = checked_alloc(new_size);
   
      /* Copy old contents, if needed */
      if (old_area != NULL) {
   
         min = (old_size < new_size) ? old_size : new_size;
         memcpy(ret_val, old_area, min);
   
         /* Free old space */
         free(old_area);
      }
   }

   return(ret_val);
}



char *
memdup(char *area, unsigned int bytes)
{
   register char *ret_val;

   /* Allocate space and copy the data */
   ret_val = checked_alloc(bytes);
   memcpy(ret_val, area, bytes);
   
   return(ret_val);
}




int
rm_dir(char *path)
{
   DIR *dirp;
   struct dirent *dp;
   dev_t dev_num;
   char rm_path[PATH_MAX];

   debug(dbg_entry("rm_dir"));

   /* Test if path is a directory-path or a file */
   if (special_f_tst(path, S_IFDIR, &dev_num) == OK) {
   
      /* Use library functions to open the directory */
      if ((dirp = opendir(path)) == NULL) {
         print_prgname();
         fprintf(stderr, MSG_OPENDIR, path);
	 perror("");
         debug(dbg_exit());
         return(NOT_OK);
      }
   
      /* Scan the directory for files to be removed, skipping what's useless */
      while ((dp = readdir(dirp)) != NULL) {
         if (eq_string(dp->d_name, ".") || eq_string(dp->d_name, ".."))
   	    continue;
   
         /* Build the whole path of the file to be removed */
	 strncpy(rm_path, path, PATH_MAX - (strlen(dp->d_name) + 1));
	 strcat(rm_path, "/");
	 strcat(rm_path, dp->d_name);
   
         if (remove(rm_path) < 0) {
   	    print_prgname();
   	    fprintf(stderr, MSG_FILE_NOT_DELETED, rm_path);
	    perror("");
   	    debug(dbg_exit());
   
   	    /* 
   	     * It doesn't make any sense to continue if one of the files
   	     * couldn't be deleted.
   	     */
            return(NOT_OK);
         }
      }

      /* Then finally delete the directory */
      if (remove(path) < 0) {
         print_prgname();
         fprintf(stderr, MSG_DIR_NOT_DELETED, path);
	 perror("");
         debug(dbg_exit());
         return(NOT_OK);
      }
   }
   /* it is a file */
   else {
      if (remove(path) < 0) {
   	 print_prgname();
   	 fprintf(stderr, MSG_FILE_NOT_DELETED, path);
	 perror("");
   	 debug(dbg_exit());
	 return(NOT_OK);
      }
   }
   return(OK);
}



int
special_f_tst(char *path, int file_type, dev_t *devnum)
{
   struct stat st;

   debug(dbg_entry("special_f_tst"));

   /*
    *   Test if file pointed at by "path" is of type "file_type".
    *	NOT_OK is returned by falure and OK by success
    */

   if (stat(path, &st) < 0) {
      debug_msg("stat(%s):\n", path);
      debug_msg("failed with errno %d\n", errno);
      debug(dbg_exit());
      return(NOT_OK);
   }

   if ((st.st_mode & S_IFMT) != file_type) {
      debug_msg("special_f_tst: 'st.st_mode & S_IFMT' = %x\n",
	       st.st_mode & S_IFMT);
      debug_msg("               file_type = %x\n", file_type);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* Should we return the device number? */
   if (devnum != NULL)
      *devnum = st.st_rdev;

   debug(dbg_exit());
   return(OK);
}



char *
mk_clean_path(char *path)
{
char tmp_path[PATH_MAX];
char *ret_cp;
int i, j, k;
int status = OK;

	debug(dbg_entry("mk_clean_path"));

	/*
	 * if path starts with "./" means that the system administrator
	 * is currently under /dev.
	 */
	if ((*path == '.') && (path[1] == '/')) {
		strcpy(tmp_path, DEVPATH);
		strcat(tmp_path, path + 2);
	}
	else if (strncmp(path, DEVPATH, strlen(DEVPATH)) == 0) {
		/*
		 * if the path starts with DEVPATH his current directory
		 * must be root
		 */
		strcpy(tmp_path, path);
	}
	else if (isalnum(*path)) {
		/*
		 * else the path must start with an isalnum. If it
		 * also contains a '/' the current directory must
		 * be root
		 */
		if (strchr(path, '/') != NULL) {
			tmp_path[0] = '/';
			strcpy(&tmp_path[1], path);
		}
		else {
			strcpy(tmp_path, DEVPATH);
			strcat(tmp_path, path);
		}
	}
	else
		/* it is an error */
		status = NOT_OK;

				
	if (status == OK) {
		/*
		 * make sure that the path don't contain more "/'es" after
		 * each other
		 */
		for (i=0; tmp_path[i]; i++) {
			if (tmp_path[i] == '/' && tmp_path[i+1] == '/') {
				/*
				 *skipp all the following \'es
				 */
				for (j=i+1; tmp_path[j] == '/'; j++);
				for (k=i+1; tmp_path[j]; j++, k++)
					tmp_path[k] = tmp_path[j];
				/*
				 * copy also the '\0'
				 */
				tmp_path[k] = tmp_path[j];
			}
		}
		/*
		 * if the tmp_path now containes more than two "/'es"
		 * it is an error.
		 */
		for (i=0, j=0; tmp_path[i]; i++)
			if (tmp_path[i] == '/')
				j++;
		if (j != 2)
			status = NOT_OK;
	}

	if (status == OK) {
		debug(dbg_exit());
		ret_cp = checked_alloc(strlen(tmp_path) + 1);
		strcpy(ret_cp, tmp_path);
		return(ret_cp);
	}
	else {
		debug(dbg_exit());
		return(NULL);
	}
}



char *
mk_save_clean_path(char *path)
{
   register char *cp, *ret_cp;
   register int l;

   debug(dbg_entry("mk_save_clean_path"));

   /* Just allocate space to save whhat's returned by mk_clean_path */
   cp = mk_clean_path(path);
   if (cp == NULL) {

      /* Consider this to be an implementation error */
      print_prgname();
      debug_msg("mk_save_clean_path: exiting\n", NULL);
      fprintf(stderr, MSG_NO_CLEAN_PATH, path);
      exit(FATAL_ERROR);
   }

   l = strlen(cp) + 1;
   ret_cp = checked_alloc(l);
   strcpy(ret_cp, cp);
   debug(dbg_exit());
   return(ret_cp);
}



int
install_pv(int vg_fd, char *vgname, struct lv_installpv *installpv, 
	   char **pv_paths, int pv_paths_cnt)
{
struct  lv_querypvpath  querypvpath;
int pv_key;
int i;
int status = OK;

	debug(dbg_entry("install_pv"));

	for (i=0; i < pv_paths_cnt ; i++) {
		/*
		 * Test if pv_paths[i] is a block special file and
		 * returnes a clean path starting from root in the second
		 * argument. If successful OK is returned, else NOT_OK.
		 * isblock_and_clean() displays the needed error messages.
		 */
		if (isblock_and_clean(pv_paths[i], &installpv->path, 
				CHECKLVMTAB) == NOT_OK) {
			status = NOT_OK;
			continue;
		}

		/* Dump struct installpv if in debug mode */
		debug(dbg_installpv_dump(installpv));

		if (ioctl(vg_fd, LVM_INSTALLPV, installpv) == -1) {
			print_prgname();
			fprintf(stderr, MSG_LVM_INSTALLPV_FAILED,
			        installpv->path);
      			lvm_perror(LVM_INSTALLPV);
			status = NOT_OK;
			continue;
		}

		/* add installpv->path to lvmtab */
		if (lvmtab_addpvtovg(vgname, installpv->path, DOWRITE) != OK) {
			/*
			 * if installpv->path could not be added to lvmtab,
			 * the phsyical volume has to be removed from the
			 * in memory VGDA
			 */
			querypvpath.path = installpv->path;
			/* get the pv_key which is need in LVM_DELETEPV */
			if (query_driver(vg_fd, LVM_QUERYPVPATH, &querypvpath)
					== -1) {
				print_prgname();
				fprintf(stderr, MSG_PV_INCONSISTENT,
					installpv->path, LVMTABPATH);
      				lvm_perror(LVM_QUERYPVPATH);
				status = NOT_OK;
				continue;
			}

			pv_key = querypvpath.pv_key;

			/*
			 * Then finaly remove the phsyical volume from
			 * the VGDA
			 */
			debug(dbg_pvID_dump(&pv_key));
			if (ioctl(vg_fd, LVM_DELETEPV, &pv_key) == -1) {
				print_prgname();
				fprintf(stderr, MSG_PV_INCONSISTENT,
					installpv->path, LVMTABPATH);
      				lvm_perror(LVM_DELETEPV);
				status = NOT_OK;
				continue;
			}
		}
	}
	debug(dbg_exit());
	return(status);
}


	  
int
isblock_and_clean(char *path, char **ret_path, int mode)
{
char *vg_name;

	debug(dbg_entry("isblock_and_clean"));

	/* 
	 * Transform PhsyicalVolumePath[i] into a clean path
	 * starting from root
	 */
	if ((*ret_path = mk_clean_path(path)) == NULL) {
		print_prgname();
		fprintf(stderr, MSG_PV_NOT_ADDED, path, LVMTABPATH);
		debug(dbg_exit());
		return(NOT_OK);
	}
	/* 
	 * It's an error if PhysicalVolumePath[i] already 
	 * exists in lvmtab 
	 */
	if (mode == CHECKLVMTAB && lvmtab_ispvinsomevg(*ret_path, &vg_name)) {
		print_prgname();
		fprintf(stderr, MSG_PV_EXISTS, *ret_path, LVMTABPATH);
		debug(dbg_exit());
		return(NOT_OK);
	}

	/* Test if PhysicalVolumePath[i] is a block special file */
	if (special_f_tst(*ret_path, S_IFBLK, (dev_t *)NULL) == NOT_OK) {
		print_prgname();
		fprintf(stderr, MSG_NOPV, *ret_path);
		debug(dbg_exit());
		return(NOT_OK);
	}
	debug(dbg_exit());
	return(OK);
}



char * 
check_and_openvg(char *vgpath, int *vg_fd)
{
char *clean_path;
char grp_path[PATH_MAX];

	debug(dbg_entry("check_and_openvg"));

	/* read /etc/lvmtab into memory */
	if (lvmtab_read() == NOT_OK) {
		print_prgname();
		fprintf(stderr, MSG_LVMTAB_READ_ERROR, LVMTABPATH);
		debug(dbg_exit());
		return(NULL);
	}

	/* test if vgpath is good and can be used by lvmtab_isvgdefined */
	if ((clean_path = mk_clean_path(vgpath)) == NULL) {
		print_prgname();
		fprintf(stderr, MSG_NO_CLEAN_PATH, vgpath);
		debug(dbg_exit());
		return(NULL);
	}

	/* it's an error ig clean_path is not defined in lvmtab */
	if (!lvmtab_isvgdefined(clean_path)) {
		print_prgname();
		fprintf(stderr, MSG_VG_NOT_IN_LVMTAB, clean_path, LVMTABPATH);
		debug(dbg_exit());
		return(NULL);
	}

	strncpy(grp_path, clean_path, PATH_MAX - sizeof(GROUP) - 1);
	strcat(grp_path, "/");
	strcat(grp_path, GROUP);

	/* open the VG control file */
	if ((*vg_fd = open(grp_path, O_RDWR)) == -1) {
		print_prgname();
	 	fprintf(stderr, MSG_CANNOT_OPEN_GRP_FILE, grp_path);
	 	perror("");
		debug_msg("(open)\n", NULL);
		debug(dbg_exit());
	 	return(NULL);
	}
	debug(dbg_exit());
	return(clean_path);
}


int
attach_pvs(int vg_fd, char **pvs, int pvs_cnt)
{
char *attachpv;
int 	i;
int	status = OK;

	debug(dbg_entry("attach_pvs"));

	for (i=0; i < pvs_cnt; i++) {
		attachpv = pvs[i];

		debug(dbg_attachpv_dump(attachpv));
		if (ioctl(vg_fd, LVM_ATTACHPV, attachpv) == -1) {
		        print_prgname();
		        fprintf(stderr, MSG_ATTACHPV_FAILED, attachpv);
      			lvm_perror(LVM_ATTACHPV);
		        debug_msg("ioctl(ATTACHPV)\n", NULL);
			status = NOT_OK;
			continue;
		}
	}
	debug(dbg_exit());
	return(status);
}



int
deattach_pvs(int vg_fd, char **pvs, int pvs_cnt, int mode)
{
int 	pv_key;
int 	i;
int	status = OK;

	debug(dbg_entry("deattach_pvs"));

	for (i=0; i < pvs_cnt; i++) {
		/*
		 * The function for deattching a physical volume is for
		 * some strange reason called LVM_REMOVEPV. LVM_ATTACHPV
		 * takes a pv_path as argumen, but LVM_REMOVEPV takes
		 * pv_key (very coherent !) as argument, so the pv_path
		 * has to be converted into a pv_key.
		 */
		pv_key = pvpathtopvkey(pvs[i]);

		debug(dbg_removepv_dump(pv_key));
		if (ioctl(vg_fd, LVM_REMOVEPV, &pv_key) == -1) {
			if (mode == WITH_WARNING) {
				print_prgname();
				fprintf(stderr, MSG_DEATTACHPV_FAILED, pvs[i],
							pv_key);
				lvm_perror(LVM_REMOVEPV);
			}
		        debug_msg("ioctl(REMOVEPV)\n", NULL);
			status = NOT_OK;
			continue;
		}
	}
	debug(dbg_exit());
	return(status);
}



int
multiple_query_lv(int vg_fd, char **lv_names, unsigned int lv_cnt,
		  struct lv_querylv **querylv_array_ptr) 
{
   register unsigned int i;
   struct lv_querylv *querylv_array;
   register struct lv_querylv *qlvp;
   dev_t lv_dev_num;

   debug(dbg_entry("multiple_query_lv"));

   /* Store a NULL pointer, just in case */
   *querylv_array_ptr = NULL;

   /* Allocate a table, and loop on ioctl */
   querylv_array = (struct lv_querylv *)
	    checked_alloc(lv_cnt * sizeof(struct lv_querylv));

   /* Use a pointer to access the structure; shorter name, faster access */
   qlvp = &querylv_array[0];
   for (i = 0; i < lv_cnt; i++, qlvp++) {

      /* If it's not a block device, then it can't be an LV */
      if (special_f_tst(lv_names[i], S_IFBLK, &lv_dev_num) == NOT_OK) {
	 print_prgname();
	 fprintf(stderr, MSG_NOT_LV, lv_names[i]);
	 debug(dbg_exit());
         return(NOT_OK);
      }

      /* Set input parameters */
      qlvp->minor_num = minor(lv_dev_num);

      /* Ask the driver some info */
      if (query_driver(vg_fd, LVM_QUERYLV, qlvp) < 0) {
	 print_prgname();
         fprintf(stderr, MSG_QUERYLV_FAILED, lv_names[i]);
         lvm_perror(LVM_QUERYLV);
	 debug(dbg_exit());
         return(NOT_OK);
      }
   }

   /* Return (indirectly) the table with the info on PV */
   *querylv_array_ptr = querylv_array;
   debug(dbg_exit());
   return(OK);
}



int
multiple_query_pv(int vg_fd, char **pv_names, unsigned int pv_cnt,
		  struct lv_querypvpath **querypv_array_ptr, char **pv_queried) 
{
   register unsigned int i;
   register unsigned int got;
   struct lv_querypvpath *querypv_array;
   char *success_query;
   register struct lv_querypvpath *qpvp;

   debug(dbg_entry("multiple_query_pv"));

   /* Store a NULL pointer, just in case */
   *querypv_array_ptr = NULL;
   *pv_queried = NULL;

   /* Allocate a table, and loop on ioctl */
   querypv_array = (struct lv_querypvpath *)
	    checked_alloc(pv_cnt * sizeof(struct lv_querypvpath));
   success_query = checked_alloc(pv_cnt * sizeof(char));

   /* Use a pointer to access the structure; shorter name, faster access */
   qpvp = &querypv_array[0];
   for (got = 0, i = 0; i < pv_cnt; i++, qpvp++) {

      /* Set input parameters */
      qpvp->path = pv_names[i];

      debug_msg("query PV with path \"%s\"\n", qpvp->path);

      /* Ask the driver some info */
      success_query[i] = FALSE;
      if (query_driver(vg_fd, LVM_QUERYPVPATH, qpvp) < 0) {
         debug_msg("failed query\n", NULL);
	 print_prgname();
         fprintf(stderr, MSG_WARN_QUERYPVPATH_FAILED, qpvp->path);
         lvm_perror(LVM_QUERYPVPATH);
      }
      else {
         debug_msg("successful query\n", NULL);
         success_query[i] = TRUE;
	 got++;
      }
   }

   /* Return (indirectly) the table with the info on PV */
   *querypv_array_ptr = querypv_array;
   *pv_queried = success_query;
   debug_msg("returning %d\n", got);

   debug(dbg_exit());
   return(got);
}



int
query_driver(int vg_fd, int op_code, void *buf)
{
   int ret_val;
   cached_data cd;
   register cached_data *cdp;
   cache_key key;
   int data_size;
   char *save_addr;
   static int hit = 0, miss = 0;

   debug(dbg_entry("query_driver"));

   ret_val = 0;

   /* Depending on which QUERY we have to call, we use a different key */
   memset(&key, 0, sizeof(cache_key));
   switch (op_code) {
      case LVM_QUERYLV:     key.lv_min = ((struct lv_querylv *)buf)->minor_num;
			    data_size = sizeof(struct lv_querylv);
	 break;
      case LVM_QUERYLVMAP:  key.lv_min = ((struct lv_lvsize *)buf)->minor_num;
			    data_size = sizeof(struct lv_lvsize);
                            save_addr = (char *)
					((struct lv_lvsize *)buf)->extents;
	 break;
      case LVM_QUERYPV:     key.pv_key = ((struct lv_querypv *)buf)->pv_key;
			    data_size = sizeof(struct lv_querypv);
	 break;
      case LVM_QUERYPVMAP:  key.pv_key = ((struct lv_querypvmap *)buf)->pv_key;
			    data_size = sizeof(struct lv_querypvmap);
                            save_addr = (char *)
					((struct lv_querypvmap *)buf)->map;
	 break;
      case LVM_QUERYPVPATH: key.pv_path = ((struct lv_querypvpath *)buf)->path;
			    data_size = sizeof(struct lv_querypvpath);
	 break;
      case LVM_QUERYVG:     key.vg_fd = vg_fd;
			    data_size = sizeof(struct lv_queryvg);
	 break;
      default:
	    debug_msg("called with op_code = 0x%08x\n", op_code);
            debug(dbg_exit());
            return(-1);
   }

   /* Search for the info within the cache */
   cdp = cache_search(op_code, &key);

   /* Do we really have to call the driver? */
   if (cdp == NULL) {

      debug_msg("calling ioctl\n", NULL);
      debug(miss++);

      /* It's hard to avoid using #ifdef DEBUG, sometimes... */
      debug( op_code == LVM_QUERYLV ?
	        dbg_querylv_dump(buf, DBG_BEFORE) :
             op_code == LVM_QUERYLVMAP ?
		dbg_lvsize_dump(buf, DBG_BEFORE, DBG_WITHOUT_MAP) :
             op_code == LVM_QUERYPV ?
		dbg_querypv_dump(buf, DBG_BEFORE) :
             op_code == LVM_QUERYPVMAP ?
		dbg_querypvmap_dump(buf, DBG_BEFORE) :
             op_code == LVM_QUERYPVPATH ?
		dbg_querypvpath_dump(buf, DBG_BEFORE) :
             op_code == LVM_QUERYVG ?
		dbg_queryvg_dump(buf, DBG_BEFORE) :
		(void)NULL
	   );

      ret_val = ioctl(vg_fd, op_code, buf);

      /* See if we got something that can be stored in the cache */
      if (ret_val == 0) {

         /* It's hard to avoid using #ifdef DEBUG, sometimes... */
         debug( op_code == LVM_QUERYLV ?
	           dbg_querylv_dump(buf, DBG_AFTER) :
                op_code == LVM_QUERYLVMAP ?
		   dbg_lvsize_dump(buf, DBG_AFTER, DBG_WITH_MAP) :
                op_code == LVM_QUERYPV ?
		   dbg_querypv_dump(buf, DBG_AFTER) :
                op_code == LVM_QUERYPVMAP ?
		   dbg_querypvmap_dump(buf, DBG_AFTER) :
                op_code == LVM_QUERYPVPATH ?
		   dbg_querypvpath_dump(buf, DBG_AFTER) :
                op_code == LVM_QUERYVG ?
		   dbg_queryvg_dump(buf, DBG_AFTER) :
		   (void)NULL
	      );

	 cd.size = data_size;
	 cd.address = memdup(buf, data_size);

	 /*
	  *   Some of the data can be a reference to other data; duplicate
	  *   that too, before saving it in the cache. Awkward syntactical
	  *   constructs are necessary.
	  */
	 switch (op_code) {
	    case LVM_QUERYLVMAP:
		  ((struct lv_lvsize *)cd.address)->extents = 
			   (lxmap_t *)
			   memdup( (char *) ((struct lv_lvsize *)buf)->extents,
                                   ((struct lv_lvsize *)buf)->size *
                                   sizeof(lxmap_t) );
	       break;
	    case LVM_QUERYPVMAP:
		  ((struct lv_querypvmap *)cd.address)->map = 
			   (pxmap_t *)
			   memdup( (char *) ((struct lv_querypvmap *)buf)->map,
                                   ((struct lv_querypvmap *)buf)->numpxs *
                                   sizeof(pxmap_t) );
	       break;
	 }
	 cache_store(&cd, op_code, &key);
      }
      else {
	 /* ioctl failed */

         /* It's hard to avoid using #ifdef DEBUG, sometimes... */
         debug_msg( op_code == LVM_QUERYLV ?     "ioctl(QUERYLV)" :
                    op_code == LVM_QUERYLVMAP ?  "ioctl(QUERYLVMAP)" :
                    op_code == LVM_QUERYPV ?     "ioctl(QUERYPV)" :
                    op_code == LVM_QUERYPVMAP ?  "ioctl(QUERYPVMAP)" :
                    op_code == LVM_QUERYPVPATH ? "ioctl(QUERYPVPATH)" :
                    op_code == LVM_QUERYVG ?     "ioctl(QUERYVG)" :
		    NULL, NULL
	      );
      }
   }
   else {

      debug_msg("retrieved cache info\n", NULL);
      debug(hit++);

      /* Cached data have been retrieved */
      memcpy(buf, cdp->address, data_size);

      /* Restore pointers to indirect data */
      switch (op_code) {
         case LVM_QUERYLVMAP:
	       ((struct lv_lvsize *)buf)->extents = (lxmap_t *)save_addr;
	       memcpy( ((struct lv_lvsize *)buf)->extents,
                       ((struct lv_lvsize *)cdp->address)->extents,
                       ((struct lv_lvsize *)buf)->size * sizeof(lxmap_t) );
            break;
         case LVM_QUERYPVMAP:
	       ((struct lv_querypvmap *)buf)->map = (pxmap_t *)save_addr;
	       memcpy( ((struct lv_querypvmap *)buf)->map,
                       ((struct lv_querypvmap *)cdp->address)->map,
                       ((struct lv_querypvmap *)buf)->numpxs * sizeof(pxmap_t));
	    break;
      }
   }
   debug_msg("Cache hit:  %d\n", hit);
   debug_msg("Cache miss: %d\n", miss);

   debug(dbg_exit());
   return(ret_val);
}



void
disable_intr()
{
   static struct sigaction act;	   /* "static" so that it is 0 filled */
   static int caught_sig[] = {
      SIGINT, SIGTSTP, SIGQUIT, SIGTERM
   };
   register int i;

   debug(dbg_entry("disable_intr"));

   act.sa_handler = SIG_IGN;
   for (i = 0; i < entries(caught_sig); i++)
      sigaction(caught_sig[i], &act, NULL);

   debug(dbg_exit());
}



/*
 *   Local functions
 */

static cached_data *
cache_search(int type, cache_key *key)
{
   register int i;
   register info_cache *icp;

   debug(dbg_entry("cache_search"));

   /* Linear search; "type" is hierarchically more important than "key" */
   for (i = 0, icp = info_cache_tab; i < entries(info_cache_tab); i++, icp++)
      if (icp->in_use && icp->type == type && eq_keys(type, &icp->key, key)) {
         debug(dbg_exit());
	 return(&icp->info);
      }

   debug(dbg_exit());
   return(NULL);
}



static void 
cache_store(cached_data *cdp, int type, cache_key *key)
{
   register info_cache *icp;

   debug(dbg_entry("cache_store"));

   /* Get a slot */
   icp = &info_cache_tab[ic_next_to_use];

   /* Release old area, if any */
   if (icp->in_use)
      free(icp->info.address);

   /* Initialize this slot */
   icp->in_use = TRUE;
   icp->type = type;
   icp->key = *key;

   /* Make the hidden copy of the data */
   icp->info.size = cdp->size;
   icp->info.address = cdp->address;

   /* Round-robin slot allocation policy */
   if (++ic_next_to_use >= entries(info_cache_tab))
      ic_next_to_use = 0;

   debug(dbg_exit());
}
