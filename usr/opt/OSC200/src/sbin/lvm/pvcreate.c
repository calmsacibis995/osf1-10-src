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
static char	*sccsid = "@(#)$RCSfile: pvcreate.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:49:11 $";
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
 *   pvcreate:
 *   Creates a physical volume that can be used as part of a volume group.
 */

/*
 *  Modification History:  pvcreate.c
 *
 *  24-Apr-91     Terry Carruthers
 *	Changed the get_device_size function to correctly interpret
 *      the status returned from the getdiskbyname routine call.
 *      Also modified get_device_size to check operator supplied
 *      device type input (-t disktype) against the actual disk
 *      type, if the device driver has an ioctl command to 
 *      supply that information.
 *
 */

/* Each file containing a main() has some privilege: see "lvmcmds.h" */
#define LVM_CMD_MAIN_FILE
#include "lvmcmds.h"

/*
 *   Here are all the declarations that are specific to this command,
 *   that is, file inclusions, definitions, variables, types, etc.
 */
#include <sys/param.h>

#ifdef multimax
#include <layout.h>
#include <mmaxio/msioctl.h>
#define MMAX_MAXPARTITIONS	MAXPARTITIONS
#undef MAXPARTITIONS
#endif

#include <sys/disklabel.h>
#include <sys/types.h>
#include <lvm/lv_defect.h>
#include <lvm/pvres.h>
#include <ufs/fs.h>
#include <sys/file.h>
#include <s5fs/s5param.h>
#include <s5fs/filsys.h>

/* Special marks for some areas of the physical volume */
#define DEFECT_MARK	"DEFECT01"
#define LVMREC_MARK	"LVMREC01"
#define sblock          (*sblk.b_un.b_fs)

/* File descriptor of special file that is to become a PV */
int fdev;

/* Block-sized buffer to write block-sized areas */
char block_buf[DEV_BSIZE];

/* How we mark the bad blocks as given by the user */
#define MANUFACTURER_CODE	(1 << 28)
#define BB_PVMANUF(num)	(MANUFACTURER_CODE | ((num) & 0xfffffff))

/* Local functions */
static int check_usage_semantics();
static int set_defaults();
static int open_pv();
static int write_LVM_record();
static int init_badblock_dir();
static int pv_in_use();
static int can_use_pv();
static int bsd_sb_read();
static int s5_sb_read();

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

#define USAGE "Usage: pvcreate [-b] [-f] [-t DiskType] PhysicalVolumePath\n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	2
#define OPTIONS_WITH_VALUE	"st"
char tflag; char *DiskType;
char sflag; ulong_t DiskSize = 0;

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	2
#define OPTIONS_WITHOUT_VALUE	"bf"
char bflag;
char fflag;

/* Required arguments (mandatory) */
#define REQ_ARGS_NUM		1
char *PhysicalVolumePath;

/* There are no extra args (optional) */



main(int argc, char **argv)
{

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

   /* 
    * Because of consistency reasons, the interupts are disabled 
    * from here on.
    */
   disable_intr();

   /*
    *   Quite straightforward; just access the raw device, and spread
    *   some information there.
    */
   
   if (open_pv() != OK ||
       write_LVM_record() != OK ||
       init_badblock_dir() != OK)
      exit(FATAL_ERROR);

   printf(MSG_PV_CREATED, PhysicalVolumePath);

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
   register char *cp;

   /*
    *   See which options with value have been used;
    *   save a flag, and the value if supplied;
    *   whenever possible, check for correct usage
    *   of values for options
    */

   if (sflag = used_opt('s')) DiskSize = atoi(value_of_opt('s'));
   if (tflag = used_opt('t')) DiskType = value_of_opt('t');

   /* See which options without value have been used */
   bflag = used_opt('b');
   fflag = used_opt('f');

   /* Set references to mandatory arguments */
   PhysicalVolumePath = next_arg();

   /* Check if too many arguments have been typed */
   if ((cp = next_arg()) != NULL) {
      usage_error(cp, "Too many arguments.");
      return(NOT_OK);
   }

   return(OK);
}



/*
 *   Here are all the subroutines that are specific to this command,
 *   that is, routines that don't fit into the LVM cmd's library
 */

static int
check_usage_semantics()
{

   /* Nothing to complain about */
   return(OK);
}



