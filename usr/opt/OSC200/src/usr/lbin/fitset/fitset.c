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
static  char *sccsid = "@(#)$RCSfile: fitset.c,v $ $Revision: 4.3.9.3 $ (DEC) $Date: 1993/09/21 20:51:15 $";
#endif
/* 
 *	fitset.c
 *		check subset sizes.
 *
 *	History:
 *
 *
 *	000	5-may-1987	ccb
 *		uses getmnt() to find filesystems and devices,
 *		reads inventory records from stdin.
 *
 *	001	28-aug-1987	ccb
 *		moved fbuf array into static space, was
 *		crashing stack when located in main().
 *
 *	002	7-JAN-1988	ccb
 *		increase margin of safety before filling disk,
 *		set different margins between system & layered products.
 *
 *	003	6-APR-1988	ccb
 *		Fix margin setting. Before this it had been allowing
 *		setld to asymptoticaly fill the disk. See 003 comment
 *		block below for implementation details.
 *
 *	004	14-APR-1988	ccb
 *		Was baling out when encountering any full filesystem
 *		regardless of any attempt to hang files there.
 *		Margins reduced to allow UWS to install to RD{5,3}3
 *
 *	005	24-jul-1989	ccb
 *		Include <sys/dir.h> as required to include setld.h
 *		Check all file systems used by the subset for overflow
 *			rather than just the last one encountered
 *		Minor lint fixes
 *
 *	006	21-jun-1991	ccb
 *		suspend the margin.
 *
 *	007	12-aug-1991	ech
 *		restore the margin to .10.
 *		added FsWritable and Tn* routines to handle:
 *		1)  symbolic links. 
 *		2)  nfs mounted file systems.
 *		3)  double booking.
 *
 *	008	23-Oct-1991	ech
 *		dynamically allocate tn_name and tn_path to reduce memory space
 *		required to store the database.
 *		create pathlist to store path(s) processed for a specific
 *		inventory record to avoid infinite loop.
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/mount.h>
#include	<sys/dir.h>
#include	<sys/errno.h>
#include	<sys/mode.h>
#include	<sys/stat.h>
#include	<ufs/dinode.h> 
#include	<rpc/rpc.h>
#include	<nfs/nfs_clnt.h>
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<math.h>
#include	<setld/setld.h>


extern char	*getenv();		/* environ(7) */
extern int	errno;			/* errno(2) */
extern char	*sys_errlist[];		/* errno(2) */
extern int	optind;			/* getopt(3) */

#define FITLOG	"./var/adm/smlogs/fitset.log"	/* logfile */

/* file system table data structure for tracking available system space */
#define	FS_INUSE	0x0001
#define	FSUSED(a)	((a->fs_inuse&FS_INUSE)==FS_INUSE)
		
#define	CURFS	fbuf[nfsys]

#define ALL_FS		1	/* flag to pass to FsShow */ 
#define AFFECTED_FS	0

#define	CHECK_LINK	1	/* to activate TnPathChange() in TnAdd() */
#define	IGNORE_LINK	0

#define DO_LSTAT	1	/* lstat in TnPathChange() is required */
#define	NO_LSTAT	0 
#define	IN_ISL		(advflag!=NULL)	/* in initial system load */ 

/* exit status */
#define	SUCCESS		0	/* enough fs space */
#define	ERROR		1	/* miscellaneous error conditions */
#define	OVER		2 	/* fs is out of free frags */
#define	WARNING		3	/* fs has free space yet is over 100% full */
 
typedef struct fstable {
	struct fstable	*fs_next;
	FlagsT		fs_inuse;	/* = 1 if file system is involved */
	PathT 		fs_path;	/* absolute path of file system */
	int		fs_pathlen;	/* length of path */
	int		fs_blocks;	/* total data blocks */
	int		fs_files;	/* total file nodes in file system */
	int		fs_bavail;	/* free blocks avail to non-su */
	int		fs_minfree;	/* min percentage of free blocks*/
	int		fs_cap;		/* capacity, % full */
	long		fs_bsize;	/* fs block size */
	long		fs_fsize;	/* fs fragment size */
	long		fs_bfree;	/* free fragments in fs */
	long		fs_ffree;	/* free file nodes in fs */	
	short		fs_type;	/* type of fs, e.g. UFS/NFS/.. */
	short		fs_bfratio;	/* block size / frag size */
	long		fs_fthsd;	/* file space allocation difference
					   threshhold */
	short		fs_auto;	/* = 1 if a automount NFS file system */
} FsT;

static FsT	*fst = NULL; 	/* head of the file system list */

static short	debug = 0;	/* debug flags */ 

static struct statfs	fbuf[NMOUNT];	/* storage for mount points */
static float		margin = .10;	/* margin of ISL fullness */
static struct stat	stb;
  
char		*prog;
char		*advflag = NULL;
short		lstatflag = DO_LSTAT; 							
/* for logging purpose */
char	buf[BUFSIZ];
int	pid;
FILE	*logfp;

/* a database tree containing information about all inventory records will
   be built first and then traversed to track file system space */

/* information stored in a database tree node */  
/* tn_path, tn_linkpath, tn_stsize are created to optimize processing time */
typedef struct node {
	struct node 	*tn_up;		/* pointer to parent node */
	struct node	*tn_next;	/* pointer to sibling node */
	struct node	*tn_sub;	/* pointer to list of children nodes */
	struct node	*tn_link;	/* is a symbolic link to */
	char		*tn_name; 	/* filename */
	char		*tn_path;	/* absolute full path */
	off_t 		tn_size;	/* size of file in bytes */
	off_t		tn_stsize;	/* size obtained from lstat */
	FtypeT		tn_type; 	/* type of file, one of [lsdf] */
	short		tn_inv;		/* = 1 if fullpath of the node is an 
					   inventory file entry */
	short		tn_stat;	/* = 1 if was able to lstat */ 
} TnT;

static TnT	*treetop = NULL;	/* head of the database tree */
static PathT	rootpath;		/* root path */
static int	rootlen;		/* length of rootpath */

