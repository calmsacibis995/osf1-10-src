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
static char	*sccsid = "@(#)$RCSfile: finder.c,v $ $Revision: 4.3.20.7 $ (DEC) $Date: 1994/01/19 19:28:26 $";
#endif	/* lint */

/*
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from finder.c	5.1	(ULTRIX)	3/30/91
 */
/*
 * FINDER:
 *		This routine finds all the devices that can be used as
 *	installation devices,root devices, or all disks.  It searches for
 *	devices in the previously defined in the structures.  A system
 *	call, using mknod, is issued to create a device special file for
 *	each device.  The special file is then opened and an ioctl call
 *	is issued to the device.  Information returned is stored in the
 *	'found' structure and is printed after the device list has been
 *	exhausted.
 *
 *****
 *
 *  Modification History
 *
 *  000 - July,   1986	- Bob Fontaine created
 *
 *  001 - August, 1986  - Bob Fontaine
 *   		Added the message for boot command.
 *
 *  002 - August 14, 1986 -  Tungning Cherng
 *		Added the -f flag to reduce the redundant check in order
 *			to have a faster output.
 *
 *  003 - Mar 9, 1987 - Tungning Cherng
 *		Added the CVAX cpu support.
 *
 *  004 - July 9, 1987 - Tungning Cherng
 *		RD51,RD52 and RD31 not for system disk, only data disk.
 *
 *  005 - June,7,1988 - Tungning cherng
 * 		Supported the C_VAXSTAR(VAX420), and VAX8820.
 * 		Supported the RZ23 disk, RRDxx cdrom, and TZ30 tape.
 *
 *  006 - May 11, 1989 - Tim Burke
 *		Allow more than 32 "ra" type disks.  Change major
 *		number of "ra"
 *
 *  007 - May 31, 1989 - Jon Wallace
 *		New User interface that allows paging through disk
 *		selections of more than 16 disks.			
 *
 *  008 - Jun 27, 1989 - Jon Wallace
 *		Added ULTRIX name to user menu, and discontinued
 *		TA90 support per Tim Burke.
 *
 *  009 - Sep 13, 1989 - Tim Burke
 *              Added the supported_root routine.  This determines if
 *              the disk qualifies as a supported system disk.
 *
 *  010 - Nov 09, 1989 - Jon Wallace
 *		Added -w option that looks to see if the console
 *		device is graphic or not
 *
 *  011 - Jul 05, 1990 - Pete Keilty
 *		Added new DEV_BICI and DEV_XMICI defines.
 *		This informs which bus the CI is on and booting from.
 *
 *  012 - Aug 31, 1990 - Jon Wallace
 *		Added new rom support for DS5000 machines with code
 *		from Joe Szczypek
 *
 *  013 - Sep 07, 1990 - Ali Rafieymehr
 *		Added support for the VAX9000.
 *
 *  014 - Oct 09, 1990 - Joe Szczypek
 *		Corrected new ROM support.
 *
 *	015	08-oct-1991	ccb
 *		Change ULTRIX to Software in the menu banner
*/

/*	vi users: set tabstop=4
*/

#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <io/common/devio.h>
#include <sys/mtio.h>
#include <sys/param.h>
#include <machine/cpuconf.h>
#include <machine/devdriver.h>
#include <machine/entrypt.h>
#include <sys/sysinfo.h>
#include	<sys/stat.h>
#include	<sys/mode.h>
#include <machine/hal_sysinfo.h>
#include <ctype.h>
#include <stdio.h>
#include <nlist.h>
#include <signal.h>
#include <errno.h>
#include	<stdlib.h>

extern int errno;
extern char* sys_errlist[];

/* Macros...
*/
#define MAXDEVICES	300	/* maximum number of devs to find  */

#define DEF_A_PARTITION	98304	/* default "a" partition 48 Mb  */
#define DEF_B_PARTITION	131072	/* default "b" partition 64 Mb  */

#define MIN_CAPACITY	(DEF_A_PARTITION + DEF_B_PARTITION)

#define usage() 		fprintf(stderr,"usage: finder -[irdfw]\n")

#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif

#define EMPTY ""

const int	pageLen = 16;		/* pagelength */
const int	maxDisks = 128;		/* maximum number of non-ra disks  */
const int	maxRAdisks = 256;	/* maximum number of ra disks      */

typedef enum IdT { NOID, RZID, TZID, RAID } IdT;		/* drive types */
typedef enum BoolT { FALSE, TRUE } BoolT;

typedef enum SupportT
{
	END = -1, NONE, WARN, FULL

} SupportT;	/* support status */

typedef enum PgTypeT { FIRST, NTH } PgTypeT;


/*	these structures used DEV_SIZE from devio.h
*/

typedef struct tagDiskT
{
	IdT		id;					/* type identifier */
	char	type[DEV_SIZE+1];	/* Ultrix name of device */
	dev_t	majnum;				/* major number of device */

} DiskT;

