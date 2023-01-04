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
static char	*sccsid = "@(#)$RCSfile: lvcreate.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 93/02/09 14:27:45 $";
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
 *   lvcreate:
 *   Creates a logical volume in a volume group.
 */

/* Each file containing a main() has some privilege: see "lvmcmds.h" */
#define LVM_CMD_MAIN_FILE
#include "lvmcmds.h"

/*
 *   Here are all the declarations that are specific to this command,
 *   that is, file inclusions, definitions, variables, types, etc.
 */

struct lv_querylv querylv;

/* Local functions */
static int check_usage_semantics();
static int set_defaults();
static char *mk_lv_name(int vg_fd, dev_t *minor_num);
static dev_t get_free_minor_number(int vg_fd);

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

/*#define USAGE	"Usage: lvcreate  [-d Schedule] [-l LogicalExtentsNumber] [\ */
/*-m MirrorCopies][-n LogicalVolumeName] [-p Permission] [\ */
/*-r Relocate] [-s Strict] [-v Verify] \ */
/* VolumeGroupName\n" */
/* TEMPORARY OPTION LIST - REMOVE TO RESTORE MIRRORING */
#define USAGE	"Usage: lvcreate  [-d Schedule] [-l LogicalExtentsNumber]\
 [-n LogicalVolumeName] [-p Permission] [\
-r Relocate] [-s Strict] [-v Verify] \
 VolumeGroupName\n"

/* Options which require an argument for their value */
/*#define OPT_WITH_VAL_NUM	8 */
/*#define OPTIONS_WITH_VALUE	"dlmnprsv" */
/* TEMPORARY OPTION LIST - REMOVE TO RESTORE MIRRORING */
#define OPT_WITH_VAL_NUM	7
#define OPTIONS_WITH_VALUE	"dlnprsv"
char dflag; char Schedule;            /* legal values: "ps" */
char lflag; int LogicalExtentsNumber;
char mflag; int MirrorCopies;
char nflag; char *LogicalVolumeName;
char pflag; char Permission;          /* legal values: "wr" */
char rflag; char Relocate;            /* legal values: "yn" */
char sflag; char Strict;              /* legal values: "yn" */
char vflag; char Verify;              /* legal values: "yn" */

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	0
#define OPTIONS_WITHOUT_VALUE	""

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char *VolumeGroupName;

/* There are no extra args (optional) */