typedef struct pathnode {
	struct pathnode	*pn_next;	/* pointer to next path node */
	char		*pn_path;	/* full path of the file */
} TnPathT;

/* for each inventory record, if there's any path change due to symbolic
   link, the new path is stored in a list and checked against every
   time there's another path replacement to avoid infinite loop */

static TnPathT	*pathlist = NULL; 	/* list of paths for an inv file */
static TnPathT	*listtail = NULL;	/* points to last path on the list */

static void	Exit();		/* end logging and exit with status */
static short	FsCheck();	/* check final statistics of affected fs */
static FsT	*FsFind();	/* FsT list search routine */
static long	FsFragAlloc();	/* returns number of fs fragments needed */   
static void	FsGetInfo();	/* use statfs/open to get fs size/access info */
static void	FsInit();	/* initialize with data from getmntinfo */
static FsT	*FsInsert();	/* FsT list insert routine */
static FsT	*FsNew();	/* allocate storage */
static void	Log();		/* write to log file */
static void	MenuUsage();	/* prints usage message */
static void	OpenLog();	/* open log file */
static void	SetRootPath();  /* check validity of rootpath and set it */ 
static TnT	*TnAdd();	/* add node(s) along a path in the tree */
static void	TnAddSymLinks();/* add all symbolic links into the tree */
static TnT	*TnAddUnder();	/* add a node under specified parent node */
static TnT	*TnFind();	/* find a node under specified node */
static TnT	*TnFindNext();	/* find a node in sibllings */
static int	TnGetName();	/* get first filename in a path */ 
static void	TnGetVarInfo(); /* see how /var and /usr/var are set up */ 
static TnT	*TnInsert();	/* insert a node among its siblings */
static TnT	*TnNew();	/* allocate space for a new node */
static void	TnPListFree();  /* free space used by pathlist */
static TnPathT	*TnPNodeNew();	/* allocate space for a new path node */
static int	TnPathChange();	/* change path if necessary */ 
static TnT	*TnPathCheck(); /* check if path already walked */
static void	TnResolvePath();/* resolve link path to absolute full path */
static void	TnTraverse();	/* traverse the database tree */
static void	TnUpdFsData();	/* update file system data per node basis */

/* 001 */

main(argc,argv)
int argc;
char *argv[];
{
	FsT		*np;		/* list and node pointers */
	PathT		path;		/* complete absolute node path */
	int		c;		/* for use w/getopt(3) */
	int		nfsys;		/* # of mounts */
	InvT		*ip;		/* Inventory Pointer */
	InvRecT		*irp;		/* inventory record pointer */
	TnT		*newnode;	/* node in the database tree */
	struct statfs	*mntbuf, *m;	/* to be used with getmntinfo */
	short		status;		/* return status */

	prog = *argv;

	/* get root path from command line */
	while( (c = getopt( argc, argv, "ds" )) != EOF )
	{
		switch( c )
		{
		case 'd':
			/* complete debug messages */ 
			debug++;
			break;
		default:
			MenuUsage ();
		}
	}
	
	OpenLog ();	/* start logging */

	SetRootPath (argv[optind]); /* set rootpath if valid */

	/* check if we are during initial system load (ISL) */
	advflag = getenv ("ADVFLAG");
	if (IN_ISL)
		/* do not call lstat during ISL, with one exception in
		   TnGetVarInfo() to lstat var and usr/var */ 
		lstatflag = NO_LSTAT; 

	/* create top node of the database tree which will contain
	   all files in the .inv(s) */
	treetop = TnNew ("/");
	treetop->tn_path = (char *) malloc ((size_t) (strlen ("/") + 1));  
	if (treetop->tn_path == NULL) 
	{
		(void) sprintf (buf, "%s: internal error, can not malloc. \n", prog);
		Log (buf);
		(void) fputs (buf, stderr);
		Exit (ERROR);
	}
	(void) strcpy (treetop->tn_path, "/");
	/* parent directory of / (i.e., .. of /) is / itself */
	treetop->tn_up = treetop;

	/* get information about allocation of var and usr/var */
 	if (IN_ISL) 
		TnGetVarInfo (); 

	ip = InvInit( stdin );

	/* process all symbolic links in input first */
	TnAddSymLinks (ip); 

	/* run down the inventory on stdin */
	while( (irp = InvRead( ip )) != NULL )
	{
		if (debug)
			(void) printf("\ns=%d\tt=%c\tp=%s\n", irp->i_size, irp->i_type, irp->i_path );

		(void) sprintf (path, "%s%s", rootpath, irp->i_path + 1); 
		if (debug)
		{
			(void) printf("Relative File:\n\t%s\n", path) ;
		}

		newnode = TnAdd (path, treetop, CHECK_LINK);

		/* If 'path' is a symbolic link, newnode returned is 
		   the referent or the real file, therefore, we shouldn't
		   set the following fields according to those of the
		   original 'path'  */ 
		if ((newnode != NULL) && (irp->i_type != 's')) 
		{
			newnode->tn_inv = 1;
			newnode->tn_type = irp->i_type;
			newnode->tn_size = irp->i_size;
		}

		TnPListFree ();

	}
	
	if( getenv( "ADVFLAG" ) == NULL ) /* installing layered software */
		margin = .0;

	(void) sprintf (buf, "margin = %f\n", margin);
	Log (buf); 
	if (debug)
		(void) puts (buf);
 
	/* gather file system information */
	/* we want to delay doing statfs and check if fs is writable 
	   till during the traversal so that we will only need to 
	   get info of involved file systems. This solves the
	   automount problem, i.e., statfs and open will cause the automount 
	   file system to get mounted. With lengthy automount entries, this may 	   exhaust system resources. Furthermore, in most cases, automount 
 	   mount points are not even involved in the inventories. */ 

	if ((nfsys = getmntinfo (&mntbuf, MNT_NOWAIT)) == 0)
	{
		(void) sprintf (buf,"%s: cannot get mount information (%s).\n",
			prog, sys_errlist[errno]);
		Log (buf);
		(void) fputs (buf, stderr);
		Exit(ERROR);
	}

	(void) sprintf (buf, "# of fs = %d\n", nfsys);
	Log (buf);
	if (debug)
		(void) puts (buf);

	/*  cull out all of the filesystems below rootpath and
	 *  queue them up into the 'fstable' list.
	*/
	while( --nfsys >= 0 )
	{
		m = mntbuf++;

		/* we take care of UFS, NFS, and MSFS (megasafe fs) only */
		if(( m->f_type == MOUNT_UFS ) ||  
		   ( m->f_type == MOUNT_NFS ) || 
		   ( m->f_type == MOUNT_MSFS ))  
		{
			/* build an fstable entry */
			np = FsNew();
			FsInit( np, m);
			fst = FsInsert( np, fst );
		}
	}

	/* traverse the database to record file system usage */
	/* mark and get file system statistics along the way */
	TnTraverse(treetop); 

	/* check on space/inode situation of each affected file systems */
	status = FsCheck ();

	Exit(status);
}

