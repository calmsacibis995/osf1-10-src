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
static char     *sccsid = "@(#)$RCSfile: tables.c,v $ $Revision: 4.2.18.8 $ (DEC) $Date: 1993/11/02 15:31:51 $";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1987 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/


/************************************************************************
 *									
 * Name: tables.c
 *
 * Modification History
 * 
 * Jun 5, 1991 - jaa
 *	ported to OSF/1.  modified to use new bus/controller/device
 *	kernel structures.  in porting, we don't use /dev/kmem anymore.
 *	getsysinfo(3) was modified to return the needed information.
 *	NOTE: ifdef TODO's in this code will have to be done as the 
 *	pieces are added (i.e. floating csr space, workstation specifics)
 *
 * 10-Aug-90	Randall Brown
 *	Added support for DS5000_100 (3MIN).
 *	Added supprot for devices scc and fb.
 *
 * 09-Aug-90	Fred L. Templin
 *	Added line for "FZA"
 *
 * 03-Aug-90	rafiey (Ali Rafieymehr)
 * 	Added support for VAX9000.
 * 
 * May 21, 1990 - Robin
 * 	Added presto NVRAM support
 *
 * Mar 2, 1990 - Randall Brown
 *	Added support for 2da and 3da graphics boards
 *
 * Dec 6, 1989 - Alan Frechette
 *	Added support for MIPSFAIR2.
 *
 * Nov 3, 1989 - Alan Frechette
 *	Added vmebus adapter type.
 *
 * Oct 11, 1989 - Alan Frechette
 *	Added support for 3MAX and PMAX.
 *
 * July 14, 1989 - Alan Frechette
 *	Added separate "config_device[]" table for mips. Mark which
 *	devices are supported and which devices are not supported.
 *	Added support for RIGEL (VAX_6400), added new (XMINODE),
 *	added new (KDM) controller.
 *
 * July 6, 1989 - Alan Frechette
 *	Added define for VAXSTAR cputype in cpu config table.
 *
 * May 10, 1989 - Alan Frechette
 *	Added an entry to the name list structure to get the 
 *	"roottype" for determining network boots.
 *
 * May 02, 1989 - Alan Frechette
 *	Changes to deal with new unique "cpu" handling for both
 *	vax and mips architectures. 
 *
 * Feb 12, 1989 - Alan Frechette
 *	New sizer code which supports multiple architectures.
 *      This file is a complete redesign and rewrite of the 
 *	original V3.0 sizer code by Tungning Cherng.
 *
 ***********************************************************************/

#include "sizer.h"

/********************************************************************
 *
 * Table Structure for configuring the NODE information.
 *
 *******************************************************************/
struct config_node nodetbl[] = {
    { "bi",		"node",		BINODE	},
    { "xmi",		"node",		XMINODE		},
    { "ci",		"cinode",	CINODE		},
    { "msi",		"msinode",	MSINODE		},
    { "\0",		"\0",		UNKNOWN		}
};

/********************************************************************
 *
 * Table Structure for configuring the CPU information.
 *
 *******************************************************************/
struct config_cpu cputbl[] = {
    { DS_3100,      "DS3100",  		32,	 "\0"      }, 
    { DS_5000,      "DS5000",		32,	 "\0"      }, 
    { DS_5000_100,  "DS5000_100",	32,	 "\0"      }, 
    { DS_5000_300,  "DS5000_300",	64,	 "\0"      }, 
#ifndef __alpha
    { DS_MAXINE,    "DSPERSONAL_DECSTATION", 	32,	 "\0"      },
#endif
    { DS_5100,      "DS5100",		32,	 "\0"      }, 
    { DS_5400,      "DS5400",		128,	 "\0"      }, 
    { DS_5500,      "DS5500",		128,	 "\0"      }, 
    { DS_5800,      "DS5800",		128,	 "\0"      }, 
#ifdef __alpha
    { ALPHA_ADU,    "ALPHAADU",		64,	 "\0"      }, 
    { DEC_4000,     "DEC4000",		32,	 "\0"      }, 
    { DEC_3000_500, "DEC3000_500",	32,	 "\0"      }, 
    { DEC_3000_300, "DEC3000_300",	32,	 "\0"      }, 
    { DEC_2000_300, "DEC2000_300",	32,	 "\0"      }, 
    { DEC_7000,     "DEC7000",		128,	 "\0"      }, 
#endif
    { UNKNOWN,	    "UNDEFINED_CPU",	0,	 "\0"  	   } 
};

