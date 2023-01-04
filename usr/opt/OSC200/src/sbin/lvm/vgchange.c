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
static char	*sccsid = "@(#)$RCSfile: vgchange.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:49:21 $";
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
 *   vgchange:
 *   Sets the status of a volume group to on or off.
 */

/* Each file containing a main() has some privilege: see "lvmcmds.h" */
#define LVM_CMD_MAIN_FILE
#include "lvmcmds.h"

/*
 *   Here are all the declarations that are specific to this command,
 *   that is, file inclusions, definitions, variables, types, etc.
 */

/* This is for the Status variable below */
#define ON	TRUE
#define OFF	(!ON)

int Status;

/* Local functions */
static int check_usage_semantics();
static int set_defaults();
static void sync_all_lvs(int vg_fd, char *vg_path);

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

#define USAGE	"Usage: vgchange -a Availability [-l] [\
-p] [-s] \
 VolumeGroupName\n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	1
#define OPTIONS_WITH_VALUE	"a"
char aflag; char Availability;        /* legal values: "yn" */

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	3
#define OPTIONS_WITHOUT_VALUE	"lps"
char lflag;
char pflag;
char sflag;

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char *VolumeGroupName;

/* There are no extra args (optional) */



main(int argc, char **argv)
{

	struct	lv_uniqueID	uniqueID;
        struct  lv_queryvg      queryvg;
	char	*clean_vgpath;
	char	**pv_in_vg;
	int	vg_fd, nr_pvs, actvg_flags, ret_code;


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
         * If successful, a clean path which can be used in
         * operations on /etc/lvmtab and vg_fd are returned, else
         * NULL is returned.
         * check_and_openvg() prints the needed error messages.
         */
	if ((clean_vgpath = check_and_openvg(VolumeGroupName, &vg_fd)) == NULL)
                exit(FATAL_ERROR);

        /* before doing any change to the VG, get the current status */
        if (query_driver(vg_fd, LVM_QUERYVG, &queryvg) == -1) {
                fprintf(stderr, MSG_QUERYVG_FAILED, VolumeGroupName);
                lvm_perror(LVM_QUERYVG);
                exit(FATAL_ERROR);
        }

	/* 
	 * Because of consistency reasons, the interupts are disabled 
	 * from here on.
	 */
	disable_intr();

	if (Status == ON) {
		/* The VG should be varied on */

                /* return straight away if the VG is already active */
                if ((queryvg.status & LVM_VGACTIVATED) != 0) {
   			printf(MSG_VG_CHANGED, VolumeGroupName);
                      	exit(OK);
                }

		/* get a list of all physical volumes in the VG */
		if (lvmtab_getpvnames(clean_vgpath, &pv_in_vg, &nr_pvs) != OK) {
			print_prgname();
			fprintf(stderr, MSG_CANT_GET_PV_NAMES, clean_vgpath);
			exit(FATAL_ERROR);
		}

		/* read the vg_id of VolumeGroupName from /etc/lvmtab */
		if (lvmtab_getvgid(clean_vgpath, &uniqueID) != OK) {
			print_prgname();
			fprintf(stderr, MSG_VG_ID_READ_ERROR, clean_vgpath,
				 LVMTABPATH);
			exit(FATAL_ERROR);
		}

		/* Dump struct uniqueID if in debug mode */
		debug(dbg_uniqueID_dump(&uniqueID));

		/* pass VG ID to LVDD */
		if (ioctl(vg_fd, LVM_SETVGID, &uniqueID) == -1) {
			/*
			 * If it fails it can, dependent on errno, only mean
			 * that the VG already has a vg_id.
			 */
			if (errno != EEXIST) {
				print_prgname();
				fprintf(stderr, MSG_SETVGID_FAILED,
							VolumeGroupName);
				lvm_perror(LVM_SETVGID);
				debug_msg("ioctl(SETVGID)\n", NULL);
				exit(FATAL_ERROR);
			}
		}

		/* 
		 * attach the physical volumes to VG. Error messages are
		 * printed by attach_pvs()
		 */
		ret_code = attach_pvs(vg_fd, pv_in_vg, nr_pvs);
		if (ret_code != OK && pflag) {
			/*
			 * all the PV's should have been attached,
			 * because of the pflag; if it failed,
			 * deattach and quit
			 */
			print_prgname();
			fprintf(stderr, MSG_NOT_ALL_PV_AVAIL, VolumeGroupName);

			/* prepare the path-key conversion table */
			if (pvkeys_and_names(vg_fd, clean_vgpath, &pv_in_vg,
					&nr_pvs) != OK) {
				/* a failure here is really weird! */
				print_prgname();
				fprintf(stderr, MSG_CANT_GET_PV_NAMES,
					clean_vgpath);
				exit(FATAL_ERROR);
			}

			/*
			 * note that pv_in_vg has been overwritten,
			 * but it's not a problem; now, clean up,
			 * deattaching the PV's (the key<->path table
			 * is needed by deattach_pvs)
			 */
			deattach_pvs(vg_fd, pv_in_vg, nr_pvs, NO_WARNING);
			exit(FATAL_ERROR);
		}

		/* Initialize struct activevg with defaults values */
		actvg_flags = LVM_ACTIVATE_LVS;
		/*
		 * Initialize the activatevg.flags dependent on flags set 
		 * on the command line.
		 */
		if (pflag)
			actvg_flags |= LVM_ALL_PVS_REQUIRED;

		if (lflag)
			actvg_flags &= ~LVM_ACTIVATE_LVS;

		debug(dbg_activatevg_dump(&actvg_flags));
		if (ioctl(vg_fd, LVM_ACTIVATEVG, &actvg_flags) == -1) {
			print_prgname();
			fprintf(stderr, MSG_ACTIVATEVG_FAILED, VolumeGroupName);
      			lvm_perror(LVM_ACTIVATEVG);
			debug_msg("ioctl(ACTIVATEVG)\n", NULL);
			exit(FATAL_ERROR);
		}

		if (!sflag) {
			/*
			 * Synchronize all the LV's in the VG. If one of
			 * the LV's can't be synchronized, a warning is
			 * printed by sync_all_lvs()
			 */
			sync_all_lvs(vg_fd, clean_vgpath);
		}
	}
	else {
		/* The VG should be varied off */

                /* return straight away if the VG is already not active */
                if ((queryvg.status & LVM_VGACTIVATED) == 0) {
                        print_prgname();
   			printf(MSG_VG_CHANGED, VolumeGroupName);
                      	exit(OK);
                }

		/* get a list of all physical volumes in the VG */
		if (pvkeys_and_names(vg_fd, clean_vgpath, &pv_in_vg, &nr_pvs)
				!= OK) {
			print_prgname();
			fprintf(stderr, MSG_CANT_GET_PV_NAMES, clean_vgpath);
			exit(FATAL_ERROR);
		}

		/* Deactivate the VG */
		if (ioctl(vg_fd, LVM_DEACTIVATEVG, NULL) == -1) {
			print_prgname();
			fprintf(stderr, MSG_DEACTIVATEVG_FAILED,
					VolumeGroupName);
      			lvm_perror(LVM_DEACTIVATEVG);
			debug_msg("ioctl(DEACTIVATEVG)\n", NULL);
			exit(FATAL_ERROR);
		}

		/* 
		 * Temporary remove the PVs from VG. Error messages are
		 * printed by deattach_pvs()
		 */
		deattach_pvs(vg_fd, pv_in_vg, nr_pvs, WITH_WARNING);
	}
   	printf(MSG_VG_CHANGED, VolumeGroupName);


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

   if (bad_char_arg_value(&aflag, &Availability, 'a', "Availability", "yn"))
      return(NOT_OK);

   /* See which options without value have been used */
   lflag = used_opt('l');
   pflag = used_opt('p');
   sflag = used_opt('s');

   /* Set references to mandatory arguments */
   VolumeGroupName = next_arg();

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

   /* "-a" must be supplied */
   if (!aflag) {
      usage_error("-a", MSG_SUPPLY_MINUS_A);
      return(NOT_OK);
   }
   Status = (Availability == 'y') ? ON : OFF;

   /* "-p" and "-s" can be used only if Status is on */
   if ((pflag || sflag) && Status != ON) {
      usage_error("-p | -s", MSG_STATUS_ON);
      return(NOT_OK);
   }

   /* Nothing to complain about */
   return(OK);
}



static int
set_defaults()
{
}



/* Synchronize all the LV's in vg_path */
static void
sync_all_lvs(int vg_fd, char *vg_path)
{
	char **lvs;
	int lv_cnt, i;

	debug(dbg_entry("sync_all_lvs"));

	/* Get the LV's belonging to vg_path */
	if (lvminors_and_names(vg_fd, vg_path, &lvs, &lv_cnt) == OK) {

		/* Loop through all the LV's in the VG and synchronize them */
		for (i = 0; i < lv_cnt; i++)
			resync_lv(vg_fd, lvpathtolvminor(lvs[i]));
	}

	debug(dbg_exit());
	return;
}
