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
static char	*sccsid = "@(#)$RCSfile: lvextend.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 93/02/09 14:27:53 $";
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
 *   lvextend:
 *   Increases the number
 *   of physical extents allocated to a logical volume.
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
static int expand_copies(int vg_fd, char *vgpath, unsigned short lv_flags, 
			 unsigned short lv_minor_num);

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

/*#define USAGE	"Usage: lvextend  {-l LogicalExtentsNumber | -m MirrorCopies}  \*/
/* LogicalVolumePath [ PhysicalVolumePath... ]\n"*/
/* TEMPORARY OPTION LIST - REMOVE TO RESTORE MIRRORING */
#define USAGE	"Usage: lvextend  {-l LogicalExtentsNumber}  \
 LogicalVolumePath [ PhysicalVolumePath... ]\n"

/* Options which require an argument for their value */
/*#define OPT_WITH_VAL_NUM	2 */
/*#define OPTIONS_WITH_VALUE	"lm" */
/* TEMPORARY OPTION LIST - REMOVE TO RESTORE MIRRORING */
#define OPT_WITH_VAL_NUM	1
#define OPTIONS_WITH_VALUE	"l"
char lflag; int LogicalExtentsNumber;
char mflag; int MirrorCopies;

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	0
#define OPTIONS_WITHOUT_VALUE	""

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char *LogicalVolumePath;

/* Extra args (optional) */
char **PhysicalVolumePath;	/* maybe a list => array of pointers */
int PhysicalVolumePath_cnt;	/* number of items in the above array */



