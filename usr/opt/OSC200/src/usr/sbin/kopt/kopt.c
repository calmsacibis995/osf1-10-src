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
static char *rcsid = "@(#)$RCSfile: kopt.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/11/13 00:08:58 $";
#endif
/************************************************************************
* kernel_options.c -							*
*	Reads kernel options from a database file, writes mandatory and	*
*	zombie options to /tmp/.config, offers a menu from which a user *
*	can select optional options, and then writes any of those sel-	*
*	ected into /tmp/.config also.					*
*									*
************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <strings.h>

#define FALSE	0
#define TRUE	1
#define MASK1	"%[^:]:%[^:]:%[^#]"
#define MASK2	"%[^:]"
#define	PATH1	"/usr/share/lib/kernel_options/kernel_options.db"
#define PATH2	"/tmp/.config"


/********************
* Global Structures *
********************/
typedef struct dbmem
{
	struct	dbmem	*nextmem;
	char	*member;
} DBMEM;

typedef struct dbent
{
	struct	dbent	*nextko;
	char	*ko_desc;
	char	*ko_stat;
	DBMEM	*ko_member;
} DBENT;


/************************
* Function Declarations *
************************/
char *	CharMalloc();
void	CloseFiles();
char	ConfirmChoice();
int	DisplayMenu();
void	Exit();
DBENT *	Fillerup();
DBENT *	LoadOptional();
DBENT *	Newdbent();
DBMEM *	Newdbmem();
void	OpenDBFile();
void	OpenWRFile();
int	ParseChoice();
void	ParseEntry();
char *	ParseField();
char *	ReadDBFile();
char *	SkipLsp();
char *	SkipTsp();
void	WriteMandatory();
void	WriteOptional();


/*******************
* Global Variables *
*******************/
FILE	*fp1,*fp2;
int	P2stat=0;
int	i_list[BUFSIZ];
char	s_list[BUFSIZ];
char	Desc[BUFSIZ];
char	Stat[BUFSIZ];
char	Type[BUFSIZ];

char *header0="*** KERNEL OPTION SELECTION ***";
char *header1="    Selection   Kernel Option";
char *header2="---------------------------------------------------------------";



/****************************************
* SKIPLSP - Skip Leading White Space	*
*					*
* Get rid of any leading white space.	*
****************************************/
char *
SkipLsp(s)
char *s;
{
	while(((iscntrl(*s)) || (isspace(*s))) && (*s != NULL)) s++;
	return(s);
}


/********************************************************
* SKIPTSP - Skip Trailing White Space			*
* 							*
* Get rid of any trailing while space, including NL.	*
********************************************************/
char *
SkipTsp(s)
char *s;
{
	char	*p;

	p=s+(strlen(s));

	for(p--;;p--)
	{
		if((!iscntrl(*p)) || (!isspace(*p)))
		{
			p++;
			*p='\0';
			break;
		}
	}
}


/********************************************************
* NEWDBENT - New database entry				*
*							*
* Malloc a new kernel option entry structure, and init	*
* the fields to NULL.					*
********************************************************/
DBENT *
Newdbent()
{
	DBENT *l;

	if((l=(DBENT *) malloc(sizeof(DBENT))) == NULL)
	{
		printf("\nCannot allocate memory.");
		Exit(1);
	}

	l->nextko=NULL;
	l->ko_desc=NULL;
	l->ko_stat=NULL;
	l->ko_member=NULL;

	return(l);
}


/********************************************************
* NEWDBMEM - New Database Member			*
*							*
* Malloc a new kernel option member structure, and init *
* the fields to NULL.					*
********************************************************/
DBMEM *
Newdbmem()
{
	DBMEM *l;

	if((l=(DBMEM *) malloc(sizeof(DBMEM))) == NULL)
	{
		printf("\nCannot allocate memory for DBMEM.");
		Exit(1);
	}

	l->nextmem=NULL;
	l->member=NULL;

	return(l);
}


