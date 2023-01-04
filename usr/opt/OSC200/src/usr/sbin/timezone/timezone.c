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
static char	*sccsid = "%W%	(DEC OSF/1)	%G%";
#endif lint

#include	<stdio.h>
#include	<ctype.h>

#define		BUF1		20
#define		BUF2		100
#define		BUF3		30
#define		LINE		BUFSIZ
#define		AREA		struct tz_area_info
#define		SPACING		14
#define		NO_SELECTION	99
#define		OPTIONS		"[-d]"
#define		PATH1		"/usr/share/lib/timezone/timezone.db"
#define		PATH2		"/usr/share/lib/timezone/dst.db"
#define		PATH3		"/etc/svid2_tZ"

/*******************
* Global Variables *
*******************/
char	*prog;
char	*err_ban="*****************************************************";
char	*err_msg="* Invalid response!  Please make another selection. *";
/* start 001-kak */
char	*err_msg2="* Invalid response or end of input file.           *";
int	filedes; /* 001-kak file descriptor for input file */
FILE	*stinptr;
/* end 001-kak */

/********************
* Global Structures *
********************/
/*
 * tz structures - these structures hold timezone data from PATH1
 */
struct tz_area_info
{
	char	name[BUF1];
	char	time[BUF1];
	int	dst;
};

struct tz_zone_info
{
        char    name[BUF1];
	AREA	area[BUF3];
};

struct tz_zone_info tz[BUF2];

/*
 * dst structure - this structure holds daylight savings data from PATH 4
 */
struct dst_info
{
	int	code;
	char	area[BUF3];
};
struct	dst_info dst_s[10];


/***********************
* Main Program Section *
***********************/
main(argc,argv)
	int	argc;
	char	**argv;
{
	int	i,j,x,dst,option;
	
	prog=argv[0];

	/* Must be superuser to run
         */
        if (geteuid() != 0){
                (void) fprintf(stderr, "timezone:  must be super user\n");
                (void) fflush(stderr);
                exit(1);
        }

        /* Pick out command line options */
        while(*++argv)
        {
                switch(argv[0][1])
                {
                        case 'd':
                                option='d';
                                break;
                        default:
                                printf("Usage: %s %s\n",prog,OPTIONS);
                                exit(0);
                                break;
                }
        }

	Load_Data_Routine();

	for( ; ; )
	{
		j=1;
		i = Display_Main_Menu();

		if(i == 0)
			exit(NO_SELECTION);
		else
		if(strcmp(tz[i].area[j].name,"LOCAL") == 0)
		{
			x = Confirm_Selection_Routine(i,j);
			if(x)
			{
				dst = DST_Routine(i,j);
				Write_Out_Routine(i,j,dst,option);
				exit(0);
			}
			else
				continue;
		}

		for( ; ; )
		{
			j = Display_Secondary_Menu(i);
			if(j)
			{
				x = Confirm_Selection_Routine(i,j);
				if(x)
				{ 
					/* 001-kak */
					dst = DST_Routine(i,j);
					Write_Out_Routine(i,j,dst,option);
					exit(0);
				}
				else
					break;
			}
			else
				break;
		}
	}
}


Load_Data_Routine()
{
	FILE	*fp;
	int	i=0,j=1,dst;
	char	s1[BUF1],s2[BUF1],s3[BUF1],pre_zone[BUF1],line[LINE];

	if((fp=fopen(PATH1,"r"))==NULL)
	{
		/* 001-kak added newline to error message for neatness */
		printf("\n%s: ERROR - Cannot open %s !!\n",prog,PATH1);
		exit(1);
	}

	/* The following is used to allow us to read standard input from */
	/* a FILE pointer. This enables us to tell the difference between */
	/* an interactive session <CR> abd a true null read (such as an EOF) */
	/* or bad character. We use fgets() instead of gets() for this. 002 ds */

	filedes = fileno(stdin); 
	if((stinptr=fdopen(filedes, "r")) ==NULL)
	{
		printf("\n%s: ERROR - Cannot open standard input\n",prog);
		exit(1);
	}

	for ( ; fgets(line, LINE, fp)!=NULL; )
	{
		if((line[0] == '#') || (iscntrl(*line)) || (isspace(*line)))
			continue;
		sscanf(line,"%s %s %s %d", s1, s2, s3, &dst);
		if(strcmp(pre_zone,s1) != 0)
		{
			i++;
			j=1;
			strcpy(pre_zone,s1);
			strcpy(tz[i].name,s1);
		}
		strcpy(tz[i].area[j].name,s2);
		strcpy(tz[i].area[j].time,s3);
		tz[i].area[j].dst = dst;
		j++;
	}
	fclose(fp);
}


