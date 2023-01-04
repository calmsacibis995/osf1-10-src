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
static char *rcsid = "@(#)$RCSfile: commands.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/03 15:55:34 $";
#endif

#include <stdlib.h>
#include <wchar.h>
#include "xpg4demo.h"
#include "xpg4demo_msg.h"
#include "commands.h"

/*
 * Command functions. If a nonzero value is returned by the function,
 * CommandLoop will return (exit from the loop).
 */
static int comModify(void),
	   comCreate(void),
	   comDelete(void),
	   comList(void),
	   comExit(void);

/*
 * Prototype of command table. InitCommand() will overwrite the
 * initial values.
 */
static Command command[COM_NCOMS] = {
	{ L'm', comModify, "m", S_COM_MODIFY },
	{ L'c', comCreate, "c", S_COM_CREATE },
	{ L'd', comDelete, "d", S_COM_DELETE },
	{ L'l', comList,   "l", S_COM_LIST },
	{ L'e', comExit,   "e", S_COM_EXIT },
};

/*
 * Initialize the command table with locale specific values.  Zero is
 * returned if the initialization succeeds; otherwise, -1 is returned.
 */
int
InitCommand(void)
{
	int i;
	Command *com;
	char *s;

	for (i = 0; i < COM_NCOMS; i++) {
		com = &command[i];
		if (mbtowc(&com->com_char,
			   (const char *) GetMessage(MSGString, com->com_id,
				(const char *) com->com_def),
			   MB_CUR_MAX) == -1)
			return (-1);
		/*
		 * one-character commands are held in lowercase.
		 */
		com->com_char = towlower(com->com_char);
	}
	/*
	 * The order of the first name and surname is locale specific.
	 */
	s = GetMsg(I_DB_SURNAME1ST, "n");
	if (*s == 'y') {
		CompareNames = CompSurnames;
		SurnameFirst = TRUE;
	} else
		CompareNames = CompFirstnames;
	return (0);
}

/*
 * Main loop for processing the command. The user can input either the 
 * numeric representation of the command (which is the index to an array) 
 * or a single character defined in the .msg file to select the command.
 * The command selection invokes each function associated with the command. 
 * If an invoked function returns a nonzero value, the loop terminates.
 */
void
CommandLoop(void)
{
	wchar_t	input[80];

	for (;;) {
		/*
		 * Display the command menu.
		 */
		ShowMenu();
		if (fgetws(input, NUM_ELEMENTS(input), stdin) == (wchar_t *) NULL) {
			if (ferror(stdin))
				Error(GetErrorMsg(E_COM_INPUT, "Cannot input"));

			return;
		}
		if (input[0] == L'\n')
			continue;
		ErasePromptLine();
		EraseMessageLine();
		/*
		 * If the input digit corresponds to an index in the command
		 * array, invoke the command function.
		 */
		if (iswdigit(input[0])) {
			long index;

			index = wcstol(input, (wchar_t **) NULL, 10);
			if (index > 0 && index <= COM_NCOMS) {
				if ((*command[index-1].com_action)())
					return;
				continue;
			}
		} else {
			int index;

			/*
			 * If the input character (in English, the first 
			 * one) identifies a command in the .msg file, 
			 * invoke the command.
			 */
			input[0] = towlower(input[0]);
			for (index = 0; index < COM_NCOMS; index++) {
				if (input[0] == command[index].com_char) {
					if ((*command[index].com_action)())
						return;
					break;
				}
			}
			continue;
		}
		Warning(GetErrorMsg(E_COM_UNKNOWN, "Unknown command"));
	}
	/*NOTREACHED*/
}

/*
 * Determine whether locale specifies first name or surname as the 
 * primary key for collating records.  Vary the order of name fields in   
 * display as appropriate for the locale.
 */
void
askOtherFields(Employee *emp)
{
	if (SurnameFirst) {
		InputSurname(emp);
		InputFirstname(emp);
	} else {
		InputFirstname(emp);
		InputSurname(emp);
	}
	InputCostCenter(emp);
	InputDOJ(emp);
}

/*
 * Action procedure for the List command.
 * `empnode' points to the node for an employee record.
 */
ActionProc
comListRecord(EmployeeNode *empnode, VISIT order, int level)
{
	Employee *emp = empnode->employee;
	char buf[80];			/* Assume 80 is wide enough */

	/*
	 * If this is the second visit, or a leaf, write data to the screen.
	 */
	if (!(order == postorder || order == leaf))
		return;

	/*
	 * Convert the DOJ to locale dependent representation.
	 */
	strftime(buf, sizeof(buf), GetStringMsg(S_DATE_FMT, "%x"),
		 &emp->date_of_join);
	printf(GetMsg(I_COM_DISP_LIST_FMT,"%6ld  %20S %-30S %3S %10s\n"),
	       emp->badge_num,
	       emp->first_name,
	       emp->surname,
	       emp->cost_center,
	       buf);
}

/*
 * Action procedure for the Modify command.
 */