/********************************************************
* CHARMALLOC - Character Memory Allocation		*
*							*
* Allocate memory for a character variable.		*
********************************************************/
char *
CharMalloc(i)
int	i;
{
	char	*s;

	if((s=(char *) malloc(i)) == NULL)
	{
		printf("Cannot allocate memory from CharMalloc().");
		Exit(1);
	}

	return(s);
}


/********************************************************
* Exit - Generic exit handler				*
*							*
* Perform cleanup and exit with proper status.		*
********************************************************/
void
Exit(status)
int	status;
{
	CloseFiles();
	switch(status)
	{
		case 0 :
			exit(0);
		case 1 :
		default:
			system("rm -f /tmp/.config");
			exit(1);
	}
}


/********************************************************
* FILLERUP - Fill up structures				*
*							*
* Put data into the entry and member structures.	*
********************************************************/
DBENT *
Fillerup(nsp)
DBENT	*nsp;
{
	DBMEM	*newmem;
	char	*cp,part[BUFSIZ];

	cp=Type;

	nsp->ko_desc=CharMalloc(strlen(Desc)+1);
	nsp->ko_stat=CharMalloc(strlen(Stat)+1);
	strcpy(nsp->ko_desc,Desc);
	strcpy(nsp->ko_stat,Stat);

	while(strlen(cp) > 0)
	{
		bzero(part,BUFSIZ);
		sscanf(cp,MASK2,part);
		cp=(cp + (strlen(part)) + 1);
		SkipTsp(part);
		newmem=Newdbmem();
		newmem->member=CharMalloc(strlen(part)+1);
		strcpy(newmem->member,part);
		newmem->nextmem=nsp->ko_member;
		nsp->ko_member=newmem;
	}
	return(nsp);
}


/************************************************
* OPENDBFILE - Open Database File		*
*						*
* Open input file for read operations.		*
************************************************/
void
OpenDBFile()
{
	if((fp1=fopen(PATH1,"r")) == NULL)
	{
		printf("Cannot open PATH1.\n");
		Exit(1);
	}
}


/************************************************
* OPENWRFILE - Open WRite File			*
*						*
* Open output file for write operations.	*
************************************************/
void
OpenWRFile()
{
	if(!P2stat)
	{
		if((fp2=fopen(PATH2,"w")) == NULL)
		{
			printf("Cannot open PATH2.\n");
			Exit(1);
		}

		P2stat=1;
	}
}


/************************************************
* CLOSEFILES - Close Files			*
*						*
* Close all open files.				*
************************************************/
void
CloseFiles()
{
	fclose(fp1);
	fclose(fp2);
}


/****************************************
* ReadDBFile - 				*
*					*
* Read database entries from PATH1, and	*
* perform some prep work on each line.	*
****************************************/
char *
ReadDBFile()
{
	char	*ip,*pp,input[BUFSIZ],parsed[BUFSIZ];

	ip=input;
	pp=parsed;
	bzero(input,BUFSIZ);
	bzero(parsed,BUFSIZ);
	while(fgets(input,BUFSIZ,fp1)!=NULL)
	{
		/*
		* If the first character is a '#', it
		* means we read in a comment line.
		* OR
		* If the first character returned by skip-white-space is
		* a control-character, it means we read in an empty line.
		* 
		* If either case is true, skip this entry and read in the
		* next line. Otherwise, send the line to SkipTsp and get
		* rid of the <NL> at the end of the string.
		*/
		ip=SkipLsp(input);
		if((*ip == '#') || (iscntrl(*ip))) continue;
		SkipTsp(ip);

		/*
		* It is very possible that an entry may be comprised of
		* more than one line.  This following part concatenates
		* multi-line database entries into a single entry, and
		* deposits it in the variable 'parsed'.  First, we skip
		* leading white space, then take just the part between
		* colons ":", and add the colons back as we need them.
		* Notice that we only add a colon to parsed (via the pp
		* pointer) if the previous character in parsed WAS NOT
		* a colon.  
		*/
                while(*ip != '\0')
                {
			ip=SkipLsp(ip);
			ip=ParseField(ip,pp);
			pp=parsed+(strlen(parsed));
			if((*ip == ':') && (*(pp-1) != ':'))
			{
				*pp=*ip;
				ip++,pp++;
			}
		}

		/*
		* Ok, so now we've gone through a complete line that was
		* read in from the database.  If the last character was
		* a colon, it means that the database entry is a multiple
		* line entry, and that there is more to come.  So reset
		* the variable 'input' and it's pointer to NULL, read in
		* another line, and go through all the prep work again.
		* Continue doing this until we don't have a colon at the
		* end of 'parsed'.  That means we hit the end of the entry.
		*/
		if(*(pp-1) == ':')
		{
			ip=input;
			bzero(input,BUFSIZ);
			continue;
		}
		else
			return(parsed);
	}
	return(NULL);
}


