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
static char	*sccsid = "@(#)$RCSfile: vgreduce.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/08 15:43:13 $";
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
 *   vgreduce:
 *   Reduce a volume group by removing one or more physical volumes from it.
 */

/* Each file containing a main() has some privilege: see "lvmcmds.h" */
#define LVM_CMD_MAIN_FILE
#include "lvmcmds.h"

/*
 *   Here are all the declarations that are specific to this command,
 *   that is, file inclusions, definitions, variables, types, etc.
 */

/* Local functions */
static int check_usage_semantics();
static int set_defaults();
static int check_pvs(char *vg_path, char **input_pvs, int input_pv_cnt);

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

#define USAGE	"Usage: vgreduce \
 VolumeGroupName  PhysicalVolumePath... \n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	0
#define OPTIONS_WITH_VALUE	""

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	0
#define OPTIONS_WITHOUT_VALUE	""

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		2
char *VolumeGroupName;
char **PhysicalVolumePath;	/* maybe a list => array of pointers */
int PhysicalVolumePath_cnt;	/* number of items in the above array */

/* There are no extra args (optional) */



main(int argc, char **argv)
{

	struct	lv_querypvpath	querypvpath;
	char	*clean_vgpath;
	int	i, vg_fd, pv_key;
	int	status = OK;


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

	/*
	 * All, but one PV belonging to the VG, can be removed. In order to
	 * find out wether the sys. admin. has specified all PV's belonging
	 * to the VG or not, the driver has to be queried for the number
	 * of PV's belonging to the VG. This is done by check_pvs().
	 * Either OK or NOT_OK is returned.
	 * All error messages are printed by check_pvs().
	 */
	if (check_pvs(clean_vgpath, PhysicalVolumePath, PhysicalVolumePath_cnt) 
			!= OK)
		exit(FATAL_ERROR);

   	/* 
    	 * Because of consistency reasons, the interupts are disabled 
    	 * from here on.
    	 */
   	disable_intr();

	/*
	 * Try to remove all the physical volumes in PhysicalVolumePath
	 * from the volume group stored in clean_vgpath
	 */
	for (i = 0; i != PhysicalVolumePath_cnt; i++) {
		/*
		 * Test if PhysicalVolumePath is a block special file and
		 * returnes a clean path starting from root in the second
		 * argument. If successful OK is returned, else NOT_OK.
		 * isblock_and_clean() displays the needed error messages.
		 */
		if (isblock_and_clean(PhysicalVolumePath[i], 
				    &querypvpath.path, DONTCHECKLVMTAB) != OK) {
			status = NOT_OK;
			continue;
		}

		/* get information about the physical volume from LVDD */
		if (query_driver(vg_fd, LVM_QUERYPVPATH, &querypvpath) == -1) {
			print_prgname();
			fprintf(stderr, MSG_QUERYPVPATH_FAILED,
					querypvpath.path);
      			lvm_perror(LVM_QUERYPVPATH);
			status = NOT_OK;
			continue;
		}

		/* The physical volume has to be empty */
		if (querypvpath.px_count != querypvpath.px_free) {
			print_prgname();
			fprintf(stderr, MSG_PX_ALLOCATED, querypvpath.path);
			status = NOT_OK;
			continue;
		}

		/* Then finaly delete the physical volume from VG */
		pv_key = querypvpath.pv_key;
		debug(dbg_pvID_dump(&pv_key));
		if(ioctl(vg_fd, LVM_DELETEPV, &pv_key) == -1) {
			print_prgname();
			fprintf(stderr, MSG_DELETEPV_FAILED, 
					querypvpath.path);
      			lvm_perror(LVM_DELETEPV);
			debug_msg("ioctl(DELETEPV)\n", NULL);
			status = NOT_OK;
			continue;
		}

		/* Delete the physical volume from /etc/lvmtab */
		if (lvmtab_removepvfromvg(clean_vgpath, querypvpath.path,
					  DOWRITE) != OK) {
			/*
			 * There is now an unconsistency between LVDD and
			 * the /etc/lvmtab file.
			 */
			print_prgname();
			fprintf(stderr, MSG_PV_NOT_DELETED, querypvpath.path,
				LVMTABPATH, LVMTABPATH);
			status = NOT_OK;
			continue;
		}
	}
	if (status == OK)
   		printf(MSG_VG_REDUCED, VolumeGroupName);
	else
		return(FATAL_ERROR);	 
		

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

   /* Set references to mandatory arguments */
   VolumeGroupName = next_arg();
   PhysicalVolumePath = left_arg(&PhysicalVolumePath_cnt);

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

static int
check_pvs(char *vg_path, char **input_pvs, int input_pv_cnt)
{
	char **pvs_in_vg;
	int pvs_in_vg_cnt;
	int i;

   	debug(dbg_entry("check_pvs"));

	/* Some information has to be retrieved from the PV's */
	if (lvmtab_read() != OK ||
	lvmtab_getpvnames(vg_path, &pvs_in_vg, &pvs_in_vg_cnt) != OK) {
		print_prgname();
		fprintf(stderr, MSG_NO_PVNAMES, vg_path);
		debug(dbg_exit());
		return(NOT_OK);
	}

	/* Nr. of PV's on the command line must be less thatn PV's in the VG */
	if (input_pv_cnt >= pvs_in_vg_cnt) {
		print_prgname();
		fprintf(stderr, MSG_TO_MANY_PVS);
		debug(dbg_exit());
		return(NOT_OK);
	}

	for (i = 0; i < input_pv_cnt; i++) {
		if (!lvmtab_ispvinvg(vg_path, input_pvs[i])) {
			print_prgname();
			fprintf(stderr, MSG_PV_NOT_IN_VG, input_pvs[i], vg_path);
			debug(dbg_exit());
			return(NOT_OK);
		}
	}
	debug(dbg_exit());
	return(OK);
}
		