/*	Support Status structure
*/

typedef struct tagSupStatT
{
	char		type[DEV_SIZE+1];		/* device type (TU77, etc) */
	SupportT	status;				/* status value */

} SupStatT;

typedef struct tagFoundT
{
	char type[DEV_SIZE+1];		/* device type (TU77,RA60...) */
	char name[DEV_SIZE+1];		/* Ultrix name */
	char interface[DEV_SIZE+1];	/* controller name (HSC70,UDA50) */
	int linktype;				/* bus type */
	int unit;			/* Unit number assigned by kernel */
	int plug;			/* physical plug number */
	int link;			/* number of controller is linked to */
	int trlevel;			/* TR level or BI node number */
	int adpt_num;			/* BI number or SBI number*/
	short bus_num;			/* xmi bus number */
	short nexus_num;		/* xmi slot number */
	short rctlr_num;		/* remote controller number,i.e. hsc #*/
	SupportT warn;			/* support state for system disk */

} FoundT;


/*	Finder file names:
 *		finder.tab - contains table entries which are presented in menu
 *		finder.dev - contains corresponding output values
 *	records from the two files match on a 1 for 1 basis.
 *
 *	Finder does not clean up these files when it is done. Later calls
 *	to finder using the -f flag will use them and skip the process of
 *	delving for device files
*/

static const char tabfile[] = "/tmp/finder.tab";
static const char fdevfile[] = "/tmp/finder.dev";

/*! all of these tables should be placed
 *! in data files that are read at
 *! run time.
*/

const DiskT roots [] =		/* disks that can be root devices */
{
	{ RAID,	"ra", 28 },
	{ RZID,	"rz", 8 },
	{ NOID, "", 0 }
};

const DiskT installs [] =	/* install devices */
{
	{ RZID,	"rz",	8 },
	/*	{ TZID,	"tz",	9 },	Not Used on AXP systems */
	{ NOID, "", 0 }
};

const DiskT disks [] =		/* any disk devices */
{
	{ RAID,	"ra",   28 },
	{ RZID,	"rz",   8 },
	{ NOID, "", 0 }
};

const SupStatT supRoots[] =		/* device support for ROOT file systems */
{
	/* Totally Unsupported
	*/
	{ "RX",		NONE },		/* Floppy - too small */
	{ "RRD",	NONE },		/* Read-only media */
	{ DEV_RD31,	NONE },		/* Disk - too small */
	{ DEV_RD32,	NONE },		/* Disk - too small */
	{ DEV_RD51,	NONE },		/* Disk - too small	*/
	{ DEV_RD52,	NONE }, 	/* Disk - too small	*/
	{ DEV_RZ22,	NONE }, 	/* Disk - too small	*/
	{ DEV_RZ23,	NONE }, 	/* Disk - too small	*/
	{ DEV_ESE20,	NONE }, 	/* Limited number of power hits */

	/* Conditionally Supported
	*/
	{ DEV_RZ24,	WARN },		/* too small for ADVANCED install */
	{ DEV_RZ55,	WARN },		/* too small for ADVANCED install */
	{ DEV_RA60,	WARN },		/* too small for ADVANCED install */
	{ DEV_RA80,	WARN },		/* too small for ADVANCED install */
	{ EMPTY, END }
};

const SupStatT supInstalls[] =	/*	device support as INSTALL devices */
{
	/* Totally Unsupported
	*/
	{ "RRD",	NONE },
	{ DEV_RA60,	NONE },
	{ DEV_RV20,	NONE },
	{ DEV_TA90,	NONE },
	{ EMPTY, END }
};

const SupStatT supDisks[] =		/*	device support as DATA devices */
{
	{ "RRD",	NONE },
	{ "RX",		NONE },
	{ EMPTY,	END }
};


/*
 *	Function Prototypes: KEEP THESE UP TO DATE. The &#*$*&
 *		compiler is lazy about enforcing them, but the benefits of
 *		keeping them up to date are remarkable.
 *
 *	they are declared here in order of use upon entry to main
*/


static BoolT	AnswerOk( const char* );
static void		FindDisks( const DiskT *, const SupStatT * );
static int		GetAnswer( const int, const PgTypeT );
static int		GetUserChoice();
static void		GetWsCons();
static SupportT	IsSupported( const int, const char *, const SupStatT *);
static BoolT	MkDir( const char *, const mode_t );
static void		MkFound(const int, const char*, FoundT*, const struct devget* );
static char		*ParentDir( char *, const char * );
static void		PrintDashes();
static void		PrintHeader( const char* );
static int		WriteTables();


/*	Global vars
*/

int				nfound = 0;		/* count of devices found */
int				fflag = 0;		/* set w/ -f */
int				rflag = 0;		/* set w/ -r */		       

				/*! this should be dynamically allocated */
FoundT			found[MAXDEVICES];	/* devices found */
char*			filesys;
char*			prog;			/* program name for error messages */