/********************************************************************
 *
 * Table Structure for configuring the DEVICE information.
 *
 *******************************************************************/

struct config_device devtbl[] = {
    { "ci",	"bus",            0,1,	"\0",		0, BUS },
    { "ibus",	"bus",            0,1,	"\0",		0, BUS },
    { "msi",	"bus",            0,1,	"msi_isr",	0, BUS },
    { "uba",	"bus",            0,1,	"\0",		0, BUS },
    { "bi",	"bus",            0,1,	"\0",		0, BUS },
    { "vba",	"bus",            0,1,	"vbaerrors",	0, BUS },
    { "xmi",	"bus",            0,1,	"xmierror",	0, BUS },
    { "tc",	"bus",            0,1,	"\0",		0, BUS },
    { "lbus",	"bus",            0,1,	"\0",		0, BUS },
    { "fbus",	"bus",            0,1,	"\0",		0, BUS },
    { "isa",	"bus",            0,1,	"\0",		0, BUS },
    { "eisa",	"bus",            0,1,	"\0",		0, BUS },
    { "lsb",	"bus",            0,1,	"\0",		0, BUS },
    { "iop",	"bus",            0,1,	"ioperror",    	0, BUS },
    { "xza",	"bus",            0,1,	"\0",      	0, BUS },
    { "tcds",	"bus",		  0,1,	"tcdsintr",	0, BUS },
    { "nvtc",	"controller",     0,1,	"nvtcintr",	0, CONTROLLER },
    { "envram",	"controller",     0,1,	"\0",	        0, CONTROLLER },
    { "siop",	"controller",     0,1,	"siopcointr",	0, CONTROLLER },
    { "aie",	"controller",     0,1,	"\0",		0, CONTROLLER },
    { "aio",	"controller",     0,1,	"\0",		0, CONTROLLER },
    { "asc",	"controller",     0,1,	"ascintr",	0, CONTROLLER },
    { "aha",	"controller",     0,1,	"ahaintr",	0, CONTROLLER },
    { "tza",  	"controller",     0,1,  "kztsa_intr",   0, CONTROLLER },
    { "bvpssp",	"controller",     0,1,	"bvpsspintr",	0, CONTROLLER },
    { "dc",	"controller",     1,1,	"dcintr",	0, CONTROLLER },
    { "ace",	"controller",     1,1,	"aceintr",	0, CONTROLLER },
    { "gpc",	"controller",     0,1,	"gpcintr",	0, CONTROLLER },
    { "lp",	"controller",	  1,1,	"lpintr",	0, CONTROLLER },
    { "dssc",	"controller",     0,1,	"\0",		0, CONTROLLER },
    { "dti",	"controller",	  1,1,	"dtiintr",	0, CONTROLLER },
    { "fx",	"controller",     0,0,	"rxintr",	0, CONTROLLER },
    { "fdi",	"controller",	  0,1,	"fdintr",	0, CONTROLLER },
    { "hk",	"controller",     0,0,	"rkintr",	0, CONTROLLER },
    { "hl",	"controller",     0,0,	"rlintr",	0, CONTROLLER },
    { "hsc",	"controller",     0,1,	"\0",		0, CONTROLLER },
    { "idc",	"controller",     0,0,	"idcintr",	0, CONTROLLER },
    { "kdb",	"controller",     0,1,	"\0",		0, CONTROLLER },
    { "kdm",	"controller",     0,1,	"\0",		0, CONTROLLER },
    { "klesib",	"controller",     0,1,	"\0",		0, CONTROLLER },
    { "klesiu",	"controller",     0,1,	"\0",		0, CONTROLLER },
    { "ln",	"controller",     0,1,	"lnintr",	0, CONTROLLER },
    { "fza",	"controller",     0,1,	"fzaintr",	0, CONTROLLER },
    { "fta",	"controller",     0,1,	"ftaintr",	0, CONTROLLER },
    { "faa",	"controller",     0,1,	"faaintr",	0, CONTROLLER },
    { "tra",	"controller",     0,1,	"traintr",	0, CONTROLLER },
    { "rqd",	"controller",     0,0,	"\0",		0, CONTROLLER },
    { "sc",	"controller",     0,1,	"upintr",	0, CONTROLLER },
    { "scsi",	"controller",     0,1,	"szintr",	0, CONTROLLER },
    { "skz",	"controller",     0,1,	"\0",   	0, CONTROLLER },
    { "sdc",	"controller",     0,1,	"sdintr",	0, CONTROLLER },
    { "sii",	"controller",     0,1,	"sii_intr",	0, CONTROLLER },
    { "stc",	"controller",     0,1,	"stintr",	0, CONTROLLER },
    { "tm",	"controller",     0,0,	"tmintr",	0, CONTROLLER },
    { "uda",	"controller",     0,1,	"\0",		0, CONTROLLER },
    { "uq",	"controller",     0,1,	"uqintr",	0, CONTROLLER },
    { "ut",	"controller",     0,0,	"utintr",	0, CONTROLLER },
    { "va",	"controller",     0,0,	"\0",		0, CONTROLLER },
    { "zs",	"controller",     0,0,	"tsintr",	0, CONTROLLER },
    { "cfb",	"controller",     0,1,	"cfbvint", 	0, CONTROLLER },
    { "fb",	"controller",     0,1,	"fbint", 	0, CONTROLLER },
    { "px",	"controller",     0,1,	"pxintr", 	0, CONTROLLER },
    { "pv",	"controller",     0,1,	"pvintr", 	0, CONTROLLER },
    { "vga",	"controller",     0,1,	"vgaintr", 	0, CONTROLLER },
    { "scc", 	"controller",     1,1,	"sccintr",	0, CONTROLLER },
    { "ne",	"controller",	  0,1,	"neintr",	0, CONTROLLER },
    { "te",	"controller",	  0,1,	"teintr",	0, CONTROLLER },
    { "bba",	"controller",	  0,1,	"bbaintr",	0, CONTROLLER },
    { "ra",	"device disk",    1,1,	"\0",		0, DISK },
    { "rb",	"device disk",    1,0,	"\0",		0, DISK },
    { "rd",	"device disk",    1,0,	"\0",		0, DISK },
    { "rk",	"device disk",	  1,0,	"\0",		0, DISK },
    { "rl",	"device disk",	  1,0,	"\0",		0, DISK },
    { "rx",	"device disk",	  1,1,	"\0",		0, DISK },
    { "rz",	"device disk",	  1,1,	"\0",		0, DISK },
    { "up",	"device disk",	  1,0,	"\0",		0, DISK },
    { "urx",	"device disk",	  1,0,	"\0",		0, DISK },
    { "vz",	"device disk",	  1,0,	"\0",		0, DISK },
    { "mu",	"device tape",	  1,0,	"\0",		0, TAPE },
    { "st",	"device tape",	  1,0,	"\0",		0, TAPE },
    { "tj",	"device tape",	  1,0,	"\0",		0, TAPE },
    { "tms",	"device tape",	  1,1,	"\0",		0, TAPE },
    { "ts",	"device tape",	  1,0,	"\0",		0, TAPE },
    { "tu",	"device tape",	  1,0,	"\0",		0, TAPE },
    { "tz",	"device tape",	  1,1,	"\0",		0, TAPE },
    { "fd",	"device disk",	  1,1,	"\0",		0, DISK },
    { "acc",	"device",	  0,0,	"accrint accxint",0, DEVICE },
    { "ad",	"device",	  1,0,	"\0",		0, DEVICE },
    { "bvpni", 	"device",	  0,1,	"bvpniintr",	0, DEVICE },
    { "css",	"device",	  0,0,	"cssrint cssxint",0xa, DEVICE },
    { "ct",	"device",	  1,0,	"\0", 		0, DEVICE },
    { "de",	"device",	  0,1,	"deintr",	0, DEVICE },
    { "dh",	"device",	  1,0,	"dhrint dhxint",0xffff, DEVICE },
    { "dhu",	"device",	  1,1,	"dhurint dhuxint",0xffff, DEVICE },
    { "dm",	"device",	  1,0,	"dmintr", 	0xffff, DEVICE },
    { "dmb",	"device",	  1,1,	"dmbsint dmbaint dmblint",0xff, DEVICE },
    { "dmc",	"device",	  0,0,	"dmcrint dmcxint",0, DEVICE },
    { "dmf",	"device",	  1,0,
	      "dmfsrint dmfsxint dmfdaint dmfdbint dmfrint dmfxint dmflint", 
	      0xff, DEVICE },
    { "dmv",	"device",	  0,0,	"dmvrint dmvxint",0, DEVICE },
    { "dmz",	"device",	  1,0,
      	      "dmzrinta dmzxinta dmzrintb dmzxintb dmzrintc dmzxintc",
	      0xffffff, DEVICE },
    { "dn",	"device",	  0,0,	"dnintr",	0, DEVICE },
    { "dpv",	"device",	  0,0,	"dpvrint dpvxint",0, DEVICE },
    { "dup",	"device",	  0,0,	"duprint dupxint",0, DEVICE },
    { "dz",	"device",	  1,0,	"dzrint dzxint",0xff, DEVICE },
    { "ec",	"device",	  0,0,	"ecrint ecxint eccollide",0, DEVICE },
    { "en",	"device",	  0,0,	"enrint enxint encollide", 0, DEVICE },
    { "fc",	"device",	  1,0,	"fcxrint",	0xf, DEVICE },
    { "fg",	"device",	  1,0,	"fgvint",	0xf, DEVICE },
    { "ga",	"device",	  0,1,	"gaintr", 	0, DEVICE },
    { "gq",	"device",	  0,1,	"gqintr", 	0, DEVICE },
    { "hy",	"device",	  0,0,	"hyint",	0, DEVICE },
    { "ik",	"device",	  1,0,	"ikintr", 	0, DEVICE },
    { "il",	"device",	  0,0,	"ilrint ilcint",0, DEVICE },
    { "kg",	"device",	  1,0,	"\0", 		0, DEVICE },
    { "lx",	"device",	  1,0, 	"lxbvpint",	0, DEVICE },
    { "mdc",	"device",	  1,1, 	"mdcintr",	0, DEVICE },
    { "pcl",	"device",	  0,0,	"pclxint pclrint",0, DEVICE },
    { "pm",	"device",	  0,1,	"pmvint", 	0, DEVICE },
    { "ps",	"device",	  1,0,	"\0", 		0, DEVICE },
    { "qd",	"device",	  1,0,	"qddint qdaint qdiint",	0xf, DEVICE },
    { "qe",	"controller",	  0,1,	"qeintr",	0,CONTROLLER },
    { "qv",	"device",	  1,0,	"qvkint qvvint",0xf, DEVICE },
    { "se",	"device",	  0,1,	"seintr",	0, DEVICE },
    { "sg",	"device",	  1,0,	"sgaint sgfint",0xf, DEVICE },
    { "sh",	"device",	  1,0,	"shrint shxint",0xff, DEVICE },
    { "sm",	"device",	  1,0,	"smvint",	0xf, DEVICE },
    { "ss",	"device",	  1,0,	"ssrint ssxint",0xf, DEVICE },
    { "sz",	"device",	  0,1,	"\0",		0, DEVICE },
    { "un",	"device",	  0,0,	"unintr",	0, DEVICE },
    { "uu",	"device",	  1,0,	"\0", 		0, DEVICE },
    { "vp",	"device",	  1,0,	"vpintr",	0, DEVICE },
    { "vv",	"device",	  1,0,	"vvrint vvxint",0, DEVICE },
    { "xna",	"controller",	  0,1,	"xnaintr", 	0, CONTROLLER },
    { "mfa",	"controller",	  0,1,	"mfaintr", 	0, CONTROLLER },
    { "\0",	"",               0,0,	"\0", 		0, DEVICE }
};

