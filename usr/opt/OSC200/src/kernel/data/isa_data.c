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

/* ka_alpha_pc_data.c
 *   initialized data structures specific to the alpha pc
 *
 *   Modification History
 *
 *   4-Sep-91  Joe Notarangelo
 *
 *             created this file
 */




struct apc_isa_struct{
  char *device_name;
  int  cntlr_dev;
  int  max_units;
  int  (*intr_handler)();
  int  intr_parameter;
};



/*
 *  char *device_name - name as it appears in config file, eg. "aic"
 *  int  cntlr_dev    - determines how the driver is probed as a controller or
 *                      as a device - dev = 0, controller = 1
 *  int  max_units  - number of possible units/controllers to check for in the
 *                        configuration
 *  int (*intr_handler)() - interrupt handler for driver
 *  int intr_parameter    - parameter to be passed to interrupt handler
 *                          (probably never used)
 */


#ifdef BINARY
extern struct apc_isa_struct apc_isa_drivers[];
extern int number_isa;

#define NO_ISA   (number_isa)

#else

#include "aha.h"
#if NAHA > 0
int ahaintr();
#else
int ahaintr() {};
#endif

#include "lp.h"
#if NLP > 0
int lpintr();
#else
int lpintr() {};
#endif


#include "ln.h"
#if NLN > 0
int lnintr();
#else
int lnintr() {};
#endif

struct apc_isa_struct apc_isa_drivers[] = {
/*  name        cntlr/dev   max     handler      param   */
/* wtfix -- these are configured separately now. */
#ifdef notdef
  { "scc",	   0,        2,     sccintr,      0 },   /* console uart */
  { "gpc",         0,        1,     gpcintr,      0 },   /* graphics device */
  { "ace",         1,        4,     aceintr,      0 },   /* serial line */
  { "aic",         1,        1,     aicintr,      0 },	 /* SCSI */
  { "wd",	   1,	     4,     wdintr,       0 },	 /* Ethernet. */
  { "lp",	   1,	     1,     lpintr,       0 },	 /* Parallel port. */
#endif /*notdef*/
  { "aha",         1,        1,     ahaintr,      0 },   /* SCSI aha 1742*/
  { "ln",	   1,	     1,     lnintr,       0 },	 /* DE422 Ethernet. */
};

#define NO_ISA   ( sizeof( apc_isa_drivers ) / sizeof( struct apc_isa_struct ) )
int number_isa = NO_ISA;

#endif BINARY

