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
static char	*sccsid = "@(#)$RCSfile: lvreduce.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 93/02/09 14:28:04 $";
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
 *   lvreduce:
 *   Decreases the number
 *   of physical extents allocated to a logical volume.
 */

/*
 *  Modification History:  lvreduce.c
 *
 *  06-Jun-91     Terry Carruthers
 *	Added code to set the new logical extent number for the
 *      logical volume, after the decrease has successfully 
 *      completed.
 *
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

/*#define USAGE	"Usage: lvreduce  {-m MirrorCopies | -l LogicalExtentsNumber} [\
-f]  \ */
/* LogicalVolumePath\n" */
/* TEMPORARY OPTION LIST - REMOVE TO RESTORE MIRRORING */
#define USAGE	"Usage: lvreduce  {-l LogicalExtentsNumber} [\
-f]  \
 LogicalVolumePath\n"

/* Options which require an argument for their value */
/*#define OPT_WITH_VAL_NUM	2 */
/*#define OPTIONS_WITH_VALUE	"ml" */
/* TEMPORARY OPTION LIST - REMOVE TO RESTORE MIRRORING */
#define OPT_WITH_VAL_NUM	1
#define OPTIONS_WITH_VALUE	"l"
char mflag; int MirrorCopies;
char lflag; int LogicalExtentsNumber;

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	1
#define OPTIONS_WITHOUT_VALUE	"f"
char fflag;

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char *LogicalVolumePath;

/* There are no extra args (optional) */



main(int argc, char **argv)
{

 	struct  lv_querylv      querylv;
 	struct  lv_statuslv     statuslv;
        char    *vgpath;
        int     vg_fd;


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
	 * Because of consistency reasons, the interupts are disabled 
	 * from here on.
	 */
	disable_intr();

	/*
         * Test if number of mirrors are going to be reduced or if
         * the size of the logical volume is going to be reduced
         */
        if (mflag) {
                /* MirrorCopies must be smaller than current setting */
                if (MirrorCopies >= querylv.maxmirrors) {
			print_prgname();
                        fprintf(stderr, MSG_MIRRORS_NOT_REMOVED);
                        exit(FATAL_ERROR);
                }
  
		/*
		 * cutdown_lv() will try to reduce the logical volume.
		 * All needed error messages will be printed by cutdown_lv()
		 * in case of error. Either OK or NOT_OK is returned.
		 */
		if (cutdown_lv(vg_fd, vgpath, querylv.minor_num,
			  	LogicalVolumePath, REMOVE_LV_MIRRORS, 
				MirrorCopies,
				fflag? DONT_ASK_USER : ASK_USER) != OK)

			exit(FATAL_ERROR);

		/*
		 * Tell the driver that the number of mirrors
		 * has been changed.
		 * Initialize lv_statuslv to be used by LVM_CHANGELV.
		 * The only element to be changed is the maxmirrors.
		 */
		statuslv.minor_num = querylv.minor_num;
		statuslv.maxlxs = querylv.maxlxs;
		statuslv.lv_flags = querylv.lv_flags;
		statuslv.sched_strat = querylv.sched_strat;
		statuslv.maxmirrors = MirrorCopies;

		/* Dump struct statuslv if in debug mode */
		debug(dbg_statuslv_dump(&statuslv));

		if (ioctl(vg_fd, LVM_CHANGELV, &statuslv) == -1) {
			print_prgname();
			fprintf(stderr, MSG_LV_NOT_CHANGED_BACK, 
				LogicalVolumePath);
			lvm_perror(LVM_CHANGELV);
			debug_msg("ioctl(CHANGELV)\n", NULL);
			return(NOT_OK);
		}

        }
        else {
		/*
		 * The logical volume can only be reduced if 
		 * LogicalExtentsNumber is smaller than current setting.
                 */
                if (LogicalExtentsNumber >= querylv.numlxs) {
			print_prgname();
                        fprintf(stderr, MSG_LVM_CANNOT_REDUCE,
                                        LogicalVolumePath);
			exit(FATAL_ERROR);
		}
		/*
		 * cutdown_lv() will try to reduce the logical volume.
		 * All needed error messages will be printed by cutdown_lv()
		 * in case of error. Either OK, NOT_OK is returned.
		 */
		if (cutdown_lv(vg_fd, vgpath, querylv.minor_num, 
				LogicalVolumePath, REMOVE_LV_EXTENTS, 
				LogicalExtentsNumber,
				fflag? DONT_ASK_USER : ASK_USER) != OK)
			exit(FATAL_ERROR);

		/* ULTRIX/OSF:  added this logic
		 *
		 * Notify the driver about the new maximum
		 * number of logical extents for this LV.
		 * 
		 * Initialize lv_statuslv to be used by LVM_CHANGELV.
		 * The only element to be changed is the maxlxs.
		 */
		statuslv.minor_num = querylv.minor_num;
		statuslv.maxlxs = LogicalExtentsNumber;
		statuslv.lv_flags = querylv.lv_flags;
		statuslv.sched_strat = querylv.sched_strat;
		statuslv.maxmirrors = querylv.maxmirrors;

		/* Dump struct statuslv if in debug mode */
		debug(dbg_statuslv_dump(&statuslv));

		if (ioctl(vg_fd, LVM_CHANGELV, &statuslv) == -1) {
			print_prgname();
			fprintf(stderr, MSG_LV_NOT_CHANGED_BACK, 
				LogicalVolumePath);
			lvm_perror(LVM_CHANGELV);
			debug_msg("ioctl(CHANGELV)\n", NULL);
			return(NOT_OK);
		}

	}
        


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

/* DE-COMMENT TO FOLLOWING STATEMENT TO RESTORE MIRRORING */
/********   if (bad_int_arg_value(&mflag, &MirrorCopies, 'm', "MirrorCopies"))
      return(NOT_OK); ********/

   if (bad_int_arg_value(&lflag, &LogicalExtentsNumber, 'l', "LogicalExtentsNumber"))
      return(NOT_OK);

   /* See which options without value have been used */
   fflag = used_opt('f');

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
   /* "-f" is a sub-option of "-l" */
   if (fflag && !lflag) {
      usage_error("-f", MSG_MINUS_L_REQUIRED);
      return(NOT_OK);
   }

   /* At least "-m" or "-l" must be specified, but not both */
   if ((!mflag && !lflag) || (mflag && lflag)) {
      usage_error("-m | -l", MSG_ONE_OF_THEM_REQUIRED);
      return(NOT_OK);
   }

   /* Check for ranges */
   if (mflag && !in_range(MirrorCopies, 0, 1)) {
      usage_error("MirrorCopies", MSG_BETWEEN_0_AND_1);
      return(NOT_OK);
   }

   if (lflag && !in_range(LogicalExtentsNumber, 0, LVM_MAXLXS)) {
         usage_error("LogicalExtentsNumber", MSG_BETWEEN_0_AND_MAXLXS);
         return(NOT_OK);
   }

   /* Nothing to complain about */
   return(OK);
}



static int
set_defaults()
{
}