Display_Main_Menu()
{
	int	i,icc,occ,i_ans;
	/* increased size of s_ans in case it is read in from a file */
	char	s_ans[LINE],pre_zone[BUF1],output[BUF1]; /* 001-kak  */
	char	*temp_ans; /* 001-kak */

	printf("\n\n***** MAIN TIMEZONE MENU *****\n\n");
	printf("--------------------------------------");
	printf("--------------------------------------\n");

	for(i=1; strcmp(tz[i].name,"\0") != 0; i++)
	{	
		if(strcmp(pre_zone,tz[i].name) != 0)
		{
			strcpy(pre_zone,tz[i].name);
			strcpy(output,tz[i].name);

			for (icc=0; output[icc] != NULL; icc++)
				;

			while ( icc < SPACING )
			{
				strcat(output," ");
				icc++;
			}

			icc=icc+4;
			occ=occ+icc;

			if((i == 1) || (occ > 72))
			{
				printf("\n     ");
				occ=5;
			}

			if(i < 10)
				printf(" %d) %s", i, output);
			else
				printf("%d) %s", i, output);
		}
	}

	printf("\n\n      0) None of the above");
	printf("\n\n--------------------------------------");
	printf("--------------------------------------\n");
	printf("\nSelect the number above that best describes");
	printf("\nyour location: ");
	fflush(stdout);

	/* start 001-kak                                             */
	/* gets returns null when hit end of file or garbage so      */
	/* do not want to do atoi on a null also need to exit when   */
	/* hit end of input file                                     */

	temp_ans = (char *)malloc(LINE + 2);
	temp_ans = fgets(s_ans, 1024, stinptr);

	if ((temp_ans == (char *)NULL) && ((isatty(stdin)) == 0))
	{
		printf("\n\n%s\n%s\n%s\n",err_ban,err_msg2,err_ban);
		exit(1);
	}
	else if (temp_ans != (char *)NULL)
		i_ans=atoi(temp_ans);

	free(temp_ans);
	/* end 001-kak */

	if((isdigit(*s_ans) == 0 ) || (i_ans > i-1))
	{
		/* 001-kak added finishing newline */
		printf("\n\n%s\n%s\n%s\n",err_ban,err_msg,err_ban);
		Display_Main_Menu();
	}
	else
		return(i_ans);
}



Display_Secondary_Menu(i)
	int i;
{
	int	j,icc,occ,i_ans;
	/* increased size of s_ans in case it is read in from a file */
	char	s_ans[LINE],pre_zone[BUF1],output[BUF1]; /* 001-kak  */
	char	*temp_ans; /* 001-kak */

	printf("\n\n***** %s TIMEZONE MENU *****\n\n",tz[i].name);
	printf("--------------------------------------");
	printf("--------------------------------------\n");

	for(j=1; strcmp(tz[i].area[j].name,"\0") != 0; j++)
	{	
		if(strcmp(pre_zone,tz[i].area[j].name) != 0)
		{
			strcpy(pre_zone,tz[i].area[j].name);
			strcpy(output,tz[i].area[j].name);

			for (icc=0; output[icc] != NULL; icc++)
				;

			while ( icc < SPACING )
			{
				strcat(output," ");
				icc++;
			}

			icc=icc+4;
			occ=occ+icc;

			if((j == 1) || (occ > 72))
			{
				printf("\n     ");
				occ=5;
			}

			if(j < 10)
				printf(" %d) %s", j, output);
			else
				printf("%d) %s", j, output);
		}
	}

	printf("\n\n      0) None of the above");
	printf("\n\n--------------------------------------");
	printf("--------------------------------------\n");
	printf("\nSelect the number above that best describes");
	printf("\nyour location: ");
	fflush(stdout);
	
	/* start 001-kak                                             */
	/* gets returns null when hit end of file or garbage so      */
	/* do not want to do atoi on a null also need to exit when   */
	/* hit end of input file                                     */

	temp_ans = (char *)malloc(LINE + 2);
	temp_ans = fgets(s_ans, 1024, stinptr);

	if ((temp_ans == (char *)NULL) && ((isatty(stdin)) == 0))
	{
		printf("\n\n%s\n%s\n%s\n",err_ban,err_msg2,err_ban);
		exit(1);
	}
	else if (temp_ans != (char *)NULL)
		i_ans=atoi(temp_ans);

	free(temp_ans);
	/* end 001-kak */

	if((isdigit(*s_ans) == 0 ) || (i_ans > j-1))
	{
		printf("\n\n%s\n%s\n%s",err_ban,err_msg,err_ban);
		Display_Secondary_Menu(i);
	}
	else
		return(i_ans);
}


