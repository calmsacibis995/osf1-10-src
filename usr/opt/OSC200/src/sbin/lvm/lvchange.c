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
static char	*sccsid = "@(#)$RCSfile: lvchange.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:48:44 $";
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
 *   lvchange :
 *   Changes the characteristics of a logical volume.
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

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

#define USAGE	"Usage: lvchange   [-a Availability] [-d Schedule] [\
-p Permission] [-r Relocate] [-s Strict] [\
-v Verify]  \
 LogicalVolumePath\n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	6
#define OPTIONS_WITH_VALUE	"adprsv"
char aflag; char Availability;        /* legal values: "yn" */
char dflag; char Schedule;            /* legal values: "ps" */
char pflag; char Permission;          /* legal values: "wr" */
char rflag; char Relocate;            /* legal values: "yn" */
char sflag; char Strict;              /* legal values: "yn" */
char vflag; char Verify;              /* legal values: "yn" */

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	0
#define OPTIONS_WITHOUT_VALUE	""

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char *LogicalVolumePath;

/* There are no extra args (optional) */



main(int argc, char **argv)
{

	struct	lv_querylv	querylv;
	struct	lv_statuslv	statuslv;
	char	*vgpath;
	char	*clean_vgpath;
	int 	vg_fd;


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


	/* Initialize the struct lv_statuslv to be used by LVM_CHANGELV */
	statuslv.minor_num = querylv.minor_num;
	statuslv.maxlxs = querylv.maxlxs;
	statuslv.lv_flags = querylv.lv_flags;
	statuslv.sched_strat = querylv.sched_strat;
	statuslv.maxmirrors = querylv.maxmirrors;

	if (aflag)
		if (Availability == 'y')
			statuslv.lv_flags &= ~LVM_DISABLED;
		else
			statuslv.lv_flags |= LVM_DISABLED;

	if (dflag)
		if (Schedule == 'p')
			statuslv.sched_strat = LVM_PARALLEL;
		else
			statuslv.sched_strat = LVM_SEQUENTIAL;

	if (pflag)
		if (Permission == 'w')
			statuslv.lv_flags &= ~LVM_RDONLY;
		else
			statuslv.lv_flags |= LVM_RDONLY;
	 
	if (rflag)
		if (Relocate == 'y')
			statuslv.lv_flags &= ~LVM_NORELOC;
		else
			statuslv.lv_flags |= LVM_NORELOC;

	if (sflag)
		if (Strict == 'y')
			statuslv.lv_flags |= LVM_STRICT;
		else
			statuslv.lv_flags &= ~LVM_STRICT;

	if (vflag)
		if (Verify == 'y')
			statuslv.lv_flags |= LVM_VERIFY;
		else
			statuslv.lv_flags &= ~LVM_VERIFY;

	/* Dump struct statuslv if in debug mode */
	debug(dbg_statuslv_dump(&statuslv));

	/* 
	 * Because of consistency reasons, the interupts are disabled 
	 * from here on.
	 */
	disable_intr();

	if (ioctl(vg_fd, LVM_CHANGELV, &statuslv) == -1) {
		print_prgname();
		fprintf(stderr, MSG_LV_NOT_CHANGED, LogicalVolumePath);
		lvm_perror(LVM_CHANGELV);
		debug_msg("ioctl(CHANGELV)\n", NULL);
		exit(FATAL_ERROR);
	}
	printf(MSG_LV_CHANGED, LogicalVolumePath);



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

   if (bad_char_arg_value(&dflag, &Schedule, 'd', "Schedule", "ps"))
      return(NOT_OK);

   if (bad_char_arg_value(&pflag, &Permission, 'p', "Permission", "wr"))
      return(NOT_OK);

   if (bad_char_arg_value(&rflag, &Relocate, 'r', "Relocate", "yn"))
      return(NOT_OK);

   if (bad_char_arg_value(&sflag, &Strict, 's', "Strict", "yn"))
      return(NOT_OK);

   if (bad_char_arg_value(&vflag, &Verify, 'v', "Verify", "yn"))
      return(NOT_OK);

   /* Set references to mandatory arguments */
   LogicalVolumePath = next_arg();

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
   if (!aflag && !dflag && !pflag && !rflag && !sflag && !vflag) {
      usage_error("-a|-d|-p|-r|-s|-v", MSG_ONE_OF_THEM);
      return(NOT_OK);
   }

   /* Nothing to complain about */
   return(OK);
}



static int
set_defaults()
{
}