/*	main()
*/

main(argc,argv)
int argc;
char *argv[];
{
	char	*devdir;	/* where to make temp dev files */

	if (argc < 2)
	{
		usage();
		return(1);
	}

	prog = *argv;
	filesys = getenv( "ROUTINE" );
	if( (devdir = getenv( "FINDERDEVS" )) == NULL )
	{
		/* use default
		*/
		devdir = "/tmp/dev";
	}
	if( !MkDir( devdir, 0777 ) )
	{
		/* something went wrong
		*/
		(void) fprintf( stderr, "%s: can't mkdir %s (%s)\n", prog,
			devdir, sys_errlist[errno] );

		return(1);
	}
	if( chdir( devdir ) < 0 )
	{
		(void) fprintf( stderr, "%s: can't chdir to %s (%s)\n",
			prog, devdir, sys_errlist[errno] );

		return(1);
	}

	/*! args check should use getopt() */
	switch(argv[1][1])
	{
		case 'i':
			/*InstallDisks();*/
			FindDisks( installs, supInstalls );
			break;
		case 'r':
			/*RootDisks();*/
			rflag++;
			FindDisks( roots, supRoots );
			break;
		case 'd':
			/*DataDisks();*/
			FindDisks( disks, supDisks );
			break;
		case 'f':
			/* to reduce redundant check, show the previous form */
			fflag++;
			break;
		case 'w':
			/* find out if the machine has graphics capability */
			GetWsCons();
			exit(0);
		default:
		        usage();
			break;
	}

	if( fflag==0 )
	{
		if( nfound == 0 )
		{
			/* no devices found, none to write
			*/
			exit(-1);
		}
		if( WriteTables() < 0 )
			exit(-1);
	}
	return( GetUserChoice() );
}



/*	AnswerOk()()
 *	       check "ans" to see if it consists of all digits
 *
 *	given:	char* choice - the user's answer
 *	does:	verify that all charcters in it are digits
 *	return:	TRUE if all are digits, FALSE otherwise
*/

static BoolT AnswerOk( const char* choice )
{
	do
	{
		if( !isdigit( *choice ) )
			return( FALSE );
	}
	while( *++choice );
	return( TRUE );
}



/*	void FindDisks()
 *		find disks suitable to a particular use
 *
 *	given:	const DiskT *dpSup - array of DiskT's to use in finding
 *			disks.
 *		const char **vsUnsup - array of strings represinting
 *			unsupported disks.
 *		const char **vsWarn - array of strings representing
 *			disks for which a warning should be issued.
*/

static void FindDisks( const DiskT *dpSup, const SupStatT *supStats )
{
	const DiskT		*dp;						/* temp pointer */
	struct devget	devInfo;					/* devio info */
	SupportT		supStat;					/* status of paticular dev */
	char			devfile[DEV_SIZE+1]; 		/* dev file path name */
	dev_t			dev;						/* maj/min num */
	int				fd;							/* file descriptor */

	for( dp = dpSup; dp->id != NOID; ++dp )
	{
		register int	i;	/* inner loop index */
		int		nDisks;

		nDisks = (dp->id == RAID) ? maxRAdisks : maxDisks;

		for( i = 0; i < nDisks; ++i )
		{
			sprintf( devfile, "r%s%da", dp->type, i );

			switch( dp->id )
			{
			case RZID:
				dev = makedev( dp->majnum, MAKECAMMINOR(i,0));
				break;
			case RAID:
				dev = makedev( dp->majnum, MAKEMINOR(i,0) );
				break;
			default:
				strcpy( devfile, "rmt0h" );
				dev = makedev(dp->majnum, MAKECAMMINOR(i,MTHR));
			}

			mknod( devfile, 0020666, dev );
			if( (fd = open( devfile, O_RDONLY|O_NDELAY )) < 0 )
			{
				unlink( devfile );
				continue;
			}
			if( ioctl( fd, DEVIOCGET, (char *) &devInfo ) < 0 )
			{
				goto LABEL_next_i;
			}
			if( (supStat = IsSupported( fd, devInfo.device, supStats ))
																	!= NONE )
			{
				MkFound( i, dp->type, (&found[nfound]), &devInfo );

				found[nfound].warn = supStat;
				++nfound;
			}
LABEL_next_i:
			unlink( devfile );
			close( fd );
		}
	}
	return;
}



/*	GetAnswer() -
 *		get the user's selection
 *
 *	given:	const PgTypeT page - type of page in { FIRST, NTH }
 *	does:	print a prompt, take and validate user input, etc.
 *	return:	the number of the user's selection if all goes well,
 *		-1 if the selection was invalid
*/