/********************************************************
* PARSEFIELD - Get the data from the fields within the	*
*              colons.					*
*							*
* This function takes two pointers, and steps through	*
* the first pointer transfering characters to the 2nd	*
* pointer until it finds a colon.			*
********************************************************/ 
char *
ParseField(p1,p2)
char *p1,*p2;
{
	int	comment=FALSE;

	for(;*p1 != '\0';p1++)
	{
		if(*p1 == '#') comment=TRUE;
		if(*p1 == ':') break;
		if(!comment)
		{
			*p2=*p1;
			p2++;
		}
	}
	return(p1);
}


/********************************************************
* LOADOPTIONAL - populate a structure with optional	*
*		 option data.				*
*							*
* If we have an optional option from the database, this *
* function allocates memory for a structure to hold the *
* option data, and then fills the structure with the	*
* data.							*
********************************************************/
DBENT *
LoadOptional(oldptr)
DBENT	*oldptr;
{
	DBENT	*newptr;

	newptr=Newdbent();
	newptr=Fillerup(newptr);
	newptr->nextko=oldptr;
	return(newptr);
}


/****************************************************************
* WRITEOPTIONAL - write optional data to the output file.	*
****************************************************************/
void
WriteOptional(rootptr)
DBENT	*rootptr;
{
	int	x,y;
	DBENT	*entptr;
	DBMEM	*memptr;

	entptr=rootptr;

	for(y=1;entptr->nextko != NULL;y++,entptr=entptr->nextko)
		for(x=0; i_list[x] != NULL; x++)
			if(y == i_list[x])
				for(memptr=entptr->ko_member;
				    memptr != NULL;
					memptr=memptr->nextmem)
					   fprintf(fp2,"%s\n",memptr->member);
}


/********************************************************
* WRITE MANDATORY - write mandatory and zombie option	*
*		    data to the output file.		*
********************************************************/
void
WriteMandatory()
{
	char	*cp,part[BUFSIZ];

	cp=Type;
	while(strlen(cp) > 0)
	{
		bzero(part,BUFSIZ);
		sscanf(cp,MASK2,part);
		cp=(cp + (strlen(part)) + 1);
		SkipTsp(part);
		switch(*Stat)
		{
			case 'M':
			case 'O':
				fprintf(fp2,"%s\n",part);
				break;
			case 'Z':
				fprintf(fp2,"#%s\n",part);
				break;
			default :
				printf("\nBad option status.");
				Exit(1);
		}
	}
}


/****************************************************************
* PARSEENTRY - separate database data into three entries	*
*								*
* Database items are made up of three major fields.  The	*
* description, the status, and the type.  These are global	*
* variables defined at the top of the program.			*
****************************************************************/
void
ParseEntry(s)
char	*s;
{
	sscanf(s,MASK1,Desc,Stat,Type);
}