/*      Exit() -
 *              log exit information and exit with specified status.
 *	given:	exit status
 *	does:	end logging and exit with given status.
 *	return:	none.
*/

static	void	Exit (status)
short	status;
{
        long    t;

	if (!status)
	{
		(void) sprintf (buf, "\nfitset completed successfully.\n");
		Log (buf);
		if (debug)
			 (void) puts (buf);
	}

        (void) sprintf( buf, "\n%s%d end logging at %s\n--------------------\n", prog, pid, asctime( localtime( ((void) time(&t),&t) ) ) );

        Log( buf );

        exit (status);
}

/*	static	short	FsCheck() -
 *		checks final statistics of affected file systems.
 *		
 *	given:	pointer to the file system table previously set up (fst).
 *	does:	checks if file system is in use. If yes, check on inode
 *		and space usage.
 *	return:	return with specific value depending on the result of the
 *		check.
 *
*/
static	short	FsCheck ()

{
	FsT		*np;
	off_t		used;
	int		availblks;
	short 		overflow = 0;	/* file system runs out of free frags */
	short		warning = 0;	/* file system will be over 100% full */
	long		bfree, ffree;


	for( np = fst; np != NULL; np = np->fs_next )
	{
		if FSUSED(np)  
		{
			/* calculate capacity */
			used = (off_t)np->fs_blocks - (off_t)np->fs_bfree;
			np->fs_bavail = np->fs_bfree - (np->fs_blocks
						* np->fs_minfree / 100);
			availblks = np->fs_bavail + used;
			np->fs_cap = (int) rint ((double)used /
						(double)availblks * 100.0);

			/* deduct margin before checking, margin is 
			   non-zero for ISL */
			/* do not update np record here because we want
			   FsShow() to reflect the real statistics in the
			   log file later */
			ffree = np->fs_ffree - np->fs_files * margin;
			bfree = np->fs_bfree - np->fs_blocks * margin; 

			if (bfree <= 0)
                        {
                                (void) sprintf (buf,
                                "\n%s:\nfile system %s needs %d Kbytes more to install the software specified.\n", prog, np->fs_path, bfree * np->fs_fsize* -1 / 1024);
                                Log (buf);
                                (void) fputs (buf, stderr);
                                overflow = 1;
                        }
			else if (!IN_ISL) 
			{

				(void) sprintf (buf, "\n%s:\n<warning> file system %s will be around %d%% full.\n", prog, np->fs_path, np->fs_cap); 
	
				if (np->fs_cap > 100) 
				{
					Log (buf);
					(void) fputs (buf, stderr);
					warning = 1; 
				}
			} 
			if  ((ffree <= 0) && (np->fs_type != MOUNT_NFS))
			{
                        	/* statfs () returns 0 in f_files and f_ffree 
				   for NFS so we shouldn't report error here */

				(void) sprintf (buf,
				"\n%s:\nfile system %s needs %d more inodes to install the software specified.\n", prog, np->fs_path, ffree*-1);
				Log (buf);
				(void) fputs (buf, stderr);
				overflow = 1;
			}
		}
	}

	(void) sprintf (buf, "\nData of affected file system(s) after traversing: \n");
	Log (buf);
	if (debug)
		(void) puts (buf);
	FsShow( fst, -1, AFFECTED_FS );

	if (overflow)
		return (OVER);
	else if (warning)
		return (WARNING);
	else
		return (SUCCESS);

}


/*	static FsT *FsFind() -
 *		searches for file system on which a file is to reside.
 *
 *	given:	PathT p - a pathname
 *		FsT *fsp - a pointer into the file system table (a list)
 *	does:	find the table entry representing the file system the
 *		pathname is located on.
 *	return:	a pointer to the entry, NULL if the entry wasn't found
*/

static FsT *FsFind( p, fsp )
PathT p;
FsT *fsp;
{
	if( fsp == NULL )	/* could not locate file system */
	{
		return( NULL );
	}

	/* does the current mount point fit inside the pathname? */
	if ( !strncmp( p, fsp->fs_path, fsp->fs_pathlen ) ) 
	{
		/* make sure not to match only part of a filename,
		   for example, /usr1/xxx shouldn't be matched under /usr */
		if ((fsp->fs_pathlen == 1) ||
	            (( p[fsp->fs_pathlen] == '/') || 
		     (p[fsp->fs_pathlen] == '\0')))
			
			/* found it */
			return( fsp );
	}

	/* look further down the list */
	return( FsFind( p, fsp->fs_next ) );
}


/*	static 	void 	FsGetInfo() -
 *		get size/access info of the specified file system
 *	given:	pointer to a file system in the file system table.
 *	does:	statfs and check whether file system is writable.
 *	return:	nothing
 *
*/

