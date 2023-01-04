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
static char	*sccsid = "@(#)$RCSfile: pvchange.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:49:08 $";
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
 *   pvchange:
 *   Changes the characteristics of a physical volume in a volume group.
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

#define USAGE	"Usage: pvchange -x Extensibility \
 PhysicalVolumePath\n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	1
#define OPTIONS_WITH_VALUE	"x"
char xflag; char Extensibility;       /* legal values: "yn" */

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	0
#define OPTIONS_WITHOUT_VALUE	""

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char *PhysicalVolumePath;

/* There are no extra args (optional) */



main(int argc, char **argv)
{
   struct lv_querypvpath qpv;
   struct lv_changepv changepv;
   char *vg_path;
   char *clean_path;
   int vg_fd;
   unsigned int info_mask;


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
    *   Now, perform the actions, specific to this command,
    *   that main() is supposed to perform
    */

   /* Get path name of VG owning this PV */
   if (lvmtab_read() != OK ||
	    !lvmtab_ispvinsomevg(PhysicalVolumePath, &vg_path)) {
      print_prgname();
      fprintf(stderr, MSG_NO_VGFORPV, PhysicalVolumePath);
      exit(FATAL_ERROR);
   }

   /* Another library function takes care of generating the clean path */
   if ((clean_path = check_and_openvg(vg_path, &vg_fd)) == NULL)
      exit(FATAL_ERROR);
   
   /* Set input parameters */
   qpv.path = PhysicalVolumePath;

   /* Ask the driver some info */
   if (query_driver(vg_fd, LVM_QUERYPVPATH, &qpv) < 0) {
      print_prgname();
      fprintf(stderr, MSG_QUERYPVPATH_FAILED, PhysicalVolumePath);
      lvm_perror(LVM_QUERYPVPATH);
      exit(FATAL_ERROR);
   }

   /* Set the change-pv structure */
   changepv.pv_key = qpv.pv_key;

   /*
    *   The flags returned by LVM_QUERYPVPATH are not useful to LVM_CHANGEPV,
    *   except for LVM_NOTATTACHED and LVM_PVNOALLOC, which should be kept
    *   as they are.
    */

   changepv.pv_flags = qpv.pv_flags & (LVM_NOTATTACHED | LVM_PVNOALLOC);

   /* Now, set flags as requested by the user */
   if (xflag)
      if (Extensibility == 'n')
	 changepv.pv_flags |= LVM_PVNOALLOC;
      else
	 changepv.pv_flags &= ~LVM_PVNOALLOC;

   /* Dump struct changepv if in debug mode */
   debug(dbg_changepv_dump(&changepv));

   /* 
    * Because of consistency reasons, the interupts are disabled 
    * from here on.
    */
   disable_intr();

   /* Do the actual change */
   if (ioctl(vg_fd, LVM_CHANGEPV, &changepv) < 0) {
      print_prgname();
      fprintf(stderr, MSG_PV_NOT_CHANGED, PhysicalVolumePath);
      lvm_perror(LVM_CHANGEPV);
      debug_msg("ioctl(CHANGEPV)\n", NULL);
      exit(FATAL_ERROR);
   }
   printf(MSG_PV_CHANGED, PhysicalVolumePath);


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

   if (bad_char_arg_value(&xflag, &Extensibility, 'x', "Extensibility", "yn"))
      return(NOT_OK);

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
   /* "-x" must be supplied */
   if (!xflag) {
      usage_error("-x", MSG_SUPPLY_MINUS_X);
      return(NOT_OK);
   }

   /* Is PV a block device? */
   if (special_f_tst(PhysicalVolumePath, S_IFBLK, (dev_t *)NULL) == NOT_OK) {
      usage_error("PhysicalVolumePath", MSG_SUPPLY_BLOCK_DEV);
      return(NOT_OK);
   }

   /* Nothing to complain about */
   return(OK);
}



static int
set_defaults()
{
}