/****************************************************************
* DISPLAYMENU - Display the menu of optional options that the	*
* user is to choose from.					*
****************************************************************/
int
DisplayMenu(rootptr)
DBENT	*rootptr;
{
        int     x=0,y=0,i=0;
	int	*ip;
	char	*sp;
	DBENT	*entptr;

	entptr=rootptr;

	ip=i_list;
	sp=s_list;
	bzero(i_list,BUFSIZ);
	bzero(s_list,BUFSIZ);
        printf("\n\n%s\n%s\n",header1,header2);

	for(;entptr->nextko != NULL; entptr=entptr->nextko)
        {
                i++;
                printf("\t%d\t%s\n",i,entptr->ko_desc);
        }

        printf("\t%d\tAll of the above\n",i+1);
        printf("\t%d\tNone of the above\n",i+2);
        printf("%s\n\n",header2);
        printf("Enter the selection number for each kernel ");
        printf("option you want.");
        printf("\nFor example, 1 3 : ");
        fflush(stdout);
        gets(s_list);
	return(i);
}


/****************************************************************
* ParseChoice()							*
*								*
* This routine handles the choice list (s_list) if it has more	*
* than one choice in it.  It's function is to determine the	*
* validity of the choice that was made, to make sure each	*
* choice was an integer, and that each choice was a valid menu	*
* option.							*
*								*
* Variables -							*
*	i - the number of optional items available to select	*
*	j,k,x,y - subscripts for array variables		*
*	i_list - a global variable integer list of menu choices	*
*	s_list - a global variable ascii list of menu choices	*
*		 from the Display_Menu_Routine() function	*
*								*
* Returns -							*
*	0 - they selected "None of the Above" from the menu	*
*	1 - they selected something from the menu		*
*	2 - the selection is not valid				*
****************************************************************/
int
ParseChoice(i)
	int	i;
{
	int	j,k,x,y,notdigit=0,duplicate=0;
	char	tmp_buf[5];

	bzero(tmp_buf,5);
	bzero(i_list,BUFSIZ);

	for(j=0,k=0,x=0; ; j++)
	{
		/************************************************
		* if the current character "s_list[j]"is a tab, *
		* space, or control character, then we want to	*
		* take some action with whatever is stored in	*
		* the temp buffer "tmp_buf".			*
		************************************************/
		if((isspace(s_list[j])) || (iscntrl(s_list[j])))
		{
			if(iscntrl(tmp_buf[0]))
			{
				if(iscntrl(s_list[j]))
				{
					if(isspace(s_list[j]))
					{
						continue;
					}
					break;
				}
				continue;
			}


			/********************************************
			* Check to see if this character is a digit *
			********************************************/
			for(y=0; tmp_buf[y] != 0; y++)
			{
				if((isdigit(tmp_buf[y])) == 0)
				{
					notdigit=1;
					break;
				}
			}


			/******************************************
			* if it is NOT a digit, or it is a number *
			* greater than the number of items in our *
			* menu, tell them the choice is invalid,  *
			* and then we'll return (2) which will    *
			* make us loop and go back to the menu.   *
			******************************************/
			if((notdigit) || (atoi(tmp_buf) > i+2))
			{
			    printf("\n\n*** '%s' is an Invalid Option!",
					tmp_buf);
			    printf("  Please make your selection again. ***");
			    return(2);
			}


			/************************************************
			* Check to see if they selected "All of the	*
			* above" as their menu choice.  If so, pump	*
			* i_list full until the number of items in the	*
			* menu is reached.				*
			************************************************/
			if(atoi(s_list) == i+1)
			{
				for(x=0,y=1; y <= i; x++,y++)
					i_list[x]=y;
				break;
			}


			/************************************************
			* Check to see if they selected "None of the	*
			* above" as their menu choice.  If so, we'll	*
			* return (0) and confirm this with them in the	*
			* next routine.					*
			************************************************/
			if(atoi(tmp_buf) == i+2)
				return(0);


			/************************************************
			* Now check for duplicates.  We don't want any	*
			* selection number appearing in our final list	*
			* more than once.				*
			************************************************/
			for(y=0; i_list[y] != 0; y++)
			{
		    		if(i_list[y] == atoi(tmp_buf))
		    		{
					duplicate=1;
					break;
		    		}
			}


			if(duplicate)
			{
				k=0;
				duplicate=0;
				bzero(tmp_buf,5);
			}
			else
			{
		    		i_list[x]=atoi(tmp_buf);
		    		k=0,x++;
		    		bzero(tmp_buf,5);
			}
		}
		else
		{
			/************************************************
			* The current character "s_list[j]" was not a	*
			* space, tab, or control character, so we'll	*
			* store the character in our temporary buffer.	*
			************************************************/
			tmp_buf[k]=s_list[j];
			k++;
		}
	}


	/****************************************************************
	* Now, if i_list still has a 0 in it, it means that s_list	*
	* did not have any valid menu selections in it, so we'll return *
	* (2).  Otherwise, we got a valid selection so we'll return (1)	*
	****************************************************************/
	if(i_list[0] == 0)
		return(2);
	else
		return(1);
}