static	void	FsGetInfo (p)
FsT	*p;
{

	struct	statfs fs;

	if (statfs (p->fs_path, &fs, sizeof (struct statfs)))
	{
		(void) sprintf (buf,"%s: cannot access %s (%s).\n",
			prog, p->fs_path, sys_errlist[errno]);
		Log (buf);
		(void) fputs (buf, stderr);
		Exit(ERROR);
	}

	/* store fs fragment and block sizes */
	p->fs_bsize = fs.f_bsize;
	p->fs_fsize = fs.f_fsize;

	/* used later when calculating capacity */
	p->fs_blocks = fs.f_blocks;
	p->fs_bavail = fs.f_bavail;
	p->fs_minfree = (int) (rint ((double)((fs.f_bfree - fs.f_bavail)*100) / (double)(fs.f_blocks)));

	/* these numbers are calculated just once and stored for 
	   later use by TnUpdFsData 
	   NDADDR (12) indicates direct addresses in inode */ 
	p->fs_fthsd = NDADDR * fs.f_bsize;   
	p->fs_bfratio = fs.f_bsize / fs.f_fsize;	

	p->fs_files = fs.f_files;
	p->fs_type = fs.f_type;
	p->fs_bfree = fs.f_bfree;
	p->fs_ffree = fs.f_ffree;

	/* check if filesystem is writable. If not, give error
	   message and exit immediately */
	if (access (p->fs_path, W_OK))
        {
		(void) sprintf (buf,
                "%s: file system %s is not writable (%s).\n", prog, p->fs_path,
		sys_errlist[errno]);
		Log (buf);
                (void) fputs (buf, stderr);
                Exit (ERROR);
        }
}


/*	static	void	FsInit ()-
 *		partially initialize the file system entry with data
 *		from getmntinfo
 *	given:	pointer to a file system in the file system table.
 *	 	data from getmntinfo.	
 *	does:	initializes the file system entry.
 *	return:	nothing.
 *
*/

static 	void	FsInit( p, fs ) 
FsT *p;
struct statfs *fs;
{
	TnT	*node;
	PathT	path; 

	/* check if it is an automount mount point */
	p->fs_auto = ((fs->mount_info.nfs_args.flags & NFSMNT_AUTO)? 1 : 0);

	/* check mount point against the database tree, the path
	   may change if it is a symbolic link. However, 
	   do not check links for automount mount points because
	   lstat will cause the file system to be mounted */
	if (p->fs_auto)
		node = TnAdd (fs->f_mntonname, treetop, IGNORE_LINK);
	else
		node = TnAdd (fs->f_mntonname, treetop, CHECK_LINK);

	(void) strcpy (p->fs_path, node->tn_path); 
	p->fs_pathlen = strlen(p->fs_path);
 
	p->fs_inuse = 0;
	p->fs_next = NULL;
	p->fs_cap = 0;

}



/*	static FsT *FsInsert() -
 *		link a new fsnode into linked list in reverse alpha order.
 *
 *	RECURSION
*/

static FsT *FsInsert( s, t )
FsT *s, *t;
{
	if( t == NULL )	/* end of list, return */
		return( s );

	if( strcmp( s->fs_path, t->fs_path ) < 0 )
	{
		/* entry belongs further down the list */
		t->fs_next = FsInsert( s, t->fs_next );
		return( t );
	}
	else
	{
		/* entry should be inserted at head of list */
		s->fs_next = t;
		return( s );
	}
}



/*	static FsT *FsNew() -
 *		allocate storage for an FsT
 *
 *	given:	void
 *	does:	allocate storage for an FsT
 *	return:	a pointer to the FsT
*/

static FsT *FsNew()
{
	FsT	*p;


	p = (FsT *) malloc( sizeof(FsT) ) ;
	if (p == NULL)
	{
		(void) sprintf (buf,
			"%s: internal error, can not malloc. \n", prog);
		Log (buf);
		(void) fputs (buf, stderr);
		Exit (ERROR);
	}
	return (p);
}


/*	FsShow() -
 *		dump out node list.
 *
 *	RECURSION
*/

FsShow( p, n, show_all )
FsT *p; 
int n; 
short show_all; /* sho all file systems */ 
{
	if( p == NULL || n == 0 )
		return;

	if (show_all || FSUSED(p)) 
	{
		(void) sprintf(buf, "   %s\n\tFsize:\t%d\tBsize:\t%d\tType:\t%d\tAuto:\t%d\n\tTotalFrags:\t%d\tAvailFrags:\t%d\tMinFree:\t%d\n\tFreeInodes:\t%d\tFreeFrags:\t%d\tCap:\t%d\n", p->fs_path, p->fs_fsize, p->fs_bsize, p->fs_type, p->fs_auto, p->fs_blocks, p->fs_bavail, p->fs_minfree, p->fs_ffree, p->fs_bfree,p->fs_cap);
		Log (buf);
		if (debug)
			(void) puts (buf);
	}
	FsShow( p->fs_next, n-1, show_all );
}

/*	static long FsFragAlloc () -
 *		returns the number of fragments needed for the file size in
 *		the specified file system.
 *
 *	given:  a file size in bytes and the file system structure pointer
 *	does:	According to the size of the file, determines how many 
 *		fragments will be required to allocate the file.
 *	returns:number of fragments needed
 *
*/

static long FsFragAlloc (size, fs)
long	size; 	/* file size in bytes */ 
FsT	*fs;	/* file system info */

{
	long	remainder;
	short	adj;

	/* files are allocated in blocks. For files greater than fs_fthsd, 
	   an extra block is needed to act as the indirect block and 
	   the mod value of the block size (if > 0) is given a full
	   block (fs_bsize) to store.  
	   For files smaller than fs_fthsd, the mod value of block size
	   (if > 0) is allocated in fragments (fs_fsize) */

	if (size > fs->fs_fthsd)
	{
		remainder = size % fs->fs_bsize;
		/* at least one indirect block needs to be counted */
		adj = (remainder > 0) ? 2: 1; 
		return ((size / fs->fs_bsize + adj) * fs->fs_bfratio);
	}
	else
	{
		remainder = size % fs->fs_fsize;
		adj = (remainder > 0) ? 1: 0;
		return (size / fs->fs_fsize + adj);
	}
}