static int GetAnswer( const int nlines, const PgTypeT page )
{
	static char	input[BUFSIZ];
	int		ans;

	fflush( stderr );
	for(;;)
	{
		gets( input );
		if( *input == '\0' )
		{
			if( page == NTH )
			{
				/* tell GetUserChoice to print the next page
				*/
				return( 0 );
			}
			else
				goto ERROR;	/* yes, a goto */
		}
		if( (AnswerOk( input ) == TRUE) &&
			(ans = atoi(input)) > 0 &&
			ans < nlines )
		{
			return( ans );
		}
		/* unusable input, try again
		*/
		switch( page )
		{
		case FIRST:
			fprintf( stderr, "\nSelect a number between 1 - %d : ",
				nlines - 1 );
			continue;
		case NTH:
ERROR:
			fprintf(stderr,
				"\nYour previous choice, '%s', is not a valid selection.",
				input );
			fprintf(stderr,
				"\nEnter another choice OR press RETURN for next screen: ");
			continue;
		}
	}
}



/*	getcpu()
 *		gets the CPU type
 *
 *	given:	nil
 *	does:	uses getsysinfo() to determine the CPU type
 *	return:	the CPU id
*/

static getcpu()
{
	int cpu;

	if( getsysinfo(GSI_CPU,&cpu,sizeof(cpu)) <= 0 )
	{
		printf("getsysinfo for cpu type failed!\n");
		exit(1);
	}
	return(cpu); 
}



/*	GetUserChoice()
 *		puts up the table for the user, outputs the choice
 *
 *	given:	nil
 *	does:	uses the contents of the table file to present a menu.
 *		gets user's selection from the menu
 *		uses user's choice to find a record in finder.dev and puts
 *			the record on standard out
 *		if isroot, runs showboot
 *	return:	-1 on failure, 0 otherwise
*/
     
static int GetUserChoice()
{
	FILE*		fp;
	int			ans;				/* user's answer */
	int			nlines;
	static char	line[BUFSIZ];

	if( (fp = fopen(tabfile,"r")) == NULL )
	{
		fprintf(stderr,"read %s failed (%s)\n",tabfile,
			sys_errlist[errno]);

		return(-1);
	}

	for( ;; )
	{
		/* print the choices */

		PrintHeader( filesys );
		for( ans = 0, nlines = 1;
			fgets( line, sizeof(line), fp ) != NULL;
			++nlines )
		{
			if( ((nlines % pageLen) == 0) && nlines != nfound )
			{
				PrintDashes();
				fprintf(stderr,
					"\nEnter your choice OR press RETURN for next screen: ");

				if( (ans = GetAnswer( nlines, NTH )) )
					break;

				/* GetAnswer returns 0 if the user want the next page
				*/
				PrintHeader( filesys );
			}
			fprintf(stderr,"%s",line);
		}

		if( ans != 0 )
			break;		/* we got one already */

		PrintDashes();
	    	fprintf(stderr, "\nEnter your choice: " );

		if( (ans = GetAnswer( nlines, FIRST )) )
			break;

		rewind(fp);
	}
	fclose(fp);

	if( ( fp = fopen( fdevfile, "r" )) == NULL )
	{
		fprintf( stderr, "open %s failed (%s)\n", fdevfile,
			sys_errlist[errno]);
		return(-1);
	}

	for( nlines = ans; nlines--; )
	{
		if( fgets(line, sizeof(line), fp ) == NULL )
		{
			fprintf( stderr, "unexpected eof on %s\n", fdevfile );
			return(-1);
		}
	}
	printf("%s",line);
	fclose(fp);	

	if (rflag)
		showboot( --ans );

	return(0);
}



/*	GetWsCons() -
 *		get workstation console information
*/

static void GetWsCons()
{
	int				fdCons;
	struct devget	devInfo;

	if( (fdCons = open("/dev/console",O_RDONLY | O_NDELAY)) < 0 )
	{
		printf("Can't open /dev/console\n");
		exit(1);
	}
	if(ioctl(fdCons,DEVIOCGET,(char *)&devInfo) < 0)
	{
		printf("devget ioctl failed!\n");
		exit(1);
	}
	close( fdCons );
	if (strcmp(devInfo.device, DEV_VR260 ) == 0 ||
	    strcmp(devInfo.device, DEV_VR290 ) == 0 ||
	    strcmp(devInfo.device, "MONO" ) == 0 ||
	    strcmp(devInfo.device, "COLOR" ) == 0)
	{
		printf("0");
	}
}



/*	IsSupported() -
 *		get device support status
 *
 *	given:	const int fd - device file descriptor
 *			const char * - pszDevName - device name string
 *			const SupStatT *supStats - array of support status structs
 *
 *	does:	search array of SupStatTs to determine if pszDevName is in
 *			it. If so return contents of the status field of the struct.
 *			Else check to verify that the disk is at least MIN_CAPACITY
 *			kbytes long
 *
 *	return:	the status field of the SupStatT that matches pszDevName if
 *			one is found.
 *			Otherwise return FULL if DEVGETGEOM can't determine disk
 *			geometry or the disk is larger than MIN_CAPACITY.
 *			Otherwise return NONE as a support status.
*/