static int
set_defaults()
{
}

int
get_device_size(special, fsi, disktype)
char *special;
int fsi;
char *disktype;
{
int unavailable = FALSE;
/* ULTRIX/OSF:  Added disktype_notused variable */
int disktype_notused = TRUE;
ulong_t nsectors = 0;
int partno;
struct disklabel label, *lp;
struct stat statbuf;
#ifdef multimax
#define BUFSIZE	((sizeof(layout_t) + DEV_BSIZE - 1) & ~(DEV_BSIZE -1))
static char layout_info[BUFSIZE];
layout_t *layout=(layout_t *)&layout_info[0];
#endif
#ifdef i386
#include <i386/disk.h>
struct disk_parms disk_parms;
#endif
/* ULTRIX/OSF:  Added devget and DEVIOCGET definitions */

/* Basic amount of storage for "interface" and "device" below */
#define DEV_SIZE	0x08		/* Eight bytes			*/

/* Structure for DEVIOCGET ioctl - device get status command */
struct	devget	{
	short	category;		/* Category			*/
	short	bus;			/* Bus				*/
	char	interface[DEV_SIZE];	/* Interface (string)		*/
	char	device[DEV_SIZE];	/* Device (string)		*/
	short	adpt_num;		/* Adapter number		*/
	short	nexus_num;		/* Nexus or node on adapter no. */
	short	bus_num;		/* Bus number			*/
	short	ctlr_num;		/* Controller number		*/
	short	rctlr_num;		/* Remote controller number	*/
	short	slave_num;		/* Plug or line number		*/
	char	dev_name[DEV_SIZE];	/* Ultrix device pneumonic	*/
	short	unit_num;		/* Ultrix device unit number	*/
	unsigned soft_count;		/* Driver soft error count	*/
	unsigned hard_count;		/* Driver hard error count	*/
	long	stat;			/* Generic status mask		*/
	long	category_stat;		/* Category specific mask	*/
}devget;

#define DEVIOCGET	_IOR('v', 1, struct devget)	/* Get device info */

char DT[DEV_SIZE];
char *ptr1;
int i;

   debug(dbg_entry("get_device_size"));

   if (fstat(fsi, &statbuf) < 0) {
	print_prgname();
	fprintf(stderr, "Internal error: cannot fstat open device.\n");
	debug(dbg_exit());
	exit(-1);
   }
#if	defined(FUTURE_WORK)
   if (statbuf.st_blocks != 0) {
	/* Need to decide if it makes more sense to return size
	 * in blocks as st_blocks, or size in bytes as st_size, or
	 * both. There is still the problem of determining the
	 * sector size (which we don't deal with at all yet.) */
	nsectors = statbuf.st_blocks;
   } else 
#endif
   /*
    * Attempt to read a disklabel. If not found, fall back to
    * machine-dependent disk information, then to /etc/disktab.
    */
   if (ioctl(fsi, DIOCGDINFO, (char *)&label) >= 0) {
	/*
	 * It has a label. We need to figure out which partition
	 * it is. We use the low bits of the minor device number,
	 */
	lp = &label;
	debug_print_disk_label(lp);
	/* The following calculation depends on MAXPARITIONS being
	 * a power of 2. (8, 16, etc.) */
	partno = minor(statbuf.st_rdev) & (MAXPARTITIONS-1);
	if (partno < lp->d_npartitions) {
		nsectors = lp->d_partitions[partno].p_size;
	} else {
		unavailable = TRUE;
	}
   } else
#if multimax
   if (mmax_get_header_info(special, layout) >= 0) {
	/* multimax MAXPARTITIONS is enormous (64). How would
	 * this get encoded in a pathname? */
	partno = minor(statbuf.st_rdev) & (MAXPARTITIONS - 1);
        if ((nsectors = layout->partitions[partno].part_size) == 0) {
		unavailable = TRUE;
	}
   } else
#endif
#if i386
   if (ioctl(fdev, V_GETPARMS, &disk_parms) >= 0) {
	partno = minor(statbuf.st_rdev) & (V_NUMPAR-1);
	if (disk_parms.dp_pflag & V_VALID) {
		nsectors = disk_parms.dp_pnumsec;
	} else {
		unavailable = TRUE;
	}
   } else
#endif
   if (disktype) {
	/* ULTRIX/OSF:  Added code to clear disktype_notused variable */
	disktype_notused = FALSE;
	/* ULTRIX/OSF:  Added code to get device type information
	 *              from the physical drive.  Best to get this
	 *              information from the driver, if possible, even if
	 *              the user supplies the disk type.  Its our only
         *              chance to check for operator entry error.
	 */
	if (ioctl(fsi, DEVIOCGET, &devget) >= 0) {
            ptr1 = disktype;
            i = 0;
            while ( *ptr1 != '\0') {
	        DT[i++] = (char) toupper((int)*ptr1++);
	    }
            DT[i] = '\0';
            if (strcmp(DT,devget.device) != 0) {
	        prgname_perror(disktype);
		return(nsectors);
	    }
        }
        /* ULTRIX/OSF:  changed != to == in following if statement */
        if ((lp = getdiskbyname(disktype)) == NULL) {
            prgname_perror(disktype);
        } else {
            debug_print_disk_label(lp);
            partno = minor(statbuf.st_rdev) & (MAXPARTITIONS-1);
            if (partno < lp->d_npartitions) {
	        nsectors = lp->d_partitions[partno].p_size;
            } else {
	        unavailable = TRUE;
            }
        }
   } else {
	print_prgname();
      	fprintf(stderr, MSG_SPECIFY_DTYPE, special);
      	perror("");
      	debug_msg("ioctl(DIOCGDINFO)", NULL);
   }
   if (unavailable) {
	print_prgname();
	fprintf(stderr, MSG_UNAVAIL_DPART, special, partno + 'a');
   }

   /* ULTRIX/OSF:  Added code to print how device information
    *              was obtained.
    */
   if (disktype && disktype_notused)
       printf(MSG_DISKTYPE_NOTUSED);

   debug(dbg_exit());
   return(nsectors);
}
#ifdef multimax
/*
 * This code unabashedly taken from newfs.
 */