/*      static	void	Log() -
 *              write entry to logfile
 *
 *      given:  char *p - pointer to a buffer to be logged
 *      does:   write the buffer to the logfile
 *      return: nothing
*/

static	void	Log(p)
char *p;
{
	if (logfp != NULL)
		(void) fputs( p, logfp );
}


/*      static void     MenuUsage ()-
 *              prints Usage message on the screen.
 */

static void MenuUsage ()

{
        (void) fprintf(stderr,
        "\nUsage: %s [ -d ] [ root-path ] < file \n", prog);
        fflush (stderr);

        exit (ERROR);
}


/*      static	void	OpenLog() -
 *              open the logfile
 *
 *      given:  nothing
 *      does:   opens a channel to the logfile and writes an initial
 *              entry
 *      return: nothing
*/

static	void	OpenLog()
{
        long    t;      /* for use with ctime(3) */

        if ((logfp = fopen( FITLOG, "a")) == NULL )
        {
                (void) fprintf(stderr, "%s: cannot open log file %s (%s)\n",
                        prog, FITLOG, sys_errlist[errno]);
                errno=0;
        }
        pid = getpid();
        (void) sprintf( buf, "\n%s%d (%d) begin logging at %s\n", prog, pid, debug, asctime( localtime( ((void) time(&t),&t) ) ) );
        Log( buf );
}


/*	static void SetRootPath
 *		checks the validity of the root path 
 *
 *	given:  a path
 *	does:	checks whether the path can be a valid rootpath
 *		also sets global 'rootpath' and 'rootlen' if it is not '/'  
 *	return:	nothing
 */

static	void	SetRootPath (path)
char	*path;

{
	*rootpath = '\0';
	rootlen = 0;

	if (path == NULL) 
		return;

	if( *path != '/' )
	{
		(void) sprintf (buf,
			"%s: root path must be absolute.\n", prog );
		Log (buf);
		(void) fputs (buf, stderr);
		Exit(ERROR);
	}
	/* make sure root path exists */
	if( stat(path,&stb) )
	{
		(void) sprintf (buf,"%s: cannot access %s (%s).\n",
			prog, path, sys_errlist[errno]);
		Log (buf);
		(void) fputs (buf, stderr);
		Exit(ERROR);
	}
	/* make sure root path is a directory */
	if( !S_ISDIR( stb.st_mode ) )
	{
		(void) sprintf (buf,
			"%s: %s not a directory (mode %o).\n",
			prog, path, stb.st_mode );
		Log (buf);
		(void) fputs (buf, stderr);
		Exit(ERROR);
	}
	/* all clear, store it */
	
	if( strcmp( path, "/" ) )
	{
		/* non-'/' path specified, store it. If '/' was specified,
		   the empty string is used. */
		(void) strcpy(rootpath, path);
		rootlen = strlen(rootpath); 
	}

	(void) sprintf (buf, "rootpath = %s\n", rootpath);
	Log (buf);
	if (debug)
		(void) puts (buf);

}


/*	static TnT *TnAdd() - 
 *		According to the path given, add node(s) under node "top".
 *		For each filename in the path, there will be a corresponding
 *		node containing the name added in the tree.
 *	given:  path - a full path 
 *		top - a node under which to add the new node(s) 
 *		linkflag - to indicate whether links should be followed.  
 *	return: pointer to the last node just added. If node already in tree,
 *		then return a pointer to the old node.
 *
 *	RECURSIVE
 *
*/ 

static TnT *TnAdd (path, top, linkflag) 
char		*path;
TnT		*top;
short		linkflag; 
{
	char 	name[FILENAME_MAX];
	int 	c_proc;
 	TnT	*newnode, *tn;
	PathT	temp;

	if (*path == '\0')
		return (top);
	else 
	{
		c_proc = TnGetName (path, name);
		if (strlen (name) > 0) 
		{
			(void) strcpy (path, path + c_proc);
			newnode = TnAddUnder (name, top);

			(void) strcpy (temp, path);
			/* path & node may change if symbolic link involved */
			if (linkflag && (TnPathChange (path, &newnode) == 1)) 
			{
				(void) strcat (path, temp);

				/* check if path already walked */
				tn = TnPathCheck (path); 
				if (tn != NULL)
					return (tn);
			}

			/* recursive call */
			TnAdd (path, newnode, linkflag);
		}
		else /* path ends with trailing slash(es) */
			return (top);
	}
}

/*	static void TnAddSymLinks() -
 *		add all symbolic links specified in input to the tree 
 *
 *	given: 	pointer to stdin (global)
 *
 *	does:  	the symbolic link information is stored in the tree.
 *		After it's done, input is reset to prepare for the next
 *		round of operations. 
 *	return: nothing 
 */

static void TnAddSymLinks (ip)
InvT		*ip;		/* Inventory Pointer */

{
	
	InvRecT		*irp;		/* inventory record pointer */
	PathT		buf, path;	/* buffer to store link referent */
	TnT		*tn, *newnode;	/* tree node */

	while( (irp = InvRead( ip )) != NULL )
	{
		(void) sprintf (path, "%s%s", rootpath, irp->i_path + 1); 
 
		/* 'path' after TnAdd() call should remain the same 
		   because IGNORE_LINK is passed */
		newnode = TnAdd (path, treetop, IGNORE_LINK); 
		/* if it's a symbolic link, resolve the link referent */ 
	    	if (irp->i_type == 's') 
		{
			(void) strcpy (buf, irp->i_ref); 

			tn = (*buf == '/')  ? treetop : newnode->tn_up; 
			TnResolvePath (buf, tn, &newnode->tn_link); 

			newnode->tn_type = 's'; 
			newnode->tn_inv = 1;
			newnode->tn_size = irp->i_size;

			if (lstatflag && 
			    (lstat (newnode->tn_path, &stb) == 0)) 
			{
				/* existing link */ 
				newnode->tn_stsize = stb.st_size; 
				newnode->tn_stat = 1; 
			}
  
			if (debug)
				(void) printf (" * link referent of %s is %s\n", newnode->tn_path, newnode->tn_link->tn_path); 
		} 
	}

	/* reset file pointer to the beginning of input for the next round */
	if ((fseek (stdin, 0, SEEK_SET)) == -1)
	{
		(void) sprintf (buf,"%s: cannnot reset file pointer (%s).\n",
			prog, sys_errlist[errno]);
		Log (buf);
		(void) fputs (buf, stderr);
		Exit (ERROR);
	} 

}
  