/********************************************************************
 *
 * Table Structure for defining the configuration file HARDWARE
 * NAME types.
 *
 *******************************************************************/
struct hardware hardtbl[] = {
    { "bus"		},
    { "controller"	},
    { "master"		},
    { "disk"		},
    { "tape"		},
    { "device"		},
    { "\0"		}
};

/********************************************************************
 *
 * Table Structure for defining the NAMELIST kernel elements we
 * need to search for.
 *
 ********************************************************************/
struct nlist nl[] = {
	{ /* 1 */	"_umem"			},
	{ /* 2 */	"_qmem"			},
	{ /* 3 */	"_ci_first_port"	},
	{ /* 4 */	"_dmb_lines"		},
	{ /* 5 */	"_dhu_lines"		},
	{ /* end */	"\0"			} 
};

/********************************************************************
 *
 * Table Structure for defining the FLOATING ADDRESS DEVICES that
 * we need to search for.
 *
 *******************************************************************/
struct float_devices floattbl[] = {
	{ "dj11",	04 	},	
	{ "dh",		010 	},	
	{ "dq11",	04 	}, 
	{ "du11",	04 	}, 	
	{ "dup",	04 	},	
	{ "lk11",	04 	}, 
	{ "dmc", 	04 	},	
	{ "dz", 	04 	}, 	
	{ "kmc11",	04 	}, 
	{ "lpp11", 	04 	},	
	{ "vmv21",	04 	},	
	{ "vmv31",	010 	},
	{ "dwr70", 	04 	},	
	{ "hl",		04 	},	
	{ "lpa11",	010 	},
	{ "kw11c", 	04 	}, 	
	{ "rsv",	04 	},	
	{ "fx",   	04 	}, 
	{ "un",	 	04 	},	
	{ "hy",   	04 	}, 	
	{ "dmp11",	04 	}, 
	{ "dpv",  	04 	},	
	{ "isb11",	04 	},	
	{ "dmv", 	010 	},
	{ "de",	 	04 	},	
	{ "uda",	02 	},	
	{ "dmf",  	020 	},
	{ "kms11", 	010 	}, 	
	{ "vs100",	010 	},	
	{ "klesiu",	02 	},
	{ "kmv11", 	010 	},	
	{ "dhu",  	010 	}, 	
	{ "dmz", 	020 	},
	{ "cpi32",	020 	},	
	{ "qvss",	040 	},
	{ "vs31",	04 	}, 
	{ "dtqna", 	04 	},	
	{ "csam",	04 	},	
	{ "adv11c",	04 	},
	{ "aav11c",	04 	}, 	
	{ "axv11c",	04 	},	
	{ "kwv11c",	02 	},
	{ "adv11d", 	04 	},	
	{ "aav11d",	04 	}, 	
	{ "drq3p", 	04 	},
	{ "\0", 	0  	}
};
int Flttblsize  = (sizeof(floattbl)/sizeof(floattbl[0]));

/* Define the sizes of all the above tables */
int Nodetblsize = (sizeof(nodetbl)/sizeof(nodetbl[0]));
int Cputblsize  = (sizeof(cputbl)/sizeof(cputbl[0]));
int Devtblsize  = (sizeof(devtbl)/sizeof(devtbl[0]));
int Nltblsize  	= (sizeof(nl)/sizeof(nl[0]));
int Hardtblsize = (sizeof(hardtbl)/sizeof(hardtbl[0]));