char	headerdev[MAXPATHLEN];

mmax_get_header_info(special, layout)
char *special;
layout_t *layout;
{
        char *headerptr, *cp, *tcp;
        int fd;

        /*
         * open the header parition of this disk. The 4th partition of
         * this disk, ie., rmdxd, will be used as header diskpartition.
         */
        headerptr = &headerdev[0];
        strcpy(headerptr, special);

        tcp = index(headerptr, '\0') - 1;
        cp = tcp;

        if (isdigit(*tcp))
                *tcp ='3';
        else
                *tcp = 'd';

        fd = open(headerptr, O_RDONLY);
        if (fd < 0) {
		return(-1);
        }

        if (ioctl(fd, (int)MSIOCRDLAY, (char *)layout) == -1) {
                return(-1);
        }

	return(0);
}
#endif

debug_print_disk_label(lp)
struct disklabel *lp;
{
   debug_msg("pvcreate.getdisklabel:\n", NULL);
   debug_msg("\td_typename:   %s\n", lp->d_typename);
   debug_msg("\td_nsectors:   %d\n", lp->d_nsectors);
   debug_msg("\td_ntracks:    %d\n", lp->d_ntracks);
   debug_msg("\td_ncylinders: %d\n", lp->d_ncylinders);
   debug_msg("\td_secpercyl:  %d\n", lp->d_secpercyl);
   debug_msg("\td_secperunit: %d\n", lp->d_secperunit);
   debug_msg("\td_secsize:    %d\n", lp->d_secsize);
   debug_msg("\td_npartitions:%d\n", lp->d_npartitions);
}