DST_Routine(i,j)
	int	i,j;
{
	int	x,dst_code,i_ans;
	FILE	*fp4;
	/* increased size of s_ans in case it is read in from a file  */
	char	s_ans[LINE],dst_area[30],line[LINE];
	char	*temp_ans; /* 001-kak */

	printf("\n\n*** DAYLIGHT SAVINGS TIME SPECIFICATION ***");
	for( ; ; )
	{
	    printf("\n\nDoes your area alternate between Daylight Savings ");
	    printf("\nand Standard time? (y/n) [y]: ");
	    fflush(stdout);
	    fgets(s_ans, 1024, stinptr);
	    switch(*s_ans)
	    {
		case '\n':
		case 'y':
		case 'Y':
			if(tz[i].area[j].dst != 0)
				return(tz[i].area[j].dst);
			else
				break;
		case '\0':
			/* start 001-kak need to check for end of file */
			if ((isatty(stdin)) == 0)
			{
				printf("\n\n%s\n%s\n%s\n",err_ban,err_msg2,err_ban);
				exit(1);
			}
			break;
			/* end 001-kak */

		case 'n':
		case 'N':
			return(0);
		default:
			*s_ans = '\0'; /* 001-kak need to reset value */
			printf("\n\n%s\n%s\n%s",err_ban,err_msg,err_ban);
			continue;
	    }
	    break;
	}

	if((fp4=fopen(PATH2,"r")) == NULL)
	{
		/* 001-kak added newline to error message for neatness */
		printf("\n%s: ERROR - Cannot open %s !!\n",prog,PATH2);
		exit(1);
	}

	for(x=1;fgets(line, LINE, fp4)!=NULL;)
	{
		if(line[0] != '#')
		{
			sscanf(line,"%d %[a-zA-Z ]",&dst_code,dst_area);
			dst_s[x].code = dst_code;
			strcpy(dst_s[x].area,dst_area);
			x++;
		}
	}

	fclose(fp4);

	for( ; ; )
	{
		printf("\n    Selection\tDST option");
		printf("\n------------------------");
		printf("------------------------\n");

		for(x=1; strcmp(dst_s[x].area,"\0") != 0; x++)
			printf("\t%d\t%s\n",dst_s[x].code,dst_s[x].area);

		printf("\n\t0\tNone of the above");
		printf("\n------------------------");
		printf("------------------------\n");
		printf("\nEnter the selection number that best describes ");
		printf("\nyour daylight savings area: ");
		fflush(stdout);

		/* start 001-kak                                             */
		/* gets returns null when hit end of file or garbage so      */
		/* do not want to do atoi on a null also need to exit when   */
		/* hit end of input file                                     */

		temp_ans = (char *)malloc(LINE + 2);
	        temp_ans = fgets(s_ans, 1024, stinptr);

		if ((temp_ans == (char *)NULL) && ((isatty(stdin)) == 0))
		{
			printf("\n\n%s\n%s\n%s\n",err_ban,err_msg2,err_ban);
			exit(1);
		}
		else if (temp_ans != (char *)NULL)
			i_ans=atoi(temp_ans);

		free(temp_ans);
		/* end 001-kak */

		if((isdigit(*s_ans) == 0) || (i_ans > x-1))
		{
			printf("\n\n%s\n%s\n%s\n",err_ban,err_msg,err_ban);
			continue;
		}
		else
		if(i_ans == 0)
			return(i_ans);
		else
		{
			printf("\nYou selected '%s' ",dst_s[i_ans].area);
			printf("as your daylight savings area.\n");
			for( ; ; )
			{
				printf("Is that correct? (y/n) [y]: ");
				fflush(stdout);
	        		fgets(s_ans, 1024, stinptr);
				switch(*s_ans)
				{
					case 'y':
					case 'Y':
					case '\0':
					case '\n':
						return(i_ans);
					case 'n':
					case 'N':
						break;
					default:
						continue;
				}
				break;
			}
		}
	}
}


Confirm_Selection_Routine(x,y)
	int	x,y;
{
	char	ans[LINE]; /* 001-kak */

	if(strcmp(tz[x].area[y].name,"LOCAL") == 0)
	{
		printf("\nYou selected '%s' as your time zone area.",
			tz[x].name);
	}
	else
	{
		printf("\nYou selected '%s %s' as your time zone area.",
			tz[x].name,tz[x].area[y].name);
	}

	for( ; ; )
	{
		printf("\nIs that correct? (y/n) [y]: ");
		fflush(stdout);
	        fgets(ans, 1024, stinptr);
		switch(*ans)
		{
			case '\n':
			case 'y':
			case 'Y':
				return(1);
			case '\0':
				/* start 001-kak check of end of input file */
				if ((isatty(stdin)) == 0)
				{
					printf("\n\n%s\n%s\n%s\n",err_ban,err_msg2,err_ban);
					exit(1);
				}
				/* end 001-kak */
			case 'n':
			case 'N':
				return(0);
			default:
				*ans = '\0'; /* 001-kak reset value */
				continue;
		}
	}
}


Write_Out_Routine(i,j,dst,option)
	int	i,j,dst,option;
{
	FILE	*fp3;
	char	s1[8],s2[8];

	if((fp3=fopen(PATH3,"w")) == NULL)
	{
		/* 001-kak added newline to error message for neatness */
		printf("\n%s: ERROR - Cannot create %s !!\n",prog,PATH3);
		exit(1);
	}

	sscanf(tz[i].area[j].time,"%[A-Z]%[-:0-9]",s1,s2);

	switch(option)
	{
		case 'd':
			fprintf(fp3,"TZC=\"%s dst %d\"\n",s2,dst);
			break;
		default:
			fprintf(fp3,"TZ=%s\n",tz[i].area[j].time);
			fprintf(fp3,"TZC=\"%s dst %d\"\n",s2,dst);
			break;
	}
	fclose(fp3);
}