main(int argc, char **argv)
{

	struct lv_querylv querylv;
        struct lv_statuslv statuslv;
	lx_descr_t *lv_map;
	char	*vgpath, *lvol_name;
	int	vg_fd, i, new_lx_cnt, old_lx_cnt, mirr_cnt = 0;
	int	new_mirr_cnt, old_mirr_cnt;


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
	 * Open the volume group if possible and query the logical
	 * volume LogicalVolumePath to get the current setting.
	 * Error messages are printed by openvg_and_querylv() in case
	 * of failure.
	 */
	if (openvg_and_querylv(LogicalVolumePath, &vgpath, &vg_fd,
				&querylv) != OK)
		exit(FATAL_ERROR);
	/*
	 * I wanted to test if there are enough physical extents available
	 * in the VG to hold the new logical extents (i.e., QUERYVG and
	 * see if "freepxs" is enough to hold the new extents), but since
	 * "freepxs" is calculated from "maxpxs" set by CREATEVG, and
	 * "maxpxs" is just a max value and not the number of physical extents
	 * actually available on the pv's in the VG, this doesn't make any 
	 * sense. 
	 * If there is really space enough can therefore only be decided by
	 * scanning the physical volume maps.
	 */

	/* 
	 * Test if PhysicalVolumePath are physical volumes belonging
	 * to the volume group vgpath. Since this info is keept in the
	 * /etc/lvmtab file, the lvmtab information first has to be read.
	 */
	if (lvmtab_read() != OK) {
		print_prgname();
		fprintf(stderr, MSG_LVMTAB_READ_ERROR, LVMTABPATH);
		exit(FATAL_ERROR);
	}
	for (i = 0; i < PhysicalVolumePath_cnt; i++) {
		if (!lvmtab_ispvinvg(vgpath, PhysicalVolumePath[i])) {
			print_prgname();
			fprintf(stderr, MSG_PV_NOT_IN_VG,
					vgpath, PhysicalVolumePath[i]);
			exit(FATAL_ERROR);
		}
	}

	/*
	 * Because of consistency reasons, the interupts are disabled
	 * from here on.
	 */
	disable_intr();

	/* 
	 * Test if number of mirrors are going to be extended or if
	 * the size of the logical volume is going to be extended
	 */
	if (mflag) {
		/* MirrorCopies must be bigger than current setting */
		if (MirrorCopies <= querylv.maxmirrors) {
			print_prgname();
			fprintf(stderr, MSG_MIRRORS_NOT_ADDED);
			exit(FATAL_ERROR);
		}

		/* 
		 * mirror can only be added if the LV has LE's
		 * allocated to it.
		 */
		if (querylv.numlxs == 0) {
			print_prgname();
			fprintf(stderr, MSG_NO_PE_ALLOCATED);
			exit(FATAL_ERROR);
		}


		new_mirr_cnt = MirrorCopies + 1;
		old_mirr_cnt = querylv.maxmirrors + 1;
		mirr_cnt = MirrorCopies;
		new_lx_cnt = querylv.numlxs;
		old_lx_cnt = 0;

	}
	else {
		/* LogicalExtentsNumber must be bigger than current setting */
		if (LogicalExtentsNumber <= querylv.numlxs) {
			print_prgname();
			fprintf(stderr, MSG_LE_NOT_ADDED);
			exit(FATAL_ERROR);
		}

		/* 
		 * If LogicalExtentsNumber > the implementation limit, 
		 * the logical volume cannot be extended
		 */
		if (LogicalExtentsNumber > LVM_MAXLXS) {
			print_prgname();
			fprintf(stderr, MSG_LVM_CANNOT_EXTEND);
			exit(FATAL_ERROR);
		}

		new_lx_cnt = LogicalExtentsNumber;
		old_lx_cnt = querylv.numlxs;
		new_mirr_cnt = querylv.maxmirrors + 1;
		old_mirr_cnt = 0;

                /* Notify the driver about the new max number of LE */
                statuslv.minor_num = querylv.minor_num;
                statuslv.maxlxs = LogicalExtentsNumber;
                statuslv.lv_flags = querylv.lv_flags;
                statuslv.sched_strat = querylv.sched_strat;
                statuslv.maxmirrors = querylv.maxmirrors;

                if (ioctl(vg_fd, LVM_CHANGELV, &statuslv) < 0) {
                        print_prgname();
                        fprintf(stderr, MSG_LV_NOT_CHANGED, LogicalVolumePath);
                        lvm_perror(LVM_CHANGELV);
                        debug_msg("ioctl(CHANGELV)\n", NULL);
                        exit(FATAL_ERROR);
                }
	}

	/* Fill the lv_map acording to the alocation policy */
	if (fill_lv_map(vg_fd, vgpath, LogicalVolumePath, querylv.minor_num, 
			querylv.lv_flags, &lv_map, PhysicalVolumePath, 
			PhysicalVolumePath_cnt, mirr_cnt, new_lx_cnt) != OK)

		exit(FATAL_ERROR);

	/* extend the logical volume */
	if (extend_lv_map(vg_fd, querylv.minor_num, LogicalVolumePath,
			lv_map, old_lx_cnt, old_mirr_cnt, new_lx_cnt, 
			new_mirr_cnt) != OK)

		exit(FATAL_ERROR);

	/*
	 * If you add mirrors to an LV the new mirrors are given
	 * the stale state by the driver because it doesn't know
	 * wether the original contains useful data or not.
	 * Since we want the LX's to have a current state, we
	 * have to synchronize the LV. If you have added many mirrors,
	 * this will take a long time.
	 */
	if (mflag) {
		printf(MSG_SYNC_TAKES_TIME);
		if (resync_lv(vg_fd, querylv.minor_num) != OK) {
			print_prgname();
			fprintf(stderr, MSG_CANT_SYNC_LV, LogicalVolumePath);
			exit(FATAL_ERROR);
		}
	}

	printf(MSG_LV_EXTENDED, LogicalVolumePath);




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

   if (bad_int_arg_value(&lflag, &LogicalExtentsNumber, 'l', "LogicalExtentsNumber"))
      return(NOT_OK);

/* DE-COMMENT TO FOLLOWING STATEMENT TO RESTORE MIRRORING */
/********   if (bad_int_arg_value(&mflag, &MirrorCopies, 'm', "MirrorCopies"))
      return(NOT_OK); ********/

   /* Set references to mandatory arguments */
   LogicalVolumePath = next_arg();

   /* Set references to extra arguments */
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
   /* At least "-m" or "-n" must be specified, but not both */
   if ((!mflag && !lflag) || (mflag && lflag)) {
      usage_error("-m | -n", MSG_ONE_OF_THEM_REQUIRED);
      return(NOT_OK);
   }

   /* Check for ranges */
   if (mflag && !in_range(MirrorCopies, 1, 2)) {
      usage_error("MirrorCopies", MSG_BETWEEN_1_AND_2);
      return(NOT_OK);
   }

   if (lflag && !in_range(LogicalExtentsNumber, 1, LVM_MAXLXS)) {
         usage_error("LogicalExtentsNumber", MSG_BETWEEN_1_AND_MAXLXS);
         return(NOT_OK);
   }


   /* Nothing to complain about */
   return(OK);
}



static int
set_defaults()
{
}

