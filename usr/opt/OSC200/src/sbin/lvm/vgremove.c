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
static char	*sccsid = "@(#)$RCSfile: vgremove.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/08 15:43:24 $";
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
 *   vgremove:
 *   Remove the definition of one or more volume groups from a set of physical
 *   volumes.
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
static int remove_vg(char *vg_path);

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

#define USAGE	"Usage: vgremove \
 VolumeGroupName... \n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	0
#define OPTIONS_WITH_VALUE	""

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	0
#define OPTIONS_WITHOUT_VALUE	""

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char **VolumeGroupName;	/* maybe a list => array of pointers */
int VolumeGroupName_cnt;	/* number of items in the above array */

/* There are no extra args (optional) */



main(int argc, char **argv)
{
   register char *vg_path;
   register int i;


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
   /* Read the lvmtab-file into memory */
   if (lvmtab_read() == NOT_OK) {
      print_prgname();
      fprintf(stderr, MSG_LVMTAB_READ_ERROR, LVMTABPATH);
      exit(FATAL_ERROR);
   }

   /* 
    * Because of consistency reasons, the interupts are disabled 
    * from here on.
    */
   disable_intr();

   /* Loop through the list of VG's supplied */
   for (i = 0; i < VolumeGroupName_cnt; i++) {

      vg_path = VolumeGroupName[i];
      if (remove_vg(vg_path) != OK) {
	 print_prgname();
	 fprintf(stderr, MSG_CANT_REMOVE_VG, vg_path);
	 return (FATAL_ERROR);
      }
      else
         printf(MSG_VG_REMOVED, vg_path);

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
   register int i;

   /* Set references to mandatory arguments */
   VolumeGroupName = left_arg(&VolumeGroupName_cnt);

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
remove_vg(char *vg_path)
{
   char *clean_path;
   int vg_fd;
   struct lv_queryvg queryvg;
   struct lv_querypvpath querypvpath;
   char **pvs_in_vg;
   int pvs_in_vg_cnt, pv_key;

   debug(dbg_entry("remove_vg"));

   /* Another library function takes care of generating the clean path */
   if ((clean_path = check_and_openvg(vg_path, &vg_fd)) == NULL) {
      debug(dbg_exit());
      return(NOT_OK);
   }
   
   if (query_driver(vg_fd, LVM_QUERYVG, &queryvg) < 0) {
      print_prgname();
      fprintf(stderr, MSG_QUERYVG_FAILED, vg_path);
      lvm_perror(LVM_QUERYVG);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* If VG still contains  LV's, we can't remove it */
   if (queryvg.cur_lvs > 0) {
      print_prgname();
      fprintf(stderr, MSG_VG_STILL_HAS_LV, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* At this stage the VG should only contain one PV */
   if (queryvg.cur_pvs > 1) {
      print_prgname();
      fprintf(stderr, MSG_VG_STILL_HAS_PV, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }


   if (lvmtab_read() != OK ||
   lvmtab_getpvnames(vg_path, &pvs_in_vg, &pvs_in_vg_cnt) != OK) {
      print_prgname();
      fprintf(stderr, MSG_NO_PVNAMES, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* At this stage we know that there is only one PV in the VG */
   querypvpath.path = *pvs_in_vg;
   debug(dbg_querypvpath_dump(&querypvpath, 0));
   if (ioctl(vg_fd, LVM_QUERYPVPATH, &querypvpath) < 0) {
      print_prgname();
      fprintf(stderr, MSG_QUERYPVPATH_FAILED, *pvs_in_vg);
      lvm_perror(LVM_QUERYPVPATH);
      debug(dbg_exit());
      return(NOT_OK);
   }
   debug(dbg_querypvpath_dump(&querypvpath, DBG_AFTER));
 
   /* The last physical volume must be empty before it can be removed */
   if (querypvpath.px_count != querypvpath.px_free) {
      print_prgname();
      fprintf(stderr, MSG_PX_ALLOCATED, querypvpath.path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   /* 
    * By deleting the last PV in the VG is the device drivers knowledge
    * about the VG removed.
    * The LVM_DELETEPV system call should put the VG in a non active
    * state.
    */
    pv_key = querypvpath.pv_key;
    debug(dbg_pvID_dump(&pv_key));
    if(ioctl(vg_fd, LVM_DELETEPV, &pv_key) == -1) {
       print_prgname();
       fprintf(stderr, MSG_DELETEPV_FAILED, *pvs_in_vg);
       lvm_perror(LVM_DELETEPV);
       debug_msg("ioctl(DELETEPV)\n", NULL);
       return(NOT_OK);
   }

   /* Delete the last physical volume from /etc/lvmtab */
   if (lvmtab_removepvfromvg(vg_path, querypvpath.path, DOWRITE) != OK) {
      /*
       * There is now an unconsistency between LVDD and
       * the /etc/lvmtab file.
       */
      print_prgname();
      fprintf(stderr, MSG_PV_NOT_DELETED, querypvpath.path,
                        LVMTABPATH, LVMTABPATH);
      return(NOT_OK);
   }

   /* Remove the entry from the lvmtab file */
   if (lvmtab_removevg(clean_path, DOWRITE) != OK) {
      print_prgname();
      fprintf(stderr, MSG_DEL_VG_FROM_LVMTAB, clean_path, LVMTABPATH);
      debug(dbg_exit());
      return(NOT_OK);
   }

   debug(dbg_exit());
   return(OK);
}