main(int argc, char **argv)
{

	struct	lv_statuslv	statuslv;
	lx_descr_t *lv_map;
	char	*clean_vgpath;
	char	*lvol_name;
	char	lvol_path[PATH_MAX + 1];
	char	rlvol_path[PATH_MAX + 1];
	int	vg_fd;
	dev_t	dev_num, minor_num, major_num;
	int	min_num_to_ioctl, new_lx_cnt = 0, new_mirr_cnt = 1;
	char 	group_path[PATH_MAX + 1];


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
	 * check VolumeGroupName and open the VG control file.
	 * check_and_openvg() prints the needed error messages.
      	 */
      	if ((clean_vgpath = check_and_openvg(VolumeGroupName, &vg_fd)) == NULL)
		exit(FATAL_ERROR);

	/* Generate the logical volume name if it's not supplied */
	if (! nflag) {
		/* error messages are printed by mk_lv_name() */
		if ((lvol_name = mk_lv_name(vg_fd, &minor_num)) == NULL) 
			exit(FATAL_ERROR);
	}
	else {
		/* Get the minor and major number to be used by mknode */
		lvol_name = LogicalVolumeName;
		if ((minor_num = get_free_minor_number(vg_fd)) == 0)
			exit(FATAL_ERROR);
		if (special_f_tst(clean_vgpath, S_IFDIR, &dev_num) != OK) {
			print_prgname();
			fprintf(stderr, MSG_VG_NOT_READ, clean_vgpath);
			exit(FATAL_ERROR);
		}
	}

	/* Generate path's for raw and block device */
	strcpy(lvol_path, clean_vgpath);
	strcat(lvol_path, "/");
	strcpy(rlvol_path, lvol_path);
	strcat(lvol_path, lvol_name);
	strcat(rlvol_path, "r");
	strcat(rlvol_path, lvol_name);

	/* get the major number from the VG-group file */
	strcpy(group_path, clean_vgpath);
	strcat(group_path, "/");
	strcat(group_path, GROUP);

	if (special_f_tst(group_path, S_IFCHR, &dev_num) != OK) {
		print_prgname();
		fprintf(stderr, MSG_LV_NAME_NOT_GENERATED);
		debug(dbg_exit());
		return(NULL);
	}

	major_num = major(dev_num);


	/* It is an error if the logical volume already exists */
	if (special_f_tst(lvol_path, S_IFBLK, &dev_num) != NOT_OK) {
		print_prgname();
		fprintf(stderr, MSG_LV_ALREADY_EXISTS, lvol_path);
		exit(FATAL_ERROR);
	}


  	/*
   	 * initialize parts of struct statuslv.
	 * The variables on the right side of '=' are either initialized
     	 * by the default value or the value read from the command line
	 */
	if (lflag) {
		statuslv.maxlxs = LogicalExtentsNumber;
		new_lx_cnt = LogicalExtentsNumber;
	}
	else
		statuslv.maxlxs = 0;

	/*
	 * NOTE:
	 * maxmirrors must allways be set to 0 here in order for
	 * extend_lv_map() function to work correctly. 
	 */
	statuslv.maxmirrors = 0;

	if (mflag)
		new_mirr_cnt = MirrorCopies + 1;

	statuslv.minor_num = minor_num;
	statuslv.lv_flags = 0;

	if (Schedule == 's')
		statuslv.sched_strat = LVM_SEQUENTIAL;
	else
		statuslv.sched_strat = LVM_PARALLEL;

	if (Permission == 'r')
		statuslv.lv_flags |= LVM_RDONLY;

	if (Relocate == 'n')
		statuslv.lv_flags |= LVM_NORELOC;
		
	if (Strict == 'y')
		statuslv.lv_flags |= LVM_STRICT;
		
	if (Verify == 'y')
		statuslv.lv_flags |= LVM_VERIFY;

	/* Dump struct statuslv if in debug mode */
	debug(dbg_statuslv_dump(&statuslv));

	/* 
	 * Because of consistency reasons, the interupts are disabled 
	 * from here on.
	 */
	disable_intr();

	/* make the new logical volume known to LVDD */
	if (ioctl(vg_fd, LVM_CREATELV, &statuslv) == -1) {
		print_prgname();
		fprintf(stderr, MSG_LV_NOT_CREATED, lvol_path);
		lvm_perror(LVM_CREATELV);
		debug_msg("ioctl(CREATELV)", NULL);
		exit(FATAL_ERROR);
	}

	/* 
	 * Create the logical volume in the file-system. If it
	 * fails, the logical volume is deleted again from the LVDD
	 */
	if ((mknod(lvol_path, S_IFBLK|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, 
		  	makedev(major_num, minor_num)) == -1) ||
	    (mknod(rlvol_path, S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, 
			makedev(major_num, minor_num)) == -1)) {

		print_prgname();
		fprintf(stderr, MSG_LV_NOT_CR_IN_FS, lvol_path);
		perror("");
		debug_msg("mknod", NULL);

		min_num_to_ioctl = (int)statuslv.minor_num;
		debug(dbg_pvID_dump(&min_num_to_ioctl));

		if (ioctl(vg_fd, LVM_DELETELV, &min_num_to_ioctl) == -1) {
			print_prgname();
			fprintf(stderr, MSG_LV_NOT_DELETED, lvol_path);
			lvm_perror(LVM_DELETELV);
			debug_msg("ioctl(DELETELV)", NULL);
		}
		exit(FATAL_ERROR);
	}

	printf(MSG_LV_CREATED, lvol_path, statuslv.minor_num);

        /* Query the LV so that the struct querylv reflects the current setting */
        querylv.minor_num = minor_num;
        if (ioctl(vg_fd, LVM_QUERYLV, &querylv) == -1) {
                print_prgname();
                fprintf(stderr, MSG_LV_NOT_QUERIED, lvol_path);
                lvm_perror(LVM_QUERYLV);
                exit(FATAL_ERROR);
        }

	/* If lflag or mflag (or both) is set, fill lv_map, first for LX */
	if (lflag && fill_lv_map(vg_fd, clean_vgpath, lvol_path, 
			querylv.minor_num, querylv.lv_flags, &lv_map, NULL, 
			0, 0, LogicalExtentsNumber) != OK)
		exit(FATAL_ERROR);

	/* And then for mirrors */
	if (mflag && fill_lv_map(vg_fd, clean_vgpath, lvol_path, 
			querylv.minor_num, querylv.lv_flags, &lv_map, NULL, 
			0, MirrorCopies, 0) != OK)
		exit(FATAL_ERROR);

	/* Then extend the lv_map */
	if ((lflag || mflag) && extend_lv_map(vg_fd, querylv.minor_num,
			lvol_path, lv_map, querylv.numlxs, 
			querylv.maxmirrors, new_lx_cnt, new_mirr_cnt) != OK)

		exit(FATAL_ERROR);

	if (lflag || mflag)
		printf(MSG_LV_EXTENDED, lvol_path);


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

   if (bad_char_arg_value(&dflag, &Schedule, 'd', "Schedule", "ps"))
      return(NOT_OK);

   if (bad_int_arg_value(&lflag, &LogicalExtentsNumber, 'l', "LogicalExtentsNumber"))
      return(NOT_OK);

/* DE-COMMENT TO FOLLOWING STATEMENT TO RESTORE MIRRORING */
/********   if (bad_int_arg_value(&mflag, &MirrorCopies, 'm', "MirrorCopies"))
      return(NOT_OK); ********/

   if (nflag = used_opt('n')) LogicalVolumeName = value_of_opt('n');

   if (bad_char_arg_value(&pflag, &Permission, 'p', "Permission", "wr"))
      return(NOT_OK);

   if (bad_char_arg_value(&rflag, &Relocate, 'r', "Relocate", "yn"))
      return(NOT_OK);

   if (bad_char_arg_value(&sflag, &Strict, 's', "Strict", "yn"))
      return(NOT_OK);

   if (bad_char_arg_value(&vflag, &Verify, 'v', "Verify", "yn"))
      return(NOT_OK);

   /* Set references to mandatory arguments */
   VolumeGroupName = next_arg();

   /* Check if too many arguments have been typed */
   if ((cp = next_arg()) != NULL) {
      usage_error(cp, "Too many arguments.");
      return(NOT_OK);
   }

   return(OK);
}