static int
comModify(void)
{
	BadgeNumber badge;
	Employee *emp = (Employee *) NULL, *temp;
	char *msg;

	/*
	 * Make sure the employee data file is not empty.
	 */
	if (CurrentEmployee == (Employee *) NULL) {
		Warning(GetErrorMsg(E_COM_MODIFY,
				    "Data file contains no records to modify"));
		return (0);
	}

	/*
	 * Get memory for creating an updated employee data record. 
	 * Keeping the record in memory allows the program to discard 
	 * the update if the user does not confirm it.
	 */
	if ((temp = (Employee *) calloc(sizeof(Employee), 1)) == (Employee *) NULL)
		NoMemory(GetMsg(I_COM_NEWEMP, "new employee"));

	/*
	 * Prompt for the badge number of the record to be modified.
	 */
	badge = InputBadgeNo(CurrentEmployee,
			     GetMsg(I_COM_ASK_MOD_FMT,
				    "Enter the %s for the record to be modified [%ld]: "));
	/*
	 * Find the employee record that has the input badge number.
	 */
	if (CurrentEmployee->badge_num == badge) {
		emp = CurrentEmployee;
		*temp = *CurrentEmployee;
	} else {
		if ((emp = FindRecByBadgeNo(badge)) == (Employee *) NULL) {
			Error(GetErrorMsg(E_COM_FINDBADGE,
					  "Cannot find badge number %ld"), badge);
			return (0);
		}
		*temp = *emp;
	}

	/*
	 * Display the employee record and prompt user to confirm it
	 * is the correct record.
	 */
	ShowEmployee(temp);
	if (! Confirm(GetMsg(I_COM_YN_MODIFY,
			     "Do you want to modify this record?"))) {
		free((char *) temp);
		CurrentEmployee = emp;
		return (0);
	}

	/*
	 * User response indicates that the record is to be modified.
	 */
	/*
	 * The program automatically inputs the badge number.
	 */
	temp->badge_num = InputBadgeNo(emp, (const char *) NULL);
	askOtherFields(temp);

	/*
	 * Display the modified record and prompt the user to confirm 
	 * changes before writing them to the file.
	 */
	ShowEmployee(temp);
	if (Confirm(GetMsg(I_COM_YN_REPLACE,
			   "Are these the changes you want to make?"))) {
		TDelete(emp, CompareNames);
		free((char *) emp);
		TSearch(temp, CompareNames);
		ModifiedFlag = TRUE;
	} else {
		free((char *) temp);
		ShowEmployee(emp);
	}
	ErasePromptLine();
	return (0);
}

/*
 * Action procedure for the Create command.
 */
static int
comCreate(void)
{
	Employee *emp;
	BadgeNumber badge;

	/*
	 * Allocate memory for the new employee record.
	 */
	if ((emp = (Employee *) calloc(sizeof(Employee), 1)) == (Employee *) NULL)
		NoMemory(GetMsg(I_COM_NEWEMP, "new employee"));

	/*
	 * Prompt for the badge number first.
	 */
	emp->badge_num = InputBadgeNo((Employee *) NULL, (const char *) NULL);

	/*
	 * See if the input badge number already exists.
	 */
	if (FindRecByBadgeNo(emp->badge_num) != (Employee *) NULL) {
		Error(GetErrorMsg(E_COM_EXISTBADGE,
				  "Employee entry for badge number %ld already exists"),
		      emp->badge_num);
		free((char *) emp);
		return (0);
	}

	/*
	 * Prompt user to enter values for other fields.
	 */
	askOtherFields(emp);
	ShowEmployee(emp);
	TSearch(emp, CompareNames);
	ModifiedFlag = TRUE;
	return (0);
}

/*
 * Action procedure for the Create command.
 */
static int
comDelete(void)
{
	BadgeNumber badge;
	EmployeeNode *node;
	Employee *emp = (Employee *) NULL;
	/*
	 * See if employee data file is empty.
	 */
	if (CurrentEmployee == (Employee *) NULL) {
		Warning(GetErrorMsg(E_COM_NOTDEL, "Data file has no records to delete"));
		return (0);
	}

	/*
	 * Prompt for the badge number in the record to be deleted.
	 */
	badge = InputBadgeNo(CurrentEmployee,
			     GetMsg(I_COM_ASK_DEL_FMT,
				    "Enter the %s for the record to be deleted [%ld]: "));
	/*
	 * Find the employee record that has the badge number.
	 */
	if (CurrentEmployee->badge_num == badge) {
		emp = CurrentEmployee;
	} else {
		if ((emp = FindRecByBadgeNo(badge)) == (Employee *) NULL) {
			Error(GetErrorMsg(E_COM_FINDBADGE,
					  "Cannot find badge number %ld"), badge);
			return (0);
		}
	}

	/*
	 * Display the employee data and ask the user to confirm 
	 * the deletion.
	 */
	ShowEmployee(emp);
	if (! Confirm(GetMsg(I_COM_YN_DELETE, "Do you want to delete this record?")))
		return (0);

	/*
	 * Delete the employee record from the file and free the memory
	 * used by the procedure.
	 */
	TDelete(emp, CompareNames);
	free((char *) emp);

	/*
	 * Reset CurrentEmployee so that it no longer points to the
	 * deleted record.
	 */
	if (EmployeeRoot)
		ShowEmployee(CurrentEmployee = EmployeeRoot->employee);
	else {
		CurrentEmployee = (Employee *) NULL;
		ScrClear();
	}
	ModifiedFlag = TRUE;
	return (0);
}

/*
 * Action procedure for the List command.
 */
static int
comList(void)
{
	/*
	 * Make sure the employee data file is not empty.
	 */
	if (CurrentEmployee == (Employee *) NULL) {
		Error(GetErrorMsg(E_COM_NOENT, "Data file contains no records to display"));
		return (0);
	}

	/*
	 * Turn off full screen mode.
	 */
	ScrClear();
	FullScreenMode = FALSE;
	printf(GetStringMsg(S_COM_LIST_TITLE,
" Badge         Name                  Surname                CC      DOJ\n"));
	printf(GetStringMsg(S_COM_LIST_LINE,
"-----------------------------------------------------------------------------\n"));
	/*
	 * Walk the tree, listing records.
	 */
	TWalk(comListRecord);
	return (0);
}

/*
 * Action procedure for the Exit command.
 */
static int
comExit(void)
{
	FullScreenMode = FALSE;
	GotoMessageLine();
	/*
	 * Return a nonzero value to exit from CommandLoop.
	 */
	return (1);
}