/*	static TnT *TnAddUnder() -
 *		add a new node under its parent
 *		set tn_name, tn_top, tn_path
 *	given:	name - filename field of the new node
 *		top - parent of the new node
 *	does:	if the node is not in the parent's sub list, insert into the 
 *		list, else, just pass.
 *	return:	pointer to the new node. If new node not created, return the 
 *		old node.
 */

static TnT *TnAddUnder (name, top)
char	*name;
TnT	*top;

{
	TnT	*newnode, *oldnode;
	PathT	temp;

	oldnode = NULL;
	top->tn_sub = TnInsert (&newnode, top->tn_sub, &oldnode, name);

	if (oldnode != NULL) 
	/* node was already in list before the insert */ 
	{
		newnode = oldnode;
		/* 
		if (debug)
			(void) printf (" node already in database\n");
		*/
	}
	else
	{
		/*
		if (debug)
			(void) printf ("--- %s added under %s \n", name, 
				top->tn_path);
		*/

		newnode->tn_up = top;

		/* construct and save full path of this node for later use */
		if (newnode->tn_up != treetop)
			(void) sprintf (temp, "%s/%s",
			       newnode->tn_up->tn_path, newnode->tn_name);
		else
			(void) sprintf (temp, "%s%s",
			       newnode->tn_up->tn_path, newnode->tn_name);

		newnode->tn_path = (char *) 
				   malloc ((size_t) (strlen (temp) + 1));
		if (newnode->tn_path == NULL)
		{
			(void) sprintf (buf,
				"%s: internal error, can not malloc. \n", prog); 
			(void) fputs (buf, stderr);
			Exit (ERROR);
		}
		(void) strcpy (newnode->tn_path, temp);
	}
	
	return (newnode);
}


/*	static TnT *TnFind() -
 *		find the node with the specified path under the specified node.
 *	given:	path - full path of a node
 *		node - the node to start looking under
 *	does:	return the node with the specified path under a specified node.
 *		NULL if not found.
 *
 *	RECURSIVE
 */

static TnT *TnFind (node, path)
TnT	*node;
char	*path;

{
	char	name[FILENAME_MAX];
	int	c_proc;
	TnT	*nextnode;

	if (*path == '\0')
		return (node);
	else
	{
		c_proc = TnGetName (path, name);
		if (strlen (name) > 0)
		{
			(void) strcpy (path, path + c_proc);
			if (node->tn_sub == NULL)
				return (NULL);
			nextnode = TnFindNext (node->tn_sub, name);
			if (nextnode == NULL)
				return (NULL);
			TnFind (nextnode, path);
		}
		else /* path ends with trailing slash(es) */
			return (node);
	}

}


/*	static TnT *TnFindNext() -
 *		find the node in the sibling list with the specified name.
 *	given:	node - a node in the sibling list
 *		name - the search key file name.
 *	does:	return the node with the specified name. NULL if not found.
 *
 *	RECURSIVE
 */

static TnT *TnFindNext (node, name)
TnT	*node;
char	*name;

{
	int 	result;

	if (node == NULL)
		return (NULL);

	result = strcmp (node->tn_name, name);

	if (result < 0)
		return (NULL);
	else if (result == 0)
		return (node);
	else
		TnFindNext (node->tn_next, name);
}


/*	static	void TnGetName() -
 *		get the first filename in a path
 *	given:	str - a pathname starting with or without /
 *		name - storage location for the filename
 *	return:	1) one filename 
 *		2) number of characters processed (may be greater than
 *		   the length of name returned if additional /'s are involved) 
 */

static int TnGetName (str, name)
char	*str, *name;
{
	char *str_orig;

	str_orig = str;		/* save the starting address */
	while (*str == '/')
		str++;

	for (; *str != '/' && *str != '\0'; str++, name++)
	{
		*name = *str;
	}
	*name = '\0';

	return (str - str_orig); 
}

/* 	static void TnGetVarInfo() -
 *		get system information about how var and usr/var are set up. 
 *		This is only necessary during ISL. 
 *	given:  global variable 'treetop'. 	
 *		Since this is done for ISL only, the value of the global
 *		variable 'lstatflag' upon entry should always be NO_LSTAT 
 *	does:	calls TnAdd() to add /var and /usr/var in the tree first.
 *		Through TnAdd(), /var and /usr/var will be lstat'ed and
 *		the link information will be stored.
 *	return:	nothing
 *
 */	

static	void 	TnGetVarInfo ()

{
	PathT	path;
	TnT	*newnode;

	/* temporarily set lstatflag so that var and usr/var can be lstat'ed 
	   in TnPathChange() */
	lstatflag = DO_LSTAT;

	sprintf (path, "%s%s", rootpath, "/var"); 
	newnode = TnAdd (path, treetop, CHECK_LINK);

	(void) sprintf (buf, "var, path = %s\n", newnode->tn_path);
	Log (buf);
	if (debug)
		(void) puts (buf);

	sprintf (path, "%s%s", rootpath, "/usr/var"); 
	newnode = TnAdd (path, treetop, CHECK_LINK);

	(void) sprintf (buf, "usr/var, path = %s\n", newnode->tn_path);
	Log (buf);
	if (debug)
		(void) puts (buf);

	/* reset lstatflag for ISL */ 
	lstatflag = NO_LSTAT;
}

/*	static TnT *TnInsert() -
 *		insert a node among its siblings, in reverse alpha order.
 *	given:	s - address of pointer to a node 
 *		t - pointer to the list of siblings 
 *		l - pointer to a node pointer 
 *		name - pointer to the name string to be inserted.
 *	return: 1) if node already exists in tree, return pointer to old
 *		   node in l.
 *		2) the function returns pointer to fisrt node of the siblings.
 *
 *	RECURSIVE
 */