static SupportT IsSupported( const int fd,
								const char *pszDevName,
								const SupStatT *supStats )
{
	const SupStatT	*pStats;
	DEVGEOMST		devgeom;

	for( pStats = supStats; pStats->status != END; ++pStats )
	{
		if( strncmp( pStats->type, pszDevName, strlen( pStats->type )) == 0)
		{
			return( pStats->status );
		}
	}
	/* unless we're looking for a ROOT device, assume that
	 *	absence from the table indicates full support
	*/
	if( !rflag )
	{
		return( FULL );
	}

	if( ioctl( fd, DEVGETGEOM, (char *)&devgeom) < 0)
	{
		/*
		 * In the event of ioctl failure do not conclude that the
		 * disk is too small.
		 */
		return( FULL );
	}
	if( devgeom.geom_info.dev_size < MIN_CAPACITY )
	{
		return( NONE );
	}
	return( FULL );
}



/*	static BoolT MkDir() -
 *		create a path
 *
 *	given:	const char *pszPath - path to make
 *			const mode_t nMode - mode to use
 *	does:	make the path
 *	return:	TRUE if successful
 *			FALSE on failure
*/

static BoolT MkDir( const char *pszPath, const mode_t nMode )
{
	static struct stat	stData;
	char				*pszTempPath;

	if( *pszPath == '\0' )
	{
		/* assume that '/' exists
		*/
		return TRUE;
	}
	if( !stat( pszPath, &stData ) )
	{
		/* this one already exists
		 *  is it a directory?
		*/
		if( (stData.st_mode & S_IFDIR) == S_IFDIR )
		{
			/* it's cool */
			return TRUE;
		}
		else
		{
			errno = EEXIST;
			return FALSE;
		}
	}
	if( (pszTempPath = (char *) malloc( strlen(pszPath) + 1 )) == NULL )
	{
		return FALSE;
	}
	/* be careful from here out to deallocate that memory
	*/
	if( !MkDir( ParentDir( pszTempPath, pszPath ), nMode ) )
	{
		free( pszTempPath );
		return FALSE;
	}
	free( pszTempPath );

	if( mkdir( pszPath, nMode ) )
	{
		/* can't make it */
		return FALSE;
	}
	return TRUE;
}



/*	MkFound()
 *		fill in an f structure with dev info
 *
 *	given:	const int unit - unit # of device
 *		const char* n - device name
 *		FoundT found - Found structure
 *		struct devget d - devinfo structure
 *	does:	fill in f with type and devinfo data
 *	return:	void
*/
	
static void MkFound( const int unit, const char* n,
	FoundT *found, const struct devget *d )
{
	/* structure initialization proceedes in the order in which
	 *  the fields are found in the structure declaration.
	 * If you have added new fields to the structure, place the
	 *  initialization in the proper order below.
	*/

	strcpy( found->type, d->device );
	strcpy( found->name, n );
	strcpy(found->interface, d->interface);
	found->linktype = d->bus;
  	found->unit = unit;
	found->plug = d->slave_num;

	/* The SCSI physical device number is created by
	 * the bus+target+lun of the device.  Here we are
	 * just picking out the device target id to display
	 * for bootpath info. 
	*/
	if (found->linktype == DEV_SCSI)
	    found->plug = GETCAMTARG(found->plug);

  	found->link = d->ctlr_num;
	found->trlevel = d->nexus_num;
	found->adpt_num = d->adpt_num;
 	found->bus_num = d->bus_num;
	found->nexus_num = d->nexus_num;
	found->rctlr_num = d->rctlr_num;
	found->warn = FULL;
}


/*	static char *ParentDir()
 *
 *	given:	char *rent - pointer to buffer for parent path
 *		const char *child - pointer to buffer for child path
 *	does:	copy all but the last path segment from the child
 *		path buffer to the parent path buffer
 *	return:	a pointer to the parent path buffer
*/

static char *ParentDir( char *rent, const char *child )
{
	register int	i;

	strcpy( rent, child );

	/* walk the buffer backwards to the first '/' and
	 *  replace it with a '\0'
	*/
	for( i = strlen(rent); i >= 0; --i )
	{
		if( rent[i] == '/' )
		{
			rent[i] = '\0';
			break;
		}
	}
	return rent;
}



/*	PrintDashes()
 *		prints a string of dashes
 *
 *	given:	nil
 *	does:	print dashes
 *	return:	void
*/

static void PrintDashes()
{
	static const char* dashes = "-----------------------------------"
								"-----------------------------------";

	fprintf(stderr, "%s\n", dashes );
}



/*	void PrintHeader()
 *		prints the table header
 *
 *	given:	const char *s - file system name from install.*
 *	does:	print the header
 *	return:	void
*/