/****************************************************************
* CONFIRMCHOICE - Ask the user to confirm that the choices they *
* made under DisplayMenu are the ones that they really want.	*
****************************************************************/
char
ConfirmChoice(i,rootptr)
int	i;
DBENT	*rootptr;
{
	int	x,y;
	char	ans[3];
	DBENT	*dbptr;

	dbptr=rootptr;

	while(TRUE)
	{
		if(i == 0)
		{
			printf("\nYou do not want to select ");
			printf("any kernel options.");
		}
		else
		{
			printf("\n\nYou selected the following ");
			printf("kernel options:\n\n");
			for(y=1;dbptr->nextko != NULL;y++,dbptr=dbptr->nextko)
			{
				for(x=0; i_list[x] != NULL; x++)
					if(y == i_list[x])
					    printf("\t%s\n",dbptr->ko_desc);
			}
		}
		printf("\nIs that correct? (y/n) [y]: ");
		fflush(stdout);
		gets(ans);
		switch(*ans)
		{
			case 'y':
			case 'Y':
			case '\0':
				return('y');
				break;
			case 'n':
			case 'N':
				return('n');
				break;
			default:
				break;
		}
	}
}


main(argc,argv)
int	argc;
char	**argv;
{
	int	i,x=1;
	int	a_flag=0;	/* Set to (1) for ALL options */
	int	m_flag=0;	/* Set to (1) for mandatory options only */
	char	a;
	char	*dbp;		/* Pointer to database entry */
	DBENT	*rootptr;	/* Pointer to beginning of linked list */

	for(++argv;*argv != '\0';++argv)
	{
		if(strcmp(*argv,"-m") == 0) m_flag=1;
		if(strcmp(*argv,"-a") == 0) a_flag=1;
	}

	rootptr=Newdbent();

	OpenDBFile();
	OpenWRFile();
	while((dbp=ReadDBFile()) != NULL)
	{
		bzero(Desc,BUFSIZ);
		bzero(Stat,BUFSIZ);
		bzero(Type,BUFSIZ);
		ParseEntry(dbp);
		switch(*Stat)
		{
			case 'M':
			case 'Z':
				WriteMandatory();
				break;
			case 'O':
				if(a_flag)
					WriteMandatory();
				else
					rootptr=LoadOptional(rootptr);
				break;
			default :
				printf("\nKernel option '%s' has ",Desc);
				printf("invalid status '%s'.",Stat);
				printf("\nCheck %s for accuracy.\n",PATH1);
				Exit(1);
		}
	}

	if(rootptr->ko_desc != NULL && !m_flag && !a_flag)
	{
        	printf("\n\n%s",header0);
		while(x)
		{
			i=DisplayMenu(rootptr);
			i=ParseChoice(i);
			switch(i)
			{
				case 0:
				case 1:
					a=ConfirmChoice(i,rootptr);
					if(a == 'y')
					{
						if(i == 0) x=0;
						else
						{
							WriteOptional(rootptr);
							x=0;
						}
					}
					break;
				default:
					break;
			}
		}
	}
	Exit(0);
}