static TnT *TnInsert (s, t, l, name)
TnT	**s, *t, **l;
char	*name;

{
	int result;

	if (t == NULL)	/* end of list, return */
	{
		*s = TnNew (name);
		return (*s);
	}

	result = strcmp (name, t->tn_name);

	if (result < 0)
	{
		/* entry belongs further down the list */
		t->tn_next = TnInsert (s, t->tn_next, l, name);
		return (t);
	}
	else if (result > 0)
	{
		/* entry should be inserted at head of list */
		*s = TnNew (name);
		(*s)->tn_next = t;
		return (*s);
	} 
	else {
		*l = t;		/* return pointer to the existing node */
		return (t); 
	} 
}

/*	static TnT *TnNew() -
 *		allocate storage for a tree node and initialize it
 *	given:	name - file name of this new node (not the full path)	
 *	return: a pointer to TnT
*/

static TnT *TnNew (name)
char	*name;
{
	TnT	*node;

	node = (TnT *) malloc (sizeof (TnT));
	if (node == NULL)
	{
		(void) sprintf (buf,
			 "%s: internal error, can not malloc. \n", prog);
		Log (buf);
		(void) fputs (buf, stderr);
		Exit (ERROR);
	}
	node->tn_name = (char *) malloc ((size_t) (strlen (name) + 1));
	if (node->tn_name == NULL)
	{
		(void) sprintf (buf,
			"%s: internal error, can not malloc. \n", prog); 	
		Log (buf);
		(void) fputs (buf, stderr);
		Exit (ERROR); 
	}
	(void) strcpy (node->tn_name, name);
	node->tn_up = node->tn_next = node->tn_sub = node->tn_link = NULL;
	node->tn_inv = 0;
	node->tn_type = ' '; 
	return (node); 
}


/* 	static void	TnPListFree() -
 *		free storage space used by pathlist
 *	given: none
 *	return: none
 */

static void TnPListFree ()

{
	TnPathT	*next;

		/* free up storage for stored paths */
		while (pathlist != NULL) 
		{
			free (pathlist->pn_path); /* free path */
			next = pathlist->pn_next;
			free (pathlist);	  /* free node */
			pathlist = next;
		}
		pathlist = listtail = NULL;
}

			

/*	static	void TnPathChange() -
 *		first checks tn_link to see if symbolic link information
 *		is already obtained (including checking for link loop),if not, 
 *		does an lstat to check if the node contains a symbolic
 *		link in its full path. If yes, updates the path and the node
 *		as well as sets tn_link.  tn_stsize is also recorded
 *		for later use.
 *	given:	path - pointer to a string 
 *		node - address of a pointer to a tree node
 *	return:	1 if path is modified.
 */

static int TnPathChange (path, node)
char	*path;
TnT	**node;
 
{
	PathT	buf, temp;
	int	n; 
	TnT	*tn;
	char	name[FILENAME_MAX];
	int	c_proc;
	

	/* if symbolic link information already stored */
	if ((*node)->tn_link != NULL)
	{
		if ((*node)->tn_link == *node) /* points to itself */
			return (0);

		/* search down the list of symbolic links for any loop */
		for (tn = (*node)->tn_link; tn->tn_link != NULL; 
		     tn = tn->tn_link)
		{
			if (tn == *node)
			{ 
				/* we just found a loop */
				(*node)->tn_link = tn; /* set for later use */
				(void) sprintf(buf,
				"%s: <warning> %s is a symbolic link to itself.\n", prog, (*node)->tn_path);
				Log (buf);
				(void) fprintf(stderr, "%s", buf);
				return (0);	
			}
			else if (tn->tn_link == tn)   
			{
				/* contains a link that points to itself */
				(void)strcpy (path, tn->tn_path);
				*node = treetop;
				return (1);
			}
		}

		if (debug)
                        (void) printf (" * replacing %s with link %s\n",
                                (*node)->tn_path, (*node)->tn_link->tn_path);

		(void) strcpy (path, (*node)->tn_link->tn_path);
		*node = treetop;
		return (1);
	}

	/* if not in ISL and no link information, do lstat */
	if (lstatflag && (lstat ((*node)->tn_path, &stb) == 0)) 
	{ 
		/* existing file, record size for use in traversal */
		(*node)->tn_stsize = stb.st_size;
		(*node)->tn_stat = 1;
	
		/* symbolic link */
		if ((S_ISLNK (stb.st_mode)) &&
	            ((n = readlink ((*node)->tn_path, buf, sizeof(buf))) != -1)) 
		{	
			(*node)->tn_type = 's';
			buf[n] = '\0';	/* null terminate, just in case */

			tn = (*buf == '/')  ? treetop : (*node)->tn_up; 
			TnResolvePath (buf, tn, &(*node)->tn_link); 

			if (debug)
				printf (" * %s points to %s\n", 
					(*node)->tn_path, buf); 

			(void) strcpy (path, buf);

			/* start from top again */
			*node = treetop; 
			return (1);
		}
	}
	else /* can not lstat */
	{
		(*node)->tn_stsize = 0;
	}
	return (0);
}


/*	static TnT *TnPathCheck() -
 *		checks if the path is already in the pathlist. If yes,
 *		then go through the pathlist to find the tree node that
 *		contains the real file. (Note: all paths in the pathlist
 *		point to the same file). Otherwise, add the path in the
 *		pathlist and return NULL.
 *	given:	a path to be checked against pathlist.
 *	return: pointer to a tree node if path already in the list or
 *		NULL if path is to be added
 */	

static	TnT *TnPathCheck (path)
char	*path;