static void PrintHeader( const char *s )
{
	static const char* header1 =
		"Selection   Device    Software    "
		"Device       Controller   Controller ";

	static const char* header2 =
		"            Name       Name       "
		"Number       Name         Number     ";

	fprintf(stderr, "\n%s TABLE \n\n", s );
        fprintf(stderr, "%s\n", header1);
	fprintf(stderr, "%s\n", header2);
	PrintDashes();
}



/*	WriteTables()
 *		write the finder.dev and finder.tab files
 *
 *	given:	nil
 *	does:	use the information in the FoundT found[] to  produce
 *		the finder.dev and finder.tab tables.
 *	return:	-1 if there is an error
*/

static int WriteTables()
{
	FILE	*fp1, *fp2;
	int	i;

	if( (fp1=fopen(tabfile,"w")) == NULL )
	{
		fprintf(stderr,"open %s failed (%s)\n", tabfile, sys_errlist[errno]);
		return(-1);
	}

	if( (fp2=fopen(fdevfile,"w")) == NULL )
	{
		fprintf(stderr,"open %s failed (%s)\n", fdevfile);
		return(-1);
	}
	for( i = 0; i < nfound; i++ )
	{
		fprintf( fp1,"   %2d       %-10s ",i+1,found[i].type);
		fprintf( fp1, "%3s%-3d", found[i].name, found[i].unit);
		fprintf( fp1, "%c    ", (found[i].warn == WARN) ? '*' : ' ');
		fprintf( fp1," %3d         ",found[i].plug);
		fprintf( fp1,"%-10s   %3d\n",found[i].interface,found[i].link);
		fprintf( fp2,"%s %s %d %s\n", found[i].type, found[i].name,
			found[i].unit, (found[i].warn == WARN) ? "WARN" : "" );
	}
	fclose(fp1);
	fclose(fp2);
	return(0);
}



/*	Controller Handling Routines, used by showboot()
 *
 *	none of these were touched in the July '93 cleanup except
 *	to remove #ifdef VAX and #ifdef MIPS code. If, by some remote
 *	twitch of fate, that code is needed again, it would be best
 *	to port it from the ULTRIX version of finder.
*/

int SB_CTLR_NUM = 0;	/* paw */
int config_bus();
int config_ctlr();
int config_dev();

int xmi_ctlr_num(i)
int i;
{
        /* find controller number */
        return(config_bus(-1, i));
}

int config_bus(busaddr, d_unit)
        caddr_t busaddr;
        int d_unit;
{
        struct bus bus;
        char bus_name[20];
	int ret_val;

        do {
                if(getsysinfo(GSI_BUS_STRUCT, &bus, sizeof(struct bus),
                           busaddr, 0) == -1)
                                break;
                if((bus.alive & ALV_ALIVE) &&
                   ((bus.alive & ALV_NOSIZER) == 0)) {
                        bzero(bus_name, sizeof(bus_name));
                        getsysinfo(GSI_BUS_NAME, bus_name, sizeof(bus_name),
                                   busaddr, 0);
                        bus.bus_name = bus_name;

                        if(bus.bus_list)
                                if((ret_val = config_bus(bus.bus_list, d_unit)) >= 0)
				        return(ret_val);
		       if((bus.bus_type == BUS_XMI) || (bus.bus_type == BUS_CI))
				if(bus.ctlr_list)
				       if((ret_val = config_ctlr(&bus, d_unit)) >= 0)
					      return(ret_val);
                }
        } while(busaddr = (caddr_t)bus.nxt_bus);
	return(-1);
}

int config_ctlr(bus, d_unit)
        struct bus *bus;
        int d_unit;
{
        caddr_t ctlraddr;
        char ctlr_name[20];
        struct controller ctlr;
	int ret_val;

        ctlraddr = (caddr_t)bus->ctlr_list;
        do {
                if(getsysinfo(GSI_CTLR_STRUCT, &ctlr,
                        sizeof(struct controller), ctlraddr, 0) == -1)
                                break;
                /*
                 * Ignore the ones with ALV_NOSIZER set because they
                 * refer to loadable drivers (which aren't specified
                 * in the config file).
                 */
                if((ctlr.alive & ALV_ALIVE) &&
                   ((ctlr.alive & ALV_NOSIZER) == 0)) {
                        bzero(ctlr_name, sizeof(ctlr_name));
                        getsysinfo(GSI_CTLR_NAME, ctlr_name, sizeof(ctlr_name),
                                   ctlraddr, 0);
			ctlr.ctlr_name = ctlr_name;
		        if(!strcmp(ctlr_name, "uq") || !strcmp(ctlr_name, "hsc") ) {
#ifdef DEBUG
			        /* paw debug */
			        printf("ctlr_name: %s, ctlr_num: %d, bus_name: %s, bus_num: %d, slot_num: %d, rctlr_num: %d\n",
			            ctlr_name, ctlr.ctlr_num, bus->bus_name, bus->bus_num, ctlr.slot, ctlr.rctlr);
#endif /*DEBUG*/
                                if(ctlr.dev_list) {
				   if((ret_val = config_dev(&ctlr, d_unit)) != -1)
				       return(SB_CTLR_NUM);
				   else {
				      if(!strcmp(ctlr_name, "hsc")) {
				         if (!(caddr_t)ctlr.nxt_ctlr) {
#ifdef DEBUG
					 printf("config_ctlr: incrementing SB_CTLR_NUM to %d\n",
						SB_CTLR_NUM + 1);
#endif
					    SB_CTLR_NUM++;
				         }
				      }
				      else {
#ifdef DEBUG
					 printf("config_ctlr: incrementing SB_CTLR_NUM to %d\n",
						SB_CTLR_NUM + 1);
#endif
					 SB_CTLR_NUM++;
				      }
				   }
				} 
				else
				  if ((strcmp(ctlr_name, "hsc")) || (!strcmp(ctlr_name, "hsc") 
								     && ((caddr_t)ctlr.nxt_ctlr == NULL)))
					SB_CTLR_NUM++;

			}
                }
        } while(ctlraddr = (caddr_t)ctlr.nxt_ctlr);
	return(-1);
}