static int
check_usage_semantics()
{
   /* Check for ranges */
   if (lflag && !in_range(LogicalExtentsNumber, 1, LVM_MAXLXS)) {
      usage_error("LogicalExtentsNumber", MSG_BETWEEN_1_AND_MAXLXS);
      return(NOT_OK);
   }

   if (mflag && !in_range(MirrorCopies, 1, 2)) {
      usage_error("MirrorCopies", MSG_BETWEEN_1_AND_2);
      return(NOT_OK);
   }

   if (mflag && !lflag) {
      usage_error("MirrorCopies", MSG_EMPTY_LV_WITH_MIRRORS);
      return(NOT_OK);
   }

   if (nflag && (strchr(LogicalVolumeName, '/') != NULL)) {
      usage_error("LogicalVolumeName", MSG_NOT_A_PATH_NAME);
      return(NOT_OK);
   }

   /* Nothing to complain about */
   return(OK);
}



static int
set_defaults()
{
   /* Values as defined in the manual pages */
   Schedule = 'p';
   Permission = 'w';
   Relocate = 'y';
   Strict = 'y';
   Verify = 'n';
}

static dev_t
get_free_minor_number(int vg_fd)
{
	
	dev_t 	i;
	
	debug(dbg_entry("get_free_minor_number"));

	/* get the next free minor number the LVDD */
	i = 0;
	do {
		i++;

		querylv.minor_num = minor(makedev(0, i));

		debug_msg("lvcreate.get_free_min...: try minor %d\n",
			querylv.minor_num);

		if (query_driver(vg_fd, LVM_QUERYLV, &querylv) == -1) {
			print_prgname();
			fprintf(stderr, MSG_LV_NAME_NOT_GENERATED);
			lvm_perror(LVM_QUERYLV);
			debug(dbg_exit());
			return(0);
		}

	} while (querylv.lv_flags & LVM_LVDEFINED);

	debug(dbg_exit());
	return(makedev(0, i));
}

static char *
mk_lv_name(int vg_fd, dev_t *minor_num)
{
	static 	char 	lvol_name[NAME_MAX + 1];

	debug(dbg_entry("mk_lv_name"));

	if ((*minor_num = minor(get_free_minor_number(vg_fd))) == 0){
		debug(dbg_exit());
		return(NULL);
	}

	debug_msg("lvcreate.mk_lv_name: minor = %d\n", *minor_num);

	strcat(lvol_name, LVPREFIX);
	sprintf(&lvol_name[strlen(lvol_name)], "%d", *minor_num);

	printf(MSG_GENERATED_LV_NAME, lvol_name);
	debug(dbg_exit());
	return(lvol_name);
}
