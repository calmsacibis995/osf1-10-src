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
/*
 * @(#)$RCSfile: mklocalconf.h,v $ $Revision: 1.1.7.7 $ (DEC) $Date: 1993/12/15 20:04:55 $
 */

/* these two defines are just so that vi can find matching {} pairs
 */
#define	STBK	"{"
#define	EDBK	"}"

#define	BUF_LINES	1200

#define	LINE_SIZE	132

#define	TOKEN_LINES	 5
#define	TOKENS		15

/* used to form a linked list of lines
 */
typedef	struct	lns	{
		char	*buf;
	struct	lns	*nxt;
	} Line;

/* used to form a linked list of minor numbers and file names for the 
 * bassign_table
 */
typedef	struct	confminor {
			int	num;
			char	dname[LINE_SIZE];
		struct	confminor	*nxt;
	} Minor;

/* used to represent the UID and GID of special devices for the bassign_table
 */
typedef	struct	owner {
			int	flag;
			union {
				int	num;
				char	alphabetic[LINE_SIZE];
			} value;
	} Owner;

/* possible values for Owner.flag
 */
#define	NUMERIC	1
#define	ALPHA	2

/* used to store all information which be used at boot time to create device
 * special files
 */
typedef struct	bassign_table {
		char			bdir[LINE_SIZE];
		char			cdir[LINE_SIZE];
		Minor			*bminor;
		Minor			*cminor;
		Owner			user;
		Owner			group;
		int			mode;
	} Bassign_table;

/* used to create linked list of entries for the bdevsw and cdevsw tables.
 * each entry in the linked list is a representation of one entry in a devsw
 * table.
 */
typedef	struct	ents	{
		Line		*lines;
		int		flag;
		int		emajor;
	struct	ents		*nxt;
	} Entry;

/* possible values for Entry.flag:
 */

#define INVALID         -3
#define FREE            -2
#define FOOTER          -1
#define HEADER          0
#define TAKEN           1


/* number of entries a single entry in the bdevsw table
 */
#define	BMAX		8

/* number of entries a single entry in the cdevsw table
 */
/* The DDI/DKI kernel hook adds two extra fields to the cdevsw structure.
 * See the declaration of cdevsw in src/kernel/sys/conf.h for details.
*/
#define	CMAX		13

typedef	struct	new_devsw	{
		char		*config_name;
		char		*description;
		char		bdevsw[BMAX][32];
		char		cdevsw[CMAX][32];
		int		bmajor_pref;
		int		bmajor_assign;
		int		cmajor_pref;
		int		cmajor_assign;
		int		req;
		int		flag;
		Bassign_table	*assign;
	struct	new_devsw	*nxt;
	} New_devsw;

/* possible values for New_devsw.flag
 */
#define	BDEV_ONLY	1
#define	CDEV_ONLY	2
#define	BDEV_CDEV	(BDEV_ONLY | CDEV_ONLY)
#define NO_BDEV		4
#define NO_CDEV		8
/* possible values for New_devsw.req
 */
#define	NONE	0
#define	SAME	1

/* value for New_devsw.bmajor_pref and New_devsw.cmajor_pref to signal that
 * any major number may be assigned to this device
 */
#define	ANY	-1

/* flags passed between routines to indicate which table is being worked on
 */
#define	BDEV_FLAG	1
#define	CDEV_FLAG	2

/* tokens which make up a default entry in the bdevsw table
 */
char	default_bdevsw[BMAX][32] = {
	"nodev",		/* open routine */
	"nodev",		/* close routine */
	"nodev",		/* strategy routine */
	"nodev",		/* dump routine */
	"nodev",		/* psize routine */
	"0", 			/* flags */
	"nodev",		/* ioctl routine */
	"DEV_FUNNEL_NULL"	/* funnel */
	};

/* tokens which make up a default entry in the cdevsw table
 */