{
	TnPathT		*pnode, *pn;
	char		*lp;
	TnT		*tn;
   
	/* check if path already walked */
	for (pnode = pathlist; pnode != NULL; pnode = pnode->pn_next) 
	{
		if (strcmp (pnode->pn_path, path) == 0)
		{
			/* find the tree node that's got the real stuff */
			for (pn = pathlist; pn != NULL; pn = pn->pn_next)
			{
				tn = TnFind (treetop, pn->pn_path);
				if (tn->tn_type != 's')
					return (tn); 
			}

			return (TnFind (treetop, path));
		}
	}
			 
	/* path not in pathlist, record the new path */
	lp = (char *) malloc ((size_t) (strlen (path) + 1));
	if (lp == NULL) 
	{
		(void) sprintf (buf,
			"%s: internal error, can not malloc. \n", prog);
		Log (buf);
		(void) fputs (buf, stderr);
		Exit (ERROR);
	}
	(void) strcpy (lp, path);
	TnPNodeNew (lp);
	return (NULL);
}


/*	static TnPathT *TnPNodeNew() -
 *		allocate storage for a path node and initialize it
 *	given:	path - full path  	
 *	return: a pointer to TnPathT
*/

static TnPathT *TnPNodeNew (path)
char	*path;
{
	TnPathT	*node;

	node = (TnPathT *) malloc (sizeof (TnPathT));
	if (node == NULL)
	{
		(void) sprintf (buf,
			"%s: internal error, can not malloc. \n", prog); 	
		Log (buf);
		(void) fputs (buf, stderr);
		Exit (ERROR);
	}
	node->pn_path = path;
	node->pn_next = NULL;

	if (debug)
		printf ("  ( --- TnPNodeNew, new path = %s)\n", node->pn_path);

	if (pathlist == NULL)
		pathlist = listtail = node;	
	else
	{
		listtail->pn_next = node;
		listtail = node;
	}

	return (node);
}


/*	static int TnResolvePath() -
 *		resolve a link path which may meet following conditions:
 *			1. contains ..
 *			2. contains .
 *			3. is relative
 *			4. contains dir(file) not yet entered in the database.
 *		to an absolute complete path. 
 *	given:	path - a link path generated by readlink()
 *		node - pointer to a tree node
 *		linknode - address of a pointer to a tree node.
 *	return: none
 *
 *	RECURSIVE
 */ 

static	void TnResolvePath (path, node, linknode)
char 	*path;
TnT	*node, **linknode;

{
	char	name[FILENAME_MAX];
 	int	c_proc;
	TnT	*n;
	PathT	temp;

	if (*path == '\0')
	{
		(void) strcpy (path, node->tn_path);
		*linknode = node;
		return;
	}
	else
	{
		c_proc = TnGetName (path, name);
		if (strlen (name) > 0)
		{
			(void) strcpy (path, path + c_proc);
			if (strcmp (name, "..") == 0) 
			{
				TnResolvePath (path, node->tn_up, linknode);
			}
			else if (strcmp (name, ".") == 0) 
			{
				TnResolvePath (path, node, linknode); 
			}
			else	
			{
				/* find node with 'name' in sub list */
				/* if not found, add right away */
				if ((n = TnFindNext (node->tn_sub, name))
				    == NULL)
					n = TnAddUnder (name, node);
				TnResolvePath (path, n, linknode);
			}
		}
		else 
		{
			(void) strcpy (path, node->tn_path);
			*linknode = node;
			return;
		}
	}
}


/*	static void TnTraverse() -
 *		traverse the database tree and update file system data
 *		at significant nodes.
 *
 *	given:	node - pointer to a node to start traversing
 *	return: none
 *
 *	RECURSIVE
 *
 */

static void TnTraverse (node)
TnT	*node;

{
	
	if (node == NULL)
		return;

	if (debug)
		(void) printf ("\n --- traverse, file = \t%s \n",node->tn_path);

	/* update file system data only when full path of this node 
		  is an inventory record */
	if (node->tn_inv == 1)
		TnUpdFsData (node);

	TnTraverse (node->tn_sub);

	TnTraverse (node->tn_next);

}


/*	static void TnUpdFsData() -
 *		update file system data according to the node type/size.
 *
 *	given: 	node - pointer to a database tree node
 *	return: none
 *
 */

static	void TnUpdFsData (node)
TnT	*node;

{
	FsT	*np;

	/* locate the filesystem this file lives on */
	if ((np = FsFind (node->tn_path, fst)) == NULL)
	{
		/* this should never happen!?! */
		(void) sprintf (buf,
			"%s: cannot find file system for %s\n", prog,
			node->tn_path);
		Log (buf);
		(void) fputs (buf, stderr);
		return;
	}

	/* mark file system as in use */
	if (!np->fs_inuse) 
	{
		np->fs_inuse |= FS_INUSE;

		FsGetInfo (np);	/* get size/access info of this fs */
		
		(void) sprintf (buf,  "\nLocated on file system (before traversing):\n" ); 
		Log (buf);
		if (debug)
			(void) puts (buf);
		FsShow( np, 1, ALL_FS );
	}

	if (node->tn_type == 'l') /* hard link */
	{ 	/* do nothing */
		if (debug)
			(void) printf( "HARD LINK\n" );
	}
	else if (!node->tn_stat) /* wasn't able to lstat earlier */
	{
		/* new file, decrement block and gnode count */

		np->fs_bfree -= FsFragAlloc ((long)node->tn_size, np);
		np->fs_ffree -= 1;

		if (debug)
			(void) printf( "   NEW FILE\n" );
	}
	else
	{
		/* existing file, determine block difference */
		if (node->tn_size != node->tn_stsize)
		{
			/* reclaim old file space and decrement new space */
			np->fs_bfree -= 
				FsFragAlloc ((long)node->tn_size, np) -
				FsFragAlloc ((long)node->tn_stsize, np);
		
		}

		if (debug)
			(void) printf( "   EXISTING FILE\n" );
	}

	if (debug)
        {
		(void) printf ("\ton:\t\t%s\n", np->fs_path);
                (void) printf( "   Updated File Stat:\n" );
		(void) printf ("\tFreeInodes:\t%d\tFreeFrags:\t%d\n",
				np->fs_ffree, np->fs_bfree); 
        }

}


/*END*/