int config_dev(ctlr, d_unit)
        struct controller *ctlr;
        int d_unit;
{
        caddr_t devaddr;
        struct device dev;

        devaddr = (caddr_t)ctlr->dev_list;
        do {
                if(getsysinfo(GSI_DEV_STRUCT, &dev, sizeof(dev), devaddr, 0) == -1)
                                break;
                /*
                 * Ignore the ones with ALV_NOSIZER set because they
                 * refer to loadable drivers (which aren't specified
                 * in the config file).
                 */
                if((dev.alive & ALV_ALIVE) &&
                   ((dev.alive & ALV_NOSIZER) == 0)) {
		        if(dev.logunit == d_unit) {
#ifdef DEBUG
			       printf("found unit %d on controller %d\n", d_unit, SB_CTLR_NUM);
#endif
		               return(1);
			}
                }
        } while(devaddr = (caddr_t)dev.nxt_dev);
	return(-1);
}


/*	showboot()
 *		write the showboot file
 *
 *	given:	const int i - index into the FOUND table
 *	does:	write out a boot command for the specified device
 *	return:	???
*/

showboot( const int i)
{
	int cpu;
	char console_magic[4];
	FILE *fp;


#ifdef __alpha
	int	wflag = 1, disk_num, adpt_num, scsi_channel, err = 0;
	char	DISKTYPE[5];
	char	BOOT_ENV[] = "bootdef_dev";
	char	BOOTPATH[80];
	char	BOOTCOMM[] = "boot";
	char    BOOTFLAG[] = "A";
	char    BOOTFLAG_ENV[] = "boot_osflags";
	char    BOOTDEVICE[256];
	char    BOOTDEV_ENV[] = "boot_dev";
	char	TCSLOT[5] = "";

	fp=fopen("/tmp/showboot","w");
	cpu = getcpu();
	BOOTPATH[0] = NULL;
	switch(cpu)
        {
	case DEC_7000:   /* ruby */
		switch(found[i].linktype)
		{
		case DEV_SCSI:
#ifdef DEBUG
		        printf("\nshowboot: scsi disk\n");
#endif
			strcpy(DISKTYPE, "DK");
			disk_num = found[i].plug;
			adpt_num = found[i].link;
			break;

		case DEV_XMICI:
#ifdef DEBUG
		        printf("\nshowboot: dsa disk on an hsc\n");
#endif
			strcpy(DISKTYPE, "DU");
			disk_num = found[i].unit;
			adpt_num = xmi_ctlr_num(disk_num);  
			break;

		default: /* ra disks on a kdm70 */
#ifdef DEBUG
		        printf("\nshowboot: dsa disk on a kdm70\n");
#endif
			strcpy(DISKTYPE, "DU");
			disk_num = found[i].unit;
			adpt_num = xmi_ctlr_num(disk_num);
			break;
		}

		if (adpt_num < 0) {
		  fprintf(fp, "\n**** Unable to locate disk controller ****\n");
		  wflag = 0;
		}
		else {
		   if (found[i].linktype == DEV_XMICI) {
			sprintf(BOOTPATH, "%s%c%d.%d",
                                DISKTYPE, 'A' + adpt_num, disk_num,
                                found[i].rctlr_num);
		   }
		   else {
			sprintf(BOOTPATH, "%s%c%d",
				DISKTYPE, 'A' + adpt_num, disk_num);
			if (disk_num != 0 && found[i].linktype == DEV_SCSI) 
				strcat(BOOTPATH, "00"); /* paw: qar 5692 */
		        }
		}

		break;

	case DEC_4000:  /* cobra */
		strcpy(DISKTYPE, "DK");
	  	sprintf(BOOTPATH, "%s%c%d",DISKTYPE,(char) 'A' + 
			found[i].link,found[i].plug);
		if (found[i].plug != 0) strcat(BOOTPATH, "00");
		break;

	case DEC_3000_500: /* flamingo */
		strcpy(DISKTYPE, "DK");
		if (found[i].link > 1) {
			sprintf(TCSLOT, "%d/", found[i].adpt_num);
		}
		scsi_channel = found[i].link % 2;
		sprintf(BOOTDEVICE, "%s %d %d %d %d %d %d %s", "SCSI", 0, found[i].adpt_num,
			found[i].link, 0, (found[i].plug == 0) ? 0 : found[i].plug * 100, 0 , "FLAMG-IO"); 
	  	sprintf(BOOTPATH, "%s%s%c%d",TCSLOT,DISKTYPE,(char) 'A' + scsi_channel,found[i].plug);
		if (found[i].plug != 0) strcat(BOOTPATH, "00");

		err = setsysinfo(SSI_PROM_ENV, BOOTPATH, strlen(BOOTPATH), BOOT_ENV);
		err += setsysinfo(SSI_PROM_ENV, BOOTFLAG, strlen(BOOTFLAG), BOOTFLAG_ENV);
		err += setsysinfo(SSI_PROM_ENV, BOOTDEVICE, strlen(BOOTDEVICE), BOOTDEV_ENV);

		/* to prevent the /tmp/showboot file from being written to (for autoboot),  */
		/* uncomment the following line */
/* 		wflag = err; */ 

		break;

        case DEC_3000_300: /* pelican  */
		strcpy(DISKTYPE, "DK");
		/* pelican only has one scsi channel on the base */
		if (found[i].link > 0) {
			sprintf(TCSLOT, "%d/", found[i].adpt_num);
			scsi_channel = (found[i].link + 1) % 2;
		} else {
		        /* Pelican base scsi is always dka### */
		        scsi_channel = 0;
		}
		sprintf(BOOTDEVICE, "%s %d %d %d %d %d %d %s", "SCSI", 0, found[i].adpt_num,
			found[i].link, 0, (found[i].plug == 0) ? 0 : found[i].plug * 100, 0 , "FLAMG-IO"); 
	  	sprintf(BOOTPATH, "%s%s%c%d",TCSLOT,DISKTYPE,(char) 'A' + scsi_channel,found[i].plug);
		if (found[i].plug != 0) strcat(BOOTPATH, "00");

		err = setsysinfo(SSI_PROM_ENV, BOOTPATH, strlen(BOOTPATH), BOOT_ENV);
		err += setsysinfo(SSI_PROM_ENV, BOOTFLAG, strlen(BOOTFLAG), BOOTFLAG_ENV);
		err += setsysinfo(SSI_PROM_ENV, BOOTDEVICE, strlen(BOOTDEVICE), BOOTDEV_ENV);

		/* to prevent the /tmp/showboot file from being written to (for autoboot),  */
		/* uncomment the following line */
/* 		wflag = err; */ 

		break;

        case DEC_2000_300: /* Jensen  */

		/*
		 * For jensen the scsi_channel represents order of the scsi
		 * controllers. A is the first and A=0.	
		 * 
		 */

		strcpy(DISKTYPE, "DK");
		scsi_channel = found[i].link;
		sprintf(BOOTDEVICE, "%s %d %d %d %d %d %d %s", "SCSI", 0, found[i].adpt_num,
			found[i].link, 0, (found[i].plug == 0) ? 0 : found[i].plug * 100, 0 , "JENS-IO"); 
	  	sprintf(BOOTPATH, "%s%c%d",DISKTYPE,(char) 'A' + scsi_channel,found[i].plug);
		if (found[i].plug != 0) strcat(BOOTPATH, "00");

		err = setsysinfo(SSI_PROM_ENV, BOOTPATH, strlen(BOOTPATH), BOOT_ENV);
		err += setsysinfo(SSI_PROM_ENV, BOOTFLAG, strlen(BOOTFLAG), BOOTFLAG_ENV);
		err += setsysinfo(SSI_PROM_ENV, BOOTDEVICE, strlen(BOOTDEVICE), BOOTDEV_ENV);

		/* to prevent the /tmp/showboot file from being written to (for autoboot),  */
		/* uncomment the following line */
		/* wflag = err; */ 

		break;

	default:
		wflag = 0;
		fprintf(fp,"\n**** Unsupported processor ****\n");
		break;
	}

	if (wflag)
	{
	fprintf(fp,"\nIssue the following console commands to set your default bootpath variable\n");
	fprintf(fp,"and to boot your system disk to multiuser:\n\n");

	fprintf(fp,"\t>>> set %s %s\n", BOOTFLAG_ENV, BOOTFLAG);

	fprintf(fp,"\t>>> set %s \"%s\"\n", BOOT_ENV,BOOTPATH);
	fprintf(fp,"\t>>> %s\n\n", BOOTCOMM);
	}

	fclose(fp);
	return;
#endif /* __alpha */
}