char	default_cdevsw[CMAX][32] = {
	"nulldev",		/* open routine */
	"nulldev",		/* close routine */
	"nodev",		/* read routine */
	"nodev",		/* write routine */
	"nodev",		/* ioctl routine */
	"nodev",		/* stop routine */
	"nulldev",		/* reset routine */
	"0",			/* ttys pointer */
	"nodev",		/* select routine */
	"nodev",		/* mmap routine */
	"DEV_FUNNEL_NULL",	/* funnel */
	"0",			/* segmap routine */
	"0"			/* if this is a DDI/DKI compliant driver */
	};


/* When adding entries to stanza_xdevsw structures must add equivalent
 * entry to stanza_xdevsw_defs structure
 */
/* labels for stanza file attribues having to do driver definition
 */
char	stanza_bdevsw [BMAX][32] = {
	"Device_Block_Open",
	"Device_Block_Close",
	"Device_Block_Strategy",
	"Device_Block_Dump",
	"Device_Block_Psize",
	"Device_Block_Flags",
	"Device_Block_Ioctl",
	"Device_Block_Funnel"
	};

/* Control whether routine definition needs to be done for routines.
 * No defintion generated for 0 entries (i.e. flags field).
 */
int	stanza_bdevsw_defs [BMAX] = {
	1,
	1,
	1,
	1,
	1,
	0,
	1,
	0
        };

char	stanza_cdevsw [CMAX][32] = {
	"Device_Char_Open",
	"Device_Char_Close",
	"Device_Char_Read",
	"Device_Char_Write",
	"Device_Char_Ioctl",
	"Device_Char_Stop",
	"Device_Char_Reset",
	"Device_Char_Ttys",
	"Device_Char_Select",
	"Device_Char_Mmap",
	"Device_Char_Funnel",
	"Device_Char_Segmap",
	"Device_Char_Flags"
	};

/* Control whether routine definition needs to be done for routines.
 * No defintion generated for 0 entries (i.e. flags field, etc.).
 */
int	stanza_cdevsw_defs [CMAX] = {
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	1,
	1,
	0,
	1,
	0
        };

char	bmajor_preference[]	= {"Device_Block_Major"};
char	cmajor_preference[]	= {"Device_Char_Major"};
char	major_require[]		= {"Device_Major_Req"};
char	module_config_name[]	= {"Module_Config_Name"};
char	subsystem_description[]	= {"Subsystem_Description"};
char	device_dir[]		= {"Device_Dir"};
char	device_dir_default[]	= {"/dev"};
char	device_subdir[]		= {"Device_Subdir"};
char	device_subdir_default[]	= {""};
char	block_minor[]		= {"Device_Block_Minor"};
char	block_files[]		= {"Device_Block_Files"};
char	char_minor[]		= {"Device_Char_Minor"};
char	char_files[]		= {"Device_Char_Files"};
char	device_b_subdir[]	= {"Device_Block_Subdir"};
char	device_c_subdir[]	= {"Device_Char_Subdir"};
char	device_user[]		= {"Device_User"};
int	device_user_default	= 0;
char	device_group[]		= {"Device_Group"};
int	device_group_default	= 0;
char	device_mode[]		= {"Device_Mode"};
int	device_mode_default	= 0664;

/* possible values for the Device_Block_Major and Device_Char_Major
 * attribues.  These may be numberic or have the following alpha values
 */
char	Any[] = {"Any"};
char	Same[] = {"Same"};

/* formats for printing out the bdevsw and cdevsw tables to the new conf.c
 * file
 */
char	dev_line1[] = {"\n"};
char	dev_line2[] = {"\t/* %s */\n"};
char	empty_comment[] = {"empty entry added by config"};

char	bdev_line3[] = {"\t{ %s,\t%s,\t%s,\t%s,\t/*%d*/\n"};
char	bdev_line4[] = {"\t  %s,\t%s,\t%s,\t%s },\n"};

char	cdev_line3[] = {"\t{ %s,\t%s,\t%s,\t%s,\t/*%d*/\n"};
char	cdev_line4[] = {"\t  %s,\t%s,\t%s,\t%s,\n"};

/* The DDI/DKI kernel hook adds two extra fields to the cdevsw structure.
 * See src/kernel/sys/conf.h for details.
*/
char	cdev_line5[] = {"\t  %s,\t%s,\t%s,\t%s,\t%s },\n"};
