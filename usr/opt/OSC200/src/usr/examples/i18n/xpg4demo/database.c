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
static char *rcsid = "@(#)$RCSfile: database.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/25 22:03:01 $";
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "xpg4demo.h"
#include "xpg4demo_msg.h"

Boolean         ModifiedFlag = FALSE;	/* True if the employee data is modified */

/*
 * The employee record is normalized with the following format which
 * is locale independent:  Badge number, First Name, Surname,
 * Cost Center, Date of Join in the `yy/mm/dd' format. Each field is
 * separated by a TAB. The space character is allowed in the First
 * Name and Surname fields.
 */
static const char *dbOutFormat = "%ld\t%S\t%S\t%S\t%02d/%02d/%02d\n";
static const char *dbInFormat = "%ld %[^\t] %[^\t] %S %02d/%02d/%02d\n";
static FILE    *dbFp;			/* For sharing fp by twalk and its
					   action function */
static Employee *dbEmployeeFound;	/* True if the employee is found */
static BadgeNumber dbEmployeeBadge;	/* Badge number to be found */

/*
 * Check the file mode of the file named by the
 * `file' argument. The datatype FileMode is returned.
 */
FileMode
CheckFileMode(const char *file)
{
	if (access(file, R_OK|W_OK) == 0)
		return (FileWritable);

	if (access(file, R_OK) == 0)
		return (FileReadonly);

	if (errno == ENOENT)
		return (FileNew);

	return (FileBad);
}

/*
 * Read employee data records from the file specified by the `dbname'
 * argument. Zero is returned if successful; otherwise, -1 is returned.
 */
int
ReadDatabase(const char *dbname)
{
	Employee *emp;
	FILE *fp;
	char record[REC_LEN+1];
	char firstname[REC_FIRSTNM_LEN+1];
	char surname[REC_SURNM_LEN+1];

	if ((fp = fopen(dbname, "r")) == (FILE *) NULL) {
		return (-1);
	}
	while (fgets(record, sizeof(record), fp) != (char *) NULL) {
		if ((emp = (Employee *) calloc(sizeof(Employee), 1))
			== (Employee *) NULL)
			NoMemory(GetErrorMsg(E_DB_NOMEMORY, "employee data"));
		sscanf(record, dbInFormat,
		       &emp->badge_num,
		       firstname,
		       surname,
		       emp->cost_center,
		       &emp->date_of_join.tm_year,
		       &emp->date_of_join.tm_mon,
		       &emp->date_of_join.tm_mday);
		(void) mbstowcs(emp->first_name, firstname, FIRSTNAME_MAX+1);
		(void) mbstowcs(emp->surname, surname, SURNAME_MAX+1);
		--emp->date_of_join.tm_mon;
		TSearch(emp, CompareNames);
		if (CurrentEmployee == (Employee *) NULL)
			CurrentEmployee = emp;
	}
	fclose(fp);
	return (0);
}

/*
 * Action procedure for finding an employee whose badge number is
 * the same as `dbEmployeeBadge'. `dbEmployeeFound' is set if found.
 */
ActionProc
dbFindBadge(EmployeeNode *empnode, VISIT order, int level)
{
	Employee *emp;

	if (dbEmployeeFound)
		return;
	if (order == postorder || order == endorder)
		return;
	emp = empnode->employee;
	if (emp->badge_num == dbEmployeeBadge)
		dbEmployeeFound = emp;
}

/*
 * Search the employee data for the badge number specified by the `bn'
 * argument. A pointer to Employee is returned if found; otherwise,
 * (Emploee *) NULL is returned.
 */
Employee *
FindRecByBadgeNo(BadgeNumber bn)
{
	/*
	 * Set interface to the action procedure dbFinfBadge().
	 */
	dbEmployeeBadge = bn;
	dbEmployeeFound = (Employee *) NULL;
	TWalk(dbFindBadge);
	return (dbEmployeeFound);
}

/*
 * Action procedure for writing employee data to the database.
 * `dbFp' is the file to which data is written.
 */
ActionProc
dbWriteData(EmployeeNode *empnode, VISIT order, int level)
{
	Employee *emp = empnode->employee;

	/*
	 * If this is the second visit, or a leaf, write data to the database.
	 */
	if (!(order == postorder || order == leaf))
		return;

	fprintf(dbFp, dbOutFormat,
	       emp->badge_num,
	       emp->first_name,
	       emp->surname,
	       emp->cost_center,
	       emp->date_of_join.tm_year,
	       emp->date_of_join.tm_mon + 1,
	       emp->date_of_join.tm_mday);
}


/*
 * Write all the employee data records to the file specified by the
 * `dbname' argument. Zero is returned if successful; otherwise, -1 is
 * returned.
 */
int
WriteDatabase(const char *dbname)
{
	if ((dbFp = fopen(dbname, "w")) == (FILE *) NULL) {
		Error(GetErrorMsg(E_DB_WRITEOPEN,
				  "Cannot open %s for output"), dbname);
		return (-1);
	}

	TWalk(dbWriteData);
	(void) fclose(dbFp);
	return(0);
}
