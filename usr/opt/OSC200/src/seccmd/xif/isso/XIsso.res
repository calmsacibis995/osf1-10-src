! (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
! ALL RIGHTS RESERVED
!
! OSF/1 Release 1.0
!
! @(#)XIsso.res	1.4 10:07:26 8/30/90 SecureWare
! X resource file for the ISSO role program
! Defaults

XIsso*background:		   pink
XIsso*foreground:		   MidnightBlue

XIsso*fontList:                    -*-helvetica-medium-r-normal--12-*
XIsso*Title.fontList:              -*-helvetica-bold-r-normal--12-*
XIsso*SessionsLabel.fontList:      -*-courier-medium-r-normal--12-*
XIsso*SessionsLabel2.fontList:     -*-courier-medium-r-normal--12-*
XIsso*SessionsList.fontList:       -*-courier-medium-r-normal--12-*
XIsso*Edit.fontList:               -*-courier-medium-r-normal--12-*
XIsso*FileDisplay*List*fontList:   -*-courier-medium-r-normal--12-*
XIsso*CurrentFuture.spacing:       40
XIsso*Button*spacing:              30
XIsso*listSpacing:                 1
XIsso*listMarginHeight:            2
XIsso*listMarginWidth:             2
XIsso*SessionsList.topOffset:      0
XIsso*SessionsLabel2.topOffset:    0
XIsso*scrolledWindowMarginWidth:   1

! Primary Window and Attachment Window

XIsso*Main.width:                  600
XIsso*Main.height:                 400
XIsso*Main.borderWidth:            1

! Backup and Delete

XIsso*BackupDelete.topOffset:              0
XIsso*BackupDelete.leftOffset:             43
XIsso*BackupDelete.TextEdit.leftOffset:    10
XIsso*BackupDelete.TextEdit.width:         175
XIsso*BackupDelete.verticalSpacing:        18
XIsso*BackupDelete.RadioBox*spacing:       40
XIsso*BackupDelete.RadioBox.RadioButton*spacing: 10
XIsso*BackupDelete*visibleItemCount:       9

! Create Reports

XIsso*CreateReports.topOffset:             0
XIsso*CreateReports.leftOffset:            23
XIsso*CreateReports.horizontalSpacing:     20
XIsso*CreateReports.verticalSpacing:       11
XIsso*CreateReports*visibleItemCount:      6
XIsso*CreateReports.TextEdit.topOffset:    0
XIsso*CreateReports.ScrolledArea.topOffset: 0
XIsso*CreateReports.FileLabel.leftOffset:  110
XIsso*CreateReports.TextEdit.width:        140

! Directory List

XIsso*DirectoryList.leftOffset:            115
XIsso*DirectoryList.topOffset:             5
! XIsso*DirectoryList.horizontalSpacing:     20
! XIsso*DirectoryList.verticalSpacing:       21
XIsso*DirectoryList*ScrolledArea.width:    272
XIsso*DirectoryList*ScrolledArea.height:   215
XIsso*DirectoryList*TextEdit.topOffset:    3
XIsso*DirectoryList*TextEdit.width:        242
XIsso*DirectoryList*TextEdit.borderWidth:  1

! Display Reports (selection screen, not actual report)

XIsso*DisplayReports.horizontalSpacing:    20
XIsso*DisplayReports.verticalSpacing:      14
XIsso*DisplayReports.topOffset:             5
XIsso*DisplayReports.leftOffset:           60
XIsso*DisplayReports*visibleItemCount:     15

! Events

XIsso*Events.topOffset:                    0
XIsso*Events.leftOffset:                   15
XIsso*Events.verticalSpacing:              10
XIsso*Events.horizontalSpacing:            18
XIsso*Events.Form*verticalSpacing:         0
XIsso*Events.Form*horizontalSpacing:       3
XIsso*Events.Form.borderWidth:             1
XIsso*Events*RadioBox.marginWidth:         5
XIsso*Events*RadioBox.marginHeight:        1
XIsso*Events*RadioBox.spacing:             5

! File Display  -->  Help, Reports, Statistics

XIsso*FileDisplay.topOffset:               0
XIsso*FileDisplay.leftOffset:              0
XIsso*FileDisplay.verticalSpacing:         12
XIsso*FileDisplay.horizontalSpacing:       10
! XIsso*FileDisplay*visibleItemCount:        18
XIsso*FileDisplay*visibleItemCount:        16
XIsso*FileDisplay.ScrolledArea.width:      635

! Parameters

XIsso*Parameters.topOffset:                7
XIsso*Parameters.leftOffset:               70
XIsso*Parameters.verticalSpacing:          7
XIsso*Parameters.horizontalSpacing:        20

! Restore

XIsso*Restore.leftOffset:                  100
XIsso*Restore*TextEdit.leftOffset:         10
XIsso*Restore*TextEdit.width:              175
XIsso*Restore.topOffset:                   0
XIsso*Restore.verticalSpacing:             40
XIsso*Restore.horizontalSpacing:           20

! Selection Files  (window to select a selection file for modification)

XIsso*SelectionFiles.leftOffset:           110
XIsso*SelectionFiles.verticalSpacing:      12
XIsso*SelectionFiles.horizontalSpacing:    10
XIsso*SelectionFiles*visibleItemCount:     11
XIsso*SelectionFiles*TextEdit.width:       150

! Selection File  (window to modify a single selection file)

XIsso*SelectionFile.topOffset:             0
XIsso*SelectionFile.leftOffset:            20
XIsso*SelectionFile.verticalSpacing:       11
XIsso*SelectionFile.horizontalSpacing:     30
XIsso*SelectionFile*visibleItemCount:      4
XIsso*SelectionFile*EventsLabel.leftOffset:        10
XIsso*SelectionFile*EndTimeLabel.leftOffset:       60
XIsso*SelectionFile*Events.leftOffset:             10
XIsso*SelectionFile*Events.topOffset:              0
XIsso*SelectionFile*Events.borderWidth:            1
XIsso*SelectionFile*Events*Form.verticalSpacing:   0
XIsso*SelectionFile*Events*Form.horizontalSpacing: 2
XIsso*SelectionFile*Events*RadioBox.spacing:       5
XIsso*SelectionFile*Events*RadioBox.marginHeight:  1
XIsso*SelectionFile*Events.width:                  270
XIsso*SelectionFile*Events.height:                 89
XIsso*SelectionFile*Events*RadioBox.marginWidth:   2
XIsso*SelectionFile*FilesLabel.leftOffset:         50
XIsso*SelectionFile*Files.leftOffset:              50
XIsso*SelectionFile*Files.topOffset:               0
XIsso*SelectionFile*Files.borderWidth:             1
XIsso*SelectionFile*Files*Form.verticalSpacing:    0
XIsso*SelectionFile*Files*Form.horizontalSpacing:  0
XIsso*SelectionFile*Files.height:                  297
XIsso*SelectionFile*TextEdit.width:                140
XIsso*SelectionFile*TextEdit.borderWidth:          1
XIsso*SelectionFile*UsersLabel.leftOffset:         30
XIsso*SelectionFile*Users.leftOffset:              30
XIsso*SelectionFile*GroupsLabel.leftOffset:        50
XIsso*SelectionFile*Groups.leftOffset:             50

! Users Groups

XIsso*UsersGroups*visibleItemCount:        11
XIsso*UsersGroups.topOffset:               3
XIsso*UsersGroups.leftOffset:              120
XIsso*UsersGroups.horizontalSpacing:       20
XIsso*UsersGroups.verticalSpacing:         15
XIsso*UsersGroups*UsersLabel.leftOffset:   40
XIsso*UsersGroups*Users.leftOffset:        40
XIsso*UsersGroups*GroupsLabel.leftOffset:  45
XIsso*UsersGroups*Groups*leftOffset:       45

! Error Message

XIsso*Warning.borderWidth:                  1

! Number of users/groups/devices in selection box
! Default is 8.
XIsso*UserSelection*listVisibleItemCount:       	11
XIsso*DeviceSelection*listVisibleItemCount:     	11

XIsso*UserSelection*textColumns:       		25
XIsso*DeviceSelection*textColumns:     		30

XIsso*PrimaryGroup*visibleItemCount:       	9
XIsso*SecondaryGroup*visibleItemCount:     	9