static int
open_pv()
{
   struct stat st;

   /*
    *   Open the device and make sure that it is a character special file.
    */

   debug(dbg_entry("open_pv"));

   if ((fdev = open(PhysicalVolumePath, O_RDWR)) < 0) {
      print_prgname();
      fprintf(stderr, MSG_OPENPV, PhysicalVolumePath);
      perror("");
      debug_msg("open()\n", NULL);
      debug(dbg_exit());
      return(NOT_OK);
   }

   if (fstat(fdev, &st) < 0) {
      print_prgname();
      fprintf(stderr, MSG_STATPV, PhysicalVolumePath);
      perror("");
      debug_msg("fstat()\n", NULL);
      debug(dbg_exit());
      return(NOT_OK);
   }

   if ((st.st_mode & S_IFMT) != S_IFCHR) {
      print_prgname();
      fprintf(stderr, MSG_NOT_CHARDEV, PhysicalVolumePath);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /*
    *   It is an error if the physical volume already belongs to a VG
    */
   if (pv_in_use()) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   /*
    *   Test if the disk has a FS or not
    */
   if (!can_use_pv()) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   debug_msg("Exiting from open_pv()\n", NULL);
   debug(dbg_exit());
   return(OK);
}



static int
write_LVM_record()
{
   lv_lvmrec_t *lvmrec;
   unsigned long pv_sectors;


   debug(dbg_entry("write_LVM_record"));

   /*
    *   Construct the LVM record.
    */
   if (DiskSize == 0) {
	   pv_sectors = get_device_size(PhysicalVolumePath, fdev, DiskType);
   } else {
	   pv_sectors = DiskSize;
   }
   if (pv_sectors == 0) {
      debug(dbg_exit());
      return(NOT_OK);
   }

   bzero(block_buf, DEV_BSIZE);
   lvmrec = (lv_lvmrec_t *)block_buf;

   /* Store the index of the last accessible physical sector */
   lvmrec->last_psn = pv_sectors - 1;
   strncpy(lvmrec->lvm_id, LVMREC_MARK, sizeof(lvmrec->lvm_id));
   lvmrec->pv_id.id1 = gethostid();
   lvmrec->pv_id.id2 = time(NULL);
   
   /*
    *   Write the LVM record to the appropriate sectors on pv.
    */

   if (lseek(fdev, PVRA_LVM_REC_SN1 * DEV_BSIZE, L_SET) < 0 ||
       write(fdev, lvmrec, DEV_BSIZE) != DEV_BSIZE ||
       lseek(fdev, PVRA_LVM_REC_SN2 * DEV_BSIZE, L_SET) < 0 ||
       write(fdev, lvmrec, DEV_BSIZE) != DEV_BSIZE) {
      
      prgname_perror(MSG_WRITE_LVMREC);
      debug(dbg_exit());
      return(NOT_OK);
   }

   debug(dbg_exit());
   return(OK);
}



static int
init_badblock_dir()
{
   register int i;
   register int bb_dir_blks;
   unsigned int bb_index;
   register lv_bblk_t *bbp;

   /* Array of bad block dir. entries; as many as fit in a block */
   lv_bblk_t bblock_array[DEV_BSIZE / sizeof(lv_bblk_t)];
   lv_bblk_t bblock_firstblock[DEV_BSIZE / sizeof(lv_bblk_t)];

   /*
    *   Clear the primary and secondary bad block directory.
    */

   bzero(block_buf, DEV_BSIZE);
   for (i = 0; i < PVRA_BBDIR_LENGTH; i++) {

      if (lseek(fdev, (PVRA_BBDIR_SN1 + i) * DEV_BSIZE, L_SET) < 0 ||
          write(fdev, block_buf, DEV_BSIZE) != DEV_BSIZE ||
          lseek(fdev, (PVRA_BBDIR_SN2 + i) * DEV_BSIZE, L_SET) < 0 ||
          write(fdev, block_buf, DEV_BSIZE) != DEV_BSIZE) {

         prgname_perror(MSG_CLEAR_BBDIR);
	 debug(dbg_exit());
         return(NOT_OK);
      }
   }

   /*
    *   We have to be a bit careful, here; the DEFECT mark has to be
    *   written when all the entries of bad blocks have already been
    *   written; since it resides in the first block, it has to 
    *   be initialized before all of the subsequent ones, but the actual
    *   recording of it has to be delayed until we're done with the list
    *   of bad blocks. So we initialize it with the DEFECT mark and,
    *   possibly, with the first 512 / 8 - 1 bad block entries; then we
    *   get from the user and write on the disk the next entries; and
    *   finally we record the mark and those first entries. All this
    *   mess is due to the fact that we're accessing a block device
    *   (though through the cdevsw), and the minimum amount of data
    *   that can be written is DEV_BSIZE.
    */

   /* Obvious cleanness... */
   bzero(bblock_firstblock, sizeof(bblock_firstblock));
   strcpy((char *)bblock_firstblock, DEFECT_MARK);

   /* Must read the list of bad blocks? */
   if (bflag) {

      /* Skip the very first entry; this contains the DEFECT01 label */
      i = 1;
      bbp = &bblock_firstblock[i];

      /* Count how many blocks of badblocks dir. entries we write on PV */
      bb_dir_blks = 0;

      /* Clean the (reusable) array */
      bzero(bblock_array, sizeof(bblock_array));

      while (read_uint(&bb_index) != EOF) {

	 /*
	  *   Does it fit? This check is here, to get out as soon as
	  *   we overflow
	  */

	 if (bb_dir_blks == PVRA_BBDIR_LENGTH) {
	    print_prgname();
	    fprintf(stderr, MSG_TOOMANY_BBLOCKS);
	    debug(dbg_exit());
	    return(NOT_OK);
	 }

	 /* Put it in the array */
	 bbp->defect_reason = BB_PVMANUF(bb_index);
	 i++;
	 bbp++;

	 /* Did we fill one of the two block-sized arrays? */
	 if (i == entries(bblock_array)) {

	    debug_msg("Filled block #%d\n", bb_dir_blks);

	    /* Write out the filled block, only if it's not the first one */
	    if (bbp == &bblock_array[entries(bblock_array)]) {

	       /* We have reached the end of the generic buffer. Write it */
               if (lseek(fdev, (PVRA_BBDIR_SN1 + bb_dir_blks) * DEV_BSIZE,
			    L_SET) < 0 ||
                   write(fdev, (char *)bblock_array, DEV_BSIZE) != DEV_BSIZE ||
                   lseek(fdev, (PVRA_BBDIR_SN2 + bb_dir_blks) * DEV_BSIZE,
			    L_SET) < 0 ||
                   write(fdev, (char *)bblock_array, DEV_BSIZE) != DEV_BSIZE) {

                  prgname_perror(MSG_WRITE_BBDIR);
		  debug(dbg_exit());
                  return(NOT_OK);
               }
	    }

	    /*
	     *   Get back to the beginning of the array; note the we don't use
	     *   anymore the bblock_firstblock, which will be written later on
	     */

            i = 0;
            bbp = &bblock_array[i];

            /* Clean the (reusable) array */
            bzero(bblock_array, sizeof(bblock_array));

	    /* Increment the number of bad blocks descriptor blocks written */
	    ++bb_dir_blks;
	 }
      }

      /* Some entry left in the buffer? (apart from those of the first block) */
      if (bbp > &bblock_array[0] &&
	       bbp < &bblock_array[entries(bblock_array)]) {
	 /* We have reached the end of the generic buffer. Write it */
         if (lseek(fdev, (PVRA_BBDIR_SN1 + bb_dir_blks) * DEV_BSIZE,
			    L_SET) < 0 ||
             write(fdev, (char *)bblock_array, DEV_BSIZE) != DEV_BSIZE ||
             lseek(fdev, (PVRA_BBDIR_SN2 + bb_dir_blks) * DEV_BSIZE,
			    L_SET) < 0 ||
             write(fdev, (char *)bblock_array, DEV_BSIZE) != DEV_BSIZE) {

            prgname_perror(MSG_WRITE_BBDIR);
	    debug(dbg_exit());
            return(NOT_OK);
         }
      }
   }

   /*
    *   At the end (when you've safely written all the bblock
    *   entries), write the bad block header at the appropriate
    *   places on the pv.
    */

   if (lseek(fdev, PVRA_BBDIR_SN1 * DEV_BSIZE, L_SET) < 0 ||
       write(fdev, (char *)bblock_firstblock, DEV_BSIZE) != DEV_BSIZE ||
       lseek(fdev, PVRA_BBDIR_SN2 * DEV_BSIZE, L_SET) < 0 ||
       write(fdev, (char *)bblock_firstblock, DEV_BSIZE) != DEV_BSIZE) {

      prgname_perror(MSG_WRITE_DEF_ENTRIES);
      debug(dbg_exit());
      return(NOT_OK);
   }

   debug(dbg_exit());
   return(OK);
}


static int
pv_in_use()
{
   lv_lvmrec_t *lvmrec;

   debug(dbg_entry("pv_in_use"));

   bzero(block_buf, DEV_BSIZE);
   lvmrec = (lv_lvmrec_t *)block_buf;

   /*
    *   Try to read one of the LVM-records from the disk. If we fail to read 
    *   it then it can't be a PV with a LVM-record on it
    */
   if (lseek(fdev, PVRA_LVM_REC_SN1 * DEV_BSIZE, L_SET) < 0 ||
       read(fdev, lvmrec, DEV_BSIZE) != DEV_BSIZE) {

       if (lseek(fdev, PVRA_LVM_REC_SN2 * DEV_BSIZE, L_SET) < 0 ||
           read(fdev, lvmrec, DEV_BSIZE) != DEV_BSIZE) {

          /*
	   *   If we fail to read the LVM-record then we can try to create
	   *   a physical volume on it.
	   */
	  debug_msg("Couldn't read the LVM record\n", NULL);
          debug(dbg_exit());
          return(FALSE);
      }
   }

   /*
    *   See if the physical volume already belongs to a VG (i.e.,
    *	the LVM Record tag exists, and  VG_ID != 0)
    */
	/* allows for version # to change */
   if ((bcmp(lvmrec, "LVMREC", 6) == 0)
	&& isdigit(lvmrec->lvm_id[6])
	&& isdigit(lvmrec->lvm_id[7])
	&& ((lvmrec->vg_id.id1 != 0) || (lvmrec->vg_id.id2 != 0))) {
      print_prgname();
      fprintf(stderr, MSG_VG_ID_ON_PV);
      debug(dbg_exit());
      return(TRUE);
   }

   debug(dbg_exit());
   return(FALSE);
}


static int
can_use_pv()
{
   char *yes, answer[10];

   debug(dbg_entry("can_use_pv"));

   /*
    *   The test if the physical volume containes a FS is only
    *   done if the -f flag is not set.
    */
   if (fflag) {
      debug(dbg_exit());
      return(TRUE);
   }

   /*
    *   If the disk has a FS on it ask for confirmation
    */
   if (bsd_sb_read() || s5_sb_read()) {
      printf(MSG_USER3_CONFIRMATION);
      if (read_line(answer, sizeof(answer)) == EOF) {
         print_prgname();
	 fprintf(stderr, MSG_BAD_INPUT_PARAMETER);
         debug(dbg_exit());
         return(FALSE);
      }

      /* Get the internationalized yes response */
      yes = nl_langinfo(YESSTR);

      debug_msg("user said \"%s\"\n", answer);
      debug_msg("nl_langinfo says \"%s\"\n", yes);

      if (*yes != *answer) {
         debug(dbg_exit());
         return(FALSE);
      }
   }

   debug(dbg_exit());
   return(TRUE);
}



static int
bsd_sb_read()
{
   char bsd_buf[SBSIZE];
   struct fs *bsd_sb;

   debug(dbg_entry("bsd_sb_read"));

   if ((lseek(fdev, SBOFF, 0) < 0) ||
       (read(fdev, bsd_buf, SBSIZE) < 0)) {

      debug_msg("BSD super block couldn't be read\n", NULL);
      debug(dbg_exit());
      return(FALSE);
   }

   bsd_sb = (struct fs *)bsd_buf;

#if SEC_FSCHANGE
   if ((bsd_sb->fs_magic != FS_MAGIC) && (bsd_sb->fs_magic != FS_SEC_MAGIC)) {
#else
   if (bsd_sb->fs_magic != FS_MAGIC) {
#endif
      debug_msg("Block read isn't a BSD super block\n", NULL);
      debug(dbg_exit());
      return(FALSE);
   }

   debug(dbg_exit());
   return(TRUE);
}


static int
s5_sb_read()
{
   char s5_buf[MAX_S5BSIZE];
   struct filsys *s5_sb;

   debug(dbg_entry("s5_sb_read"));

   /* 
    *   Try to read the system V super block
    */
   if (lseek(fdev, (long)SUPERBOFF,0) < 0 || 
       read(fdev, s5_buf, MAX_S5BSIZE) != MAX_S5BSIZE) {

      debug_msg("Sys V super block couldn't be read\n", NULL);
      debug(dbg_exit());
      return(FALSE);
   }

   s5_sb = (struct filsys *)s5_buf;

   if (s5_sb->s_magic != FsMAGIC ||
       s5_sb->s_type < Fs1b ||
       s5_sb->s_type > Fs3b) {

      debug_msg("Block read isn't a Sys V super block\n", NULL);
      debug(dbg_exit());
      return(FALSE);
   }

   debug(dbg_exit());
   return(TRUE);
}

